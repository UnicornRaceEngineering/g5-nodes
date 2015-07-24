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
 * @file paddleshift.c
 * Implements a basic interface to paddleshift controls connected to board
 */

#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils.h>
#include <io.h>
#include <util/delay.h>
#include <sysclock.h>

#include "paddleshift.h"

/**
 * @name Pin layout
 * @{
 */
#define PADDLE_UP_PORT          PORTE
#define PADDLE_UP_PIN           PIN7
#define PADDLE_UP_INT           INT7
#define PADDLE_UP_ISR_VECT      INT7_vect
#define PADDLE_UP_ISC1          ISC71
#define PADDLE_UP_ISC0          ISC70

#define PADDLE_DOWN_PORT        PORTE
#define PADDLE_DOWN_PIN         PIN6
#define PADDLE_DOWN_INT         INT6
#define PADDLE_DOWN_ISR_VECT    INT6_vect
#define PADDLE_DOWN_ISC1        ISC61
#define PADDLE_DOWN_ISC0        ISC60
/** @} */

#define DEBOUNCE_TIME	100

static volatile uint8_t state;

static volatile uint32_t last_time = 0;

void paddle_init(void) {

	// init paddle up
	{
		SET_PIN_MODE(PADDLE_UP_PORT, PADDLE_UP_PIN, INPUT);
		// Generate synchronous interrupt on rising edge
		EICRB |= ((1 << PADDLE_UP_ISC1) | (1 << PADDLE_UP_ISC0));
		BIT_SET(EIMSK, PADDLE_UP_INT); // Interrupt enable Paddle up
	}

	// init paddle down
	{
		SET_PIN_MODE(PADDLE_DOWN_PORT, PADDLE_DOWN_PIN, INPUT);
		// Generate synchronous interrupt on rising edge
		EICRB |= ((1 << PADDLE_DOWN_ISC1) | (1 << PADDLE_DOWN_ISC0));
		BIT_SET(EIMSK, PADDLE_DOWN_INT); // Interrupt enable Paddle down
	}
}

uint8_t paddle_state(void) {
	const uint8_t st = state;
	state = 0;
	return st;
}

ISR(PADDLE_UP_ISR_VECT) {
	if (get_tick() - last_time >= DEBOUNCE_TIME) {
		state |= PADDLE_UP;
		last_time = get_tick();
	}
}

ISR(PADDLE_DOWN_ISR_VECT) {
	if (get_tick() - last_time >= DEBOUNCE_TIME) {
		state |= PADDLE_DOWN;
		last_time = get_tick();
	}
}
