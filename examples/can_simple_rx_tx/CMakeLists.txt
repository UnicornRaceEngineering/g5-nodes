include_directories(${PROJECT_SOURCE_DIR}/libat90)
link_directories(${PROJECT_BINARY_DIR}/libat90)

set(NODE_NAME can_simple_rx_tx)

set(SRC_FILES
	main.c)

add_executable(${NODE_NAME} ${SRC_FILES})
target_link_libraries(${NODE_NAME} libat90)

avr_make_flashable(${NODE_NAME})
