# Flutter Vision

A framework for 2D & 3D image processing with AI (Tensorflow Lite)


---
## Prerequisite
### Linux

* `apt install ninja-build cmake build-essential libglew-dev libopencv-dev libglm-dev libgtk-3-dev`

### Windows
* Install `OpenCV`, `GLEW`,  and `GLFW`

---
## Create and Connect to Camera
* UVC Camera
```dart
UvcCamera? cam = await FvCamera.create(ctl.text, CameraType.UVC) as UvcCamera?;
```

* OpenNI2 Camera
```dart
OpenniCamera? cam = await FvCamera.create(ctl.text, CameraType.OPENNI) as OpenniCamera?;
```

* Realsense, LIPSedge AE400 and LIPSedge AE450
```dart
RealsenseCamera? cam = await FvCamera.create(ctl.text, CameraType.REALSENSE) as RealsenseCamera?;
```

* Virtual Camera (Load frame from file system without video stream)
```dart
DummyCamera? cam = await FvCamera.create(ctl.text, CameraType.DUMMY) as DummyCamera?;
```

---
## Enable/Disable camera video stream
```dart
await cam.enableStream();
await cam.disableStream();
```
---
## Bind texture widget
```dart
Texture(textureId: cam.rgbTextureId);
```
---
## Pipeline
* Display UVC video stream
```dart
FvPipeline uvcPipeline = cam.rgbPipeline;
await uvcPipeline.cvtColor(OpenCV.COLOR_BGR2RGBA);
await uvcPipeline.show();
```

* Display 3D camera depth and IR frame with color map applied
```dart
FvPipeline depthPipeline = cam.depthPipeline;
await depthPipeline.convertTo(0, 255.0 / 1024.0);
await depthPipeline.applyColorMap(OpenCV.COLORMAP_JET);
await depthPipeline.cvtColor(OpenCV.COLOR_RGB2RGBA);
await depthPipeline.show();
```

* Object detection with Efficient Net (Tensorflow Lite)
```dart
// Create Tensorflow Lite Model
TFLiteModel model = await TFLiteModel.create('/path/to/model.tflite');

// Use pipeline to set input for model
FvPipeline rgbPipeline = cam!.rgbPipeline;
await rgbPipeline.setInputTensorData(model!.index, 0, FvPipeline.DATATYPE_UINT8);
await rgbPipeline.inference(model!.index);

// Set the callback function. Called when inference is done.
FlutterVision.listen((MethodCall call) async {
    if (call.method == 'onInference') {
        ...
    }
});
```
---
## APIs
```dart
// Enumberation
enum CameraType { OPENNI, REALSENSE, DUMMY, UVC }


// FlutterVision Functions
class FvCamera {
    static Future<FvCamera?> create(String serial, CameraType type)
    Future<void> close()
    Future<bool> enableStream()
    Future<bool> disableStream()
    Future<void> enablePointCloud()
    Future<void> disablePointCloud()
    Future<bool> isConnected()
    Future<void> configure(int prop, double value)
    Future<bool> screenshot(int index, String path, {int? cvtCode})
}

class OpenniCamera extends FvCamera {}
class RealsenseCamera extends FvCamera {}
class UvcCamera extends FvCamera {}

class FlutterVision {
    static listen(Future<dynamic> Function(MethodCall) callback)
    static Future<int> niInitialize()
    static Future<List<OpenNi2Device>> enumerateDevices()
    static Future<List<String>> rsEnumerateDevices()

    static Future<int> getOpenglTextureId()
    static Future<void> openglRender()
}

class FvPipeline {
    Future<void> clear()
    Future<void> cvtColor(int mode, {int? at, int? interval, bool? append})
    Future<void> imwrite(String path, {int? at, int? interval, bool? append})
    Future<void> imread(String path, {int? at, int? interval, bool? append})
    Future<void> show({int? at, int? interval, bool? append})
    Future<void> convertTo(int mode, double scale, {int? at, double? shift, int? interval, bool? append})
    Future<void> applyColorMap(int colorMap, {int? at, int? interval, bool? append})
    Future<void> resize(int width, int height, {int? at, int? mode, int? interval, bool? append})
    Future<void> crop(int xStart, int xEnd, int yStart, int yEnd, {int? at, int? interval, bool? append})
    Future<void> rotate(int rotateCode, {int? at, int? interval, bool? append})
    Future<void> cvRectangle(double x1, double y1, double x2, double y2, int r, int g, int b, {int? at, int? thickness, int? lineType, int? shift, int? alpha, int? interval, bool? append})
    Future<void> setInputTensorData(int modelIndex, int tensorIndex, int dataType, {int? at, int? interval, bool? append})
    Future<void> inference(int modelIndex, {int? at, int? interval, bool? append})
    Future<void> customHandler(int size, {int? at, int? interval, bool? append})
    Future<int> run({int? from, int? to})
}

class TFLiteModel{
    static Future<TFLiteModel> create(modelPath)

    Future<Float32List> getTensorOutput(int tensorIndex, List<int> size)
}
```

## Supported 3D Camera
| Camera                | Supported | Tested | Product Link |
| --------------------- | --------- | ------ | ------------ |
| Intel Realsense D415  |     ✅    |   ✅   |  [Link](https://www.intelrealsense.com/depth-camera-d415/) |
| Intel Realsense D435  |     ✅    |   ✅   |  [Link](https://www.intelrealsense.com/depth-camera-d435/) |
| Intel Realsense D435i |     ✅    |        |  [Link](https://www.intelrealsense.com/depth-camera-d435i/) |
| Intel Realsense D455  |     ✅    |        |  [Link](https://www.intelrealsense.com/depth-camera-d455/) |
| Intel Realsense T265  |     ✅    |        |  [Link](https://www.intelrealsense.com/tracking-camera-t265/) |
| Intel Realsense L515  |     ✅    |        |  [Link](https://www.intelrealsense.com/lidar-camera-l515/) |
| LIPSedge AE400        |     ✅    |   ✅   |  [Link](https://www.lips-hci.com/lipsedge-ae400) |
| LIPSedge AE450        |     ✅    |   ✅   |  [Link](https://www.lips-hci.com/lipsedge-ae450) |
| LIPSedge DL           |     ✅    |   ✅   |  [Link](https://www.lips-hci.com/lipsedge-dl-series) |
| LIPSedge M3           |     ✅    |   ✅   |  [Link](https://www.lips-hci.com/lipsedge-m3-series) |
| LIPSedge L Series     |     ✅    |   ✅   |  [Link](https://www.lips-hci.com/lipsedge-l-series) |