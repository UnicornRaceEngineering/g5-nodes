SET(NAME fatfs)

set(SRC_FILES
	${THIRD_PARTY_SOURCE_DIR}/${NAME}/diskio.c
	${THIRD_PARTY_SOURCE_DIR}/${NAME}/ff.c
)

add_library(${NAME} ${SRC_FILES})

target_include_directories(${NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/drivers
	PRIVATE ${PROJECT_SOURCE_DIR}/libat90
	PRIVATE ${THIRD_PARTY_INCLUDE_DIR}
)

target_link_libraries(${NAME}
	drivers
	libat90
)
