#ifndef _DEF_TEXTURE_
#define _DEF_TEXTURE_

#include <OpenNI.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class Pipeline;

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

    FvTexture(flutter::TextureRegistrar *textureRegistrar) : textureRegistrar(textureRegistrar)
    {
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
#endif