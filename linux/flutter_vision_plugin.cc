#include "include/flutter_vision/flutter_vision_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include "include/flutter_vision/pipeline/pipeline.h"
#include "include/flutter_vision/tflite.h"

#include "include/flutter_vision/camera/realsense.h"
#include "include/flutter_vision/camera/openni2.h"
#include "include/flutter_vision/camera/dummy.h"
#include "include/flutter_vision/camera/uvc.h"
#include "include/flutter_vision/fv_texture.h"

#include <cstring>
#include <memory>

#define FL_ARG_STRING(args, name) fl_value_get_string(fl_value_lookup_string(args, name))
#define FL_ARG_INT(args, name) fl_value_get_int(fl_value_lookup_string(args, name))
#define FL_ARG_FLOAT(args, name) fl_value_get_float(fl_value_lookup_string(args, name))
#define FL_ARG_BOOL(args, name) fl_value_get_bool(fl_value_lookup_string(args, name))
#define FL_ARG_INT32_LIST(args, name) fl_value_get_int32_list(fl_value_lookup_string(args, name))
#define FL_ARG_FLOAT_LIST(args, name) fl_value_get_float_list(fl_value_lookup_string(args, name))

#define FLUTTER_VISION_PLUGIN(obj)                                     \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), flutter_vision_plugin_get_type(), \
                              FlutterVisionPlugin))

enum CameraType
{
  OPENNI = 0,
  REALSENSE = 1,
  DUMMY = 2,
  UVC = 3,
};

RealsenseCam *findRsCam(const char *serial, std::vector<RealsenseCam *> *cams)
{
  for (auto c : *cams)
  {
    if (strcmp(c->serial.c_str(), serial) == 0)
    {
      return c;
    }
  }

  return nullptr;
}

struct _FlutterVisionPlugin
{
  GObject parent_instance;

  FlTextureRegistrar *texture_registrar;
  OpenGLTexture *openglTexture;

  FlMethodChannel *flChannel;
  FlView *flView;
  OpenGLFL *glfl;

  std::vector<TFLiteModel *> models{};
  std::vector<Pipeline *> pipelines{};
  std::vector<FvCamera *> cams{};
};

G_DEFINE_TYPE(FlutterVisionPlugin, flutter_vision_plugin, g_object_get_type())

