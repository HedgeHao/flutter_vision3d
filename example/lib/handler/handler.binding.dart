import 'package:flutter_vision3d_example/handler/handler.controller.dart';
import 'package:get/get.dart';

class HandlerBinding implements Bindings {
  @override
  void dependencies() {
    Get.put(HandlerController());
  }
}
