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
 * @file neutral.c
 * Handles input from neutral enable button
 */

#include <stdbool.h>
#include <avr/io.h>
#include <io.h>
#include <utils.h>

#include "neutral.h"


void neutral_btn_init(void) {
	SET_PIN_MODE(NEUTRAL_BTN_PORT, NEUTRAL_BTN_PIN, INPUT);
}

bool neutral_btn_pressed(void) {
	/**
	 * @TODO This should really be an interrupt telling if we are changing state
	 * otherwise we are gonna fill the CAN bus with neutral enable messages
	 */
	return (bool)DIGITAL_READ(NEUTRAL_BTN_PORT, NEUTRAL_BTN_PIN);
}
