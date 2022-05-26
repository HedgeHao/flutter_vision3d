class OpenCV {
  static const COLOR_BGR2BGRA = 0; //!< add alpha channel to RGB or BGR image
  static const COLOR_RGB2RGBA = COLOR_BGR2BGRA;
  static const COLOR_BGRA2BGR = 1; //!< remove alpha channel from RGB or BGR image
  static const COLOR_RGBA2RGB = COLOR_BGRA2BGR;
  static const COLOR_BGR2RGBA = 2; //!< convert between RGB and BGR color spaces (with or without alpha channel)
  static const COLOR_RGB2BGRA = COLOR_BGR2RGBA;
  static const COLOR_RGBA2BGR = 3;
  static const COLOR_BGRA2RGB = COLOR_RGBA2BGR;
  static const COLOR_BGR2RGB = 4;
  static const COLOR_RGB2BGR = COLOR_BGR2RGB;
  static const COLOR_BGRA2RGBA = 5;
  static const COLOR_RGBA2BGRA = COLOR_BGRA2RGBA;
  static const COLOR_BGR2GRAY = 6; //!< convert between RGB/BGR and grayscale, @ref color_convert_rgb_gray "color conversions"
  static const COLOR_RGB2GRAY = 7;
  static const COLOR_GRAY2BGR = 8;
  static const COLOR_GRAY2RGB = COLOR_GRAY2BGR;
  static const COLOR_GRAY2BGRA = 9;
  static const COLOR_GRAY2RGBA = COLOR_GRAY2BGRA;
  static const COLOR_BGRA2GRAY = 10;
  static const COLOR_RGBA2GRAY = 11;
  static const COLOR_BGR2BGR565 = 12; //!< convert between RGB/BGR and BGR565 (16-bit images)
  static const COLOR_RGB2BGR565 = 13;
  static const COLOR_BGR5652BGR = 14;
  static const COLOR_BGR5652RGB = 15;
  static const COLOR_BGRA2BGR565 = 16;
  static const COLOR_RGBA2BGR565 = 17;
  static const COLOR_BGR5652BGRA = 18;
  static const COLOR_BGR5652RGBA = 19;
  static const COLOR_GRAY2BGR565 = 20; //!< convert between grayscale to BGR565 (16-bit images)
  static const COLOR_BGR5652GRAY = 21;
  static const COLOR_BGR2BGR555 = 22; //!< convert between RGB/BGR and BGR555 (16-bit images)
  static const COLOR_RGB2BGR555 = 23;
  static const COLOR_BGR5552BGR = 24;
  static const COLOR_BGR5552RGB = 25;
  static const COLOR_BGRA2BGR555 = 26;
  static const COLOR_RGBA2BGR555 = 27;
  static const COLOR_BGR5552BGRA = 28;
  static const COLOR_BGR5552RGBA = 29;
  static const COLOR_GRAY2BGR555 = 30; //!< convert between grayscale and BGR555 (16-bit images)
  static const COLOR_BGR5552GRAY = 31;
  static const COLOR_BGR2XYZ = 32; //!< convert RGB/BGR to CIE XYZ, @ref color_convert_rgb_xyz "color conversions"
  static const COLOR_RGB2XYZ = 33;
  static const COLOR_XYZ2BGR = 34;
  static const COLOR_XYZ2RGB = 35;
  static const COLOR_BGR2YCrCb = 36; //!< convert RGB/BGR to luma-chroma (aka YCC), @ref color_convert_rgb_ycrcb "color conversions"
  static const COLOR_RGB2YCrCb = 37;
  static const COLOR_YCrCb2BGR = 38;
  static const COLOR_YCrCb2RGB = 39;
  static const COLOR_BGR2HSV = 40; //!< convert RGB/BGR to HSV (hue saturation value), @ref color_convert_rgb_hsv "color conversions"
  static const COLOR_RGB2HSV = 41;
  static const COLOR_BGR2Lab = 44; //!< convert RGB/BGR to CIE Lab, @ref color_convert_rgb_lab "color conversions"
  static const COLOR_RGB2Lab = 45;
  static const COLOR_BGR2Luv = 50; //!< convert RGB/BGR to CIE Luv, @ref color_convert_rgb_luv "color conversions"
  static const COLOR_RGB2Luv = 51;
  static const COLOR_BGR2HLS = 52; //!< convert RGB/BGR to HLS (hue lightness saturation), @ref color_convert_rgb_hls "color conversions"
  static const COLOR_RGB2HLS = 53;
  static const COLOR_HSV2BGR = 54; //!< backward conversions to RGB/BG;
  static const COLOR_HSV2RGB = 55;
  static const COLOR_Lab2BGR = 56;
  static const COLOR_Lab2RGB = 57;
  static const COLOR_Luv2BGR = 58;
  static const COLOR_Luv2RGB = 59;
  static const COLOR_HLS2BGR = 60;
  static const COLOR_HLS2RGB = 61;
  static const COLOR_BGR2HSV_FULL = 66;
  static const COLOR_RGB2HSV_FULL = 67;
  static const COLOR_BGR2HLS_FULL = 68;
  static const COLOR_RGB2HLS_FULL = 69;
  static const COLOR_HSV2BGR_FULL = 70;
  static const COLOR_HSV2RGB_FULL = 71;
  static const COLOR_HLS2BGR_FULL = 72;
  static const COLOR_HLS2RGB_FULL = 73;
  static const COLOR_LBGR2Lab = 74;
  static const COLOR_LRGB2Lab = 75;
  static const COLOR_LBGR2Luv = 76;
  static const COLOR_LRGB2Luv = 77;
  static const COLOR_Lab2LBGR = 78;
  static const COLOR_Lab2LRGB = 79;
  static const COLOR_Luv2LBGR = 80;
  static const COLOR_Luv2LRGB = 81;
  static const COLOR_BGR2YUV = 82; //!< convert between RGB/BGR and YU
  static const COLOR_RGB2YUV = 83;
  static const COLOR_YUV2BGR = 84;
  static const COLOR_YUV2RGB = 85;

//! YUV 4:2:0 family to RG
  static const COLOR_YUV2RGB_NV12 = 90;
  static const COLOR_YUV2BGR_NV12 = 91;
  static const COLOR_YUV2RGB_NV21 = 92;
  static const COLOR_YUV2BGR_NV21 = 93;
  static const COLOR_YUV420sp2RGB = COLOR_YUV2RGB_NV21;
  static const COLOR_YUV420sp2BGR = COLOR_YUV2BGR_NV21;
  static const COLOR_YUV2RGBA_NV12 = 94;
  static const COLOR_YUV2BGRA_NV12 = 95;
  static const COLOR_YUV2RGBA_NV21 = 96;
  static const COLOR_YUV2BGRA_NV21 = 97;
  static const COLOR_YUV420sp2RGBA = COLOR_YUV2RGBA_NV21;
  static const COLOR_YUV420sp2BGRA = COLOR_YUV2BGRA_NV21;
  static const COLOR_YUV2RGB_YV12 = 98;
  static const COLOR_YUV2BGR_YV12 = 99;
  static const COLOR_YUV2RGB_IYUV = 100;
  static const COLOR_YUV2BGR_IYUV = 101;
  static const COLOR_YUV2RGB_I420 = COLOR_YUV2RGB_IYUV;
  static const COLOR_YUV2BGR_I420 = COLOR_YUV2BGR_IYUV;
  static const COLOR_YUV420p2RGB = COLOR_YUV2RGB_YV12;
  static const COLOR_YUV420p2BGR = COLOR_YUV2BGR_YV12;
  static const COLOR_YUV2RGBA_YV12 = 102;
  static const COLOR_YUV2BGRA_YV12 = 103;
  static const COLOR_YUV2RGBA_IYUV = 104;
  static const COLOR_YUV2BGRA_IYUV = 105;
  static const COLOR_YUV2RGBA_I420 = COLOR_YUV2RGBA_IYUV;
  static const COLOR_YUV2BGRA_I420 = COLOR_YUV2BGRA_IYUV;
  static const COLOR_YUV420p2RGBA = COLOR_YUV2RGBA_YV12;
  static const COLOR_YUV420p2BGRA = COLOR_YUV2BGRA_YV12;
  static const COLOR_YUV2GRAY_420 = 106;
  static const COLOR_YUV2GRAY_NV21 = COLOR_YUV2GRAY_420;
  static const COLOR_YUV2GRAY_NV12 = COLOR_YUV2GRAY_420;
  static const COLOR_YUV2GRAY_YV12 = COLOR_YUV2GRAY_420;
  static const COLOR_YUV2GRAY_IYUV = COLOR_YUV2GRAY_420;
  static const COLOR_YUV2GRAY_I420 = COLOR_YUV2GRAY_420;
  static const COLOR_YUV420sp2GRAY = COLOR_YUV2GRAY_420;
  static const COLOR_YUV420p2GRAY = COLOR_YUV2GRAY_420;
//! YUV 4:2:2 family to RG;
  static const COLOR_YUV2RGB_UYVY = 107;
  static const COLOR_YUV2BGR_UYVY = 108;
//static const COLOR_YUV2RGB_VYUY = 109;
//static const COLOR_YUV2BGR_VYUY = 110;
  static const COLOR_YUV2RGB_Y422 = COLOR_YUV2RGB_UYVY;
  static const COLOR_YUV2BGR_Y422 = COLOR_YUV2BGR_UYVY;
  static const COLOR_YUV2RGB_UYNV = COLOR_YUV2RGB_UYVY;
  static const COLOR_YUV2BGR_UYNV = COLOR_YUV2BGR_UYVY;
  static const COLOR_YUV2RGBA_UYVY = 111;
  static const COLOR_YUV2BGRA_UYVY = 112;
//static const COLOR_YUV2RGBA_VYUY = 113;
//static const COLOR_YUV2BGRA_VYUY = 114;
  static const COLOR_YUV2RGBA_Y422 = COLOR_YUV2RGBA_UYVY;
  static const COLOR_YUV2BGRA_Y422 = COLOR_YUV2BGRA_UYVY;
  static const COLOR_YUV2RGBA_UYNV = COLOR_YUV2RGBA_UYVY;
  static const COLOR_YUV2BGRA_UYNV = COLOR_YUV2BGRA_UYVY;
  static const COLOR_YUV2RGB_YUY2 = 115;
  static const COLOR_YUV2BGR_YUY2 = 116;
  static const COLOR_YUV2RGB_YVYU = 117;
  static const COLOR_YUV2BGR_YVYU = 118;
  static const COLOR_YUV2RGB_YUYV = COLOR_YUV2RGB_YUY2;
  static const COLOR_YUV2BGR_YUYV = COLOR_YUV2BGR_YUY2;
  static const COLOR_YUV2RGB_YUNV = COLOR_YUV2RGB_YUY2;
  static const COLOR_YUV2BGR_YUNV = COLOR_YUV2BGR_YUY2;
  static const COLOR_YUV2RGBA_YUY2 = 119;
  static const COLOR_YUV2BGRA_YUY2 = 120;
  static const COLOR_YUV2RGBA_YVYU = 121;
  static const COLOR_YUV2BGRA_YVYU = 122;
  static const COLOR_YUV2RGBA_YUYV = COLOR_YUV2RGBA_YUY2;
  static const COLOR_YUV2BGRA_YUYV = COLOR_YUV2BGRA_YUY2;
  static const COLOR_YUV2RGBA_YUNV = COLOR_YUV2RGBA_YUY2;
  static const COLOR_YUV2BGRA_YUNV = COLOR_YUV2BGRA_YUY2;
  static const COLOR_YUV2GRAY_UYVY = 123;
  static const COLOR_YUV2GRAY_YUY2 = 124;
//const CV_YUV2GRAY_VYUY    = CV_YUV2GRAY_UYVY;
  static const COLOR_YUV2GRAY_Y422 = COLOR_YUV2GRAY_UYVY;
  static const COLOR_YUV2GRAY_UYNV = COLOR_YUV2GRAY_UYVY;
  static const COLOR_YUV2GRAY_YVYU = COLOR_YUV2GRAY_YUY2;
  static const COLOR_YUV2GRAY_YUYV = COLOR_YUV2GRAY_YUY2;
  static const COLOR_YUV2GRAY_YUNV = COLOR_YUV2GRAY_YUY2;
//! alpha premultiplicatio
  static const COLOR_RGBA2mRGBA = 125;
  static const COLOR_mRGBA2RGBA = 126;
//! RGB to YUV 4:2:0 famil
  static const COLOR_RGB2YUV_I420 = 127;
  static const COLOR_BGR2YUV_I420 = 128;
  static const COLOR_RGB2YUV_IYUV = COLOR_RGB2YUV_I420;
  static const COLOR_BGR2YUV_IYUV = COLOR_BGR2YUV_I420;
  static const COLOR_RGBA2YUV_I420 = 129;
  static const COLOR_BGRA2YUV_I420 = 130;
  static const COLOR_RGBA2YUV_IYUV = COLOR_RGBA2YUV_I420;
  static const COLOR_BGRA2YUV_IYUV = COLOR_BGRA2YUV_I420;
  static const COLOR_RGB2YUV_YV12 = 131;
  static const COLOR_BGR2YUV_YV12 = 132;
  static const COLOR_RGBA2YUV_YV12 = 133;
  static const COLOR_BGRA2YUV_YV12 = 134;
//! Demosaicin
  static const COLOR_BayerBG2BGR = 46;
  static const COLOR_BayerGB2BGR = 47;
  static const COLOR_BayerRG2BGR = 48;
  static const COLOR_BayerGR2BGR = 49;
  static const COLOR_BayerBG2RGB = COLOR_BayerRG2BGR;
  static const COLOR_BayerGB2RGB = COLOR_BayerGR2BGR;
  static const COLOR_BayerRG2RGB = COLOR_BayerBG2BGR;
  static const COLOR_BayerGR2RGB = COLOR_BayerGB2BGR;
  static const COLOR_BayerBG2GRAY = 86;
  static const COLOR_BayerGB2GRAY = 87;
  static const COLOR_BayerRG2GRAY = 88;
  static const COLOR_BayerGR2GRAY = 89;
//! Demosaicing using Variable Number of Gradient
  static const COLOR_BayerBG2BGR_VNG = 62;
  static const COLOR_BayerGB2BGR_VNG = 63;
  static const COLOR_BayerRG2BGR_VNG = 64;
  static const COLOR_BayerGR2BGR_VNG = 65;
  static const COLOR_BayerBG2RGB_VNG = COLOR_BayerRG2BGR_VNG;
  static const COLOR_BayerGB2RGB_VNG = COLOR_BayerGR2BGR_VNG;
  static const COLOR_BayerRG2RGB_VNG = COLOR_BayerBG2BGR_VNG;
  static const COLOR_BayerGR2RGB_VNG = COLOR_BayerGB2BGR_VNG;
//! Edge-Aware Demosaicin
  static const COLOR_BayerBG2BGR_EA = 135;
  static const COLOR_BayerGB2BGR_EA = 136;
  static const COLOR_BayerRG2BGR_EA = 137;
  static const COLOR_BayerGR2BGR_EA = 138;
  static const COLOR_BayerBG2RGB_EA = COLOR_BayerRG2BGR_EA;
  static const COLOR_BayerGB2RGB_EA = COLOR_BayerGR2BGR_EA;
  static const COLOR_BayerRG2RGB_EA = COLOR_BayerBG2BGR_EA;
  static const COLOR_BayerGR2RGB_EA = COLOR_BayerGB2BGR_EA;
//! Demosaicing with alpha channe
  static const COLOR_BayerBG2BGRA = 139;
  static const COLOR_BayerGB2BGRA = 140;
  static const COLOR_BayerRG2BGRA = 141;
  static const COLOR_BayerGR2BGRA = 142;
  static const COLOR_BayerBG2RGBA = COLOR_BayerRG2BGRA;
  static const COLOR_BayerGB2RGBA = COLOR_BayerGR2BGRA;
  static const COLOR_BayerRG2RGBA = COLOR_BayerBG2BGRA;
  static const COLOR_BayerGR2RGBA = COLOR_BayerGB2BGRA;
  static const COLOR_COLORCVT_MAX = 143;

  static const CV_8U = 0;
  static const CV_8S = 1;
  static const CV_16U = 2;
  static const CV_16S = 3;
  static const CV_32S = 4;
  static const CV_32F = 5;
  static const CV_64F = 6;
  static const CV_16F = 7;
  static const CV_8UC1 = 0;
  static const CV_8UC2 = 8;
  static const CV_8UC3 = 16;
  static const CV_8UC4 = 24;
  static const CV_8SC1 = 1;
  static const CV_8SC2 = 9;
  static const CV_8SC3 = 17;
  static const CV_8SC4 = 25;
  static const CV_16UC1 = 2;
  static const CV_16UC2 = 10;
  static const CV_16UC3 = 18;
  static const CV_16UC4 = 26;
  static const CV_16SC1 = 3;
  static const CV_16SC2 = 11;
  static const CV_16SC3 = 19;
  static const CV_16SC4 = 27;
  static const CV_32SC1 = 4;
  static const CV_32SC2 = 12;
  static const CV_32SC3 = 20;
  static const CV_32SC4 = 28;
  static const CV_32FC1 = 5;
  static const CV_32FC2 = 13;
  static const CV_32FC3 = 21;
  static const CV_32FC4 = 29;
  static const CV_64FC1 = 6;
  static const CV_64FC2 = 14;
  static const CV_64FC3 = 22;
  static const CV_64FC4 = 30;
  static const CV_16FC1 = 7;
  static const CV_16FC2 = 15;
  static const CV_16FC3 = 23;
  static const CV_16FC4 = 31;

  /** nearest neighbor interpolation */
  static const INTER_NEAREST = 0;
  /** bilinear interpolation */
  static const INTER_LINEAR = 1;
  /** bicubic interpolation */
  static const INTER_CUBIC = 2;
  /** resampling using pixel area relation. It may be a preferred method for image decimation, as
    it gives moire'-free results. But when the image is zoomed, it is similar to the INTER_NEAREST
    method. */
  static const INTER_AREA = 3;
  /** Lanczos interpolation over 8x8 neighborhood */
  static const INTER_LANCZOS4 = 4;
  /** Bit exact bilinear interpolation */
  static const INTER_LINEAR_EXACT = 5;
  /** mask for interpolation codes */
  static const INTER_MAX = 7;
  /** flag, fills all of the destination image pixels. If some of them correspond to outliers in the
    source image, they are set to zero */
  static const WARP_FILL_OUTLIERS = 8;
  /** flag, inverse transformation

    For example, #linearPolar or #logPolar transforms:
    - flag is __not__ set: \f$dst( \rho , \phi ) = src(x,y)\f$
    - flag is set: \f$dst(x,y) = src( \rho , \phi )\f$ */
  static const WARP_INVERSE_MAP = 1;

  static const LINE_TYPE_FILLED = -1;
  static const LINE_TYPE_LINE_4 = 4; //!< 4-connected line
  static const LINE_TYPE_LINE_8 = 8; //!< 8-connected line
  static const LINE_TYPE_LINE_AA = 16; //!< antialiased line

  static const COLORMAP_AUTUMN = 0;
  static const COLORMAP_BONE = 1;
  static const COLORMAP_JET = 2;
  static const COLORMAP_WINTER = 3;
  static const COLORMAP_RAINBOW = 4;
  static const COLORMAP_OCEAN = 5;
  static const COLORMAP_SUMMER = 6;
  static const COLORMAP_SPRING = 7;
  static const COLORMAP_COOL = 8;
  static const COLORMAP_HSV = 9;
  static const COLORMAP_PINK = 10;
  static const COLORMAP_HOT = 11;
  static const COLORMAP_PARULA = 12;
  static const COLORMAP_MAGMA = 13;
  static const COLORMAP_INFERNO = 14;
  static const COLORMAP_PLASMA = 15;
  static const COLORMAP_VIRIDIS = 16;
  static const COLORMAP_CIVIDIS = 17;
  static const COLORMAP_TWILIGHT = 18;
  static const COLORMAP_TWILIGHT_SHIFTED = 19;
  static const COLORMAP_TURBO = 20;

  /* Rotate Code */
  static const ROTATE_90_CLOCKWISE = 0;
  static const ROTATE_180 = 1;
  static const ROTATE_90_COUNTERCLOCKWISE = 2;

  /* VideoCaptureProperties */
  static const CAP_PROP_POS_MSEC = 0;
  static const CAP_PROP_POS_FRAMES = 1;
  static const CAP_PROP_POS_AVI_RATIO = 2;
  static const CAP_PROP_FRAME_WIDTH = 3;
  static const CAP_PROP_FRAME_HEIGHT = 4;
  static const CAP_PROP_FPS = 5;
  static const CAP_PROP_FOURCC = 6;
  static const CAP_PROP_FRAME_COUNT = 7;
  static const CAP_PROP_FORMAT = 8;
  static const CAP_PROP_MODE = 9;
  static const CAP_PROP_BRIGHTNESS = 10;
  static const CAP_PROP_CONTRAST = 11;
  static const CAP_PROP_SATURATION = 12;
  static const CAP_PROP_HUE = 13;
  static const CAP_PROP_GAIN = 14;
  static const CAP_PROP_EXPOSURE = 15;
  static const CAP_PROP_CONVERT_RGB = 16;
  static const CAP_PROP_WHITE_BALANCE_BLUE_U = 17;
  static const CAP_PROP_RECTIFICATION = 18;
  static const CAP_PROP_MONOCHROME = 19;
  static const CAP_PROP_SHARPNESS = 20;
  static const CAP_PROP_AUTO_EXPOSURE = 21;
  static const CAP_PROP_GAMMA = 22;
  static const CAP_PROP_TEMPERATURE = 23;
  static const CAP_PROP_TRIGGER = 24;
  static const CAP_PROP_TRIGGER_DELAY = 25;
  static const CAP_PROP_WHITE_BALANCE_RED_V = 26;
  static const CAP_PROP_ZOOM = 27;
  static const CAP_PROP_FOCUS = 28;
  static const CAP_PROP_GUID = 29;
  static const CAP_PROP_ISO_SPEED = 30;
  static const CAP_PROP_BACKLIGHT = 32;
  static const CAP_PROP_PAN = 33;
  static const CAP_PROP_TILT = 34;
  static const CAP_PROP_ROLL = 35;
  static const CAP_PROP_IRIS = 36;
  static const CAP_PROP_SETTINGS = 37;
  static const CAP_PROP_BUFFERSIZE = 38;
  static const CAP_PROP_AUTOFOCUS = 39;
  static const CAP_PROP_SAR_NUM = 40;
  static const CAP_PROP_SAR_DEN = 41;
  static const CAP_PROP_BACKEND = 42;
  static const CAP_PROP_CHANNEL = 43;
  static const CAP_PROP_AUTO_WB = 44;
  static const CAP_PROP_WB_TEMPERATURE = 45;
  static const CAP_PROP_CODEC_PIXEL_FORMAT = 46;

  /* VideoCaptureModes */
  static const CAP_MODE_BGR = 0;
  static const CAP_MODE_RGB = 1;
  static const CAP_MODE_GRAY = 2;
  static const CAP_MODE_YUYV = 3;
}

