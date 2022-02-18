#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <OpenNI.h>

#include <vector>
#include <thread>

class IrTexture
{
public:
    std::vector<uint8_t> buffer{};
    int64_t textureId;
    openni::VideoStream *stream;
    int videoWidth = 0;
    int videoHeight = 0;
    cv::Mat cvImage;

    IrTexture(flutter::TextureRegistrar *textureRegistrar) : textureRegistrar(textureRegistrar)
    {
        texture =
            std::make_unique<flutter::TextureVariant>(flutter::PixelBufferTexture(
                [=](size_t width, size_t height) -> const FlutterDesktopPixelBuffer *
                {
                    return &flutterPixelBuffer;
                }));

        textureId = textureRegistrar->RegisterTexture(texture.get());
    };

    void genPixels()
    {
        int width = 80;
        int height = 60;
        int len = width * height * 4;
        buffer.resize(len);

        flutterPixelBuffer.buffer = buffer.data();
        flutterPixelBuffer.width = width;
        flutterPixelBuffer.height = height;

        for (int i = 0; i < width * height; i++)
        {
            buffer[4 * i] = 0;
            buffer[(4 * i) + 1] = 0;
            buffer[(4 * i) + 2] = 255;
            buffer[(4 * i) + 3] = 255;
        }
        textureRegistrar->MarkTextureFrameAvailable(textureId);
    }

    void setPixelBuffer()
    {
        flutterPixelBuffer.buffer = buffer.data();
        flutterPixelBuffer.width = videoWidth;
        flutterPixelBuffer.height = videoHeight;
    }

    void updateFrame()
    {
        cv::Mat img8bitDepth;
        cvImage.convertTo(img8bitDepth, CV_8U, 255.0 / 4096);
        for (int i = 0; i < videoWidth * videoHeight; i++)
        {
            buffer[4 * i] = *(img8bitDepth.data + i);
            buffer[(4 * i) + 1] = *(img8bitDepth.data + i);
            buffer[(4 * i) + 2] = *(img8bitDepth.data + i);
            buffer[(4 * i) + 3] = 255;
        }
        textureRegistrar->MarkTextureFrameAvailable(textureId);
    }

    ~IrTexture(){};

private:
    FlutterDesktopPixelBuffer flutterPixelBuffer{};
    flutter::TextureRegistrar *textureRegistrar = nullptr;
    std::unique_ptr<flutter::TextureVariant> texture = nullptr;
};