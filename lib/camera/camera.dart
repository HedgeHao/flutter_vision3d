import 'package:flutter_vision/flutter_vision.dart';

import '../flutter_vision.dart';
import 'openni.dart';
import 'realsense.dart';

class FvCamera {
  late final CameraType cameraType;

  static Future<FvCamera?> create(String serial, CameraType type) async {
    Map<Object?, Object?> result = await FlutterVision.channel.invokeMethod('fvCameraOpen', {'serial': serial, 'cameraType': type.index});

    print(result);

    Map<String, dynamic> flResult = {};
    flResult['ret'] = (result['ret'] ?? -1) as int;
    flResult['rgbTextureId'] = (result['rgbTextureId'] ?? 0) as int;
    flResult['depthTextureId'] = (result['depthTextureId'] ?? 0) as int;
    flResult['irTextureId'] = (result['irTextureId'] ?? 0) as int;

    if (flResult['ret'] == 0) {
      flResult['serial'] = serial;
      return type == CameraType.OPENNI ? OpenniCamera(flResult) : RealsenseCamera(flResult);
    }

    return null;
  }

  late String serial;
  late int rgbTextureId;
  late int depthTextureId;
  late int irTextureId;
  late bool isPointCloudEnabled;

  Future<void> close() async {
    return await FlutterVision.channel.invokeMethod('fvCameraClose', {'serial': serial, 'cameraType': cameraType.index});
  }

  Future<bool> enableStream({int videoModeIndex = 7}) async {
    return await FlutterVision.channel.invokeMethod('fvCameraConfigVideoStream', {'serial': serial, 'videoModeIndex': videoModeIndex, 'enable': true, 'cameraType': cameraType.index});
  }

  Future<bool> disableStream({int videoModeIndex = 7}) async {
    return await FlutterVision.channel.invokeMethod('fvCameraConfigVideoStream', {'serial': serial, 'videoModeIndex': videoModeIndex, 'enable': false, 'cameraType': cameraType.index});
  }

  Future<void> enablePointCloud() async {
    return await FlutterVision.channel.invokeMethod('fvCameraEnablePointCloud', {'serial': serial, 'enable': true, 'cameraType': cameraType.index});
  }

  Future<void> disablePointCloud() async {
    return await FlutterVision.channel.invokeMethod('fvCameraEnablePointCloud', {'serial': serial, 'enable': false, 'cameraType': cameraType.index});
  }

  Future<bool> isConnected() async {
    return true;
  }
}
