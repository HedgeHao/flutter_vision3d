import 'package:flutter_vision/flutter_vision.dart';

import '../flutter_vision.dart';
import 'camera.dart';

class OpenniCamera extends FvCamera {
  OpenniCamera(Map<String, dynamic> m) {
    cameraType = CameraType.OPENNI;
    serial = m['serial'] as String;
    rgbTextureId = m['rgbTextureId'] as int;
    depthTextureId = m['depthTextureId'] as int;
    irTextureId = m['irTextureId'] as int;
  }
}
