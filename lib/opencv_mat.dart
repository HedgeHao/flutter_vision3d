import 'package:flutter_vision/flutter_vision.dart';

class OpencvMatShape {
  int cols;
  int rows;
  int channels;

  OpencvMatShape({required this.cols, required this.rows, required this.channels});

  @override
  String toString() => '[$cols x $rows x $channels]';
}

class OpencvMat {
  int pointer = 0;

  static Future<OpencvMat> create() async {
    OpencvMat mat = OpencvMat();
    mat.pointer = await FlutterVision.channel.invokeMethod('cvCreateMat');
    return mat;
  }

  Future<OpencvMatShape> shape() async {
    var shape = await FlutterVision.channel.invokeMethod('cvGetShape', {'imagePointer': pointer});
    return OpencvMatShape(cols: shape['cols'], rows: shape['rows'], channels: shape['channels']);
  }

  Future<int> copyTo({OpencvMat? matB, int? matBPointer}) async {
    if (matB == null && matBPointer == null) return -1;

    int pointerB = (matB != null ? matB.pointer : matBPointer!);
    return await FlutterVision.channel.invokeMethod('cvCopyTo', {'imagePointerA': pointer, 'imagePointerB': pointerB});
  }

  Future<int> subtract({OpencvMat? matB, OpencvMat? matDest, int? matBPointer, int? matDestPointer}) async {
    if ((matB == null && matBPointer == null) || (matDest == null && matDestPointer == null)) return -1;

    int pointerB = (matB != null ? matB.pointer : matBPointer!);
    int pointerDest = (matDest != null) ? matDest.pointer : matDestPointer!;

    return await FlutterVision.channel.invokeMethod('cvSubtract', {'imagePointerA': pointer, 'imagePointerB': pointerB, 'imagePointerDest': pointerDest});
  }

  Future<int> threshold({OpencvMat? matDest, int? matDestPointer, required double min, required double max, required int type}) async {
    if (matDest == null && matDestPointer == null) return -1;
    int pointerDest = (matDest != null) ? matDest.pointer : matDestPointer!;
    return await FlutterVision.channel.invokeMethod('cvThreshold', {'imagePointerA': pointer, 'imagePointerDest': pointerDest, 'min': min, 'max': max, 'type': type});
  }

  Future<int> countNonZero() async {
    return await FlutterVision.channel.invokeMethod('cvCountNonZero', {'imagePointerA': pointer});
  }
}
