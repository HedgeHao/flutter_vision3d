import 'package:flutter/gestures.dart';
import 'package:flutter_vision3d/flutter_vision3d.dart';
import 'package:get/get.dart';

enum MOUSEBUTTON {
  release,
  left,
  right,
  unknown,
  middle,
}

class PointCloudController extends GetxController {
  double camPosX = 0.0, camPosY = 0.0, camPosZ = -3.0;
  double fov = 45.0;
  final eyeZOffset = 0.5;
  double yaw = 90.0;
  double pitch = 0;
  double mouseLastX = 0, mouseLastY = 0;
  double mouseMoveSensitivityAngle = 0.5;
  double mouseMoveSensitivityPos = 0.05;
  bool mouseDown = false;

  void updateMouseClick(PointerEvent details) {
    mouseDown = details.down;

    if (mouseDown) {
      mouseLastX = details.position.dx;
      mouseLastY = details.position.dy;
    }

    update();
  }

  void updateMousePosition(PointerEvent details) {
    if (details.down) {
      if (details.buttons == MOUSEBUTTON.left.index) {
        yaw -= (details.position.dx - mouseLastX) * mouseMoveSensitivityAngle;
        pitch -= (details.position.dy - mouseLastY) * mouseMoveSensitivityAngle;
        yaw %= 360;
        pitch %= 360;
        FlutterVision3d.openglSetCamAngle(yaw, pitch);
      } else if (details.buttons == MOUSEBUTTON.right.index) {
        camPosX += (details.position.dx - mouseLastX) * mouseMoveSensitivityPos;
        camPosY -= (details.position.dy - mouseLastY) * mouseMoveSensitivityPos;
        FlutterVision3d.openglSetCamPosition(camPosX, camPosY, camPosZ);
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
      FlutterVision3d.openglSetCamFov(fov);
    }
  }
}
