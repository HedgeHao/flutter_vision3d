#ifndef _MODEL_POINTCLOUD_HEADER_
#define _MODEL_POINTCLOUD_HEADER_

#include <gl/glew.h>
#include <glm/ext.hpp>
#include "camera.hpp"

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

#endif