// Called when a method call is received from Flutter.
static void flutter_vision_plugin_handle_method_call(
    FlutterVisionPlugin *self,
    FlMethodCall *method_call)
{
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar *method = fl_method_call_get_name(method_call);
  FlValue *args = fl_method_call_get_args(method_call);

  if (strcmp(method, "ni2Initialize") == 0)
  {
    int ret = OpenniCam::openniInit();
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(ret)));
  }
  else if (strcmp(method, "ni2EnumerateDevices") == 0)
  {
#ifdef DISABLE_OPENNI
    response = FL_METHOD_RESPONSE(fl_method_error_response_new("NOT SUPPORT", "NOT SUPPORT", nullptr));
#else
    Array<DeviceInfo> devs;
    OpenniCam::enumerateDevices(&devs);

    auto flDeviceList = fl_value_new_list();
    for (int i = 0; i < devs.getSize(); i++)
    {
      const DeviceInfo d = devs[i];
      auto m = fl_value_new_map();
      fl_value_set(m, fl_value_new_string("name"), fl_value_new_string(d.getName()));
      fl_value_set(m, fl_value_new_string("uri"), fl_value_new_string(d.getUri()));
      fl_value_set(m, fl_value_new_string("productId"), fl_value_new_int(d.getUsbProductId()));
      fl_value_set(m, fl_value_new_string("vendorId"), fl_value_new_int(d.getUsbVendorId()));
      fl_value_set(m, fl_value_new_string("vendor"), fl_value_new_string(d.getVendor()));
      fl_value_append_take(flDeviceList, m);
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(flDeviceList));
#endif
  }
  else if (strcmp(method, "ni2GetAvailableVideoModes") == 0)
  {
#ifdef DISABLE_OPENNI
    response = FL_METHOD_RESPONSE(fl_method_error_response_new("NOT SUPPORT", "NOT SUPPORT", nullptr));
#else
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");

    auto flList = fl_value_new_list();

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    if (cam != nullptr)
    {
      std::vector<std::string> modes;
      cam->getAvailableVideoModes(index, modes);

      for (auto s : modes)
      {
        fl_value_append_take(flList, fl_value_new_string(s.c_str()));
      }
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(flList));
#endif
  }
  else if (strcmp(method, "ni2GetCurrentVideoMode") == 0)
  {
#ifdef DISABLE_OPENNI
    response = FL_METHOD_RESPONSE(fl_method_error_response_new("NOT SUPPORT", "NOT SUPPORT", nullptr));
#else
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");

    std::string mode = "";

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    if (cam != nullptr)
    {
      cam->getCurrentVideoMode(index, mode);
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_string(mode.c_str())));
#endif
  }
  else if (strcmp(method, "ni2SetVideoMode") == 0)
  {
#ifdef DISABLE_OPENNI
    response = FL_METHOD_RESPONSE(fl_method_error_response_new("NOT SUPPORT", "NOT SUPPORT", nullptr));
#else
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");
    const int mode = FL_ARG_INT(args, "mode");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    bool ret = false;
    if (cam != nullptr)
    {
      cam->setVideoMode(index, mode);

      ret = true;
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(ret)));
#endif
  }
  else if (strcmp(method, "fvCameraOpen") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int cameraType = FL_ARG_INT(args, "cameraType");

    auto m = fl_value_new_map();
    FvCamera *cam = nullptr;
    if (cameraType == CameraType::OPENNI)
    {
      cam = new OpenniCam(serial);
    }
    else if (cameraType == CameraType::REALSENSE)
    {
      cam = new RealsenseCam(serial);
    }
    else if (cameraType == CameraType::DUMMY)
    {
      cam = new DummyCam(serial);
    }
    else if (cameraType == CameraType::UVC)
    {
      cam = new UvcCam(serial);
    }

    cam->type = cameraType;

    int ret = -1;
    if (cam != nullptr)
    {
      cam->fvInit(self->texture_registrar, &self->models, self->flChannel, self->glfl);
      cam->camInit();

      ret = cam->openDevice();
      if (ret == 0)
      {
        self->cams.push_back(cam);
        fl_value_set(m, fl_value_new_string("rgbTextureId"), fl_value_new_int(cam->rgbTexture->textureId));
        fl_value_set(m, fl_value_new_string("depthTextureId"), fl_value_new_int(cam->depthTexture->textureId));
        fl_value_set(m, fl_value_new_string("irTextureId"), fl_value_new_int(cam->irTexture->textureId));
      }
    }

    fl_value_set(m, fl_value_new_string("ret"), fl_value_new_int(ret));
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(m));
  }
  else if (strcmp(method, "fvCameraClose") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    int ret = -1;
    if (cam != nullptr)
    {
      ret = cam->closeDevice();

      FvCamera::removeCam(serial, &self->cams);
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "fvCameraIsConnected") == 0)
  {
    // TODO: not implement
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(true)));
  }
  else if (strcmp(method, "fvCameraConfigVideoStream") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    // int cameraType = FL_ARG_INT(args, "cameraType");
    int videoModeIndex = FL_ARG_INT(args, "videoModeIndex");
    bool enable = FL_ARG_BOOL(args, "enable");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    int ret = -1;
    if (cam)
    {
      ret = cam->configVideoStream(videoModeIndex, &enable);
      if (enable)
      {
        cam->readVideoFeed();
      }
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(ret)));
  }
  else if (strcmp(method, "fvGetOpenCVMat") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");

    int64_t pointer = 0;
    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    if (cam)
    {
      pointer = cam->getOpenCVMat(index);
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(pointer)));
  }
  else if (strcmp(method, "fvPauseStream") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const bool pause = FL_ARG_BOOL(args, "pause");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    bool ret = false;
    if (cam)
    {
      cam->pause(pause);
      ret = true;
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(ret)));
  }
  else if (strcmp(method, "fvGetSerialNumber") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    bool ret = false;
    std::string sn = "";
    if (cam)
    {
      ret = cam->getSerialNumber(sn);
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_string(sn.c_str())));
  }
  else if (strcmp(method, "ni2SetVideoSize") == 0)
  {
    // const int index = FL_ARG_INT(args, "videoIndex");
    // const int width = FL_ARG_INT(args, "width");
    // const int height = FL_ARG_INT(args, "height");

    // TODO: not implement

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "rsEnumerateDevices") == 0)
  {
#ifdef DISABLE_REALSENSE
    response = FL_METHOD_RESPONSE(fl_method_error_response_new("NOT SUPPORT", "NOT SUPPORT", nullptr));
#else
    std::vector<std::string> serials = RealsenseHelper::enumerateDevices();

    auto list = fl_value_new_list();
    for (auto s : serials)
    {
      fl_value_append_take(list, fl_value_new_string(s.c_str()));
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(list));
#endif
  }
  else if (strcmp(method, "getOpenglTextureId") == 0)
  {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(OPENGL_TEXTURE_GET_CLASS(self->openglTexture)->texture_id)));
  }
  else if (strcmp(method, "openglSetCamPosition") == 0)
  {
    const float x = FL_ARG_FLOAT(args, "x");
    const float y = FL_ARG_FLOAT(args, "y");
    const float z = FL_ARG_FLOAT(args, "z");

    self->glfl->setCamPosition(x, y, z);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "openglSetCamAngle") == 0)
  {
    const float yaw = FL_ARG_FLOAT(args, "yaw");
    const float pitch = FL_ARG_FLOAT(args, "pitch");

    self->glfl->setYawPitch(yaw, pitch);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "openglSetCamFov") == 0)
  {
    const float fov = FL_ARG_FLOAT(args, "fov");

    self->glfl->setFov(fov);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "openglRender") == 0)
  {
    self->glfl->render();
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "fvCameraEnablePointCloud") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const bool enable = FL_ARG_BOOL(args, "enable");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    int ret = -1;
    if (cam != nullptr)
    {
      ret = cam->enablePointCloud = enable;
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "pipelineAdd") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    int index = FL_ARG_INT(args, "index");
    int funcIndex = FL_ARG_INT(args, "funcIndex");
    int len = FL_ARG_INT(args, "len");

    FlValue *valueParams = fl_value_lookup_string(args, "params");
    const uint8_t *params;
    if (valueParams != nullptr && fl_value_get_type(valueParams) == FL_VALUE_TYPE_NULL)
    {
      params = NULL;
    }
    else
    {
      params = fl_value_get_uint8_list(valueParams);
    }

    int insertAt = -1;
    FlValue *valueAt = fl_value_lookup_string(args, "at");
    if (valueAt != nullptr && fl_value_get_type(valueAt) != FL_VALUE_TYPE_NULL)
      insertAt = fl_value_get_int(valueAt);

    int interval = 0;
    FlValue *valueInterval = fl_value_lookup_string(args, "interval");
    if (valueInterval != nullptr && fl_value_get_type(valueInterval) != FL_VALUE_TYPE_NULL)
      interval = fl_value_get_int(valueInterval);

    bool append = false;
    FlValue *valueAppend = fl_value_lookup_string(args, "append");
    if (valueAppend != nullptr && fl_value_get_type(valueAppend) != FL_VALUE_TYPE_NULL)
      append = fl_value_get_bool(valueAppend);

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
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

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "pipelineRun") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");

    int from = 0;
    FlValue *valueFrom = fl_value_lookup_string(args, "from");
    if (valueFrom != nullptr && fl_value_get_type(valueFrom) != FL_VALUE_TYPE_NULL)
      from = fl_value_get_int(valueFrom);

    int to = -1;
    FlValue *valueTo = fl_value_lookup_string(args, "to");
    if (valueTo != nullptr && fl_value_get_type(valueTo) != FL_VALUE_TYPE_NULL)
      to = fl_value_get_int(valueTo);

    int ret = 0;
    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    if (cam)
    {
      if (index == VideoIndex::RGB)
      {
        ret = cam->rgbTexture->pipeline->runOnce(*self->texture_registrar, *FL_TEXTURE(cam->rgbTexture), cam->rgbTexture->video_width, cam->rgbTexture->video_height, cam->rgbTexture->buffer, &self->models, self->flChannel, from, to);
      }
      else if (index == VideoIndex::Depth)
      {
        ret = cam->depthTexture->pipeline->runOnce(*self->texture_registrar, *FL_TEXTURE(cam->depthTexture), cam->depthTexture->video_width, cam->depthTexture->video_height, cam->depthTexture->buffer, &self->models, self->flChannel, from, to);
      }
      else if (index == VideoIndex::IR)
      {
        ret = cam->irTexture->pipeline->runOnce(*self->texture_registrar, *FL_TEXTURE(cam->irTexture), cam->irTexture->video_width, cam->irTexture->video_height, cam->irTexture->buffer, &self->models, self->flChannel, from, to);
      }
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(ret)));
  }
  else if (strcmp(method, "pipelineClear") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
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

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "pipelineInfo") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");

    std::string info;
    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
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

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_string(info.c_str())));
  }
  else if (strcmp(method, "pipelineError") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");

    std::string error;
    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
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

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_string(error.c_str())));
  }
  else if (strcmp(method, "fvCameraConfig") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int prop = FL_ARG_INT(args, "prop");
    FlValue *flValue = fl_value_lookup_string(args, "value");
    const float *value = fl_value_get_float32_list(flValue);
    const int len = fl_value_get_length(flValue);

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    int ret = -1;
    if (cam)
    {
      std::vector<float> values(value, value + len);
      cam->configure(prop, values);
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(ret == 0)));
  }
  else if (strcmp(method, "fvCameraGetConfiguration") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int prop = FL_ARG_INT(args, "prop");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    int ret = -1;
    if (cam)
    {
      ret = cam->getConfiguration(prop);
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(ret)));
  }
  else if (strcmp(method, "rsLoadPresetParameters") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const char *path = FL_ARG_STRING(args, "path");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    if (cam)
    {
      std::string pathStr = std::string(serial);
      cam->loadPresetParameters(pathStr);
    }
  }
  else if (strcmp(method, "fvGetIntrinsic") == 0)
  {

    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");

    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    double fx = 0.0, fy = 0.0, cx = 0.0, cy = 0.0;

    FlValue *map = fl_value_new_map();
    if (cam)
    {
      cam->getIntrinsic(index, fx, fy, cx, cy);
    }

    fl_value_set(map, fl_value_new_string("fx"), fl_value_new_float(fx));
    fl_value_set(map, fl_value_new_string("fy"), fl_value_new_float(fy));
    fl_value_set(map, fl_value_new_string("cx"), fl_value_new_float(cx));
    fl_value_set(map, fl_value_new_string("cy"), fl_value_new_float(cy));

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(map));
  }
  else if (strcmp(method, "tfliteCreateModel") == 0)
  {
    const char *path = FL_ARG_STRING(args, "modelPath");

    TFLiteModel *m = new TFLiteModel(path);
    self->models.push_back(m);

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "tfliteGetModelInfo") == 0)
  {
    const int index = FL_ARG_INT(args, "index");

    TFLiteModel *m = self->models[index];

    FlValue *modelMap = fl_value_new_map();

    if (index < self->models.size())
    {
      fl_value_set(modelMap, fl_value_new_string("valid"), fl_value_new_bool(m->valid));
      fl_value_set(modelMap, fl_value_new_string("error"), fl_value_new_string(m->error.c_str()));
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(modelMap));
  }
  else if (strcmp(method, "tfliteGetTensorOutput") == 0)
  {
    const int index = FL_ARG_INT(args, "tensorIndex");
    FlValue *valueSize = fl_value_lookup_string(args, "size");
    const int32_t *size = fl_value_get_int32_list(valueSize);
    const int len = fl_value_get_length(valueSize);

    int outputSize = 1;
    for (int i = 0; i < len; i++)
      outputSize *= *(size + i);
    float *data = new float[outputSize];

    self->models[0]->retrieveOutput<float>(index, outputSize, data);

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_float32_list(data, outputSize)));
  }
  else if (strcmp(method, "_float2uint8") == 0)
  {
    float f = FL_ARG_FLOAT(args, "value");
    uint8_t *bytes = reinterpret_cast<uint8_t *>(&f);

    FlValue *result = fl_value_new_uint8_list(bytes, 4);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  else if (strcmp(method, "fvCameraScreenshot") == 0)
  {
    const char *serial = FL_ARG_STRING(args, "serial");
    const int index = FL_ARG_INT(args, "index");
    const char *path = FL_ARG_STRING(args, "path");
    const int cvtCode = FL_ARG_INT(args, "cvtCode");

    bool ret = false;
    FvCamera *cam = FvCamera::findCam(serial, &self->cams);
    if (cam != nullptr)
    {
      if (index == VideoIndex::RGB)
      {
        cam->rgbTexture->pipeline->screenshot(path, cvtCode);
      }
      else if (index == VideoIndex::Depth)
      {
        cam->depthTexture->pipeline->screenshot(path, cvtCode);
      }
      else if (index == VideoIndex::IR)
      {
        cam->irTexture->pipeline->screenshot(path, cvtCode);
      }

      ret = true;
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(ret)));
  }
  else
  {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  fl_method_call_respond(method_call, response, nullptr);
}

