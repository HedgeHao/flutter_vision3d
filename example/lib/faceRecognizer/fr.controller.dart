import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/uvc.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision_example/faceRecognizer/LIPSFace.dart';
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

class FaceRecognizerController extends GetxController {
  FaceRecognizerController();

  @override
  void onInit() {
    FlutterVision.listen(fvCallback);
    super.onInit();
  }

  final MODEL_FACE_DETECTOR = '/home/hedgehao/test/faceDetector.tflite';

  List<UvcCamera> cams = [];
  List<PositionedRect> rects = [];
  int rgbTextureId = 0;

  TFLiteModel? model;

  Future<dynamic> fvCallback(MethodCall call) async {
    if (call.method == 'onInference') {
      Float32List output = await model!.getTensorOutput(0, [28, 28, 5]);
      List<FaceInfo> faces = processFaceDetectorOutputs(output, 320, 180);
      if (faces.isEmpty) return;

      faces = nms(faces, 0.3);
      List<PositionedRect> r = [];
      if (faces.isNotEmpty) {
        for (FaceInfo f in faces) {
          r.add(PositionedRect(f.x1, f.y1, f.x2 - f.x1, f.y2 - f.y1, Colors.red));
        }
      }

      rects = r;

      update();
    } else if (call.method == 'onUvcFrame') {
    } else if (call.method == 'onNiFrame') {
    } else if (call.method == 'onHandled') {}

    return;
  }

  Future<void> openUvcCamera(String serial) async {
    RxStatus.empty();
    UvcCamera? cam = cams.firstWhereOrNull((e) => e.serial == serial.toString());

    if (cam == null) {
      cam = await FvCamera.create(serial, CameraType.UVC) as UvcCamera?;
      if (cam == null) {
        print('Create Camera Failed');
        return;
      }
      cams.add(cam);
    }

    update();
  }

  void closeUvcCamera(String serial) async {
    UvcCamera? cam = cams.firstWhereOrNull((e) => e.serial == serial.toString());

    if (cam == null) {
      return;
    }

    await cam.close();
  }

  void enableStreaming(String serial) async {
    UvcCamera? cam = cams.firstWhereOrNull((e) => e.serial == serial.toString());

    if (cam == null) {
      return;
    }

    await cam.enableStream();
  }

  void disableStreaming(String serial) async {
    UvcCamera? cam = cams.firstWhereOrNull((e) => e.serial == serial.toString());

    if (cam == null) {
      return;
    }

    await cam.disableStream();
  }

  void frPipeline() async {
    if (cams.isEmpty) return;

    model ??= await TFLiteModel.create(MODEL_FACE_DETECTOR);

    FvPipeline rgbPipeline = cams.first.rgbPipeline;
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
