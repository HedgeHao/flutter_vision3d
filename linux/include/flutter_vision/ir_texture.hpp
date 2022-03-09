#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "pipeline/pipeline.h"

struct _IrTextureClass
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

G_DECLARE_DERIVABLE_TYPE(IrTexture,
                         ir_texture,
                         IR,
                         TEXTURE,
                         FlPixelBufferTexture)

G_DEFINE_TYPE(IrTexture,
              ir_texture,
              fl_pixel_buffer_texture_get_type())

static gboolean ir_texture_copy_pixels(
    FlPixelBufferTexture *texture,
    const uint8_t **out_buffer,
    uint32_t *width,
    uint32_t *height,
    GError **error)
{
    *out_buffer = IR_TEXTURE_GET_CLASS(texture)->buffer.data();
    *width = IR_TEXTURE_GET_CLASS(texture)->video_width;
    *height = IR_TEXTURE_GET_CLASS(texture)->video_height;

    return TRUE;
}

static void ir_texture_class_init(
    IrTextureClass *klass)
{
    FL_PIXEL_BUFFER_TEXTURE_CLASS(klass)->copy_pixels = ir_texture_copy_pixels;
}

static void ir_texture_init(IrTexture *self)
{
}