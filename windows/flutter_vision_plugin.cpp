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
  void parseDartArugment(const flutter::EncodableMap *arguments, const char *name, T *value)
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
    std::vector<OpenCVCamera *> cameras{};
    std::vector<Pipeline *> pipelines{};
    std::vector<FvCamera *> cams{};

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
    else if (method_call.method_name().compare("fvCameraOpen") == 0)
    {
      std::string serial;
      parseDartArugment<std::string>(arguments, "serial", &serial);
      int cameraType;
      parseDartArugment<int>(arguments, "cameraType", &cameraType);

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
      parseDartArugment<std::string>(arguments, "serial", &serial);

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
      parseDartArugment<std::string>(arguments, "serial", &serial);

      int cameraType;
      parseDartArugment<int>(arguments, "cameraType", &cameraType);

      int videoModeIndex;
      parseDartArugment<int>(arguments, "videoModeIndex", &videoModeIndex);

      bool enable;
      parseDartArugment<bool>(arguments, "enable", &enable);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        cam->configVideoStream(videoModeIndex, &enable);
        if (enable)
        {
          cam->readVideoFeed();
        }
      }

      // TODO: check enable/disable is really success
      result->Success(flutter::EncodableValue(enable));
    }
    else if (method_call.method_name().compare("ni2SetVideoSize") == 0)
    {
      int videoIndex;
      parseDartArugment<int>(arguments, "videoIndex", &videoIndex);

      int width;
      parseDartArugment<int>(arguments, "width", &width);

      int height;
      parseDartArugment<int>(arguments, "height", &height);

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
      parseDartArugment<double>(arguments, "x", &x);
      parseDartArugment<double>(arguments, "y", &y);
      parseDartArugment<double>(arguments, "z", &z);

      glfl->setCamPosition(x, y, z);
      glfl->renderManually();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("openglSetCamAngle") == 0)
    {
      double yaw, pitch;
      parseDartArugment<double>(arguments, "yaw", &yaw);
      parseDartArugment<double>(arguments, "pitch", &pitch);

      glfl->setYawPitch(yaw, pitch);
      glfl->renderManually();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("openglSetCamFov") == 0)
    {
      double fov;
      parseDartArugment<double>(arguments, "fov", &fov);

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
      parseDartArugment<std::string>(arguments, "serial", &serial);

      bool enable = false;
      parseDartArugment<bool>(arguments, "enable", &enable);

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
      parseDartArugment<int>(arguments, "index", &index);

      int funcIndex = -1;
      parseDartArugment<int>(arguments, "funcIndex", &funcIndex);

      int len = 0;
      parseDartArugment<int>(arguments, "len", &len);

      std::vector<uint8_t> params{};
      if (len > 0)
      {
        parseDartArugment<std::vector<uint8_t>>(arguments, "params", &params);
      }

      int insertAt = -1;
      parseDartArugment<int>(arguments, "at", &insertAt);

      int interval = 0;
      parseDartArugment<int>(arguments, "interval", &interval);

      bool append = false;
      parseDartArugment<bool>(arguments, "append", &append);

      std::string serial;
      parseDartArugment<std::string>(arguments, "serial", &serial);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam != nullptr)
      {
        if (index == VideoIndex::RGB)
        {
          cam->rgbTexture->pipeline->add(funcIndex, params, len, insertAt, interval, append);
        }
        else if (index == VideoIndex::Depth)
        {
          cam->depthTexture->pipeline->add(funcIndex, params, len, insertAt, interval, append);
        }
        else if (index == VideoIndex::IR)
        {
          cam->irTexture->pipeline->add(funcIndex, params, len, insertAt, interval, append);
        }
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("pipelineRun") == 0)
    {
      int index = -1;
      parseDartArugment<int>(arguments, "index", &index);

      std::string serial;
      parseDartArugment<std::string>(arguments, "serial", &serial);

      int from = 0;
      parseDartArugment<int>(arguments, "from", &from);

      int to = -1;
      parseDartArugment<int>(arguments, "to", &to);

      int ret = 0;
      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if (cam)
      {
        if (index == VideoIndex::RGB)
        {
          ret = cam->rgbTexture->pipeline->runOnce(textureRegistrar, cam->rgbTexture->textureId, cam->rgbTexture->videoWidth, cam->rgbTexture->videoHeight, cam->rgbTexture->buffer, &models, flChannel, from, to);
          cam->rgbTexture->setPixelBuffer();
        }
        else if (index == VideoIndex::Depth)
        {
          ret = cam->depthTexture->pipeline->runOnce(textureRegistrar, cam->depthTexture->textureId, cam->depthTexture->videoWidth, cam->depthTexture->videoHeight, cam->depthTexture->buffer, &models, flChannel, from, to);
          cam->depthTexture->setPixelBuffer();
        }
        else if (index == VideoIndex::IR)
        {
          ret = cam->irTexture->pipeline->runOnce(textureRegistrar, cam->irTexture->textureId, cam->irTexture->videoWidth, cam->irTexture->videoHeight, cam->irTexture->buffer, &models, flChannel, from, to);
          cam->irTexture->setPixelBuffer();
        }
      }

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("pipelineClear") == 0)
    {
      int index = -1;
      parseDartArugment<int>(arguments, "index", &index);

      std::string serial;
      parseDartArugment<std::string>(arguments, "serial", &serial);

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
      parseDartArugment<int>(arguments, "index", &index);

      std::string serial;
      parseDartArugment<std::string>(arguments, "serial", &serial);

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
      parseDartArugment<int>(arguments, "index", &index);

      std::string serial;
      parseDartArugment<std::string>(arguments, "serial", &serial);

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
      parseDartArugment<std::string>(arguments, "serial", &serial);

      int prop = 0;
      parseDartArugment<int>(arguments, "prop", &prop);

      std::vector<float> value{};
      parseDartArugment<std::vector<float>>(arguments, "value", &value);

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
      parseDartArugment<std::string>(arguments, "serial", &serial);

      int prop = 0;
      parseDartArugment<int>(arguments, "prop", &prop);

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      int ret = -1;
      if (cam)
      {
        ret = cam->getConfiguration(prop);
      }

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("tfliteCreateModel") == 0)
    {
      std::string path;
      parseDartArugment<std::string>(arguments, "modelPath", &path);

      TFLiteModel *m = new TFLiteModel(path.c_str());
      this->models.push_back(m);

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("tfliteGetModelInfo") == 0)
    {
      int index = 0;
      parseDartArugment<int>(arguments, "index", &index);

      TFLiteModel *m = this->models[index];

      flutter::EncodableMap map = flutter::EncodableMap();
      map[flutter::EncodableValue("valid")] = m->valid;
      map[flutter::EncodableValue("error")] = m->error.c_str();

      result->Success(flutter::EncodableValue(map));
    }
    else if (method_call.method_name().compare("tfliteGetTensorOutput") == 0)
    {
      int index = 0;
      parseDartArugment<int>(arguments, "tensorIndex", &index);

      std::vector<int32_t> size;
      parseDartArugment<std::vector<int32_t>>(arguments, "size", &size);

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
    else if (method_call.method_name().compare("_float2uint8") == 0)
    {
      double vD = 0.0f;
      parseDartArugment<double>(arguments, "value", &vD);
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
      parseDartArugment<std::string>(arguments, "serial", &serial);

      int index = 0;
      parseDartArugment<int>(arguments, "index", &index);

      std::string path;
      parseDartArugment<std::string>(arguments, "path", &path);

      int cvtCode = 0;
      parseDartArugment<int>(arguments, "cvtCode", &cvtCode);

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
      // cv::Mat b(1280, 720, CV_8UC4, cv::Scalar(255, 0, 0, 255));
      cv::Mat b = cv::imread("D:/test/faces.jpg", cv::IMREAD_COLOR);
      cv::cvtColor(b, b, cv::COLOR_BGR2RGB);
      cv::Mat g(500, 500, CV_16UC1, cv::Scalar(125, 125, 125, 255));
      cv::Mat r(500, 500, CV_16UC1, cv::Scalar(220, 220, 220, 255));

      cams[1]->rgbTexture->pipeline->run(b, textureRegistrar, cams[1]->rgbTexture->textureId, cams[1]->rgbTexture->videoWidth, cams[1]->rgbTexture->videoHeight, cams[1]->rgbTexture->buffer, &models, flChannel);
      cams[1]->rgbTexture->setPixelBuffer();
      cams[1]->irTexture->pipeline->run(g, textureRegistrar, cams[1]->irTexture->textureId, cams[1]->irTexture->videoWidth, cams[1]->irTexture->videoHeight, cams[1]->irTexture->buffer, &models, flChannel);
      cams[1]->irTexture->setPixelBuffer();
      cams[1]->depthTexture->pipeline->run(r, textureRegistrar, cams[1]->depthTexture->textureId, cams[1]->depthTexture->videoWidth, cams[1]->depthTexture->videoHeight, cams[1]->depthTexture->buffer, &models, flChannel);
      cams[1]->depthTexture->setPixelBuffer();
      // uvcTexture->pipeline->run(b, textureRegistrar, uvcTexture->textureId, uvcTexture->videoWidth, uvcTexture->videoHeight, uvcTexture->buffer, &models, flChannel);
      // uvcTexture->setPixelBuffer();

      // rgbTexture->genPixels();
      // depthTexture->genPixels();
      // irTexture->genPixels();

      // ni2->test();

      // glfl->render();
      // glfl->test();

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
