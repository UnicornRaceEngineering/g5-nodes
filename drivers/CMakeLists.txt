include_directories(${PROJECT_SOURCE_DIR}/libat90)
link_directories(${PROJECT_BINARY_DIR}/libat90)

set(SRC_FILES
	m41t81s_rtc.c)

add_library(drivers ${SRC_FILES})
target_link_libraries(drivers libat90)
