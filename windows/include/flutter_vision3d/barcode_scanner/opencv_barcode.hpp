#ifndef _H_OPENCV_BARCODE_
#define _H_OPENCV_BARCODE_

#include <memory>

#include <opencv2/barcode.hpp>

using namespace std;
using namespace cv;
using namespace cv::barcode;

class CVBarCodeDetector
{
public:
    vector<cv::String> decode_info;
    vector<BarcodeType> decoded_type;
    vector<cv::Point> corners;

    void init(string prototxt, string model)
    {
        barcode = make_unique<BarcodeDetector>(prototxt, model);
        inited = true;
    }

    int detect(Mat &input)
    {
        if(!inited) return false;

        bool ret = barcode->detect(input, corners);

        if(!ret) return -1;

        return corners.size();
    }

    bool decode(Mat &input){
        if(!inited) return false;

        return barcode->decode(input, corners, decode_info, decoded_type);
    }

private:
    unique_ptr<BarcodeDetector> barcode;
    bool inited;
};

#endif