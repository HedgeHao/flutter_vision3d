import 'package:flutter/material.dart';
import 'package:flutter_vision_example/faceRecognizer/fr.controller.dart';
import 'package:get/get.dart';

final TextEditingController ctl = TextEditingController()..text = '0';

class FaceRecognizerView extends GetView<FaceRecognizerController> {
  const FaceRecognizerView({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    Size size = MediaQuery.of(context).size;
    return Scaffold(
      appBar: AppBar(
        title: const Text("UVC"),
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
                Row(children: [
                  const Text('Camera Index:'),
                  const SizedBox(width: 10),
                  SizedBox(
                      width: 50,
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
                          controller.openUvcCamera(ctl.text);
                        },
                        child: const Text('open')),
                    TextButton(
                        onPressed: () {
                          controller.closeUvcCamera(ctl.text);
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
                const Text('Pipeline:'),
                Column(children: [
                  TextButton(
                      onPressed: () {
                        controller.frPipeline();
                      },
                      child: const Text('Facial Recognition')),
                ]),
              ],
            )),
        Expanded(
            child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Container(
                decoration: BoxDecoration(border: Border.all(width: 1)),
                width: 320,
                height: 180,
                child: GetBuilder<FaceRecognizerController>(
                  builder: (controller) {
                    return controller.cams.isEmpty
                        ? const SizedBox()
                        : Stack(children: [
                            Texture(textureId: controller.cams.first.rgbTextureId),
                            ...controller.rects,
                          ]);
                  },
                ))
          ],
        ))
      ]),
    );
  }
}
