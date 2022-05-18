import 'package:collection/src/iterable_extensions.dart';
import 'package:flutter/material.dart';
import 'package:flutter_switch/flutter_switch.dart';
import 'package:flutter_vision/camera/openni.dart';
import 'package:flutter_vision/camera/realsense.dart';
import 'package:flutter_vision/constants.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision_example/ui.dart';
import 'package:flutter_vision_example/viewModel.dart' as ViewModel;

class VideoConfig extends StatefulWidget {
  const VideoConfig({Key? key}) : super(key: key);

  @override
  VideoConfigState createState() => VideoConfigState();
}

class VideoStreamingConfig extends StatefulWidget {
  @override
  VideoStreamingConfigState createState() => VideoStreamingConfigState();
}

class VideoStreamingConfigState extends State<VideoStreamingConfig> {
  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        TextButton(
            onPressed: () async {
              ViewModel.configuration.niCams.firstWhereOrNull((e) => e.serial == ViewModel.configuration.selectedDevice!.uri!)?.enableStream();
            },
            child: const Text('Start')),
        TextButton(
            onPressed: () async {
              ViewModel.configuration.niCams.firstWhereOrNull((e) => e.serial == ViewModel.configuration.selectedDevice!.uri!)?.disableStream();
            },
            child: const Text('Stop')),
        const Text('PointCloud'),
        const SizedBox(width: 5),
        SizedBox(
            height: 25,
            width: 50,
            child: FlutterSwitch(
                value: ViewModel.configuration.niPointCloud,
                onToggle: (v) async {
                  if (ViewModel.configuration.niCams.isEmpty) return;

                  v ? ViewModel.configuration.niCams[0].enablePointCloud() : ViewModel.configuration.niCams[0].disablePointCloud();

                  setState(() {
                    ViewModel.configuration.niPointCloud = v;
                  });
                }))
      ],
    );
  }
}

class VideoConfigState extends State<VideoConfig> {
  bool isConnected = false;
  List<ViewModel.VideoModeData> videoModes = ViewModel.configuration.videoModes;

  @override
  void initState() {
    ViewModel.configuration.resetVideModesChecked();
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    List<Widget> widgets = [];

    widgets.addAll([
      Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          TextButton(
              onPressed: () async {
                if (ViewModel.configuration.selectedDevice != null) {
                  OpenniCamera? cam = ViewModel.configuration.niCams.firstWhereOrNull((e) => e.serial == ViewModel.configuration.selectedDevice!.uri!);

                  if (cam == null) {
                    cam = await OpenniCamera.create(ViewModel.configuration.selectedDevice!.uri!);
                    if (cam == null) {
                      print('Create Camera Failed');
                      return;
                    }
                    ViewModel.configuration.niCams.add(cam);
                  }

                  isConnected = await cam.isConnected();
                  setState(() {});
                }
              },
              child: const Text('Connect')),
          TextButton(
              onPressed: () {
                ViewModel.configuration.niCams.firstWhereOrNull((e) => e.serial == ViewModel.configuration.selectedDevice!.uri!)?.close().then((value) => setState);
              },
              child: const Text('Disconnect')),
        ],
      ),
      Padding(
          padding: const EdgeInsets.only(left: 8),
          child: Row(
            children: [
              const Text('Status:'),
              const SizedBox(width: 8),
              Text(isConnected ? 'Connected' : 'Disconnected', style: TextStyle(color: isConnected ? Colors.green : Colors.red)),
              IconButton(
                  onPressed: () {
                    FlutterVision.deviceIsConnected().then((isValid) {
                      setState(() {
                        isConnected = isValid;
                      });
                    });
                  },
                  splashRadius: 15,
                  icon: const Icon(Icons.refresh))
            ],
          ))
    ]);

    return Column(children: widgets);
  }
}

class DeviceDropdown extends StatefulWidget {
  @override
  DeviceDropdownState createState() => DeviceDropdownState();
}

class DeviceDropdownState extends State<DeviceDropdown> {
  List<OpenNi2Device> deviceList = <OpenNi2Device>[];
  int selected = 0;

