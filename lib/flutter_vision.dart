import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/services.dart';
import 'package:flutter_vision/constants.dart';

enum CameraType { OPENNI, REALSENSE, DUMMY, UVC }

class StreamIndex {
  static const RGB = 1;
  static const DEPTH = 2;
  static const IR = 4;
}

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
  late String name;
  late String vendor;
  late int productId;
  late int vendorId;
  late String uri;

  OpenNi2Device(
    this.name,
    this.vendor,
    this.productId,
    this.vendorId,
    this.uri,
  );
  OpenNi2Device.fromJson(Map<dynamic, dynamic> json) {
    name = json['name'].toString();
    vendor = json['vendor'].toString();
    productId = json['productId'].toInt();
    vendorId = json['vendorId'].toInt();
    uri = json['uri'].toString();
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

class FlutterVision {
  static const MethodChannel channel = MethodChannel('flutter_vision');

  static listen(Future<dynamic> Function(MethodCall) callback) {
    channel.setMethodCallHandler(callback);
  }

  static Future<List<String>> rsEnumerateDevices() async {
    try {
      List<Object?> list = await channel.invokeMethod('rsEnumerateDevices');

      return list.map((e) => e.toString()).toList();
    } catch (e) {
      print('Realsense SDK is not found');
    }

    return [];
  }

  static Future<int> niInitialize() async {
    return await channel.invokeMethod('ni2Initialize') ?? OpenNi2Status.STATUS_ERROR;
  }

  static Future<List<OpenNi2Device>> enumerateDevices() async {
    List<OpenNi2Device> deviceList = <OpenNi2Device>[];

    try {
      List<Object?> list = await channel.invokeMethod('ni2EnumerateDevices');

      deviceList.addAll(list.map((e) => OpenNi2Device.fromJson(e as Map<dynamic, dynamic>)));

      print(deviceList[0].toJson());
    } catch (e) {
      print('OpenNI SDK is not found');
    }

    return deviceList;
  }

  static Future<int> openDevice(OpenNi2Device device) async {
    int ret = await channel.invokeMethod('ni2OpenDevice', {'uri': device.uri});

    return ret;
  }

  static Future<int> getOpenglTextureId() async {
    return await channel.invokeMethod('getOpenglTextureId');
  }

  static Future<void> openglSetCamPosition(double x, double y, double z) async {
    return await channel.invokeMethod('openglSetCamPosition', {'x': x, 'y': y, 'z': z});
  }

  static Future<void> openglSetCamAngle(double yaw, double pitch) async {
    return await channel.invokeMethod('openglSetCamAngle', {'yaw': yaw, 'pitch': pitch});
  }

  static Future<void> openglSetCamFov(double fov) async {
    return await channel.invokeMethod('openglSetCamFov', {'fov': fov});
  }

  static Future<void> openglRender() async {
    return await channel.invokeMethod('openglRender');
  }

  static Future<void> tfliteCreateModel(String modelPath) async {
    return await channel.invokeMethod('tfliteCreateModel', {'modelPath': modelPath});
  }

  static Future<void> cameraOpen(int index) async {
    return await channel.invokeMethod('cameraOpen', {'index': index});
  }
}

const _FUNC_TEST = 0;
const _FUNC_CVTCOLOR = 1;
const _FUNC_IMWRITE = 2;
const _FUNC_SHOW = 3;
const _FUNC_CONVERTO = 4;
const _FUNC_APPLY_COLOR_MAP = 5;
const _FUNC_RESIZE = 6;
const _FUNC_CROP = 7;
const _FUNC_IMREAD = 8;
const _FUNC_CV_RECTANGLE = 9;
const _FUNC_CV_ROTATE = 10;

const _FUNC_SET_INPUT_TENSOR = 11;
const _FUNC_INFERENCE = 12;

const _FUNC_CUSTOM_HANDLER = 13;

// TODO: check method can be added to that pipeline
class FvPipeline {
  static const RGB_FRAME = 1;
  static const DEPTH_FRAME = 2;
  static const IR_FRAME = 4;
  static const UVC_FRAME = 3;

  static const DATATYPE_UINT8 = 0;
  static const DATATYPE_FLOAT = 1;

  int index;
  String serial;

  Future<String> info() async {
    return await FlutterVision.channel.invokeMethod('pipelineInfo', {'index': index, 'serial': serial});
  }

  Future<String> error() async {
    return await FlutterVision.channel.invokeMethod('pipelineError', {'index': index, 'serial': serial});
  }

  FvPipeline(this.serial, this.index);

  Future<int> run({int? from, int? to}) async {
    return await FlutterVision.channel.invokeMethod('pipelineRun', {'index': index, 'serial': serial, 'from': from ?? 0, 'to': to ?? -1});
  }

  Future<void> clear() async {
    await FlutterVision.channel.invokeMethod('pipelineClear', {'index': index, 'serial': serial});
  }

  Future<void> test(int t, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_TEST,
      'params': Uint8List.fromList([t]),
      'len': 1,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> cvtColor(int mode, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_CVTCOLOR,
      'params': Uint8List.fromList([mode]),
      'len': 1,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> imwrite(String path, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_IMWRITE,
      'params': Uint8List.fromList([path.length, ...utf8.encode(path)]),
      'len': path.length + 1,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> imread(String path, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_IMREAD,
      'params': Uint8List.fromList([path.length, ...utf8.encode(path)]),
      'len': path.length + 1,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> show({int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_SHOW,
      'params': null,
      'len': 0,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> convertTo(int mode, double scale, {int? at, double? shift, int? interval, bool? append}) async {
    List<Object?> scaleList = await FlutterVision.channel.invokeMethod("_float2uint8", {'value': scale});
    Uint8List scaleBytes = Uint8List.fromList(scaleList.map((e) => e as int).toList());

    List<Object?> shiftList = await FlutterVision.channel.invokeMethod("_float2uint8", {'value': shift ?? 0});
    Uint8List shiftBytes = Uint8List.fromList(shiftList.map((e) => e as int).toList());

    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_CONVERTO,
      'params': Uint8List.fromList([mode, ...scaleBytes, ...shiftBytes]),
      'len': 9,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> applyColorMap(int colorMap, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_APPLY_COLOR_MAP,
      'params': Uint8List.fromList([colorMap]),
      'len': 1,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> resize(int width, int height, {int? at, int? mode, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_RESIZE,
      'params': Uint8List.fromList([(width >> 8) & 0xff, width & 0xff, (height >> 8) & 0xff, height & 0xff, mode ?? OpenCV.INTER_NEAREST]),
      'len': 5,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> crop(int xStart, int xEnd, int yStart, int yEnd, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_CROP,
      'params': Uint8List.fromList([xStart >> 8, xStart & 0xff, xEnd >> 8, xEnd & 0xff, yStart >> 8, yStart & 0xff, yEnd >> 8, yEnd & 0xff]),
      'len': 8,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> cvRectangle(double x1, double y1, double x2, double y2, int r, int g, int b, {int? at, int? thickness, int? lineType, int? shift, int? alpha, int? interval, bool? append}) async {
    List<Object?> x1f = await FlutterVision.channel.invokeMethod("_float2uint8", {'value': x1});
    Uint8List x1Bytes = Uint8List.fromList(x1f.map((e) => e as int).toList());
    List<Object?> y1f = await FlutterVision.channel.invokeMethod("_float2uint8", {'value': y1});
    Uint8List y1Bytes = Uint8List.fromList(y1f.map((e) => e as int).toList());
    List<Object?> x2f = await FlutterVision.channel.invokeMethod("_float2uint8", {'value': x2});
    Uint8List x2Bytes = Uint8List.fromList(x2f.map((e) => e as int).toList());
    List<Object?> y2f = await FlutterVision.channel.invokeMethod("_float2uint8", {'value': y2});
    Uint8List y2Bytes = Uint8List.fromList(y2f.map((e) => e as int).toList());

    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_CV_RECTANGLE,
      'params': Uint8List.fromList([...x1Bytes, ...y1Bytes, ...x2Bytes, ...y2Bytes, r, g, b, alpha ?? 255, thickness ?? 1, lineType ?? OpenCV.LINE_TYPE_LINE_8, shift ?? 0]),
      'len': 23,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> rotate(int rotateCode, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_CV_ROTATE,
      'params': Uint8List.fromList([rotateCode]),
      'len': 1,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> setInputTensorData(int modelIndex, int tensorIndex, int dataType, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_SET_INPUT_TENSOR,
      'params': Uint8List.fromList([modelIndex, tensorIndex, dataType]),
      'len': 3,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> inference(int modelIndex, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_INFERENCE,
      'params': Uint8List.fromList([modelIndex]),
      'len': 1,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
  }

  Future<void> customHandler(int size, {int? at, int? interval, bool? append}) async {
    await FlutterVision.channel.invokeMethod('pipelineAdd', {
      'index': index,
      'funcIndex': _FUNC_CUSTOM_HANDLER,
      'params': Uint8List.fromList([size >> 8, size & 0xff]),
      'len': 2,
      'at': at ?? -1,
      'interval': interval ?? 0,
      'serial': serial,
      'append': append ?? false,
    });
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

  Future<Float32List> getTensorOutput(int tensorIndex, List<int> size) async {
    List l = await FlutterVision.channel.invokeMethod('tfliteGetTensorOutput', {'tensorIndex': tensorIndex, 'size': Int32List.fromList(size)});

    if (Platform.isWindows) {
      Float32List flist = Float32List(l.length);
      for (int i = 0; i < l.length; i++) {
        flist[i] = l[i] as double;
      }
      return flist;
    } else {
      return l as Float32List;
    }
  }

  Future<dynamic> _getModelInfo(String key) async {
    Map<dynamic, dynamic> m = await FlutterVision.channel.invokeMethod('tfliteGetModelInfo', {'index': index});

    return m[key];
  }

  get valid async {
    return await _getModelInfo('valid') as bool;
  }

  get error async {
    return await _getModelInfo('error') as String;
  }
}
