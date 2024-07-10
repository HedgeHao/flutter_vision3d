#ifndef _CAMERA_HEADER_
#define _CAMERA_HEADER_

#include <glm/ext.hpp>

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
    };
};
#endif