  @override
  Widget build(BuildContext context) {
    List<DropdownMenuItem<int>>? items = [];
    int index = 0;
    for (var d in deviceList) {
      items.add(DropdownMenuItem<int>(
        child: Text('${d.vendor}:${d.name}'),
        value: index,
      ));
      index++;
    }

    return Padding(
        padding: const EdgeInsets.only(left: 8),
        child: Row(children: [
          const Text('Device List:'),
          const SizedBox(width: 10),
          DropdownButton(
              value: selected,
              selectedItemBuilder: (context) {
                return deviceList.map((e) => Center(child: Text('${e.vendor}:${e.name}'))).toList();
              },
              items: items,
              onChanged: (value) {
                ViewModel.configuration.selectedDevice = deviceList[value as int];
                setState(() {
                  selected = value;
                });
              }),
          TextButton(
              onPressed: () async {
                List<OpenNi2Device> list = await FlutterVision.enumerateDevices();
                setState(() {
                  deviceList = list;
                });
                if (list.isNotEmpty) {
                  ViewModel.configuration.selectedDevice = deviceList[0];
                }
              },
              child: const Text('Refresh'))
        ]));
  }
}

class Camera2dConfigure extends StatelessWidget {
  final TextEditingController ctl = TextEditingController()..text = '0';

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        const Spacer(),
        SizedBox(width: 50, child: TextField(controller: ctl, textAlign: TextAlign.center)),
        TextButton(
            onPressed: () async {
              int index = int.parse(ctl.text);
              await FlutterVision.cameraOpen(int.parse(ctl.text));
              await FlutterVision.uvcConfig(index, OpenCV.CAP_PROP_MODE, 1);
              await FlutterVision.uvcConfig(index, OpenCV.CAP_PROP_FPS, 30.0);
              await FlutterVision.uvcConfig(index, OpenCV.CAP_PROP_FRAME_WIDTH, 640);
              await FlutterVision.uvcConfig(index, OpenCV.CAP_PROP_FRAME_HEIGHT, 480);
            },
            child: const Text('Open')),
        TextButton(
            onPressed: () async {
              await FlutterVision.cameraConfig(int.parse(ctl.text), true);
            },
            child: const Text('Start')),
        TextButton(
            onPressed: () async {
              await FlutterVision.cameraConfig(int.parse(ctl.text), false);
            },
            child: const Text('Stop')),
        TextButton(
            onPressed: () async {
              await FlutterVision.videoScreenshot(16, 'test.png');
            },
            child: const Text('Shot')),
        const Spacer(),
      ],
    );
  }
}

class RsDeviceDropdown extends StatefulWidget {
  @override
  RsDeviceDropdownState createState() => RsDeviceDropdownState();
}

class RsDeviceDropdownState extends State<RsDeviceDropdown> {
  List<String> deviceList = <String>[];
  int selected = 0;

  @override
  Widget build(BuildContext context) {
    List<DropdownMenuItem<int>>? items = [];
    int index = 0;
    for (var d in deviceList) {
      items.add(DropdownMenuItem<int>(
        child: Text(d),
        value: index,
      ));
      index++;
    }

    return Padding(
        padding: const EdgeInsets.only(left: 8),
        child: Row(children: [
          const Text('Device List:'),
          const SizedBox(width: 10),
          DropdownButton(
              value: selected,
              selectedItemBuilder: (context) {
                return deviceList.map((e) => Center(child: Text(e))).toList();
              },
              items: items,
              onChanged: (value) {
                ViewModel.configuration.selectedRsDevice = deviceList[value as int];
                setState(() {
                  selected = value;
                });
              }),
          TextButton(
              onPressed: () async {
                List<String> list = await FlutterVision.rsEnumerateDevices();
                setState(() {
                  deviceList = list;
                });
                if (list.isNotEmpty) {
                  ViewModel.configuration.selectedRsDevice = deviceList[0];
                }
              },
              child: const Text('Refresh'))
        ]));
  }
}

class RsVideoConfig extends StatefulWidget {
  const RsVideoConfig({Key? key}) : super(key: key);

  @override
  RsVideoConfigState createState() => RsVideoConfigState();
}

