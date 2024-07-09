import 'package:flutter/material.dart';
import 'package:flutter_vision3d_example/efficient_net/efficient_net.binding.dart';
import 'package:flutter_vision3d_example/efficient_net/efficient_net.view.dart';
import 'package:flutter_vision3d_example/faceRecognizer/fr.binding.dart';
import 'package:flutter_vision3d_example/faceRecognizer/fr.view.dart';
import 'package:flutter_vision3d_example/hand_detection/hand_detection.binding.dart';
import 'package:flutter_vision3d_example/hand_detection/hand_detection.view.dart';
import 'package:flutter_vision3d_example/handler/handler.binding.dart';
import 'package:flutter_vision3d_example/handler/handler.view.dart';
import 'package:flutter_vision3d_example/opencv/opencv.binding.dart';
import 'package:flutter_vision3d_example/opencv/opencv.view.dart';
import 'package:flutter_vision3d_example/openni2/openni2.binding.dart';
import 'package:flutter_vision3d_example/openni2/openni2.view.dart';
import 'package:flutter_vision3d_example/realsense/realsense.binding.dart';
import 'package:flutter_vision3d_example/realsense/realsense.view.dart';
import 'package:flutter_vision3d_example/ros/ros.binding.dart';
import 'package:flutter_vision3d_example/ros/ros.view.dart';
import 'package:flutter_vision3d_example/route.dart';
import 'package:flutter_vision3d_example/uvc/uvc.binding.dart';
import 'package:flutter_vision3d_example/uvc/uvc.view.dart';
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
        GetPage(name: AppRoutes.ros, page: () => const RosView(), binding: RosBinding()),
        GetPage(name: AppRoutes.realsense, page: () => const RealsenseView(), binding: RealsenseBinding()),
        GetPage(name: AppRoutes.openni, page: () => const OpenNIView(), binding: OpenNIBinding()),
        GetPage(name: AppRoutes.fr, page: () => const FaceRecognizerView(), binding: FaceRecognizerBinding()),
        GetPage(name: AppRoutes.efficientNet, page: () => const EfficientNetView(), binding: EfficientNetBinding()),
        GetPage(name: AppRoutes.handDetection, page: () => const HandDetectionView(), binding: HandDetectionBinding()),
        GetPage(name: AppRoutes.opencv, page: () => const OpencvView(), binding: OpencvBinding()),
        GetPage(name: AppRoutes.handler, page: () => const HandlerView(), binding: HandlerBinding()),
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
        title: const Text("Flutter Vision 3D Examples"),
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
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.uvc])),
          ),
          ListTile(
            title: const Text('Realsense'),
            subtitle: const Text('Realsense'),
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.realsense])),
          ),
          ListTile(
            title: const Text('OpenNI'),
            subtitle: const Text('OpenNI'),
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.openni])),
          ),
          ListTile(
            title: const Text('ROS Viewer'),
            subtitle: const Text('ROS Viewer'),
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.ros])),
          ),
          const Divider(),
          const ListTile(
            title: Text('Pipeline - OpenCV', style: TextStyle(fontSize: 20)),
            enabled: false,
          ),
          ListTile(
            title: const Text('OpenCV'),
            subtitle: const Text('Image process functions'),
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.opencv])),
          ),
          ListTile(
            title: const Text('Custom Handler'),
            subtitle: const Text('Tutorial for using custom handler written in C++'),
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.handler])),
          ),
          const Divider(),
          const ListTile(
            title: Text('Pipeline - Tensorflow Lite', style: TextStyle(fontSize: 20)),
            enabled: false,
          ),
          ListTile(
            title: const Text('Hand Detection'),
            subtitle: const Text('Hand Detection with model provided by MediaPipe'),
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.handDetection])),
          ),
          ListTile(
            title: const Text('Object Detection'),
            subtitle: const Text('Object Detection with Efficient Net Model'),
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.efficientNet])),
          ),
          ListTile(
            title: const Text('Facial Recognition'),
            subtitle: const Text('Facial Recognition with LIPSFace AI Model'),
            onTap: () => Get.toNamed(AppRoutes.join([AppRoutes.home, AppRoutes.fr])),
          ),
        ],
      ),
    );
  }
}
