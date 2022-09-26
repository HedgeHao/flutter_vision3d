import 'package:flutter_vision_example/efficient_net/efficient_net.controller.dart';
import 'package:get/get.dart';

class EfficientNetBinding extends Bindings {
  @override
  void dependencies() {
    Get.put(EfficientNetController());
  }
}
