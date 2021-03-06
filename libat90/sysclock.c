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
* @file can.c
* @brief
*   Used for setting up a universal timer
*	and count elapsed milliseconds.
*/


#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>  // for uint32_t
#include <util/atomic.h>


static volatile uint32_t tick;


/**
 * Setup 32-bit sysclock timer.
 */
void sysclock_init(void) {
	tick = 0;

	// control regiters set to Mode 12 (CTC) and no prescaling.
	TCCR3A = 0;
	TCCR3B = (1 << WGM32) + (1 << CS30);
	TCCR3C = 0;

	// Output Compare Register A to 11059
	// equal to 1ms
	OCR3A = 11059;

	// Set counter value to 0
	TCNT3L = 0;
	TCNT3H = 0;

	// Set to interrupt on output compare match A.
	TIMSK3 = 1 << OCIE3A;
}


/**
 * Atommically reads the milliseconds counted since clock init.
 * @preturn numbers of counted milliseconds since clock init.
 */
uint32_t get_tick(void) {
	uint32_t read_tick;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		read_tick = tick;
	}
	return read_tick;
}

ISR(TIMER3_COMPA_vect) {
	++tick;
}
