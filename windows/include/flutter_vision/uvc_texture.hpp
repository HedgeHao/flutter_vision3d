#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>
#include <thread>

#include "texture.h"

class UvcTexture : public Texture
{
public:
    UvcTexture(flutter::TextureRegistrar *textureRegistrar) : textureRegistrar(textureRegistrar)
    {
        texture =
            std::make_unique<flutter::TextureVariant>(flutter::PixelBufferTexture(
                [=](size_t width, size_t height) -> const FlutterDesktopPixelBuffer *
                {
                    return &flutterPixelBuffer;
                }));

        textureId = textureRegistrar->RegisterTexture(texture.get());
    };

    ~UvcTexture(){};

private:
    flutter::TextureRegistrar *textureRegistrar = nullptr;
    std::unique_ptr<flutter::TextureVariant> texture = nullptr;
};

class OpenCVCamera
{
public:
    UvcTexture *texture;
    int capIndex = -1;
    cv::VideoCapture *cap = nullptr;

    OpenCVCamera(int index, UvcTexture *t, flutter::TextureRegistrar *r, std::vector<TFLiteModel *> *m, flutter::MethodChannel<flutter::EncodableValue> *c)
    {
        texture = t;
        capIndex = index;
        registrar = r;
        models = m;
        flChannel = c;
    }

    bool open()
    {
        cap = new cv::VideoCapture(capIndex);
        return cap->isOpened();
    }

    void start()
    {
        if (cap == nullptr)
            return;

        videoStart = true;
        std::thread t(&OpenCVCamera::_readVideoFeed, this);
        t.detach();
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
            }
        }
    }
};