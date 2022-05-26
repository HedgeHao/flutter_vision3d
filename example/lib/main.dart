import 'dart:io';
import 'dart:typed_data';

import 'package:desktop_window/desktop_window.dart';
import 'package:file_picker/file_picker.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';

const frameWidth = 800.0;
const frameHeight = 600.0;

String mSerial = '';

enum MOUSEBUTTON {
  release,
  left,
  right,
  unknown,
  middle,
}

void main() {
  runApp(const MyApp());
  if (!Platform.isWindows) {
    DesktopWindow.setWindowSize(const Size(1600, 900));
  }
  WidgetsFlutterBinding.ensureInitialized();
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => MyAppState();
}

class MyAppState extends State<MyApp> {
  int textureId = 0;

  int cameraType = -1;

  FvCamera? cam;
  Widget? camConfigWidget;
  TFLiteModel? model;
  List<PositionedRect> rects = [];
  String displayText = '';

  @override
  void initState() {
    FlutterVision.listen(fvCallback);
    super.initState();
  }

  /* OpenGL Texture Gesture*/
  bool mouseDown = false;
  double mouseLastX = 0, mouseLastY = 0;
  double yaw = 90.0, pitch = 0;
  double mouseMoveSensitivityAngle = 0.5;
  double mouseMoveSensitivityPos = 0.05;
  double fov = 45.0;
  double camPosX = 0.0, camPosY = 0.0, camPosZ = -3.0;
  final eyeZOffset = 0.5;
  bool openglIsRendering = false;

  void updateMouseClick(PointerEvent details) {
    if (!openglIsRendering) return;

    setState(() {
      mouseDown = details.down;
    });

    if (mouseDown) {
      mouseLastX = details.position.dx;
      mouseLastY = details.position.dy;
    }
  }

  void updateMousePosition(PointerEvent details) {
    if (!openglIsRendering) return;

    if (details.down) {
      if (details.buttons == MOUSEBUTTON.left.index) {
        yaw -= (details.position.dx - mouseLastX) * mouseMoveSensitivityAngle;
        pitch -= (details.position.dy - mouseLastY) * mouseMoveSensitivityAngle;
        yaw %= 360;
        pitch %= 360;
        FlutterVision.openglSetCamAngle(yaw, pitch);
      } else if (details.buttons == MOUSEBUTTON.right.index) {
        camPosX += (details.position.dx - mouseLastX) * mouseMoveSensitivityPos;
        camPosY -= (details.position.dy - mouseLastY) * mouseMoveSensitivityPos;
        FlutterVision.openglSetCamPosition(camPosX, camPosY, camPosZ);
      }
    }

    mouseLastX = details.position.dx;
    mouseLastY = details.position.dy;
  }

  void updateMouseWheel(PointerSignalEvent details) {
    if (!openglIsRendering) return;

    if (details is PointerScrollEvent) {
      if (details.scrollDelta.dy < 0) {
        fov -= eyeZOffset;
      } else {
        fov += eyeZOffset;
      }
      FlutterVision.openglSetCamFov(fov);
    }
  }

  void openglRender() {
    FlutterVision.openglRender().then((value) => {
          if (openglIsRendering) {Future.delayed(const Duration(milliseconds: 1), openglRender)}
        });
  }
/* OpenGL Texture Gesture */

  void chooseCamera(int? index) {
    if (index == null) return;

    setState(() {
      cameraType = index;
      if (index == CameraType.UVC.index)
        camConfigWidget = UvcCameraConfig();
      else if (index == CameraType.OPENNI.index)
        camConfigWidget = OpenniCameraConfig();
      else if (index == CameraType.REALSENSE.index)
        camConfigWidget = RealsenseCameraConfig();
      else {}
    });
  }

