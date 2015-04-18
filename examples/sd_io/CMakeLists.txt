set(NODE_NAME sd_io)

set(SRC_FILES
	main.c
)

add_executable(${NODE_NAME} ${SRC_FILES})

target_include_directories(${NODE_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90 ${PROJECT_SOURCE_DIR}/drivers)

target_link_libraries(${NODE_NAME} libat90 drivers)

avr_make_flashable(${NODE_NAME})
