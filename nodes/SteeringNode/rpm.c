/*
The MIT License (MIT)

Copyright (c) 2015 UnicornRaceEngineering

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
 * @files rpm.c
 * Provides functionality to display the rpm (Rounds per minute)
 */

#include <avr/io.h>
#include <io.h>

#include "rpm.h"

#define RPM_MAX_VALUE   13000
#define RPM_MIN_VALUE   3300

#define CALIBRATION 80

void rpm_init(void) {
	// init Timer0 PWM PB4 for the RPM-counter
	{
		RPM_TCCR |= (1 << RPM_WGM1) | (1 << RPM_WGM0); // Fast PWM mode
		RPM_TCCR |= (1 << RPM_CS1) | (1 << RPM_CS1); // F_CPU/64 prescalar
		// Clear RPM_OCR on compare match. Set RPM_OCR at TOP.
		RPM_TCCR |= (1 << RPM_COMA1) | (0 <<  RPM_COMA0);

		set_rpm(0);
		SET_PIN_MODE(RPM_PORT, RPM_PIN, OUTPUT);
	}
}

extern void set_rpm(int16_t rpm) {
	// Because the PWM signal goes through a low pass filter we loose some
	// granularity so we must lower the max value or the RPM meter will max out
	// too soon. The value is determined by increasing the calibration value
	// until it "looked right".
	RPM_OCR = map(rpm, RPM_MIN_VALUE, RPM_MAX_VALUE, 0, 0xFF - CALIBRATION);
}
