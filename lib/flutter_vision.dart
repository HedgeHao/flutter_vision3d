import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';

import 'package:flutter/services.dart';
import 'package:flutter_vision/constants.dart';

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

  static Future<void> tfliteCreateModel(String modelPath) async {
    return await _channel.invokeMethod('tfliteCreateModel', {'modelPath': modelPath});
  }

  static Future<void> cameraOpen(int index) async {
    return await _channel.invokeMethod('cameraOpen', {'index': index});
  }

  static Future<void> cameraConfig(int index, bool start) async {
    return await _channel.invokeMethod('cameraConfig', {'index': index, 'start': start});
  }

  static Future<void> test() async {
    await _channel.invokeMethod('test');
    print('');
  }
}

const FUNC_TEST = 0;
const FUNC_CVTCOLOR = 1;
const FUNC_IMWRITE = 2;
const FUNC_SHOW = 3;
const FUNC_CONVERTO = 4;
const FUNC_APPLY_COLOR_MAP = 5;
const FUNC_RESIZE = 6;
const FUNC_CROP = 7;
const FUNC_IMREAD = 8;
const FUNC_CV_RECTANGLE = 9;

const FUNC_SET_INPUT_TENSOR = 0;
const FUNC_INFERENCE = 1;

// TODO: check method can be added to that pipeline
class LipsPipeline {
  static const RGB_FRAME = 0;
  static const DEPTH_FRAME = 1;
  static const IR_FRAME = 2;

  static const DATATYPE_UINT8 = 0;
  static const DATATYPE_FLOAT = 1;

  int index;

  LipsPipeline(this.index);

  Future<void> clear() async {
    await FlutterVision._channel.invokeMethod('pipelineClear', {'index': index});
  }

  Future<void> test(int t) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_TEST,
      'params': Uint8List.fromList([t]),
      'len': 1
    });
  }

  Future<void> cvtColor(int mode) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_CVTCOLOR,
      'params': Uint8List.fromList([mode]),
      'len': 1
    });
  }

  Future<void> imwrite(String path) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_IMWRITE,
      'params': Uint8List.fromList([path.length, ...utf8.encode(path)]),
      'len': path.length + 1
    });
  }

  Future<void> imread(String path) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_IMREAD,
      'params': Uint8List.fromList([path.length, ...utf8.encode(path)]),
      'len': path.length + 1
    });
  }

  Future<void> show() async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_SHOW,
      'params': null,
      'len': 0,
    });
  }

  Future<void> convertTo(int mode, double alpha) async {
    List<Object?> list = await FlutterVision._channel.invokeMethod("_float2uint8", {'value': alpha});
    Uint8List alphaBytes = Uint8List.fromList(list.map((e) => e as int).toList());

    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_CONVERTO,
      'params': Uint8List.fromList([mode, ...alphaBytes]),
      'len': 5
    });
  }

  Future<void> applyColorMap(int colorMap, {int? at}) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_APPLY_COLOR_MAP,
      'params': Uint8List.fromList([colorMap]),
      'len': 1,
      'at': at ?? -1
    });
  }

  Future<void> resize(int width, int height, {int? at, int? mode}) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_RESIZE,
      'params': Uint8List.fromList([(width >> 8) & 0xff, width & 0xff, (height >> 8) & 0xff, height & 0xff, mode ?? OpenCV.INTER_NEAREST]),
      'len': 5,
      'at': at ?? -1
    });
  }

  Future<void> crop(int xStart, int xEnd, int yStart, int yEnd, {int? at}) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_CROP,
      'params': Uint8List.fromList([xStart >> 8, xStart & 0xff, xEnd >> 8, xEnd & 0xff, yStart >> 8, yStart & 0xff, yEnd >> 8, yEnd & 0xff]),
      'len': 8,
      'at': at ?? -1
    });
  }

  Future<void> cvRectangle(double x1, double y1, double x2, double y2, int r, int g, int b, {int? at, int? thickness, int? lineType, int? shift, int? alpha}) async {
    Uint8List x1f = await FlutterVision._channel.invokeMethod("_float2uint8", {'value': x1});
    Uint8List y1f = await FlutterVision._channel.invokeMethod("_float2uint8", {'value': y1});
    Uint8List x2f = await FlutterVision._channel.invokeMethod("_float2uint8", {'value': x2});
    Uint8List y2f = await FlutterVision._channel.invokeMethod("_float2uint8", {'value': y2});

    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_CV_RECTANGLE,
      'params': Uint8List.fromList([...x1f, ...y1f, ...x2f, ...y2f, r, g, b, alpha ?? 255, thickness ?? 1, lineType ?? OpenCV.LINE_TYPE_LINE_8, shift ?? 0]),
      'len': 23,
      'at': at ?? -1
    });
  }

  Future<void> setInputTensorData(int frame, int tensorIndex, int dataType, {int? at}) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': FUNC_SET_INPUT_TENSOR,
      'params': Uint8List.fromList([frame, tensorIndex, dataType]),
      'len': 3,
      'at': at ?? -1
    });
  }

  Future<void> inference({int? at}) async {
    await FlutterVision._channel.invokeMethod('pipelineAdd', {'index': index, 'funcIndex': FUNC_INFERENCE, 'params': null, 'len': 0, 'at': at ?? -1});
  }
}

int _tflite_model_counter_ = 0;

class TFLiteModel {
  String modelPath;
  int index;

  TFLiteModel._create(this.modelPath, this.index);

  static Future<TFLiteModel> create(modelPath) async {
    await FlutterVision.tfliteCreateModel(modelPath);
    return TFLiteModel._create(modelPath, _tflite_model_counter_++);
  }

  Future<List> getTensorOutput(int tensorIndex, List<int> size) async {
    return await FlutterVision._channel.invokeMethod('tfliteGetTensorOutput', {'tensorIndex': tensorIndex, 'size': Int32List.fromList(size)}) as Float32List;
  }

  Future<dynamic> _getModelInfo(String key) async {
    Map<dynamic, dynamic> m = await FlutterVision._channel.invokeMethod('tfliteGetModelInfo', {'index': index});

    return m[key];
  }

  get valid async {
    return await _getModelInfo('valid') as bool;
  }

  get error async {
    return await _getModelInfo('error') as String;
  }
}

class OpenNiVideoMode {
  static const COLOR = 1;
  static const DEPTH = 2;
  static const IR = 4;
}
