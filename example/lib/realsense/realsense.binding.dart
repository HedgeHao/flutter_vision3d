import 'package:flutter_vision_example/realsense/realsense.controller.dart';
import 'package:get/get.dart';

class RealsenseBinding implements Bindings {
  @override
  void dependencies() {
    Get.put(RealsenseController());
  }
}
