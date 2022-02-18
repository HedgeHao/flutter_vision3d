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

#include "include/flutter_vision/rgb_texture.hpp"
#include "include/flutter_vision/depth_texture.hpp"
#include "include/flutter_vision/ir_texture.hpp"
#include "include/flutter_vision/openni2_wrapper.hpp"

#include "include/flutter_vision/opengl/opengl.h"

namespace
{

  class FlutterVisionPlugin : public flutter::Plugin
  {
  public:
    flutter::MethodChannel<flutter::EncodableValue> *flChannel;
    OpenNi2Wrapper *ni2 = new OpenNi2Wrapper();
    OpenGLFL *glfl;

    static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

    FlutterVisionPlugin(flutter::TextureRegistrar *texture_registrar);

    virtual ~FlutterVisionPlugin();

  private:
    // Called when a method is called on this plugin's channel from Dart.
    void HandleMethodCall(
        const flutter::MethodCall<flutter::EncodableValue> &method_call,
        std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    flutter::TextureRegistrar *textureRegistrar;
    std::unique_ptr<RgbTexture> rgbTexture;
    std::unique_ptr<DepthTexture> depthTexture;
    std::unique_ptr<IrTexture> irTexture;
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
    rgbTexture = std::make_unique<RgbTexture>(textureRegistrar);
    depthTexture = std::make_unique<DepthTexture>(textureRegistrar);
    irTexture = std::make_unique<IrTexture>(textureRegistrar);
    glfl = new OpenGLFL(textureRegistrar);

    ni2->registerFlContext(rgbTexture.get(), depthTexture.get(), irTexture.get(), glfl);

    rgbTexture->stream = &ni2->vsColor;
    depthTexture->stream = &ni2->vsDepth;
    irTexture->stream = &ni2->vsIR;
  }

  FlutterVisionPlugin::~FlutterVisionPlugin() {}

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

      result->Success(flutter::EncodableValue(nullptr));
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
    else if (method_call.method_name().compare("test") == 0)
    {
      // rgbTexture->genPixels();
      // depthTexture->genPixels();
      // irTexture->genPixels();

      // ni2->test();

      glfl->render();
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
