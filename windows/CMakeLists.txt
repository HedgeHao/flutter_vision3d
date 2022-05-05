cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "flutter_vision")
project(${PROJECT_NAME} LANGUAGES CXX)

# To avoid multiple warnings
if(CMAKE_BUILD_TOOL MATCHES "(MSBuild)")
  add_definitions(/wd4190 /W3 /W2 /W1)
endif()


# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "flutter_vision_plugin")

set(OpenCV_DIR "D:/bin/opencv/prebuilt/opencv/build")
set(OPENNI_DIR "C:/Program Files/OpenNI2")
include_directories(
	"${OpenCV_DIR}/include"
	"${OPENNI_DIR}/Include"
  "${PROJECT_SOURCE_DIR}/include/"
  "${PROJECT_SOURCE_DIR}/include/tensorflow"
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

find_package(OpenCV REQUIRED PATHS ${OpenCV_DIR})
target_link_libraries(${PLUGIN_NAME} PRIVATE
  "${PROJECT_SOURCE_DIR}/lib/tensorflowlite_c.dll.if.lib"
  "${PROJECT_SOURCE_DIR}/lib/tensorflowlite.dll.if.lib"
  "${PROJECT_SOURCE_DIR}/lib/flatbuffers.lib"
  "${PROJECT_SOURCE_DIR}/lib/glew32.lib"
  "${PROJECT_SOURCE_DIR}/lib/glfw3.lib"
  "${OPENNI_DIR}/Lib/OpenNI2.lib"
  ${OpenCV_LIBS}
  opengl32
)

# List of absolute paths to libraries that should be bundled with the plugin
set(flutter_vision_bundled_libraries
  ""
  PARENT_SCOPE
)

# add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD 
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different
#  "./OpenNI"
#  "${CMAKE_BINARY_DIR}/runner/Debug/OpenNI"
#  COMMENT "Copying 'OpenNI2.dll' to ${CMAKE_BINARY_DIR}/runner/Debug/"
# )