import 'package:flutter_vision3d_example/openni2/openni2.controller.dart';
import 'package:get/get.dart';

class OpenNIBinding implements Bindings {
  @override
  void dependencies() {
    Get.put(OpenNIController());
  }
}
