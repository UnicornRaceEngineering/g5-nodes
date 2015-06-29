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

#include <74ls138d_demultiplexer.h>
#include <utils.h>
#include <avr/io.h>
#include <io.h>
#include "statuslight.h"

void statuslight_init(void) {
	// init status LEDS
	{
		SET_PIN_MODE(STATUS_LED_PORT, STATUS_LED_R, OUTPUT);
		SET_PIN_MODE(STATUS_LED_PORT, STATUS_LED_G, OUTPUT);
		SET_PIN_MODE(STATUS_LED_PORT, STATUS_LED_B, OUTPUT);
	}
	dmux_init();
}

void set_rgb_color(uint8_t led, enum color_masks color) {
	// if (led > 8) return;
	dmux_set_y_low(	(const enum dmux_y_values []) {
		DMUX_Y0, DMUX_Y1, DMUX_Y2, DMUX_Y3, DMUX_Y4, DMUX_Y5, DMUX_Y6, DMUX_Y7,
	}[led]);
	SET_REGISTER_BITS(STATUS_LED_PORT, color, STATUS_LED_MASK);
	// BITMASK_CLEAR(STATUS_LED_PORT, STATUS_LED_MASK);
	// BITMASK_SET(STATUS_LED_PORT, color);
}
