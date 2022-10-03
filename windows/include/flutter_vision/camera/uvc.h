#ifndef _DEF_UVC_CAM_
#define _DEF_UVC_CAM_
#include <flutter/plugin_registrar_windows.h>

#include <iostream>

#include "fv_camera.h"

#include <opencv2/core/core.hpp>

class UvcCam : public FvCamera
{
public:
    UvcCam(const char *s) : FvCamera(s){};

    void camInit()
    {
        uvcIndex = stoi(serial);
        if (uvcIndex >= 0)
        {
            cap = new cv::VideoCapture();
        }
    }

    int openDevice()
    {
        if (!cap)
            return -1;
        bool ret = cap->open(uvcIndex);
        if (!ret)
            return -2;
        return cap->isOpened() ? 0 : -3;
    }

    int closeDevice()
    {
        cap->release();
        return 0;
    }

    int isConnected() { return 0; }

    int configVideoStream(int streamIndex, bool *enable)
    {
        if (!cap)
            return -1;

        if (!cap->isOpened())
            return -2;

        videoStart = *enable;

        return 0;
    }

    void readVideoFeed()
    {
        videoStart = true;
        std::thread t(&UvcCam::_readVideoFeed, this);
        t.detach();
    }

    void configure(int prop, std::vector<float> &value)
    {
        if (!cap)
            return;

        cap->set(prop, value[0]);
    }

    int getConfiguration(int prop) { return 0; }

private:
    int uvcIndex = -1;
    cv::VideoCapture *cap;
    void _readVideoFeed()
    {
        bool newFrame = false;

        if (!(videoStart))
            return;

        while (videoStart)
        {
            newFrame = cap->read(rgbTexture->cvImage);

            if (newFrame)
            {
                rgbTexture->pipeline->run(rgbTexture->cvImage, flRegistrar, rgbTexture->textureId, rgbTexture->videoWidth, rgbTexture->videoHeight, rgbTexture->buffer, models, flChannel);
                rgbTexture->setPixelBuffer();
                flChannel->InvokeMethod("onUvcFrame", nullptr, nullptr);
            }
        }
    }
};
#endif