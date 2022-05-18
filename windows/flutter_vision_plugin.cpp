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
#include "include/flutter_vision/pipeline/tf_pipeline.h"
#include "include/flutter_vision/texture.h"
#include "include/flutter_vision/openni2_wrapper.hpp"
#include "include/flutter_vision/realsense.h"

#include "include/flutter_vision/opengl/opengl.h"

#define PIPELINE_INDEX_TFLITE 8

cv::Mat emptyMat = cv::Mat::zeros(3, 3, CV_64F);

namespace
{

  class FlutterVisionPlugin : public flutter::Plugin
  {
  public:
    flutter::MethodChannel<flutter::EncodableValue> *flChannel;
    OpenNi2Wrapper *ni2 = new OpenNi2Wrapper();
    OpenGLFL *glfl;

    // TfPipeline *tfPipeline;
    std::vector<TFLiteModel *> models{};
    std::vector<OpenCVCamera *> cameras{};
    std::vector<Pipeline *> pipelines{};
    std::vector<RealsenseCam *> rsCams{};

    static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

    FlutterVisionPlugin(flutter::TextureRegistrar *texture_registrar);

    virtual ~FlutterVisionPlugin();

  private:
    // Called when a method is called on this plugin's channel from Dart.
    void HandleMethodCall(
        const flutter::MethodCall<flutter::EncodableValue> &method_call,
        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    flutter::TextureRegistrar *textureRegistrar;
    std::unique_ptr<FvTexture> rgbTexture;
    std::unique_ptr<FvTexture> depthTexture;
    std::unique_ptr<FvTexture> irTexture;
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

    plugin->ni2->registerFlContext(plugin->flChannel);
    plugin->glfl->init();

    registrar->AddPlugin(std::move(plugin));
  }

  FlutterVisionPlugin::FlutterVisionPlugin(flutter::TextureRegistrar *texture_registrar)
  {
    textureRegistrar = texture_registrar;
    rgbTexture = std::make_unique<FvTexture>(textureRegistrar);
    depthTexture = std::make_unique<FvTexture>(textureRegistrar);
    irTexture = std::make_unique<FvTexture>(textureRegistrar);
    uvcTexture = std::make_unique<FvTexture>(textureRegistrar);
    glfl = new OpenGLFL(textureRegistrar);

    ni2->registerFlContext(textureRegistrar, rgbTexture.get(), depthTexture.get(), irTexture.get(), glfl, &models);

    rgbTexture->stream = &ni2->vsColor;
    depthTexture->stream = &ni2->vsDepth;
    irTexture->stream = &ni2->vsIR;
  }

  FlutterVisionPlugin::~FlutterVisionPlugin() {}

  RealsenseCam *findRsCam(const char *serial, std::vector<RealsenseCam *> *cams)
  {
    RealsenseCam *cam = nullptr;
    for (auto c : *cams)
    {
      if (strcmp(c->serial.c_str(), serial) == 0)
      {
        return c;
      }
    }

    return nullptr;
  }

