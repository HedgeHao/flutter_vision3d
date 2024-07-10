import 'package:flutter_vision3d_example/hand_detection/hand_detection.controller.dart';
import 'package:get/get.dart';

class HandDetectionBinding extends Bindings {
  @override
  void dependencies() {
    Get.put(HandDetectionController());
  }
}
