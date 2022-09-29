#ifndef _DEF_CAMERA_
#define _DEF_CAMERA_

#include "../pipeline/pipeline.h"
#include "../fv_texture.h"
#include "../opengl.h"

#define NOT_SUPPORT -99

enum VideoIndex
{
  RGB = 0b1,
  Depth = 0b10,
  IR = 0b100,
  OPENGL = 0b1000,
  Camera2D = 0b10000,
};

class FvCamera
{
public:
  OpenGLFL *glfl;
  FlTextureRegistrar *flRegistrar;
  std::vector<TFLiteModel *> *models;
  FlMethodChannel *flChannel;

  std::string serial;
  FvTexture *rgbTexture;
  FvTexture *depthTexture;
  FvTexture *irTexture;
  bool videoStart = false;
  bool enablePointCloud = false;

  FvCamera() {}

  FvCamera(const char *s)
  {
    serial = std::string(s);
  }

  static FvCamera *findCam(const char *serial, std::vector<FvCamera *> *cams)
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

  static bool removeCam(const char *serial, std::vector<FvCamera *> *cams)
  {
    for (int i = 0; i < cams->size(); i++)
    {
      if (strcmp(cams->at(i)->serial.c_str(), serial) == 0)
      {
        cams->erase(cams->begin() + i);
        return true;
      }
    }

    return false;
  }

  void fvInit(FlTextureRegistrar *r, std::vector<TFLiteModel *> *m, FlMethodChannel *f, OpenGLFL *g)
  {
    flRegistrar = r;
    // TODO: check if this is duplicate from texture
    models = m;
    flChannel = f;
    glfl = g;

    // Create texture
    rgbTexture = FV_TEXTURE(g_object_new(fv_texture_get_type(), nullptr));
    FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(rgbTexture)->copy_pixels = fv_texture_copy_pixels;
    fl_texture_registrar_register_texture(flRegistrar, FL_TEXTURE(rgbTexture));
    rgbTexture->textureId = reinterpret_cast<int64_t>(FL_TEXTURE(rgbTexture));
    fl_texture_registrar_mark_texture_frame_available(flRegistrar, FL_TEXTURE(rgbTexture));
    depthTexture = FV_TEXTURE(g_object_new(fv_texture_get_type(), nullptr));
    FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(depthTexture)->copy_pixels = fv_texture_copy_pixels;
    fl_texture_registrar_register_texture(flRegistrar, FL_TEXTURE(depthTexture));
    depthTexture->textureId = reinterpret_cast<int64_t>(FL_TEXTURE(depthTexture));
    fl_texture_registrar_mark_texture_frame_available(flRegistrar, FL_TEXTURE(depthTexture));
    irTexture = FV_TEXTURE(g_object_new(fv_texture_get_type(), nullptr));
    FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(irTexture)->copy_pixels = fv_texture_copy_pixels;
    fl_texture_registrar_register_texture(flRegistrar, FL_TEXTURE(irTexture));
    irTexture->textureId = reinterpret_cast<int64_t>(FL_TEXTURE(irTexture));
    fl_texture_registrar_mark_texture_frame_available(flRegistrar, FL_TEXTURE(irTexture));

    // Create Pipeline
    FV_TEXTURE(rgbTexture)->pipeline = new Pipeline(&FV_TEXTURE(rgbTexture)->cvImage);
    FV_TEXTURE(rgbTexture)->models = models;
    FV_TEXTURE(depthTexture)->pipeline = new Pipeline(&FV_TEXTURE(depthTexture)->cvImage);
    FV_TEXTURE(depthTexture)->models = models;
    FV_TEXTURE(irTexture)->pipeline = new Pipeline(&FV_TEXTURE(irTexture)->cvImage);
    FV_TEXTURE(irTexture)->models = models;
  }

  int64_t getTextureId(int index)
  {
    if (index == VideoIndex::RGB)
    {
      return rgbTexture->textureId;
    }
    else if (index == VideoIndex::Depth)
    {
      return depthTexture->textureId;
    }
    else if (index == VideoIndex::IR)
    {
      return irTexture->textureId;
    }

    return -1;
  }

  virtual int camInit() = 0;
  virtual int openDevice() = 0;
  virtual int closeDevice() = 0;
  virtual int isConnected() = 0;
  virtual int configVideoStream(int streamIndex, bool *enable) = 0;
  virtual int readVideoFeed() = 0;
  virtual int configure(int prop, std::vector<float> &value) = 0;
  virtual int getConfiguration(int prop) = 0;

private:
  virtual int _readVideoFeed() = 0;
};
#endif