cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "flutter_vision")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "flutter_vision_plugin")

include_directories(
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

target_link_libraries(${PLUGIN_NAME} PRIVATE OpenNI2)
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
target_link_libraries( ${PLUGIN_NAME} PRIVATE ${OpenCV_LIBS})

set(OpenGL_GL_PREFERENCE LEGACY)
find_package( OpenGL REQUIRED )
target_link_libraries( ${PLUGIN_NAME} PRIVATE ${OPENGL_LIBRARIES})

find_package( GLEW REQUIRED )
target_link_libraries( ${PLUGIN_NAME} PRIVATE GLEW::GLEW)

target_link_libraries( ${PLUGIN_NAME} PRIVATE tensorflowlite)


# List of absolute paths to libraries that should be bundled with the plugin
set(flutter_vision_bundled_libraries
  "${PROJECT_SOURCE_DIR}/lib/"
  PARENT_SCOPE
)
