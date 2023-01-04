import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/uvc.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:get/get.dart';

class UvcControllerDevice extends GetxController {
  UvcCamera? cam;
  int rgbTextureId = 0;
  int cvMatPointer = 0;

  Future<void> openUvcCamera(String serial) async {
    RxStatus.empty();

    if (cam == null) {
      cam = await FvCamera.create(serial, CameraType.UVC) as UvcCamera?;
      if (cam == null) {
        print('Create Camera Failed');
        update();
        return;
      }

      await cam?.configure(OpenCV.CAP_PROP_MODE, [1]);
      await cam?.configure(OpenCV.CAP_PROP_FPS, [30.0]);
      await cam?.configure(OpenCV.CAP_PROP_FRAME_WIDTH, [640]);
      await cam?.configure(OpenCV.CAP_PROP_FRAME_HEIGHT, [480]);
    }

    update();
  }

  void closeUvcCamera(String serial) async {
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

    FvPipeline uvcPipeline = cam!.rgbPipeline;
    await uvcPipeline.clear();
    await uvcPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await uvcPipeline.show();
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

  void test() {
    cam?.test(cvMatPointer);
  }
}
