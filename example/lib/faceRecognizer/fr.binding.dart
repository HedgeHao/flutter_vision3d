import 'package:flutter_vision_example/faceRecognizer/fr.controller.dart';
import 'package:get/get.dart';

class FaceRecognizerBinding extends Bindings {
  @override
  void dependencies() {
    Get.put(FaceRecognizerController());
  }
}
