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
* @file pwm.c
* @brief Provides some abstraction for pwm operation
*/

#include "pwm.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "bitwise.h"
#include "io.h"


void pwm_PE5_init(void) {
	// OC3C, Output Compare Match C output (counter 3 output compare)

	// Set prescalar to 64
#if PWM_PRESCALAR == 64
	timer3_set_prescalar(TIMER3_PRESCALAR_64);
#else
#	error undefined PWM_PRESCALAR
#endif

	// Count to the specified value
	const uint16_t count_to = PWM_TOP;
	ICR3H = HIGH_BYTE(count_to);
	ICR3L = LOW_BYTE(count_to);

	// Set Wave Generation Mode to Fast PWM counting to ICR
	timer3_set_waveform_generation_mode(TIMER3_WGM_FAST_PWM_ICR);

	// Clear on Compare Match
	SET_REGISTER_BITS(TCCR3A, (1<<COM3C1|1<<0    ), (1<<COM3C1|1<<COM3C0));
	SET_PIN_MODE(PORTE, PIN5, OUTPUT);
}
