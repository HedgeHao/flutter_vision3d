#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "pipeline/pipeline.h"
#include "tflite.h"
struct _UvcTextureClass
{
    FlPixelBufferTextureClass parent_class;
    int64_t texture_id = 0;
    std::vector<uint8_t> buffer{};
    int32_t video_width = 0;
    int32_t video_height = 0;
    bool video_start = false;
    cv::Mat cvImage;
    Pipeline *pipeline;
    std::vector<TFLiteModel *> *models;
};

G_DECLARE_DERIVABLE_TYPE(UvcTexture,
                         uvc_texture,
                         UVC,
                         TEXTURE,
                         FlPixelBufferTexture)

G_DEFINE_TYPE(UvcTexture,
              uvc_texture,
              fl_pixel_buffer_texture_get_type())

static gboolean uvc_texture_copy_pixels(
    FlPixelBufferTexture *texture,
    const uint8_t **out_buffer,
    uint32_t *width,
    uint32_t *height,
    GError **error)
{
    *out_buffer = UVC_TEXTURE_GET_CLASS(texture)->buffer.data();
    *width = UVC_TEXTURE_GET_CLASS(texture)->video_width;
    *height = UVC_TEXTURE_GET_CLASS(texture)->video_height;

    return TRUE;
}

static void uvc_texture_class_init(
    UvcTextureClass *klass)
{
    FL_PIXEL_BUFFER_TEXTURE_CLASS(klass)->copy_pixels = uvc_texture_copy_pixels;
}

static void uvc_texture_init(UvcTexture *self)
{
}

class OpenCVCamera
{
public:
    UvcTexture *texture;
    int capIndex = -1;
    cv::VideoCapture *cap = nullptr;

    OpenCVCamera(int index, UvcTexture *t, FlTextureRegistrar *r, FlMethodChannel *c)
    {
        texture = t;
        registrar = r;
        flChannel = c;
        capIndex = index;
        if (cap == nullptr)
        {
            cap = new cv::VideoCapture();
        }
    }

    void config(int prop, float value)
    {
        cap->set(prop, value);
    }

    bool open()
    {
        cap->open(capIndex);
        return cap->isOpened();
    }

    int start()
    {
        if (cap == nullptr)
            return -1;

        bool ret = cap->open(capIndex);
        if (!ret && !cap->isOpened())
            return -2;

        videoStart = true;
        std::thread t(&OpenCVCamera::_readVideoFeed, this);
        t.detach();
        return 0;
    }

    void stop()
    {
        videoStart = false;
    }

private:
    FlTextureRegistrar *registrar;
    FlMethodChannel *flChannel;
    bool videoStart = false;
    void _readVideoFeed()
    {
        UvcTextureClass *cls = UVC_TEXTURE_GET_CLASS(texture);
        bool newFrame = false;

        if (!(videoStart))
            return;

        while (videoStart)
        {
            newFrame = cap->read(cls->cvImage);

            if (newFrame)
            {
                cls->pipeline->run(cls->cvImage, *registrar, *FL_TEXTURE(texture), cls->video_width, cls->video_height, cls->buffer, cls->models, flChannel);
                fl_method_channel_invoke_method(flChannel, "onUvcFrame", nullptr, nullptr, nullptr, NULL);
            }
        }

        printf("Release VideoCap\n");
        cap->release();
    }
};