import 'dart:io';
import 'dart:math';
import 'dart:typed_data';

import 'package:desktop_window/desktop_window.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/dummy.dart';
import 'package:flutter_vision/camera/openni.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision_example/configurePanel.dart';
import 'package:flutter_vision_example/demo/LIPSFace.dart';
import 'package:flutter_vision_example/viewModel.dart';

const texture_width = 240.0;
const texture_height = 180.0;

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
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  double camPosX = 0.0, camPosY = 0.0, camPosZ = -3.0;
  double fov = 45.0;
  final eyeZOffset = 0.5;
  double yaw = 90.0;
  double pitch = 0;
  double mouseLastX = 0, mouseLastY = 0;
  double mouseMoveSensitivityAngle = 0.5;
  double mouseMoveSensitivityPos = 0.05;

  int rgbTextureId = 0;
  int depthTextureId = 0;
  int irTextureId = 0;
  int openglTextureId = 0;
  int cameraTextureId = 0;

  String debugText = '';

  bool mouseDown = false;

  List<TFLiteModel> models = [];

  List<PositionedRect> rects = [];

  bool openglIsRendering = false;

  int ts = 0;

  void updateMouseClick(PointerEvent details) {
    setState(() {
      mouseDown = details.down;
    });

    if (mouseDown) {
      mouseLastX = details.position.dx;
      mouseLastY = details.position.dy;
    }
  }

  void updateMousePosition(PointerEvent details) {
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

  @override
  void initState() {
    super.initState();
    FlutterVision.listen(update);
    registerTexture();
  }

  registerTexture() async {
    openglTextureId = await FlutterVision.getOpenglTextureId();
    configuration.dummyCams.add((await FvCamera.create(DummyCamera.generateSerial(), CameraType.DUMMY) as DummyCamera?)!);
    rgbTextureId = configuration.dummyCams[0].rgbTextureId;
    depthTextureId = configuration.dummyCams[0].depthTextureId;
    irTextureId = configuration.dummyCams[0].irTextureId;

    setState(() {});

    print('Texture Ready');
  }

  Future<dynamic> update(MethodCall call) async {
    if (call.method == 'onInference') {
      Float32List output = await models[0].getTensorOutput(0, [28, 28, 5]);
      List<FaceInfo> faces = processFaceDetectorOutputs(output, 240, 180);
      if (faces.isEmpty) return;

      faces = nms(faces, 0.3);
      // if (faces.length > 2) return;
      List<PositionedRect> r = [];
      if (faces.isNotEmpty) {
        for (FaceInfo f in faces) {
          r.add(PositionedRect(f.x1, f.y1, f.x2 - f.x1, f.y2 - f.y1, Colors.red));
        }
      }

      setState(() {
        rects = r;
      });
    } else if (call.method == 'onUvcFrame') {
    } else if (call.method == 'onNiFrame') {
    } else if (call.method == 'onHandled') {
      var result = call.arguments as Float32List;
      print(result);
    }

    return;
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Flutter Vision Example'),
        ),
        body: Center(
            child: Row(
          children: [
            const ConfigurePannel(),
            Expanded(
                child: Column(mainAxisAlignment: MainAxisAlignment.center, children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.end,
                children: [
                  rgbTextureId == 0
                      ? const SizedBox()
                      : Container(
                          decoration: BoxDecoration(border: Border.all(width: 1)),
                          width: texture_width,
                          height: texture_height,
                          child: Stack(children: [
                            Texture(textureId: rgbTextureId),
                            ...rects,
                          ]),
                        ),
                  const SizedBox(width: 10),
                  depthTextureId == 0 ? const SizedBox() : Container(decoration: BoxDecoration(border: Border.all(width: 1)), width: 240, height: 180, child: Texture(textureId: depthTextureId)),
                  const SizedBox(width: 10),
                  irTextureId == 0 ? const SizedBox() : Container(decoration: BoxDecoration(border: Border.all(width: 1)), width: 240, height: 180, child: Texture(textureId: irTextureId)),
                  const SizedBox(width: 10),
                  cameraTextureId == 0
                      ? const SizedBox()
                      : Container(
                          decoration: BoxDecoration(border: Border.all(width: 1)),
                          width: 240,
                          height: 180,
                          child: Stack(children: [
                            Texture(textureId: cameraTextureId),
                            // ...rects,
                          ])),
                  const SizedBox(width: 10),
                ],
              ),
              openglTextureId == 0
                  ? const SizedBox()
                  : Listener(
                      onPointerDown: updateMouseClick,
                      onPointerCancel: updateMouseClick,
                      onPointerUp: updateMouseClick,
                      onPointerMove: updateMousePosition,
                      onPointerSignal: updateMouseWheel,
                      child: Container(decoration: BoxDecoration(border: Border.all(width: 1)), width: 540, height: 405, child: Transform.rotate(angle: 180 * pi / 180, child: Texture(textureId: openglTextureId)))),
              Text(debugText, style: const TextStyle(fontSize: 30)),
              Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                TextButton(
                    onPressed: () async {
                      if (configuration.niCams.isEmpty) return;

                      OpenniCamera cam = configuration.niCams[0];
                      rgbTextureId = cam.rgbTextureId;
                      depthTextureId = cam.depthTextureId;
                      irTextureId = cam.irTextureId;
                      setState(() {});

                      FvPipeline rgbPipeline = cam.rgbPipeline;
                      await rgbPipeline.clear();
                      // await rgbPipeline.crop(80, 560, 0, 480);
                      await rgbPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
                      // await rgbPipeline.rotate(OpenCV.ROTATE_90_CLOCKWISE);
                      await rgbPipeline.show();
                      // await rgbPipeline.resize(28, 28);
                      // await rgbPipeline.cvtColor(7);
                      // await rgbPipeline.convertTo(3, 255.0 / 1024.0, shift: 0);

                      FvPipeline depthPipeline = cam.depthPipeline;
                      await depthPipeline.clear();
                      await depthPipeline.convertTo(0, 255.0 / 1024.0);
                      // await depthPipeline.cvtColor(9); // COLOR_GRAY2BGRA
                      await depthPipeline.applyColorMap(OpenCV.COLORMAP_JET);
                      await depthPipeline.cvtColor(0); //COLOR_RGB2RGBA
                      await depthPipeline.show();

                      FvPipeline irPipeline = cam.irPipeline;
                      await irPipeline.clear();
                      await irPipeline.convertTo(0, 255.0 / 1024.0);
                      await irPipeline.cvtColor(9); // COLOR_GRAY2BGRA
                      await irPipeline.show();

                      await FlutterVision.test();

                      // await FlutterVision.videoScreenshot(16, 'test.jpg');

                      print('');
                    },
                    child: const Text('NI')),
                TextButton(
                    onPressed: () async {
                      if (configuration.uvcCams.isEmpty) return;

                      cameraTextureId = configuration.uvcCams.first.rgbTextureId;

                      FvPipeline uvcPipeline = configuration.uvcCams.first.rgbPipeline;
                      await uvcPipeline.clear();
                      await uvcPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
                      await uvcPipeline.show();

                      setState(() {});
                    },
                    child: const Text('UVC')),
                TextButton(
                    onPressed: () async {
                      FvPipeline? depthPipeline;
                      if (configuration.niCams.isNotEmpty) {
                        depthPipeline = configuration.niCams.first.depthPipeline;
                      } else if (configuration.rsCams.isNotEmpty) {
                        depthPipeline = configuration.rsCams.first.depthPipeline;
                      } else {
                        return;
                      }

                      await depthPipeline.applyColorMap(Random().nextInt(10), at: 1);
                    },
                    child: const Text('Replace pipeline')),
                // TextButton(
                //   onPressed: () async {
                //     TFLiteModel model = await TFLiteModel.create(MODEL_EFFECIENT_NET);
                //     models.add(model);

                //     // FvPipeline rgbPipeline = FvPipeline(1);
                //     // await rgbPipeline.clear();
                //     // await rgbPipeline.resize(320, 320, mode: OpenCV.INTER_CUBIC);
                //     // await rgbPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
                //     // // await rgbPipeline.crop(160, 480, 80, 400);
                //     // await rgbPipeline.show();
                //     // await rgbPipeline.cvtColor(OpenCV.CV_8UC1);
                //     // await rgbPipeline.cvtColor(OpenCV.COLOR_RGB2BGR);
                //     // await rgbPipeline.setInputTensorData(model.index, 0, FvPipeline.DATATYPE_UINT8);
                //     // await rgbPipeline.inference(model.index);

                //     await FlutterVision.test();

                //     Float32List outputBoxes = await model.getTensorOutput(0, [25, 4]);
                //     Float32List outputClass = await model.getTensorOutput(1, [25]);
                //     Float32List outputScore = await model.getTensorOutput(2, [25]);

                //     print('Class(raw):$outputClass');
                //     List<String> classes = outputClass.map((e) => e == 0 ? '' : COCO_CLASSES[e.toInt() - 1]).toList();

                //     print('Class:$classes');
                //     print('Score:${outputScore.map((e) => e.toStringAsFixed(2)).toList()}');
                //     print('Boxes:${outputBoxes.map((e) => e.toStringAsFixed(2)).toList()}');

                //     List<PositionedRect> r = [];
                //     for (int i = 0; i < 5; i++) {
                //       double x = texture_width * outputBoxes[i * 4 + 1];
                //       double y = texture_height * outputBoxes[i * 4];
                //       double width = texture_width * outputBoxes[i * 4 + 3] - x;
                //       double height = texture_height * outputBoxes[i * 4 + 2] - y;
                //       r.add(PositionedRect(x, y, width, height, Colors.red));
                //     }

                //     setState(() {
                //       rects = r;
                //     });
                //   },
                //   child: const Text('EfficientNet'),
                // ),
                TextButton(
                    onPressed: () async {
                      if (configuration.uvcCams.isEmpty) return;

                      rgbTextureId = configuration.uvcCams.first.rgbTextureId;

                      if (models.isEmpty) {
                        TFLiteModel model = await TFLiteModel.create(configuration.MODEL_FACE_DETECTOR);
                        models.add(model);
                      }

                      FvPipeline rgbPipeline = configuration.uvcCams.first.rgbPipeline;
                      await rgbPipeline.clear();
                      await rgbPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
                      await rgbPipeline.show();
                      await rgbPipeline.resize(224, 224, mode: OpenCV.INTER_LINEAR);
                      await rgbPipeline.cvtColor(OpenCV.COLOR_RGBA2RGB);
                      await rgbPipeline.convertTo(OpenCV.CV_32FC3, 1.0 / 255.0);
                      await rgbPipeline.setInputTensorData(models[0].index, 0, FvPipeline.DATATYPE_FLOAT);
                      await rgbPipeline.inference(models[0].index, interval: 100);
                    },
                    child: const Text('SW200')),
                TextButton(
                    onPressed: () {
                      FlutterVision.openglSetCamAngle(90, 0);
                      FlutterVision.openglSetCamPosition(0, 0, -3);
                      FlutterVision.openglSetCamFov(45.0);
                      print('');
                    },
                    child: const Text('Reset')),
                TextButton(
                    onPressed: () {
                      if (!openglIsRendering) {
                        openglIsRendering = true;
                        openglRender();
                      } else {
                        openglIsRendering = false;
                      }
                    },
                    child: const Text('Render')),
                TextButton(
                    onPressed: () async {
                      TFLiteModel model = await TFLiteModel.create(configuration.MODEL_FACE_DETECTOR);
                      models.add(model);

                      FvPipeline pipeline = configuration.dummyCams[0].rgbPipeline;
                      await pipeline.clear();
                      await pipeline.imread(configuration.TEST_IMAGE);
                      await pipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
                      await pipeline.show();
                      await pipeline.resize(224, 224, mode: OpenCV.INTER_LINEAR);
                      await pipeline.cvtColor(OpenCV.COLOR_RGBA2RGB);
                      await pipeline.convertTo(OpenCV.CV_32FC3, 1.0 / 255.0);
                      await pipeline.setInputTensorData(model.index, 0, FvPipeline.DATATYPE_FLOAT);
                      await pipeline.inference(model.index, interval: 100);
                      await pipeline.run();
                      print('');
                    },
                    child: const Text('Pipeline')),
                TextButton(
                    onPressed: () async {
                      if (configuration.dummyCams.isEmpty) return;

                      FvPipeline pipeline = configuration.dummyCams.first.rgbPipeline;
                      await pipeline.clear();
                      await pipeline.imread(configuration.TEST_IMAGE);
                      await pipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
                      await pipeline.customHandler(5);
                      await pipeline.show();
                      await pipeline.run();
                    },
                    child: const Text('Handler')),
                TextButton(
                    onPressed: () async {
                      if (configuration.rsCams.isEmpty) return;
                      rgbTextureId = configuration.rsCams[0].rgbTextureId;
                      depthTextureId = configuration.rsCams[0].depthTextureId;
                      irTextureId = configuration.rsCams[0].irTextureId;

                      print('$rgbTextureId, $depthTextureId, $irTextureId');

                      FvPipeline rgbPipeline = configuration.rsCams[0].rgbPipeline;
                      await rgbPipeline.clear();
                      await rgbPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
                      await rgbPipeline.show();

                      setState(() {});
                    },
                    child: const Text('RS')),
              ])
            ])),
          ],
        )),
      ),
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
