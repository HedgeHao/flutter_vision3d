import 'package:flutter_vision3d_example/opencv/opencv.controller.dart';
import 'package:get/get.dart';

class OpencvBinding implements Bindings {
  @override
  void dependencies() {
    Get.put(OpencvController());
  }
}
