cmake_minimum_required(VERSION 2.6.0)

include_directories(${PROJECT_SOURCE_DIR}/libat90)
link_directories(${PROJECT_BINARY_DIR}/libat90)

set(HEADER_FILES
	m41t81s_rtc.h
	)

set(SRC_FILES
	m41t81s_rtc.c
	)

add_library(drivers ${HEADER_FILES} ${SRC_FILES})
target_link_libraries(drivers libat90)
