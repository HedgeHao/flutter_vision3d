import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_vision/camera/camera.dart';
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

class EfficientNetController extends GetxController {
  static const frameWidth = 800.0;
  static const frameHeight = 600.0;

  EfficientNetController();

  @override
  void onInit() {
    FlutterVision.listen(fvCallback);
    super.onInit();
  }

  UvcCamera? cam;
  List<PositionedRect> rects = [];
  int rgbTextureId = 0;
  String displayText = '';

  TFLiteModel? model;

  Future<dynamic> fvCallback(MethodCall call) async {
    if (call.method == 'onInference') {
      Float32List outputBoxes = await model!.getTensorOutput(0, [25, 4]);
      Float32List outputClass = await model!.getTensorOutput(1, [25]);
      Float32List outputScore = await model!.getTensorOutput(2, [25]);

      if (outputScore[0] < 0.55) {
        rects.clear();
        displayText = '';
        update();
        return;
      }

      List<PositionedRect> r = [];
      for (int i = 0; i < 1; i++) {
        double x = frameWidth * outputBoxes[i * 4 + 1];
        double y = frameHeight * outputBoxes[i * 4];
        double width = frameWidth * outputBoxes[i * 4 + 3] - x;
        double height = frameHeight * outputBoxes[i * 4 + 2] - y;
        r.add(PositionedRect(x, y, width, height, Colors.red));
      }

      rects = r;
      displayText = '${COCO_CLASSES[outputClass[0].toInt()]} ${outputScore[0]}';

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
        debugPrint('Create Camera Failed');
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

  void setPipeline() async {
    if (cam == null) return;

    if (!File(Define.EFFICIENT_NET_MODEL).existsSync()) {
      debugPrint('Model File Not found at: ${Define.EFFICIENT_NET_MODEL}');
      return;
    }

    model ??= await TFLiteModel.create(Define.EFFICIENT_NET_MODEL);

    FvPipeline rgbPipeline = cam!.rgbPipeline;
    await rgbPipeline.clear();
    // await rgbPipeline.crop(100, 600, 100, 600);
    await rgbPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await rgbPipeline.show();
    await rgbPipeline.resize(320, 320, mode: OpenCV.INTER_CUBIC);
    await rgbPipeline.cvtColor(OpenCV.COLOR_RGBA2BGR);
    await rgbPipeline.setInputTensorData(model!.index, 0, FvPipeline.DATATYPE_UINT8);
    await rgbPipeline.inference(model!.index);
  }

  Future<void> deconstruct() async {
    await cam?.disableStream();
    await cam?.close();
    cam = null;
    update();
  }
}

const List<String> COCO_CLASSES = [
  "person",
  "bicycle",
  "car",
  "motorcycle",
  "airplane",
  "bus",
  "train",
  "truck",
  "boat",
  "traffic light",
  "fire hydrant",
  "street sign",
  "stop sign",
  "parking meter",
  "bench",
  "bird",
  "cat",
  "dog",
  "horse",
  "sheep",
  "cow",
  "elephant",
  "bear",
  "zebra",
  "giraffe",
  "hat",
  "backpack",
  "umbrella",
  "shoe",
  "eye glasses",
  "handbag",
  "tie",
  "suitcase",
  "frisbee",
  "skis",
  "snowboard",
  "sports ball",
  "kite",
  "baseball bat",
  "baseball glove",
  "skateboard",
  "surfboard",
  "tennis racket",
  "bottle",
  "plate",
  "wine glass",
  "cup",
  "fork",
  "knife",
  "spoon",
  "bowl",
  "banana",
  "apple",
  "sandwich",
  "orange",
  "broccoli",
  "carrot",
  "hot dog",
  "pizza",
  "donut",
  "cake",
  "chair",
  "couch",
  "potted plant",
  "bed",
  "mirror",
  "dining table",
  "window",
  "desk",
  "toilet",
  "door",
  "tv",
  "laptop",
  "mouse",
  "remote",
  "keyboard",
  "cell phone",
  "microwave",
  "oven",
  "toaster",
  "sink",
  "refrigerator",
  "blender",
  "book",
  "clock",
  "vase",
  "scissors",
  "teddy bear",
  "hair drier",
  "toothbrush",
  "hair brush"
];
