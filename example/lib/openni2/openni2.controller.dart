import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter_vision3d/camera/camera.dart';
import 'package:flutter_vision3d/camera/openni.dart';
import 'package:flutter_vision3d/constants.dart';
import 'package:flutter_vision3d/flutter_vision3d.dart';
import 'package:get/get.dart';

class OpenNIController extends GetxController {
  OpenNIController() {
    FlutterVision3d.niInitialize();
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
  bool registration = false;
  List<DropdownMenuItem<int>> items = [];
  List<DropdownMenuItem<int>> videoModeItems = const [
    DropdownMenuItem(child: Text('RGB'), value: StreamIndex.RGB),
    DropdownMenuItem(child: Text('Depth'), value: StreamIndex.DEPTH),
    DropdownMenuItem(child: Text('IR'), value: StreamIndex.IR),
  ];
  double fx = 0, fy = 0, cx = 0, cy = 0;
  String videoModes = '';
  String currentModeRGB = '';
  String currentModeDepth = '';
  String currentModeIR = '';
  String sn = '';

  int selected = 0;
  int selectedVideoModeItem = 1;
  List<OpenNi2Device> deviceList = <OpenNi2Device>[];
  OpenNi2Device? selectedNiDevice;

  TextEditingController videoModeCtl = TextEditingController();

  Future<void> openOpenNICamera() async {
    if (selectedNiDevice == null) return;

    cam = await FvCamera.create(selectedNiDevice!.uri, CameraType.OPENNI) as OpenniCamera?;
    if (cam == null) {
      print('Create Camera Failed');
      return;
    }

    sn = await cam!.getSerialNumber();
    print(sn);

    update();
  }

  void closeOpenNICamera() async {
    await cam?.close();
  }

  void enableStreaming() async {
    await cam?.enableStream();
  }

  void disableStreaming() async {
    await cam?.disableStream();
  }

  void pipelineRGB() async {
    FvPipeline uvcPipeline = cam!.rgbPipeline;
    await uvcPipeline.clear();
    await uvcPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
    await uvcPipeline.show();

    update([BUILDER_TEXTURE]);
  }

  void pipelineDepth() async {
    FvPipeline depthPipeline = cam!.depthPipeline;
    await depthPipeline.clear();
    await depthPipeline.convertTo(OpenCV.CV_8U, 255.0 / 1024.0);
    await depthPipeline.applyColorMap(Random().nextInt(11));
    await depthPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
    await depthPipeline.show();

    update([BUILDER_TEXTURE]);
  }

  void pipelineIr() async {
    FvPipeline irPipeline = cam!.irPipeline;
    await irPipeline.clear();
    await irPipeline.convertTo(OpenCV.CV_8U, 255.0 / 1024.0);
    // await irPipeline.threshold(50, 255, type: OpenCV.THRESH_TOZERO_INV);
    // await irPipeline.normalize(0, 255, normType: OpenCV.NORM_MINMAX);
    await irPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
    await irPipeline.show();

    update([BUILDER_TEXTURE]);
  }

  void selectDevice(int selectIndex) {
    selectedNiDevice = deviceList[selectIndex];
  }

  void enumerateDevice() async {
    deviceList = await FlutterVision3d.enumerateDevices();

    if (deviceList.isNotEmpty) {
      selectedNiDevice = deviceList[0];
    }

    update([BUILDER_DEVICE_LIST]);
  }

  void openglRender() {
    FlutterVision3d.openglRender().then((value) => {
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

    openglTextureId = await FlutterVision3d.getOpenglTextureId();

    update([BUILDER_TEXTURE_OPENGL]);
  }

  void enableRegistration(bool value) async {
    if (cam == null) return;

    cam!.enableRegistration(value);

    update();
  }

  Future<void> deconstruct() async {
    await cam?.disableStream();
    await cam?.close();
    cam = null;
    update([BUILDER_TEXTURE]);
  }

  Future<void> getIntrinsic() async {
    if (cam == null) return;

    Map<String, double> param = await cam!.getIntrinsic(2);
    fx = param['fx']!;
    fy = param['fy']!;
    cx = param['cx']!;
    cy = param['cy']!;

    update();
  }

  Future<void> getVideoModes(int index) async {
    if (cam == null) return;

    videoModes = (await cam!.getVideoModes(index)).join('\n');

    update();
  }

  Future<void> setVideoMode() async {
    if (cam == null) return;

    await cam!.setVideMode(selectedVideoModeItem, int.parse(videoModeCtl.text));

    getCurrentVideoMode();
  }

  Future<void> getCurrentVideoMode() async {
    currentModeRGB = await cam!.getCurrentVideoMode(StreamIndex.RGB);
    currentModeDepth = await cam!.getCurrentVideoMode(StreamIndex.DEPTH);
    currentModeIR = await cam!.getCurrentVideoMode(StreamIndex.IR);

    update();
  }

  Future<void> videoPause(bool pause) async {
    await cam?.pauseStream(pause);
  }

  Future<void> test() async {
    var data = await cam?.getDepthData(DepthType.AT, x: 320, y: 200);
    print(data?[0]);
  }
}
