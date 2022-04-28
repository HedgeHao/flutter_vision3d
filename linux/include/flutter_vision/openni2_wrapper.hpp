#include <flutter_linux/flutter_linux.h>
#include <flutter_linux/fl_value.h>
#include <openni2/OpenNI.h>
#include <gtk/gtk.h>
#include <memory>
#include <opencv2/core/core.hpp>

#include "rgb_texture.hpp"
#include "depth_texture.hpp"
#include "ir_texture.hpp"
#include <thread>
#include "opengl.h"
#include "tflite.h"

using namespace openni;

enum VideoIndex
{
  RGB = 0b1,
  Depth = 0b10,
  IR = 0b100,
  POINTCLOUD = 0b1000,
  Camera2D = 0b10000,
};

class FlValueWrapper
{
public:
  FlValue *value;

  FlValueWrapper(FlValue *v)
  {
    value = v;
  }

  ~FlValueWrapper(){

  };
};

class OpenNi2Wrapper : public OpenNI::DeviceConnectedListener,
                       public OpenNI::DeviceDisconnectedListener,
                       public OpenNI::DeviceStateChangedListener
{
public:
  virtual void onDeviceStateChanged(const DeviceInfo *pInfo, DeviceState state)
  {
    printf("[LISTENER] Device \"%s\" error state changed to %d\n", pInfo->getUri(), state);
  }

  virtual void onDeviceConnected(const DeviceInfo *pInfo)
  {
    printf("[LISTENER] Device \"%s\" connected\n", pInfo->getUri());
  }

  virtual void onDeviceDisconnected(const DeviceInfo *pInfo)
  {
    // TODO: When supporting multiple devices. Find the right device to destory
    device = nullptr;
  }

  OpenGLFL *glfl;
  Device *device;
  VideoStream vsDepth;
  VideoStream vsColor;
  VideoStream vsIR;
  bool videoStart;

  void registerFlContext(FlTextureRegistrar *r, RgbTexture *rgb, DepthTexture *depth, IrTexture *ir, FlMethodChannel *channel, OpenGLFL *g, std::vector<TFLiteModel *> *m, TfPipeline *tp)
  {
    registrar = r;
    rgbTexture = rgb;
    depthTexture = depth;
    irTexture = ir;
    flChannel = channel;
    glfl = g;
    models = m;
    tfPipeline = tp;
  };

  ~OpenNi2Wrapper() {}
  OpenNi2Wrapper()
  {
    device = new Device();
  }

  void init(FlValueWrapper *ret)
  {
    ret->value = fl_value_new_int(OpenNI::initialize());
    OpenNI::addDeviceDisconnectedListener(this);
  }

  void enumerateDevices(FlValue *flDeviceList)
  {
    Array<DeviceInfo> deviceList;
    OpenNI::enumerateDevices(&deviceList);

    for (int i = 0; i < deviceList.getSize(); i++)
    {
      DeviceInfo d = deviceList[i];
      std::unique_ptr<FlValueWrapper> flDeviceMap = std::make_unique<FlValueWrapper>(fl_value_new_map());

      fl_value_set((flDeviceMap.get())->value, fl_value_new_string("name"), fl_value_new_string(d.getName()));
      fl_value_set((flDeviceMap.get())->value, fl_value_new_string("uri"), fl_value_new_string(d.getUri()));
      fl_value_set((flDeviceMap.get())->value, fl_value_new_string("productId"), fl_value_new_int(d.getUsbProductId()));
      fl_value_set((flDeviceMap.get())->value, fl_value_new_string("vendorId"), fl_value_new_int(d.getUsbVendorId()));
      fl_value_set((flDeviceMap.get())->value, fl_value_new_string("vendor"), fl_value_new_string(d.getVendor()));
      fl_value_append_take(flDeviceList, (flDeviceMap.get())->value);
    }
  }

  void openDevice(FlValueWrapper *ret, const char *uri, int videoMode)
  {
    if (this->device->isValid())
    {
      ret->value = fl_value_new_int(0);
      return;
    }

    Status s = this->device->open(uri);
    if (s == STATUS_OK)
    {
      bool isValid = this->device->isValid();
      ret->value = fl_value_new_int(isValid ? 0 : -1);

      if (isValid)
      {
        // Read camera SN
        // int size = 32;
        // char sn[size];
        // this->device->getProperty(openni::DEVICE_PROPERTY_SERIAL_NUMBER, sn, &size);

        createVideoStream(videoMode); // create all video stream
      }
    }
    else
      ret->value = fl_value_new_int(-2);
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

  void isConnected(FlValueWrapper *ret)
  {
    ret->value = fl_value_new_bool(this->device->isValid());
  }

  void getEnabledVideoModes(FlValueWrapper *ret)
  {
    int enableVideoMode = 0;
    if (niRgbAvailable)
      enableVideoMode += VideoIndex::RGB;
    if (niDepthAvailable)
      enableVideoMode += VideoIndex::Depth;
    if (niIrAvailable)
      enableVideoMode += VideoIndex::IR;
    ret->value = fl_value_new_int(enableVideoMode);
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
      vsColor.setMirroringEnabled(false);
      niRgbAvailable = true;
    }
    if ((videoMode & VideoIndex::Depth) > 0 && vsDepth.create(*device, SENSOR_DEPTH) == STATUS_OK)
    {
      vsDepth.setMirroringEnabled(false);
      niDepthAvailable = true;
    }
    if ((videoMode & VideoIndex::IR) > 0 && vsIR.create(*device, SENSOR_IR) == STATUS_OK)
    {
      vsIR.setMirroringEnabled(false);
      niIrAvailable = true;
    }
  }

  void configVideoStream(FlValueWrapper *ret, int &index, bool *enable)
  {
    if (device == nullptr)
    {
      ret->value = fl_value_new_bool(false);
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

    ret->value = fl_value_new_bool(isValid);
  }

  void readVideoFeed()
  {
    if (device == nullptr)
    {
      return;
    }

    videoStart = true;
    std::thread t(&OpenNi2Wrapper::_readVideoFeed, this);
    t.detach();
  }

private:
  FlTextureRegistrar *registrar;
  RgbTexture *rgbTexture;
  IrTexture *irTexture;
  DepthTexture *depthTexture;
  FlMethodChannel *flChannel;
  std::vector<TFLiteModel *> *models;
  TfPipeline *tfPipeline;

  bool niRgbAvailable = false;
  bool niDepthAvailable = false;
  bool niIrAvailable = false;
  bool enableRgb = false;
  bool enableDepth = false;
  bool enableIr = false;

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
    RgbTextureClass *rgbCls = RGB_TEXTURE_GET_CLASS(rgbTexture);
    DepthTextureClass *depthCls = DEPTH_TEXTURE_GET_CLASS(depthTexture);
    IrTextureClass *irCls = IR_TEXTURE_GET_CLASS(irTexture);
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
          rgbCls->cvImage = cv::Mat(rgbFrame.getHeight(), rgbFrame.getWidth(), CV_8UC3, (void *)rgbFrame.getData());
          rgbCls->pipeline->run(rgbCls->cvImage, *registrar, *FL_TEXTURE(rgbTexture), rgbCls->video_width, rgbCls->video_height, rgbCls->buffer, models, flChannel);
          rgbNewFrame = true;
        }
      }

      if (niDepthAvailable && enableIr && vsDepth.isValid())
      {
        if (vsDepth.readFrame(&depthFrame) == STATUS_OK)
        {
          depthCls->cvImage = cv::Mat(depthFrame.getHeight(), depthFrame.getWidth(), CV_16UC1, (void *)depthFrame.getData());
          depthCls->pipeline->run(depthCls->cvImage, *registrar, *FL_TEXTURE(depthTexture), depthCls->video_width, depthCls->video_height, depthCls->buffer, models, flChannel);
          depthNewFrame = true;
        }
      }

      if (niIrAvailable && enableDepth && vsDepth.isValid())
      {
        if (vsIR.readFrame(&irFrame) == STATUS_OK)
        {
          irCls->cvImage = cv::Mat(irFrame.getHeight(), irFrame.getWidth(), CV_16UC1, (void *)irFrame.getData());
          irCls->pipeline->run(irCls->cvImage, *registrar, *FL_TEXTURE(irTexture), irCls->video_width, irCls->video_height, irCls->buffer, models, flChannel);
          irNewFrame = true;
        }
      }

      // // TODO: chose model
      // if ((rgbNewFrame || depthNewFrame || irNewFrame) && models->size())
      // {
      //   tfPipeline->run(rgbCls->cvImage, depthCls->cvImage, irCls->cvImage, *models->at(0));
      // }

      // if (niRgbAvailable && depthNewFrame && rgbNewFrame)
      // {
      //   niComputeCloud(vsDepth, (const openni::DepthPixel *)depthFrame.getData(), (const openni::RGB888Pixel *)rgbFrame.getData(), glfl->modelPointCloud->vertices, glfl->modelPointCloud->colors, glfl->modelPointCloud->colorsMap, &glfl->modelPointCloud->vertexPoints);
      // }

      fl_method_channel_invoke_method(flChannel, "onFrame", nullptr, nullptr, nullptr, NULL);
    }

    rgbCls->pipeline->reset();
    depthCls->pipeline->reset();
    irCls->pipeline->reset();
  }
};
