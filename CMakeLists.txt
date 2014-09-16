cmake_minimum_required(VERSION 2.6.0)
include(cmake/avr_crosscompile.cmake)
include(cmake/flags.cmake)

project(G5_NODES C)

add_subdirectory(libat90)
add_subdirectory(drivers)
add_subdirectory(TemplateNode)
add_subdirectory(GPSNode)
add_subdirectory(GearNode)
add_subdirectory(GearNodeDewalt)
add_subdirectory(SteeringNode)
add_subdirectory(examples)


