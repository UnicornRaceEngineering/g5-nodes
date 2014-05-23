cmake_minimum_required(VERSION 2.6.0)

include(${PROJECT_SOURCE_DIR}/cmake/avr_crosscompile.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/flags.cmake)

include_directories(${PROJECT_SOURCE_DIR}/libat90)
link_directories(${PROJECT_BINARY_DIR}/libat90)

set(SRC_FILES
	src/gps.c
	src/main.c)

add_executable(GPSNode ${SRC_FILES})
target_link_libraries(GPSNode libat90)

set(TEST_SRC_FILES
	tests/gps_test.c)

# Add the tests
add_executable(GPSNode_tests ${TEST_SRC_FILES})
target_link_libraries(GPSNode_tests libat90)

