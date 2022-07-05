import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/openni.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:get/get.dart';

class OpenNIController extends GetxController {
  OpenNIController() {
    FlutterVision.niInitialize();
  }

  static const BUILDER_DEVICE_LIST = 'BUILDER_DEVICE_LIST';
  static const BUILDER_TEXTURE = 'BUILDER_TEXTURE';
  static const BUILDER_TEXTURE_OPENGL = 'BUILDER_TEXTURE_OPENGL';
  static const BUILDER_SLIDER = 'BUILDER_SLIDER';

  OpenniCamera? cam;
  int rgbTextureId = 0;
  int depthTextureId = 0;
  int irTextureId = 0;
  int openglTextureId = 0;
  bool pointCloud = false;
  List<DropdownMenuItem<int>> items = [];

  int selected = 0;
  List<OpenNi2Device> deviceList = <OpenNi2Device>[];
  OpenNi2Device? selectedNiDevice;

  Future<void> openOpenNICamera() async {
    if (selectedNiDevice == null) return;

    cam = await FvCamera.create(selectedNiDevice!.uri, CameraType.OPENNI) as OpenniCamera?;
    if (cam == null) {
      print('Create Camera Failed');
      return;
    }

    update([BUILDER_TEXTURE, BUILDER_SLIDER]);
  }

  void closeOpenNICamera() async {
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

  void pipelineRGB() async {
    FvPipeline uvcPipeline = cam!.rgbPipeline;
    await uvcPipeline.clear();
    await uvcPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await uvcPipeline.show();

    update([BUILDER_TEXTURE]);
  }

  void pipelineDepth() async {
    FvPipeline depthPipeline = cam!.depthPipeline;
    await depthPipeline.clear();
    await depthPipeline.convertTo(0, 255.0 / 1024.0);
    await depthPipeline.applyColorMap(Random().nextInt(20));
    await depthPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
    await depthPipeline.show();

    update([BUILDER_TEXTURE]);
  }

  void selectDevice(int selectIndex) {
    selectedNiDevice = deviceList[selectIndex];
  }

  void enumerateDevice() async {
    deviceList = await FlutterVision.enumerateDevices();

    if (deviceList.isNotEmpty) {
      selectedNiDevice = deviceList[0];
    }

    update([BUILDER_DEVICE_LIST]);
  }

  void openglRender() {
    FlutterVision.openglRender().then((value) => {
          if (pointCloud) {Future.delayed(const Duration(milliseconds: 1), openglRender)}
        });
  }

  void enablePointCloud(bool value) async {
    if (cam == null) {
      pointCloud = false;
    } else {
      pointCloud = value;
      if (pointCloud) {
        cam!.enablePointCloud();
        openglRender();
      } else {
        cam!.disablePointCloud();
      }
    }

    openglTextureId = await FlutterVision.getOpenglTextureId();

    update([BUILDER_TEXTURE_OPENGL]);
  }
}
