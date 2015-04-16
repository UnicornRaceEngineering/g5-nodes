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
* @file pwm.h
* @brief Provides some abstraction for pwm operation
*/

#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <avr/io.h>
#include "utils.h"

#ifndef PWM_PRESCALAR
#	define PWM_PRESCALAR	(64)
#endif

#ifndef PWM_TOP
	// (F_CPU / (Prescalar * ( 1+2047)) =
	// (11059200 Hz / (64 * (1+2047))) 	=
	// 84.375 Hz = 11.852 ms period
	// Resolution = log(2047+1)/log(2) = 11 bits
#	define PWM_TOP	(2047)
#endif

#define TOP_TO_HZ(top)	(F_CPU / (PWM_PRESCALAR * (1+(top))))
#define HZ_TO_MS(hz)	((1/(hz)) * 1000)

#define MS_TO_TOP(ms)	((uint16_t) \
	((((F_CPU/1000.0)/(double)PWM_PRESCALAR)*((double)(ms)))) - 1)

#define DUTY_TO_TOP(duty)	(duty * (PWM_TOP / 100))

void pwm_PE5_init(void);

static inline void pwm_PE5_set_top(uint16_t top) {
	OCR3CH = HIGH_BYTE(top);
	OCR3CL = LOW_BYTE(top);
}

static inline void pwm_PE5_set_dutycycle(uint8_t dutycycle) {
	const uint16_t top = DUTY_TO_TOP(dutycycle);
	pwm_PE5_set_top(top);
}

void pwm_PB5_init(void);

static inline void pwm_PB5_set_top(uint16_t top) {
	OCR1AH = HIGH_BYTE(top);
	OCR1AL = LOW_BYTE(top);
}

static inline void pwm_PB5_set_dutycycle(uint8_t dutycycle) {
	const uint16_t top = DUTY_TO_TOP(dutycycle);
	pwm_PB5_set_top(top);
}

#endif /* PWM_H */
