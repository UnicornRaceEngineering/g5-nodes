/*
The MIT License (MIT)

Copyright (c) 2014 UnicornRaceEngineering

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file usart.c
 *
 * @brief
 * Provides USART input / output functions.
 *
 * By default functions for using both USART0 and USART1 is available with
 * buffered input and output. Each USART can be disabled by defining
 * NO_USART[n]_SUPPORT where n is either 0 or one 1. These must be defined at
 * compile time.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>     // for bool
#include <stddef.h>      // for size_t
#include <stdint.h>      // for uint8_t, uint32_t, uint16_t
#include <stdio.h>       // for FILE, NULL, stdin, stdout

#include "usart.h"
#include "utils.h"       // for HIGH_BYTE, LOW_BYTE

enum usart_charSelect_t {
	USART_CHAR_5BIT = 0x00,
	USART_CHAR_6BIT = 0x01,
	USART_CHAR_7BIT = 0x02,
	USART_CHAR_8BIT = 0x03,
	USART_CHAR_9BIT = 0x07
};

#ifndef NO_USART0_SUPPORT
#include "ringbuffer.h"  // for ringbuffer_t, rb_init, rb_pop, rb_push, etc

	static volatile ringbuffer_t usart0_rb_in;
	static volatile ringbuffer_t usart0_rb_out;

	FILE usart0_io = FDEV_SETUP_STREAM(usart0_putc, usart0_getc, _FDEV_SETUP_RW);
	FILE usart0_byte_output = FDEV_SETUP_STREAM(usart0_putbyte, NULL, _FDEV_SETUP_WRITE);
#endif

#ifndef NO_USART1_SUPPORT
	static volatile ringbuffer_t usart1_rb_in;
	static volatile ringbuffer_t usart1_rb_out;

	FILE usart1_io = FDEV_SETUP_STREAM(usart1_putc, usart1_getc, _FDEV_SETUP_RW);
	FILE usart1_byte_output = FDEV_SETUP_STREAM(usart1_putbyte, NULL, _FDEV_SETUP_WRITE);
#endif


/**
 * Convert a given baud-rate to UBRR prescalar.
 * @param  baudrate Target baud-rate
 * @param  mode     UART operation mode
 * @return          UBRR prescalar
 */
static inline uint16_t uart_baud2ubrr(const uint32_t baudrate, enum usart_operationModes_t mode){
	uint16_t ubrr_val;
	switch (mode) {
		case USART_MODE_ASYNC_NORMAL:
			ubrr_val = ((F_CPU / (baudrate * 16UL))) - 1;
			break;
		case USART_MODE_ASYNC_DOUBLE:
			ubrr_val = ((F_CPU / (baudrate * 8UL))) - 1;
			break;
		case USART_MODE_SYNC_MASTER:
			ubrr_val = ((F_CPU / (baudrate * 2UL))) - 1;
			break;
		default:
			ubrr_val = 0;
			break;
	}
	return ubrr_val;
}

/**
 * @name USART0
 * Functions for sending and receiving on USART0
 * @{
 */
#ifndef NO_USART0_SUPPORT

/**
 * Set up the USART with defaults values.
 * Enables RX and TX 1 stop bit with 8bit char size in async normal mode. if a
 * zero value baudrate is give it will default to 115200.
 * @param baudrate the desired baudrate
 */
int usart0_init(uint32_t baudrate, uint8_t* in_buf, size_t in_size, uint8_t* out_buf, size_t out_size) {
	if (baudrate == 0) {
		baudrate = 115200;
	}

	int rc;

	rc = rb_init((ringbuffer_t*)&usart0_rb_in, in_buf, in_size);
	if (rc != 0) return rc;
	USART0_ENABLE_RX_INTERRUPT();

	rc = rb_init((ringbuffer_t*)&usart0_rb_out, out_buf, out_size);
	if (rc != 0) return rc;

	//Enable TXen and RXen
	USART0_ENABLE_RX();
	USART0_ENABLE_TX();

	USART0_SET_1_STOP_BIT();
	USART0_SET_CHAR_SIZE(USART_CHAR_8BIT);

	// Baud rate
	usart0_setBaudrate(baudrate, USART_MODE_ASYNC_NORMAL);

	stdout = stdin = &usart0_io;
	return rc;
}

/**
 * Set USART baud-rate and operation mode.
 * @param baudrate baud-rate that the USART will use
 * @param mode     USART operation mode
 */
void usart0_setBaudrate(const uint32_t baudrate,
						enum usart_operationModes_t mode){
	switch (mode) {
		case USART_MODE_ASYNC_NORMAL: USART0_SET_MODE_ASYNC(); break;
		case USART_MODE_ASYNC_DOUBLE: USART0_SET_MODE_ASYNC(); break;
		case USART_MODE_SYNC_MASTER: USART0_SET_MODE_SYNC(); break;
	}

	const uint16_t prescale = uart_baud2ubrr(baudrate, mode);

	UBRR0L = LOW_BYTE(prescale);
	UBRR0H = HIGH_BYTE(prescale);
}

/**
 * Check the input buffer for new data.
 * @return  true if it as data. Else false
 */
bool usart0_has_data(void){
	return !rb_isEmpty(&usart0_rb_in);
}

/**
 * Get a byte from USART. This call is alway blocking. if input buffer is
 * enabled use usart[N]_hasData() to check if data is available.
 * @return  received byte
 */
int usart0_getc(FILE *stream) {
	char data;
	while (!usart0_has_data());
	rb_pop((ringbuffer_t*)&usart0_rb_in, (uint8_t*)&data);
	return (int)data;
}

