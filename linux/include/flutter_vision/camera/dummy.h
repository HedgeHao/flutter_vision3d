#ifndef _DEF_DUMMY_CAM_
#define _DEF_DUMMY_CAM_
#include "fv_camera.h"

using namespace openni;

class DummyCam : public FvCamera
{
public:
  DummyCam(const char *s) : FvCamera(s){};

  void camInit() {}

  int openDevice() { return 0; }

  int closeDevice() { return 0; }

  int isConnected() { return 0; }

  int configVideoStream(int streamIndex, bool *enable) { return 0; }

  void readVideoFeed() {}

  void configure(int prop, std::vector<float> &value) {}

private:
  void _readVideoFeed() {}
};
#endif