cmake_minimum_required(VERSION 3.27.7) # 3.28.1)

project(audio2waveform LANGUAGES CXX)

# include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.71.0 REQUIRED) # 1.82.0
find_package(Threads REQUIRED)
# find_package(FFmpeg REQUIRED) # COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE 
find_package(nlohmann_json REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil)

add_executable(${PROJECT_NAME} source/test.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${Boost_LIBRARIES}
    Threads::Threads
    PkgConfig::LIBAV
    nlohmann_json::nlohmann_json)