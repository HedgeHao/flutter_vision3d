#ifndef _DEF_OPENNI_CAM_
#define _DEF_OPENNI_CAM_
#include <flutter/plugin_registrar_windows.h>

#define LIPS_FACE_RECOGNITION 0x258
#define LIPS_FACE_REGISTRATION 0x259
#define LIPS_FACE_DELETE_FACE_DATABASE 0x25A
#define LIPS_L21X_LASER 307
#define STREAM_PROPERTY_FOCAL_LENGTH_X 200
#define STREAM_PROPERTY_FOCAL_LENGTH_Y 201
#define STREAM_PROPERTY_PRINCIPAL_POINT_X 202
#define STREAM_PROPERTY_PRINCIPAL_POINT_Y 203

#include <OpenNI.h>
#include <iostream>
#include <sstream>

#include "fv_camera.h"

using namespace openni;

class OpenniCam : public FvCamera,
                  public OpenNI::DeviceConnectedListener,
                  public OpenNI::DeviceDisconnectedListener,
                  public OpenNI::DeviceStateChangedListener
{
public:
    static int openniInit()
    {
        return static_cast<int>(OpenNI::initialize());
    }

    static void enumerateDevices(Array<DeviceInfo> *niDevices)
    {
        OpenNI::enumerateDevices(niDevices);
        return;
    }

    Device *device = new Device();

    virtual void onDeviceStateChanged(const DeviceInfo *pInfo, DeviceState state)
    {
        // printf("[LISTENER] Device \"%s\" error state changed to %d\n", pInfo->getUri(), state);
    }

    virtual void onDeviceConnected(const DeviceInfo *pInfo)
    {
        // printf("[LISTENER] Device \"%s\" connected\n", pInfo->getUri());
    }

    virtual void onDeviceDisconnected(const DeviceInfo *pInfo)
    {
        // TODO: When supporting multiple devices. Find the right device to destory
        device = nullptr;
    }

    OpenniCam(const char *s) : FvCamera(s){};

    void camInit() {}

    int openDevice()
    {
        if (this->device->isValid())
        {
            return 0;
        }

        Status s = this->device->open(serial.c_str());
        if (s == STATUS_OK)
        {
            bool isValid = this->device->isValid();

            if (isValid)
            {
                // Get camera SN
                // int size = 32;
                // char sn[32];
                // this->device->getProperty(openni::DEVICE_PROPERTY_SERIAL_NUMBER, sn, &size);

                std::cout << "Valid" << std::endl;
                createVideoStream();
            }

            return isValid ? 0 : -1;
        }

        return -2;
    }

    int closeDevice()
    {
        if (device == nullptr)
        {
            return -1;
        }

        if (vsColor.isValid())
        {
            vsColor.stop();
            vsColor.destroy();
        }
        if (vsDepth.isValid())
        {
            vsDepth.stop();
            vsDepth.destroy();
        }
        if (vsIR.isValid())
        {
            vsIR.stop();
            vsIR.destroy();
        }

        this->device->close();
        return 0;
    }

    int isConnected()
    {
        if (device == nullptr)
        {
            return false;
        }

        return this->device->isValid();
    }

    void getIntrinsic(int index, double &fx, double &fy, double &cx, double &cy)
    {

        if (device == nullptr)
            return;

        if (index == VideoIndex::RGB)
        {
            vsColor.getProperty(STREAM_PROPERTY_FOCAL_LENGTH_X, &fx);
            vsColor.getProperty(STREAM_PROPERTY_FOCAL_LENGTH_Y, &fy);
            vsColor.getProperty(STREAM_PROPERTY_PRINCIPAL_POINT_X, &cx);
            vsColor.getProperty(STREAM_PROPERTY_PRINCIPAL_POINT_Y, &cy);
        }
        else if (index == VideoIndex::Depth)
        {
            vsDepth.getProperty(STREAM_PROPERTY_FOCAL_LENGTH_X, &fx);
            vsDepth.getProperty(STREAM_PROPERTY_FOCAL_LENGTH_Y, &fy);
            vsDepth.getProperty(STREAM_PROPERTY_PRINCIPAL_POINT_X, &cx);
            vsDepth.getProperty(STREAM_PROPERTY_PRINCIPAL_POINT_Y, &cy);
        }
    }

    int configVideoStream(int streamIndex, bool *enable)
    {
        if (device == nullptr)
            return -1;

        int isValid = 0;

        if (((streamIndex & VideoIndex::RGB) > 0) && niRgbAvailable)
        {
            if (*enable)
            {
                vsColor.start();
                if (vsColor.isValid())
                    isValid += VideoIndex::RGB;
                enableRgb = true;
            }
            else
            {
                vsColor.stop();
                enableRgb = false;
            }
        }

        if (((streamIndex & VideoIndex::Depth) > 0) && niDepthAvailable)
        {
            if (*enable)
            {
                vsDepth.start();
                if (vsDepth.isValid())
                    isValid += VideoIndex::Depth;
                enableDepth = true;
            }
            else
            {
                vsDepth.stop();
                enableDepth = false;
            }
        }

        if (((streamIndex & VideoIndex::IR) > 0) && niIrAvailable)
        {
            if (*enable)
            {
                vsIR.start();
                if (vsIR.isValid())
                    isValid += VideoIndex::IR;
                enableIr = true;
            }
            else
            {
                vsIR.stop();
                enableIr = false;
            }
        }

        if (isValid == 0)
        {
            *enable = false;
        }

        if (!enableRgb && !enableIr && !enableDepth)
        {
            videoStart = false;
            *enable = false;
        }

        return 0;
    }

    void readVideoFeed()
    {
        videoStart = true;
        std::thread t(&OpenniCam::_readVideoFeed, this);
        t.detach();
    }

    void configure(int prop, std::vector<float> &value)
    {
        switch (prop)
        {
        case LIPS_FACE_REGISTRATION:
        case LIPS_FACE_RECOGNITION:
        case LIPS_FACE_DELETE_FACE_DATABASE:
        case LIPS_L21X_LASER:
            int param = value[0];
            device->setProperty(prop, &param, sizeof(int));
            break;
        }
    }

    int getConfiguration(int prop)
    {
        unsigned short int result = -1;
        device->getProperty(prop, &result);
        return (int)result;
    }

    bool enableImageRegistration(bool enable)
    {
        if (device == nullptr)
            return false;

        if (device->isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR))
        {
            return openni::STATUS_OK == device->setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
        }

        return false;
    }

    void getAvailableVideoModes(int index, std::vector<std::string> &rModes)
    {
        const openni::SensorInfo *info;
        if (index == VideoIndex::RGB)
        {
            info = device->getSensorInfo(openni::SENSOR_COLOR);
        }
        else if (index == VideoIndex::Depth)
        {
            info = device->getSensorInfo(openni::SENSOR_DEPTH);
        }
        else if (index == VideoIndex::IR)
        {
            info = device->getSensorInfo(openni::SENSOR_IR);
        }

        auto &modes = info->getSupportedVideoModes();

        rModes.clear();
        for (int i = 0; i < modes.getSize(); i++)
        {
            std::stringstream ss;
            ss << modes[i].getResolutionX();
            ss << ",";
            ss << modes[i].getResolutionY();
            ss << ",";
            ss << modes[i].getFps();
            ss << ",";
            ss << modes[i].getPixelFormat();

            std::string s;
            ss >> s;
            rModes.push_back(s);
        }
    }

    void getCurrentVideoMode(int index, std::string &mode)
    {
        openni::VideoMode m;
        if (index == VideoIndex::RGB)
        {
            m = vsColor.getVideoMode();
        }
        else if (index == VideoIndex::Depth)
        {
            m = vsDepth.getVideoMode();
        }
        else if (index == VideoIndex::IR)
        {
            m = vsIR.getVideoMode();
        }

        std::stringstream ss;
        ss << m.getResolutionX();
        ss << ",";
        ss << m.getResolutionY();
        ss << ",";
        ss << m.getFps();
        ss << ",";
        ss << m.getPixelFormat();

        mode.clear();
        ss >> mode;
    }

    bool setVideoMode(int index, int mode)
    {
        if (index == VideoIndex::RGB)
        {
            auto &modes = device->getSensorInfo(openni::SENSOR_COLOR)->getSupportedVideoModes();
            if (mode > modes.getSize())
                return false;
            vsColor.setVideoMode(modes[mode]);
            return true;
        }
        else if (index == VideoIndex::Depth)
        {
            auto &modes = device->getSensorInfo(openni::SENSOR_DEPTH)->getSupportedVideoModes();
            if (mode > modes.getSize())
                return false;
            vsDepth.setVideoMode(modes[mode]);
            return true;
        }
        else if (index == VideoIndex::IR)
        {
            auto &modes = device->getSensorInfo(openni::SENSOR_IR)->getSupportedVideoModes();
            if (mode > modes.getSize())
                return false;
            vsIR.setVideoMode(modes[mode]);
            return true;
        }

        return false;
    }

    bool getSerialNumber(std::string &sn)
    {
        if(device == nullptr) return false;

        char s[32];
        device->getProperty(openni::DEVICE_PROPERTY_SERIAL_NUMBER, &s);
        sn = s;

        return true;
    }

