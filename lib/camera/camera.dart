import 'dart:typed_data';

import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision/camera/uvc.dart';
import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/camera/openni.dart';
import 'package:flutter_vision/camera/realsense.dart';

class FvCamera {
  late final CameraType cameraType;

  static Future<FvCamera?> create(String serial, CameraType type) async {
    if (type == CameraType.UVC && int.tryParse(serial) == null) {
      print('UVC index should be integer.');
      return null;
    }

    Map<Object?, Object?> result = await FlutterVision.channel.invokeMethod('fvCameraOpen', {'serial': serial, 'cameraType': type.index});

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

    rgbPipeline = FvPipeline(serial, FvPipeline.RGB_FRAME);
    depthPipeline = FvPipeline(serial, FvPipeline.DEPTH_FRAME);
    irPipeline = FvPipeline(serial, FvPipeline.IR_FRAME);
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

  Future<bool> configure(int prop, List<double> value) async {
    return await FlutterVision.channel.invokeMethod('fvCameraConfig', {'prop': prop, 'value': Float32List.fromList(value), 'serial': serial});
  }

  Future<int> getConfiguration(int prop) async {
    return await FlutterVision.channel.invokeMethod('fvCameraGetConfiguration', {'prop': prop, 'serial': serial});
  }

  Future<bool> screenshot(int index, String path, {int? cvtCode}) async {
    return await FlutterVision.channel.invokeMethod('fvCameraScreenshot', {'index': index, 'path': path, 'cvtCode': cvtCode ?? -1, 'serial': serial});
  }
}
