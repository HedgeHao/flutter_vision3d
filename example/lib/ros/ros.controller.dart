import 'package:flutter_vision3d/camera/camera.dart';
import 'package:flutter_vision3d/camera/ros_camera.dart';
import 'package:flutter_vision3d/flutter_vision3d.dart';
import 'package:get/get.dart';

class RosControllerDevice extends GetxController {
  RosCamera? cam;
  int rgbTextureId = 0;
  int cvMatPointer = 0;

  Future<void> openRosCamera(String serial) async {
    RxStatus.empty();

    if (cam == null) {
      cam = await FvCamera.create(serial, CameraType.ROS) as RosCamera?;
      if (cam == null) {
        print('Create Camera Failed');
        update();
        return;
      }
    }

    update();
  }

  void closeRosCamera(String serial) async {
    await cam?.close();
    cam = null;
  }

  void enableStreaming(String serial) async {
    await cam?.enableStream();
  }

  void disableStreaming(String serial) async {
    await cam?.disableStream();
  }

  void pipelineDisplay() async {
    if (cam == null) return;

    FvPipeline rosPipeline = cam!.rgbPipeline;
    await rosPipeline.clear();
    await rosPipeline.show();
  }

  Future<void> deconstruct() async {
    await cam?.disableStream();
    await cam?.close();
    cam = null;
    update();
  }

  Future<void> getOpenCVMat() async {
    cvMatPointer = await cam?.getOpenCVMat(1) ?? 0;
    update();
  }
}
