import 'dart:math';

import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:get/get.dart';

class OpencvController extends GetxController {
  final BUILDER_CONNECT_STATUS = 'BUILDER_CONNECT_STATUS';
  final BUILDER_TEXTURE = 'BUILDER_TEXTURE';

  DummyCamera? processCam;
  DummyCamera? originalCam;
  int processTextureId = 0;
  int originalTextureId = 0;
  String imgPath = '';
  String pipelineInfo = '';

  OpencvController() {
    Future.wait([
      FvCamera.create("origin", CameraType.DUMMY).then((c) {
        processCam = c as DummyCamera;
        processTextureId = processCam!.rgbTextureId;
      }),
      FvCamera.create("process", CameraType.DUMMY).then((c) {
        originalCam = c as DummyCamera;
        originalTextureId = originalCam!.rgbTextureId;
      })
    ]).then((value) => update());
  }

  Future<void> _loadImage() async {
    if (originalCam == null || processCam == null || imgPath.isEmpty) return;

    FvPipeline processPipeline = processCam!.rgbPipeline;
    await processPipeline.clear();
    await processPipeline.imread(imgPath);
    await processPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await processPipeline.show();
    await processPipeline.run();

    FvPipeline originalPipeline = originalCam!.rgbPipeline;
    await originalPipeline.clear();
    await originalPipeline.imread(imgPath);
    await originalPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await originalPipeline.show();
    await originalPipeline.run();
  }

  Future<void> loadImage() async {
    if (originalCam == null || processCam == null || imgPath.isEmpty) return;

    await _loadImage();

    FvPipeline processPipeline = processCam!.rgbPipeline;
    pipelineInfo = await processPipeline.info();
    update();
  }

  Future<void> cropImage() async {
    if (originalCam == null || processCam == null || imgPath.isEmpty) return;

    await _loadImage();

    FvPipeline processPipeline = processCam!.rgbPipeline;
    await processPipeline.crop(0, 300, 0, 100, at: 1, append: true);
    await processPipeline.run();

    pipelineInfo = await processPipeline.info();
    update();
  }

  Future<void> resize() async {
    if (originalCam == null || processCam == null || imgPath.isEmpty) return;

    await _loadImage();

    FvPipeline processPipeline = processCam!.rgbPipeline;
    await processPipeline.resize(100, 100, at: 1, append: true);
    await processPipeline.run();

    pipelineInfo = await processPipeline.info();
    update();
  }

  Future<void> colorMap() async {
    if (originalCam == null || processCam == null || imgPath.isEmpty) return;

    await _loadImage();

    FvPipeline processPipeline = processCam!.rgbPipeline;
    await processPipeline.convertTo(OpenCV.CV_8UC3, 1, at: 1, append: true);
    await processPipeline.applyColorMap(OpenCV.COLORMAP_JET, at: 2, append: true);
    await processPipeline.run();

    pipelineInfo = await processPipeline.info();
    update();
  }

  Future<void> replaceColorMap() async {
    if (originalCam == null || processCam == null || imgPath.isEmpty) return;

    await colorMap();

    FvPipeline processPipeline = processCam!.rgbPipeline;
    await processPipeline.applyColorMap(Random().nextInt(20), at: 2);
    await processPipeline.run();

    pipelineInfo = await processPipeline.info();
    update();
  }

  Future<void> drawRectangle() async {
    if (originalCam == null || processCam == null || imgPath.isEmpty) return;

    await _loadImage();

    FvPipeline processPipeline = processCam!.rgbPipeline;
    await processPipeline.cvRectangle(200, 30, 370, 140, 255, 0, 0, thickness: 2, at: 1, append: true);
    await processPipeline.run();

    pipelineInfo = await processPipeline.info();
    update();
  }

  Future<void> rotate() async {
    if (originalCam == null || processCam == null || imgPath.isEmpty) return;

    await _loadImage();

    FvPipeline processPipeline = processCam!.rgbPipeline;
    await processPipeline.rotate(OpenCV.ROTATE_90_CLOCKWISE, at: 1, append: true);
    await processPipeline.run();

    pipelineInfo = await processPipeline.info();
    update();
  }
}
