#ifndef _DEF_PIPELINE_
#define _DEF_PIPELINE_

#include <flutter_linux/flutter_linux.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
#include <algorithm>
#include "../fv_texture.h"

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
    void (*func)(FvTexture *, std::vector<uint8_t> params, FlTextureRegistrar &, std::vector<TFLiteModel *> *, FlMethodChannel *);
    std::vector<uint8_t> params = {};
    bool runOnce = false;
};

void PipelineFuncTest(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    printf("[Pipeline:Test()] %d\n", params[0]);
}

void PipelineFuncOpencvCvtColor(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    // printf("PipelineFuncOpencvCvtColor:%d\n", params[0]);
    cv::cvtColor(fv->cvImage, fv->cvImage, params[0]);
}

/**
 * @brief cv::imwrite
 *
 * @param params first byte is length of file path string
 */
void PipelineFuncOpencvImwrite(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    std::string path;
    std::stringstream ss;
    for (int i = 1; i <= params[0]; i++)
        ss << params[i];
    ss >> path;

    cv::imwrite(path.c_str(), fv->cvImage);
}

void PipelineFuncOpencvImread(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    std::string path;
    std::stringstream ss;
    for (int i = 1; i <= params[0]; i++)
        ss << params[i];
    ss >> path;

    fv->cvImage = cv::imread(path.c_str());
}

void PipelineFuncShow(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    // printf("PipelineFuncShow: %d, %d\n", fv->cvImage.cols, fv->cvImage.rows);
    fv->video_width = fv->cvImage.cols;
    fv->video_height = fv->cvImage.rows;
    fv->buffer.clear();
    fv->buffer.resize(fv->video_width * fv->video_height * 4);
    fv->buffer.assign(fv->cvImage.data, fv->cvImage.data + fv->cvImage.total() * fv->cvImage.channels());
    fl_texture_registrar_mark_texture_frame_available(&registrar, FL_TEXTURE(fv));
}

/**
 * @brief cv::Mat::convertTo
 *
 * @param params 0: convert type, 1~4: alpha (double)
 */
void PipelineFuncOpencvConvertTo(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    // printf("ConvertTo:Param:%d\n", params[0]);
    float scale = *reinterpret_cast<float *>(&params[1]);
    float shift = *reinterpret_cast<float *>(&params[5]);
    fv->cvImage.convertTo(fv->cvImage, params[0], scale, shift);
}

void PipelineFuncOpencvApplyColorMap(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    cv::applyColorMap(fv->cvImage, fv->cvImage, params[0]);
}

/**
 * @brief OpenCV resize
 *
 * @param params [0-1]: width, [2-3]: height [4]: mode
 */
void PipelineFuncOpencvResize(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    int width = (params[0] << 8) + params[1];
    int height = (params[2] << 8) + params[3];

    cv::resize(fv->cvImage, fv->cvImage, cv::Size(width, height), params[4]);
}

/**
 * @brief Crop image
 *
 * @param params x, y, width, height: each contains 2 bytes
 */
void PipelineFuncCrop(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    // TODO: Check ROI is valid
    int xStart = (params[0] << 8) + params[1];
    int xEnd = (params[2] << 8) + params[3];
    int yStart = (params[4] << 8) + params[5];
    int yEnd = (params[6] << 8) + params[7];
    fv->cvImage = fv->cvImage(cv::Range(yStart, yEnd), cv::Range(xStart, xEnd));
    // printf("Crop: %d, %d, %d\n", fv->cvImage.cols, fv->cvImage.rows, fv->cvImage.channels());
}

void PipelineFuncOpencvRectangle(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
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
    cv::rectangle(fv->cvImage, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(b, g, r, alpha), thickness, lineType, shift);
}

void PipelineFuncOpencvRotate(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    cv::rotate(fv->cvImage, fv->cvImage, params[0]);
}

void PipelineFuncTfSetInputTensor(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    if (params[2] == 0)
        models->at(params[0])->setInput<uint8_t>(params[1], fv->cvImage, fv->cvImage.cols * fv->cvImage.rows * fv->cvImage.channels());
    else if (params[2] == 1)
        models->at(params[0])->setInput<float>(params[1], fv->cvImage, fv->cvImage.cols * fv->cvImage.rows * fv->cvImage.channels());
}

void PipelineFuncTfInference(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    bool success = models->at(params[0])->inference();
    if (!success)
    {
        printf("Inference Failed!!!!\n");
        return;
    }

    fl_method_channel_invoke_method(flChannel, "onInference", nullptr, nullptr, nullptr, NULL);
}

void PipelineFuncCustomHandler(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    int size = (params[0] << 8) + params[1];
    float *result = new float[size]{0};
    flutterVisionHandler(fv->cvImage, result);
    fl_method_channel_invoke_method(flChannel, "onHandled", fl_value_new_float32_list(result, size), nullptr, nullptr, NULL);
}

void PipelineFuncOpencvNormalize(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    float alpha = *reinterpret_cast<float *>(&params[0]);
    float beta = *reinterpret_cast<float *>(&params[4]);
    uint8_t normType = params[8];
    uint8_t dType = params[9];

    cv::normalize(fv->cvImage, fv->cvImage, alpha, beta, normType, dType);
}

void PipelineFuncOpencvThreshold(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    float threshold = *reinterpret_cast<float *>(&params[0]);
    float max = *reinterpret_cast<float *>(&params[4]);
    uint8_t type = params[8];

    cv::threshold(fv->cvImage, fv->cvImage, threshold, max, type);
}

