import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/camera/uvc.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision_example/define.dart';
import 'package:get/get.dart';

class PositionedRect extends StatelessWidget {
  final double top, left, width, height;
  final Color color;

  const PositionedRect(this.left, this.top, this.width, this.height, this.color, {Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Positioned(
        top: top,
        left: left,
        child: Container(
          width: width,
          height: height,
          decoration: BoxDecoration(border: Border.all(color: color)),
        ));
  }
}

class HandDetectionController extends GetxController {
  HandDetectionController();

  @override
  void onInit() {
    FlutterVision.listen(fvCallback);
    super.onInit();
  }

  FvCamera? cam;
  List<PositionedRect> rects = [];
  int rgbTextureId = 0;
  List<Offset> points = [];

  TFLiteModel? model;

  Future<dynamic> fvCallback(MethodCall call) async {
    if (call.method == 'onInference') {
      Float32List output = await model!.getTensorOutput(0, [63]);

      points.clear();
      for (int i = 0; i < 21; i++) {
        points.add(Offset(output[i * 3] * 640 / 224, output[i * 3 + 1] * 480 / 224));
      }

      update();
    } else if (call.method == 'onUvcFrame') {
    } else if (call.method == 'onNiFrame') {
    } else if (call.method == 'onHandled') {}

    return;
  }

  Future<void> openUvcCamera(String serial) async {
    RxStatus.empty();

    if (cam == null) {
      cam = await FvCamera.create(serial, CameraType.UVC) as UvcCamera?;
      if (cam == null) {
        print('Create Camera Failed');
        return;
      }
    }

    update();
  }

  void closeUvcCamera(String serial) async {
    await cam?.close();
  }

  void enableStreaming(String serial) async {
    await cam?.enableStream();
  }

  void disableStreaming(String serial) async {
    await cam?.disableStream();
  }

  void frPipeline() async {
    if (cam == null) return;

    if (!File(Define.HAND_DETECTOR_MODEL).existsSync()) {
      debugPrint('Model File Not found at: ${Define.HAND_DETECTOR_MODEL}');
      return;
    }

    model ??= await TFLiteModel.create(Define.HAND_DETECTOR_MODEL);

    FvPipeline rgbPipeline = cam!.rgbPipeline;
    await rgbPipeline.clear();
    await rgbPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await rgbPipeline.show();
    await rgbPipeline.resize(224, 224, mode: OpenCV.INTER_LINEAR);
    await rgbPipeline.cvtColor(OpenCV.COLOR_RGBA2RGB);
    await rgbPipeline.convertTo(OpenCV.CV_32FC3, 1.0 / 255.0);
    await rgbPipeline.setInputTensorData(model!.index, 0, FvPipeline.DATATYPE_FLOAT);
    await rgbPipeline.inference(model!.index, interval: 100);
  }
}
