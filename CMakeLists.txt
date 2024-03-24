cmake_minimum_required(VERSION 3.22.1)# 3.28.1)3.27.7

project(audio2waveform LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/conan_paths.cmake")
find_package(Boost REQUIRED) # 1.82.0
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenCV REQUIRED)
find_package(FFmpeg REQUIRED COMPONENTS avcodec avformat avutil avdevice swresample swscale avfilter)

add_executable(${PROJECT_NAME} source/test.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Boost_LIBRARIES}
    Threads::Threads
    nlohmann_json::nlohmann_json
    avcodec
    avformat
    avutil
    avdevice
    swresample
    swscale
    avfilter
    opencv_core 
    opencv_imgproc 
    opencv_highgui)