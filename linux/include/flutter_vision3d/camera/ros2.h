#include "fv_camera.h"

#ifdef DISABLE_ROS
class RosCamera : public FvCamera
{
public:
  static int openniInit() { return NOT_SUPPORT; }

  RosCamera(const char *s) : FvCamera(s) {}

  int camInit() { return NOT_SUPPORT; }
  int openDevice() { return NOT_SUPPORT; }
  int closeDevice() { return NOT_SUPPORT; }
  int isConnected() { return NOT_SUPPORT; }
  int configVideoStream(int streamIndex, bool *enable) { return NOT_SUPPORT; }
  int readVideoFeed() { return NOT_SUPPORT; }
  int configure(int prop, std::vector<float> &value) { return NOT_SUPPORT; }
  int getConfiguration(int prop) { return NOT_SUPPORT; }
  void getIntrinsic(int index, double &fx, double &fy, double &cx, double &cy){};
  bool enableImageRegistration(bool enable) { return NOT_SUPPORT; };
  void getAvailableVideoModes(int index, std::vector<std::string> &){};
  void getCurrentVideoMode(int index, std::string &mode){};
  bool setVideoMode(int index, int mode) { return NOT_SUPPORT; };
  bool getSerialNumber(std::string &sn) { return NOT_SUPPORT; };
  void loadPresetParameters(std::string &path) {}

private:
  int _readVideoFeed() { return NOT_SUPPORT; }
};
#else
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
class RosCamera : public FvCamera, public rclcpp::Node
{
public:
  RosCamera(const char *topic) : FvCamera(topic), Node("fv_image_viewer")
  {
    auto callback = [this](sensor_msgs::msg::Image::SharedPtr img) -> void
    {
      if (pauseStream)
        return;

      cv_bridge::toCvShare(img, "rgba8")->image.copyTo(rgbTexture->cvImage);
      rgbTexture->pipeline->run(rgbTexture, *flRegistrar, models, flChannel);
    };
    subscriber = this->create_subscription<sensor_msgs::msg::Image>(serial, 10, callback);
  }

  int camInit() { return 0; }

  int openDevice() { return 0; }

  int closeDevice() { return 0; }

  int isConnected() { return 0; }

  int configVideoStream(int streamIndex, bool *enable)
  {
    videoStart = *enable;
    return 0;
  }

  int readVideoFeed()
  {
    videoStart = true;
    std::thread t(&RosCamera::_readVideoFeed, this);
    t.detach();
    return 0;
  }

  int configure(int prop, std::vector<float> &value) { return 0; }

  int getConfiguration(int prop) { return 0; }

  void getIntrinsic(int index, double &fx, double &fy, double &cx, double &cy) {}

  bool enableImageRegistration(bool enable) { return true; }

  void getAvailableVideoModes(int index, std::vector<std::string> &rModes) {}

  void getCurrentVideoMode(int index, std::string &mode) {}

  bool setVideoMode(int index, int mode) { return true; }

  bool getSerialNumber(std::string &sn) { return true; }

  void loadPresetParameters(std::string &path) {}

private:
  int _readVideoFeed()
  {
    while (videoStart)
    {
      rclcpp::spin_some(shared_from_this());
    }
    return 0;
  }
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscriber;
};
#endif