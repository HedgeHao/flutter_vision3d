#ifndef _OPENGL_TEXTURE_HEADER_
#define _OPENGL_TEXTURE_HEADER_

#include <flutter/plugin_registrar_windows.h>

#include <iostream>

class OpenGLTexture
{
public:
    int64_t textureId;

    OpenGLTexture(flutter::TextureRegistrar *textureRegistrar, FlutterDesktopPixelBuffer *flb) : textureRegistrar(textureRegistrar)
    {
        texture =
            std::make_unique<flutter::TextureVariant>(flutter::PixelBufferTexture(
                [=](size_t width, size_t height) -> const FlutterDesktopPixelBuffer *
                {
                    return flb;
                }));

        textureId = textureRegistrar->RegisterTexture(texture.get());
    };

    void markTextureAvailable()
    {
        textureRegistrar->MarkTextureFrameAvailable(textureId);
    }

private:
    flutter::TextureRegistrar *textureRegistrar = nullptr;
    std::unique_ptr<flutter::TextureVariant> texture = nullptr;
};

#endif