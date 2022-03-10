import 'package:flutter/material.dart';
import 'package:flutter_vision/flutter_vision.dart';
import 'package:flutter_vision_example/ui.dart';
import 'package:flutter_vision_example/viewModel.dart' as ViewModel;

class VideoConfig extends StatefulWidget {
  const VideoConfig({Key? key}) : super(key: key);

  @override
  VideoConfigState createState() => VideoConfigState();
}

class VideoStreamingConfig extends StatelessWidget {
  const VideoStreamingConfig({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        TextButton(
            onPressed: () async {
              ViewModel.configuration.lastRequestTS = DateTime.now().millisecondsSinceEpoch + 8000; // Prevent first wrong recognition
              await FlutterVision.configVideoStream(7, true);
            },
            child: const Text('Start')),
        TextButton(
            onPressed: () async {
              await FlutterVision.configVideoStream(7, false);
            },
            child: const Text('Stop')),
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
                  int result = await FlutterVision.openDevice(ViewModel.configuration.selectedDevice!);
                  int enabledVideoMode = await FlutterVision.getEnabledVideoModes();
                  setState(() {
                    isConnected = result >= 0;
                    ViewModel.configuration.setVideoModesInitialized(enabledVideoMode);
                  });
                }
              },
              child: const Text('Connect')),
          TextButton(
              onPressed: () {
                FlutterVision.closeDevice().then((value) {
                  FlutterVision.deviceIsConnected().then((isValid) {
                    setState(() {
                      isConnected = isValid;
                    });
                  });
                });
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
              await FlutterVision.cameraOpen(int.parse(ctl.text));
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
        const Spacer(),
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
            StaticUI.divider,
            Text('Device', style: subTitleStyle),
            DeviceDropdown(),
            const VideoConfig(),
            StaticUI.divider,
            Text('Streaming', style: subTitleStyle),
            const VideoStreamingConfig(),
            StaticUI.divider,
            Text('2D Camera', style: subTitleStyle),
            Camera2dConfigure(),
          ],
        ));
  }
}
