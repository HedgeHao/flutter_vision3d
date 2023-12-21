#include "include/flutter_vision/flutter_vision_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <thread>

#include "include/flutter_vision/pipeline/pipeline.h"
#include "include/flutter_vision/texture.h"
#include "include/flutter_vision/camera/realsense.h"
#include "include/flutter_vision/camera/openni2.h"
#include "include/flutter_vision/camera/dummy.h"
#include "include/flutter_vision/camera/uvc.h"
#include "include/flutter_vision/barcode_scanner/opencv_barcode.hpp"
#include "include/flutter_vision/barcode_scanner/zxing.hpp"

#include "include/flutter_vision/opengl/opengl.h"

#define PIPELINE_INDEX_TFLITE 8

cv::Mat emptyMat = cv::Mat::zeros(3, 3, CV_64F);

enum CameraType
{
  OPENNI = 0,
  REALSENSE = 1,
  DUMMY = 2,
  UVC = 3,
};

namespace
{
  template <typename T>
  void parseDartArgument(const flutter::EncodableMap *arguments, const char *name, T *value)
  {
    auto valueIt = arguments->find(flutter::EncodableValue(name));
    if (valueIt != arguments->end())
    {
      *value = std::get<T>(valueIt->second);
    }
  }

  class FlutterVisionPlugin : public flutter::Plugin
  {
  public:
    flutter::MethodChannel<flutter::EncodableValue> *flChannel;
    OpenGLFL *glfl;

    std::vector<TFLiteModel *> models{};
    std::vector<Pipeline *> pipelines{};
    std::vector<FvCamera *> cams{};
    std::vector<cv::Mat *> cvMats{};

    static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

    FlutterVisionPlugin(flutter::TextureRegistrar *texture_registrar);

    virtual ~FlutterVisionPlugin();

  private:
    // Called when a method is called on this plugin's channel from Dart.
    void HandleMethodCall(
        const flutter::MethodCall<flutter::EncodableValue> &method_call,
        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    flutter::TextureRegistrar *textureRegistrar;
    std::unique_ptr<FvTexture> uvcTexture;

    std::unique_ptr<CVBarCodeDetector> barcodeDetector = std::make_unique<CVBarCodeDetector>();
  };

  // static
  void FlutterVisionPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarWindows *registrar)
  {
    auto plugin = std::make_unique<FlutterVisionPlugin>(registrar->texture_registrar());

    plugin->flChannel =
        new flutter::MethodChannel<flutter::EncodableValue>(
            registrar->messenger(),
            "flutter_vision",
            &flutter::StandardMethodCodec::GetInstance());

    plugin->flChannel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result)
        {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    plugin->glfl->init();

    registrar->AddPlugin(std::move(plugin));
  }

  FlutterVisionPlugin::FlutterVisionPlugin(flutter::TextureRegistrar *texture_registrar)
  {
    textureRegistrar = texture_registrar;
    uvcTexture = std::make_unique<FvTexture>(textureRegistrar);
    glfl = new OpenGLFL(textureRegistrar);
  }

  FlutterVisionPlugin::~FlutterVisionPlugin() {}

  void FlutterVisionPlugin::HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
  {
    const flutter::EncodableMap *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

    if (method_call.method_name().compare("ni2Initialize") == 0)
    {
      int ret = OpenniCam::openniInit();
      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("ni2EnumerateDevices") == 0)
    {
      Array<DeviceInfo> devs;
      OpenniCam::enumerateDevices(&devs);

      flutter::EncodableList flDeviceList = flutter::EncodableList();
      for (int i = 0; i < devs.getSize(); i++)
      {
        const DeviceInfo d = devs[i];
        flutter::EncodableMap map = flutter::EncodableMap();
        map[flutter::EncodableValue("name")] = d.getName();
        map[flutter::EncodableValue("uri")] = d.getUri();
        map[flutter::EncodableValue("productId")] = d.getUsbProductId();
        map[flutter::EncodableValue("vendorId")] = d.getUsbVendorId();
        map[flutter::EncodableValue("vendor")] = d.getVendor();
        flDeviceList.push_back(map);
      }

      result->Success(flDeviceList);
    }
    else if (method_call.method_name().compare("ni2GetAvailableVideoModes") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int index;
      parseDartArgument<int>(arguments, "index", &index);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);

      flutter::EncodableList list = flutter::EncodableList();
      if (cam != nullptr && cam->type == (CameraType::OPENNI))
      {
        std::vector<std::string> modes;
        cam->getAvailableVideoModes(index, modes);

        for (auto s : modes)
        {
          list.push_back(s);
        }
      }

      result->Success(flutter::EncodableValue(list));
    }
    else if (method_call.method_name().compare("ni2GetCurrentVideoMode") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int index;
      parseDartArgument<int>(arguments, "index", &index);

      std::string mode = "";

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam != nullptr && cam->type == (CameraType::OPENNI))
      {
        cam->getCurrentVideoMode(index, mode);

        result->Success(flutter::EncodableValue(mode));
        return;
      }

