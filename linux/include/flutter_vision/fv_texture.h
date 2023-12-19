#ifndef _DEF_FV_TEXTURE_
#define _DEF_FV_TEXTURE_
#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <vector>
#include <thread>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "tflite.h"

#define FV_TEXTURE_TYPE (fv_texture_get_type())
#define FV_TEXTURE(obj) (G_TYPE_CHECK_INSTANCE_CAST(obj, FV_TEXTURE_TYPE, FvTexture))
struct FvTextureClass
{
  FlPixelBufferTextureClass parent_class;
};

class Pipeline;
struct FvTexture
{
  FlPixelBufferTexture flPixelBufferTexture;
  int64_t textureId = 0;
  std::vector<uint8_t> buffer{};
  int32_t video_width = 0;
  int32_t video_height = 0;
  bool video_start = false;
  cv::Mat cvImage;
  Pipeline *pipeline;
  std::vector<TFLiteModel *> *models;
};

G_DEFINE_TYPE(FvTexture,
              fv_texture,
              fl_pixel_buffer_texture_get_type())

static gboolean fv_texture_copy_pixels(
    FlPixelBufferTexture *texture,
    const uint8_t **out_buffer,
    uint32_t *width,
    uint32_t *height,
    GError **error)
{
  *out_buffer = FV_TEXTURE(texture)->buffer.data();
  *width = FV_TEXTURE(texture)->video_width;
  *height = FV_TEXTURE(texture)->video_height;

  return TRUE;
}

static void fv_texture_class_init(
    FvTextureClass *klass)
{
  FL_PIXEL_BUFFER_TEXTURE_CLASS(klass)->copy_pixels = fv_texture_copy_pixels;
}

static void fv_texture_init(FvTexture *self)
{
}
#endif