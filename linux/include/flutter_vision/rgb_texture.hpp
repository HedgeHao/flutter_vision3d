#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "pipeline/pipeline.h"
struct _RgbTextureClass
{
    FlPixelBufferTextureClass parent_class;
    int64_t texture_id = 0;
    std::vector<uint8_t> buffer{};
    int32_t video_width = 0;
    int32_t video_height = 0;
    bool video_start = false;
    cv::Mat cvImage;
    Pipeline *pipeline;
};

G_DECLARE_DERIVABLE_TYPE(RgbTexture,
                         rgb_texture,
                         RGB,
                         TEXTURE,
                         FlPixelBufferTexture)

G_DEFINE_TYPE(RgbTexture,
              rgb_texture,
              fl_pixel_buffer_texture_get_type())

static gboolean rgb_texture_copy_pixels(
    FlPixelBufferTexture *texture,
    const uint8_t **out_buffer,
    uint32_t *width,
    uint32_t *height,
    GError **error)
{
    *out_buffer = RGB_TEXTURE_GET_CLASS(texture)->buffer.data();
    *width = RGB_TEXTURE_GET_CLASS(texture)->video_width;
    *height = RGB_TEXTURE_GET_CLASS(texture)->video_height;

    return TRUE;
}

static void rgb_texture_class_init(
    RgbTextureClass *klass)
{
    FL_PIXEL_BUFFER_TEXTURE_CLASS(klass)->copy_pixels = rgb_texture_copy_pixels;
}

static void rgb_texture_init(RgbTexture *self)
{
}