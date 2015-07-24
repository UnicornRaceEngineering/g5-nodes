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

#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <usart.h>
#include <io.h>
#include <utils.h>
#include <can_transport.h>
#include "../SteeringNode/paddleshift.h"

#include "dewalt.h"
#include "vnh2sp30.h"
#include "neutralsensor.h"

#define IGN_PORT	(PORTE)
#define IGN_PIN		(PIN4)
#define IGNITION_CUT()			( IO_SET_HIGH(IGN_PORT, IGN_PIN) )
#define IGNITION_UNCUT()		( IO_SET_LOW(IGN_PORT, IGN_PIN) )
#define TIMEOUT 700

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

volatile uint8_t current_gear = 1;
volatile uint8_t neutral_flag = 0;
volatile uint8_t neutral_button = 0;

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
	usart1_init(115200);
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
		if (get_queue_length()) {
			struct can_message *message = read_inbox();
			//printf("Got id: %d and data: %d\n", MESSAGE_INFO(message->index).id, message->data[0]);
			if (message->index == PADDLE_STATUS) {
				gearshift_procedure(message->data[0]);
			}

			if (message->index == NEUTRAL_ENABLED) {
				neutral_button = message->data[0];
			}

			can_free(message);

			while(get_queue_length()) {
				struct can_message *crap = read_inbox();
				can_free(crap);
			}
		}

		// As long as the gear is in neutral state the GearNode will broadcast
		// its current gear(neutral).
		// It should be enough to do this once only, but it doesn't allways seem
		// to work. That's a problem with the CAN and this is a temporary workaround.
		if (GEAR_IS_NEUTRAL()) {
			current_gear = 0;
			uint8_t msg[1] = {current_gear};
			can_broadcast(CURRENT_GEAR, msg);
		}
		
		_delay_ms(100);
	}

	return 0;
}

void gearshift_procedure(uint8_t gear_request) {

	if (gear_request == (PADDLE_UP | PADDLE_DOWN)) {
		return;
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
		printf("gear up\n");
		shift_gear(GEAR_UP);
	} else if(gear_request & PADDLE_DOWN){
		printf("gear down\n");
		shift_gear(GEAR_DOWN);
	}

	// counts the number of times that the ampere reaches a set maximum.
	uint16_t maxcounter = 0;

	neutral_flag = 0;

	tick = 0;

	while(tick <= TIMEOUT) {
		// add new data to array
		//volatile const uint16_t current_sense = vnh2sp30_read_CS();
		uint8_t done = 0;

		// after the 150 first measurements we expect the motor to be in a
		// state where it's moving and it's using fewer ampere. From here we
		// can look at out for when the ampere goes up again to 1000 where
		// we expect that the motor will be back in a standstill.
//		if ((tick > 0) && (current_sense > 1000)) {
//			maxcounter++;
//
//			const uint16_t saved_time = tick;
//			while (current_sense > 1000) {
//				printf("%d;%d\n", tick, current_sense);
//				if ((tick - saved_time) > 30) {
//					done = 1;
//					break;
//				}
//			}
//
//			if (done) {
//				break;
//			}
//		}
		//printf("%d;%d\n", tick, current_sense);

		if (GEAR_IS_NEUTRAL()) {
			neutral_flag = 1;
		}
	}

	if (done) {
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
	if (gear_request & PADDLE_UP){
		if (current_gear < 6)
			++current_gear;

		if (!current_gear)
			current_gear = 2;

		if (neutral_flag)
			current_gear = 2;

	} else if(gear_request & PADDLE_DOWN){
		if (current_gear > 1)
			--current_gear;

		if (!current_gear)
			current_gear = 1;

		if (neutral_flag)
			current_gear = 1;
	}

	uint8_t msg[1] = {current_gear};
	can_broadcast(CURRENT_GEAR, msg);
}

void go_slow(uint8_t gear_request) {
	IGNITION_CUT();
	if (gear_request & PADDLE_UP){
		shift_gear(GEAR_NEUTRAL_UP);
	} else if(gear_request & PADDLE_DOWN){
		shift_gear(GEAR_NEUTRAL_DOWN);
	}

	tick = 0;

	// The gear shifting motor is allowed to run for 370 ms on 50% PWM.
	// This number has been found by trial and error and can be changed.
	while(tick <= 2000) {
		if (GEAR_IS_NEUTRAL()) {
			break;
		}
	}

	stop_gearshift();
}
