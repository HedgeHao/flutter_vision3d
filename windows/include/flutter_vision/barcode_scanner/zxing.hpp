#include "ZXing/ReadBarcode.h"
#include "opencv2/core/core.hpp"

namespace ZXing
{
    inline ZXing::ImageView ImageViewFromMat(const cv::Mat &image)
    {
        using ZXing::ImageFormat;

        auto fmt = ImageFormat::None;
        switch (image.channels())
        {
        case 1:
            fmt = ImageFormat::Lum;
            break;
        case 3:
            fmt = ImageFormat::BGR;
            break;
        case 4:
            fmt = ImageFormat::BGRX;
            break;
        }

        if (image.depth() != CV_8U || fmt == ImageFormat::None)
            return {nullptr, 0, 0, ImageFormat::None};

        return {image.data, image.cols, image.rows, fmt};
    }

    inline ZXing::Results ReadBarcodes(const cv::Mat &image, const ZXing::DecodeHints &hints = {})
    {
        return ZXing::ReadBarcodes(ImageViewFromMat(image), hints);
    }
}