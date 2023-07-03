cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "flutter_vision")
project(${PROJECT_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

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
  "$ENV{OPENNI2_REDIST}"
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
find_package(OpenGL REQUIRED)
target_link_libraries(${PLUGIN_NAME} PRIVATE ${OPENGL_LIBRARIES})

find_package(GLEW REQUIRED)
target_link_libraries(${PLUGIN_NAME} PRIVATE GLEW::GLEW)

target_link_libraries(${PLUGIN_NAME} PRIVATE flutterVisionHandler)

# OpenCV
find_package(OpenCV 4.0.0 REQUIRED COMPONENTS core imgproc highgui)

if(OpenCV_FOUND)
  target_link_libraries(${PLUGIN_NAME} PRIVATE ${OpenCV_LIBS})
else()
  message("CANNOT FIND OPENCV LIBRARY")
endif()

# Realsense SDK
find_package(realsense2)

if(realsense2_FOUND)
  target_link_libraries(${PLUGIN_NAME} PRIVATE realsense2)
else()
  add_definitions(-DDISABLE_REALSENSE)
  message("BUILD WITHOUT REALSENSE SDK")
endif()

# OpenNI2 SDK
find_path(OPENNI2_INC NAMES OpenNI.h HINTS "$ENV{OPENNI2_INCLUDE}")
find_library(OPENNI2_LIB NAMES OpenNI2 HINTS "$ENV{OPENNI2_REDIST}")

if(OPENNI2_INC AND OPENNI2_LIB)
  include_directories($ENV{OPENNI2_INCLUDE})
  target_link_libraries(${PLUGIN_NAME} PRIVATE OpenNI2)
else()
  add_definitions(-DDISABLE_OPENNI)
  message("BUILD WITHOUT OPENNI2 SDK")
endif()

# Tensorflow Lite
find_library(TENSORFLOWLITE_LIB NAMES tensorflowlite REQUIRED)

if(TENSORFLOWLITE_LIB)
  target_link_libraries(${PLUGIN_NAME} PRIVATE tensorflowlite)
else()
  message("CANNOT FIND TENSORFLOW LITE LIBRARY")
endif()

# ROS Packages
# TODO: comment out /opt/ros/humble/include/rclcpp/rclcpp/any_subscription_callback.hpp:391
include_directories(
  "$ENV{AMENT_PREFIX_PATH}/include/rclcpp"
  "$ENV{AMENT_PREFIX_PATH}/include/rcl"
  "$ENV{AMENT_PREFIX_PATH}/include/rcutils"
  "$ENV{AMENT_PREFIX_PATH}/include/rmw"
  "$ENV{AMENT_PREFIX_PATH}/include/rcl_yaml_param_parser"
  "$ENV{AMENT_PREFIX_PATH}/include/rosidl_runtime_c"
  "$ENV{AMENT_PREFIX_PATH}/include/rosidl_typesupport_interface"
  "$ENV{AMENT_PREFIX_PATH}/include/rcpputils"
  "$ENV{AMENT_PREFIX_PATH}/include/builtin_interfaces"
  "$ENV{AMENT_PREFIX_PATH}/include/rosidl_runtime_cpp"
  "$ENV{AMENT_PREFIX_PATH}/include/tracetools"
  "$ENV{AMENT_PREFIX_PATH}/include/rcl_interfaces"
  "$ENV{AMENT_PREFIX_PATH}/include/libstatistics_collector"
  "$ENV{AMENT_PREFIX_PATH}/include/statistics_msgs"
  "$ENV{AMENT_PREFIX_PATH}/include/statistics_msgs"
  "$ENV{AMENT_PREFIX_PATH}/include/sensor_msgs"
  "$ENV{AMENT_PREFIX_PATH}/include/std_msgs"
  "$ENV{AMENT_PREFIX_PATH}/include/cv_bridge"
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_action.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_yaml_param_parser.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librclcpp_lifecycle.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_logging_spdlog.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_interfaces__rosidl_typesupport_introspection_c.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_interfaces__rosidl_generator_py.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_interfaces__rosidl_typesupport_fastrtps_c.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librclcpp_action.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_interfaces__rosidl_typesupport_introspection_cpp.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_lifecycle.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_interfaces__rosidl_generator_c.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_interfaces__rosidl_typesupport_c.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_interfaces__rosidl_typesupport_cpp.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_interfaces__rosidl_typesupport_fastrtps_cpp.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librcl_logging_interface.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/librclcpp.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/libsensor_msgs__rosidl_typesupport_cpp.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/libsensor_msgs__rosidl_typesupport_introspection_c.so"
  "$ENV{AMENT_PREFIX_PATH}/lib/libstd_msgs__rosidl_typesupport_c.so"
)

# List of absolute paths to libraries that should be bundled with the plugin
set(flutter_vision_bundled_libraries
  "${CMAKE_BINARY_DIR}/plugins/flutter_vision/handler/libflutterVisionHandler.so"
  PARENT_SCOPE
)