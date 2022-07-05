import 'package:flutter_vision_example/uvc/uvc.controller.dart';
import 'package:get/get.dart';

class UvcBinding implements Bindings {
  @override
  void dependencies() {
    Get.put(UvcControllerDevice());
  }
}
