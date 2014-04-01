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

#include <avr/interrupt.h>
#include <util/delay.h>

#include <can.h>
#include <usart.h>
#include <io.h>
#include <bitwise.h>

static void rx_complete(uint8_t mob);
static void tx_complete(uint8_t mob);
static void can_default(uint8_t mob);


static void init_pwm16(void) {
	// OC3C, Output Compare Match C output (counter 3 output compare)
	SET_PIN_MODE(PORTE, PIN5, OUTPUT);

	// Clear on Compare Match
	BIT_SET(TCCR3A, COM3C1);
	BIT_CLEAR(TCCR3A, COM3C0);

	// Set Wave Generation Mode to Fast PWM counting to ICR
	BIT_CLEAR(TCCR3A, WGM30);
	BIT_SET(TCCR3A, WGM31);
	BIT_SET(TCCR3B, WGM32);
	BIT_SET(TCCR3B, WGM33);

	// Count to 2047
	ICR3H = 0x07;
	ICR3L = 0xFF;

	// Set prescalar to 64
	BIT_SET(TCCR3B, CS30);
	BIT_SET(TCCR3B, CS31);
	BIT_CLEAR(TCCR3B, CS32);
}

int main(void) {
	set_canit_callback(CANIT_RX_COMPLETED, rx_complete);
	set_canit_callback(CANIT_TX_COMPLETED, tx_complete);
	set_canit_callback(CANIT_DEFAULT, can_default);

	//Initialise the Gear node
	usart1_init(115200);

	can_init();

	CAN_SEI();
	CAN_EN_RX_INT();
	CAN_EN_TX_INT();

	init_pwm16();

	sei();										//Enable interrupt


	usart1_printf("\n\n\nSTARTING\n");

	while(1){
		// Main work loop
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
