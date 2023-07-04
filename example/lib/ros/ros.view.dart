import 'package:flutter/material.dart';
import 'package:flutter_vision_example/ros/ros.controller.dart';
import 'package:get/get.dart';

final TextEditingController ctl = TextEditingController()..text = '/image_raw';

class RosView extends GetView<RosControllerDevice> {
  const RosView({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    Size size = MediaQuery.of(context).size;
    return Scaffold(
      appBar: AppBar(
        title: const Text("ROS"),
        leading: BackButton(
          color: Colors.white,
          onPressed: () async {
            await controller.deconstruct();
            Get.back();
          },
        ),
      ),
      body: Row(children: [
        SizedBox(
            width: 300,
            height: size.height,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(children: [
                  const Text('Image Topic:'),
                  const SizedBox(width: 10),
                  SizedBox(
                      width: 100,
                      child: TextField(
                        controller: ctl,
                        textAlign: TextAlign.center,
                      )),
                ]),
                Row(
                  children: [
                    const Text('Device:'),
                    const SizedBox(width: 10),
                    TextButton(
                        onPressed: () {
                          controller.openRosCamera(ctl.text);
                        },
                        child: const Text('open')),
                    TextButton(
                        onPressed: () {
                          controller.closeRosCamera(ctl.text);
                        },
                        child: const Text('close')),
                  ],
                ),
                Row(
                  children: [
                    const Text('Streaming:'),
                    const SizedBox(width: 10),
                    TextButton(
                        onPressed: () {
                          controller.enableStreaming(ctl.text);
                        },
                        child: const Text('start')),
                    TextButton(
                        onPressed: () {
                          controller.disableStreaming(ctl.text);
                        },
                        child: const Text('stop')),
                  ],
                ),
                const Text('Pipelines:'),
                Column(children: [
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.pipelineDisplay();
                        },
                        child: const Text('Display')),
                    const Text(': Display original color frame')
                  ])
                ]),
                Row(
                  children: [
                    const Text("CV Mat:"),
                    const SizedBox(width: 6),
                    GetBuilder<RosControllerDevice>(builder: (controller) {
                      return Text(controller.cvMatPointer.toString());
                    }),
                    TextButton(
                        onPressed: () {
                          controller.getOpenCVMat();
                        },
                        child: const Text('Get')),
                  ],
                )
              ],
            )),
        Expanded(
            child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Container(
                decoration: BoxDecoration(border: Border.all(width: 1)),
                width: 640,
                height: 480,
                child: GetBuilder<RosControllerDevice>(
                  builder: (controller) {
                    return controller.cam == null
                        ? const SizedBox()
                        : Texture(
                            textureId: controller.cam!.rgbTextureId,
                          );
                  },
                ))
          ],
        ))
      ]),
    );
  }
}
