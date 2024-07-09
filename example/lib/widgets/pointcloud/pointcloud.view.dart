import 'package:flutter/material.dart';
import 'package:flutter_vision3d_example/widgets/pointcloud/pointcloud.controller.dart';
import 'package:get/get.dart';

class PointCloud extends StatelessWidget {
  late final PointCloudController controller;
  final double width, height;
  final int textureId;
  int? quarterTurns;

  PointCloud({Key? key, required this.width, required this.height, required this.textureId, this.quarterTurns}) : super(key: key) {
    controller = Get.put(PointCloudController());
  }

  @override
  Widget build(BuildContext context) {
    return RotatedBox(
        quarterTurns: quarterTurns ?? 0,
        child: Listener(
          onPointerDown: controller.updateMouseClick,
          onPointerCancel: controller.updateMouseClick,
          onPointerUp: controller.updateMouseClick,
          onPointerMove: controller.updateMousePosition,
          onPointerSignal: controller.updateMouseWheel,
          child: Container(
            decoration: BoxDecoration(border: Border.all(width: 1)),
            width: width,
            height: height,
            child: Texture(textureId: textureId),
          ),
        ));
  }
}
