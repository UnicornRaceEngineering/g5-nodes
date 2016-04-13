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

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <io.h>               // for io_pinmode_t::INPUT, SET_PIN_MODE
#include <stdint.h>           // for uint8_t, uint32_t, uint16_t
#include <stdio.h>            // for printf
#include <sysclock.h>         // for get_tick, sysclock_init
#include <usart.h>            // for usart1_init
#include <util/delay.h>
#include <utils.h>            // for ARR_LEN, BIT_SET
#include <stdbool.h>
#include <can.h>
#include "system_messages.h"  // for message_id, etc


#define WHEEL_TICK_PORT		PORTE
#define WHEEL_TICK_PIN		PIN5

#define WHEEL_TICK_ISC1		ISC51
#define WHEEL_TICK_ISC0		ISC50

#define WHEEL_TICK_INT		INT5
#define WHEEL_TICK_ISR_vect	INT5_vect

#define HOLES_PR_WHEEL	56
#define WHEEL_CIRC		1.62 // Circumference in meters

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static volatile uint16_t wheel_tick = 0;

void wheel_tick_init(void) {
	SET_PIN_MODE(WHEEL_TICK_PORT, WHEEL_TICK_PIN, INPUT);
	// Generate synchronous interrupt on rising edge
	EICRB = ((1 << WHEEL_TICK_ISC1) | (1 << WHEEL_TICK_ISC0));
	BIT_SET(EIMSK, WHEEL_TICK_INT); // Interrupt enable
}

ISR(WHEEL_TICK_ISR_vect) {
	++wheel_tick;
}

static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	sysclock_init();
	wheel_tick_init();
	can_init();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

void wheel_speed(enum message_id wheel_id, const uint32_t current_time) {
	static uint32_t last_time = 0;

	const uint32_t duration = (current_time - last_time);
	const float holes_pr_ms = (float)wheel_tick / duration;
	const float rpm = (1000.0 * 60.0) / (HOLES_PR_WHEEL / holes_pr_ms); // Convert ms to min

	const float v_mps = WHEEL_CIRC * rpm / 60.0; // m/s
	const float v_kmph  = v_mps * ((60.0*60.0)/1000.0); // km/h

	printf("ticks: %4u, holes/ms: %4.3f, rpm: %4.3f, v (km/h): %4.3f, v (m/s) %4.3f\n", wheel_tick, (double)holes_pr_ms, (double)rpm, (double)v_kmph, (double)v_mps);

	can_broadcast(wheel_id, (void*)&v_kmph);

	// Reset
	last_time  = current_time;
	wheel_tick = 0;
}

int main(void) {
	init();

	while (1) {
		static uint32_t timers[1] = {0};
		uint32_t tick = get_tick();

		if (tick > timers[0]) {
			wheel_speed(FRONT_RIGHT_WHEEL_SPEED, tick);
			timers[0] += 50;
		}
	}

	return 0;
}
