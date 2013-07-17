cmake_minimum_required(VERSION 2.6)
project(HESP)

find_package(OpenGL)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/lib/opencl-cmake/")
find_package(OpenCL)

add_subdirectory(lib)
add_subdirectory(src)
