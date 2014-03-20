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
 * @file main.c
 * Main entry for the GPS node. This continuously receives data from the GPS and
 * broadcasts it via CAN.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include <can.h>
#include <usart.h>
#include <bitwise.h>
#include "gps.h"

static void rx_complete(uint8_t mob);
static void tx_complete(uint8_t mob);
static void can_default(uint8_t mob);

int main(void) {
	set_canit_callback(CANIT_RX_COMPLETED, rx_complete);
	set_canit_callback(CANIT_TX_COMPLETED, tx_complete);
	set_canit_callback(CANIT_DEFAULT, can_default);

	gps_set_getc(usart1_getc);
	usart1_init(GPS_BAUDRATE);
	CAN_SEI();
	CAN_EN_RX_INT();
	CAN_EN_TX_INT();

	sei();	//Enable interrupt

	gps_fix_t fix;

	// Main work loop
	while(1){
		if (gps_get_fix(&fix) == 0 ) {
			can_msg_t lat = {
				.mob = 1,
				.id = 4,
				.data = {
					1,
					fix.latitude.direction,
					HIGH_BYTE(fix.latitude.degrees),
					LOW_BYTE(fix.latitude.degrees),
					fix.latitude.minutes,
					fix.latitude.seconds,
					fix.valid
				},
				.dlc = 6,
				.mode = MOB_TRANSMIT
			};

			can_msg_t lon = {
				.mob = 2,
				.id = 4,
				.data = {
					2,
					fix.longitude.direction,
					HIGH_BYTE(fix.longitude.degrees),
					LOW_BYTE(fix.longitude.degrees),
					fix.longitude.minutes,
					fix.longitude.seconds,
					HIGH_BYTE(fix.speed),
					LOW_BYTE(fix.speed)
				},
				.dlc = 7,
				.mode = MOB_TRANSMIT
			};
			can_send(&lat);
			can_send(&lon);
		}
	}

    return 0;
}

static void rx_complete(uint8_t mob) {
	can_msg_t msg = {
		.mob = mob
	};
	can_receive(&msg);
}

static void tx_complete(uint8_t mob) {
	MOB_ABORT();					// Freed the MOB
	MOB_CLEAR_INT_STATUS();			// and reset MOb status
	CAN_DISABLE_MOB_INTERRUPT(mob);	// Unset interrupt
}

static void can_default(uint8_t mob) {
	MOB_CLEAR_INT_STATUS(); 		// and reset MOb status
}
