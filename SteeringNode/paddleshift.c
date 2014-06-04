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
 * @files paddleshift.c
 * Implements a basic interface to paddleshift controls connected to board
 */

#include <avr/interrupt.h>
#include <stdbool.h>
#include <bitwise.h>
#include <io.h>

#include "paddleshift.h"

volatile bool paddle_up_pressed = false;
volatile bool paddle_down_pressed = false;

void paddle_init(void) {
	// init paddle up
	SET_PIN_MODE(PADDLE_UP_PORT, PADDLE_UP_PIN, INPUT);
	// Generate async interrupt on rising edge
	SET_REGISTER_BITS(EICRA,
		(1<<PADDLE_UP_ISC1|1<<PADDLE_UP_ISC0),
		(1<<PADDLE_UP_ISC1|1<<PADDLE_UP_ISC0));
	BIT_SET(EIMSK, PADDLE_UP_INT); // Interrupt enable Paddle up

	// init paddle down
	SET_PIN_MODE(PADDLE_DOWN_PORT, PADDLE_DOWN_PIN, INPUT);
	// Generate async interrupt on rising edge
	SET_REGISTER_BITS(EICRA,
		(1<<PADDLE_DOWN_ISC1|1<<PADDLE_DOWN_ISC0),
		(1<<PADDLE_DOWN_ISC1|1<<PADDLE_DOWN_ISC0));
	BIT_SET(EIMSK, PADDLE_DOWN_INT); // Interrupt enable Paddle down
}

bool paddle_up_status(void) {
	const bool status = paddle_up_pressed;
	paddle_up_pressed = false; // Reset the pressed status here
	return status;
}

bool paddle_down_status(void) {
	const bool status = paddle_down_pressed;
	paddle_down_pressed = false; // Reset the pressed status here
	return status;
}

ISR(PADDLE_UP_ISR_VECT) {
	paddle_up_pressed = (bool)DIGITAL_READ(PADDLE_UP_PORT, PADDLE_UP_PIN);
}

ISR(PADDLE_DOWN_ISR_VECT) {
	paddle_down_pressed = (bool)DIGITAL_READ(PADDLE_DOWN_PORT, PADDLE_DOWN_PIN);
}
