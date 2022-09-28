import 'package:flutter/material.dart';
import 'package:flutter_vision_example/realsense/realsense.controller.dart';
import 'package:flutter_vision_example/widgets/pointcloud/pointcloud.view.dart';
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
            width: 400,
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
                          controller.pipelineRGB();
                        },
                        child: const Text('RGB Frame')),
                    const Text(': Original color frame')
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.pipelineDepth();
                        },
                        child: const Text('Depth Frame')),
                    const Text(': Depth frame with random color map')
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.pipelineIr();
                        },
                        child: const Text('Ir Frame')),
                    const Text(': IR frame with greys color map')
                  ])
                ]),
                const Divider(),
                const Align(alignment: Alignment.center, child: Text('Configuration')),
                const Text('Range Filter(m)'),
                Row(
                  children: [
                    const Text('Min:'),
                    GetBuilder<RealsenseController>(
                        id: RealsenseController.BUILDER_SLIDER,
                        builder: (controller) => Slider(
                              min: 0.0,
                              max: 4.0,
                              divisions: 40,
                              value: controller.rangeFilterValueMin,
                              onChanged: controller.cam == null ? null : (double value) => controller.minRange = value,
                            )),
                    GetBuilder<RealsenseController>(id: RealsenseController.BUILDER_SLIDER, builder: (controller) => Text(controller.rangeFilterValueMin.toStringAsFixed(1))),
                  ],
                ),
                Row(
                  children: [
                    const Text('Max:'),
                    GetBuilder<RealsenseController>(
                        id: RealsenseController.BUILDER_SLIDER,
                        builder: (controller) => Slider(
                              min: 0.0,
                              max: 4.0,
                              divisions: 40,
                              value: controller.rangeFilterValueMax,
                              onChanged: controller.cam == null ? null : (double value) => controller.maxRange = value,
                            )),
                    GetBuilder<RealsenseController>(id: RealsenseController.BUILDER_SLIDER, builder: (controller) => Text(controller.rangeFilterValueMax.toStringAsFixed(1))),
                  ],
                ),
                Row(
                  children: [
                    const Text('PointCloud:'),
                    GetBuilder<RealsenseController>(
                        id: RealsenseController.BUILDER_TEXTURE_OPENGL,
                        builder: (controller) => Switch(
                              value: controller.pointCloud,
                              onChanged: (value) => controller.enablePointCloud(value),
                            ))
                  ],
                )
              ],
            )),
        Expanded(
            child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Row(mainAxisAlignment: MainAxisAlignment.center, children: [
              Container(
                  decoration: BoxDecoration(border: Border.all(width: 1)),
                  width: 270,
                  height: 180,
                  child: GetBuilder<RealsenseController>(
                    id: RealsenseController.BUILDER_TEXTURE,
                    builder: (controller) {
                      return controller.cam == null
                          ? const SizedBox()
                          : Texture(
                              textureId: controller.cam!.rgbTextureId,
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
                      return controller.cam == null
                          ? const SizedBox()
                          : Texture(
                              textureId: controller.cam!.depthTextureId,
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
                      return controller.cam == null
                          ? const SizedBox()
                          : Texture(
                              textureId: controller.cam!.irTextureId,
                            );
                    },
                  ))
            ]),
            const SizedBox(height: 8),
            GetBuilder<RealsenseController>(
              id: RealsenseController.BUILDER_TEXTURE_OPENGL,
              builder: (controller) => controller.pointCloud
                  ? PointCloud(
                      width: 540,
                      height: 360,
                      textureId: controller.openglTextureId,
                      quarterTurns: 2,
                    )
                  : const SizedBox(),
            )
          ],
        ))
      ]),
    );
  }
}
