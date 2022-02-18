import 'dart:async';
import 'dart:io';

import 'package:flutter/services.dart';

class OpenNi2Status {
  // OpenNI2
  static const int STATUS_OK = 0;
  static const int STATUS_ERROR = 1;
  static const int STATUS_NOT_IMPLEMENTED = 2;
  static const int STATUS_NOT_SUPPORTED = 3;
  static const int STATUS_BAD_PARAMETER = 4;
  static const int STATUS_OUT_OF_FLOW = 5;
  static const int STATUS_NO_DEVICE = 6;
  static const int STATUS_TIME_OUT = 102;
}

class OpenNi2Device {
  String? name;
  String? vendor;
  int? productId;
  int? vendorId;
  String? uri;

  OpenNi2Device({
    this.name,
    this.vendor,
    this.productId,
    this.vendorId,
    this.uri,
  });
  OpenNi2Device.fromJson(Map<dynamic, dynamic> json) {
    name = json['name']?.toString();
    vendor = json['vendor']?.toString();
    productId = json['productId']?.toInt();
    vendorId = json['vendorId']?.toInt();
    uri = json['uri']?.toString();
  }
  Map<String, dynamic> toJson() {
    final data = <String, dynamic>{};
    data['name'] = name;
    data['vendor'] = vendor;
    data['productId'] = productId;
    data['vendorId'] = vendorId;
    data['uri'] = uri;
    return data;
  }
}

enum OPENNI_DEVICE_STATUS { INVALID, VALID }

class FlutterVision {
  static const MethodChannel _channel = MethodChannel('flutter_vision');
  static bool openglIsRendering = false;

  static listen(Future<dynamic> Function(MethodCall) callback) {
    _channel.setMethodCallHandler(callback);
  }

  static Future<int> initialize() async {
    return await _channel.invokeMethod('ni2Initialize') ?? OpenNi2Status.STATUS_ERROR;
  }

  static Future<List<OpenNi2Device>> enumerateDevices() async {
    List<Object?> list = await _channel.invokeMethod('ni2EnumerateDevices');

    List<OpenNi2Device> deviceList = <OpenNi2Device>[];

    deviceList.addAll(list.map((e) => OpenNi2Device.fromJson(e as Map<dynamic, dynamic>)));

    return deviceList;
  }

  static Future<int> openDevice(OpenNi2Device device, {int? videoMode}) async {
    int ret = await _channel.invokeMethod('ni2OpenDevice', {'uri': device.uri, 'videoMode': videoMode ?? 7});
    // if (ret == 1) print('SN not valid');

    return ret;
  }

  static Future<void> closeDevice() async {
    await _channel.invokeMethod('ni2CloseDevice');
  }

  static Future<bool> deviceIsConnected() async {
    return await _channel.invokeMethod('ni2DeviceIsConnected');
  }

  static Future<int> getEnabledVideoModes() async {
    return await _channel.invokeMethod('ni2GetEnabledVideoModes');
  }

  static Future<bool> configVideoStream(int videoModeIndex, bool enable) async {
    return await _channel.invokeMethod('ni2ConfigVideoStream', {'videoMode': videoModeIndex, 'enable': enable});
  }

  static Future<int> getVideoTextureId(int videoModeIndex) async {
    return await _channel.invokeMethod('ni2GetVideoTexture', {'videoIndex': videoModeIndex});
  }

  static Future<int> getVideoFramePointer(int videoModeIndex) async {
    return await _channel.invokeMethod('ni2GetFramePointer', {'videoIndex': videoModeIndex});
  }

  static Future<void> setVideoSize(int videoIndex, int width, int height) async {
    await _channel.invokeMethod('ni2SetVideoSize', {'videoIndex': videoIndex, 'width': width, 'height': height});
  }

  static Future<void> openglSetCamPosition(double x, double y, double z) async {
    return await _channel.invokeMethod('openglSetCamPosition', {'x': x, 'y': y, 'z': z});
  }

  static Future<void> openglSetCamAngle(double yaw, double pitch) async {
    return await _channel.invokeMethod('openglSetCamAngle', {'yaw': yaw, 'pitch': pitch});
  }

  static Future<void> openglSetCamFov(double fov) async {
    return await _channel.invokeMethod('openglSetCamFov', {'fov': fov});
  }

  static Future<void> openglRender() async {
    return await _channel.invokeMethod('openglRender');
  }

  static Future<void> test() async {
    await _channel.invokeMethod('test');
    print('');
  }
}

class OpenNiVideoMode {
  static const COLOR = 1;
  static const DEPTH = 2;
  static const IR = 4;
}
