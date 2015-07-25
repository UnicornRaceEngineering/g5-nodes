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
#include <can_transport.h>                // for can_message, can_broadcast, etc
#include <io.h>                           // for IO_SET_HIGH, IO_SET_LOW, etc
#include <stdbool.h>                      // for bool, false, true
#include <stddef.h>                       // for size_t
#include <stdint.h>                       // for uint8_t, uint16_t, etc
#include <stdio.h>                        // for printf
#include <usart.h>                        // for usart1_init
#include <util/delay.h>
#include <utils.h>                        // for ARR_LEN

#include "../SteeringNode/paddleshift.h"  // for paddle_status::PADDLE_DOWN, etc
#include "adc.h"                          // for adc_init, adc_vref_t::AVCC
#include "dewalt.h"                       // for dewalt_set_pwm_dutycycle, etc
#include "neutralsensor.h"                // for GEAR_IS_NEUTRAL, NEUT_PIN, etc
#include "system_messages.h"              // for message_id::CURRENT_GEAR, etc
#include "vnh2sp30.h"                     // for vnh2sp30_is_faulty, etc

#define IGN_PORT	(PORTE)
#define IGN_PIN		(PIN4)
#define IGNITION_CUT()			( IO_SET_HIGH(IGN_PORT, IGN_PIN) )
#define IGNITION_UNCUT()		( IO_SET_LOW(IGN_PORT, IGN_PIN) )

#define TIMEOUT 		700 	// Timeout for gearshift given in timer ticks
#define TIMEOUT_SLOW	2000	// Like TIMEOUT just for when going to neutral

enum {
	GEAR_DOWN = -1,
	GEAR_UP = 1,
	GEAR_NEUTRAL_UP = 2,
	GEAR_NEUTRAL_DOWN = 3,
};

void start_gearshift(uint8_t gear_request);
void stop_gearshift(void);
void gearshift_procedure(uint8_t gear_request);
void gear_estimate(uint8_t gear_request);
void go_slow(uint8_t gear_request);

static uint8_t buf_in[64];
static uint8_t buf_out[64];

uint8_t current_gear = 1;
uint8_t neutral_flag = 0;
uint8_t neutral_button = 0;


static int shift_gear(int gear_dir) {
	//!< TODO: check the right way to slow down
	switch (gear_dir) {
		case GEAR_DOWN:
			dewalt_set_direction_B();
			dewalt_set_pwm_dutycycle(100);
			break;
		case GEAR_UP:
			dewalt_set_direction_A();
			dewalt_set_pwm_dutycycle(100);
			break;
		case GEAR_NEUTRAL_DOWN:
			dewalt_set_direction_B();
			dewalt_set_pwm_dutycycle(50);
			break;
		case GEAR_NEUTRAL_UP:
			dewalt_set_direction_A();
			dewalt_set_pwm_dutycycle(50);
			break;
	}
	return 0;
}

void timer_init(void) {
	// setup timer 0 which is used for gearshift
	// 1/((F_CPU/Prescaler)/n_timersteps)
	// 1/((11059200/8)/256) = approx 0.185185 ms or about 5400 Hz
	OCR0A = 0; // Set start value
	TIMSK0 |= 1<<OCIE0A; // Enable timer compare match interrupt
	TCCR0A |= 0<<CS02 | 1<<CS01 | 0<<CS00; // Set prescaler 8
}

volatile uint16_t tick = 0;

ISR (TIMER0_COMP_vect) {
	++tick;
}

static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	timer_init();
	adc_init(1, AVCC, 4);
	init_can_node(GEAR_NODE);

	dewalt_init();
	dewalt_kill();

	SET_PIN_MODE(NEUT_PORT, NEUT_PIN, INPUT_PULLUP);
	SET_PIN_MODE(IGN_PORT, IGN_PIN, OUTPUT);
	IGNITION_UNCUT();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();
	//printf("diagA: %d diagB: %d\n", vnh2sp30_read_DIAGA(), vnh2sp30_read_DIAGB());

	while (1) {

		bool has_changed_gear = false; // We dont want two gear in a row
		while (can_has_data()) {
			struct can_message message = read_inbox();

			if (message.id == PADDLE_STATUS && !has_changed_gear) {
				gearshift_procedure(message.data[0]);
				has_changed_gear = true;
			}

			if (message.id == NEUTRAL_ENABLED) {
				neutral_button = message.data[0];
			}
		}

		// As long as the gear is in neutral state the GearNode will broadcast
		// its current gear(neutral).
		// It should be enough to do this once only, but it doesn't allways seem
		// to work. That's a problem with the CAN and this is a temporary workaround.
		if (GEAR_IS_NEUTRAL()) {
			can_broadcast(CURRENT_GEAR, &(uint8_t){0});
			_delay_ms(100);
		}

		if (vnh2sp30_is_faulty()) {
			printf("faulty H-Bridge!!");
			vnh2sp30_reset();
			_delay_ms(1000); // 1 second to cool off
		}
	}

	return 0;
}

