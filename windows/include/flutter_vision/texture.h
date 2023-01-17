#ifndef _DEF_TEXTURE_
#define _DEF_TEXTURE_

#include <OpenNI.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "pipeline/pipeline.h"

class FvTexture
{
public:
    FlutterDesktopPixelBuffer flutterPixelBuffer{};
    std::vector<uint8_t> buffer{};
    int64_t textureId = 0;
    openni::VideoStream *stream;
    int videoWidth = 0;
    int videoHeight = 0;
    Pipeline *pipeline;
    cv::Mat cvImage;

    FvTexture(flutter::TextureRegistrar *textureRegistrar): textureRegistrar(textureRegistrar)
    {
        pipeline = new Pipeline(&cvImage);

        flutterPixelBuffer.buffer = buffer.data();
        flutterPixelBuffer.width = videoWidth;
        flutterPixelBuffer.height = videoHeight;

        texture =
            std::make_unique<flutter::TextureVariant>(flutter::PixelBufferTexture(
                [=](size_t width, size_t height) -> const FlutterDesktopPixelBuffer *
                {
                    return &flutterPixelBuffer;
                }));

        textureId = textureRegistrar->RegisterTexture(texture.get());
    };

    void setPixelBuffer()
    {
        flutterPixelBuffer.buffer = buffer.data();
        flutterPixelBuffer.width = videoWidth;
        flutterPixelBuffer.height = videoHeight;
        textureRegistrar->MarkTextureFrameAvailable(textureId);
    }
private:
    flutter::TextureRegistrar *textureRegistrar = nullptr;
    std::unique_ptr<flutter::TextureVariant> texture = nullptr;
};


class OpenCVCamera
{
public:
    FvTexture *texture;
    int capIndex = -1;
    cv::VideoCapture *cap = nullptr;

    OpenCVCamera(int index, FvTexture *t, flutter::TextureRegistrar *r, std::vector<TFLiteModel *> *m, flutter::MethodChannel<flutter::EncodableValue> *c)
    {
        texture = t;
        capIndex = index;
        registrar = r;
        models = m;
        flChannel = c;
        if(cap == nullptr){
            cap = new cv::VideoCapture();
        }
    }

    void config(int prop, float value)
    {
        cap->set(prop, value);
    }

    bool open()
    {
        cap->open(capIndex);
        return cap->isOpened();
    }

    int start()
    {
        if (cap == nullptr)
            return -1;

        bool ret = cap->open(capIndex);
        if(!ret && !cap->isOpened())
            return -2;

        videoStart = true;
        std::thread t(&OpenCVCamera::_readVideoFeed, this);
        t.detach();

        return 0;
    }

    void stop()
    {
        videoStart = false;
    }

private:
    flutter::MethodChannel<flutter::EncodableValue> *flChannel;
    flutter::TextureRegistrar *registrar;
    std::vector<TFLiteModel *> *models;
    bool videoStart = false;
    void _readVideoFeed()
    {
        bool newFrame = false;

        if (!(videoStart))
            return;

        while (videoStart)
        {
            newFrame = cap->read(texture->cvImage);

            if (newFrame)
            {
                texture->pipeline->run(texture->cvImage, registrar, texture->textureId, texture->videoWidth, texture->videoHeight, texture->buffer, models, flChannel);
                texture->setPixelBuffer();
                flChannel->InvokeMethod("onUvcFrame", nullptr, nullptr);
            }
        }
    }
};
#endif