import 'package:flutter/material.dart';
import 'package:flutter_vision_example/efficient_net/efficient_net.binding.dart';
import 'package:flutter_vision_example/efficient_net/efficient_net.view.dart';
import 'package:flutter_vision_example/faceRecognizer/fr.binding.dart';
import 'package:flutter_vision_example/faceRecognizer/fr.view.dart';
import 'package:flutter_vision_example/realsense/realsense.binding.dart';
import 'package:flutter_vision_example/realsense/realsense.view.dart';
import 'package:flutter_vision_example/route.dart';
import 'package:flutter_vision_example/uvc/uvc.binding.dart';
import 'package:flutter_vision_example/uvc/uvc.view.dart';
import 'package:get/get.dart';

Future<void> main() async {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return GetMaterialApp(
      debugShowCheckedModeBanner: false,
      enableLog: true,
      // logWriterCallback: Logger.write,
      initialRoute: AppPages.defaultRoute,
      getPages: AppPages.routes,
      // unknownRoute: AppPages.unknownRoute,
      // locale: TranslationService.locale,
      // fallbackLocale: TranslationService.fallbackLocale,
      // translations: TranslationService(),
      // locale: TranslationService.locale,
      // fallbackLocale: TranslationService.fallbackLocale,
      // translations: TranslationService(),
    );
  }
}

class AppPages {
  static const defaultRoute = AppRoutes.home;

  static final List<GetPage> routes = [
    GetPage(
      name: AppRoutes.home,
      page: () => const HomeView(),
      children: [
        GetPage(name: AppRoutes.uvc, page: () => const UvcView(), binding: UvcBinding()),
        GetPage(name: AppRoutes.realsense, page: () => const RealsenseView(), binding: RealsenseBinding()),
        GetPage(name: AppRoutes.fr, page: () => const FaceRecognizerView(), binding: FaceRecognizerBinding()),
        GetPage(name: AppRoutes.efficientNet, page: () => const EfficientNetView(), binding: EfficientNetBinding()),
      ],
    ),
  ];
}

class HomeView extends StatelessWidget {
  const HomeView({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("Flutter Vision Examples"),
      ),
      body: ListView(
        children: [
          const ListTile(
            title: Text('Camera', style: TextStyle(fontSize: 20)),
            enabled: false,
          ),
          ListTile(
            title: const Text('UVC Camera'),
            subtitle: const Text('USB camera'),
            onTap: () => Get.toNamed('/home/uvc'),
          ),
          ListTile(
            title: const Text('Realsense'),
            subtitle: const Text('Realsense'),
            onTap: () => Get.toNamed('/home/realsense'),
          ),
          const Divider(),
          const ListTile(
            title: Text('Tensorflow Model', style: TextStyle(fontSize: 20)),
            enabled: false,
          ),
          ListTile(
            title: const Text('Facial Recognition'),
            subtitle: const Text('Facial Recognition with LIPSFace AI Model'),
            onTap: () => Get.toNamed('/home/fr'),
          ),
          ListTile(
            title: const Text('Object Detection'),
            subtitle: const Text('Object Detection with Efficient Net Model'),
            onTap: () => Get.toNamed('/home/efficient_net'),
          ),
        ],
      ),
    );
  }
}