static void flutter_vision_plugin_dispose(GObject *object)
{
  G_OBJECT_CLASS(flutter_vision_plugin_parent_class)->dispose(object);
}

static void flutter_vision_plugin_class_init(FlutterVisionPluginClass *klass)
{
  G_OBJECT_CLASS(klass)->dispose = flutter_vision_plugin_dispose;
}

static void flutter_vision_plugin_init(FlutterVisionPlugin *self) {}

static void method_call_cb(FlMethodChannel *channel, FlMethodCall *method_call,
                           gpointer user_data)
{
  FlutterVisionPlugin *plugin = FLUTTER_VISION_PLUGIN(user_data);
  flutter_vision_plugin_handle_method_call(plugin, method_call);
}

void flutter_vision_plugin_register_with_registrar(FlPluginRegistrar *registrar)
{
  FlutterVisionPlugin *plugin = FLUTTER_VISION_PLUGIN(
      g_object_new(flutter_vision_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(FlMethodChannel) channel =
      fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                            "flutter_vision",
                            FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(channel, method_call_cb,
                                            g_object_ref(plugin),
                                            g_object_unref);
  plugin->flChannel = channel;

  plugin->flView = fl_plugin_registrar_get_view(registrar);

  plugin->texture_registrar = fl_plugin_registrar_get_texture_registrar(registrar);

  plugin->openglTexture = OPENGL_TEXTURE(g_object_new(opengl_texture_get_type(), nullptr));
  OPENGL_TEXTURE_GET_CLASS(plugin->openglTexture)->video_width = 1280;
  OPENGL_TEXTURE_GET_CLASS(plugin->openglTexture)->video_height = 720;
  FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(plugin->openglTexture)->copy_pixels = opengl_texture_copy_pixels;
  fl_texture_registrar_register_texture(plugin->texture_registrar, FL_TEXTURE(plugin->openglTexture));
  OPENGL_TEXTURE_GET_CLASS(plugin->openglTexture)->texture_id = reinterpret_cast<int64_t>(FL_TEXTURE(plugin->openglTexture));
  fl_texture_registrar_mark_texture_frame_available(plugin->texture_registrar, FL_TEXTURE(plugin->openglTexture));
  // TODO: glfl and glTexture depend on each other
  plugin->glfl = new OpenGLFL(gtk_widget_get_parent_window(GTK_WIDGET(plugin->flView)), plugin->texture_registrar, plugin->openglTexture);
  OPENGL_TEXTURE_GET_CLASS(plugin->openglTexture)->buffer = plugin->glfl->pixelBuffer;

  g_object_unref(plugin);
}