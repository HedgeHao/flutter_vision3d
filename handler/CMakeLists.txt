cmake_minimum_required(VERSION 3.9)

set(TARGET flutterVisionHandler)

add_library(${TARGET} SHARED flutter_vision_handler.cpp)

find_package(OpenCV REQUIRED)
target_link_libraries( ${TARGET} ${OpenCV_LIBS})