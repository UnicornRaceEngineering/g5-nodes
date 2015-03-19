set(ROOT petitfs_root)

set(SRC_FILES
	${ROOT}/src/diskio.c
	${ROOT}/src/pff.c
)

add_library(pff3 ${SRC_FILES})

target_include_directories(pff3
	PRIVATE ${PROJECT_SOURCE_DIR}/drivers)

target_link_libraries(pff3 drivers)
