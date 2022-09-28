cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "flutter_vision")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "flutter_vision_plugin")

add_subdirectory("${PROJECT_SOURCE_DIR}/../handler/" "handler/")

include_directories(
  "${PROJECT_SOURCE_DIR}/include/"
  "${PROJECT_SOURCE_DIR}/include/tensorflow"
)
link_directories(
  "${PROJECT_SOURCE_DIR}/lib"
)

add_library(${PLUGIN_NAME} SHARED
  "flutter_vision_plugin.cc"
)
apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter)
target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::GTK)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package( OpenGL REQUIRED )
target_link_libraries( ${PLUGIN_NAME} PRIVATE ${OPENGL_LIBRARIES})

find_package( GLEW REQUIRED )
target_link_libraries( ${PLUGIN_NAME} PRIVATE GLEW::GLEW)

target_link_libraries( ${PLUGIN_NAME} PRIVATE flutterVisionHandler)

# OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
if(OpenCV_FOUND)
  target_link_libraries( ${PLUGIN_NAME} PRIVATE ${OpenCV_LIBS})
else()
  message("CANNOT FIND OPENCV LIBRARY")
endif()

# Realsense SDK
find_package(realsense2)
if(realsense2_FOUND)
  target_link_libraries( ${PLUGIN_NAME} PRIVATE realsense2)
else()
  add_definitions(-DDISABLE_REALSENSE)
  message("BUILD WINOUT REALSENSE SDK")
endif()

# OpenNI2 SDK
find_path(OPENNI2_INC NAMES OpenNI.h HINTS "$ENV{OPENNI2_INCLUDE}")
find_library(OPENNI2_LIB NAMES OpenNI2 HINTS "$ENV{OPENNI2_REDIST}")
if(OPENNI2_INC AND OPENNI2_LIB)
  target_link_libraries(${PLUGIN_NAME} PRIVATE OpenNI2)
else()
  add_definitions(-DDISABLE_OPENNI)
  message("BUILD WINOUT OPENNI2 SDK")
endif()

# Tensorflow Lite
find_library(TENSORFLOWLITE_LIB NAMES tensorflowlite REQUIRED)
if(TENSORFLOWLITE_LIB)
  target_link_libraries( ${PLUGIN_NAME} PRIVATE tensorflowlite)
else()
  message("CANNOT FIND TENSORFLOW LITE LIBRARY")
endif()


# List of absolute paths to libraries that should be bundled with the plugin
set(flutter_vision_bundled_libraries
  "${CMAKE_BINARY_DIR}/plugins/flutter_vision/handler/libflutterVisionHandler.so"
  PARENT_SCOPE
)
