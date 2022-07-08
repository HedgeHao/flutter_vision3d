#include <librealsense2/rs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <thread>

#include "fv_camera.h"

enum RsVideoIndex
{
  RS_RGB = 0b1,
  RS_Depth = 0b01,
  RS_IR = 0b001,
};

enum RsConfiguration
{
  THRESHOLD_FILTER = 0,
};

enum RsFilterType
{
  THRESHOLD = 0
};

class RsFilter
{
public:
  RsFilterType type;
  rs2::filter *filter;

  RsFilter(RsFilterType t, rs2::filter *f) : type(t), filter(f) {}
};

class RealsenseCam : public FvCamera
{
public:
  rs2::context ctx;
  rs2::pipeline *pipeline;

  RealsenseCam(const char *s) : FvCamera(s) {}

  void camInit()
  {
    glfl->modelRsPointCloud->rgbFrame = &rgbFrame;
  }

  int openDevice()
  {
    pipeline = new rs2::pipeline(ctx);
    cfg = rs2::config();
    cfg.enable_device(serial);

    return 0;
  }

  int closeDevice()
  {
    try
    {
      videoStart = false;
      pipeline->stop();
    }
    catch (rs2::error &e)
    {
      printf("[Realsense Error]: %s\n", e.what());
      return -1;
    }

    return 0;
  }

  int isConnected()
  {
    return 0;
  }

  int configVideoStream(int streamIndex, bool *enable)
  {
    if ((streamIndex & RsVideoIndex::RS_RGB) > 0)
    {
      // enable ? cfg.enable_stream(RS2_STREAM_INFRARED) : cfg.disable_stream(RS2_STREAM_INFRARED);
      isRgbEnable = enable;
    }

    if ((streamIndex & RsVideoIndex::RS_Depth) > 0)
    {
      // enable ? cfg.enable_stream(RS2_STREAM_DEPTH) : cfg.disable_stream(RS2_STREAM_DEPTH);
      isDepthEnable = enable;
    }

    if ((streamIndex & RsVideoIndex::RS_IR) > 0)
    {
      // enable ? cfg.enable_stream(RS2_STREAM_INFRARED) : cfg.disable_stream(RS2_STREAM_INFRARED);
      isIrEnable = enable;
    }

    if (*enable)
    {
      pipeline->start();
    }
    else
    {
      videoStart = false;
      pipeline->stop();
    }

    return -1;
  }

  void readVideoFeed()
  {
    videoStart = true;
    std::thread t(&RealsenseCam::_readVideoFeed, this);
    t.detach();
  }

  void configure(int prop, std::vector<float> &value)
  {
    if (prop == RsConfiguration::THRESHOLD_FILTER)
    {
      for (RsFilter f : filters)
      {
        if (f.type == RsFilterType::THRESHOLD)
        {
          f.filter->as<rs2::threshold_filter>().set_option(rs2_option::RS2_OPTION_MIN_DISTANCE, value[0]);
          f.filter->as<rs2::threshold_filter>().set_option(rs2_option::RS2_OPTION_MAX_DISTANCE, value[1]);
          return;
        }
      }

      RsFilter f(RsFilterType::THRESHOLD, new rs2::threshold_filter(value[0], value[1]));
      filters.push_back(f);
    }
  }

  int getConfiguration(int prop) { return 0; }

private:
  rs2::config cfg;
  unsigned int timeout = 1500;
  bool isRgbEnable = false, isDepthEnable = false, isIrEnable = false;
  rs2::pointcloud rsPointcloud;
  rs2::frame rgbFrame;
  std::vector<RsFilter> filters{};

  void _readVideoFeed()
  {
    while (videoStart)
    {
      try
      {
        rs2::frameset frames = pipeline->wait_for_frames(timeout);
        if (filters.size() > 0)
        {
          for (RsFilter f : filters)
          {

            frames = f.filter->as<rs2::threshold_filter>().process(frames);
          }
        }

        rgbFrame = frames.get_color_frame();
        auto depthFrame = frames.get_depth_frame();
        auto irFrame = frames.get_infrared_frame();

        if (isRgbEnable && rgbFrame)
        {
          rgbTexture->cvImage = frame_to_mat(rgbFrame);
          rgbTexture->pipeline->run(rgbTexture->cvImage, *flRegistrar, *FL_TEXTURE(rgbTexture), rgbTexture->video_width, rgbTexture->video_height, rgbTexture->buffer, models, flChannel);
        }

        if (isDepthEnable && depthFrame)
        {
          depthTexture->cvImage = frame_to_mat(depthFrame);
          depthTexture->pipeline->run(depthTexture->cvImage, *flRegistrar, *FL_TEXTURE(depthTexture), depthTexture->video_width, depthTexture->video_height, depthTexture->buffer, models, flChannel);
        }

        if (isIrEnable && irFrame)
        {
          irTexture->cvImage = frame_to_mat(irFrame);
          irTexture->pipeline->run(irTexture->cvImage, *flRegistrar, *FL_TEXTURE(irTexture), irTexture->video_width, irTexture->video_height, irTexture->buffer, models, flChannel);
        }

        if (enablePointCloud)
        {
          glfl->modelRsPointCloud->points = rsPointcloud.calculate(depthFrame);
          rsPointcloud.map_to(rgbFrame);
        }
      }
      catch (const rs2::error &e)
      {
        std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
      }
      catch (const std::exception &e)
      {
        std::cerr << e.what() << std::endl;
        videoStart = false;
        break;
      }
    }
  }

  static cv::Mat frame_to_mat(const rs2::frame &f)
  {
    using namespace cv;
    using namespace rs2;

    auto vf = f.as<video_frame>();
    const int w = vf.get_width();
    const int h = vf.get_height();

    if (f.get_profile().format() == RS2_FORMAT_BGR8)
    {
      return Mat(Size(w, h), CV_8UC3, (void *)f.get_data(), Mat::AUTO_STEP);
    }
    else if (f.get_profile().format() == RS2_FORMAT_RGB8)
    {
      auto r_rgb = Mat(Size(w, h), CV_8UC3, (void *)f.get_data(), Mat::AUTO_STEP);
      Mat r_bgr;
      cvtColor(r_rgb, r_bgr, COLOR_RGB2BGR);
      return r_bgr;
    }
    else if (f.get_profile().format() == RS2_FORMAT_Z16)
    {
      return Mat(Size(w, h), CV_16UC1, (void *)f.get_data(), Mat::AUTO_STEP);
    }
    else if (f.get_profile().format() == RS2_FORMAT_Y8)
    {
      return Mat(Size(w, h), CV_8UC1, (void *)f.get_data(), Mat::AUTO_STEP);
    }
    else if (f.get_profile().format() == RS2_FORMAT_DISPARITY32)
    {
      return Mat(Size(w, h), CV_32FC1, (void *)f.get_data(), Mat::AUTO_STEP);
    }

    throw std::runtime_error("Frame format is not supported yet!");
  }
};

namespace RealsenseHelper
{
  static std::vector<std::string> enumerateDevices()
  {
    rs2::context ctx;
    std::vector<std::string> serials{};
    for (auto dev : ctx.query_devices())
      serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));

    return serials;
  }
};