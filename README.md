# Flutter Vision

A framework for 2D & 3D image processing with AI (Tensorflow Lite)


## Prerequisite
### Linux

* `apt install ninja-build cmake build-essential libglew-dev libopencv-dev libglm-dev libgtk-3-dev`

### Windows
* Install `OpenCV`, `GLEW`,  and `GLFW`

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

## Enable/Disable camera video stream
```dart
await cam.enableStream()
await cam.disableStream()
```

## Bind texture widget
```dart
Texture(textureId: cam.rgbTextureId)
```

## Pipeline example
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
// TBD
```