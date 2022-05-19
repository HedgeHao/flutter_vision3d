#ifndef _DEF_OPENGL_
#define _DEF_OPENGL_
#include <gtk/gtk.h>
#include <GL/glew.h>
#include <glm/ext.hpp>
#include "shader.h"
#include <librealsense2/rs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <mutex>

#include "opengl_texture.h"

#define GL_WINDOW_WIDTH 1280
#define GL_WINDOW_HEIGHT 720
#define GL_COLOR_CHANNEL 4

class Camera
{
public:
    glm::mat4 projection;
    glm::mat4 modelIdentified;

    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 target = glm::vec3(0, 0, 0);
    glm::vec3 worldUp = glm::vec3(0, 1.0f, 0);

    Camera()
    {
        position = glm::vec3(-0.0f, 0.0f, -3.0f);
        forward = glm::normalize(target - position);
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(forward, right));

        projection = glm::perspective(glm::radians(45.0f), (float)GL_WINDOW_WIDTH / (float)GL_WINDOW_HEIGHT, 0.1f, 100.0f);
        modelIdentified = glm::mat4(1.0f);
    }
};

class ModelRsPointCloud
{
public:
    rs2::points points;

    ModelRsPointCloud(GdkGLContext *g, unsigned int shader, unsigned int fbo, unsigned int w, unsigned int h)
    {
        gdkContext = g;
        shaderProgram = shader;
        FBO = fbo;
        width = w;
        height = h;

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    void updateTexture(const rs2::video_frame &frame)
    {
        if (!frame)
            return;

        gdk_gl_context_make_current(gdkContext);
        glBindTexture(GL_TEXTURE_2D, TEXTURE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.get_width(), frame.get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, frame.get_data());
    }

    void render(Camera *cam)
    {
        if (!points || points.get_data_size() == 0)
            return;

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
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
    unsigned int VBO_TEX;
    unsigned int FBO;
    unsigned int TEXTURE;
    unsigned int width;
    unsigned int height;
    GdkGLContext *gdkContext;
};

class ModelPointCloud
{
public:
    unsigned int vertexPoints = 0;
    float *vertices;
    float *colors;
    float *colorsMap;

    ModelPointCloud(unsigned int shader, unsigned int fbo, unsigned int w, unsigned int h)
    {
        shaderProgram = shader;
        FBO = fbo;
        width = w;
        height = h;

        vertices = new float[w * h * 3];

        colorsMap = new float[w * h * 3];

        colors = new float[w * h * 3];
        for (int i = 0; i < w * h * 3; i++)
        {
            if (i % 3 == 0)
                colors[i] = 1.0f;
            else
                colors[i] = 0.0f;
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

        glGenBuffers(1, &VBO_COLOR);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_COLOR);
        glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    }

    void render(Camera *cam, bool colorMap)
    {
        if (vertexPoints == 0)
            return;

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_VERTEX);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexPoints * 3, vertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_COLOR);
        if (colorMap)
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexPoints * 3, colorsMap, GL_DYNAMIC_DRAW);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexPoints * 3, colors, GL_DYNAMIC_DRAW);
        }
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(cam->modelIdentified));

        glm::mat4 viewMat(1.0f);
        viewMat = glm::lookAt(cam->position, cam->position + cam->forward, cam->worldUp);

        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));

        unsigned int projLoc = glGetUniformLocation(shaderProgram, "proj");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam->projection));

        glPointSize(1.0);
        glDrawArrays(GL_POINTS, 0, vertexPoints);
    }

private:
    unsigned int shaderProgram;
    unsigned int VAO;
    unsigned int VBO_VERTEX;
    unsigned int VBO_COLOR;
    unsigned int FBO;
    unsigned int width;
    unsigned int height;
};

class ModelAxis
{
public:
    ModelAxis(unsigned int shader, unsigned int fbo)
    {
        shaderProgram = shader;
        FBO = fbo;
    }

    void init()
    {
        float vertices[] = {
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};
        unsigned int indices[] = {
            0, 3,
            1, 4,
            2, 5};

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

    void render(Camera *cam)
    {
        glUseProgram(shaderProgram);

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(cam->modelIdentified));

        glm::mat4 viewMat(1.0f);
        viewMat = glm::lookAt(cam->position, cam->position + cam->forward, cam->worldUp);

        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));

        unsigned int projLoc = glGetUniformLocation(shaderProgram, "proj");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam->projection));

        glLineWidth(3);
        glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, (void *)0);
    }

private:
    unsigned int shaderProgram;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int FBO;
};

class OpenGLFL
{
public:
    GdkGLContext *gdkContext;
    uint8_t *pixelBuffer;
    ModelAxis *modelAxis;
    ModelPointCloud *modelPointCloud;
    ModelRsPointCloud *modelRsPointCloud;

    OpenGLFL(GdkWindow *w, FlTextureRegistrar *r, OpenGLTexture *t)
    {
        gdkWindow = w;
        registrar = r;
        openglTexture = t;
        GError *error = NULL;
        gdkContext = gdk_window_create_gl_context(gdkWindow, &error);
        gdk_gl_context_make_current(gdkContext);

        if (glewInit() != GLEW_OK)
            return;
        initFrameBuffer();

        cam = new Camera();
        shader = new Shader();
        modelAxis = new ModelAxis(shader->vertextWithColor, FBO);
        modelAxis->init();
        modelPointCloud = new ModelPointCloud(shader->vertextWithColor, FBO, 1280, 720);
        modelPointCloud->init();
        modelRsPointCloud = new ModelRsPointCloud(gdkContext, shader->textureShader, FBO, 1280, 720);
        modelRsPointCloud->init();

        pixelBuffer = new uint8_t[GL_WINDOW_WIDTH * GL_WINDOW_HEIGHT * GL_COLOR_CHANNEL];
    };
    ~OpenGLFL(){};

    void render()
    {
        gdk_gl_context_make_current(gdkContext);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glViewport(0, 0, GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        modelAxis->render(cam);
        modelPointCloud->render(cam, false);
        modelRsPointCloud->render(cam);
        glEnable(GL_DEPTH_TEST);
        glReadPixels(0, 0, GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // screenshot();

        fl_texture_registrar_mark_texture_frame_available(registrar, FL_TEXTURE(openglTexture));
    };

    void setCamPosition(float x, float y, float z)
    {
        cam->position.x = x;
        cam->position.y = y;
        cam->position.z = z;
    }

    void setYawPitch(float yaw, float pitch)
    {
        cam->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cam->forward.y = sin(glm::radians(pitch));
        cam->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    }

    void setFov(float fov)
    {
        cam->projection = glm::perspective(glm::radians(fov), (float)GL_WINDOW_WIDTH / (float)GL_WINDOW_HEIGHT, 0.1f, 100.f);
    }

private:
    GdkWindow *gdkWindow;
    FlTextureRegistrar *registrar;
    OpenGLTexture *openglTexture;
    Shader *shader;
    unsigned int FBO = 0;
    unsigned int texture = 0;

    Camera *cam;

    void initFrameBuffer()
    {
        if (FBO == 0)
        {
            glGenFramebuffers(1, &FBO);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        if (texture == 0)
        {
            glGenTextures(1, &texture);
        }
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            return;
        }
    }

    void screenshot()
    {
        cv::Mat m(GL_WINDOW_HEIGHT, GL_WINDOW_WIDTH, CV_8UC4, pixelBuffer, 0);
        cv::rotate(m, m, cv::ROTATE_180);
        cv::imwrite("test.png", m);
    }
};
#endif