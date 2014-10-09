set(SRC_FILES
	twi.c
	adc.c
	can.c
	pwm.c
	timer.c
	usart.c
	spi.c
	)

add_library(libat90 ${SRC_FILES})
