#include <opencv2/core/core.hpp>

#ifdef _WIN32
#define _EXPORT_ __declspec(dllexport)
#else
#define _EXPORT_
#endif

_EXPORT_ void flutter3dVisionHandler(cv::Mat &, float *);