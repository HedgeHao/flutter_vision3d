import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_vision_example/handler/handler.controller.dart';
import 'package:get/get.dart';
import 'package:path/path.dart';

final TextEditingController ctl = TextEditingController();

class HandlerView extends GetView<HandlerController> {
  const HandlerView({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    ctl.text = join(File(Platform.resolvedExecutable).parent.absolute.path, 'data', 'flutter_assets', 'assets', 'flutter_logo.png');
    controller.imgPath = ctl.text;

    Size size = MediaQuery.of(context).size;
    return Scaffold(
      appBar: AppBar(
        title: const Text("Custom Handler"),
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
                            controller.handler();
                          },
                          child: const Text('Custom Handler')),
                      const Text(': Custom handler')
                    ],
                  ),
                ]),
              ],
            )),
        Expanded(
          child: Center(
              child: Container(
                  decoration: BoxDecoration(border: Border.all(width: 1)),
                  width: 640,
                  height: 320,
                  child: GetBuilder<HandlerController>(
                    builder: (controller) {
                      return controller.cam == null
                          ? const SizedBox()
                          : Texture(
                              textureId: controller.cam!.rgbTextureId,
                            );
                    },
                  ))),
        )
      ]),
    );
  }
}
