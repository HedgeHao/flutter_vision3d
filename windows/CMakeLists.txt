cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "flutter_vision")
project(${PROJECT_NAME} LANGUAGES CXX)

# To avoid multiple warnings
if(CMAKE_BUILD_TOOL MATCHES "(MSBuild)")
  add_definitions(/wd4819 /wd4190 /W3 /W2 /W1)
endif()

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "flutter_vision_plugin")

message("FLUTTER_VISION_DEP=$ENV{FLUTTER_VISION_DEP}")
include_directories(
	"$ENV{FLUTTER_VISION_DEP}/opencv/include"
  "$ENV{FLUTTER_VISION_DEP}/realsense2/include"
  "$ENV{FLUTTER_VISION_DEP}/opengl/include"
  "$ENV{FLUTTER_VISION_DEP}/tensorflowlite/include/"
  "$ENV{FLUTTER_VISION_DEP}/zxing/include/"
	"$ENV{OPENNI2_INCLUDE64}"
  "${PROJECT_SOURCE_DIR}/include/"
)

add_library(${PLUGIN_NAME} SHARED
  "flutter_vision_plugin.cpp"
  "include/flutter_vision/opengl/opengl.cpp"
)
apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)


set(OpenCV_DIR "$ENV{FLUTTER_VISION_DEP}/opencv")
find_package(OpenCV REQUIRED)

target_link_libraries(${PLUGIN_NAME} PRIVATE
  "$ENV{FLUTTER_VISION_DEP}/realsense2/lib/x64/realsense2.lib"
  "$ENV{FLUTTER_VISION_DEP}/tensorflowlite/lib/tensorflowlite.dll.if.lib"
  "$ENV{FLUTTER_VISION_DEP}/opengl/lib/glew32.lib"
  "$ENV{FLUTTER_VISION_DEP}/opengl/lib/glfw3.lib"
  optimized "$ENV{FLUTTER_VISION_DEP}/zxing/lib/ZXing.lib"
  debug "$ENV{FLUTTER_VISION_DEP}/zxing/lib/ZXingd.lib"
  "$ENV{OPENNI2_LIB64}/OpenNI2.lib"
  ${OpenCV_LIBS}
  opengl32
  "$ENV{FLUTTER_VISION_DEP}/handler/build/flutterVisionHandler.lib"
)

# List of absolute paths to libraries that should be bundled with the plugin
set(flutter_vision_bundled_libraries
  "$ENV{FLUTTER_VISION_DEP}/tensorflowlite/bin/tensorflowlite.dll"
  "$ENV{FLUTTER_VISION_DEP}/opengl/bin/glew32.dll"
  "$ENV{FLUTTER_VISION_DEP}/realsense2/bin/x64/realsense2.dll"
  "$ENV{FLUTTER_VISION_DEP}/handler/build/flutterVisionHandler.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_barcode460.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_barcode460d.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_core460.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_core460d.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_dnn460.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_dnn460d.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_highgui460.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_highgui460d.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_imgcodecs460.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_imgcodecs460d.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_imgproc460.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_imgproc460d.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_videoio460.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_videoio460d.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_wechat_qrcode460.dll"
  "$ENV{FLUTTER_VISION_DEP}/opencv/x64/vc16/bin/opencv_wechat_qrcode460d.dll"
  "$ENV{OPENNI2_REDIST64}/OpenNI2"
  "$ENV{OPENNI2_REDIST64}/OpenNI.ini"
  "$ENV{OPENNI2_REDIST64}/OpenNI2.dll"
  PARENT_SCOPE
)