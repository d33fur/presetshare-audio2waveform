cmake_minimum_required(VERSION 3.22.1)# 3.28.1)3.27.7

project(audio2waveform LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.84.0 REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)
find_package(SndFile REQUIRED)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)

file(GLOB_RECURSE SOURCES
    ${INCLUDE_DIR}/*.hpp
    ${SOURCE_DIR}/*.cpp)

include_directories(
    ${INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${Boost_LIBRARIES}
    Threads::Threads
    SndFile::sndfile
    ${OpenCV_LIBS})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${OpenCV_INCLUDE_DIRS})