const List<String> COCO_CLASSES = [
  "person",
  "bicycle",
  "car",
  "motorcycle",
  "airplane",
  "bus",
  "train",
  "truck",
  "boat",
  "traffic light",
  "fire hydrant",
  "street sign",
  "stop sign",
  "parking meter",
  "bench",
  "bird",
  "cat",
  "dog",
  "horse",
  "sheep",
  "cow",
  "elephant",
  "bear",
  "zebra",
  "giraffe",
  "hat",
  "backpack",
  "umbrella",
  "shoe",
  "eye glasses",
  "handbag",
  "tie",
  "suitcase",
  "frisbee",
  "skis",
  "snowboard",
  "sports ball",
  "kite",
  "baseball bat",
  "baseball glove",
  "skateboard",
  "surfboard",
  "tennis racket",
  "bottle",
  "plate",
  "wine glass",
  "cup",
  "fork",
  "knife",
  "spoon",
  "bowl",
  "banana",
  "apple",
  "sandwich",
  "orange",
  "broccoli",
  "carrot",
  "hot dog",
  "pizza",
  "donut",
  "cake",
  "chair",
  "couch",
  "potted plant",
  "bed",
  "mirror",
  "dining table",
  "window",
  "desk",
  "toilet",
  "door",
  "tv",
  "laptop",
  "mouse",
  "remote",
  "keyboard",
  "cell phone",
  "microwave",
  "oven",
  "toaster",
  "sink",
  "refrigerator",
  "blender",
  "book",
  "clock",
  "vase",
  "scissors",
  "teddy bear",
  "hair drier",
  "toothbrush",
  "hair brush"
];
