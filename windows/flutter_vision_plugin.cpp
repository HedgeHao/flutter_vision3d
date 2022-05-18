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

#include "include/flutter_vision/opengl/opengl.h"


#define PIPELINE_INDEX_TFLITE 8

cv::Mat emptyMat = cv::Mat::zeros(3, 3, CV_64F);

enum CameraType
{
  OPENNI = 0,
  REALSENSE = 1,
  DUMMY = 2,
};

namespace
{
  class FlutterVisionPlugin : public flutter::Plugin
  {
  public:
    flutter::MethodChannel<flutter::EncodableValue> *flChannel;
    OpenGLFL *glfl;

    // TfPipeline *tfPipeline;
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
      std::string uri;
      auto uriIt = arguments->find(flutter::EncodableValue("serial"));
      if (uriIt != arguments->end())
      {
        uri = std::get<std::string>(uriIt->second);
      }

      int cameraType;
      auto cameraTypeIt = arguments->find(flutter::EncodableValue("cameraType"));
      if (cameraTypeIt != arguments->end())
      {
        cameraType = std::get<int>(cameraTypeIt->second);
      }

      flutter::EncodableMap map = flutter::EncodableMap();

      FvCamera *cam;
      if(cameraType == CameraType::OPENNI){
        cam = new OpenniCam(uri.c_str());
      } else if(cameraType == CameraType::REALSENSE){
        cam = new RealsenseCam(uri.c_str());
      } else if(cameraType == CameraType::DUMMY){
        cam = new DummyCam(uri.c_str());
      }

      int ret = -1;
      if(cam){
        cam->fvInit(textureRegistrar, &models, flChannel, glfl);
        cam->camInit();
         
        ret = cam->openDevice();
        if(ret == 0){
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
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if(serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      int ret = -1;
      if (cam != nullptr){
         ret = cam->closeDevice();
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
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if (serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      int cameraType;
      auto cameraTypeIt = arguments->find(flutter::EncodableValue("cameraType"));
      if (cameraTypeIt != arguments->end())
      {
        cameraType = std::get<int>(cameraTypeIt->second);
      }

      int videoModeIndex;
      auto videoModeIndexIt = arguments->find(flutter::EncodableValue("videoModeIndex"));
      if (videoModeIndexIt != arguments->end())
      {
        videoModeIndex = std::get<int>(videoModeIndexIt->second);
      }

      bool enable;
      auto enableIt = arguments->find(flutter::EncodableValue("enable"));
      if (enableIt != arguments->end())
      {
        enable = std::get<bool>(enableIt->second);
      }

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);   
      if(cam)
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

     
      if (videoIndex == VideoIndex::IR)
      {
        uvcTexture->videoWidth = width;
        uvcTexture->videoHeight = height;
        uvcTexture->buffer.resize(width * height * 4);
      }else{
        // TODO: set video size for camera
      }

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
    else if(method_call.method_name().compare("getOpenglTextureId") == 0)
    {
      result->Success(flutter::EncodableValue(glfl->openglTexture->textureId));
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
    else if(method_call.method_name().compare("fvCameraEnablePointCloud") == 0)
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

      std::string serial;
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if(serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams); 
      if(cam)
      {
        if (index == VideoIndex::RGB)
        {
          cam->rgbTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
        }
        else if (index == VideoIndex::Depth)
        {
          cam->depthTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
        }
        else if (index == VideoIndex::IR)
        {
          cam->irTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
        }
      }
      // TODO: handle UVC Pipeline
      else if (index == VideoIndex::Camera2D)
      {
        uvcTexture->pipeline->add(funcIndex, params, len, insertAt, interval);
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
        index = std::get<int>(flIndex->second);
      }

      std::string serial;
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if(serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams);
      if(cam)
      {
        if(index == VideoIndex::RGB)
        {
          cam->rgbTexture->pipeline->runOnce(textureRegistrar, cam->rgbTexture->textureId, cam->rgbTexture->videoWidth, cam->rgbTexture->videoHeight, cam->rgbTexture->buffer, &models, flChannel);;
          cam->rgbTexture->setPixelBuffer();
        }
        else if(index == VideoIndex::Depth)
        {
          cam->depthTexture->pipeline->runOnce(textureRegistrar, cam->depthTexture->textureId, cam->depthTexture->videoWidth, cam->depthTexture->videoHeight, cam->depthTexture->buffer, &models, flChannel);;
          cam->depthTexture->setPixelBuffer();
        }
         else if(index == VideoIndex::IR)
        {
          cam->irTexture->pipeline->runOnce(textureRegistrar, cam->irTexture->textureId, cam->irTexture->videoWidth, cam->irTexture->videoHeight, cam->irTexture->buffer, &models, flChannel);;
          cam->irTexture->setPixelBuffer();
        }
      }

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

      std::string serial;
      auto serialIt = arguments->find(flutter::EncodableValue("serial"));
      if(serialIt != arguments->end())
      {
        serial = std::get<std::string>(serialIt->second);
      }

      FvCamera *cam = FvCamera::findCam(serial.c_str(), &cams); 
      if(cam)
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
      // TODO: handle UVC pipeline
      else if (index == VideoIndex::Camera2D)
      {
        uvcTexture->pipeline->clear();
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
      if (index == VideoIndex::Camera2D)
      {
        uvcTexture->pipeline->screenshot(path.c_str(), cvtCode);
      } else{
        // TODO: Implement for FvCamera
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
