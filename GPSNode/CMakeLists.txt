set(NODE_NAME GPSNode)

set(SRC_FILES
	src/gps.c
	src/main.c)

add_executable(${NODE_NAME} ${SRC_FILES})
target_include_directories(${NODE_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90)
target_link_libraries(${NODE_NAME} libat90)

avr_make_flashable(${NODE_NAME})


set(TEST_SRC_FILES
	tests/gps_test.c)

# Add the tests
add_executable(${NODE_NAME}_tests ${TEST_SRC_FILES})
target_include_directories(${NODE_NAME}_tests
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90)
target_link_libraries(${NODE_NAME}_tests libat90)

avr_make_flashable(${NODE_NAME}_tests)
