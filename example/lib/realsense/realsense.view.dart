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
          onPressed: () async {
            await controller.deconstruct();
            Get.back();
          },
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
                Row(children: [const Text('Serial Number:'), GetBuilder<RealsenseController>(builder: (controller) => Text(controller.sn))]),
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
                        onPressed: () async {
                          bool ret = await controller.enableStreaming();
                          print('Enable:$ret');
                        },
                        child: const Text('start')),
                    TextButton(
                        onPressed: () {
                          controller.disableStreaming();
                        },
                        child: const Text('stop')),
                  ],
                ),
                Row(
                  children: [
                    const Text('Video Render:'),
                    const SizedBox(width: 10),
                    TextButton(
                        onPressed: () async {
                          await controller.videoPause(true);
                        },
                        child: const Text('pause')),
                    TextButton(
                        onPressed: () async {
                          await controller.videoPause(false);
                        },
                        child: const Text('resume')),
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
                              onChanged: (double value) => controller.minRange = value,
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
                              onChanged: (double value) => controller.maxRange = value,
                            )),
                    GetBuilder<RealsenseController>(id: RealsenseController.BUILDER_SLIDER, builder: (controller) => Text(controller.rangeFilterValueMax.toStringAsFixed(1))),
                  ],
                ),
                const Text('Relu Threshold'),
                Row(
                  children: [
                    const Text('Threshold:'),
                    GetBuilder<RealsenseController>(
                        id: RealsenseController.BUILDER_RELU_SLIDER,
                        builder: (controller) => Slider(
                              min: 0.0,
                              max: 1.0,
                              divisions: 50,
                              value: controller.reluThresholdValue,
                              onChanged: (double value) => controller.reluThreshold = value,
                              onChangeEnd: (value) => controller.updateReluThreshold(value),
                            )),
                    GetBuilder<RealsenseController>(id: RealsenseController.BUILDER_RELU_SLIDER, builder: (controller) => Text(controller.reluThresholdValue.toStringAsFixed(2))),
                  ],
                ),
                Row(
                  children: [
                    const Text('Depth Zero Filter'),
                    GetBuilder<RealsenseController>(
                        id: RealsenseController.BUILDER_DEPTH_FILTER,
                        builder: (controller) => Switch(
                              value: controller.depthFilter,
                              onChanged: (value) => controller.enableDepthFilter(value),
                            ))
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
                ),
                Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                  const Text('Intrinsic'),
                  TextButton(
                      onPressed: () {
                        controller.getIntrinsic();
                      },
                      child: const Text('Update'))
                ]),
                GetBuilder<RealsenseController>(
                  builder: (controller) => Column(children: [
                    Row(children: [const Text('fx:'), const SizedBox(width: 8), Text(controller.fx.toStringAsFixed(6))]),
                    Row(children: [const Text('fy:'), const SizedBox(width: 8), Text(controller.fy.toStringAsFixed(6))]),
                    Row(children: [const Text('cx:'), const SizedBox(width: 8), Text(controller.cx.toStringAsFixed(6))]),
                    Row(children: [const Text('cy:'), const SizedBox(width: 8), Text(controller.cy.toStringAsFixed(6))]),
                  ]),
                ),
                Row(
                  children: [
                    const Text('Current Modes:'),
                    TextButton(
                        onPressed: () {
                          controller.getCurrentVideoMode();
                        },
                        child: const Text('Update')),
                  ],
                ),
                Row(children: [const Text('RGB:'), GetBuilder<RealsenseController>(builder: (controller) => Text(controller.currentModeRGB))]),
                Row(children: [const Text('Depth:'), GetBuilder<RealsenseController>(builder: (controller) => Text(controller.currentModeDepth))]),
                Row(children: [const Text('IR:'), GetBuilder<RealsenseController>(builder: (controller) => Text(controller.currentModeIR))]),
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
            ),
            TextButton(onPressed: () => controller.test(), child: const Text('Test'))
          ],
        ))
      ]),
    );
  }
}