      result->Success(flutter::EncodableValue(""));
    }
    else if (method_call.method_name().compare("ni2SetVideoMode") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int index;
      parseDartArgument<int>(arguments, "index", &index);

      int mode;
      parseDartArgument<int>(arguments, "mode", &mode);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);

      if (cam != nullptr && cam->type == (CameraType::OPENNI))
      {
        cam->setVideoMode(index, mode);

        result->Success(flutter::EncodableValue(true));
        return;
      }

      result->Success(flutter::EncodableValue(false));
    }
    else if (method_call.method_name().compare("fvCameraOpen") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);
      int cameraType;
      parseDartArgument<int>(arguments, "cameraType", &cameraType);

      flutter::EncodableMap map = flutter::EncodableMap();
      FvCamera *cam;
      if (cameraType == CameraType::OPENNI)
      {
        cam = new OpenniCam(serial.c_str());
      }
      else if (cameraType == CameraType::REALSENSE)
      {
        cam = new RealsenseCam(serial.c_str());
      }
      else if (cameraType == CameraType::DUMMY)
      {
        cam = new DummyCam(serial.c_str());
      }
      else if (cameraType == CameraType::UVC)
      {
        cam = new UvcCam(serial.c_str());
      }

      cam->type = cameraType;

      int ret = -1;
      if (cam != nullptr)
      {
        cam->fvInit(textureRegistrar, &models, flChannel, glfl);
        cam->camInit();

        ret = cam->openDevice();
        if (ret == 0)
        {
          cams.push_back(cam);
          map[flutter::EncodableValue("rgbTextureId")] = cam->rgbTexture->textureId;
          map[flutter::EncodableValue("depthTextureId")] = cam->depthTexture->textureId;
          map[flutter::EncodableValue("irTextureId")] = cam->irTexture->textureId;
        }
      }

      map[flutter::EncodableValue("ret")] = ret;
      result->Success(flutter::EncodableValue(map));
    }
    else if (method_call.method_name().compare("fvCameraClose") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      int ret = -1;
      if (cam != nullptr)
      {
        ret = cam->closeDevice();

        FvCamera::removeCam(serial.c_str(), &cams);
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("fvCameraIsConnected") == 0)
    {
      // TODO: unimplement
      result->Success(flutter::EncodableValue(true));
    }
    else if (method_call.method_name().compare("fvCameraConfigVideoStream") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int cameraType;
      parseDartArgument<int>(arguments, "cameraType", &cameraType);

      int videoModeIndex;
      parseDartArgument<int>(arguments, "videoModeIndex", &videoModeIndex);

      bool enable;
      parseDartArgument<bool>(arguments, "enable", &enable);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        int ret = cam->configVideoStream(videoModeIndex, &enable);
        if (ret != 0)
        {
          result->Success(flutter::EncodableValue(false));
          return;
        }

        if (enable)
        {
          cam->readVideoFeed();
        }
      }

      // TODO: check enable/disable is really success
      result->Success(flutter::EncodableValue(enable));
    }
    else if (method_call.method_name().compare("fvGetOpenCVMat") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int index;
      parseDartArgument<int>(arguments, "index", &index);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        int64_t pointer = cam->getOpenCVMat(index);
        result->Success(flutter::EncodableValue(pointer));
        return;
      }

      result->Success(flutter::EncodableValue(0));
    }
    else if (method_call.method_name().compare("fvPauseStream") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      bool pause;
      parseDartArgument<bool>(arguments, "pause", &pause);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        cam->pause(pause);
        result->Success(flutter::EncodableValue(true));
        return;
      }

      result->Success(flutter::EncodableValue(false));
    }
    else if (method_call.method_name().compare("fvGetSerialNumber") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        std::string sn;
        cam->getSerialNumber(sn);

        result->Success(flutter::EncodableValue(sn.c_str()));
        return;
      }

      result->Success(flutter::EncodableValue(""));
    }
    else if (method_call.method_name().compare("fvGetDepthData") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        int index;
        parseDartArgument<int>(arguments, "index", &index);

        flutter::EncodableList list = flutter::EncodableList();
        uint16_t *data;
        int *temp;
        int length = 0;
        if (index == 0)
        {
          data = cam->getDepthData();
          length = cam->depthWidth * cam->depthHeight;
          temp = new int[length];
          for (int i = 0; i < length; i++)
          {
            list.push_back(data[i]);
          }
        }
        else if (index == 1)
        {
          int x;
          parseDartArgument<int>(arguments, "x", &x);

          int y;
          parseDartArgument<int>(arguments, "y", &y);

          data = cam->getDepthData();
          list.push_back(data[y * cam->depthHeight + x]);
        }
        else if (index == 2)
        {
          int roi_x;
          parseDartArgument<int>(arguments, "x", &roi_x);

          int roi_y;
          parseDartArgument<int>(arguments, "y", &roi_y);

          int roi_width;
          parseDartArgument<int>(arguments, "roi_width", &roi_width);

          int roi_height;
          parseDartArgument<int>(arguments, "roi_height", &roi_height);

          if (roi_x < 0 || roi_y < 0 || roi_x + roi_width > cam->depthWidth || roi_y + roi_height > cam->depthHeight)
          {
            // TODO: Workaround: put error code in return data. should throw error.
            // Wrong ROI
            list.push_back(-2);
          }
          else
          {
            data = cam->getDepthData();
            length = roi_width * roi_height;
            temp = new int[length];
            int index = 0;
            for (int y = roi_y; y < roi_y + roi_height; ++y)
            {
              for (int x = roi_x; x < roi_x + roi_width; ++x)
              {
                list.push_back(data[y * cam->depthWidth + x]);
              }
            }
          }
        }
        else
        {
          // TODO: Workaround: put error code in return data. should throw error.
          list.push_back(-1);
        }

        result->Success(list);
      }
    }
    else if (method_call.method_name().compare("ni2SetVideoSize") == 0)
    {
      int videoIndex;
      parseDartArgument<int>(arguments, "videoIndex", &videoIndex);

      int width;
      parseDartArgument<int>(arguments, "width", &width);

      int height;
      parseDartArgument<int>(arguments, "height", &height);

      // TODO: implement

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("rsEnumerateDevices") == 0)
    {
      std::vector<std::string> serials = RealsenseHelper::enumerateDevices();

      flutter::EncodableList list = flutter::EncodableList();
      for (auto s : serials)
      {
        list.push_back(s);
      }

      result->Success(list);
    }
    else if (method_call.method_name().compare("getOpenglTextureId") == 0)
    {
      result->Success(flutter::EncodableValue(glfl->openglTexture->textureId));
    }
    else if (method_call.method_name().compare("openglSetCamPosition") == 0)
    {
      double x, y, z;
      parseDartArgument<double>(arguments, "x", &x);
      parseDartArgument<double>(arguments, "y", &y);
      parseDartArgument<double>(arguments, "z", &z);

      glfl->setCamPosition(x, y, z);
      glfl->renderManually();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("openglSetCamAngle") == 0)
    {
      double yaw, pitch;
      parseDartArgument<double>(arguments, "yaw", &yaw);
      parseDartArgument<double>(arguments, "pitch", &pitch);

      glfl->setYawPitch(yaw, pitch);
      glfl->renderManually();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("openglSetCamFov") == 0)
    {
      double fov;
      parseDartArgument<double>(arguments, "fov", &fov);

      glfl->setFov(fov);
      glfl->renderManually();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("openglRender") == 0)
    {
      glfl->isRendering = true;
      glfl->render();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("fvCameraEnablePointCloud") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      bool enable = false;
      parseDartArgument<bool>(arguments, "enable", &enable);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      int ret = -1;
      if (cam != nullptr)
      {
        ret = cam->enablePointCloud = enable;
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("pipelineAdd") == 0)
    {
      int index = -1;
      parseDartArgument<int>(arguments, "index", &index);

      int funcIndex = -1;
      parseDartArgument<int>(arguments, "funcIndex", &funcIndex);

      int len = 0;
      parseDartArgument<int>(arguments, "len", &len);

      std::vector<uint8_t> params{};
      if (len > 0)
      {
        parseDartArgument<std::vector<uint8_t>>(arguments, "params", &params);
      }

      int insertAt = -1;
      parseDartArgument<int>(arguments, "at", &insertAt);

      int interval = 0;
      parseDartArgument<int>(arguments, "interval", &interval);

      bool append = false;
      parseDartArgument<bool>(arguments, "append", &append);

      bool runOnce = false;
      parseDartArgument<bool>(arguments, "runOnce", &runOnce);

      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam != nullptr)
      {
        if (index == VideoIndex::RGB)
        {
          cam->rgbTexture->pipeline->add(funcIndex, params, len, insertAt, interval, append, runOnce);
        }
        else if (index == VideoIndex::Depth)
        {
          cam->depthTexture->pipeline->add(funcIndex, params, len, insertAt, interval, append, runOnce);
        }
        else if (index == VideoIndex::IR)
        {
          cam->irTexture->pipeline->add(funcIndex, params, len, insertAt, interval, append, runOnce);
        }
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("pipelineRun") == 0)
    {
      int index = -1;
      parseDartArgument<int>(arguments, "index", &index);

      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int from = 0;
      parseDartArgument<int>(arguments, "from", &from);

      int to = -1;
      parseDartArgument<int>(arguments, "to", &to);

      int ret = 0;
      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        if (index == VideoIndex::RGB)
        {
          ret = cam->rgbTexture->pipeline->runOnce(cam->rgbTexture, textureRegistrar, &models, flChannel, from, to);
          cam->rgbTexture->setPixelBuffer();
        }
        else if (index == VideoIndex::Depth)
        {
          ret = cam->depthTexture->pipeline->runOnce(cam->depthTexture, textureRegistrar, &models, flChannel, from, to);
          cam->depthTexture->setPixelBuffer();
        }
        else if (index == VideoIndex::IR)
        {
          ret = cam->irTexture->pipeline->runOnce(cam->irTexture, textureRegistrar, &models, flChannel, from, to);
          cam->irTexture->setPixelBuffer();
        }
      }

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("pipelineClear") == 0)
    {
      int index = -1;
      parseDartArgument<int>(arguments, "index", &index);

      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        if (index == VideoIndex::RGB)
        {
          cam->rgbTexture->pipeline->clear();
        }
        else if (index == VideoIndex::Depth)
        {
          cam->depthTexture->pipeline->clear();
        }
        else if (index == VideoIndex::IR)
        {
          cam->irTexture->pipeline->clear();
        }
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("pipelineInfo") == 0)
    {
      int index = -1;
      parseDartArgument<int>(arguments, "index", &index);

      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      std::string info;
      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        if (index == VideoIndex::RGB)
        {
          info = cam->rgbTexture->pipeline->getPipelineInfo();
        }
        else if (index == VideoIndex::Depth)
        {
          info = cam->depthTexture->pipeline->getPipelineInfo();
        }
        else if (index == VideoIndex::IR)
        {
          info = cam->irTexture->pipeline->getPipelineInfo();
        }
      }

      result->Success(flutter::EncodableValue(info));
    }
    else if (method_call.method_name().compare("pipelineError") == 0)
    {
      int index = -1;
      parseDartArgument<int>(arguments, "index", &index);

      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      std::string error;
      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        if (index == VideoIndex::RGB)
        {
          error = cam->rgbTexture->pipeline->error;
        }
        else if (index == VideoIndex::Depth)
        {
          error = cam->depthTexture->pipeline->error;
        }
        else if (index == VideoIndex::IR)
        {
          error = cam->irTexture->pipeline->error;
        }
      }

      result->Success(flutter::EncodableValue(error));
    }
    else if (method_call.method_name().compare("fvCameraConfig") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int prop = 0;
      parseDartArgument<int>(arguments, "prop", &prop);

      std::vector<float> value{};
      parseDartArgument<std::vector<float>>(arguments, "value", &value);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      int ret = -1;
      if (cam)
      {
        cam->configure(prop, value);
        ret = 0;
      }

      result->Success(flutter::EncodableValue(ret == 0));
    }
    else if (method_call.method_name().compare("fvCameraGetConfiguration") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int prop = 0;
      parseDartArgument<int>(arguments, "prop", &prop);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      int ret = -1;
      if (cam)
      {
        ret = cam->getConfiguration(prop);
      }

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("rsLoadPresetParameters") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      std::string path;
      parseDartArgument<std::string>(arguments, "path", &serial);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        cam->loadPresetParameters(path);
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("fvGetIntrinsic") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int index = -1;
      parseDartArgument<int>(arguments, "index", &index);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      flutter::EncodableMap map = flutter::EncodableMap();
      double fx, fy, cx, cy;
      if (cam)
      {
        cam->getIntrinsic(index, fx, fy, cx, cy);
      }

      map[flutter::EncodableValue("fx")] = fx;
      map[flutter::EncodableValue("fy")] = fy;
      map[flutter::EncodableValue("cx")] = cx;
      map[flutter::EncodableValue("cy")] = cy;

      result->Success(flutter::EncodableValue(map));
    }
    else if (method_call.method_name().compare("fvEnableRegistration") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      bool enable;
      parseDartArgument<bool>(arguments, "enable", &enable);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      bool ret = false;
      if (cam)
      {
        ret = cam->enableImageRegistration(enable);
      }

      result->Success(flutter::EncodableValue(ret ? 0 : -1));
    }
    else if (method_call.method_name().compare("tfliteCreateModel") == 0)
    {
      std::string path;
      parseDartArgument<std::string>(arguments, "modelPath", &path);

      TFLiteModel *m = new TFLiteModel(path.c_str());
      this->models.push_back(m);

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("tfliteGetModelInfo") == 0)
    {
      int index = 0;
      parseDartArgument<int>(arguments, "index", &index);

      TFLiteModel *m = this->models[index];

      flutter::EncodableMap map = flutter::EncodableMap();
      map[flutter::EncodableValue("valid")] = m->valid;
      map[flutter::EncodableValue("error")] = m->error.c_str();

      result->Success(flutter::EncodableValue(map));
    }
    else if (method_call.method_name().compare("tfliteGetTensorOutput") == 0)
    {
      int index = 0;
      parseDartArgument<int>(arguments, "tensorIndex", &index);

      std::vector<int32_t> size;
      parseDartArgument<std::vector<int32_t>>(arguments, "size", &size);

      int outputSize = 1;
      for (int i = 0; i < size.size(); i++)
        outputSize *= size[i];
      float *data = new float[outputSize];

      models[0]->retrieveOutput<float>(index, outputSize, data);

      flutter::EncodableList fl = flutter::EncodableList();
      for (int i = 0; i < outputSize; i++)
      {
        fl.push_back(*(data + i));
      }

      result->Success(fl);
    }
    else if (method_call.method_name().compare("cvBarcodeInit") == 0)
    {
      std::string prototxt;
      parseDartArgument<std::string>(arguments, "prototxt", &prototxt);

      std::string model;
      parseDartArgument<std::string>(arguments, "model", &model);

      barcodeDetector->init(prototxt, model);
      result->Success(flutter::EncodableValue(true));
    }
    else if (method_call.method_name().compare("cvBarcodeScan") == 0)
    {
      int64_t imagePointer;
      parseDartArgument<int64_t>(arguments, "imagePointer", &imagePointer);
      std::uintptr_t pointer = imagePointer;

      cv::Mat *mat = (cv::Mat *)pointer;
      if (mat->empty())
      {
        result->Success(flutter::EncodableValue(-5));
        return;
      }

      cv::cvtColor(*mat, *mat, COLOR_RGBA2BGR);
      int n = barcodeDetector->detect(*mat);

      if (n <= 0)
      {
        result->Success(flutter::EncodableValue(n));
        return;
      }

      if (!barcodeDetector->decode(*mat))
      {
        result->Success(flutter::EncodableValue(-2));
        return;
      }

      flutter::EncodableList list = flutter::EncodableList();

      for (int i = 0; i < barcodeDetector->decode_info.size(); i++)
      {
        flutter::EncodableMap map = flutter::EncodableMap();
        map[flutter::EncodableValue("px1")] = barcodeDetector->corners[i * 4].x;
        map[flutter::EncodableValue("py1")] = barcodeDetector->corners[i * 4].y;
        map[flutter::EncodableValue("px2")] = barcodeDetector->corners[i * 4 + 1].x;
        map[flutter::EncodableValue("py2")] = barcodeDetector->corners[i * 4 + 1].y;
        map[flutter::EncodableValue("px3")] = barcodeDetector->corners[i * 4 + 2].x;
        map[flutter::EncodableValue("py3")] = barcodeDetector->corners[i * 4 + 2].y;
        map[flutter::EncodableValue("px4")] = barcodeDetector->corners[i * 4 + 3].x;
        map[flutter::EncodableValue("py4")] = barcodeDetector->corners[i * 4 + 3].y;
        map[flutter::EncodableValue("data")] = barcodeDetector->decode_info[i];
        list.push_back(map);
      }

      result->Success(flutter::EncodableValue(list));
    }
    else if (method_call.method_name().compare("zxingBarcodeScan") == 0)
    {
      int64_t imagePointer;
      parseDartArgument<int64_t>(arguments, "imagePointer", &imagePointer);
      std::uintptr_t pointer = imagePointer;

      cv::Mat *mat = (cv::Mat *)pointer;
      if (mat->empty())
      {
        result->Success(flutter::EncodableValue(-5));
        return;
      }

      cv::cvtColor(*mat, *mat, COLOR_RGBA2BGR);

      ZXing::Results ret = ZXing::ReadBarcodes(*mat);

      auto zx2cv = [](ZXing::PointI p)
      { return cv::Point(p.x, p.y); };

      flutter::EncodableList list = flutter::EncodableList();
      for (int i = 0; i < ret.size(); i++)
      {
        auto pos = ret[i].position();
        flutter::EncodableMap map = flutter::EncodableMap();
        map[flutter::EncodableValue("px1")] = pos[0].x;
        map[flutter::EncodableValue("py1")] = pos[0].y;
        map[flutter::EncodableValue("px2")] = pos[1].x;
        map[flutter::EncodableValue("py2")] = pos[1].y;
        map[flutter::EncodableValue("px3")] = pos[2].x;
        map[flutter::EncodableValue("py3")] = pos[2].y;
        map[flutter::EncodableValue("px4")] = pos[3].x;
        map[flutter::EncodableValue("py4")] = pos[3].y;
        map[flutter::EncodableValue("data")] = ret[i].text();
        list.push_back(map);
      }

      result->Success(flutter::EncodableValue(list));
    }
    else if (method_call.method_name().compare("_float2uint8") == 0)
    {
      double vD = 0.0f;
      parseDartArgument<double>(arguments, "value", &vD);
      float v = (float)vD;

      flutter::EncodableList fl = flutter::EncodableList();
      uint8_t *bytes = reinterpret_cast<uint8_t *>(&v);
      for (int i = 0; i < 4; i++)
      {
        fl.push_back(flutter::EncodableValue(*(bytes + i)));
      }
      result->Success(fl);
    }
    else if (method_call.method_name().compare("fvCameraScreenshot") == 0)
    {
      std::string serial;
      parseDartArgument<std::string>(arguments, "serial", &serial);

      int index = 0;
      parseDartArgument<int>(arguments, "index", &index);

      std::string path;
      parseDartArgument<std::string>(arguments, "path", &path);

      int cvtCode = 0;
      parseDartArgument<int>(arguments, "cvtCode", &cvtCode);

      bool ret = false;
      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam != nullptr)
      {
        if (index == VideoIndex::RGB)
        {
          cam->rgbTexture->pipeline->screenshot(path.c_str(), cvtCode);
        }
        else if (index == VideoIndex::Depth)
        {
          cam->depthTexture->pipeline->screenshot(path.c_str(), cvtCode);
        }
        else if (index == VideoIndex::IR)
        {
          cam->irTexture->pipeline->screenshot(path.c_str(), cvtCode);
        }

        ret = true;
      }

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("test") == 0)
    {
      result->Success(flutter::EncodableValue(true));
    }
    else
    {
      result->NotImplemented();
    }
  }

} // namespace

void FlutterVisionPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar)
{
  FlutterVisionPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
