import 'package:flutter_vision/flutter_vision.dart';
import '../flutter_vision.dart';
import 'camera.dart';

class RealsenseCamera extends FvCamera {
  RealsenseCamera(Map<String, dynamic> m) {
    cameraType = CameraType.REALSENSE;
    serial = m['serial'] as String;
    rgbTextureId = m['rgbTextureId'] as int;
    depthTextureId = m['depthTextureId'] as int;
    irTextureId = m['irTextureId'] as int;
  }
}
