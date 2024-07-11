import 'package:flutter/material.dart';
import 'package:flutter_vision3d/flutter_vision3d.dart';
import 'package:flutter_vision3d_example/openni2/openni2.controller.dart';
import 'package:flutter_vision3d_example/widgets/pointcloud/pointcloud.view.dart';
import 'package:get/get.dart';

class OpenNIView extends GetView<OpenNIController> {
  const OpenNIView({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    Size size = MediaQuery.of(context).size;

    return Scaffold(
      appBar: AppBar(
        title: const Text("OpenNI"),
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
                    GetBuilder<OpenNIController>(
                        id: OpenNIController.BUILDER_DEVICE_LIST,
                        builder: ((controller) {
                          controller.items.clear();
                          int index = 0;
                          for (var d in controller.deviceList) {
                            controller.items.add(DropdownMenuItem<int>(
                              child: Text(d.name),
                              value: index,
                            ));
                            index++;
                          }

                          return DropdownButton(
                              value: controller.selected,
                              selectedItemBuilder: (context) {
                                return controller.deviceList.map((e) => Center(child: Text(e.name))).toList();
                              },
                              items: controller.items,
                              onChanged: (value) {
                                controller.selectedNiDevice = controller.deviceList[value as int];
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
                Row(children: [const Text('Serial Number:'), GetBuilder<OpenNIController>(builder: (controller) => Text(controller.sn))]),
                Row(
                  children: [
                    const Text('Device:'),
                    const SizedBox(width: 10),
                    TextButton(
                        onPressed: () {
                          controller.openOpenNICamera();
                        },
                        child: const Text('open')),
                    TextButton(
                        onPressed: () {
                          controller.closeOpenNICamera();
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
                        child: const Text('IR Frame')),
                    const Text(': IR frame with greys color map')
                  ])
                ]),
                const Divider(),
                const Align(alignment: Alignment.center, child: Text('Configuration')),
                Row(
                  children: [
                    const Text('PointCloud:'),
                    GetBuilder<OpenNIController>(
                        id: OpenNIController.BUILDER_TEXTURE_OPENGL,
                        builder: (controller) => Switch(
                              value: controller.pointCloud,
                              onChanged: (value) => controller.enablePointCloud(value),
                            ))
                  ],
                ),
                Row(
                  children: [
                    const Text('Image Rigistration'),
                    GetBuilder<OpenNIController>(
                        builder: (controller) => Switch(
                              value: controller.registration,
                              onChanged: (value) => controller.enableRegistration(value),
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
                GetBuilder<OpenNIController>(
                  builder: (controller) => Column(children: [
                    Row(children: [const Text('fx:'), const SizedBox(width: 8), Text(controller.fx.toStringAsFixed(6))]),
                    Row(children: [const Text('fy:'), const SizedBox(width: 8), Text(controller.fy.toStringAsFixed(6))]),
                    Row(children: [const Text('cx:'), const SizedBox(width: 8), Text(controller.cx.toStringAsFixed(6))]),
                    Row(children: [const Text('cy:'), const SizedBox(width: 8), Text(controller.cy.toStringAsFixed(6))]),
                  ]),
                ),
                Row(
                  children: [
                    const Text('Available Video Modes:'),
                    TextButton(onPressed: () => controller.getVideoModes(StreamIndex.RGB), child: const Text('Color')),
                    TextButton(onPressed: () => controller.getVideoModes(StreamIndex.DEPTH), child: const Text('Depth')),
                    TextButton(onPressed: () => controller.getVideoModes(StreamIndex.IR), child: const Text('IR')),
                  ],
                ),
                GetBuilder<OpenNIController>(builder: (controller) => Text(controller.videoModes)),
                Row(
                  children: [
                    const Text('Set Video Mode: '),
                    DropdownButton(
                        value: controller.selectedVideoModeItem,
                        items: controller.videoModeItems,
                        onChanged: (value) {
                          controller.selectedVideoModeItem = value as int;
                        }),
                    SizedBox(
                      width: 50,
                      child: TextField(
                        controller: controller.videoModeCtl,
                        textAlign: TextAlign.center,
                      ),
                    ),
                    TextButton(
                        onPressed: () {
                          controller.setVideoMode();
                        },
                        child: const Text('Set'))
                  ],
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
                Row(children: [const Text('RGB:'), GetBuilder<OpenNIController>(builder: (controller) => Text(controller.currentModeRGB))]),
                Row(children: [const Text('Depth:'), GetBuilder<OpenNIController>(builder: (controller) => Text(controller.currentModeDepth))]),
                Row(children: [const Text('IR:'), GetBuilder<OpenNIController>(builder: (controller) => Text(controller.currentModeIR))]),
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
                  child: GetBuilder<OpenNIController>(
                    id: OpenNIController.BUILDER_TEXTURE,
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
                  child: GetBuilder<OpenNIController>(
                    id: OpenNIController.BUILDER_TEXTURE,
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
                  child: GetBuilder<OpenNIController>(
                    id: OpenNIController.BUILDER_TEXTURE,
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
            GetBuilder<OpenNIController>(
              id: OpenNIController.BUILDER_TEXTURE_OPENGL,
              builder: (controller) => controller.pointCloud
                  ? PointCloud(
                      width: 540,
                      height: 360,
                      textureId: controller.openglTextureId,
                      quarterTurns: 2,
                    )
                  : const SizedBox(),
            ),
          ],
        ))
      ]),
    );
  }
}
