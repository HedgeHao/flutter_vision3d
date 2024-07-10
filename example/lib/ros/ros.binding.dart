import 'package:flutter_vision3d_example/ros/ros.controller.dart';
import 'package:get/get.dart';

class RosBinding implements Bindings {
  @override
  void dependencies() {
    Get.put(RosControllerDevice());
  }
}
