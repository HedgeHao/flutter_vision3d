#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <vector>

struct _OpenGLTextureClass
{
    FlPixelBufferTextureClass parent_class;
    int64_t texture_id = 0;
    uint8_t *buffer{};
    int32_t video_width = 0;
    int32_t video_height = 0;
    // bool video_start = false;
    // cv::Mat cvImage;
};

G_DECLARE_DERIVABLE_TYPE(OpenGLTexture,
                         opengl_texture,
                         OPENGL,
                         TEXTURE,
                         FlPixelBufferTexture)

G_DEFINE_TYPE(OpenGLTexture,
              opengl_texture,
              fl_pixel_buffer_texture_get_type())

static gboolean opengl_texture_copy_pixels(
    FlPixelBufferTexture *texture,
    const uint8_t **out_buffer,
    uint32_t *width,
    uint32_t *height,
    GError **error)
{
    *out_buffer = OPENGL_TEXTURE_GET_CLASS(texture)->buffer;
    *width = OPENGL_TEXTURE_GET_CLASS(texture)->video_width;
    *height = OPENGL_TEXTURE_GET_CLASS(texture)->video_height;

    return TRUE;
}

static void opengl_texture_class_init(
    OpenGLTextureClass *klass)
{
    FL_PIXEL_BUFFER_TEXTURE_CLASS(klass)->copy_pixels = opengl_texture_copy_pixels;
}

static void opengl_texture_init(OpenGLTexture *self)
{
}