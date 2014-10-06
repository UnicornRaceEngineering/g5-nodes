set(SRC_FILES
	m41t81s_rtc.c
	max7221_7seg.c
	)

add_library(drivers ${SRC_FILES})

target_include_directories(drivers
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90)

target_link_libraries(drivers libat90)
