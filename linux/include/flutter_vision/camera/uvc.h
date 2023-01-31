#ifndef _DEF_UVC_CAM_
#define _DEF_UVC_CAM_
#include <iostream>

#include "fv_camera.h"

#include <opencv2/core/core.hpp>

class UvcCam : public FvCamera
{
public:
  UvcCam(const char *s) : FvCamera(s){};

  int camInit()
  {
    uvcIndex = stoi(serial);
    if (uvcIndex >= 0)
    {
      cap = new cv::VideoCapture();
    }

    return 0;
  }

  int openDevice()
  {
    if (!cap)
      return -1;
    bool ret = cap->open(uvcIndex);
    if (!ret)
      return -2;
    return cap->isOpened() ? 0 : -3;
  }

  int closeDevice()
  {
    cap->release();
    return 0;
  }

  int isConnected() { return 0; }

  int configVideoStream(int streamIndex, bool *enable)
  {
    if (!cap)
      return -1;

    if (!cap->isOpened())
      return -2;

    videoStart = *enable;

    return 0;
  }

  int readVideoFeed()
  {
    videoStart = true;
    std::thread t(&UvcCam::_readVideoFeed, this);
    t.detach();
    return 0;
  }

  int configure(int prop, std::vector<float> &value)
  {
    if (!cap)
      return -1;

    cap->set(prop, value[0]);

    return 0;
  }

  int getConfiguration(int prop) { return 0; }

  void getIntrinsic(int index, double &fx, double &fy, double &cx, double &cy) {}

  bool enableImageRegistration(bool enable) { return true; }

  void getAvailableVideoModes(int index, std::vector<std::string> &rModes) {}

  void getCurrentVideoMode(int index, std::string &mode) {}

  bool setVideoMode(int index, int mode) { return true; }

  // TODO: Use USB library to do this if needed
  bool getSerialNumber(std::string &sn) { return true; }

private:
  int uvcIndex = -1;
  cv::VideoCapture *cap;
  int _readVideoFeed()
  {
    bool newFrame = false;

    if (!(videoStart))
      return -1;

    while (videoStart)
    {
      newFrame = cap->read(rgbTexture->cvImage);

      if (newFrame)
      {
        rgbTexture->pipeline->run(rgbTexture->cvImage, *flRegistrar, *FL_TEXTURE(rgbTexture), rgbTexture->video_width, rgbTexture->video_height, rgbTexture->buffer, models, flChannel);
        fl_method_channel_invoke_method(flChannel, "onUvcFrame", nullptr, nullptr, nullptr, NULL);
      }
    }

    return 0;
  }
};
#endif