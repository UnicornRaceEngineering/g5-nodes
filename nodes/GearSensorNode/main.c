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
#include <can.h>
#include "system_messages.h"  // for message_id, etc


#define BUTTON_A_PORT		PORTE
#define BUTTON_A_PIN		PIN6

#define BUTTON_A_ISC1		ISC61
#define BUTTON_A_ISC0		ISC60

#define BUTTON_A_INT		INT6
#define BUTTON_A_ISR_vect	INT6_vect


#define BUTTON_B_PORT		PORTE
#define BUTTON_B_PIN		PIN5

#define BUTTON_B_ISC1		ISC51
#define BUTTON_B_ISC0		ISC50

#define BUTTON_B_INT		INT5
#define BUTTON_B_ISR_vect	INT5_vect


#define NEUTRAL_PORT		PORTF
#define NEUTRAL_PIN			PIN3

#define ON true
#define OFF false


volatile static bool button_A_pressed = false;
volatile static bool button_B_pressed = false;
volatile static uint32_t button_timer = 0;


void button_init(void) {
	// Set up for right wheel
	SET_PIN_MODE(BUTTON_A_PORT, BUTTON_A_PIN, INPUT_PULLUP);
	// Generate synchronous interrupt on rising edge
	EICRB |= ((1 << BUTTON_A_ISC1) | (0 << BUTTON_A_ISC0));
	BIT_SET(EIMSK, BUTTON_A_INT); // Interrupt enable

	// Set up for left wheel
	SET_PIN_MODE(BUTTON_B_PORT, BUTTON_B_PIN, INPUT_PULLUP);
	// Generate synchronous interrupt on rising edge
	EICRB |= ((1 << BUTTON_B_ISC1) | (0 << BUTTON_B_ISC0));
	BIT_SET(EIMSK, BUTTON_B_INT); // Interrupt enable


	// Setup neutral button
	//SET_PIN_MODE(NEUTRAL_PORT, NEUTRAL_PIN, INPUT);

	SET_PIN_MODE(PORTF, PIN5, INPUT_PULLUP);
	SET_PIN_MODE(PORTF, PIN7, INPUT_PULLUP);
}


ISR(BUTTON_A_ISR_vect) {
	if (get_tick() > button_timer) {
		button_A_pressed = true;
		button_timer = get_tick() + 200;
	}
}


ISR(BUTTON_B_ISR_vect) {
	if (get_tick() > button_timer) {
		button_B_pressed = true;
		button_timer = get_tick() + 200;
	}
}


static void init(void) {
	sysclock_init();
	_delay_ms(100);
	button_init();
	can_init();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

static bool neutral_status = OFF;

static uint8_t last_state = 0;

int main(void) {
	init();

	while (1) {
		if (button_A_pressed) {
			can_broadcast(PADDLE_STATUS, &(uint8_t){1});
			button_A_pressed = false;
		}

		if (button_B_pressed) {
			can_broadcast(PADDLE_STATUS, &(uint8_t){0});
			button_B_pressed = false;
		}


		if (DIGITAL_READ(NEUTRAL_PORT, NEUTRAL_PIN) && (neutral_status == OFF)) {
			neutral_status = ON;
			_delay_ms(100);
			can_broadcast(NEUTRAL_ENABLED, &(uint8_t){1});
		} else if (!DIGITAL_READ(NEUTRAL_PORT, NEUTRAL_PIN) && (neutral_status == ON)) {
			neutral_status = OFF;
			_delay_ms(100);
			can_broadcast(NEUTRAL_ENABLED, &(uint8_t){0});
		}

		// new_state = 2 if front stop-button is pressed.
		// new_state = 1 if rear stop-button is pressed.
		const uint8_t new_state = (!DIGITAL_READ(PORTF, PIN5) * 1) + (!DIGITAL_READ(PORTF, PIN7) * 2);
		if (new_state != last_state) {
			last_state = new_state;
			can_broadcast(GEAR_STOP_BUTTON, &(uint8_t){new_state});
			_delay_ms(20);
		}
	}

	return 0;
}
