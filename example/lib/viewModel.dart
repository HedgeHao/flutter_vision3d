import 'dart:io';

import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/camera/openni.dart';
import 'package:flutter_vision/camera/realsense.dart';
import 'package:flutter_vision/camera/uvc.dart';
import 'package:flutter_vision/flutter_vision.dart';

ConfigurationViewModel configuration = ConfigurationViewModel()..init();

class VideoModeData {
  int index;
  String text;
  bool selected = false;
  bool initialized = false;
  bool start = false;

  VideoModeData(this.text, this.index);
}

class ConfigurationViewModel {
  List<RealsenseCamera> rsCams = [];
  List<OpenniCamera> niCams = [];
  List<DummyCamera> dummyCams = [];
  List<UvcCamera> uvcCams = [];
  OpenNi2Device? selectedDevice;
  String selectedRsDevice = '';
  int lastRequestTS = 0;
  late File serial;
  bool niPointCloud = false;
  bool rsPointCloud = false;
  late String TEST_IMAGE;
  late String MODEL_FACE_DETECTOR;
  late String MODEL_EFFECIENT_NET;

  List<VideoModeData> videoModes = [
    VideoModeData('RGB', 1),
    VideoModeData('Depth', 2),
    VideoModeData('IR', 4),
  ];

  void init() {
    if (Platform.isWindows) {
      TEST_IMAGE = 'D:/test/faces.jpg';
      MODEL_FACE_DETECTOR = 'D:/test/faceDetector.tflite';
      MODEL_EFFECIENT_NET = 'D:/test/efficientNet.tflite';
    } else {
      TEST_IMAGE = '/home/hedgehao/test/faces.jpg';
      MODEL_FACE_DETECTOR = '/home/hedgehao/test/faceDetector.tflite';
      MODEL_EFFECIENT_NET = '/home/hedgehao/test/efficientNet.tflite';
    }
  }

  void resetVideModesChecked() {
    for (var element in videoModes) {
      element.selected = false;
      element.initialized = false;
      element.start = false;
    }
  }

  void setVideoModesInitialized(mode) {
    for (var v in videoModes) {
      v.initialized = mode & v.index > 0;
    }
  }

  int getVideoModes() {
    int modes = 0;
    for (var v in videoModes) {
      if (v.selected) {
        modes += v.index;
      }
    }
    return modes;
  }
}
