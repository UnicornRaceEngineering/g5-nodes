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
 * NO_USART[n]_SUPPORT where n is either 0 or one 1. Furthermore either buffered
 * input or buffered output can be disabled for each USART by defining
 * NO_USART[n]_BUFFERED_INPUT or NO_USART[n]_BUFFERED_OUTPUT. These must be
 * defined at compile time
 *
 * @bug
 * There seems to be an issue when using buffered output that causes the board
 * to reset. Lookup issue #2
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> // size_t
#include <stdarg.h> // va args
#include <stdio.h> // vsprintf

#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitwise.h"
#include "usart.h"

#ifndef NO_USART0_SUPPORT
	#ifndef NO_USART0_BUFFERED_INPUT
		#include "ringbuffer.h"
		static volatile ringbuffer_t usart0_inBuff = {{0}};
	#endif

	#ifndef NO_USART0_BUFFERED_OUTPUT
		#include "ringbuffer.h"
		static volatile ringbuffer_t usart0_outBuff = {{0}};
	#endif
#endif

#ifndef NO_USART1_SUPPORT
	#ifndef NO_USART1_BUFFERED_INPUT
		#include "ringbuffer.h"
		static volatile ringbuffer_t usart1_inBuff = {{0}};
	#endif

	#ifndef NO_USART1_BUFFERED_OUTPUT
		#include "ringbuffer.h"
		static volatile ringbuffer_t usart1_outBuff = {{0}};
	#endif
#endif

/**
 * Convert a given baud-rate to UBRR prescalar.
 * @param  baudrate Target baud-rate
 * @param  mode     UART operation mode
 * @return          UBRR prescalar
 */
