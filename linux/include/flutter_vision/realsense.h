#include <librealsense2/rs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <thread>

#include "opengl.h"
#include "fv_texture.h"
#include "pipeline/pipeline.h"

enum RsVideoIndex
{
  RS_RGB = 0b1,
  RS_Depth = 0b01,
  RS_IR = 0b001,
};

class RealsenseCam
{
public:
  rs2::context ctx;
  rs2::pipeline *pipeline;
  std::string serial;

  FvTexture *rgbTexture;
  FvTexture *depthTexture;
  FvTexture *irTexture;

  bool enablePointCloud = false;

  RealsenseCam(const char *s)
  {
    serial = std::string(s);
  }

  void fv_init(FlTextureRegistrar *r, std::vector<TFLiteModel *> *m, FlMethodChannel *f, OpenGLFL *g)
  {
    flRegistrar = r;
    // TODO: check if this is duplicate from texture
    models = m;
    flChannel = f;
    glfl = g;

    // Create texture
    rgbTexture = FV_TEXTURE(g_object_new(fv_texture_get_type(), nullptr));
    FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(rgbTexture)->copy_pixels = fv_texture_copy_pixels;
    fl_texture_registrar_register_texture(flRegistrar, FL_TEXTURE(rgbTexture));
    rgbTexture->texture_id = reinterpret_cast<int64_t>(FL_TEXTURE(rgbTexture));
    fl_texture_registrar_mark_texture_frame_available(flRegistrar, FL_TEXTURE(rgbTexture));
    depthTexture = FV_TEXTURE(g_object_new(fv_texture_get_type(), nullptr));
    FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(depthTexture)->copy_pixels = fv_texture_copy_pixels;
    fl_texture_registrar_register_texture(flRegistrar, FL_TEXTURE(depthTexture));
    depthTexture->texture_id = reinterpret_cast<int64_t>(FL_TEXTURE(depthTexture));
    fl_texture_registrar_mark_texture_frame_available(flRegistrar, FL_TEXTURE(depthTexture));
    irTexture = FV_TEXTURE(g_object_new(fv_texture_get_type(), nullptr));
    FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(irTexture)->copy_pixels = fv_texture_copy_pixels;
    fl_texture_registrar_register_texture(flRegistrar, FL_TEXTURE(irTexture));
    irTexture->texture_id = reinterpret_cast<int64_t>(FL_TEXTURE(irTexture));
    fl_texture_registrar_mark_texture_frame_available(flRegistrar, FL_TEXTURE(irTexture));

    // Create Pipeline
    FV_TEXTURE(rgbTexture)->pipeline = new Pipeline(&FV_TEXTURE(rgbTexture)->cvImage);
    FV_TEXTURE(rgbTexture)->models = models;
    FV_TEXTURE(depthTexture)->pipeline = new Pipeline(&FV_TEXTURE(depthTexture)->cvImage);
    FV_TEXTURE(depthTexture)->models = models;
    FV_TEXTURE(irTexture)->pipeline = new Pipeline(&FV_TEXTURE(irTexture)->cvImage);
    FV_TEXTURE(irTexture)->models = models;
  }

  int64_t getTextureId(int index)
  {
    if (index == RsVideoIndex::RS_RGB)
    {
      return rgbTexture->texture_id;
    }
    else if (index == RsVideoIndex::RS_Depth)
    {
      return depthTexture->texture_id;
    }
    else if (index == RsVideoIndex::RS_IR)
    {
      return irTexture->texture_id;
    }

    return -1;
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
    pipeline->stop();
    return -1;
  }

  int isConnected()
  {
    return 0;
  }

  int configVideoStream(int streamIndex, bool enable)
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

    if (enable)
    {
      pipeline->start();
    }
    else
    {
      pipeline->stop();
      videoStart = false;
    }

    return -1;
  }

  void readVideoFeed()
  {
    videoStart = true;
    std::thread t(&RealsenseCam::_readVideoFeed, this);
    t.detach();
  }

private:
  rs2::config cfg;
  bool videoStart = false;
  unsigned int timeout = 1500;

  OpenGLFL *glfl;
  FlTextureRegistrar *flRegistrar;
  std::vector<TFLiteModel *> *models;
  FlMethodChannel *flChannel;

  bool isRgbEnable = false, isDepthEnable = false, isIrEnable = false;
  rs2::pointcloud rsPointcloud;

  void _readVideoFeed()
  {
    while (videoStart)
    {
      try
      {
        rs2::frameset frames = pipeline->wait_for_frames(timeout);

        auto rgbFrame = frames.get_color_frame();
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
          rsPointcloud.map_to(rgbFrame);
          glfl->modelRsPointCloud->points = rsPointcloud.calculate(depthFrame);
          glfl->modelRsPointCloud->updateTexture(rgbFrame);
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