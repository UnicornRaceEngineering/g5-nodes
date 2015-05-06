set(NODE_NAME fatfs_io)

set(SRC_FILES
	main.c
)

add_executable(${NODE_NAME} ${SRC_FILES})

target_include_directories(${NODE_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90
	${PROJECT_SOURCE_DIR}/drivers
	${PROJECT_SOURCE_DIR}/third_party/include
)

target_link_libraries(${NODE_NAME} libat90 drivers fatfs)

avr_make_flashable(${NODE_NAME})