private:
    VideoStream vsDepth;
    VideoStream vsColor;
    VideoStream vsIR;
    bool niRgbAvailable = false;
    bool niDepthAvailable = false;
    bool niIrAvailable = false;
    bool enableRgb = false;
    bool enableDepth = false;
    bool enableIr = false;

    void createVideoStream(int videoMode = 7) // create all video stream by default
    {
        if (device == nullptr)
        {
            return;
        }

        vsColor.destroy();
        vsDepth.destroy();
        vsIR.destroy();

        if ((videoMode & VideoIndex::RGB) > 0 && vsColor.create(*device, SENSOR_COLOR) == STATUS_OK)
        {
            niRgbAvailable = true;
        }
        if ((videoMode & VideoIndex::Depth) > 0 && vsDepth.create(*device, SENSOR_DEPTH) == STATUS_OK)
        {
            niDepthAvailable = true;
        }
        if ((videoMode & VideoIndex::IR) > 0 && vsIR.create(*device, SENSOR_IR) == STATUS_OK)
        {
            niIrAvailable = true;
        }
    }

    void colorMap(float dis, float *outputR, float *outputG, float *outputB)
    {
        int r = 0, g = 0, b = 0; // 0 ~ 255
        int map[5][4] =
            {
                // {distance, R, G, B}
                {0, 0, 0, 255},
                {500, 0, 255, 255},
                {1000, 255, 255, 0},
                {2000, 255, 0, 0},
                {4000, 102, 0, 0}};
        if (dis >= map[4][0])
        {
            r = map[4][1];
            g = map[4][2];
            b = map[4][3];
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {
                if (map[i][0] <= dis && dis < map[i + 1][0])
                {
                    float t = (dis - map[i][0]) / (map[i + 1][0] - map[i][0]);
                    r = map[i][1] + (map[i + 1][1] - map[i][1]) * t;
                    g = map[i][2] + (map[i + 1][2] - map[i][2]) * t;
                    b = map[i][3] + (map[i + 1][3] - map[i][3]) * t;
                    break;
                }
            }
        }
        *outputR = r / 255.0;
        *outputG = g / 255.0;
        *outputB = b / 255.0;
    }

    void niComputeCloud(const VideoStream &depthStream, const void *depthFrameData, const void *colorFrameData,
                        float *imgDepth, float *imgColor, float *imgColorMap, unsigned int *vertexCount)
    {
        int frameWidth = depthStream.getVideoMode().getResolutionX();
        int frameHeight = depthStream.getVideoMode().getResolutionY();

        const openni::DepthPixel *pDepth = (const openni::DepthPixel *)depthFrameData;
        const openni::RGB888Pixel *pColor = (const openni::RGB888Pixel *)colorFrameData;
        float fX, fY, fZ;
        int count = 0;
        for (int y = 0; y < frameHeight; y++)
        {
            for (int x = 0; x < frameWidth; x++)
            {
                fX = 0.0;
                fY = 0.0;
                fZ = 0.0;
                if (pDepth[count] != 0)
                {
                    CoordinateConverter::convertDepthToWorld(depthStream, x, y, pDepth[count], &fX, &fY, &fZ);
                    if (pColor != nullptr)
                    {
                        imgColor[count * 3 + 0] = pColor[count].r / 255.0;
                        imgColor[count * 3 + 1] = pColor[count].g / 255.0;
                        imgColor[count * 3 + 2] = pColor[count].b / 255.0;
                    }
                    colorMap(pDepth[count], &imgColorMap[count * 3], &imgColorMap[count * 3 + 1], &imgColorMap[count * 3 + 2]);
                }
                imgDepth[count * 3 + 0] = fX * 0.001;
                imgDepth[count * 3 + 1] = fY * 0.001;
                imgDepth[count * 3 + 2] = fZ * 0.001;
                count++;
            }
        }

        *vertexCount = count;
    }

    void _readVideoFeed()
    {
        VideoFrameRef rgbFrame;
        VideoFrameRef depthFrame;
        VideoFrameRef irFrame;

        bool rgbNewFrame = false;
        bool depthNewFrame = false;
        bool irNewFrame = false;

        if (!(videoStart))
            return;

        while (videoStart)
        {
            rgbNewFrame = false;
            depthNewFrame = false;
            irNewFrame = false;

            if (niRgbAvailable && enableRgb && vsColor.isValid())
            {
                if (vsColor.readFrame(&rgbFrame) == STATUS_OK)
                {
                    rgbTexture->cvImage = cv::Mat(rgbFrame.getHeight(), rgbFrame.getWidth(), CV_8UC3, (void *)rgbFrame.getData());
                    rgbTexture->pipeline->run(rgbTexture->cvImage, flRegistrar, rgbTexture->textureId, rgbTexture->videoWidth, rgbTexture->videoHeight, rgbTexture->buffer, models, flChannel);
                    rgbTexture->setPixelBuffer();
                    rgbNewFrame = true;
                }
            }

            if (niDepthAvailable && enableDepth && vsDepth.isValid())
            {
                if (vsDepth.readFrame(&depthFrame) == STATUS_OK)
                {
                    depthTexture->cvImage = cv::Mat(depthFrame.getHeight(), depthFrame.getWidth(), CV_16UC1, (void *)depthFrame.getData());
                    depthTexture->pipeline->run(depthTexture->cvImage, flRegistrar, depthTexture->textureId, depthTexture->videoWidth, depthTexture->videoHeight, depthTexture->buffer, models, flChannel);
                    depthTexture->setPixelBuffer();
                    depthNewFrame = true;
                }
            }

            if (niIrAvailable && enableIr && vsDepth.isValid())
            {
                if (vsIR.readFrame(&irFrame) == STATUS_OK)
                {
                    irTexture->cvImage = cv::Mat(irFrame.getHeight(), irFrame.getWidth(), CV_16UC1, (void *)irFrame.getData());
                    irTexture->pipeline->run(irTexture->cvImage, flRegistrar, irTexture->textureId, irTexture->videoWidth, irTexture->videoHeight, irTexture->buffer, models, flChannel);
                    irTexture->setPixelBuffer();
                    irNewFrame = true;
                }
            }

            if (enablePointCloud && niRgbAvailable && depthNewFrame && rgbNewFrame)
            {
                niComputeCloud(vsDepth, (const openni::DepthPixel *)depthFrame.getData(), (const openni::RGB888Pixel *)rgbFrame.getData(), glfl->modelPointCloud->vertices, glfl->modelPointCloud->colors, glfl->modelPointCloud->colorsMap, &glfl->modelPointCloud->vertexPoints);
            }

            flChannel->InvokeMethod("onNiFrame", nullptr, nullptr);
        }
    }
};
#endif