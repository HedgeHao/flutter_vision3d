import 'package:flutter_vision_example/hand_detection/hand_detection.controller.dart';
import 'package:get/get.dart';

class HandDetectionBinding extends Bindings {
  @override
  void dependencies() {
    Get.put(HandDetectionController());
  }
}
