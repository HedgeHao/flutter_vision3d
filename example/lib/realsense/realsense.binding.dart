import 'package:flutter_vision3d_example/realsense/realsense.controller.dart';
import 'package:get/get.dart';

class RealsenseBinding implements Bindings {
  @override
  void dependencies() {
    Get.put(RealsenseController());
  }
}
