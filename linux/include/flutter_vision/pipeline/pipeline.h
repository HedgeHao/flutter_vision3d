#ifndef _DEF_PIPELINE_
#define _DEF_PIPELINE_

#include <flutter_linux/flutter_linux.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
#include <algorithm>

#include <sys/time.h>
void getCurrentTime(int64_t *timer)
{
    struct timeval now
    {
    };

    gettimeofday(&now, nullptr);
    *timer = (now.tv_sec * 1000) + now.tv_usec / 1000;
}

#include "../tflite.h"
#include "flutter_vision_handler.h"

struct FuncDef
{
    unsigned int index;
    const char *name;
    int interval = 0;
    int64_t timer = 0;
    // TODO: function parameter definition should be relaxable
    void (*func)(cv::Mat &, std::vector<uint8_t> params, FlTextureRegistrar &, FlTexture &, int32_t &, int32_t &, std::vector<uint8_t> &, std::vector<TFLiteModel *> *, FlMethodChannel *);
    std::vector<uint8_t> params = {};
};

void PipelineFuncTest(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    printf("[Pipeline:Test()] %d\n", params[0]);
}

void PipelineFuncOpencvCvtColor(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    // printf("PipelineFuncOpencvCvtColor:%d\n", params[0]);
    cv::cvtColor(img, img, params[0]);
}

/**
 * @brief cv::imwrite
 *
 * @param params first byte is length of file path string
 */
void PipelineFuncOpencvImwrite(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    std::string path;
    std::stringstream ss;
    for (int i = 1; i <= params[0]; i++)
        ss << params[i];
    ss >> path;

    cv::imwrite(path.c_str(), img);
}

void PipelineFuncOpencvImread(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    std::string path;
    std::stringstream ss;
    for (int i = 1; i <= params[0]; i++)
        ss << params[i];
    ss >> path;

    img = cv::imread(path.c_str());
}

void PipelineFuncShow(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    // printf("PipelineFuncShow: %d, %d\n", img.cols, img.rows);
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
void PipelineFuncOpencvConvertTo(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    // printf("ConvertTo:Param:%d\n", params[0]);
    float scale = *reinterpret_cast<float *>(&params[1]);
    float shift = *reinterpret_cast<float *>(&params[5]);
    img.convertTo(img, params[0], scale, shift);
}

void PipelineFuncOpencvApplyColorMap(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    cv::applyColorMap(img, img, params[0]);
}

/**
 * @brief OpenCV resize
 *
 * @param params [0-1]: width, [2-3]: height [4]: mode
 */
void PipelineFuncOpencvResize(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
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
void PipelineFuncCrop(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    // TODO: Check ROI is valid
    int xStart = (params[0] << 8) + params[1];
    int xEnd = (params[2] << 8) + params[3];
    int yStart = (params[4] << 8) + params[5];
    int yEnd = (params[6] << 8) + params[7];
    img = img(cv::Range(yStart, yEnd), cv::Range(xStart, xEnd));
    // printf("Crop: %d, %d, %d\n", img.cols, img.rows, img.channels());
}

void PipelineFuncOpencvRectangle(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
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

void PipelineFuncOpencvRotate(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    cv::rotate(img, img, params[0]);
}

void PipelineFuncTfSetInputTensor(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    if (params[2] == 0)
        models->at(params[0])->setInput<uint8_t>(params[1], img, img.cols * img.rows * img.channels());
    else if (params[2] == 1)
        models->at(params[0])->setInput<float>(params[1], img, img.cols * img.rows * img.channels());
}

void PipelineFuncTfInference(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    bool success = models->at(params[0])->inference();
    if (!success)
    {
        printf("Inference Failed!!!!\n");
        return;
    }

    fl_method_channel_invoke_method(flChannel, "onInference", nullptr, nullptr, nullptr, NULL);
}

void PipelineFuncCustomHandler(cv::Mat &img, std::vector<uint8_t> params, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    int size = (params[0] << 8) + params[1];
    float *result = new float[size]{0};
    flutterVisionHandler(img, result);
    fl_method_channel_invoke_method(flChannel, "onHandled", fl_value_new_float32_list(result, size), nullptr, nullptr, NULL);
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
    {13, "customHandler", 0, 0, PipelineFuncCustomHandler},
};

class Pipeline
{
public:
    std::string error = "";

    Pipeline()
    {
        img = cv::Mat(1, 1, CV_8UC4, cv::Scalar(255, 0, 0, 255));
        imgPtr = &img;
    }

    Pipeline(cv::Mat *m)
    {
        imgPtr = m;
    }

    void add(unsigned int index, const uint8_t *params, unsigned int len, int insertAt = -1, int interval = 0, bool append = false)
    {
        FuncDef f = pipelineFuncs[index];
        f.interval = interval;

        for (unsigned int i = 0; i < len; i++)
            f.params.push_back(*(params + i));

        if (append)
        {
            if (insertAt == -1)
                funcs.push_back(f);
            else
            {
                if (insertAt > (int)funcs.size() - 1)
                    insertAt = funcs.size() - 1;

                if (insertAt < 0)
                    insertAt = 0;

                funcs.insert(funcs.begin() + insertAt, f);
            }
        }
        else
        {
            if (insertAt == -1)
                funcs.push_back(f);
            else
                funcs.at(insertAt) = f;
        }
    }

    int runOnce(FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel, int &from, int &to)
    {
        if (to == -1 || to >= funcs.size())
            to = funcs.size();

        for (int i = from; i < to; i++)
        {
            // printf("Run:%s\n", funcs[i].name);
            try
            {
                error = "";
                funcs[i].func(img, funcs[i].params, registrar, texture, texture_width, texture_height, pixelBuf, models, flChannel);
            }
            catch (std::exception &e)
            {
                error = e.what();
                return -1;
            }
        }

        return 0;
    }

    int run(cv::Mat &img, FlTextureRegistrar &registrar, FlTexture &texture, int32_t &texture_width, int32_t &texture_height, std::vector<uint8_t> &pixelBuf, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
    {
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

            try
            {
                // printf("[Run] %s\n", funcs[i].name);
                funcs[i].func(img, funcs[i].params, registrar, texture, texture_width, texture_height, pixelBuf, models, flChannel);
            }
            catch (cv::Exception &e)
            {
                error = e.what();
                return -1;
            }

            if (funcs[i].interval > 0)
            {
                getCurrentTime(&funcs[i].timer);
            }
        }

        return 0;
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

    void reset()
    {
        for (int i = 0; i < funcs.size(); i++)
        {
            funcs[i].timer = 0;
        }
    }

    std::string getPipelineInfo()
    {
        std::string info;

        for (int i = 0; i < funcs.size(); i++)
        {
            info += funcs[i].name;
            info += " => ";
        }

        return info;
    }

private:
    std::vector<FuncDef> funcs = {};
    int64_t ts = 0;
    bool doScreenshot = false;
    std::string screenshotSavePath;
    int screenshotCvtColor = -1;
    cv::Mat *imgPtr;
    cv::Mat img;
};
#endif