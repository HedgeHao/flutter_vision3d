#ifndef _DEF_PIPELINE_
#define _DEF_PIPELINE_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
#include <algorithm>

#include "../tflite.h"

void getCurrentTime(int64_t *timer)
{
    *timer = GetTickCount();
}
struct FuncDef
{
    unsigned int index;
    const char *name;
    int interval = 0;
    int64_t timer = 0;
    // TODO: function parameter definition should be flexable
    void (*func)(cv::Mat &, std::vector<uint8_t> params, flutter::TextureRegistrar *, int64_t &, int32_t &, int32_t &, std::vector<uint8_t> &, std::vector<TFLiteModel *> *, flutter::MethodChannel<flutter::EncodableValue> *);
    std::vector<uint8_t> params = {};
};

void PipelineFuncTest(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    printf("[Pipeline:Test()] %d\n", params[0]);
}

void PipelineFuncOpencvCvtColor(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    // printf("PipelineFuncOpencvCvtColor:%d\n", params[0]);
    cv::cvtColor(img, img, params[0]);
}

/**
 * @brief cv::imwrite
 *
 * @param params first byte is length of file path string
 */
void PipelineFuncOpencvImwrite(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    std::string path;
    std::stringstream ss;
    for (int i = 1; i <= params[0]; i++)
        ss << params[i];
    ss >> path;

    cv::imwrite(path.c_str(), img);
}

void PipelineFuncOpencvImread(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    std::string path;
    std::stringstream ss;
    for (int i = 1; i <= params[0]; i++)
        ss << params[i];
    ss >> path;

    img = cv::imread(path.c_str());
}

void PipelineFuncShow(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    // printf("PipelineFuncShow\n");
    texture_width = img.cols;
    texture_height = img.rows;
    pixelBuf.clear();
    pixelBuf.resize(texture_width * texture_height * 4);
    pixelBuf.assign(img.data, img.data + img.total() * img.channels());
    registrar->MarkTextureFrameAvailable(textureId);
}

/**
 * @brief cv::Mat::convertTo
 *
 * @param params 0: convert type, 1~4: alpha (double)
 */
void PipelineFuncOpencvConvertTo(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    // printf("ConvertTo:Param:%d\n", params[0]);
    float scale = *reinterpret_cast<float *>(&params[1]);
    float shift = *reinterpret_cast<float *>(&params[5]);
    img.convertTo(img, params[0], scale, shift);
}

void PipelineFuncOpencvApplyColorMap(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    cv::applyColorMap(img, img, params[0]);
}

/**
 * @brief OpenCV resize
 *
 * @param params [0-1]: width, [2-3]: height [4]: mode
 */
void PipelineFuncOpencvResize(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
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
void PipelineFuncCrop(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    // TODO: Check ROI is valid
    int xStart = (params[0] << 8) + params[1];
    int xEnd = (params[2] << 8) + params[3];
    int yStart = (params[4] << 8) + params[5];
    int yEnd = (params[6] << 8) + params[7];
    img = img(cv::Range(yStart, yEnd), cv::Range(xStart, xEnd));
    // printf("Crop: %d, %d, %d\n", img.cols, img.rows, img.channels());
}

void PipelineFuncOpencvRectangle(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
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

void PipelineFuncOpencvRotate(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    cv::rotate(img, img, params[0]);
}

void PipelineFuncTfSetInputTensor(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    if (params[2] == 0)
        models->at(params[0])->setInput<uint8_t>(params[1], img, img.cols * img.rows * img.channels());
    else if (params[2] == 1)
        models->at(params[0])->setInput<float>(params[1], img, img.cols * img.rows * img.channels());
}

void PipelineFuncTfInference(cv::Mat &img, std::vector<uint8_t> params, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
{
    bool success = models->at(params[0])->inference();
    if (!success)
        printf("Inference Failed!!!!\n");
}

const FuncDef pipelineFuncs[] = {
    {0, "test", 0, 0, PipelineFuncTest},
    {1, "cvtColor", 0, 0, PipelineFuncOpencvCvtColor},
    {2, "imwrite", 0, 0, PipelineFuncOpencvImwrite},
    {3, "show", 0, 0, PipelineFuncShow},
    {4, "convertTo", 0, 0, PipelineFuncOpencvConvertTo},
    {5, "applyColorMap", 0, 0, PipelineFuncOpencvApplyColorMap},
    {6, "resize", 0, 0, PipelineFuncOpencvResize},
    {7, "crop", 0, 0, PipelineFuncCrop},
    {8, "imread", 0, 0, PipelineFuncOpencvImread},
    {9, "cvRectangle", 0, 0, PipelineFuncOpencvRectangle},
    {10, "rotate", 0, 0, PipelineFuncOpencvRotate},
    {11, "tfSetTenorInput", 0, 0, PipelineFuncTfSetInputTensor},
    {12, "tfInference", 0, 0, PipelineFuncTfInference},
};

class Pipeline
{
    bool doScreenshot = false;
    std::string screenshotSavePath;
    int screenshotCvtColor = -1;

public:
    void add(unsigned int index, const std::vector<uint8_t> &params, unsigned int len, int insertAt = -1, int interval = 0)
    {
        FuncDef f = pipelineFuncs[index];
        f.interval = interval;

        for (unsigned int i = 0; i < len; i++)
            f.params.push_back(params.at(i));

        if (insertAt == -1)
            funcs.push_back(f);
        else
            funcs.at(insertAt) = f;
    }

    void run(cv::Mat &img, flutter::TextureRegistrar *registrar, int64_t &textureId, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, flutter::MethodChannel<flutter::EncodableValue> *flChannel)
    {
        for (int i = 0; i < funcs.size(); i++)
        {
            if (funcs[i].interval > 0)
            {
                getCurrentTime(&ts);
                if (ts - funcs[i].timer < funcs[i].interval)
                {
                    continue;
                }
            }

            // std::cout << "Run:" << funcs[i].name << std::endl;
            funcs[i].func(img, funcs[i].params, registrar, textureId, texture_width, texture_height, pixelBuf, models, flChannel);

            if (funcs[i].interval > 0)
            {
                getCurrentTime(&funcs[i].timer);
            }
        }

        if (doScreenshot)
        {
            if (!img.empty())
            {
                if (screenshotCvtColor > 0)
                {
                    cv::Mat temp;
                    cv::cvtColor(img, temp, screenshotCvtColor);
                    cv::imwrite(screenshotSavePath.c_str(), temp);
                }
                else
                {
                    cv::imwrite(screenshotSavePath.c_str(), img);
                }
            }
            doScreenshot = false;
        }
    }

    void screenshot(const char *filePath, int convert = -1)
    {
        doScreenshot = true;
        screenshotSavePath = std::string(filePath);
        screenshotCvtColor = convert;
    }

    void clear()
    {
        funcs.clear();
    }

private:
    std::vector<FuncDef> funcs = {};
    int64_t ts = 0;
};
#endif