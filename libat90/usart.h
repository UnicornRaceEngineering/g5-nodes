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
 * @file usart.h
 *
 * @brief
 * Provides usart input / output functions.
 *
 * By default functions for using both usart0 and usart1 is available with
 * buffered input and output. Each usart can be disabled by defining
 * NO_USART[n]_SUPPORT where n is either 0 or one 1. Futhermore either buffered
 * input or buffered output can be disabled for each usart by defining
 * NO_USART[n]_BUFFERED_INPUT or NO_USART[n]_BUFFERED_OUTPUT. These must be
 * defined at compile time
 */


#ifndef USART_H
#define USART_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> // size_t
#include <stdio.h>
#include <avr/io.h>
#include "utils.h"

enum usart_operationModes_t {
	USART_MODE_ASYNC_NORMAL,
	USART_MODE_ASYNC_DOUBLE,
	USART_MODE_SYNC_MASTER
};

/**
 * @name USART0
 * Functions for sending and receiving on USART0
 * @{
 */
#ifndef NO_USART0_SUPPORT

	#define USART0_PRNT_BUFF_SIZE (256) //!< The size of the uart print buffer

	#define USART0_ENABLE_RX()	BIT_SET(UCSR0B, RXEN)
	#define USART0_ENABLE_TX()	BIT_SET(UCSR0B, TXEN)

	#define USART0_ENABLE_RX_INTERRUPT()	BIT_SET(UCSR0B, RXCIE0)
	#define USART0_ENABLE_TX_INTERRUPT()	BIT_SET(UCSR0B, TXCIE0)

	#define USART0_RAISE_UDRE_INTERRUPT_FLAG()	BIT_SET(UCSR0A, UDRE0)
	#define USART0_LOWER_UDRE_INTERRUPT_FLAG()	BIT_CLEAR(UCSR0A, UDRE0)

	#define USART0_ENABLE_UDRE_INTERRUPT() 	BIT_SET(UCSR0B, UDRIE0)
	#define USART0_DISABLE_UDRE_INTERRUPT() BIT_CLEAR(UCSR0B, UDRIE0)

	#define USART0_SET_MODE_ASYNC()	BIT_CLEAR(UCSR0C, UMSEL0)
	#define USART0_SET_MODE_SYNC()	BIT_SET(UCSR0C, UMSEL0)

	#define USART0_SET_1_STOP_BIT()	BIT_CLEAR(UCSR0C, USBS0)
	#define USART0_SET_2_STOP_BIT()	BIT_SET(UCSR0C, USBS0)

	#define USART0_RX_IS_BUSY()	(!(BIT_CHECK(UCSR0A, RXC0)))
	#define USART0_TX_IS_BUSY()	(!(BIT_CHECK(UCSR0A, UDRE0)))

	#define USART0_SET_CHAR_SIZE(size) do { \
		BITMASK_CLEAR(UCSR0C, (0x07 << UCSZ0)); \
		UCSR0C |= (size << UCSZ0); \
	} while (0)


	void usart0_init(uint32_t baudrate);
	void usart0_setBaudrate(const uint32_t baudrate,
							enum usart_operationModes_t mode);

	bool usart0_has_data(void);
	char usart0_getc(FILE *stream);
	void usart0_putbyte(uint8_t c, FILE *stream);
	void usart0_putc(char c, FILE *stream);

	extern FILE usart0_io;
	extern FILE usart0_byte_output;
#endif
/** @} */


/**
 * @name USART1
 * Functions for sending and receiving on USART1
 * @{
 */
#ifndef NO_USART1_SUPPORT

	#define USART1_PRNT_BUFF_SIZE (256) //!< The size of the uart print buffer

	#define USART1_ENABLE_RX()	BIT_SET(UCSR1B, RXEN1)
	#define USART1_ENABLE_TX()	BIT_SET(UCSR1B, TXEN1)

	#define USART1_ENABLE_RX_INTERRUPT()	BIT_SET(UCSR1B, RXCIE1)
	#define USART1_ENABLE_TX_INTERRUPT()	BIT_SET(UCSR1B, TXCIE1)

	#define USART1_RAISE_UDRE_INTERRUPT_FLAG()	BIT_SET(UCSR1A, UDRE1)
	#define USART1_LOWER_UDRE_INTERRUPT_FLAG()	BIT_CLEAR(UCSR1A, UDRE1)

	#define USART1_ENABLE_UDRE_INTERRUPT() 	BIT_SET(UCSR1B, UDRIE1)
	#define USART1_DISABLE_UDRE_INTERRUPT() BIT_CLEAR(UCSR1B, UDRIE1)

	#define USART1_SET_MODE_ASYNC()	BIT_CLEAR(UCSR1C, UMSEL1)
	#define USART1_SET_MODE_SYNC()	BIT_SET(UCSR1C, UMSEL1)

	#define USART1_SET_1_STOP_BIT()	BIT_CLEAR(UCSR1C, USBS1)
	#define USART1_SET_2_STOP_BIT()	BIT_SET(UCSR1C, USBS1)

	#define USART1_RX_IS_BUSY()	(!(BIT_CHECK(UCSR1A, RXC1)))
	#define USART1_TX_IS_BUSY()	(!(BIT_CHECK(UCSR1A, UDRE1)))

	#define USART1_SET_CHAR_SIZE(size) do { \
		BITMASK_CLEAR(UCSR1C, (0x07 << UCSZ1)); \
		UCSR1C |= (size << UCSZ1); \
	} while (0)

	void usart1_init(uint32_t baudrate);
	void usart1_setBaudrate(const uint32_t baudrate,
							enum usart_operationModes_t mode);

	bool usart1_has_data(void);
	char usart1_getc(FILE *stream);
	void usart1_putbyte(uint8_t b, FILE *stream);
	void usart1_putc(char c, FILE *stream);

	extern FILE usart1_io;
	extern FILE usart1_byte_output;
#endif
/** @} */

#endif /* USART_H */
