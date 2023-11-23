import 'dart:typed_data';

import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/camera/openni.dart';
import 'package:flutter_vision/camera/realsense.dart';
import 'package:flutter_vision/camera/ros_camera.dart';
import 'package:flutter_vision/camera/uvc.dart';
import 'package:flutter_vision/flutter_vision.dart';

enum DepthType { ALL, AT, RANGE }

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
      } else if (type == CameraType.ROS) {
        return RosCamera(flResult);
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

  Future<int> getOpenCVMat(int index) async {
    return await FlutterVision.channel.invokeMethod('fvGetOpenCVMat', {'index': index, 'serial': serial});
  }

  Future<Map<String, double>> getIntrinsic(int index) async {
    Map<dynamic, dynamic> map = await FlutterVision.channel.invokeMethod('fvGetIntrinsic', {'index': index, 'serial': serial});

    return <String, double>{'fx': map['fx'], 'fy': map['fy'], 'cx': map['cx'], 'cy': map['cy']};
  }

  Future<bool> enableRegistration(bool enable) async {
    return await FlutterVision.channel.invokeMethod('fvEnableRegistration', {'enable': enable, 'serial': serial});
  }

  Future<bool> pauseStream(bool pause) async {
    return await FlutterVision.channel.invokeMethod('fvPauseStream', {'pause': pause, 'serial': serial});
  }

  Future<List<String>> getVideoModes(int index) async {
    List<Object?> modes = await FlutterVision.channel.invokeMethod('ni2GetAvailableVideoModes', {'index': index, 'serial': serial});
    return modes.map((e) => e.toString()).toList();
  }

  Future<bool> setVideMode(int index, int mode) async {
    return await FlutterVision.channel.invokeMethod('ni2SetVideoMode', {'index': index, 'mode': mode, 'serial': serial});
  }

  Future<String> getCurrentVideoMode(int index) async {
    return await FlutterVision.channel.invokeMethod('ni2GetCurrentVideoMode', {'index': index, 'serial': serial});
  }

  Future<String> getSerialNumber() async {
    return await FlutterVision.channel.invokeMethod('fvGetSerialNumber', {'serial': serial});
  }

  Future<void> loadPresetParameter(String path) async {
    return await FlutterVision.channel.invokeMethod('rsLoadPresetParameters', {'serial': serial, 'path': path});
  }

  Future<List<int>> getDepthData(DepthType depthType, {int? x, int? y, int? width, int? height}) async {
    if (depthType == DepthType.AT && (x == null || y == null)) {
      throw Exception("coordinate not provide");
    } else if (depthType == DepthType.RANGE && (x == null || y == null || width == null || height == null)) {
      throw Exception("roi not provide");
    }

    List<Object?> data = await FlutterVision.channel.invokeMethod('fvGetDepthData', {'serial': serial, 'index': depthType.index, 'x': x, 'y': y, 'roi_width': width, 'roi_height': height});
    List<int> rData = data.map((e) => e as int).toList();

    return rData;
  }

  Future<void> test(int rgbPointer, int depthPointer, int irPointer) async {}
}
