#ifndef _OPENGLFL_HEADER_
#define _OPENGLFL_HEADER_

#include "opengl_texture.h" // window.h should include before glew

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define GL_WINDOW_WIDTH 1280
#define GL_WINDOW_HEIGHT 720
#define GL_COLOR_CHANNEL 4

#include "model_axis.h"
#include "model_pointcloud.h"

#include "shader.h"
#include <memory>

class OpenGLFL
{
public:
    FlutterDesktopPixelBuffer flutterPixelBuffer{};
    std::vector<uint8_t> pixelBuffer{};
    ModelAxis *modelAxis;
    ModelPointCloud *modelPointCloud;
    std::unique_ptr<OpenGLTexture> openglTexture;
    bool isRendering = false;

    OpenGLFL(flutter::TextureRegistrar *);
    ~OpenGLFL();
    int init();
    void render();
    void renderManually();
    void setCamPosition(float, float, float);
    void setYawPitch(float, float);
    void setFov(float);
    void test();

private:
    GLFWwindow *window;
    Camera *cam;
    Shader *shader;
    unsigned int FBO = 0;
};

#endif