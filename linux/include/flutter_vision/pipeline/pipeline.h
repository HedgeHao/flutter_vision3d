#ifndef _DEF_PIPELINE_
#define _DEF_PIPELINE_

#include <flutter_linux/flutter_linux.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
#include <algorithm>
struct FuncDef
{
    unsigned int index;
    const char *name;
    // TODO: function parameter definition should be relaxable
    void (*func)(cv::Mat &, std::vector<uint8_t> params, FlTextureRegistrar &, FlTexture &, int32_t &, int32_t &, std::vector<uint8_t> &);
    std::vector<uint8_t> params = {};
};

void PipelineFuncTest(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    printf("[Pipeline:Test()] %d\n", params[0]);
}

void PipelineFuncOpencvCvtColor(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    // printf("PipelineFuncOpencvCvtColor:%d\n", params[0]);
    cv::cvtColor(img, img, params[0]);
}

/**
 * @brief cv::imwrite
 *
 * @param params first byte is length of file path string
 */
void PipelineFuncOpencvImwrite(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    std::string path;
    std::stringstream ss;
    for (int i = 1; i <= params[0]; i++)
        ss << params[i];
    ss >> path;

    cv::imwrite(path.c_str(), img);
}

void PipelineFuncOpencvImread(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    std::string path;
    std::stringstream ss;
    for (int i = 1; i <= params[0]; i++)
        ss << params[i];
    ss >> path;

    img = cv::imread(path.c_str());
}

void PipelineFuncShow(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    // printf("PipelineFuncShow\n");
    texture_width = img.cols;
    texture_height = img.rows;
    pixelBuf.clear();
    pixelBuf.resize(texture_width * texture_height * 4);
    pixelBuf.assign(img.data, img.data + img.total() * img.channels());
    fl_texture_registrar_mark_texture_frame_available(&registrar, FL_TEXTURE(&texture));
}

/**
 * @brief cv::Mat::convertTo
 *
 * @param params 0: convert type, 1~4: alpha (double)
 */
void PipelineFuncOpencvConvertTo(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    // printf("ConvertTo:Param:%d\n", params[0]);
    float alpha = *reinterpret_cast<float *>(&params[1]);
    img.convertTo(img, params[0], alpha);
}

void PipelineFuncOpencvApplyColorMap(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    cv::applyColorMap(img, img, params[0]);
}

/**
 * @brief OpenCV resize
 *
 * @param params [0-1]: width, [2-3]: height [4]: mode
 */
void PipelineFuncOpencvResize(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    int width = (params[0] << 8) + params[1];
    int height = (params[2] << 8) + params[3];

    cv::resize(img, img, cv::Size(width, height), params[4]);
}

/**
 * @brief Crop image
 *
 * @param params x, y, width, height: each contains 2 bytes
 */
void PipelineFuncCrop(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    // TODO: Check ROI is valid
    int xStart = (params[0] << 8) + params[1];
    int xEnd = (params[2] << 8) + params[3];
    int yStart = (params[4] << 8) + params[5];
    int yEnd = (params[6] << 8) + params[7];
    img = img(cv::Range(yStart, yEnd), cv::Range(xStart, xEnd));
    // printf("Crop: %d, %d, %d\n", img.cols, img.rows, img.channels());
}

void PipelineFuncOpencvRectangle(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    float x1 = *reinterpret_cast<float *>(&params[0]);
    float y1 = *reinterpret_cast<float *>(&params[4]);
    float x2 = *reinterpret_cast<float *>(&params[8]);
    float y2 = *reinterpret_cast<float *>(&params[12]);
    uint8_t r = params[16];
    uint8_t g = params[17];
    uint8_t b = params[18];
    uint8_t alpha = params[19];
    uint8_t thickness = params[20];
    uint8_t lineType = params[21];
    uint8_t shift = params[22];
    cv::rectangle(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(b, g, r, alpha), thickness, lineType, shift);
}

void PipelineFuncOpencvRotate(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
{
    cv::rotate(img, img, params[0]);
}

const FuncDef pipelineFuncs[] = {
    {0, "test", PipelineFuncTest},
    {1, "cvtColor", PipelineFuncOpencvCvtColor},
    {2, "imwrite", PipelineFuncOpencvImwrite},
    {3, "show", PipelineFuncShow},
    {4, "convertTo", PipelineFuncOpencvConvertTo},
    {5, "applyColorMap", PipelineFuncOpencvApplyColorMap},
    {6, "resize", PipelineFuncOpencvResize},
    {7, "crop", PipelineFuncCrop},
    {8, "imread", PipelineFuncOpencvImread},
    {9, "cvRectangle", PipelineFuncOpencvRectangle},
    {10, "rotate", PipelineFuncOpencvRotate},
};

class Pipeline
{
public:
    void add(unsigned int index, const uint8_t *params, unsigned int len, int insertAt = -1)
    {
        FuncDef f = pipelineFuncs[index];

        for (unsigned int i = 0; i < len; i++)
            f.params.push_back(*(params + i));

        if (insertAt == -1)
            funcs.push_back(f);
        else
            funcs.at(insertAt) = f;
    }

    void run(cv::Mat &img, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf)
    {
        for (int i = 0; i < funcs.size(); i++)
        {
            funcs[i].func(img, funcs[i].params, registrar, texture, texture_width, texture_height, pixelBuf);
        }
    }

    void clear()
    {
        funcs.clear();
    }

private:
    std::vector<FuncDef> funcs = {};
};
#endif