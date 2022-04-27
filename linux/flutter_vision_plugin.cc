#include "include/flutter_vision/flutter_vision_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include "include/flutter_vision/pipeline/pipeline.h"
#include "include/flutter_vision/pipeline/tf_pipeline.h"
#include "include/flutter_vision/tflite.h"

#include "include/flutter_vision/openni2_wrapper.hpp"
#include "include/flutter_vision/uvc_texture.hpp"

#include <cstring>
#include <memory>

#define PIPELINE_INDEX_TFLITE 8

#define FLUTTER_VISION_PLUGIN(obj)                                     \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), flutter_vision_plugin_get_type(), \
                              FlutterVisionPlugin))

cv::Mat emptyMat = cv::Mat::zeros(3, 3, CV_64F);

struct _FlutterVisionPlugin
{
  GObject parent_instance;
  OpenNi2Wrapper *ni2;

  FlTextureRegistrar *texture_registrar;
  RgbTexture *rgbTexture;
  DepthTexture *depthTexture;
  IrTexture *irTexture;
  UvcTexture *uvcTexture;
  OpenGLTexture *openglTexture;

  FlView *flView;
  OpenGLFL *glfl;

  TfPipeline *tfPipeline;
  std::vector<TFLiteModel *> models{};
  std::vector<OpenCVCamera *> cameras{};
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

