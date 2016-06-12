set(SRC_FILES
	twi.c
	adc.c
	can.c
	pwm.c
	timer.c
	usart.c
	spi.c
	system_messages.c
	utils.c
	sysclock.c
	cpu_load.c
	bson.c
	watchdog.c
	eeprom.c
)

add_library(libat90 ${SRC_FILES})

target_include_directories(libat90
	PRIVATE ${PROJECT_SOURCE_DIR}/drivers)

target_link_libraries(libat90 drivers)
