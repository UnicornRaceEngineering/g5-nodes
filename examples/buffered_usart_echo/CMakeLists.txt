cmake_minimum_required(VERSION 2.6.0)

include(${PROJECT_SOURCE_DIR}/cmake/avr_crosscompile.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/flags.cmake)

include_directories(${PROJECT_SOURCE_DIR}/libat90)
link_directories(${PROJECT_BINARY_DIR}/libat90)

set(NODE_NAME buffered_usart_echo)

set(SRC_FILES
	main.c)

add_executable(${NODE_NAME} ${SRC_FILES})
target_link_libraries(${NODE_NAME} libat90)

avr_make_flashable(${NODE_NAME})
