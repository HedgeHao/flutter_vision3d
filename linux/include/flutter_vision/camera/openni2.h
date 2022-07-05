#ifndef _DEF_OPENNI_CAM_
#define _DEF_OPENNI_CAM_
#include <openni2/OpenNI.h>

#include "fv_camera.h"

using namespace openni;

class OpenniCam : public FvCamera,
                  public OpenNI::DeviceConnectedListener,
                  public OpenNI::DeviceDisconnectedListener,
                  public OpenNI::DeviceStateChangedListener
{
public:
  static int openniInit()
  {
    return static_cast<int>(OpenNI::initialize());
  }

  static void enumerateDevices(Array<DeviceInfo> *niDevices)
  {
    OpenNI::enumerateDevices(niDevices);
    return;
  }

  Device *device = new Device();

  virtual void onDeviceStateChanged(const DeviceInfo *pInfo, DeviceState state)
  {
    // printf("[LISTENER] Device \"%s\" error state changed to %d\n", pInfo->getUri(), state);
  }

  virtual void onDeviceConnected(const DeviceInfo *pInfo)
  {
    // printf("[LISTENER] Device \"%s\" connected\n", pInfo->getUri());
  }

  virtual void onDeviceDisconnected(const DeviceInfo *pInfo)
  {
    // TODO: When supporting multiple devices. Find the right device to destory
    device = nullptr;
  }

  OpenniCam(const char *s) : FvCamera(s){};

  void camInit() {}

  int openDevice()
  {
    if (this->device->isValid())
    {
      return 0;
    }

    Status s = this->device->open(serial.c_str());
    if (s == STATUS_OK)
    {
      bool isValid = this->device->isValid();

      if (isValid)
      {
        // Get camera SN
        // int size = 32;
        // char sn[32];
        // this->device->getProperty(openni::DEVICE_PROPERTY_SERIAL_NUMBER, sn, &size);

        createVideoStream();
      }

      return isValid ? 0 : -1;
    }

    return -2;
  }

  int closeDevice()
  {
    if (device == nullptr)
    {
      return -1;
    }

    if (vsColor.isValid())
    {
      vsColor.stop();
      vsColor.destroy();
    }
    if (vsDepth.isValid())
    {
      vsDepth.stop();
      vsDepth.destroy();
    }
    if (vsIR.isValid())
    {
      vsIR.stop();
      vsIR.destroy();
    }

    this->device->close();
    return 0;
  }

  int isConnected()
  {
    if (device == nullptr)
    {
      return false;
    }

    return this->device->isValid();
  }

  int configVideoStream(int streamIndex, bool *enable)
  {
    if (device == nullptr)
      return -1;

    int isValid = 0;

    if (((streamIndex & VideoIndex::RGB) > 0) && niRgbAvailable)
    {
      if (*enable)
      {
        vsColor.start();
        if (vsColor.isValid())
          isValid += VideoIndex::RGB;
        enableRgb = true;
      }
      else
      {
        vsColor.stop();
        enableRgb = false;
      }
    }

    if (((streamIndex & VideoIndex::Depth) > 0) && niDepthAvailable)
    {
      if (*enable)
      {
        vsDepth.start();
        if (vsDepth.isValid())
          isValid += VideoIndex::Depth;
        enableDepth = true;
      }
      else
      {
        vsDepth.stop();
        enableDepth = false;
      }
    }

    if (((streamIndex & VideoIndex::IR) > 0) && niIrAvailable)
    {
      if (*enable)
      {
        vsIR.start();
        if (vsIR.isValid())
          isValid += VideoIndex::IR;
        enableIr = true;
      }
      else
      {
        vsIR.stop();
        enableIr = false;
      }
    }

    if (isValid == 0)
    {
      *enable = false;
    }

    if (!enableRgb && !enableIr && !enableDepth)
    {
      videoStart = false;
      *enable = false;
    }

    return 0;
  }

  void readVideoFeed()
  {
    videoStart = true;
    std::thread t(&OpenniCam::_readVideoFeed, this);
    t.detach();
  }

  void configure(int prop, std::vector<float> &value) {}

private:
  VideoStream vsDepth;
  VideoStream vsColor;
  VideoStream vsIR;
  bool niRgbAvailable = false;
  bool niDepthAvailable = false;
  bool niIrAvailable = false;
  bool enableRgb = false;
  bool enableDepth = false;
  bool enableIr = false;

  void createVideoStream(int videoMode = 7) // create all video stream by default
  {
    if (device == nullptr)
    {
      return;
    }

    vsColor.destroy();
    vsDepth.destroy();
    vsIR.destroy();

    if ((videoMode & VideoIndex::RGB) > 0 && vsColor.create(*device, SENSOR_COLOR) == STATUS_OK)
    {
      niRgbAvailable = true;
    }
    if ((videoMode & VideoIndex::Depth) > 0 && vsDepth.create(*device, SENSOR_DEPTH) == STATUS_OK)
    {
      niDepthAvailable = true;
    }
    if ((videoMode & VideoIndex::IR) > 0 && vsIR.create(*device, SENSOR_IR) == STATUS_OK)
    {
      niIrAvailable = true;
    }
  }

  void colorMap(float dis, float *outputR, float *outputG, float *outputB)
  {
    int r = 0, g = 0, b = 0; // 0 ~ 255
    int map[5][4] =
        {
            // {distance, R, G, B}
            {0, 0, 0, 255},
            {500, 0, 255, 255},
            {1000, 255, 255, 0},
            {2000, 255, 0, 0},
            {4000, 102, 0, 0}};
    if (dis >= map[4][0])
    {
      r = map[4][1];
      g = map[4][2];
      b = map[4][3];
    }
    else
    {
      for (int i = 0; i < 4; i++)
      {
        if (map[i][0] <= dis && dis < map[i + 1][0])
        {
          float t = (dis - map[i][0]) / (map[i + 1][0] - map[i][0]);
          r = map[i][1] + (map[i + 1][1] - map[i][1]) * t;
          g = map[i][2] + (map[i + 1][2] - map[i][2]) * t;
          b = map[i][3] + (map[i + 1][3] - map[i][3]) * t;
          break;
        }
      }
    }
    *outputR = r / 255.0;
    *outputG = g / 255.0;
    *outputB = b / 255.0;
  }

  void niComputeCloud(const VideoStream &depthStream, const void *depthFrameData, const void *colorFrameData,
                      float *imgDepth, float *imgColor, float *imgColorMap, unsigned int *vertexCount)
  {
    int frameWidth = depthStream.getVideoMode().getResolutionX();
    int frameHeight = depthStream.getVideoMode().getResolutionY();

    const openni::DepthPixel *pDepth = (const openni::DepthPixel *)depthFrameData;
    const openni::RGB888Pixel *pColor = (const openni::RGB888Pixel *)colorFrameData;
    float fX, fY, fZ;
    int count = 0;
    for (int y = 0; y < frameHeight; y++)
    {
      for (int x = 0; x < frameWidth; x++)
      {
        fX = 0.0;
        fY = 0.0;
        fZ = 0.0;
        if (pDepth[count] != 0)
        {
          CoordinateConverter::convertDepthToWorld(depthStream, x, y, pDepth[count], &fX, &fY, &fZ);
          if (pColor != nullptr)
          {
            imgColor[count * 3 + 0] = pColor[count].r / 255.0;
            imgColor[count * 3 + 1] = pColor[count].g / 255.0;
            imgColor[count * 3 + 2] = pColor[count].b / 255.0;
          }
          colorMap(pDepth[count], &imgColorMap[count * 3], &imgColorMap[count * 3 + 1], &imgColorMap[count * 3 + 2]);
        }
        imgDepth[count * 3 + 0] = fX * 0.001;
        imgDepth[count * 3 + 1] = fY * 0.001;
        imgDepth[count * 3 + 2] = fZ * 0.001;
        count++;
      }
    }

    *vertexCount = count;
  }

  void _readVideoFeed()
  {
    VideoFrameRef rgbFrame;
    VideoFrameRef depthFrame;
    VideoFrameRef irFrame;

    bool rgbNewFrame = false;
    bool depthNewFrame = false;
    bool irNewFrame = false;

    if (!(videoStart))
      return;

    while (videoStart)
    {
      rgbNewFrame = false;
      depthNewFrame = false;
      irNewFrame = false;

      if (niRgbAvailable && enableRgb && vsColor.isValid())
      {
        if (vsColor.readFrame(&rgbFrame) == STATUS_OK)
        {
          rgbTexture->cvImage = cv::Mat(rgbFrame.getHeight(), rgbFrame.getWidth(), CV_8UC3, (void *)rgbFrame.getData());
          rgbTexture->pipeline->run(rgbTexture->cvImage, *flRegistrar, *FL_TEXTURE(rgbTexture), rgbTexture->video_width, rgbTexture->video_height, rgbTexture->buffer, models, flChannel);
          rgbNewFrame = true;
        }
      }

      if (niDepthAvailable && enableDepth && vsDepth.isValid())
      {
        if (vsDepth.readFrame(&depthFrame) == STATUS_OK)
        {
          depthTexture->cvImage = cv::Mat(depthFrame.getHeight(), depthFrame.getWidth(), CV_16UC1, (void *)depthFrame.getData());
          depthTexture->pipeline->run(depthTexture->cvImage, *flRegistrar, *FL_TEXTURE(depthTexture), depthTexture->video_width, depthTexture->video_height, depthTexture->buffer, models, flChannel);
          depthNewFrame = true;
        }
      }

      if (niIrAvailable && enableIr && vsDepth.isValid())
      {
        if (vsIR.readFrame(&irFrame) == STATUS_OK)
        {
          irTexture->cvImage = cv::Mat(irFrame.getHeight(), irFrame.getWidth(), CV_16UC1, (void *)irFrame.getData());
          irTexture->pipeline->run(irTexture->cvImage, *flRegistrar, *FL_TEXTURE(irTexture), irTexture->video_width, irTexture->video_height, irTexture->buffer, models, flChannel);
          irNewFrame = true;
        }
      }

      if (enablePointCloud && niRgbAvailable && depthNewFrame && rgbNewFrame)
      {
        niComputeCloud(vsDepth, (const openni::DepthPixel *)depthFrame.getData(), (const openni::RGB888Pixel *)rgbFrame.getData(), glfl->modelPointCloud->vertices, glfl->modelPointCloud->colors, glfl->modelPointCloud->colorsMap, &glfl->modelPointCloud->vertexPoints);
      }

      fl_method_channel_invoke_method(flChannel, "onNiFrame", nullptr, nullptr, nullptr, NULL);
    }
  }
};
#endif