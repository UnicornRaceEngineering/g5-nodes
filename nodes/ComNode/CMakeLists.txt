set(NODE_NAME ComNode)

set(SRC_FILES
	main.c
	ecu.c
	xbee.c
	bson.c
)

add_executable(${NODE_NAME} ${SRC_FILES})

target_include_directories(${NODE_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90
	PRIVATE ${PROJECT_SOURCE_DIR}/drivers
	PRIVATE ${PROJECT_SOURCE_DIR}/third_party/petitfs/petitfs_root/src
)

target_link_libraries(${NODE_NAME} libat90 drivers)

avr_make_flashable(${NODE_NAME})
