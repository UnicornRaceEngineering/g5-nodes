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

#include <avr/io.h>
#include <io.h>

#include "shiftlight.h"

void shiftlight_init(void) {
	SET_PIN_MODE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B, OUTPUT);
	SET_PIN_MODE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_R, OUTPUT);
	shiftlight_off();
}

void shiftlight_off(void) {
	IO_SET_LOW(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B);
	IO_SET_LOW(SHIFT_LIGHT_PORT, SHIFT_LIGHT_R);
}

void shiftlight_on(void) {
	IO_SET_HIGH(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B);
	IO_SET_HIGH(SHIFT_LIGHT_PORT, SHIFT_LIGHT_R);
}

void shiftlight_toggle(void) {
	DIGITAL_TOGGLE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_R);
	DIGITAL_TOGGLE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B);
}
