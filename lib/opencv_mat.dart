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
}
