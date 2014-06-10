set(NODE_NAME usart_pipe)

set(SRC_FILES
	main.c)

add_executable(${NODE_NAME} ${SRC_FILES})

target_include_directories(${NODE_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90
	PRIVATE ${PROJECT_SOURCE_DIR}/drivers
)

target_link_libraries(${NODE_NAME} libat90)

avr_make_flashable(${NODE_NAME})