void PipelineFuncOpencvRelu(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    float threshold = *reinterpret_cast<float *>(&params[0]);
    uchar thresholdValueUchar = static_cast<uchar>(threshold * 255.0);
    uchar *p;
    for (int i = 0; i < fv->cvImage.rows; ++i)
    {
        p = fv->cvImage.ptr<uchar>(i);
        for (int j = 0; j < fv->cvImage.cols; ++j)
        {
            if (p[j] < thresholdValueUchar)
            {
                p[j] = 0;
            }
        }
    }
}

void PipelineZeroDepthFilter(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    cv::Mat temp;
    fv->cvImage.copyTo(temp);

    int threshold = params[0];
    int halfRange = params[1];

    cv::parallel_for_(cv::Range(halfRange, fv->cvImage.rows - halfRange), [&](const cv::Range &rowRange)
                      {
        for (int i = rowRange.start; i < rowRange.end; ++i)
        {
            for (int j = halfRange; j < fv->cvImage.cols - halfRange; ++j)
            {
                if (fv->cvImage.at<uchar>(i, j) <= threshold)
                {
                    int sum = 0;
                    int count = 0;

                    for (int x = -halfRange; x <= halfRange; ++x)
                    {
                        for (int y = -halfRange; y <= halfRange; ++y)
                        {
                            if (temp.at<uchar>(i + x, j + y) != 0)
                            {
                                sum += temp.at<uchar>(i + x, j + y);
                                ++count;
                            }
                        }
                    }

                    if (count > 0)
                    {
                        fv->cvImage.at<uchar>(i, j) = sum / count;
                    }
                }
            }
        } });
}

void PipelineCopyTo(FvTexture *fv, std::vector<uint8_t> params, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
{
    uint64_t pointer =
        (static_cast<uint64_t>(params[0]) << 56) +
        (static_cast<uint64_t>(params[1]) << 48) +
        (static_cast<uint64_t>(params[2]) << 40) +
        (static_cast<uint64_t>(params[3]) << 32) +
        (static_cast<uint64_t>(params[4]) << 24) +
        (static_cast<uint64_t>(params[5]) << 16) +
        (static_cast<uint64_t>(params[6]) << 8) +
        static_cast<uint64_t>(params[7]);

    std::uintptr_t p = pointer;
    cv::Mat *mat = (cv::Mat *)p;
    fv->cvImage.copyTo(*mat);
    // std::cout << "[CopyTo]: " << pointer << "," << mat->cols << "," << mat->rows << "," << mat->channels() << std::endl;
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
    {14, "cvNormalized", 0, 0, PipelineFuncOpencvNormalize},
    {15, "cvThreshold", 0, 0, PipelineFuncOpencvThreshold},
    {16, "relu", 0, 0, PipelineFuncOpencvRelu},
    {17, "zeroDepthFilter", 0, 0, PipelineZeroDepthFilter},
    {18, "PipelineCopyTo", 0, 0, PipelineCopyTo},
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

    void add(unsigned int index, const uint8_t *params, unsigned int len, int insertAt = -1, int interval = 0, bool append = false, bool runOnce = false)
    {
        FuncDef f = pipelineFuncs[index];
        f.interval = interval;
        f.runOnce = runOnce;

        if (runOnce)
        {
            runOnceFinished = false;
        }

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

    int removeAt(unsigned int index)
    {
        funcs.erase(funcs.begin() + index);
        return 0;
    }

    int runOnce(FvTexture *fv, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel, int &from, int &to)
    {
        if (to == -1 || to >= funcs.size())
            to = funcs.size();

        for (int i = from; i < to; i++)
        {
            // printf("Run:%s\n", funcs[i].name);
            try
            {
                error = "";
                funcs[i].func(fv, funcs[i].params, registrar, models, flChannel);
            }
            catch (std::exception &e)
            {
                error = e.what();
                return -1;
            }
        }

        return 0;
    }

    int run(FvTexture *fv, FlTextureRegistrar &registrar, std::vector<TFLiteModel *> *models, FlMethodChannel *flChannel)
    {
        std::vector<size_t> removeIndex = {};

        isRunning = true;
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
                funcs[i].func(fv, funcs[i].params, registrar, models, flChannel);
                if (funcs[i].runOnce)
                    removeIndex.push_back(i);
            }
            catch (std::exception &e)
            {
                error = e.what();
                return -1;
            }

            if (funcs[i].interval > 0)
            {
                getCurrentTime(&funcs[i].timer);
            }
        }

        for (size_t index : removeIndex)
        {
            if (index < funcs.size())
            {
                funcs.erase(funcs.begin() + index);
            }
        }

        if (!removeIndex.empty())
        {
            runOnceFinished = true;
        }

        isRunning = false;

        return 0;
    }

    void clear()
    {
        while(isRunning);

        funcs.clear();
    }

    void reset()
    {
        for (int i = 0; i < funcs.size(); i++)
        {
            funcs[i].timer = 0;
        }
    }

    bool checkRunOnceFinished()
    {
        if (runOnceFinished)
        {
            runOnceFinished = false;
            return true;
        }

        return false;
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
    cv::Mat *imgPtr;
    cv::Mat img;

    bool runOnceFinished = true;
    bool isRunning = false;
};
#endif