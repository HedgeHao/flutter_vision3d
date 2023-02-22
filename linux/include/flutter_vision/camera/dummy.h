#ifndef _DEF_DUMMY_CAM_
#define _DEF_DUMMY_CAM_
#include "fv_camera.h"

class DummyCam : public FvCamera
{
public:
  DummyCam(const char *s) : FvCamera(s){};

  int camInit() { return 0; }

  int openDevice() { return 0; }

  int closeDevice() { return 0; }

  int isConnected() { return 0; }

  int configVideoStream(int streamIndex, bool *enable) { return 0; }

  int readVideoFeed() { return 0; }

  int configure(int prop, std::vector<float> &value) { return 0; }

  int getConfiguration(int prop) { return 0; }

  void getIntrinsic(int index, double &fx, double &fy, double &cx, double &cy) {}

  bool enableImageRegistration(bool enable) { return true; }

  void getAvailableVideoModes(int index, std::vector<std::string> &rModes) {}

  void getCurrentVideoMode(int index, std::string &mode) {}

  bool setVideoMode(int index, int mode) { return true; }

  bool getSerialNumber(std::string &sn) { return true; }

  void loadPresetParameters(std::string &path) {}

private:
  int _readVideoFeed() { return 0; }
};
#endif