import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_vision_example/opencv/opencv.controller.dart';
import 'package:get/get.dart';
import 'package:path/path.dart';

final TextEditingController ctl = TextEditingController();

class OpencvView extends GetView<OpencvController> {
  const OpencvView({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    ctl.text = join(File(Platform.resolvedExecutable).parent.absolute.path, 'data', 'flutter_assets', 'assets', 'flutter_logo.png');
    ctl.text = 'C:/Users/Acer/Desktop/barcode.jpg';
    controller.imgPath = ctl.text;

    Size size = MediaQuery.of(context).size;
    return Scaffold(
      appBar: AppBar(
        title: const Text("Pipeline - OpenCV"),
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
                const Text('Image source'),
                Row(children: [
                  const Text('Path:'),
                  const SizedBox(width: 10),
                  SizedBox(
                      width: 220,
                      child: TextField(
                        controller: ctl,
                        onChanged: (value) => controller.imgPath = ctl.text,
                        textAlign: TextAlign.center,
                      )),
                ]),
                const Text('Pipelines:'),
                Column(children: [
                  Row(
                    children: [
                      TextButton(
                          onPressed: () {
                            controller.loadImage();
                          },
                          child: const Text('Load Image')),
                      const Text(': Load original image')
                    ],
                  ),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.cropImage();
                        },
                        child: const Text('Crop')),
                    const Text(': Crop image'),
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.resize();
                        },
                        child: const Text('Resize')),
                    const Text(': Resize image'),
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.colorMap();
                        },
                        child: const Text('ColorMap')),
                    const Text(': Convert to grayscale then apply color map'),
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.replaceColorMap();
                        },
                        child: const Text('Replace')),
                    const Text(': Replace pipeline function at specific index'),
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.drawRectangle();
                        },
                        child: const Text('Draw')),
                    const Text(': Draw rectangle on the source image'),
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.rotate();
                        },
                        child: const Text('Rotate')),
                    const Text(': Rotate image'),
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.runFromTo();
                        },
                        child: const Text('RunFromTo')),
                    const Text(': Run pipeline from index m to n'),
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () {
                          controller.getError();
                        },
                        child: const Text('GetError')),
                    const Text(': Get error message when running pipeline'),
                  ]),
                  Row(children: [
                    TextButton(
                        onPressed: () async {
                          var barcodes = await controller.getBarcode();
                          print(barcodes.map((e) => e.data));
                        },
                        child: const Text('Barcode')),
                    const Text(': Decode barcode from image'),
                  ]),
                ]),
              ],
            )),
        Expanded(
            child: Column(mainAxisAlignment: MainAxisAlignment.center, children: [
          GetBuilder<OpencvController>(builder: (controller) => Text('Current Pipeline: ${controller.pipelineInfo}')),
          const SizedBox(height: 50),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Text('Original'),
                  const SizedBox(height: 8),
                  Container(
                      decoration: BoxDecoration(border: Border.all(width: 1)),
                      width: 320,
                      height: 240,
                      child: GetBuilder<OpencvController>(
                        builder: (controller) {
                          return controller.originalCam == null
                              ? const SizedBox()
                              : Texture(
                                  textureId: controller.originalCam!.rgbTextureId,
                                );
                        },
                      )),
                ],
              ),
              const SizedBox(width: 32),
              Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Text('Processed'),
                  const SizedBox(height: 8),
                  Container(
                      decoration: BoxDecoration(border: Border.all(width: 1)),
                      width: 320,
                      height: 240,
                      child: GetBuilder<OpencvController>(
                        builder: (controller) {
                          return controller.processCam == null
                              ? const SizedBox()
                              : Texture(
                                  textureId: controller.processCam!.rgbTextureId,
                                );
                        },
                      ))
                ],
              )
            ],
          )
        ]))
      ]),
    );
  }
}
