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


#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>  // for uint32_t
#include <util/atomic.h>


static volatile uint32_t tick;


void sysclock_init(void) {
	tick = 0;

	// control regiters set to Mode 12 (CTC) and no prescaling.
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) + (1 << CS10);
	TCCR1C = 0;

	// Output Compare Register A to 11059
	// equal to 1ms
	OCR1A = 11059;

	// Set counter value to 0
	TCNT1L = 0;
	TCNT1H = 0;

	// Set to interrupt on output compare match A.
	TIMSK1 = 1 << OCIE1A;
}

uint32_t get_tick(void) {
	uint32_t read_tick;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		read_tick = tick;
	}
	return read_tick;
}

ISR(TIMER1_COMPA_vect) {
	++tick;
}
