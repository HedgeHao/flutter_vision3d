cmake_minimum_required(VERSION 3.9)

set(TARGET flutter3dVisionHandler)

add_library(${TARGET} SHARED flutter_3d_vision_handler.cpp)

IF(MSVC)
    file(TO_CMAKE_PATH $ENV{OPENCV_ROOT} OpenCV_DIR)

    IF(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release")
    ENDIF()
ENDIF()

set(OpenCV_DIR "$ENV{FLUTTER_3D_VISION_DEP}/opencv")
find_package(OpenCV REQUIRED)
target_link_libraries(${TARGET} ${OpenCV_LIBS})