  Future<dynamic> fvCallback(MethodCall call) async {
    if (call.method == 'onInference') {
      Float32List outputBoxes = await model!.getTensorOutput(0, [25, 4]);
      Float32List outputClass = await model!.getTensorOutput(1, [25]);
      Float32List outputScore = await model!.getTensorOutput(2, [25]);

      if (outputScore[0] < 0.55) {
        setState(() {
          rects.clear();
          displayText = '';
        });
        return;
      }

      List<PositionedRect> r = [];
      for (int i = 0; i < 1; i++) {
        double x = frameWidth * outputBoxes[i * 4 + 1];
        double y = frameHeight * outputBoxes[i * 4];
        double width = frameWidth * outputBoxes[i * 4 + 3] - x;
        double height = frameHeight * outputBoxes[i * 4 + 2] - y;
        r.add(PositionedRect(x, y, width, height, Colors.red));
      }

      setState(() {
        rects = r;
        displayText = '${COCO_CLASSES[outputClass[0].toInt()]} ${outputScore[0]}';
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
          body: Column(
        children: [
          Visibility(
              visible: cameraType == -1,
              child: Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Text('CameraType:'),
                  const SizedBox(width: 5),
                  const Text('UVC'),
                  Radio<int>(value: CameraType.UVC.index, groupValue: cameraType, onChanged: chooseCamera),
                  const Text('OpenNI2'),
                  Radio<int>(value: CameraType.OPENNI.index, groupValue: cameraType, onChanged: chooseCamera),
                  const Text('Realsense'),
                  Radio<int>(value: CameraType.REALSENSE.index, groupValue: cameraType, onChanged: chooseCamera),
                  const Text('Dummy'),
                  Radio<int>(value: CameraType.DUMMY.index, groupValue: cameraType, onChanged: chooseCamera),
                ],
              )),
          camConfigWidget ?? const SizedBox(),
          Visibility(
              visible: camConfigWidget != null,
              child: Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                TextButton(
                    onPressed: () async {
                      if (mSerial.isEmpty) return;
                      cam = await FvCamera.create(mSerial, CameraType.values[cameraType]);
                      if (cam == null) return;

                      print('Connect OK');
                    },
                    child: const Text('Connect')),
                TextButton(
                    onPressed: () {
                      if (cam == null) return;

                      cam!.enableStream();
                    },
                    child: const Text('Enable')),
                TextButton(
                    onPressed: () {
                      if (cam == null) return;

                      cam!.disableStream();
                    },
                    child: const Text('Disable')),
                TextButton(
                    onPressed: () {
                      if (cam == null) return;

                      // TODO: Crash
                      cam!.close();
                    },
                    child: const Text('Disconnect')),
                const Text('PointCloud'),
                TextButton(
                    onPressed: () {
                      if (cam == null) return;

                      cam!.enablePointCloud();
                    },
                    child: const Text('Enable')),
                TextButton(
                    onPressed: () {
                      if (cam == null) return;

                      cam!.disablePointCloud();
                    },
                    child: const Text('Disable')),
              ])),
          Listener(
              // Listener is only for PointCloud
              // TODO: Opengl render is upside down
              onPointerDown: updateMouseClick,
              onPointerCancel: updateMouseClick,
              onPointerUp: updateMouseClick,
              onPointerMove: updateMousePosition,
              onPointerSignal: updateMouseWheel,
              child: SizedBox(
                  width: frameWidth,
                  height: frameHeight,
                  child: Stack(children: [
                    Texture(textureId: textureId),
                    ...rects,
                    Text(displayText, style: const TextStyle(color: Colors.red, fontSize: 24)),
                  ]))),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const Text('Pipeline:'),
              TextButton(
                  onPressed: () async {
                    if (cam == null) return;

                    FvPipeline pipeline = cam!.rgbPipeline;
                    await pipeline.clear();
                    await pipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
                    await pipeline.show();

                    textureId = cam!.rgbTextureId;
                    setState(() {});
                  },
                  child: const Text('RGB Display')),
              TextButton(
                  onPressed: () async {
                    FvPipeline depthPipeline = cam!.depthPipeline;
                    await depthPipeline.clear();
                    await depthPipeline.convertTo(0, 255.0 / 1024.0);
                    await depthPipeline.applyColorMap(OpenCV.COLORMAP_JET);
                    await depthPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
                    await depthPipeline.show();
                    textureId = cam!.depthTextureId;
                    setState(() {});
                  },
                  child: const Text('Depth ColorMap')),
              TextButton(
                  onPressed: () async {
                    if (cam == null) return;

                    FilePickerResult? result = await FilePicker.platform.pickFiles();
                    if (result == null) return;

                    model ??= await TFLiteModel.create(result.files.first.path);

                    FvPipeline rgbPipeline = cam!.rgbPipeline;
                    await rgbPipeline.clear();
                    await rgbPipeline.crop(100, 600, 100, 600);
                    await rgbPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
                    await rgbPipeline.show();
                    await rgbPipeline.resize(320, 320, mode: OpenCV.INTER_CUBIC);
                    await rgbPipeline.cvtColor(OpenCV.COLOR_RGBA2BGR);
                    await rgbPipeline.setInputTensorData(model!.index, 0, FvPipeline.DATATYPE_UINT8);
                    await rgbPipeline.inference(model!.index);

                    textureId = cam!.rgbTextureId;
                    setState(() {});
                  },
                  child: const Text('ObjectDetector AI Model')),
              TextButton(
                  onPressed: () async {
                    if (!openglIsRendering) {
                      openglIsRendering = true;
                      openglRender();
                    } else {
                      openglIsRendering = false;
                    }

                    textureId = await FlutterVision.getOpenglTextureId();

                    setState(() {});
                  },
                  child: const Text('PointCloud'))
            ],
          )
        ],
      )),
    );
  }
}

class UvcCameraConfig extends StatelessWidget {
  TextEditingController ctl = TextEditingController();

  UvcCameraConfig() {
    ctl.text = '0';
    ctl.addListener(updateInput);
  }

  void updateInput() {
    mSerial = ctl.text;
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        const Text('Camera Index (number):'),
        SizedBox(
          width: 50,
          child: TextField(controller: ctl, textAlign: TextAlign.center),
        ),
      ],
    );
  }
}

class OpenniCameraConfig extends StatelessWidget {
  TextEditingController ctl = TextEditingController();

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        TextButton(
            onPressed: () async {
              await FlutterVision.niInitialize();
            },
            child: const Text('Init')),
        TextButton(
            onPressed: () async {
              List<OpenNi2Device> list = await FlutterVision.enumerateDevices();
              if (list.isEmpty) return;

              mSerial = list[0].uri;
              ctl.text = mSerial;
            },
            child: const Text('Find Device')),
        SizedBox(width: 400, child: TextField(controller: ctl))
      ],
    );
  }
}

class RealsenseCameraConfig extends StatelessWidget {
  TextEditingController ctl = TextEditingController();

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        TextButton(
            onPressed: () async {
              List<String> list = await FlutterVision.rsEnumerateDevices();
              if (list.isEmpty) return;

              mSerial = list[0];
              ctl.text = mSerial;
            },
            child: const Text('Find Device')),
        SizedBox(width: 200, child: TextField(controller: ctl)),
      ],
    );
  }
}

class PositionedRect extends StatelessWidget {
  final double top, left, width, height;
  final Color color;

  const PositionedRect(this.left, this.top, this.width, this.height, this.color, {Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Positioned(
        top: top,
        left: left,
        child: Container(
          width: width,
          height: height,
          decoration: BoxDecoration(border: Border.all(color: color)),
        ));
  }
}
