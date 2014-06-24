cmake_minimum_required(VERSION 2.6.0)

set(HEADER_FILES
	adc.h
	bitwise.h
	can.h
	io.h
	pwm.h
	ringbuffer.h
	timer.h
	usart.h)

set(SRC_FILES
	adc.c
	can.c
	pwm.c
	timer.c
	usart.c)


add_library(libat90 ${HEADER_FILES} ${SRC_FILES})
