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
#define MEASUREMENTS 500

enum {
	GEAR_DOWN = -1,
	GEAR_NEUTRAL = 0,
	GEAR_UP = 1
};

void start_gearshift(uint8_t gear_request);
void stop_gearshift(void);
void gearshift_procedure(uint8_t gear_request);

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static volatile uint16_t mesnumber= 0;
static volatile uint16_t maxCS = 0;
static volatile uint16_t time_counter = 0;
static volatile uint16_t cslist [MEASUREMENTS] = {0};
static volatile uint16_t poslist [MEASUREMENTS] = {0};
static volatile uint16_t limit_time = 0;
static volatile uint8_t on_off = 0;
volatile uint8_t current_gear = 0;

int maxcounter;
int numbermess;
int done;

static int shift_gear(int gear_dir) {
	//!< TODO: check the right way to slow down
	switch (gear_dir) {
		case GEAR_DOWN:
			dewalt_set_direction_B();
			dewalt_set_pwm_dutycycle(100);
			break;
		case GEAR_NEUTRAL:
			//!< TODO: Implement this
			dewalt_set_pwm_dutycycle(10);
			break;
		case GEAR_UP:
			dewalt_set_direction_A();
			dewalt_set_pwm_dutycycle(100);
			break;
	}
	return 0;
}

void timer_init(void) {
	// setup timer 0 which periodically sends heartbeat to the ECU
	// 1/((F_CPU/Prescaler)/n_timersteps)
	// 1/((11059200/8)/256) = approx 0.185185 us or about 5400 Hz
	OCR0A = 0; // Set start value
	TIMSK0 |= 1<<OCIE0A; // Enable timer compare match interrupt
	TCCR0A |= 0<<CS02 | 1<<CS01 | 0<<CS00; // Set prescaler 8
}

ISR (TIMER0_COMP_vect) {
	if (on_off == 1)
		if (mesnumber< MEASUREMENTS) {
			// add new data to array
			cslist[mesnumber] = vnh2sp30_read_CS();
			poslist[mesnumber] = adc_readChannel(1);

			// do ignition cut after 150 measurements. May need adjustments but
			// this number seems to fit with observable data.
			if (mesnumber> 150) {
				IGNITION_CUT();
			}

			// after the 150 first measurements we expect the motor to be in a
			// state where it's moving and it's using fewer ampere. From here we
			// can look at out for when the ampere goes up again to 1000 where
			// we expect that the motor will be back in a standstill.
			if (mesnumber> 150 && cslist[mesnumber] > 1000) {
				maxcounter++;
			}

			// Do we get the same high ampere measurement 5 times we stop the
			// motor and assume that the gearshift has taken place.
			if (maxcounter == 5 && done == 0) {
				IGNITION_UNCUT();
				done = 1;
				numbermess = mesnumber;
				dewalt_kill();
				printf("Killed before timeout\n");
			}

			// counts up the number f measurements done
			++mesnumber;
		}

	// if the maximum number of measurements have been done, stop the motor.
	if (mesnumber== MEASUREMENTS) {
		IGNITION_UNCUT();
		dewalt_kill();
	}
}

static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	timer_init();
	init_neutral_gear_sensor();
	adc_init(1, AVCC, 4);
	init_can_node(GEAR_NODE);

	dewalt_init();
	dewalt_kill();

	SET_PIN_MODE(IGN_PORT, IGN_PIN, OUTPUT);
	IGNITION_UNCUT();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();
	//printf("diagA: %d diagB: %d\n", vnh2sp30_read_DIAGA(), vnh2sp30_read_DIAGB());

	while (1) {
		while(get_queue_length()) {
			struct can_message *message = read_inbox();
			//printf("Got id: %d and data: %d\n", MESSAGE_INFO(message->index).id, message->data[0]);
			if (MESSAGE_INFO(message->index).id == 512)
				gearshift_procedure(message->data[0]);
			can_free(message);
		}
	}

	return 0;
}

void gearshift_procedure(uint8_t gear_request) {
	start_gearshift(gear_request);

	while(mesnumber != MEASUREMENTS);

	stop_gearshift();
}

void start_gearshift(uint8_t gear_request) {
	if (gear_request & PADDLE_UP){
		IGNITION_CUT();
		shift_gear(GEAR_UP);
		gear_shift_dir = UP;
		if (current_gear < 6)
			++current_gear;
	}
	else if(gear_request & PADDLE_DOWN){
		IGNITION_CUT();
		shift_gear(GEAR_DOWN);
		gear_shift_dir = DOWN;
		if (current_gear > 1)
			--current_gear;
	}

	gear_request = 0;

	// Before starting measurements we have a small delay. Not for any
	// testable reason, but it takes a while before the motor starts
	// moving. This delay can probably be increased as it takes a while
	// before we get any usable data.
	_delay_ms(1);

	// Done being set to 1 indicates that the motor is deactivated and
	// it's done changing gear.
	done = 0;

	// counts the number of times that the ampere reaches a set maximum.
	maxcounter = 0;

	// The timer interrupt only does something when on_off = 1;
	on_off = 1;

	// counts number of measurements executed.
	mesnumber= 0;
}

void stop_gearshift() {
	IGNITION_UNCUT();
	on_off = 0;
	dewalt_kill();

	int i;
	for (i = 0; i < MEASUREMENTS; i++) {
		cslist[i] = 0;
	}

	// prints out the position meter data.
	for (i = 0; i < MEASUREMENTS; i++) {
		poslist[i] = 0;
	}

	// prints out diagnosis pins
	//printf("diagA: %d diagB: %d\n", vnh2sp30_read_DIAGA(), vnh2sp30_read_DIAGB());

	mesnumber = 0;

	if (real_gear_pos) {
		current_gear = real_gear_pos;
	}

	real_gear_pos = 0;

	uint8_t msg[1] = {current_gear};
	can_broadcast(CURRENT_GEAR, msg);
}
