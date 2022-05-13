#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <OpenNI.h>
#include <memory>

#include "opengl/opengl.h"

using namespace openni;

enum VideoIndex
{
  RGB = 0b1,
  Depth = 0b10,
  IR = 0b100,
  OPENGL = 0b1000,
  Camera2D = 0b10000,
};

class OpenNi2Wrapper : public OpenNI::DeviceConnectedListener,
                       public OpenNI::DeviceDisconnectedListener,
                       public OpenNI::DeviceStateChangedListener
{
public:
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

  OpenGLFL *glfl;
  Array<DeviceInfo> deviceList;
  Device *device;
  VideoStream vsDepth;
  VideoStream vsColor;
  VideoStream vsIR;
  bool videoStart = false;
  bool enablePointCloud = false;

  ~OpenNi2Wrapper() {}
  OpenNi2Wrapper()
  {
    device = new Device();
  }

  void test()
  {
  }

  void registerFlContext(flutter::MethodChannel<flutter::EncodableValue> *c)
  {
    flChannel = c;
  }

  void registerFlContext(flutter::TextureRegistrar *re, RgbTexture *r, DepthTexture *d, IrTexture *i, OpenGLFL *g, std::vector<TFLiteModel *> *m)
  {
    rgbTexture = r;
    depthTexture = d;
    irTexture = i;
    glfl = g;
    registrar = re;
    models = m;
  }

  int init()
  {
    return OpenNI::initialize();
  }

  void enumerateDevices()
  {
    OpenNI::enumerateDevices(&deviceList);
  }

  int openDevice(const char *uri, int videoMode)
  {
    if (this->device->isValid())
    {
      return 0;
    }

    Status s = this->device->open(uri);
    if (s == STATUS_OK)
    {
      bool isValid = this->device->isValid();

      bool cameraIsValid = 0;
      if (isValid)
      {
        // Get camera SN
        // int size = 32;
        // char sn[32];
        // this->device->getProperty(openni::DEVICE_PROPERTY_SERIAL_NUMBER, sn, &size);

        createVideoStream(videoMode); // create all video stream by default
      }

      return isValid ? (cameraIsValid ? 0 : 1) : -1;
    }
    else
      return -2;
  }

  void closeDevice()
  {
    if (device == nullptr)
    {
      return;
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
  }

  bool isConnected()
  {
    if (device == nullptr)
    {
      return false;
    }
    return this->device->isValid();
  }

  int getEnabledVideoModes()
  {
    if (device == nullptr)
    {
      return 0;
    }

    int enabledVideoMode = 0;
    if (niRgbAvailable)
      enabledVideoMode += VideoIndex::RGB;
    if (niDepthAvailable)
      enabledVideoMode += VideoIndex::Depth;
    if (niIrAvailable)
      enabledVideoMode += VideoIndex::IR;
    return enabledVideoMode;
  }

  void createVideoStream(int videoMode)
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

  void configVideoStream(int &index, bool *enable)
  {
    if (device == nullptr)
    {
      return;
    }

    int isValid = 0;

    if (((index & VideoIndex::RGB) > 0) && niRgbAvailable)
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

    if (((index & VideoIndex::Depth) > 0) && niDepthAvailable)
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

    if (((index & VideoIndex::IR) > 0) && niIrAvailable)
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
  }

  void readVideoFeed()
  {
    if (device == nullptr)
    {
      return;
    }

    videoStart = true;
    rgbTexture->setPixelBuffer();
    depthTexture->setPixelBuffer();
    irTexture->setPixelBuffer();
    std::thread t(&OpenNi2Wrapper::_readVideoFeed, this);
    t.detach();
  }

private:
  flutter::MethodChannel<flutter::EncodableValue> *flChannel;
  std::vector<TFLiteModel *> *models;

  bool niRgbAvailable = false;
  bool niDepthAvailable = false;
  bool niIrAvailable = false;
  bool enableRgb = false;
  bool enableDepth = false;
  bool enableIr = false;

  RgbTexture *rgbTexture;
  DepthTexture *depthTexture;
  IrTexture *irTexture;
  flutter::TextureRegistrar *registrar;

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

    // printf("VideoStart:%d, %d, %d\n", niRgbAvailable, niDepthAvailable, niIrAvailable);

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
          rgbTexture->pipeline->run(rgbTexture->cvImage, registrar, rgbTexture->textureId, rgbTexture->videoWidth, rgbTexture->videoHeight, rgbTexture->buffer, models, flChannel);
          rgbTexture->setPixelBuffer();
          rgbNewFrame = true;
        }
      }

      if (niDepthAvailable && enableDepth && vsDepth.isValid())
      {
        if (vsDepth.readFrame(&depthFrame) == STATUS_OK)
        {
          depthTexture->cvImage = cv::Mat(depthFrame.getHeight(), depthFrame.getWidth(), CV_16UC1, (void *)depthFrame.getData());
          depthTexture->pipeline->run(depthTexture->cvImage, registrar, depthTexture->textureId, depthTexture->videoWidth, depthTexture->videoHeight, depthTexture->buffer, models, flChannel);
          depthTexture->setPixelBuffer();
          depthNewFrame = true;
        }
      }

      if (niIrAvailable && enableIr && vsDepth.isValid())
      {
        if (vsIR.readFrame(&irFrame) == STATUS_OK)
        {
          irTexture->cvImage = cv::Mat(irFrame.getHeight(), irFrame.getWidth(), CV_16UC1, (void *)irFrame.getData());
          irTexture->pipeline->run(irTexture->cvImage, registrar, irTexture->textureId, irTexture->videoWidth, irTexture->videoHeight, irTexture->buffer, models, flChannel);
          irTexture->setPixelBuffer();
          irNewFrame = true;
        }
      }

      if (enablePointCloud && niRgbAvailable && depthNewFrame && rgbNewFrame)
      {
        niComputeCloud(vsDepth, (const openni::DepthPixel *)depthFrame.getData(), (const openni::RGB888Pixel *)rgbFrame.getData(), glfl->modelPointCloud->vertices, glfl->modelPointCloud->colors, glfl->modelPointCloud->colorsMap, &glfl->modelPointCloud->vertexPoints);
      }

      flChannel->InvokeMethod("onNiFrame", nullptr, nullptr);
    }
  }
};