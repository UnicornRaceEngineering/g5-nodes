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
#include <stdbool.h>
#include <stdio.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include <can.h>
#include <usart.h>

#include "paddleshift.h"

static void rx_complete(uint8_t mob);
static void tx_complete(uint8_t mob);
static void can_default(uint8_t mob);


int main(void) {
	set_canit_callback(CANIT_RX_COMPLETED, rx_complete);
	set_canit_callback(CANIT_TX_COMPLETED, tx_complete);
	set_canit_callback(CANIT_DEFAULT, can_default);

	can_init();

	CAN_SEI();
	CAN_EN_RX_INT();
	CAN_EN_TX_INT();

	usart1_init(115200);
	paddle_init();

	sei();										//Enable interrupt


	usart1_printf("\n\n\nSTARTING\n");

	while(1){
		// Main work loop

		// First lets store the current status of the paddleshifters
		const bool paddle_up_is_pressed = paddle_up_status();
		const bool paddle_down_is_pressed = paddle_down_status();

		if (paddle_up_is_pressed) {
			//!< @TODO: broadcast this event on the can.
		} else if (paddle_down_is_pressed) {
			//!< @TODO: broadcast this event on the can.
		}
	}

    return 0;
}

static void rx_complete(uint8_t mob) {
	can_msg_t msg = {
		.mob = mob
	};
	can_receive(&msg);
	usart1_printf("Received id: %d on mob %d :: ", msg.id, msg.mob);
#if 0
	// Print ascii data
	usart1_putn(msg.dlc, msg.data);
#else
	// Print binary data as hex
	for (int i = 0; i < msg.dlc; ++i) {
		usart1_printf("0x%02x ", msg.data[i]);
	}
#endif
	usart1_putc('\n');
}

static void tx_complete(uint8_t mob) {
	MOB_ABORT();					// Freed the MOB
	MOB_CLEAR_INT_STATUS();			// and reset MOb status
	CAN_DISABLE_MOB_INTERRUPT(mob);	// Unset interrupt
}

static void can_default(uint8_t mob) {
	MOB_CLEAR_INT_STATUS(); 		// and reset MOb status
}
