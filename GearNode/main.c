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
#include <stdbool.h>

#include <can.h>
#include <usart.h>
#include <io.h>
#include <bitwise.h>

static void rx_complete(uint8_t mob);
static void tx_complete(uint8_t mob);
static void can_default(uint8_t mob);

#define IGNITION_CUT_ON()	( BIT_SET(PORTE, PIN4) )
#define IGNITION_CUT_OFF()	( BIT_CLEAR(PORTE, PIN4) )

#define GEAR_IS_NEUTRAL()	( !DIGITAL_READ(PORTE, PIN7) )

#define SERVO_UP				(210)
#define SERVO_DOWN 				(500)
#define SERVO_MIDT				(332)
#define SERVO_NEUTRAL_FROM_1	(264)
#define SERVO_NEUTRAL_FROM_2	(400)

enum {
	GEAR_DOWN = -1,
	GEAR_NEUTRAL = 0,
	GEAR_UP = 1
};

static void init_pwm16_OC3C_prescalar64(unsigned int count_to) {
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

	// Count to the specified value
	ICR3H = HIGH_BYTE(count_to);
	ICR3L = LOW_BYTE(count_to);

	// Set prescalar to 64
	BIT_SET(TCCR3B, CS30);
	BIT_SET(TCCR3B, CS31);
	BIT_CLEAR(TCCR3B, CS32);
}

void set_servo_duty_from_pos(unsigned int pos) {
	int duty_cycle = (0.6278260870 * (float)pos) + 42.63130435;
	OCR3CH = HIGH_BYTE(duty_cycle);
	OCR3CL = LOW_BYTE(duty_cycle);
}

int shift_gear(int gear_dir) {
	static int current_gear = 0;
	bool err = 0;

	/**
	 * @todo: Should this be in an interrupt?
	 */
	if (GEAR_IS_NEUTRAL()) current_gear = 0;

	IGNITION_CUT_ON();
	_delay_ms(100);

	switch (gear_dir) {
		case GEAR_DOWN:
			if (current_gear != 0) {
				set_servo_duty_from_pos(SERVO_DOWN);
				current_gear--;
			}
			break;
		case GEAR_NEUTRAL:
			if (current_gear >= 2) {
				set_servo_duty_from_pos(SERVO_NEUTRAL_FROM_2);
			} else {
				set_servo_duty_from_pos(SERVO_NEUTRAL_FROM_1);
			}

			current_gear = 0;
			break;
		case GEAR_UP:
			if (current_gear != 0) {
				set_servo_duty_from_pos(SERVO_UP);
			} else {
				// Special case. If we are in neutral we have to shift down to
				// get to gear 1 as it is laid out as [1, 0, 2, 3, 4, 5, 6]
				set_servo_duty_from_pos(SERVO_DOWN);
			}
			current_gear++;
			break;
		default: err = 1; break;
	}
	_delay_ms(500);

	set_servo_duty_from_pos(SERVO_MIDT);
	_delay_ms(500);

	IGNITION_CUT_OFF();

	return !err ? current_gear : -current_gear;
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

	init_pwm16_OC3C_prescalar64(2047); // 0x07FF (11 bits)

	// setup neutral gear sensor
	SET_PIN_MODE(PORTE, PIN7, INPUT_PULLUP);

	sei();										//Enable interrupt


	usart1_printf("\n\n\nSTARTING\n");

	int gear = 0;

	while(1){
		if (usart1_hasData()) {
			char c = usart1_getc();
			switch (c) {
				case 'q':
					gear = shift_gear(GEAR_DOWN);
					break;
				case 'w':
					gear = shift_gear(GEAR_NEUTRAL);
					break;
				case 'e':
					gear = shift_gear(GEAR_UP);
					break;
			}
			usart1_printf("Gear: %d Neutral: %d\n", gear, GEAR_IS_NEUTRAL());
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
