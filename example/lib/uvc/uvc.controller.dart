import 'package:flutter_vision/camera/camera.dart';
import 'package:flutter_vision/camera/uvc.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:get/get.dart';

class UvcControllerDevice extends GetxController {
  final BUILDER_CONNECT_STATUS = 'BUILDER_CONNECT_STATUS';
  final BUILDER_TEXTURE = 'BUILDER_TEXTURE';

  List<UvcCamera> cams = [];
  int rgbTextureId = 0;

  Future<void> openUvcCamera(String serial) async {
    RxStatus.empty();
    UvcCamera? cam = cams.firstWhereOrNull((e) => e.serial == serial.toString());

    if (cam == null) {
      cam = await FvCamera.create(serial, CameraType.UVC) as UvcCamera?;
      if (cam == null) {
        print('Create Camera Failed');
        update();
        return;
      }
      cams.add(cam);
    }

    await cam.configure(OpenCV.CAP_PROP_MODE, [1]);
    await cam.configure(OpenCV.CAP_PROP_FPS, [30.0]);
    await cam.configure(OpenCV.CAP_PROP_FRAME_WIDTH, [640]);
    await cam.configure(OpenCV.CAP_PROP_FRAME_HEIGHT, [480]);

    update();
  }

  void closeUvcCamera(String serial) async {
    UvcCamera? cam = cams.firstWhereOrNull((e) => e.serial == serial.toString());

    if (cam == null) {
      return;
    }

    await cam.close();
  }

  void enableStreaming(String serial) async {
    UvcCamera? cam = cams.firstWhereOrNull((e) => e.serial == serial.toString());

    if (cam == null) {
      return;
    }

    await cam.enableStream();
  }

  void disableStreaming(String serial) async {
    UvcCamera? cam = cams.firstWhereOrNull((e) => e.serial == serial.toString());

    if (cam == null) {
      return;
    }

    await cam.disableStream();
  }

  void pipelineDisplay() async {
    FvPipeline uvcPipeline = cams.first.rgbPipeline;
    await uvcPipeline.clear();
    await uvcPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
    await uvcPipeline.show();
  }
}
