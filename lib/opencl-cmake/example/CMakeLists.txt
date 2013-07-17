cmake_minimum_required (VERSION 2.8)
project (opencl-example)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

find_package (OpenCL 1.0 REQUIRED)

include_directories (${OPENCL_INCLUDE_DIRS})

add_executable (${PROJECT_NAME} main.cpp)

target_link_libraries (${PROJECT_NAME} ${OPENCL_LIBRARIES})