class RsVideoConfigState extends State<RsVideoConfig> {
  bool isConnected = false;

  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    List<Widget> widgets = [];

    widgets.addAll([
      Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          TextButton(
              onPressed: () async {
                RealsenseCamera? cam = ViewModel.configuration.rsCams.firstWhereOrNull((e) => e.serial == ViewModel.configuration.selectedRsDevice);

                if (cam == null) {
                  cam = await RealsenseCamera.create(ViewModel.configuration.selectedRsDevice);
                  if (cam == null) {
                    print('Create Camera Failed');
                    return;
                  }
                  ViewModel.configuration.rsCams.add(cam);
                }

                isConnected = await cam.isConnected();
                setState(() {});
              },
              child: const Text('Connect')),
          TextButton(
              onPressed: () {
                ViewModel.configuration.rsCams.firstWhereOrNull((e) => e.serial == ViewModel.configuration.selectedRsDevice)?.close().then((value) => setState);
              },
              child: const Text('Disconnect')),
        ],
      ),
      Padding(
          padding: const EdgeInsets.only(left: 8),
          child: Row(
            children: [
              const Text('Status:'),
              const SizedBox(width: 8),
              Text(isConnected ? 'Connected' : 'Disconnected', style: TextStyle(color: isConnected ? Colors.green : Colors.red)),
              IconButton(
                  onPressed: () {
                    FlutterVision.deviceIsConnected().then((isValid) {
                      setState(() {
                        isConnected = isValid;
                      });
                    });
                  },
                  splashRadius: 15,
                  icon: const Icon(Icons.refresh))
            ],
          ))
    ]);

    return Column(children: widgets);
  }
}

class RsVideoStreamingConfig extends StatefulWidget {
  @override
  RsVideoStreamingConfigState createState() => RsVideoStreamingConfigState();
}

class RsVideoStreamingConfigState extends State<RsVideoStreamingConfig> {
  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        TextButton(
            onPressed: () async {
              ViewModel.configuration.rsCams.firstWhereOrNull((e) => e.serial == ViewModel.configuration.selectedRsDevice)?.enableStream();
            },
            child: const Text('Start')),
        TextButton(
            onPressed: () async {
              ViewModel.configuration.rsCams.firstWhereOrNull((e) => e.serial == ViewModel.configuration.selectedRsDevice)?.disableStream();
            },
            child: const Text('Stop')),
        const Text('PointCloud'),
        const SizedBox(width: 5),
        SizedBox(
            height: 25,
            width: 50,
            child: FlutterSwitch(
                value: ViewModel.configuration.rsPointCloud,
                onToggle: (v) async {
                  if (ViewModel.configuration.rsCams.isEmpty) return;

                  v ? ViewModel.configuration.rsCams[0].enablePointCloud() : ViewModel.configuration.rsCams[0].disablePointCloud();

                  setState(() {
                    ViewModel.configuration.rsPointCloud = v;
                  });
                }))
      ],
    );
  }
}

class ConfigurePannel extends StatelessWidget {
  const ConfigurePannel({Key? key}) : super(key: key);
  final subTitleStyle = const TextStyle(fontSize: 18, fontWeight: FontWeight.w500);

  @override
  Widget build(BuildContext context) {
    return SizedBox(
        width: 300,
        child: Column(
          children: [
            const Text('Configuration', style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold)),
            StaticUI.divider,
            Text('OpenNI2', style: subTitleStyle),
            TextButton(
                onPressed: () async {
                  await FlutterVision.initialize();
                },
                child: const Text('Ni2 Init')),
            Text('Device', style: subTitleStyle),
            DeviceDropdown(),
            const VideoConfig(),
            Text('Streaming', style: subTitleStyle),
            VideoStreamingConfig(),
            StaticUI.divider,
            Text('2D Camera', style: subTitleStyle),
            Camera2dConfigure(),
            StaticUI.divider,
            Text('Realsense', style: subTitleStyle),
            RsDeviceDropdown(),
            const RsVideoConfig(),
            RsVideoStreamingConfig(),
          ],
        ));
  }
}