void gearshift_procedure(uint8_t gear_request) {
	if (gear_request == (PADDLE_UP | PADDLE_DOWN)) {
		return; // We cant do both so ignore it
	}

	if (neutral_button) {
		go_slow(gear_request);
		return;
	}

	start_gearshift(gear_request);
	stop_gearshift();

	tick = 0;

	// It waits 370 ms extra to check if it passes by neutral gear state
	// This does NOT determain if current gear is neutral, but simply if
	// current gear has been passed.
	// This is used to synchronize to 1st and 2nd gear.
	// the number 2000 ticks is found by test and can be changed without
	// affecting the gear shift it itself.
	// That is: Only affects gear estimate.
	while (tick < 2000) {
		if (GEAR_IS_NEUTRAL()) {
			neutral_flag = 1;
		}
	}

	gear_estimate(gear_request);
}

void start_gearshift(uint8_t gear_request) {
	IGNITION_CUT();
	if (gear_request & PADDLE_UP){
		printf("gear up %u\n", current_gear);
		shift_gear(GEAR_UP);
	} else if(gear_request & PADDLE_DOWN){
		printf("gear down %u\n", current_gear);
		shift_gear(GEAR_DOWN);
	}


	// Reset
	neutral_flag = 0;
	tick = 0;

	// Moving average buffer
	uint16_t buf[16] = {0}; // pow2 so division can be done with bitshifts
	size_t buf_i = 0;

	bool high_resistance = false;
	while(tick <= TIMEOUT) {
		// add new data to array
		const uint16_t current_sense = vnh2sp30_read_CS();
		buf[buf_i++] = current_sense;
		if (buf_i == ARR_LEN(buf)) buf_i = 0;

		// Low pass moving avarage filter data
		uint32_t accumulator = 0;
		for (size_t i = 0; i < ARR_LEN(buf); i++) {
			accumulator += buf[i];
		}
		const uint16_t filtered_cs = accumulator / ARR_LEN(buf);

		printf("%d;%d\n", tick, filtered_cs);

		if (GEAR_IS_NEUTRAL()) {
			neutral_flag = 1;
		}

		if (tick > TIMEOUT/2) {
			if (filtered_cs > 75) {
				high_resistance = true;
				break;
			}
		}

	}

	if (high_resistance) {
		const uint16_t limit_time = tick;
		printf("reached limit - with time %d\n", limit_time);
	} else {
		printf("timeout\n");
	}
}

void stop_gearshift() {
	IGNITION_UNCUT();
	dewalt_kill();
}

void gear_estimate(uint8_t gear_request) {
	if (gear_request & PADDLE_UP) {
		if (current_gear < 6) ++current_gear;
		if (!current_gear)      current_gear = 2;
		if (neutral_flag)       current_gear = 2;

	} else if(gear_request & PADDLE_DOWN) {
		if (current_gear > 1) --current_gear;
		if (!current_gear)      current_gear = 1;
		if (neutral_flag)       current_gear = 1;
	}

	can_broadcast(CURRENT_GEAR, &(uint8_t){current_gear});
}

void go_slow(uint8_t gear_request) {
	IGNITION_CUT();
	if (gear_request & PADDLE_UP) {
		shift_gear(GEAR_NEUTRAL_UP);
	} else if(gear_request & PADDLE_DOWN) {
		shift_gear(GEAR_NEUTRAL_DOWN);
	}

	tick = 0;

	// The gear shifting motor is allowed to run for 370 ms on 50% PWM.
	// This number has been found by trial and error and can be changed.
	while(tick <= TIMEOUT_SLOW) {
		if (GEAR_IS_NEUTRAL()) {
			break;
		}
	}

	stop_gearshift();
}
