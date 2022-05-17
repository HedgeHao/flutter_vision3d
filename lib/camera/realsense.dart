import 'package:flutter_vision/flutter_vision.dart';

class RealsenseCamera {
  static Future<RealsenseCamera?> create(String serial) async {
    Map<Object?, Object?> result = await FlutterVision.channel.invokeMethod('rsOpenDevice', {'serial': serial});

    Map<String, dynamic> flResult = {};
    flResult['ret'] = (result['ret'] ?? -1) as int;
    flResult['rgbTextureId'] = (result['rgbTextureId'] ?? 0) as int;
    flResult['depthTextureId'] = (result['depthTextureId'] ?? 0) as int;
    flResult['irTextureId'] = (result['irTextureId'] ?? 0) as int;

    if (flResult['ret'] == 0) {
      flResult['serial'] = serial;
      return RealsenseCamera(flResult);
    }

    return null;
  }

  late String serial;
  late int rgbTextureId;
  late int depthTextureId;
  late int irTextureId;
  late bool isPointCloudEnabled;

  RealsenseCamera(Map<String, dynamic> m) {
    serial = m['serial'] as String;
    rgbTextureId = m['rgbTextureId'] as int;
    depthTextureId = m['depthTextureId'] as int;
    irTextureId = m['irTextureId'] as int;
  }

  Future<void> close() async {
    return await FlutterVision.channel.invokeMethod('rsCloseDevice', {'serial': serial});
  }

  Future<bool> enableStream({int videoModeIndex = 7}) async {
    return await FlutterVision.channel.invokeMethod('rsConfigVideoStream', {'serial': serial, 'videoModeIndex': videoModeIndex, 'enable': true});
  }

  Future<bool> disableStream({int videoModeIndex = 7}) async {
    return await FlutterVision.channel.invokeMethod('rsConfigVideoStream', {'serial': serial, 'videoModeIndex': videoModeIndex, 'enable': false});
  }

  Future<void> enablePointCloud() async {
    return await FlutterVision.channel.invokeMethod('rsEnablePointCloud', {'serial': serial, 'enable': true});
  }

  Future<void> disablePointCloud() async {
    return await FlutterVision.channel.invokeMethod('rsEnablePointCloud', {'serial': serial, 'enable': false});
  }

  Future<bool> isConnected() async {
    return true;
  }
}
