cmake_minimum_required(VERSION 2.6.0)

include_directories(
	${PROJECT_SOURCE_DIR}/libat90
	${PROJECT_SOURCE_DIR}/drivers
 	)
link_directories(
	${PROJECT_BINARY_DIR}/libat90
	${PROJECT_BINARY_DIR}/drivers
	)

set(NODE_NAME twi_master)

set(SRC_FILES
	main.c)

add_executable(${NODE_NAME} ${SRC_FILES})
target_link_libraries(${NODE_NAME} libat90 drivers)

avr_make_flashable(${NODE_NAME})
