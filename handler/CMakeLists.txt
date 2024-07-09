cmake_minimum_required(VERSION 3.9)

set(TARGET flutterVision3dHandler)

IF(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
ENDIF()

add_library(${TARGET} SHARED flutter_vision3d_handler.cpp)

IF(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(OpenCV_DIR "$ENV{FLUTTER_VISION3D_DEP}/opencv")

    IF(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release")
    ENDIF()
ENDIF()

find_package(OpenCV REQUIRED)
target_link_libraries(${TARGET} ${OpenCV_LIBS})