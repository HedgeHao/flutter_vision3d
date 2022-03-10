#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "pipeline/pipeline.h"

struct _DepthTextureClass
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

G_DECLARE_DERIVABLE_TYPE(DepthTexture,
                         depth_texture,
                         DEPTH,
                         TEXTURE,
                         FlPixelBufferTexture)

G_DEFINE_TYPE(DepthTexture,
              depth_texture,
              fl_pixel_buffer_texture_get_type())

static gboolean depth_texture_copy_pixels(
    FlPixelBufferTexture *texture,
    const uint8_t **out_buffer,
    uint32_t *width,
    uint32_t *height,
    GError **error)
{
    *out_buffer = DEPTH_TEXTURE_GET_CLASS(texture)->buffer.data();
    *width = DEPTH_TEXTURE_GET_CLASS(texture)->video_width;
    *height = DEPTH_TEXTURE_GET_CLASS(texture)->video_height;

    return TRUE;
}

static void depth_texture_class_init(
    DepthTextureClass *klass)
{
    FL_PIXEL_BUFFER_TEXTURE_CLASS(klass)->copy_pixels = depth_texture_copy_pixels;
}

static void depth_texture_init(DepthTexture *self)
{
}