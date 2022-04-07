#ifndef _DEF_TF_PIPELINE_
#define _DEF_TF_PIPELINE_

#include <iostream>
#include <opencv2/core/core.hpp>

#include "../tflite.h"
struct TfFuncDef
{
    unsigned int index;
    const char *name;
    // TODO: function parameter definition should be relaxable
    void (*func)(cv::Mat &, cv::Mat &, cv::Mat &, std::vector<uint8_t> params, TFLiteModel &);
    std::vector<uint8_t> params = {};
};

void PipelineFuncTfSetInputTensor(cv::Mat &rgb, cv::Mat &depth, cv::Mat &ir, std::vector<uint8_t> params, TFLiteModel &model)
{
    cv::Mat *img;
    if (params[0] == 0)
        img = &rgb;
    else if (params[0] == 1)
        img = &depth;
    else if (params[0] == 2)
        img = &ir;
    else
        return;

    if (params[2] == 0)
        model.setInput<uint8_t>(params[1], *img, img->cols * img->rows * img->channels());
    else if (params[2] == 1)
        model.setInput<float>(params[1], *img, img->cols * img->rows * img->channels());
}

void PipelineFuncTfInference(cv::Mat &rgb, cv::Mat &depth, cv::Mat &ir, std::vector<uint8_t> params, TFLiteModel &model)
{
    bool success = model.inference();
    if (!success)
        printf("Inference Failed!!!!\n");
}

const TfFuncDef tfPipelineFuncs[] = {
    {0, "setInputTensor", PipelineFuncTfSetInputTensor},
    {1, "inference", PipelineFuncTfInference},
};

class TfPipeline
{
public:
    void add(unsigned int index, const uint8_t *params, unsigned int len, int insertAt = -1)
    {
        TfFuncDef f = tfPipelineFuncs[index];

        for (unsigned int i = 0; i < len; i++)
            f.params.push_back(*(params + i));

        if (insertAt == -1)
            funcs.push_back(f);
        else
            funcs.at(insertAt) = f;
    }

    void run(cv::Mat &rgb, cv::Mat &depth, cv::Mat &ir, TFLiteModel &model)
    {
        for (int i = 0; i < funcs.size(); i++)
        {
            funcs[i].func(rgb, depth, ir, funcs[i].params, model);
        }

        // TODO: need callback after TF invoke
    }

    void clear()
    {
        funcs.clear();
    }

private:
    std::vector<TfFuncDef> funcs = {};
};
#endif