  if (strcmp(method, "ni2GetVideoTexture") == 0)
  {
    FlValue *valueVideoIndex = fl_value_lookup_string(args, "videoIndex");
    const int index = fl_value_get_int(valueVideoIndex);

    int64_t result = 0;
    if (index == VideoIndex::RGB)
    {
      result = RGB_TEXTURE_GET_CLASS(self->rgbTexture)->texture_id;
    }
    else if (index == VideoIndex::Depth)
    {
      result = DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->texture_id;
    }
    else if (index == VideoIndex::IR)
    {
      result = IR_TEXTURE_GET_CLASS(self->irTexture)->texture_id;
    }
    else if (index == VideoIndex::POINTCLOUD)
    {
      result = OPENGL_TEXTURE_GET_CLASS(self->openglTexture)->texture_id;
    }
    else if (index == VideoIndex::Camera2D)
    {
      result = UVC_TEXTURE_GET_CLASS(self->uvcTexture)->texture_id;
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(result)));
  }
  else if (strcmp(method, "ni2Initialize") == 0)
  {
    std::unique_ptr<FlValueWrapper> ret = std::make_unique<FlValueWrapper>(nullptr);
    self->ni2->init(ret.get());
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(ret.get()->value));
  }
  else if (strcmp(method, "ni2EnumerateDevices") == 0)
  {
    g_autoptr(FlValue) deviceList = fl_value_new_list(); // TODO: initialize inside function not work.
    self->ni2->enumerateDevices(deviceList);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(deviceList));
  }
  else if (strcmp(method, "ni2OpenDevice") == 0)
  {
    FlValue *valueUri = fl_value_lookup_string(args, "uri");
    const char *uri = fl_value_get_string(valueUri);

    FlValue *valueVideoMode = fl_value_lookup_string(args, "videoMode");
    const int videoMode = fl_value_get_int(valueVideoMode);

    std::unique_ptr<FlValueWrapper> ret = std::make_unique<FlValueWrapper>(nullptr);
    self->ni2->openDevice(ret.get(), uri, videoMode);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(ret.get()->value));
  }
  else if (strcmp(method, "ni2CloseDevice") == 0)
  {
    self->ni2->closeDevice();
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "ni2DeviceIsConnected") == 0)
  {
    std::unique_ptr<FlValueWrapper> ret = std::make_unique<FlValueWrapper>(nullptr);
    self->ni2->isConnected(ret.get());
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(ret.get()->value));
  }
  else if (strcmp(method, "ni2GetEnabledVideoModes") == 0)
  {
    std::unique_ptr<FlValueWrapper> ret = std::make_unique<FlValueWrapper>(nullptr);
    self->ni2->getEnabledVideoModes(ret.get());
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(ret.get()->value));
  }
  else if (strcmp(method, "ni2ConfigVideoStream") == 0)
  {
    FlValue *valueVideoMode = fl_value_lookup_string(args, "videoMode");
    FlValue *valueEnable = fl_value_lookup_string(args, "enable");
    int videMode = fl_value_get_int(valueVideoMode);
    bool enable = fl_value_get_bool(valueEnable);

    std::unique_ptr<FlValueWrapper> ret = std::make_unique<FlValueWrapper>(nullptr);
    self->ni2->configVideoStream(ret.get(), videMode, &enable);
    if (enable)
    {
      self->ni2->readVideoFeed();
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(ret.get()->value));
  }
  else if (strcmp(method, "ni2GetFramePointer") == 0)
  {
    FlValue *valueVideoIndex = fl_value_lookup_string(args, "videoIndex");
    const int index = fl_value_get_int(valueVideoIndex);

    long result = 0;
    if (index == VideoIndex::RGB)
    {
      result = reinterpret_cast<long>(&RGB_TEXTURE_GET_CLASS(self->rgbTexture)->cvImage);
    }
    else if (index == VideoIndex::Depth)
    {
      result = reinterpret_cast<long>(&DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->cvImage);
    }
    else if (index == VideoIndex::IR)
    {
      result = reinterpret_cast<long>(&IR_TEXTURE_GET_CLASS(self->irTexture)->cvImage);
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(result)));
  }
  else if (strcmp(method, "ni2SetVideoSize") == 0)
  {
    FlValue *valueVideoIndex = fl_value_lookup_string(args, "videoIndex");
    const int index = fl_value_get_int(valueVideoIndex);
    FlValue *valueWidth = fl_value_lookup_string(args, "width");
    const int width = fl_value_get_int(valueWidth);
    FlValue *valueHeight = fl_value_lookup_string(args, "height");
    const int height = fl_value_get_int(valueHeight);

    if (index == VideoIndex::RGB)
    {
      RgbTextureClass *c = RGB_TEXTURE_GET_CLASS(self->rgbTexture);
      c->video_width = width;
      c->video_height = height;
      c->buffer.resize(width * height * 4);
    }
    else if (index == VideoIndex::Depth)
    {
      DepthTextureClass *c = DEPTH_TEXTURE_GET_CLASS(self->depthTexture);
      c->video_width = width;
      c->video_height = height;
      c->buffer.resize(width * height * 4);
    }
    else if (index == VideoIndex::IR)
    {
      IrTextureClass *c = IR_TEXTURE_GET_CLASS(self->irTexture);
      c->video_width = width;
      c->video_height = height;
      c->buffer.resize(width * height * 4);
    }
    else if (index == VideoIndex::Camera2D)
    {
      UvcTextureClass *c = UVC_TEXTURE_GET_CLASS(self->uvcTexture);
      c->video_width = width;
      c->video_height = height;
      c->buffer.resize(width * height * 4);
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "openglSetCamPosition") == 0)
  {
    const float x = fl_value_get_float(fl_value_lookup_string(args, "x"));
    const float y = fl_value_get_float(fl_value_lookup_string(args, "y"));
    const float z = fl_value_get_float(fl_value_lookup_string(args, "z"));

    self->glfl->setCamPosition(x, y, z);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "openglSetCamAngle") == 0)
  {
    const float yaw = fl_value_get_float(fl_value_lookup_string(args, "yaw"));
    const float pitch = fl_value_get_float(fl_value_lookup_string(args, "pitch"));

    self->glfl->setYawPitch(yaw, pitch);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "openglSetCamFov") == 0)
  {
    const float fov = fl_value_get_float(fl_value_lookup_string(args, "fov"));

    self->glfl->setFov(fov);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "openglRender") == 0)
  {
    self->glfl->render();
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "pipelineAdd") == 0)
  {
    FlValue *valueIndex = fl_value_lookup_string(args, "index");
    const int index = fl_value_get_int(valueIndex);
    FlValue *valueFuncIndex = fl_value_lookup_string(args, "funcIndex");
    const int funcIndex = fl_value_get_int(valueFuncIndex);
    FlValue *valueLen = fl_value_lookup_string(args, "len");
    const int len = fl_value_get_int(valueLen);
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

    Pipeline *pipeline;
    if (index == VideoIndex::RGB)
    {
      RGB_TEXTURE_GET_CLASS(self->rgbTexture)->pipeline->add(funcIndex, params, len, insertAt);
    }
    else if (index == VideoIndex::Depth)
    {
      DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->pipeline->add(funcIndex, params, len, insertAt);
    }
    else if (index == VideoIndex::IR)
    {
      IR_TEXTURE_GET_CLASS(self->irTexture)->pipeline->add(funcIndex, params, len, insertAt);
    }
    else if (index == VideoIndex::Camera2D)
    {
      UVC_TEXTURE_GET_CLASS(self->uvcTexture)->pipeline->add(funcIndex, params, len, insertAt);
    }
    else if (index == PIPELINE_INDEX_TFLITE)
    {
      self->tfPipeline->add(funcIndex, params, len, insertAt);
    }
    else
    {
      return;
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "pipelineClear") == 0)
  {
    FlValue *valueIndex = fl_value_lookup_string(args, "index");
    const int index = fl_value_get_int(valueIndex);
    if (index == VideoIndex::RGB)
    {
      RGB_TEXTURE_GET_CLASS(self->rgbTexture)->pipeline->clear();
    }
    else if (index == VideoIndex::Depth)
    {
      DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->pipeline->clear();
    }
    else if (index == VideoIndex::IR)
    {
      IR_TEXTURE_GET_CLASS(self->irTexture)->pipeline->clear();
    }
    else if (index == VideoIndex::Camera2D)
    {
      UVC_TEXTURE_GET_CLASS(self->uvcTexture)->pipeline->clear();
    }
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "tfliteCreateModel") == 0)
  {
    FlValue *valuePath = fl_value_lookup_string(args, "modelPath");
    const char *path = fl_value_get_string(valuePath);

    TFLiteModel *m = new TFLiteModel(path);
    self->models.push_back(m);

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }
  else if (strcmp(method, "tfliteGetModelInfo") == 0)
  {
    FlValue *valueIndex = fl_value_lookup_string(args, "index");
    const int index = fl_value_get_int(valueIndex);

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
    FlValue *valueIndex = fl_value_lookup_string(args, "tensorIndex");
    const int index = fl_value_get_int(valueIndex);
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
    FlValue *value = fl_value_lookup_string(args, "value");
    float f = fl_value_get_float(value);
    uint8_t *bytes = reinterpret_cast<uint8_t *>(&f);

    FlValue *result = fl_value_new_uint8_list(bytes, 4);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  else if (strcmp(method, "cameraOpen") == 0)
  {
    FlValue *flIndex = fl_value_lookup_string(args, "index");
    int index = fl_value_get_int(flIndex);

    bool newCam = true;
    bool result = false;
    for (int i = 0; i < self->cameras.size(); i++)
    {
      if (self->cameras[i]->capIndex == index)
      {
        newCam = false;
        if (!self->cameras[i]->cap->isOpened())
        {
          self->cameras[i]->cap->open(index);
        }

        result = true;
        break;
      }
    }

    if (newCam)
    {
      OpenCVCamera *c = new OpenCVCamera(index, self->uvcTexture, self->texture_registrar);
      c->open();
      self->cameras.push_back(c);
      result = c->cap->isOpened();
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(result)));
  }
  else if (strcmp(method, "cameraConfig") == 0)
  {
    FlValue *flIndex = fl_value_lookup_string(args, "index");
    int index = fl_value_get_int(flIndex);
    FlValue *flStart = fl_value_lookup_string(args, "start");
    bool start = fl_value_get_bool(flStart);

    bool result = false;
    for (int i = 0; i < self->cameras.size(); i++)
    {
      if (self->cameras[i]->capIndex == index)
      {
        if (start)
        {
          self->cameras[i]->start();
        }
        else
        {
          self->cameras[i]->stop();
        }

        result = true;
        break;
      }
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(result)));
  }
  else if (strcmp(method, "screenshot") == 0)
  {
    // TODO: this function should be one of the pipeline function

    FlValue *flIndex = fl_value_lookup_string(args, "index");
    int index = fl_value_get_int(flIndex);
    FlValue *flPath = fl_value_lookup_string(args, "path");
    const char *path = fl_value_get_string(flPath);
    FlValue *flCvtCode = fl_value_lookup_string(args, "cvtCode");
    int cvtCode = fl_value_get_int(flCvtCode);

    bool result = false;
    cv::Mat frame;
    if (index == VideoIndex::RGB)
    {
      RGB_TEXTURE_GET_CLASS(self->rgbTexture)->pipeline->screenshot(path, cvtCode);
    }
    else if (index == VideoIndex::Depth)
    {
      DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->pipeline->screenshot(path, cvtCode);
    }
    else if (index == VideoIndex::IR)
    {
      IR_TEXTURE_GET_CLASS(self->irTexture)->pipeline->screenshot(path, cvtCode);
    }
    else if (index == VideoIndex::Camera2D)
    {
      UVC_TEXTURE_GET_CLASS(self->uvcTexture)->pipeline->screenshot(path, cvtCode);
    }
    else
    {
    }

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_bool(true)));
  }
  else if (strcmp(method, "test") == 0)
  {
    // cv::Mat b(1280, 720, CV_8UC4, cv::Scalar(255, 0, 0, 255));
    cv::Mat b = cv::imread("/home/hedgehao/test/cpp/tflite/images/faces.jpg", cv::IMREAD_COLOR);
    cv::cvtColor(b, b, cv::COLOR_BGR2RGB);
    cv::Mat g(500, 500, CV_16UC1, cv::Scalar(125, 125, 125, 255));
    cv::Mat r(500, 500, CV_16UC1, cv::Scalar(220, 220, 220, 255));

    RGB_TEXTURE_GET_CLASS(self->rgbTexture)->pipeline->run(b, *self->texture_registrar, *FL_TEXTURE(self->rgbTexture), RGB_TEXTURE_GET_CLASS(self->rgbTexture)->video_width, RGB_TEXTURE_GET_CLASS(self->rgbTexture)->video_height, RGB_TEXTURE_GET_CLASS(self->rgbTexture)->buffer, &self->models);
    DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->pipeline->run(g, *self->texture_registrar, *FL_TEXTURE(self->depthTexture), DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->video_width, DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->video_height, DEPTH_TEXTURE_GET_CLASS(self->depthTexture)->buffer, &self->models);
    IR_TEXTURE_GET_CLASS(self->irTexture)->pipeline->run(r, *self->texture_registrar, *FL_TEXTURE(self->irTexture), IR_TEXTURE_GET_CLASS(self->irTexture)->video_width, IR_TEXTURE_GET_CLASS(self->irTexture)->video_height, IR_TEXTURE_GET_CLASS(self->irTexture)->buffer, &self->models);

    // if (self->models.size())
    // {
    //   printf("TF Test\n");
    //   cv::Mat img = cv::imread("/home/hedgehao/test/BlazeFace-TFLite-Inference/img/image.jpg");
    //   img = img(cv::Range(0, 448), cv::Range(0, 448));
    //   img.convertTo(img, CV_32FC2, 1.0f / 255.0f);
    //   cv::resize(img, img, cv::Size(128, 128));
    //   self->tfPipeline->run(img, g, b, *self->models[0]);
    // }

    fl_texture_registrar_mark_texture_frame_available(self->texture_registrar, FL_TEXTURE(self->rgbTexture));

    response = FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
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

  plugin->flView = fl_plugin_registrar_get_view(registrar);

  plugin->ni2 = new OpenNi2Wrapper();
  plugin->texture_registrar = fl_plugin_registrar_get_texture_registrar(registrar);

  plugin->uvcTexture = UVC_TEXTURE(g_object_new(uvc_texture_get_type(), nullptr));
  FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(plugin->uvcTexture)->copy_pixels = uvc_texture_copy_pixels;
  fl_texture_registrar_register_texture(plugin->texture_registrar, FL_TEXTURE(plugin->uvcTexture));
  UVC_TEXTURE_GET_CLASS(plugin->uvcTexture)->texture_id = reinterpret_cast<int64_t>(FL_TEXTURE(plugin->uvcTexture));
  fl_texture_registrar_mark_texture_frame_available(plugin->texture_registrar, FL_TEXTURE(plugin->uvcTexture));
  UVC_TEXTURE_GET_CLASS(plugin->uvcTexture)->pipeline = new Pipeline();
  UVC_TEXTURE_GET_CLASS(plugin->uvcTexture)->models = &plugin->models;

  plugin->rgbTexture = RGB_TEXTURE(g_object_new(rgb_texture_get_type(), nullptr));
  FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(plugin->rgbTexture)->copy_pixels = rgb_texture_copy_pixels;
  fl_texture_registrar_register_texture(plugin->texture_registrar, FL_TEXTURE(plugin->rgbTexture));
  RGB_TEXTURE_GET_CLASS(plugin->rgbTexture)->texture_id = reinterpret_cast<int64_t>(FL_TEXTURE(plugin->rgbTexture));
  fl_texture_registrar_mark_texture_frame_available(plugin->texture_registrar, FL_TEXTURE(plugin->rgbTexture));
  RGB_TEXTURE_GET_CLASS(plugin->rgbTexture)->pipeline = new Pipeline();

  plugin->depthTexture = DEPTH_TEXTURE(g_object_new(depth_texture_get_type(), nullptr));
  FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(plugin->depthTexture)->copy_pixels = depth_texture_copy_pixels;
  fl_texture_registrar_register_texture(plugin->texture_registrar, FL_TEXTURE(plugin->depthTexture));
  DEPTH_TEXTURE_GET_CLASS(plugin->depthTexture)->texture_id = reinterpret_cast<int64_t>(FL_TEXTURE(plugin->depthTexture));
  fl_texture_registrar_mark_texture_frame_available(plugin->texture_registrar, FL_TEXTURE(plugin->depthTexture));
  DEPTH_TEXTURE_GET_CLASS(plugin->depthTexture)->pipeline = new Pipeline();

  plugin->irTexture = IR_TEXTURE(g_object_new(ir_texture_get_type(), nullptr));
  FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(plugin->irTexture)->copy_pixels = ir_texture_copy_pixels;
  fl_texture_registrar_register_texture(plugin->texture_registrar, FL_TEXTURE(plugin->irTexture));
  IR_TEXTURE_GET_CLASS(plugin->irTexture)->texture_id = reinterpret_cast<int64_t>(FL_TEXTURE(plugin->irTexture));
  fl_texture_registrar_mark_texture_frame_available(plugin->texture_registrar, FL_TEXTURE(plugin->irTexture));
  IR_TEXTURE_GET_CLASS(plugin->irTexture)->pipeline = new Pipeline();

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

  plugin->tfPipeline = new TfPipeline();
  plugin->ni2->registerFlContext(plugin->texture_registrar, plugin->rgbTexture, plugin->depthTexture, plugin->irTexture, channel, plugin->glfl, &plugin->models, plugin->tfPipeline);

  g_object_unref(plugin);
}