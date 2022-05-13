#ifndef _DEF_TEXTURE_
#define _DEF_TEXTURE_

#include <OpenNI.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "pipeline/pipeline.h"

class Texture
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

    Texture()
    {
        pipeline = new Pipeline(&cvImage);

        flutterPixelBuffer.buffer = buffer.data();
        flutterPixelBuffer.width = videoWidth;
        flutterPixelBuffer.height = videoHeight;
    };

    void setPixelBuffer()
    {
        flutterPixelBuffer.buffer = buffer.data();
        flutterPixelBuffer.width = videoWidth;
        flutterPixelBuffer.height = videoHeight;
    }
};
#endif