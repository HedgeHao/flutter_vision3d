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
    bool pauseStream = false;
    int type;
    bool videoFeedProcessing = false;
    uint16_t *depthData = new uint16_t[1280 * 720];
    int depthWidth = 0, depthHeight = 0;
    cv::Rect crop;

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

    static bool removeCam(const char *serial, std::vector<FvCamera *> *cams)
    {
        for (int i = 0; i < cams->size(); i++)
        {
            if (strcmp(cams->at(i)->serial.c_str(), serial) == 0)
            {
                cams->erase(cams->begin() + i);
                return true;
            }
        }

        return false;
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

        // Create pipeline
        rgbTexture->pipeline = new Pipeline();
        depthTexture->pipeline = new Pipeline();
        irTexture->pipeline = new Pipeline();
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

    // TODO: check if long is enough
    uintptr_t getOpenCVMat(int index)
    {
        if (index == VideoIndex::RGB)
        {
            return reinterpret_cast<std::uintptr_t>(&rgbTexture->cvImage);
        }
        else if (index == VideoIndex::IR)
        {
            return reinterpret_cast<std::uintptr_t>(&irTexture->cvImage);
        }
        else if (index == VideoIndex::Depth)
        {
            return reinterpret_cast<std::uintptr_t>(&depthTexture->cvImage);
        }

        return 0;
    }

    void pause(bool p)
    {
        pauseStream = p;

        // [HedgeHao]
        // Wait for video feed loop finish, or memory leak will happened.
        // Empty while loop is not working. Must do something inside. DKW.
        if (p)
        {
            while (videoFeedProcessing)
            {
                std::cout << "";
            }
        }
    }

    void setCrop(int startX, int endX, int startY, int endY)
    {
        crop = cv::Rect(startX, startY, endX, endY);
        std::cout << "Set Crop Area:" << startX << "," << startY << "," << endX << "," << endY << std::endl;
    }

    uint16_t *getDepthData()
    {
        return depthData;
    }

    virtual void camInit() = 0;
    virtual int openDevice() = 0;
    virtual int closeDevice() = 0;
    virtual int isConnected() = 0;
    virtual int configVideoStream(int streamIndex, bool *enable) = 0;
    virtual void readVideoFeed() = 0;
    virtual void configure(int prop, std::vector<float> &value) = 0;
    virtual int getConfiguration(int prop) = 0;
    virtual void getIntrinsic(int index, double &fx, double &fy, double &cx, double &cy) = 0;
    virtual bool enableImageRegistration(bool enable) = 0;
    virtual void getAvailableVideoModes(int index, std::vector<std::string> &) = 0;
    virtual void getCurrentVideoMode(int index, std::string &mode) = 0;
    virtual bool setVideoMode(int index, int mode) = 0;
    virtual bool getSerialNumber(std::string &sn) = 0;
    virtual void loadPresetParameters(std::string &path) = 0;

private:
    virtual void _readVideoFeed() = 0;
};
#endif