  void FlutterVisionPlugin::HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
  {
    const flutter::EncodableMap *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

    if (method_call.method_name().compare("ni2GetVideoTexture") == 0)
    {
      int index;
      auto indexIt = arguments->find(flutter::EncodableValue("videoIndex"));
      if (indexIt != arguments->end())
      {
        index = std::get<int>(indexIt->second);
      }

      int64_t ret = 0;
      if (index == VideoIndex::RGB)
      {
        ret = rgbTexture->textureId;
      }
      else if (index == VideoIndex::Depth)
      {
        ret = depthTexture->textureId;
      }
      else if (index == VideoIndex::IR)
      {
        ret = irTexture->textureId;
      }
      else if (index == VideoIndex::Camera2D)
      {
        ret = uvcTexture->textureId;
      }
      else if (index == VideoIndex::OPENGL)
      {
        ret = glfl->openglTexture->textureId;
      }

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("ni2Initialize") == 0)
    {
      int ret = ni2->init();
      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("ni2EnumerateDevices") == 0)
    {
      ni2->enumerateDevices();

      flutter::EncodableList flDeviceList = flutter::EncodableList();
      for (int i = 0; i < ni2->deviceList.getSize(); i++)
      {
        const DeviceInfo d = ni2->deviceList[i];
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
    else if (method_call.method_name().compare("ni2OpenDevice") == 0)
    {
      std::string uri;
      auto uriIt = arguments->find(flutter::EncodableValue("uri"));
      if (uriIt != arguments->end())
      {
        uri = std::get<std::string>(uriIt->second);
      }

      int videoMode;
      auto videoModeIt = arguments->find(flutter::EncodableValue("videoMode"));
      if (videoModeIt != arguments->end())
      {
        videoMode = std::get<int>(videoModeIt->second);
      }

      int ret = ni2->openDevice(uri.c_str(), videoMode);

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("ni2CloseDevice") == 0)
    {
      ni2->closeDevice();
      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("ni2DeviceIsConnected") == 0)
    {
      result->Success(flutter::EncodableValue(ni2->isConnected()));
    }
    else if (method_call.method_name().compare("ni2GetEnabledVideoModes") == 0)
    {
      result->Success(flutter::EncodableValue(ni2->getEnabledVideoModes()));
    }
    else if (method_call.method_name().compare("ni2ConfigVideoStream") == 0)
    {
      int videoMode;
      auto videoModeIt = arguments->find(flutter::EncodableValue("videoMode"));
      if (videoModeIt != arguments->end())
      {
        videoMode = std::get<int>(videoModeIt->second);
      }

      bool enable;
      auto enableIt = arguments->find(flutter::EncodableValue("enable"));
      if (enableIt != arguments->end())
      {
        enable = std::get<bool>(enableIt->second);
      }

      ni2->configVideoStream(videoMode, &enable);
      if (enable)
      {
        ni2->readVideoFeed();
      }
      result->Success(flutter::EncodableValue(enable));
    }
    else if (method_call.method_name().compare("ni2SetVideoSize") == 0)
    {
      int videoIndex;
      auto videoIndexIt = arguments->find(flutter::EncodableValue("videoIndex"));
      if (videoIndexIt != arguments->end())
      {
        videoIndex = std::get<int>(videoIndexIt->second);
      }

      int width;
      auto widthIt = arguments->find(flutter::EncodableValue("width"));
      if (widthIt != arguments->end())
      {
        width = std::get<int>(widthIt->second);
      }

      int height;
      auto heightIt = arguments->find(flutter::EncodableValue("height"));
      if (heightIt != arguments->end())
      {
        height = std::get<int>(heightIt->second);
      }

      // TODO: [Refactor] use Texture class
      if (videoIndex == VideoIndex::RGB)
      {
        rgbTexture->videoWidth = width;
        rgbTexture->videoHeight = height;
        rgbTexture->buffer.resize(width * height * 4);
      }
      else if (videoIndex == VideoIndex::Depth)
      {
        depthTexture->videoWidth = width;
        depthTexture->videoHeight = height;
        depthTexture->buffer.resize(width * height * 4);
      }
      else if (videoIndex == VideoIndex::IR)
      {
        irTexture->videoWidth = width;
        irTexture->videoHeight = height;
        irTexture->buffer.resize(width * height * 4);
      }
      else if (videoIndex == VideoIndex::IR)
      {
        uvcTexture->videoWidth = width;
        uvcTexture->videoHeight = height;
        uvcTexture->buffer.resize(width * height * 4);
      }

      result->Success(flutter::EncodableValue(nullptr));
    }else if(method_call.method_name().compare("rsGetTextureId") == 0)
    {
      std::string serial;
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if (serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      int videoModeIndex;
      auto videoModeIndexIt = arguments->find(flutter::EncodableValue("videoModeIndex"));
      if (videoModeIndexIt != arguments->end())
      {
        videoModeIndex = std::get<int>(videoModeIndexIt->second);
      }

      int ret = -1;
      RealsenseCam *cam = findRsCam(serial.c_str(), &rsCams);
      if (cam != nullptr)
      {
        ret = cam->getTextureId(videoModeIndex);
      }

      result->Success(flutter::EncodableValue(ret));
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
    else if (method_call.method_name().compare("rsOpenDevice") == 0)
    {
      std::string serial;
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if (serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      RealsenseCam *r = new RealsenseCam(serial.c_str());

      flutter::EncodableMap map = flutter::EncodableMap();
      int ret = r->openDevice();
      if (ret == 0)
      {
        r->fv_init(textureRegistrar, &models, flChannel, glfl);
        rsCams.push_back(r);
        map[flutter::EncodableValue("rgbTextureId")] = r->rgbTexture->textureId;
        map[flutter::EncodableValue("depthTextureId")] = r->depthTexture->textureId;
        map[flutter::EncodableValue("irTextureId")] = r->irTexture->textureId;
      }

      map[flutter::EncodableValue("ret")] = ret;
      result->Success(flutter::EncodableValue(map));
    }
    else if (method_call.method_name().compare("rsConfigVideoStream") == 0)
    {
      std::string serial;
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if (serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      int videoModeIndex;
      auto videoModeIndexIt = arguments->find(flutter::EncodableValue("videoModeIndex"));
      if (videoModeIndexIt != arguments->end())
      {
        videoModeIndex = std::get<int>(videoModeIndexIt->second);
      }

      bool enable = false;
      auto flEnable = arguments->find(flutter::EncodableValue("enable"));
      if (flEnable != arguments->end())
      {
        enable = std::get<bool>(flEnable->second);
      }

      int ret = -1;
      RealsenseCam *cam = findRsCam(serial.c_str(), &rsCams);
      if (cam != nullptr)
      {
        ret = cam->configVideoStream(videoModeIndex, enable);
        if (enable)
        {
          cam->readVideoFeed();
        }
      }

      result->Success(flutter::EncodableValue(ret == 0));
    }
    else if (method_call.method_name().compare("rsCloseDevice") == 0)
    {
      std::string serial;
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if (serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      int ret = -1;
      RealsenseCam *cam = findRsCam(serial.c_str(), &rsCams);
      if (cam != nullptr)
      {
        ret = cam->closeDevice();
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("rsEnablePointCloud") == 0)
    {
      std::string serial;
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if (serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      bool enable = false;
      auto flEnable = arguments->find(flutter::EncodableValue("enable"));
      if (flEnable != arguments->end())
      {
        enable = std::get<bool>(flEnable->second);
      }

      int ret = -1;
      RealsenseCam *cam = findRsCam(serial.c_str(), &rsCams);
      if (cam != nullptr)
      {
        ret = cam->enablePointCloud = enable;
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("rsDeviceIsConnected") == 0)
    {
      // TODO: implement
      result->Success(flutter::EncodableValue(true));
    }
    else if (method_call.method_name().compare("openglSetCamPosition") == 0)
    {
      double x, y, z;
      auto xIt = arguments->find(flutter::EncodableValue("x"));
      if (xIt != arguments->end())
        x = std::get<double>(xIt->second);
      auto yIt = arguments->find(flutter::EncodableValue("y"));
      if (yIt != arguments->end())
        y = std::get<double>(yIt->second);
      auto zIt = arguments->find(flutter::EncodableValue("z"));
      if (zIt != arguments->end())
        z = std::get<double>(zIt->second);

      glfl->setCamPosition(x, y, z);
      glfl->renderManually();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("openglSetCamAngle") == 0)
    {
      double yaw, pitch;
      auto yawIt = arguments->find(flutter::EncodableValue("yaw"));
      if (yawIt != arguments->end())
        yaw = std::get<double>(yawIt->second);

      auto pitchIt = arguments->find(flutter::EncodableValue("pitch"));
      if (pitchIt != arguments->end())
        pitch = std::get<double>(pitchIt->second);

      glfl->setYawPitch(yaw, pitch);
      glfl->renderManually();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("openglSetCamFov") == 0)
    {
      double fov;
      auto fovIt = arguments->find(flutter::EncodableValue("fov"));
      if (fovIt != arguments->end())
        fov = std::get<double>(fovIt->second);

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
    else if(method_call.method_name().compare("enablePointCloud") == 0)
    {
      bool enable = false;
      auto flEnable = arguments->find(flutter::EncodableValue("enable"));
      if (flEnable != arguments->end())
      {
        enable = std::get<bool>(flEnable->second);
      }

      ni2->enablePointCloud = enable;
      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("pipelineAdd") == 0)
    {
      int index = -1;
      auto flIndex = arguments->find(flutter::EncodableValue("index"));
      if (flIndex != arguments->end())
      {
        index = std::get<int>(flIndex->second);
      }

      int funcIndex = -1;
      auto flFuncIndex = arguments->find(flutter::EncodableValue("funcIndex"));
      if (flFuncIndex != arguments->end())
      {
        funcIndex = std::get<int>(flFuncIndex->second);
      }

      int len = 0;
      auto flLen = arguments->find(flutter::EncodableValue("len"));
      if (flLen != arguments->end())
      {
        len = std::get<int>(flLen->second);
      }

      std::vector<uint8_t> params{};
      if (len > 0)
      {
        auto flParams = arguments->find(flutter::EncodableValue("params"));
        if (flParams != arguments->end())
        {
          params = std::get<std::vector<uint8_t>>(flParams->second);
        }
      }

      int insertAt = -1;
      auto flAt = arguments->find(flutter::EncodableValue("at"));
      if (flAt != arguments->end())
      {
        insertAt = std::get<int>(flAt->second);
      }

      int interval = 0;
      auto flInterval = arguments->find(flutter::EncodableValue("interval"));
      if (flInterval != arguments->end())
      {
        interval = std::get<int>(flInterval->second);
      }

      if (index == VideoIndex::RGB)
      {
        rgbTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
      }
      else if (index == VideoIndex::Depth)
      {
        depthTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
      }
      else if (index == VideoIndex::IR)
      {
        irTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
      }
      else if (index == VideoIndex::Camera2D)
      {
        uvcTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
      }
      else if (index == 200) // Realsense RGB Pipeline
      {
        rsCams[0]->rgbTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
      }
      else if (index == 201) // Realsense RGB Pipeline
      {
        rsCams[0]->depthTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
      }
      else if (index == 202) // Realsense RGB Pipeline
      {
        rsCams[0]->irTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
      }
      else
      {
        pipelines[index-100]->add(funcIndex, params, len, insertAt, interval);
      }

      result->Success(flutter::EncodableValue(nullptr));
    } 
    else if (method_call.method_name().compare("pipelineCreate") == 0)
    {
      Pipeline *p = new Pipeline();
      pipelines.push_back(p);

      // TODO: Fix index
      int index = pipelines.size()-1+100;
      result->Success(flutter::EncodableValue(index));
    } 
    else if (method_call.method_name().compare("pipelineRun") == 0)
    {
      int index = -1;
      auto flIndex = arguments->find(flutter::EncodableValue("index"));
      if (flIndex != arguments->end())
      {
        // TODO: Fix index
        index = std::get<int>(flIndex->second) - 100;
      }

      std::cout << "PipelineRun:" << index << std::endl;

      // TODO: normal pipline should have their own texture.
      pipelines[index]->runOnce(textureRegistrar, rgbTexture->textureId, rgbTexture->videoWidth, rgbTexture->videoHeight, rgbTexture->buffer, &models, flChannel);
      rgbTexture->setPixelBuffer();

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("pipelineClear") == 0)
    {
      int index = -1;
      auto flIndex = arguments->find(flutter::EncodableValue("index"));
      if (flIndex != arguments->end())
      {
        index = std::get<int>(flIndex->second);
      }

      if (index == VideoIndex::RGB)
      {
        rgbTexture->pipeline->clear();
      }
      else if (index == VideoIndex::Depth)
      {
        depthTexture->pipeline->clear();
      }
      else if (index == VideoIndex::IR)
      {
        irTexture->pipeline->clear();
      }
      else if (index == VideoIndex::Camera2D)
      {
        uvcTexture->pipeline->clear();
      }
      else if (index >= 100 && index < 200) // Normal Pipeline
      {
        pipelines[index - 100]->clear();
      }
      else if (index == 200) // Realsense RGB Pipeline
      {
        rsCams[0]->rgbTexture->pipeline->clear();
      }
      else if (index == 201) // Realsense RGB Pipeline
      {
        rsCams[0]->depthTexture->pipeline->clear();
      }
      else if (index == 202) // Realsense RGB Pipeline
      {
        rsCams[0]->irTexture->pipeline->clear();
      }

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("cameraOpen") == 0)
    {
      int index = 0;
      auto flIndex = arguments->find(flutter::EncodableValue("index"));
      if (flIndex != arguments->end())
      {
        index = std::get<int>(flIndex->second);
      }

      bool newCam = true;
      bool ret = false;
      for (int i = 0; i < cameras.size(); i++)
      {
        if (cameras[i]->capIndex == index)
        {
          newCam = false;
          if (!cameras[i]->cap->isOpened())
          {
            cameras[i]->cap->open(index);
          }

          ret = true;
          break;
        }
      }

      if (newCam)
      {
        OpenCVCamera *c = new OpenCVCamera(index, uvcTexture.get(), textureRegistrar, &models, flChannel);
        c->open();
        cameras.push_back(c);
        ret = c->cap->isOpened();
      }

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("uvcConfig") == 0)
    {
      int index = 0;
      auto flIndex = arguments->find(flutter::EncodableValue("index"));
      if (flIndex != arguments->end())
      {
        index = std::get<int>(flIndex->second);
      }

      int prop = 0;
      auto flProp = arguments->find(flutter::EncodableValue("prop"));
      if (flProp != arguments->end())
      {
        prop = std::get<int>(flProp->second);
      }

      float value = 0.0f;
      auto flValue = arguments->find(flutter::EncodableValue("value"));
      if(flValue != arguments->end())
      {
        value = std::get<double>(flValue->second);
      }

      bool ret = false;
      for (int i = 0; i < cameras.size(); i++)
      {
        if (cameras[i]->capIndex == index)
        {
          cameras[i]->config(prop, value);
          ret = true;
          break;
        }
      }

      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("cameraConfig") == 0)
    {
      int index = 0;
      auto flIndex = arguments->find(flutter::EncodableValue("index"));
      if (flIndex != arguments->end())
      {
        index = std::get<int>(flIndex->second);
      }

      bool start = false;
      auto flStart = arguments->find(flutter::EncodableValue("start"));
      if (flStart != arguments->end())
      {
        start = std::get<bool>(flStart->second);
      }

      bool ret = false;
      for (int i = 0; i < cameras.size(); i++)
      {
        if (cameras[i]->capIndex == index)
        {
          if (start)
          {
            cameras[i]->start();
          }
          else
          {
            cameras[i]->stop();
          }

          ret = true;
          break;
        }
      }
      result->Success(flutter::EncodableValue(ret));
    }
    else if (method_call.method_name().compare("tfliteCreateModel") == 0)
    {
      std::string path;
      auto pathIt = arguments->find(flutter::EncodableValue("modelPath"));
      if (pathIt != arguments->end())
      {
        path = std::get<std::string>(pathIt->second);
      }

      printf("Path:%s\n", path.c_str());

      TFLiteModel *m = new TFLiteModel(path.c_str());
      this->models.push_back(m);

      result->Success(flutter::EncodableValue(nullptr));
    }
    else if (method_call.method_name().compare("tfliteGetModelInfo") == 0)
    {
      int index = 0;
      auto flIndex = arguments->find(flutter::EncodableValue("index"));
      if (flIndex != arguments->end())
      {
        index = std::get<int>(flIndex->second);
      }

      TFLiteModel *m = this->models[index];

      flutter::EncodableMap map = flutter::EncodableMap();
      map[flutter::EncodableValue("valid")] = m->valid;
      map[flutter::EncodableValue("error")] = m->error.c_str();

      result->Success(flutter::EncodableValue(map));
    }
    else if (method_call.method_name().compare("tfliteGetTensorOutput") == 0)
    {
      int index = 0;
      auto flIndex = arguments->find(flutter::EncodableValue("tensorIndex"));
      if (flIndex != arguments->end())
      {
        index = std::get<int>(flIndex->second);
      }

      std::vector<int32_t> size;
      auto flSize = arguments->find(flutter::EncodableValue("size"));
      if (flSize != arguments->end())
      {
        size = std::get<std::vector<int32_t>>(flSize->second);
      }

      int outputSize = 1;
      for (int i = 0; i < size.size(); i++)
        outputSize *= size[i];
      float *data = new float[outputSize];


      models[0]->retrieveOutput<float>(index, outputSize, data);

      flutter::EncodableList fl = flutter::EncodableList();
      for (int i = 0; i < outputSize; i++){
        fl.push_back(*(data + i));
      }

      result->Success(fl);
    }
    else if (method_call.method_name().compare("_float2uint8") == 0)
    {
      float v = 0.0f;
      auto flV = arguments->find(flutter::EncodableValue("value"));
      if (flV != arguments->end())
      {
        v = std::get<double>(flV->second);
      }

      flutter::EncodableList fl = flutter::EncodableList();
      uint8_t *bytes = reinterpret_cast<uint8_t *>(&v);
      for (int i = 0; i < 4; i++)
      {
        fl.push_back(flutter::EncodableValue(*(bytes + i)));
      }
      result->Success(fl);
    }
    else if (method_call.method_name().compare("screenshot") == 0)
    {
      int index = 0;
      auto flIndex = arguments->find(flutter::EncodableValue("index"));
      if (flIndex != arguments->end())
      {
        index = std::get<int>(flIndex->second);
      }

      std::string path;
      auto flPath = arguments->find(flutter::EncodableValue("path"));
      if (flPath != arguments->end())
      {
        path = std::get<std::string>(flPath->second);
      }

      int cvtCode = 0;
      auto flCvtCode = arguments->find(flutter::EncodableValue("cvtCode"));
      if (flCvtCode != arguments->end())
      {
        cvtCode = std::get<int>(flCvtCode->second);
      }

      bool ret = false;
      cv::Mat frame;
      if (index == VideoIndex::RGB)
      {
        rgbTexture->pipeline->screenshot(path.c_str(), cvtCode);
      }
      else if (index == VideoIndex::Depth)
      {
        depthTexture->pipeline->screenshot(path.c_str(), cvtCode);
      }
      else if (index == VideoIndex::IR)
      {
        irTexture->pipeline->screenshot(path.c_str(), cvtCode);
      }
      else if (index == VideoIndex::Camera2D)
      {
        uvcTexture->pipeline->screenshot(path.c_str(), cvtCode);
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

      rgbTexture->pipeline->run(b, textureRegistrar, rgbTexture->textureId, rgbTexture->videoWidth, rgbTexture->videoHeight, rgbTexture->buffer, &models, flChannel);
      rgbTexture->setPixelBuffer();
      irTexture->pipeline->run(g, textureRegistrar, irTexture->textureId, irTexture->videoWidth, irTexture->videoHeight, irTexture->buffer, &models, flChannel);
      irTexture->setPixelBuffer();
      depthTexture->pipeline->run(r, textureRegistrar, depthTexture->textureId, depthTexture->videoWidth, depthTexture->videoHeight, depthTexture->buffer, &models, flChannel);
      depthTexture->setPixelBuffer();
      uvcTexture->pipeline->run(b, textureRegistrar, uvcTexture->textureId, uvcTexture->videoWidth, uvcTexture->videoHeight, uvcTexture->buffer, &models, flChannel);
      uvcTexture->setPixelBuffer();

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
