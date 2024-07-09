import 'package:flutter_vision3d_example/efficient_net/efficient_net.controller.dart';
import 'package:get/get.dart';

class EfficientNetBinding extends Bindings {
  @override
  void dependencies() {
    Get.put(EfficientNetController());
  }
}
