cmake_minimum_required (VERSION 3.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

enable_testing()

project (mumble-web-demo)
add_subdirectory (3rdparty)
add_subdirectory (lib)
add_subdirectory (src)
