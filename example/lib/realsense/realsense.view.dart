import 'package:flutter/material.dart';
import 'package:flutter_vision_example/realsense/realsense.controller.dart';
import 'package:get/get.dart';

class RealsenseView extends GetView<RealsenseController> {
  const RealsenseView({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    Size size = MediaQuery.of(context).size;

    return Scaffold(
      appBar: AppBar(
        title: const Text("Realsense"),
        leading: BackButton(
          color: Colors.white,
          onPressed: () => Get.back(),
        ),
      ),
      body: Row(children: [
        SizedBox(
            width: 300,
            height: size.height,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    const Text('Device List:'),
                    const SizedBox(width: 10),
                    GetBuilder<RealsenseController>(
                        id: RealsenseController.BUILDER_DEVICE_LIST,
                        builder: ((controller) {
                          controller.items.clear();
                          int index = 0;
                          for (var d in controller.deviceList) {
                            controller.items.add(DropdownMenuItem<int>(
                              child: Text(d),
                              value: index,
                            ));
                            index++;
                          }

                          return DropdownButton(
                              value: controller.selected,
                              selectedItemBuilder: (context) {
                                return controller.deviceList.map((e) => Center(child: Text(e))).toList();
                              },
                              items: controller.items,
                              onChanged: (value) {
                                controller.selectedRsDevice = controller.deviceList[value as int];
                                controller.selectDevice(value);
                              });
                        })),
                    TextButton(
                        onPressed: () async {
                          controller.enumerateDevice();
                        },
                        child: const Text('Refresh'))
                  ],
                ),
                Row(
                  children: [
                    const Text('Device:'),
                    const SizedBox(width: 10),
                    TextButton(
                        onPressed: () {
                          controller.openRealsenseCamera();
                        },
                        child: const Text('open')),
                    TextButton(
                        onPressed: () {
                          controller.closeRealsenseCamera();
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
                          controller.enableStreaming();
                        },
                        child: const Text('start')),
                    TextButton(
                        onPressed: () {
                          controller.disableStreaming();
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
              ],
            )),
        Expanded(
            child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Row(children: [
              Container(
                  decoration: BoxDecoration(border: Border.all(width: 1)),
                  width: 270,
                  height: 180,
                  child: GetBuilder<RealsenseController>(
                    id: RealsenseController.BUILDER_TEXTURE,
                    builder: (controller) {
                      return controller.cams.isEmpty
                          ? const SizedBox()
                          : Texture(
                              textureId: controller.cams.first.rgbTextureId,
                            );
                    },
                  )),
              const SizedBox(width: 10),
              Container(
                  decoration: BoxDecoration(border: Border.all(width: 1)),
                  width: 270,
                  height: 180,
                  child: GetBuilder<RealsenseController>(
                    id: RealsenseController.BUILDER_TEXTURE,
                    builder: (controller) {
                      return controller.cams.isEmpty
                          ? const SizedBox()
                          : Texture(
                              textureId: controller.cams.first.depthTextureId,
                            );
                    },
                  )),
              const SizedBox(width: 10),
              Container(
                  decoration: BoxDecoration(border: Border.all(width: 1)),
                  width: 270,
                  height: 180,
                  child: GetBuilder<RealsenseController>(
                    id: RealsenseController.BUILDER_TEXTURE,
                    builder: (controller) {
                      return controller.cams.isEmpty
                          ? const SizedBox()
                          : Texture(
                              textureId: controller.cams.first.irTextureId,
                            );
                    },
                  ))
            ]),
          ],
        ))
      ]),
    );
  }
}
