cmake_minimum_required(VERSION 2.8)
project(HESP)

find_package(OpenGL)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/lib/opencl-cmake/")
find_package(OpenCL)

if (APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  find_library(COCOA_LIB Cocoa)
  find_library(IOKIT_LIB IOKit)
endif (APPLE)

add_subdirectory(lib)
add_subdirectory(src)