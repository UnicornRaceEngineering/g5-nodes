# CMake version 2.8.12 required
# to use target_include_directories() function.
cmake_minimum_required(VERSION 2.8.12)

include(cmake/avr_crosscompile.cmake)
include(cmake/flags.cmake)

project(G5_NODES C)

add_subdirectory(third_party)
add_subdirectory(libat90)
add_subdirectory(drivers)
add_subdirectory(examples)
add_subdirectory(nodes)
