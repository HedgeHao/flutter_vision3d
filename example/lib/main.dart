import 'dart:io';
import 'dart:math';

import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision_example/configurePanel.dart';
import 'package:desktop_window/desktop_window.dart';

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

  String debugText = '';

  bool mouseDown = false;

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
          if (FlutterVision.openglIsRendering) {Future.delayed(const Duration(milliseconds: 1), openglRender)}
        });
  }

  @override
  void initState() {
    super.initState();
    FlutterVision.listen(update);
    registerTexture();
  }

  registerTexture() async {
    await FlutterVision.setVideoSize(1, 640, 480);
    // await FlutterVision.setVideoSize(2, 80, 60);
    // await FlutterVision.setVideoSize(4, 80, 60);

    await FlutterVision.setVideoSize(2, 640, 480);
    await FlutterVision.setVideoSize(4, 640, 480);

    rgbTextureId = await FlutterVision.getVideoTextureId(1);
    depthTextureId = await FlutterVision.getVideoTextureId(2);
    irTextureId = await FlutterVision.getVideoTextureId(4);
    openglTextureId = await FlutterVision.getVideoTextureId(8);

    setState(() {});
  }

  Future<dynamic> update(MethodCall call) async {}

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
                  rgbTextureId == 0 ? const SizedBox() : Container(decoration: BoxDecoration(border: Border.all(width: 1)), width: 240, height: 180, child: Texture(textureId: rgbTextureId)),
                  const SizedBox(width: 10),
                  depthTextureId == 0 ? const SizedBox() : Container(decoration: BoxDecoration(border: Border.all(width: 1)), width: 240, height: 180, child: Texture(textureId: depthTextureId)),
                  const SizedBox(width: 10),
                  irTextureId == 0 ? const SizedBox() : Container(decoration: BoxDecoration(border: Border.all(width: 1)), width: 240, height: 180, child: Texture(textureId: irTextureId)),
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
                      child: Container(decoration: BoxDecoration(border: Border.all(width: 1)), width: 720, height: 540, child: Transform.rotate(angle: 180 * pi / 180, child: Texture(textureId: openglTextureId)))),
              Text(debugText, style: const TextStyle(fontSize: 30)),
              Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                TextButton(
                    onPressed: () async {
                      // rgbTextureId = 0;
                      // depthTextureId = 0;
                      // irTextureId = 0;
                      // openglTextureId = 0;
                      // setState(() {});

                      // registerTexture();

                      await FlutterVision.test();

                      print('');
                    },
                    child: const Text('Test')),
                TextButton(
                    onPressed: () {
                      FlutterVision.openglSetCamAngle(90, 0);
                      FlutterVision.openglSetCamPosition(0, 0, -3);
                      FlutterVision.openglSetCamFov(45.0);
                    },
                    child: const Text('Reset')),
                TextButton(
                    onPressed: () {
                      if (!FlutterVision.openglIsRendering) {
                        FlutterVision.openglIsRendering = true;
                        openglRender();
                      } else {
                        FlutterVision.openglIsRendering = false;
                      }
                    },
                    child: const Text('Render'))
              ])
            ])),
          ],
        )),
      ),
    );
  }
}
