import 'package:flutter/material.dart';
import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/realsense.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:get/get.dart';

class RealsenseController extends GetxController {
  RealsenseController();

  static const BUILDER_DEVICE_LIST = 'BUILDER_DEVICE_LIST';
  static const BUILDER_TEXTURE = 'BUILDER_TEXTURE';
  static const BUILDER_SLIDER = 'BUILDER_SLIDER';

  RealsenseCamera? cam;
  int rgbTextureId = 0;
  int depthTextureId = 0;
  int irTextureId = 0;
  List<DropdownMenuItem<int>> items = [];
  double rangeFilterValueMin = 0.1;
  set minRange(double v) {
    if (v > rangeFilterValueMax) return;
    rangeFilterValueMin = v;
    cam!.configure(RealsenseConfiguration.THRESHOLD_FILTER.index, [rangeFilterValueMin, rangeFilterValueMax]);
    update([BUILDER_SLIDER]);
  }

  double rangeFilterValueMax = 4.0;
  set maxRange(double v) {
    if (v < rangeFilterValueMin) return;
    rangeFilterValueMax = v;
    cam!.configure(RealsenseConfiguration.THRESHOLD_FILTER.index, [rangeFilterValueMin, rangeFilterValueMax]);
    update([BUILDER_SLIDER]);
  }

  int selected = 0;
  List<String> deviceList = <String>[];
  String selectedRsDevice = '';

  Future<void> openRealsenseCamera() async {
    if (selectedRsDevice.isEmpty) return;

    cam = await FvCamera.create(selectedRsDevice, CameraType.REALSENSE) as RealsenseCamera?;
    if (cam == null) {
      print('Create Camera Failed');
      return;
    }

    update([BUILDER_TEXTURE, BUILDER_SLIDER]);
  }

  void closeRealsenseCamera() async {
    if (cam == null) {
      return;
    }

    await cam!.close();
  }

  void enableStreaming() async {
    if (cam == null) {
      return;
    }

    await cam!.enableStream();
  }

  void disableStreaming() async {
    if (cam == null) {
      return;
    }

    await cam!.disableStream();
  }

  void pipelineDisplay() async {
    FvPipeline uvcPipeline = cam!.rgbPipeline;
    await uvcPipeline.clear();
    await uvcPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await uvcPipeline.show();

    FvPipeline depthPipeline = cam!.depthPipeline;
    await depthPipeline.clear();
    await depthPipeline.convertTo(0, 255.0 / 1024.0);
    await depthPipeline.applyColorMap(OpenCV.COLORMAP_JET);
    await depthPipeline.cvtColor(0); //COLOR_RGB2RGBA
    await depthPipeline.show();

    update([BUILDER_TEXTURE]);
  }

  void selectDevice(int selectIndex) {
    selectedRsDevice = deviceList[selectIndex];
  }

  void enumerateDevice() async {
    deviceList = await FlutterVision.rsEnumerateDevices();

    if (deviceList.isNotEmpty) {
      selectedRsDevice = deviceList[0];
    }

    update([BUILDER_DEVICE_LIST]);
  }
}
