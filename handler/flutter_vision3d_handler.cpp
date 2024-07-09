#include "flutter_vision3d_handler.h"

#include <opencv2/imgproc/imgproc.hpp>

void flutterVision3dHandler(cv::Mat &img, float *result)
{
  // OpenCV
  cv::putText(img, "Handled By Flutter Vision 3D", cv::Point(0, img.rows), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 0), 2, cv::LINE_4);

  // Tensorflow Inference
  result[0] = 11;
  result[1] = 22;
  result[2] = 33;
}