static inline uint16_t uart_baud2ubrr(const uint32_t baudrate, enum uart_operationModes_t mode){
	uint16_t ubrr_val;
	switch (mode){
		case UART_MODE_ASYNC_NORMAL:
			ubrr_val = ((F_CPU / (baudrate * 16UL))) - 1;
			break;
		case UART_MODE_ASYNC_DOUBLE:
			ubrr_val = ((F_CPU / (baudrate * 8UL))) - 1;
			break;
		case UART_MODE_SYNC_MASTER:
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
 * Functions for sending and receiving on USAR0
 * @{
 */
#ifndef NO_USART0_SUPPORT

/**
 * Set up the USART with defaults values.
 * Enables RX and TX 1 stop bit with 8bit char size in async normal mode. if a
 * zero value baudrate is give it will default to 115200.
 * @param baudrate the desired baudrate
 */
void usart0_init(uint32_t baudrate) {
	if (baudrate == 0) {
		baudrate = 115200;
	}

#ifndef NO_USART0_BUFFERED_INPUT
	rb_init((ringbuffer_t*)&usart0_inBuff);
	USART0_ENABLE_RX_INTERRUPT();
#endif

#ifndef NO_USART0_BUFFERED_OUTPUT
	rb_init((ringbuffer_t*)&usart0_outBuff);
#endif

	//Enable TXen og RXen
	USART0_ENABLE_RX();
	USART0_ENABLE_TX();

	USART0_SET_1_STOP_BIT();
	USART0_SET_CHAR_SIZE(UART_CHAR_8BIT);

	// Baud rate
	usart0_setBaudrate(baudrate, UART_MODE_ASYNC_NORMAL);
}

/**
 * Set USART baud-rate and operation mode.
 * @param baudrate baud-rate that the USART will use
 * @param mode     USART operation mode
 */
void usart0_setBaudrate(const uint32_t baudrate,
						enum uart_operationModes_t mode){
	switch (mode) {
		case UART_MODE_ASYNC_NORMAL: USART0_SET_MODE_ASYNC(); break;
		case UART_MODE_ASYNC_DOUBLE: USART0_SET_MODE_ASYNC(); break;
		case UART_MODE_SYNC_MASTER: USART0_SET_MODE_SYNC(); break;
	}

	const uint16_t prescale = uart_baud2ubrr(baudrate, mode);

	UBRR0L = LOW_BYTE(prescale);
	UBRR0H = HIGH_BYTE(prescale);
}

#ifndef NO_USART0_BUFFERED_INPUT
/**
 * Check the input buffer for new data.
 * @return  true if it as data. Else false
 */
bool usart0_hasData(void){
	return !rb_isEmpty(&usart0_inBuff);
}
#endif

/**
 * Get a byte from USART. This call is alway blocking. if input buffer is
 * enabled use usart[N]_hasData() to check if data is available.
 * @return  received byte
 */
uint8_t usart0_getc(void) {
#ifdef NO_USART0_BUFFERED_INPUT
	while(USART0_RX_IS_BUSY());
	return UDR0;
#else
	uint8_t data;
	while(rb_pop((ringbuffer_t*)&usart0_inBuff, &data) != 0);
	return data;
#endif
}

/**
 * Put a byte on USART. unless USART0_NON_UNIX_LIKE_LINE_ENDINGS is defined this
 * will put a '\r' before every '\n' to mimic Unix like line endings.
 * @param  c Byte to transmit
 * @return   positive if success
 */
int usart0_putc(const uint8_t c) {
#ifndef USART0_NON_UNIX_LIKE_LINE_ENDINGS
	if(c == '\n'){
		usart0_putc('\r');
	}
#endif

#ifdef NO_USART0_BUFFERED_OUTPUT
	while (USART0_TX_IS_BUSY());
	UDR0 = c;
#else
	// Wait for free space in buffer
	while (rb_isFull(&usart0_outBuff));
	rb_push((ringbuffer_t*)&usart0_outBuff, c);

	USART0_ENABLE_UDRE_INTERRUPT();
#endif

	return c;
}

/**
 * Writes a null terminated c-string to the USART.
 * @param  str String that is written
 * @return     Number of bytes written
 */
int usart0_puts(const char *str) {
	if (str == NULL) return -1;
	int i = 0;

	while(str[i] != '\0'){
		usart0_putc(str[i++]);
	}

	return i;
}

/**
 * Writes an array of bytes with the length n.
 * @param  n      Number of bytes to write
 * @param  array  Array of bytes to be written
 * @return        Number of bytes written
 */
int usart0_putn(size_t n, const uint8_t *array) {
	if (array == NULL) return -1;

	int i;
	for (i = 0; i < n; ++i){
		usart0_putc(array[i]);
	}

	return i;
}

/**
 * Writes a formatted c string. Works like printf is expected to work except it
 * uses a static buffer of size UART[n]_PRNT_BUFF_SIZE to store the intermediate
 * string in.
 */
int usart0_printf(const char *str, ...){
	if(str == NULL) return -1;

	// Warning this might overflow on long str
	char buffer[USART0_PRNT_BUFF_SIZE] = {'\0'};
	va_list args;
	int rc_vsprintf;
	int rc_tx;

	va_start(args, str);
	if((rc_vsprintf = vsnprintf(buffer, USART0_PRNT_BUFF_SIZE, str, args)) < 0){
		return rc_vsprintf; // vsprintf return a negative value on err
	}
	va_end(args);

	if((rc_tx = usart0_puts(buffer)) != rc_vsprintf ||
			rc_tx > USART0_PRNT_BUFF_SIZE) {
		// We haven't send the same amount as sprintf wrote the the buffer
		return -rc_tx;
	}

	return rc_tx;
}

#ifndef NO_USART0_BUFFERED_INPUT
ISR(USART0_RX_vect){
	uint8_t data = UDR0;

	rb_push((ringbuffer_t*)&usart0_inBuff, data);
}
#endif

#ifndef NO_USART0_BUFFERED_OUTPUT
ISR(USART0_UDRE_vect){
	uint8_t data;
	if(rb_pop((ringbuffer_t*)&usart0_outBuff, &data) == 0) {
		UDR0 = data;
	} else {
		// output buffer is empty so disable UDRE interrupt flag
		USART0_DISABLE_UDRE_INTERRUPT();
	}
}
#endif

#endif /* NO_USART0_SUPPORT */
/** @} */




/**
 * @name USART1
 * Functions for sending and receiving on USAR1
 * @{
 */
#ifndef NO_USART1_SUPPORT

/**
 * Set up the USART with defaults values.
 * Enables RX and TX 1 stop bit with 8bit char size in async normal mode. if a
 * zero value baudrate is give it will default to 115200.
 * @param baudrate the desired baudrate
 */
void usart1_init(uint32_t baudrate) {
	if (baudrate == 0) {
		baudrate = 115200;
	}

#ifndef NO_USART1_BUFFERED_INPUT
	rb_init((ringbuffer_t*)&usart1_inBuff);
	USART1_ENABLE_RX_INTERRUPT();
#endif

#ifndef NO_USART1_BUFFERED_OUTPUT
	rb_init((ringbuffer_t*)&usart1_outBuff);
#endif

	//Enable TXen og RXen
	USART1_ENABLE_RX();
	USART1_ENABLE_TX();

	USART1_SET_1_STOP_BIT();
	USART1_SET_CHAR_SIZE(UART_CHAR_8BIT);

	// Baud rate
	usart1_setBaudrate(baudrate, UART_MODE_ASYNC_NORMAL);
}

/**
 * Set USART baud-rate and operation mode.
 * @param baudrate baud-rate that the USART will use
 * @param mode     USART operation mode
 */
void usart1_setBaudrate(const uint32_t baudrate,
						enum uart_operationModes_t mode){
	switch (mode) {
		case UART_MODE_ASYNC_NORMAL: USART1_SET_MODE_ASYNC(); break;
		case UART_MODE_ASYNC_DOUBLE: USART1_SET_MODE_ASYNC(); break;
		case UART_MODE_SYNC_MASTER: USART1_SET_MODE_SYNC(); break;
	}

	const uint16_t prescale = uart_baud2ubrr(baudrate, mode);

	UBRR1L = LOW_BYTE(prescale);
	UBRR1H = HIGH_BYTE(prescale);
}

#ifndef NO_USART1_BUFFERED_INPUT
/**
 * Check the input buffer for new data.
 * @return  true if it as data. Else false
 */
bool usart1_hasData(void){
	return !rb_isEmpty(&usart1_inBuff);
}
#endif

/**
 * Get a byte from USART. This call is alway blocking. if input buffer is
 * enabled use usart[N]_hasData() to check if data is available.
 * @return  received byte
 */
uint8_t usart1_getc(void) {
#ifdef NO_USART1_BUFFERED_INPUT
	while(USART1_RX_IS_BUSY());
	return UDR1;
#else
	uint8_t data;
	while(rb_pop((ringbuffer_t*)&usart1_inBuff, &data) != 0);
	return data;
#endif
}

/**
 * Put a byte on USART. unless USART1_NON_UNIX_LIKE_LINE_ENDINGS is defined this
 * will put a '\r' before every '\n' to mimic Unix like line endings.
 * @param  c Byte to transmit
 * @return   positive if success
 */
int usart1_putc(const uint8_t c) {
#ifndef USART1_NON_UNIX_LIKE_LINE_ENDINGS
	if(c == '\n'){
		usart1_putc('\r');
	}
#endif

#ifdef NO_USART1_BUFFERED_OUTPUT
	while (USART1_TX_IS_BUSY());
	UDR1 = c;
#else
	// Wait for free space in buffer
	while (rb_isFull(&usart1_outBuff));
	rb_push((ringbuffer_t*)&usart1_outBuff, c);

	USART1_ENABLE_UDRE_INTERRUPT();
#endif

	return c;
}

/**
 * Writes a null terminated c-string to the USART.
 * @param  str String that is written
 * @return     Number of bytes written
 */
int usart1_puts(const char *str) {
	if (str == NULL) return -1;
	int i = 0;

	while(str[i] != '\0'){
		usart1_putc(str[i++]);
	}

	return i;
}

/**
 * Writes an array of bytes with the length n.
 * @param  n      Number of bytes to write
 * @param  array  Array of bytes to be written
 * @return        Number of bytes written
 */
int usart1_putn(size_t n, const uint8_t *array) {
	if (array == NULL) return -1;

	int i;
	for (i = 0; i < n; ++i){
		usart1_putc(array[i]);
	}

	return i;
}

/**
 * Writes a formatted c string. Works like printf is expected to work except it
 * uses a static buffer of size UART[n]_PRNT_BUFF_SIZE to store the intermediate
 * string in.
 */
int usart1_printf(const char *str, ...){
	if(str == NULL) return -1;

	// Warning this might overflow on long str
	char buffer[USART1_PRNT_BUFF_SIZE] = {'\0'};
	va_list args;
	int rc_vsprintf;
	int rc_tx;

	va_start(args, str);
	if((rc_vsprintf = vsnprintf(buffer, USART1_PRNT_BUFF_SIZE, str, args)) < 0){
		return rc_vsprintf; // vsprintf return a negative value on err
	}
	va_end(args);

	if((rc_tx = usart1_puts(buffer)) != rc_vsprintf ||
			rc_tx > USART1_PRNT_BUFF_SIZE) {
		// We haven't send the same amount as sprintf wrote the the buffer
		return -rc_tx;
	}

	return rc_tx;
}

#ifndef NO_USART1_BUFFERED_INPUT
ISR(USART1_RX_vect){
	uint8_t data = UDR1;

	rb_push((ringbuffer_t*)&usart1_inBuff, data);
}
#endif

#ifndef NO_USART1_BUFFERED_OUTPUT
ISR(USART1_UDRE_vect){
	uint8_t data;
	if(rb_pop((ringbuffer_t*)&usart1_outBuff, &data) == 0) {
		UDR1 = data;
	} else {
		// output buffer is empty so disable UDRE interrupt flag
		USART1_DISABLE_UDRE_INTERRUPT();
	}
}
#endif

#endif /* NO_USART1_SUPPORT */
/** @} */

