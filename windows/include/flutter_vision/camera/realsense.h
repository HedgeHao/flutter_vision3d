#include <librealsense2/rs.hpp>
#include <librealsense2/rs_advanced_mode.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <thread>
#include <memory>

#include "fv_camera.h"

int64_t tsRs = 0;

enum RsVideoIndex
{
  RS_RGB = 0b1,
  RS_Depth = 0b01,
  RS_IR = 0b001,
};

enum RsConfiguration
{
  THRESHOLD_FILTER = 0,
  FRAME_SYNC_COLOR_FILTER = 1,
  FRAME_SYNC_DEPTH_FILTER = 2,
};

enum RsFilterType
{
  THRESHOLD = 0,
  FRAME_SYNC_COLOR = 1,
  FRAME_SYNC_DEPTH = 2,
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
  rs2::pipeline_profile profile;

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

  void getIntrinsic(int index, double &fx, double &fy, double &cx, double &cy)
  {
    if (pipeline == nullptr)
      return;

    rs2_intrinsics intrinsics;
    auto profile = pipeline->get_active_profile();
    if (index == VideoIndex::RGB)
    {
      intrinsics = profile.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>().get_intrinsics();
    }
    else if (index == VideoIndex::Depth)
    {
      intrinsics = profile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>().get_intrinsics();
    }

    fx = intrinsics.fx;
    fy = intrinsics.fy;
    cx = intrinsics.ppx;
    cy = intrinsics.ppy;
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
      cfg.enable_all_streams();
      try
      {
        profile = pipeline->start(cfg);
      }
      catch (rs2::error &e)
      {
        std::cout << "[Realsense SDK error]" << e.what() << std::endl;
        return -1;
      }
    }
    else
    {
      videoStart = false;
      try
      {
        pipeline->stop();
      }
      catch (rs2::wrong_api_call_sequence_error &e)
      {
        std::cout << "[Realsense SDK Error]" << e.what() << std::endl;
        return -1;
      }
    }

    return 0;
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
    else if (prop == RsConfiguration::FRAME_SYNC_COLOR_FILTER)
    {
      for (int i = 0; i < filters.size(); i++)
      {
        if (filters[i].type == RsFilterType::FRAME_SYNC_COLOR)
        {
          if (value[0] == 0)
          {
            filters.erase(filters.begin() + i);
          }
          return;
        }
      }

      if (value[0] == 1)
      {
        filters.push_back(RsFilter(RsFilterType::FRAME_SYNC_COLOR, new rs2::align(RS2_STREAM_COLOR)));
      }
    }
    else if (prop == RsConfiguration::FRAME_SYNC_DEPTH_FILTER)
    {
      for (int i = 0; i < filters.size(); i++)
      {
        if (filters[i].type == RsFilterType::FRAME_SYNC_DEPTH)
        {
          if (value[0] == 0)
          {
            filters.erase(filters.begin() + i);
          }
          return;
        }
      }

      if (value[0] == 1)
      {
        filters.push_back(RsFilter(RsFilterType::FRAME_SYNC_DEPTH, new rs2::align(RS2_STREAM_DEPTH)));
      }
    }
  }

  int getConfiguration(int prop) { return 0; }

  void loadPresetParameters(std::string &path)
  {
    profile.get_device().as<rs400::advanced_mode>().load_json(path);
  }

  // TODO: Not Implement
  bool enableImageRegistration(bool enable) { return true; }

  void getAvailableVideoModes(int index, std::vector<std::string> &rModes) {}
  void getCurrentVideoMode(int index, std::string &mode) {}
  bool setVideoMode(int index, int mode) { return true; }

  bool getSerialNumber(std::string &sn)
  {
    sn = serial;
    return true;
  }

private:
  rs2::config cfg;

  unsigned int timeout = 1500;
  bool isRgbEnable = false, isDepthEnable = false, isIrEnable = false;
  rs2::pointcloud rsPointcloud;
  rs2::frame rgbFrame;
  std::vector<RsFilter> filters{};

  void _readVideoFeed()
  {
    int64_t now;
    while (videoStart)
    {
      if (pauseStream)
        continue;

      videoFeedProcessing = true;

      getCurrentTime(&now);
      if (now - tsRs < 32){
        videoFeedProcessing = false;
        continue;
      }
      tsRs = now;

      try
      {
        rs2::frameset frames = pipeline->wait_for_frames(timeout);
        if (filters.size() > 0)
        {
          for (RsFilter f : filters)
          {
            frames = f.filter->process(frames);
          }
        }

        rgbFrame = frames.get_color_frame();
        auto depthFrame = frames.get_depth_frame();
        auto irFrame = frames.get_infrared_frame();

        if (isRgbEnable && rgbFrame)
        {
          frame_to_mat(rgbFrame, rgbTexture->cvImage);
          rgbTexture->pipeline->run(rgbTexture->cvImage, flRegistrar, rgbTexture->textureId, rgbTexture->videoWidth, rgbTexture->videoHeight, rgbTexture->buffer, models, flChannel);
          rgbTexture->setPixelBuffer();
        }

        if (isDepthEnable && depthFrame)
        {
          frame_to_mat(depthFrame, depthTexture->cvImage);
          depthTexture->pipeline->run(depthTexture->cvImage, flRegistrar, depthTexture->textureId, depthTexture->videoWidth, depthTexture->videoHeight, depthTexture->buffer, models, flChannel);
          depthTexture->setPixelBuffer();
        }

        if (isIrEnable && irFrame)
        {
          frame_to_mat(irFrame, irTexture->cvImage);
          irTexture->pipeline->run(irTexture->cvImage, flRegistrar, irTexture->textureId, irTexture->videoWidth, irTexture->videoHeight, irTexture->buffer, models, flChannel);
          irTexture->setPixelBuffer();
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

      videoFeedProcessing = false;
    }
  }

  static void frame_to_mat(const rs2::frame &f, cv::Mat &frame)
  {
    using namespace cv;
    using namespace rs2;

    auto vf = f.as<video_frame>();
    const int w = vf.get_width();
    const int h = vf.get_height();

    // std::cout << w << "," << h << ", format=" << f.get_profile().format()<< "," << f.get_data_size() << std::endl;

    if (f.get_profile().format() == RS2_FORMAT_BGR8)
    {
      Mat(Size(w, h), CV_8UC3, (void *)f.get_data(), Mat::AUTO_STEP).copyTo(frame);
    }
    else if (f.get_profile().format() == RS2_FORMAT_RGB8)
    {
      auto r_rgb = Mat(Size(w, h), CV_8UC3, (void *)f.get_data(), Mat::AUTO_STEP);
      Mat r_bgr;
      cvtColor(r_rgb, r_bgr, COLOR_RGB2BGR);
      r_bgr.copyTo(frame);
    }
    else if (f.get_profile().format() == RS2_FORMAT_Z16)
    {
      Mat(Size(w, h), CV_16UC1, (void *)f.get_data(), Mat::AUTO_STEP).copyTo(frame);
    }
    else if (f.get_profile().format() == RS2_FORMAT_Y8)
    {
      Mat(Size(w, h), CV_8UC1, (void *)f.get_data(), Mat::AUTO_STEP).copyTo(frame);
    }
    else if (f.get_profile().format() == RS2_FORMAT_DISPARITY32)
    {
      Mat(Size(w, h), CV_32FC1, (void *)f.get_data(), Mat::AUTO_STEP).copyTo(frame);
    }
    else
    {
      throw std::runtime_error("Frame format is not supported yet!");
    }

    return;
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