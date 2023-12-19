#include "fv_camera.h"

#ifdef DISABLE_REALSENSE
class RealsenseCam : public FvCamera
{
public:
  RealsenseCam(const char *s) : FvCamera(s) {}

  int camInit() { return NOT_SUPPORT; }
  int openDevice() { return NOT_SUPPORT; }
  int closeDevice() { return NOT_SUPPORT; }
  int isConnected() { return NOT_SUPPORT; }
  int configVideoStream(int streamIndex, bool *enable) { return NOT_SUPPORT; }
  int readVideoFeed() { return NOT_SUPPORT; }
  int configure(int prop, std::vector<float> &value) { return NOT_SUPPORT; }
  int getConfiguration(int prop) { return NOT_SUPPORT; }
  bool enableImageRegistration(bool enable) { return true; }
  void getAvailableVideoModes(int index, std::vector<std::string> &rModes) {}
  void getCurrentVideoMode(int index, std::string &mode) {}
  bool setVideoMode(int index, int mode) { return true; }
  bool getSerialNumber(std::string &sn) {}

private:
  int _readVideoFeed() { return NOT_SUPPORT; }
};
#else

#include <librealsense2/rs.hpp>
#include <librealsense2/rs_advanced_mode.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <thread>

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

  RealsenseCam(const char *s) : FvCamera(s) {}

  int camInit()
  {
    glfl->modelRsPointCloud->rgbFrame = &rgbFrame;
    return 0;
  }

  int openDevice()
  {
    pipeline = new rs2::pipeline(ctx);
    cfg = rs2::config();
    cfg.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_BGR8, 30);
    cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_INFRARED);
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
        return -2;
      }
    }

    return -1;
  }

  int readVideoFeed()
  {
    videoStart = true;
    std::thread t(&RealsenseCam::_readVideoFeed, this);
    t.detach();
    return 0;
  }

  int configure(int prop, std::vector<float> &value)
  {
    if (prop == RsConfiguration::THRESHOLD_FILTER)
    {
      for (RsFilter f : filters)
      {
        if (f.type == RsFilterType::THRESHOLD)
        {
          f.filter->as<rs2::threshold_filter>().set_option(rs2_option::RS2_OPTION_MIN_DISTANCE, value[0]);
          f.filter->as<rs2::threshold_filter>().set_option(rs2_option::RS2_OPTION_MAX_DISTANCE, value[1]);
          return 0;
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
          return 0;
        }
      }

      if (value[0] == 1)
      {
        filters.push_back(RsFilter(RsFilterType::FRAME_SYNC_COLOR, new rs2::align(RS2_STREAM_COLOR)));
        return 0;
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
          return 0;
        }
      }

      if (value[0] == 1)
      {
        filters.push_back(RsFilter(RsFilterType::FRAME_SYNC_DEPTH, new rs2::align(RS2_STREAM_DEPTH)));
        return 0;
      }
    }

    return 0;
  }

  int getConfiguration(int prop) { return 0; }

  bool enableImageRegistration(bool enable)
  {
    std::vector<float> param = {enable ? 1.0f : 0.0f};
    configure(RsConfiguration::FRAME_SYNC_COLOR_FILTER, param);
    return true;
  }

  void getAvailableVideoModes(int index, std::vector<std::string> &rModes) {}
  void getCurrentVideoMode(int index, std::string &mode)
  {
    rs2::pipeline_profile profile = pipeline->get_active_profile();
    rs2::stream_profile streamProfile;
    if (index == VideoIndex::RGB)
    {
      streamProfile = profile.get_stream(RS2_STREAM_COLOR);
    }
    else if (index == VideoIndex::Depth)
    {
      streamProfile = profile.get_stream(RS2_STREAM_DEPTH);
    }
    else if (index == VideoIndex::IR)
    {
      streamProfile = profile.get_stream(RS2_STREAM_INFRARED);
    }

    std::stringstream ss;
    ss << streamProfile.as<rs2::video_stream_profile>().width();
    ss << ",";
    ss << streamProfile.as<rs2::video_stream_profile>().height();
    ss << ",";
    ss << streamProfile.fps();
    ss << ",";
    ss << streamProfile.format();

    mode.clear();
    ss >> mode;
  }
  bool setVideoMode(int index, int mode) { return true; }

  bool getSerialNumber(std::string &sn)
  {
    sn = serial;
    return true;
  }

  void loadPresetParameters(std::string &path)
  {
    profile.get_device().as<rs400::advanced_mode>().load_json(path);
  }

private:
  rs2::config cfg;
  rs2::pipeline_profile profile;
  unsigned int timeout = 1500;
  bool isRgbEnable = false, isDepthEnable = false, isIrEnable = false;
  rs2::pointcloud rsPointcloud;
  rs2::frame rgbFrame;
  std::vector<RsFilter> filters{};

  int _readVideoFeed()
  {
    int64_t now;

    if (videoStart)
    {
      rs2::frameset frames = pipeline->wait_for_frames(timeout);
      depthWidth = frames.get_depth_frame().get_width();
      depthHeight = frames.get_depth_frame().get_height();
    }

    while (videoStart)
    {
      if (pauseStream)
        continue;

      videoFeedProcessing = true;

      getCurrentTime(&now);
      if (now - tsRs < 32)
      {
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
          rgbTexture->cvImage = frame_to_mat(rgbFrame);
          rgbTexture->pipeline->run(rgbTexture, *flRegistrar, models, flChannel);
        }

        if (isDepthEnable && depthFrame)
        {
          depthTexture->cvImage = frame_to_mat(depthFrame);
          depthTexture->pipeline->run(depthTexture, *flRegistrar, models, flChannel);

          depthData = (uint16_t *)(depthFrame.get_data());
        }

        if (isIrEnable && irFrame)
        {
          irTexture->cvImage = frame_to_mat(irFrame);
          irTexture->pipeline->run(irTexture, *flRegistrar, models, flChannel);
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
    return 0;
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

#endif