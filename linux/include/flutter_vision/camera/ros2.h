#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include "fv_camera.h"

class RosCamera : public FvCamera, rclcpp::Node
{
public:
  RosCamera(const char *topic) : FvCamera(topic), Node("fv_image_viewer")
  {
    auto callback = [this](sensor_msgs::msg::Image::SharedPtr img) -> void
    {
      if (pauseStream)
        return;

      cv_bridge::toCvShare(img, img->encoding)->image.copyTo(rgbTexture->cvImage);
      rgbTexture->pipeline->run(rgbTexture->cvImage, *flRegistrar, *FL_TEXTURE(rgbTexture), rgbTexture->video_width, rgbTexture->video_height, rgbTexture->buffer, models, flChannel);
    };
    subscriber = this->create_subscription<sensor_msgs::msg::Image>(serial, 10, callback);
  }

  int camInit()
  {
    std::cout << "camInit" << std::endl;
    node_ptr = this->shared_from_this();
    return 0;
  }

  int openDevice() { return 0; }

  int closeDevice() { return 0; }

  int isConnected() { return 0; }

  int configVideoStream(int streamIndex, bool *enable) { return 0; }

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

    // rclcpp::spin(node_ptr);
    // while (videoStart)
    // {
    //   rclcpp::spin_some(node_ptr);
    // }
    return 0;
  }
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscriber;
  rclcpp::Node::SharedPtr node_ptr;
};