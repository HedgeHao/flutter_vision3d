#include <librealsense2/rs.hpp>
#include <gl/glew.h>
#include <glm/ext.hpp>
#include "camera.hpp"

#include <opencv2/core/core.hpp>

class ModelRsPointCloud
{
public:
    rs2::frame *rgbFrame;
    rs2::points points;

    ModelRsPointCloud(unsigned int shader, unsigned int fbo, unsigned int w, unsigned int h, GLFWwindow* win)
    {
        shaderProgram = shader;
        FBO = fbo;
        width = w;
        height = h;
        window = win;

        vertices = new float[w * h * 3];
        textureCoord = new float[w * h * 2];
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                textureCoord[(y * w + x) * 2] = (float)x / (float)w;
                textureCoord[(y * w + x) * 2 + 1] = (float)y / (float)h;
            }
        }
    }

    void init()
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO_VERTEX);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_VERTEX);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        glGenBuffers(1, &VBO_TEX);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_TEX);
        glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoord), textureCoord, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

        glGenTextures(1, &TEXTURE);
        glBindTexture(GL_TEXTURE_2D, TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        cv::Mat g(100, 100, CV_8UC3, cv::Scalar(255, 255, 0));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g.cols, g.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, g.data);
    }

    void updateTexture()
    {
        if (!rgbFrame)
            return;

        auto frame = rgbFrame->as<rs2::video_frame>();
        glBindTexture(GL_TEXTURE_2D, TEXTURE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.get_width(), frame.get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, frame.get_data());
    } 

    void render(Camera *cam)
    {
        if (!points || points.get_data_size() == 0)
            return;

        // [HedgeHao]: glfw context cannot access from different thread at the same time
        updateTexture();

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(2);

        unsigned int count = 0;
        const rs2::vertex *rsVertices = points.get_vertices();
        const rs2::texture_coordinate *rsTextureCoord = points.get_texture_coordinates();

        for (int i = 0; i < points.size(); i++)
        {
            if (rsVertices[i].z)
            {
                vertices[count * 3] = rsVertices[i].x * 0.5f;
                vertices[count * 3 + 1] = rsVertices[i].y * -0.5f;
                vertices[count * 3 + 2] = rsVertices[i].z - 1.5f;
                textureCoord[count * 2] = rsTextureCoord[i].u;
                textureCoord[count * 2 + 1] = rsTextureCoord[i].v;
                count++;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO_VERTEX);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count * 3, vertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        glBindTexture(GL_TEXTURE_2D, TEXTURE);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_TEX);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count * 2, textureCoord, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(cam->modelIdentified));

        glm::mat4 viewMat(1.0f);
        viewMat = glm::lookAt(cam->position, cam->position + cam->forward, cam->worldUp);
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));

        unsigned int projLoc = glGetUniformLocation(shaderProgram, "proj");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam->projection));

        glPointSize(1.0);
        glDrawArrays(GL_POINTS, 0, count);
    }

private:
    float *vertices;
    float *textureCoord;
    unsigned int shaderProgram;
    unsigned int VAO;
    unsigned int VBO_VERTEX;
    unsigned int VBO_COLOR;
    unsigned int VBO_TEX;
    unsigned int FBO;
    unsigned int TEXTURE;
    unsigned int width;
    unsigned int height;
    GLFWwindow *window;

};