int usart0_putbyte(char c, FILE *stream) {
	// Wait for free space in buffer
	while (rb_isFull(&usart0_rb_out));
	rb_push((ringbuffer_t*)&usart0_rb_out, c);

	USART0_ENABLE_UDRE_INTERRUPT();
	return 0;
}

/**
 * Put a byte on USART. unless USART0_NON_UNIX_LIKE_LINE_ENDINGS is defined this
 * will put a '\r' before every '\n' to mimic Unix like line endings.
 * @param  c Byte to transmit
 * @return   positive if success
 */
int usart0_putc(char c, FILE *stream) {
#ifndef USART0_NON_UNIX_LIKE_LINE_ENDINGS
	if (c == '\n') {
		usart0_putbyte('\r', stream);
	}
#endif
	usart0_putbyte(c, stream);
	return 0;
}

ISR(USART0_RX_vect){
	uint8_t data = UDR0;
	//!< @TODO should we not check if the rb is full here!?
	rb_push((ringbuffer_t*)&usart0_rb_in, data);
}

ISR(USART0_UDRE_vect){
	uint8_t data;
	if (rb_pop((ringbuffer_t*)&usart0_rb_out, &data) == 0) {
		UDR0 = data;
	} else {
		// output buffer is empty so disable UDRE interrupt flag
		USART0_DISABLE_UDRE_INTERRUPT();
	}
}

#endif /* NO_USART0_SUPPORT */
/** @} */




/**
 * @name USART1
 * Functions for sending and receiving on USART1
 * @{
 */
#ifndef NO_USART1_SUPPORT

/**
 * Set up the USART with defaults values.
 * Enables RX and TX 1 stop bit with 8bit char size in async normal mode. if a
 * zero value baudrate is give it will default to 115200.
 * @param baudrate the desired baudrate
 */
int usart1_init(uint32_t baudrate, uint8_t* in_buf, size_t in_size, uint8_t* out_buf, size_t out_size) {
	if (baudrate == 0) {
		baudrate = 115200;
	}

	int rc;

	rc = rb_init((ringbuffer_t*)&usart1_rb_in, in_buf, in_size);
	if (rc != 0) return rc;
	USART1_ENABLE_RX_INTERRUPT();

	rc = rb_init((ringbuffer_t*)&usart1_rb_out, out_buf, out_size);
	if (rc != 0) return rc;

	//Enable TXen and RXen
	USART1_ENABLE_RX();
	USART1_ENABLE_TX();

	USART1_SET_1_STOP_BIT();
	USART1_SET_CHAR_SIZE(USART_CHAR_8BIT);

	// Baud rate
	usart1_setBaudrate(baudrate, USART_MODE_ASYNC_NORMAL);

	stdout = stdin = &usart1_io;
	return rc;
}

/**
 * Set USART baud-rate and operation mode.
 * @param baudrate baud-rate that the USART will use
 * @param mode     USART operation mode
 */
void usart1_setBaudrate(const uint32_t baudrate,
						enum usart_operationModes_t mode){
	switch (mode) {
		case USART_MODE_ASYNC_NORMAL: USART1_SET_MODE_ASYNC(); break;
		case USART_MODE_ASYNC_DOUBLE: USART1_SET_MODE_ASYNC(); break;
		case USART_MODE_SYNC_MASTER: USART1_SET_MODE_SYNC(); break;
	}

	const uint16_t prescale = uart_baud2ubrr(baudrate, mode);

	UBRR1L = LOW_BYTE(prescale);
	UBRR1H = HIGH_BYTE(prescale);
}

/**
 * Check the input buffer for new data.
 * @return  true if it as data. Else false
 */
bool usart1_has_data(void){
	return !rb_isEmpty(&usart1_rb_in);
}

/**
 * Get a byte from USART. This call is alway blocking. if input buffer is
 * enabled use usart[N]_hasData() to check if data is available.
 * @return  received byte
 */
int usart1_getc(FILE *stream) {
	char data = 0;
	while (!usart1_has_data());
	rb_pop((ringbuffer_t*)&usart1_rb_in, (uint8_t*)&data);
	return (int)data;
}

int usart1_putbyte(char b, FILE *stream) {
	// Wait for free space in buffer
	while (rb_isFull(&usart1_rb_out));
	rb_push((ringbuffer_t*)&usart1_rb_out, b);

	USART1_ENABLE_UDRE_INTERRUPT();
	return 0;
}

/**
 * Put a byte on USART. unless USART1_NON_UNIX_LIKE_LINE_ENDINGS is defined this
 * will put a '\r' before every '\n' to mimic Unix like line endings.
 * @param  c Byte to transmit
 * @return   positive if success
 */
int usart1_putc(char c, FILE *stream) {
	if (c == '\n') {
		usart1_putbyte('\r', stream);
	}
	usart1_putbyte(c, stream);
	return 0;
}

ISR(USART1_RX_vect){
	uint8_t data = UDR1;
	rb_push((ringbuffer_t*)&usart1_rb_in, data);
}

ISR(USART1_UDRE_vect){
	uint8_t data;
	if (rb_pop((ringbuffer_t*)&usart1_rb_out, &data) == 0) {
		UDR1 = data;
	} else {
		// output buffer is empty so disable UDRE interrupt flag
		USART1_DISABLE_UDRE_INTERRUPT();
	}
}

#endif /* NO_USART1_SUPPORT */
/** @} */
