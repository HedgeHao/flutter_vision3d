#ifndef _DEF_TFLITE_
#define _DEF_TFLITE_

#include <iostream>
#include <memory>
#include <vector>

#include <opencv2/core/core.hpp>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/optional_debug_tools.h>

struct TensorOutput
{
    int tensorIndex;
    unsigned int index;
    unsigned int size;
    unsigned int type;
    const char *name;
};

class TFLiteModel
{
public:
    bool valid = false;
    std::string error;
    std::vector<TensorOutput> outputTensors{};
    std::unique_ptr<tflite::Interpreter> interpreter;

    TFLiteModel(const char *modelPath)
    {
        model = tflite::FlatBufferModel::BuildFromFile(modelPath);
        if (!model)
        {
            error = "Failed to load model";
            return;
        }

        tflite::ops::builtin::BuiltinOpResolver resolver;
        tflite::InterpreterBuilder(*model, resolver)(&interpreter);
        if (!interpreter)
        {
            error = "Failed to create interpreter builder";
            return;
        }
        interpreter->SetAllowFp16PrecisionForFp32(true);
        interpreter->SetNumThreads(4);

        if (interpreter->AllocateTensors() != TfLiteStatus::kTfLiteOk)
        {
            error = "Failed to allocate tensors";
            return;
        }

        for (unsigned int i = 0; i < interpreter->outputs().size(); i++)
        {
            auto t = interpreter->tensor(interpreter->outputs()[i]);
            unsigned int size = 1;
            for (int j = 0; j < t->dims->size; j++)
            {
                size *= t->dims->data[j];
            }

            // TODO: check type
            outputTensors.push_back(TensorOutput{interpreter->outputs()[i], i, size, 0, t->name});
        }

        valid = true;
    }

    ~TFLiteModel() {}

    template <typename T>
    void setInput(unsigned int tensorIndex, cv::Mat &img, size_t size)
    {
        // TODO: check out which way is better
        memcpy(interpreter->typed_input_tensor<T>(tensorIndex), img.data, size * sizeof(T));

        // auto tensor = interpreter->typed_input_tensor<T>(tensorIndex);
        // unsigned int index = 0;
        // for (unsigned int x = 0; x < img.cols; x++)
        // {
        //     for (unsigned int y = 0; y < img.rows; y++)
        //     {
        //         tensor[index++] = img.at<T>(x, y);
        //     }
        // }
    }

    bool inference()
    {
        if (!valid)
            return false;

        bool ret;
        try
        {
            ret = interpreter->Invoke() == TfLiteStatus::kTfLiteOk;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        return ret;
    }

    template <typename T>
    void retrieveOutput(unsigned int tensorIndex, unsigned int size, void *output)
    {
        /* Retreive tensor from all tensors */
        // TfLiteTensor *tensor = interpreter->tensor(tensorIndex);
        for (unsigned int i = 0; i < size; i++)
        {
            *(((T *)output) + i) = interpreter->typed_output_tensor<T>(tensorIndex)[i];
        }
    }

private:
    std::unique_ptr<tflite::FlatBufferModel> model;
};
#endif