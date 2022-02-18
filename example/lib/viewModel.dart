import 'dart:io';

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
  OpenNi2Device? selectedDevice;
  int lastRequestTS = 0;
  late File serial;

  List<VideoModeData> videoModes = [
    VideoModeData('RGB', 1),
    VideoModeData('Depth', 2),
    VideoModeData('IR', 4),
  ];

  void init() {}

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
