#ifndef _MODEL_AXIS_
#define _MODEL_AXIS_

#include <gl/glew.h>
#include <glm/ext.hpp>
#include "camera.hpp"

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
    unsigned int FBO = 0;
};

#endif