import 'dart:math';

import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:get/get.dart';

class HandlerController extends GetxController {
  DummyCamera? cam;
  int rgbTextureId = 0;
  String imgPath = '';

  HandlerController() {
    FvCamera.create("dummy", CameraType.DUMMY).then((c) => cam = c as DummyCamera);
  }

  Future<void> handler() async {
    if (cam == null || cam == null || imgPath.isEmpty) return;

    FvPipeline processPipeline = cam!.rgbPipeline;
    await processPipeline.clear();
    await processPipeline.imread(imgPath);
    await processPipeline.customHandler(3);
    await processPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await processPipeline.show();
    await processPipeline.run();

    update();
  }
}
