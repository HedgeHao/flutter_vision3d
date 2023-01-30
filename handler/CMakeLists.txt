cmake_minimum_required(VERSION 3.9)

set(TARGET flutterVisionHandler)

add_library(${TARGET} SHARED flutter_vision_handler.cpp)

IF (MSVC)
    file(TO_CMAKE_PATH $ENV{OPENCV_ROOT} OpenCV_DIR)
    IF(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release")
    ENDIF()
ENDIF()

set(OpenCV_DIR "$ENV{FLUTTER_VISION_DEP}/opencv")
find_package(OpenCV REQUIRED)
target_link_libraries( ${TARGET} ${OpenCV_LIBS})