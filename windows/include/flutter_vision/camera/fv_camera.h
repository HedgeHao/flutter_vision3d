#ifndef _DEF_CAMERA_
#define _DEF_CAMERA_
#include <flutter/plugin_registrar_windows.h>
#include <iostream>

#include "../pipeline/pipeline.h"
#include "../texture.h"
#include "../opengl/opengl.h"

enum VideoIndex
{
    RGB = 0b1,
    Depth = 0b10,
    IR = 0b100,
    OPENGL = 0b1000,
    Camera2D = 0b10000,
};

class FvCamera
{
public:
    flutter::TextureRegistrar *flRegistrar;
    OpenGLFL *glfl;
    std::vector<TFLiteModel *> *models;
    flutter::MethodChannel<flutter::EncodableValue> *flChannel;

    std::string serial;
    std::unique_ptr<FvTexture> rgbTexture;
    std::unique_ptr<FvTexture> depthTexture;
    std::unique_ptr<FvTexture> irTexture;
    bool videoStart = false;
    bool enablePointCloud = false;

    FvCamera() {}

    FvCamera(const char *s)
    {
        serial = std::string(s);
    }

    static FvCamera *findCam(const char *serial, std::vector<FvCamera *> *cams)
    {
        FvCamera *cam = nullptr;
        for (auto c : *cams)
        {
            if (strcmp(c->serial.c_str(), serial) == 0)
            {
                return c;
            }
        }

        return nullptr;
    }

    void fvInit(flutter::TextureRegistrar *r, std::vector<TFLiteModel *> *m, flutter::MethodChannel<flutter::EncodableValue> *f, OpenGLFL *g)
    {
        flRegistrar = r;
        // TODO: check if this is duplicate from texture
        models = m;
        flChannel = f;
        glfl = g;

        // Create texture
        rgbTexture = std::make_unique<FvTexture>(flRegistrar);
        depthTexture = std::make_unique<FvTexture>(flRegistrar);
        irTexture = std::make_unique<FvTexture>(flRegistrar);
    }

    int64_t getTextureId(int index)
    {
        if (index == VideoIndex::RGB)
        {
            return rgbTexture->textureId;
        }
        else if (index == VideoIndex::Depth)
        {
            return depthTexture->textureId;
        }
        else if (index == VideoIndex::IR)
        {
            return irTexture->textureId;
        }

        return -1;
    }

    virtual void camInit() = 0;
    virtual int openDevice() = 0;
    virtual int closeDevice() = 0;
    virtual int isConnected() = 0;
    virtual int configVideoStream(int streamIndex, bool *enable) = 0;
    virtual void readVideoFeed() = 0;
    virtual void configure(int prop, std::vector<float> &value) = 0;

private:
    virtual void _readVideoFeed() = 0;
};
#endif