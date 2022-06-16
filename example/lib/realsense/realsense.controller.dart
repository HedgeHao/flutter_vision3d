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

  List<RealsenseCamera> cams = [];
  int rgbTextureId = 0;
  int depthTextureId = 0;
  int irTextureId = 0;
  List<DropdownMenuItem<int>> items = [];

  int selected = 0;
  List<String> deviceList = <String>[];
  String selectedRsDevice = '';

  Future<void> openRealsenseCamera() async {
    if (selectedRsDevice.isEmpty) return;

    RealsenseCamera? cam = cams.firstWhereOrNull((e) => e.serial == selectedRsDevice.toString());

    if (cam == null) {
      cam = await FvCamera.create(selectedRsDevice, CameraType.REALSENSE) as RealsenseCamera?;
      if (cam == null) {
        print('Create Camera Failed');
        return;
      }
      cams.add(cam);
    }

    update([BUILDER_TEXTURE]);
  }

  void closeRealsenseCamera() async {
    RealsenseCamera? cam = cams.firstWhereOrNull((e) => e.serial == selectedRsDevice.toString());

    if (cam == null) {
      return;
    }

    await cam.close();
  }

  void enableStreaming() async {
    RealsenseCamera? cam = cams.firstWhereOrNull((e) => e.serial == selectedRsDevice.toString());

    if (cam == null) {
      return;
    }

    await cam.enableStream();
  }

  void disableStreaming() async {
    RealsenseCamera? cam = cams.firstWhereOrNull((e) => e.serial == selectedRsDevice.toString());

    if (cam == null) {
      return;
    }

    await cam.disableStream();
  }

  void pipelineDisplay() async {
    FvPipeline uvcPipeline = cams.first.rgbPipeline;
    await uvcPipeline.clear();
    await uvcPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await uvcPipeline.show();

    FvPipeline depthPipeline = cams.first.depthPipeline;
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
