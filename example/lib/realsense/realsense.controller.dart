import 'dart:async';
import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/realsense.dart';
import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision/opencv_mat.dart';
import 'package:get/get.dart';

class RealsenseController extends GetxController {
  static const BUILDER_DEVICE_LIST = 'BUILDER_DEVICE_LIST';
  static const BUILDER_TEXTURE = 'BUILDER_TEXTURE';
  static const BUILDER_TEXTURE_OPENGL = 'BUILDER_TEXTURE_OPENGL';
  static const BUILDER_SLIDER = 'BUILDER_SLIDER';
  static const BUILDER_RELU_SLIDER = 'BUILDER_RELU_SLIDER';
  static const BUILDER_DEPTH_FILTER = 'BUILDER_DEPTH_FILTER';
  static const BUILDER_TEXTURE_PROCESS_CAM = 'BUILDER_TEXTURE_PROCESS_CAM';
  static const BUILDER_VOLUME = 'BUILDER_VOLUME';

  @override
  void onInit() {
    FvCamera.create("process", CameraType.DUMMY).then((c) async {
      matCamera = c as DummyCamera;
      processTextureId = matCamera!.rgbTextureId;
      matCameraPointer = await matCamera!.getOpenCVMat(StreamIndex.RGB);
      update([BUILDER_TEXTURE_PROCESS_CAM]);
    });

    OpencvMat.create().then((mat) => depthBaseline = mat);
    OpencvMat.create().then((mat) => processMat = mat);

    super.onInit();
  }

  RealsenseCamera? cam;
  DummyCamera? matCamera;
  late OpencvMat depthBaseline;
  late OpencvMat processMat;
  String currentModeRGB = '';
  String currentModeDepth = '';
  String currentModeIR = '';
  String sn = '';
  String volume = '';
  double fx = 0, fy = 0, cx = 0, cy = 0;
  int rgbTextureId = 0;
  int depthTextureId = 0;
  int irTextureId = 0;
  int openglTextureId = 0;
  int processTextureId = 0;
  int matCameraPointer = 0;
  bool pointCloud = false;
  bool depthFilter = false;
  List<DropdownMenuItem<int>> items = [];
  double rangeFilterValueMin = 0.1;
  set minRange(double v) {
    if (cam == null) return;
    if (v > rangeFilterValueMax) return;
    rangeFilterValueMin = v;
    cam!.configure(RealsenseConfiguration.THRESHOLD_FILTER.index, [rangeFilterValueMin, rangeFilterValueMax]);
    update([BUILDER_SLIDER]);
  }

  double rangeFilterValueMax = 4.0;
  set maxRange(double v) {
    if (cam == null) return;
    if (v < rangeFilterValueMin) return;
    rangeFilterValueMax = v;
    cam!.configure(RealsenseConfiguration.THRESHOLD_FILTER.index, [rangeFilterValueMin, rangeFilterValueMax]);
    update([BUILDER_SLIDER]);
  }

  double reluThresholdValue = 1.0;
  set reluThreshold(double v) {
    if (cam == null) return;
    reluThresholdValue = v;
    update([BUILDER_RELU_SLIDER]);
  }

  updateReluThreshold(double v) async {
    FvPipeline depthPipeline = cam!.depthPipeline;
    await depthPipeline.relu(v, at: 1);
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

    sn = await cam!.getSerialNumber();
    await cam!.enableRegistration(true);

    update();
  }

  void closeRealsenseCamera() async {
    await cam?.close();
  }

  Future<bool> enableStreaming() async {
    return await cam?.enableStream() ?? false;
  }

  void disableStreaming() async {
    await cam?.disableStream();
  }

  Future<void> videoPause(bool pause) async {
    await cam?.pauseStream(pause);
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
    await depthPipeline.crop(320, 960, 180, 540);
    await depthPipeline.convertTo(OpenCV.CV_8U, 255.0 / 1024.0);
    // await depthPipeline.relu(reluThresholdValue);
    await depthPipeline.applyColorMap(Random().nextInt(11));
    await depthPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
    await depthPipeline.show();

    update([BUILDER_TEXTURE]);
  }

  void pipelineIr() async {
    FvPipeline irPipeline = cam!.irPipeline;
    await irPipeline.clear();
    await irPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
    await irPipeline.show();

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

  Future<void> getCurrentVideoMode() async {
    if (cam == null) {
      print('Camera is not connected');
      return;
    }

    currentModeRGB = await cam!.getCurrentVideoMode(StreamIndex.RGB);
    currentModeDepth = await cam!.getCurrentVideoMode(StreamIndex.DEPTH);
    currentModeIR = await cam!.getCurrentVideoMode(StreamIndex.IR);

    update();
  }

  Future<void> enableDepthFilter(bool enable) async {
    FvPipeline depthPipeline = cam!.depthPipeline;
    depthFilter = enable;
    if (enable) {
      await depthPipeline.zeroDepthFilter(0, 5, at: 2, append: true);
    } else {
      await depthPipeline.removeAt(2);
    }

    update([BUILDER_DEPTH_FILTER]);
  }

  Future<void> screenshot() async {
    FvPipeline rgbPipeline = cam!.rgbPipeline;
    await rgbPipeline.imwrite("test.jpg", at: 0, append: true, runOnce: true);
  }

  Future<void> _displayProcessFrame() async {
    FvPipeline processPipeline = matCamera!.rgbPipeline;
    await processPipeline.clear();
    await processPipeline.applyColorMap(OpenCV.COLORMAP_JET);
    await processPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await processPipeline.show();
    await processPipeline.run();

    processTextureId = matCamera!.rgbTextureId;
    update([BUILDER_TEXTURE_PROCESS_CAM]);
  }

  Future<void> setDepthBaseline() async {
    FvPipeline depthPipeline = cam!.depthPipeline;
    await depthPipeline.copyTo(depthBaseline, at: 2, append: true, runOnce: true);

    bool finished = await depthPipeline.waitUntilFinished();

    if (finished) {
      await depthBaseline.copyTo(matBPointer: matCameraPointer);

      await _displayProcessFrame();
    } else {
      print('timeout');
    }
  }

  Future<void> volumeMetric() async {
    FvPipeline depthPipeline = cam!.depthPipeline;
    await depthPipeline.copyTo(processMat, at: 2, append: true, runOnce: true);
    bool finished = await depthPipeline.waitUntilFinished();

    if (finished) {
      await depthBaseline.subtract(matB: processMat, matDest: processMat);
      await processMat.threshold(matDest: processMat, min: 5, max: 255, type: OpenCV.THRESH_BINARY);
      await processMat.copyTo(matBPointer: matCameraPointer);
      await _displayProcessFrame();

      OpencvMatShape shape = await processMat.shape();
      int nonZero = await processMat.countNonZero();

      volume = (nonZero / (shape.rows * shape.cols) * 100).toStringAsFixed(2);
      update([BUILDER_VOLUME]);
    }
  }
}
