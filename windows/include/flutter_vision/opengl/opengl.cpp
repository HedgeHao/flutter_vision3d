#include "opengl.h"
#include "model_pointcloud.h"
#include "model_axis.h"

OpenGLFL::OpenGLFL(flutter::TextureRegistrar *textureRegistrar)
{
    cam = new Camera();
    pixelBuffer.resize(GL_WINDOW_WIDTH * GL_WINDOW_HEIGHT * GL_COLOR_CHANNEL);
    flutterPixelBuffer.buffer = pixelBuffer.data();
    flutterPixelBuffer.width = GL_WINDOW_WIDTH;
    flutterPixelBuffer.height = GL_WINDOW_HEIGHT;

    openglTexture = std::make_unique<OpenGLTexture>(textureRegistrar, &flutterPixelBuffer);
}

OpenGLFL::~OpenGLFL()
{
    if (window)
    {
        glfwDestroyWindow(window);
    }

    // TODO: Only one context support for now, so just terminate it.
    glfwTerminate();
}

int OpenGLFL::init()
{
    int ret = 0;

    ret = glfwInit();
    if (!ret)
        return -1;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window = glfwCreateWindow(GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT, "openglContext", NULL, NULL);
    if (!window)
        return -2;

    glfwMakeContextCurrent(window);

    ret = glewInit();
    if (ret != GLEW_OK)
        return -3;

    shader = new Shader();
    modelAxis = new ModelAxis(shader->vertextWithColor, FBO);
    modelAxis->init();
    modelPointCloud = new ModelPointCloud(shader->vertextWithColor, FBO, 1280, 720);
    modelPointCloud->init();
    modelRsPointCloud = new ModelRsPointCloud(shader->textureShader, FBO, 1280, 720, window);
    modelRsPointCloud->init();

    return 0;
}

void OpenGLFL::test()
{
    flutterPixelBuffer.buffer = pixelBuffer.data();
    flutterPixelBuffer.width = GL_WINDOW_WIDTH;
    flutterPixelBuffer.height = GL_WINDOW_HEIGHT;

    for (int i = 0; i < GL_WINDOW_WIDTH * GL_WINDOW_HEIGHT; i++)
    {
        pixelBuffer[4 * i] = 0;
        pixelBuffer[(4 * i) + 1] = 0;
        pixelBuffer[(4 * i) + 2] = 255;
        pixelBuffer[(4 * i) + 3] = 255;
    }
    openglTexture->markTextureAvailable();
}

void OpenGLFL::render()
{
    glfwMakeContextCurrent(window);
    glViewport(0, 0, GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelAxis->render(cam);
    modelPointCloud->render(cam, false);
    modelRsPointCloud->render(cam);
    glEnable(GL_DEPTH_TEST);
    glReadPixels(0, 0, GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer.data());

    openglTexture->markTextureAvailable();
}

void OpenGLFL::renderManually()
{
    if (isRendering)
        return;

    render();
}

void OpenGLFL::setCamPosition(float x, float y, float z)
{
    cam->position.x = x;
    cam->position.y = y;
    cam->position.z = z;
}

void OpenGLFL::setYawPitch(float yaw, float pitch)
{
    cam->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cam->forward.y = sin(glm::radians(pitch));
    cam->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
}

void OpenGLFL::setFov(float fov)
{
    cam->projection = glm::perspective(glm::radians(fov), (float)GL_WINDOW_WIDTH / (float)GL_WINDOW_HEIGHT, 0.1f, 100.f);
}