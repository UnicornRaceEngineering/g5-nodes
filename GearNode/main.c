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

#include <usart.h>
#include <io.h>
#include <bitwise.h>
#include <pwm.h>

#define IGN_PORT	(PORTE)
#define IGN_PIN		(PIN4)
#define IGNITION_CUT_DELAY()	( _delay_ms(100) )
#define IGNITION_CUT_ON()		( BIT_SET(IGN_PORT, IGN_PIN) )
#define IGNITION_CUT_OFF()		( BIT_CLEAR(IGN_PORT, IGN_PIN) )


#define NEUT_PORT	(PORTE)
#define NEUT_PIN	(PIN7)
#define GEAR_IS_NEUTRAL()	( !DIGITAL_READ(NEUT_PORT, NEUT_PIN) )

#define SERVER_DELAY()			( _delay_ms(500) )

#define SERVO_UP				(MS_TO_TOP(1))
#define SERVO_DOWN				(MS_TO_TOP(2))
#define SERVO_MIDT				(MS_TO_TOP(1.5))
#define SERVO_NEUTRAL_FROM_1	(MS_TO_TOP(1.25))
#define SERVO_NEUTRAL_FROM_2	(MS_TO_TOP(1.75))

#define SERVO_SET_UP()				(pwm_PE5_set_top(SERVO_UP))
#define SERVO_SET_DOWN()			(pwm_PE5_set_top(SERVO_DOWN))
#define SERVO_SET_MIDT()			(pwm_PE5_set_top(SERVO_MIDT))
#define SERVO_SET_NEUTRAL_FROM_1()	(pwm_PE5_set_top(SERVO_NEUTRAL_FROM_1))
#define SERVO_SET_NEUTRAL_FROM_2()	(pwm_PE5_set_top(SERVO_NEUTRAL_FROM_2))

enum {
	GEAR_DOWN = -1,
	GEAR_NEUTRAL = 0,
	GEAR_UP = 1
};

volatile int current_gear = 0;


static void init_neutral_gear_sensor(void) {
	SET_PIN_MODE(NEUT_PORT, NEUT_PIN, INPUT_PULLUP);

	// Interrupt on any logical change on port E pin 7
	BIT_CLEAR(EICRA, ISC71);
	BIT_SET(EICRA, ISC70);

	BIT_SET(EIMSK, INT7); // Enables external interrupt request
}


static int shift_gear(int gear_dir) {
	bool err = 0;

	/**
	 * @todo: Should this still be done outside the interrupt?
	 */
	if (GEAR_IS_NEUTRAL()) current_gear = 0;

	IGNITION_CUT_ON();
	IGNITION_CUT_DELAY();

	switch (gear_dir) {
		case GEAR_DOWN:
			if (current_gear != 0) {
				SERVO_SET_DOWN();
				--current_gear;
			}
			break;
		case GEAR_NEUTRAL:
			if (current_gear >= 2) {
				SERVO_SET_NEUTRAL_FROM_2();
			} else {
				SERVO_SET_NEUTRAL_FROM_1();
			}

			current_gear = 0;
			break;
		case GEAR_UP:
			if (current_gear != 0) {
				SERVO_SET_UP();
			} else {
				// Special case. If we are in neutral we have to shift down to
				// get to gear 1 as it is laid out as [1, 0, 2, 3, 4, 5, 6]
				SERVO_SET_DOWN();
			}
			++current_gear;
			break;
		default: err = 1; break;
	}
	SERVER_DELAY();

	SERVO_SET_MIDT();
	SERVER_DELAY();

	IGNITION_CUT_OFF();

	return !err ? current_gear : -current_gear;
}

int main(void) {
	//Initialise the Gear node
	usart1_init(115200);

	pwm_PE5_init();

	init_neutral_gear_sensor();

	// Set ignition cut pin to output
	SET_PIN_MODE(IGN_PORT, IGN_PIN, OUTPUT);

	IGNITION_CUT_OFF();
	sei();										//Enable interrupt


	usart1_printf("\n\n\nSTARTING\n");


	while(1){
#if 0
		if (usart1_hasData()) {
			char c = usart1_getc();
			switch (c) {
				case 'q':
					shift_gear(GEAR_DOWN);
					break;
				case 'w':
					shift_gear(GEAR_NEUTRAL);
					break;
				case 'e':
					shift_gear(GEAR_UP);
					break;
			}
			usart1_printf("Gear: %d Neutral: %d\n", current_gear, GEAR_IS_NEUTRAL());
		}
#else
		while (shift_gear(GEAR_UP) <= 6){
			_delay_ms(250);
			usart1_printf("Gear: %d", current_gear);
		}
		while (shift_gear(GEAR_DOWN) != 0) {
			_delay_ms(250);
			usart1_printf("Gear: %d", current_gear);
		}
#endif
	}

    return 0;
}

// Gear neutral interrupt
ISR(INT7_vect) {
	if (GEAR_IS_NEUTRAL()) {
		if (current_gear != 0) {
			// An error has occured in the gear estimate. So lets correct it.
			current_gear = 0;
		}
	} else {
		const uint16_t servo_pos = MERGE_BYTE(OCR3CH, OCR3CL);
		switch (servo_pos) {
			case SERVO_UP:
				if (current_gear != 2) {
					// An error has occured in the gear estimate. So lets
					// correct it.
					current_gear = 2;
				}
				break;
			case SERVO_DOWN:
				if (current_gear != 1) {
					// An error has occured in the gear estimate. So lets
					// correct it.
					current_gear = 1;
				}
				break;
			case SERVO_MIDT:
				// An error has occured in the gear estimate. So lets correct it
				break;
			case SERVO_NEUTRAL_FROM_1:
				// An error has occured in the gear estimate. So lets correct it
				current_gear = 2;
				break;
			case SERVO_NEUTRAL_FROM_2:
				// An error has occured in the gear estimate. So lets correct it
				current_gear = 1;
				break;

			default:
				// The gear is in an unknown position. This should never happen.
				break;
		}
	}
}
