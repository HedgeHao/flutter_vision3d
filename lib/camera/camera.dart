import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision/camera/uvc.dart';
import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/camera/openni.dart';
import 'package:flutter_vision/camera/realsense.dart';

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
    flResult['cameraType'] = type.index;

    if (flResult['ret'] == 0) {
      flResult['serial'] = serial;

      if (type == CameraType.OPENNI) {
        return OpenniCamera(flResult);
      } else if (type == CameraType.REALSENSE) {
        return RealsenseCamera(flResult);
      } else if (type == CameraType.DUMMY) {
        return DummyCamera(flResult);
      } else if (type == CameraType.UVC) {
        return UvcCamera(flResult);
      }
    }

    return null;
  }

  late final FvPipeline rgbPipeline;
  late final FvPipeline depthPipeline;
  late final FvPipeline irPipeline;

  late String serial;
  late int rgbTextureId;
  late int depthTextureId;
  late int irTextureId;
  late bool isPointCloudEnabled;

  FvCamera(Map<String, dynamic> m) {
    cameraType = CameraType.values[m['cameraType']];
    serial = m['serial'] as String;
    rgbTextureId = m['rgbTextureId'] as int;
    depthTextureId = m['depthTextureId'] as int;
    irTextureId = m['irTextureId'] as int;

    rgbPipeline = new FvPipeline(serial, FvPipeline.RGB_FRAME);
    depthPipeline = new FvPipeline(serial, FvPipeline.DEPTH_FRAME);
    irPipeline = new FvPipeline(serial, FvPipeline.IR_FRAME);
  }

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

  Future<void> configure(int prop, double value) async {
    return await FlutterVision.channel.invokeMethod('fvCameraConfig', {'prop': prop, 'value': value, 'serial': serial});
  }
}
