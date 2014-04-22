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

#include <avr/io.h>
#include <stdint.h>
#include "timer.h"
#include "bitwise.h"

/**
 * Sets the prescalar for timer0
 * @param  prescalar see timer0_prescalar_t for valid input
 */
void timer0_set_prescalar(uint8_t prescalar) {
	const uint8_t mask = CS02|CS01|CS00;
	prescalar = BITMASK_CHECK(prescalar, mask); // Filter valid input

	BITMASK_CLEAR(TCCR0A, mask); // Clear the register before writing new values
	BITMASK_SET(TCCR0A, prescalar);
}

/**
 * Sets the prescalar for timer1
 * @param prescalar see timer1_prescalar_t for valid input
 */
void timer1_set_prescalar(uint8_t prescalar) {
	const uint8_t mask = CS12|CS11|CS10;
	prescalar = BITMASK_CHECK(prescalar, mask); // Filter valid input

	BITMASK_CLEAR(TCCR1B, mask); // Clear the register before writing new values
	BITMASK_SET(TCCR1B, prescalar);
}

/**
 * Sets the prescalar for timer2
 * @param prescalar See timer2_prescalar_t for valid input
 */
void timer2_set_prescalar(uint8_t prescalar) {
	const uint8_t mask = CS22|CS21|CS20;
	prescalar = BITMASK_CHECK(prescalar, mask); // Filter valid input

	BITMASK_CLEAR(TCCR2A, mask); // Clear the register before writing new values
	BITMASK_SET(TCCR2A, prescalar);
}

/**
 * Sets the prescalar for timer3
 * @param prescalar see timer3_prescalar_t for valid input
 */
void timer3_set_prescalar(uint8_t prescalar) {
	const uint8_t mask = CS32|CS31|CS30;
	prescalar = BITMASK_CHECK(prescalar, mask); // Filter valid input

	BITMASK_CLEAR(TCCR3B, mask); // Clear the register before writing new values
	BITMASK_SET(TCCR3B, prescalar);
}
