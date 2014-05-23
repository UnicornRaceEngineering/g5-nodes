cmake_minimum_required(VERSION 2.6.0)

include(${PROJECT_SOURCE_DIR}/cmake/avr_crosscompile.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/flags.cmake)

include_directories(${PROJECT_SOURCE_DIR}/libat90)
link_directories(${PROJECT_BINARY_DIR}/libat90)

set(SRC_FILES
	main.c)

add_executable(GearNode ${SRC_FILES})
target_link_libraries(GearNode libat90)
