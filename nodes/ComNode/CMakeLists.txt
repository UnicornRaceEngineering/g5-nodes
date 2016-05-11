set(NODE_NAME ComNode)

set(SRC_FILES
	main.c
	ecu.c
	xbee.c
	log.c
	protocol.c
	flags.c
	send_file.c
)

add_executable(${NODE_NAME} ${SRC_FILES})

target_include_directories(${NODE_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90
	PRIVATE ${PROJECT_SOURCE_DIR}/drivers
	PRIVATE ${PROJECT_SOURCE_DIR}/third_party/include
)

target_link_libraries(${NODE_NAME} libat90 drivers fatfs)

avr_make_flashable(${NODE_NAME})
