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
#include "timer.h"
#include "bitwise.h"

void timer_setPrescaler(const enum timer_prescalar_t p) {
	switch(p) {
		case PRESCALAR_1:
			BIT_SET(TCCR0A, CS00);
			BIT_CLEAR(TCCR0A, CS01);
			BIT_CLEAR(TCCR0A, CS02);
			break;
		case PRESCALAR_8:
			BIT_CLEAR(TCCR0A, CS00);
			BIT_SET(TCCR0A, CS01);
			BIT_CLEAR(TCCR0A, CS02);
			break;
		case PRESCALAR_64:
			BIT_SET(TCCR0A, CS00);
			BIT_SET(TCCR0A, CS01);
			BIT_CLEAR(TCCR0A, CS02);
			break;
		case PRESCALAR_256:
			BIT_CLEAR(TCCR0A, CS00);
			BIT_CLEAR(TCCR0A, CS01);
			BIT_SET(TCCR0A, CS02);
			break;
		case PRESCALAR_1024:
			BIT_SET(TCCR0A, CS00);
			BIT_CLEAR(TCCR0A, CS01);
			BIT_SET(TCCR0A, CS02);
			break;
	}
}

void timer_setMode(const enum timer_16bit_ConReg timer, const unsigned int Waveform_Generation_Mode) {
	const unsigned int mask = 0x03;

	switch (timer) {
		case TIMER1:
			BIT_CLEAR(TCCR1A, mask);
			BIT_CLEAR(TCCR1B, mask);
			BIT_SET(TCCR1A, BITMASK_CHECK(Waveform_Generation_Mode, mask));
			BIT_SET(TCCR1B, BITMASK_CHECK((Waveform_Generation_Mode >> 2), mask));
			break;
		case TIMER3:
			BIT_CLEAR(TCCR3A, mask);
			BIT_CLEAR(TCCR3B, mask);
			BIT_SET(TCCR3A, BITMASK_CHECK(Waveform_Generation_Mode, mask));
			BIT_SET(TCCR3B, BITMASK_CHECK((Waveform_Generation_Mode >> 2), mask));
			break;
	}
}