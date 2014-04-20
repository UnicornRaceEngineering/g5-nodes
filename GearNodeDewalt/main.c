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

#define PRESCALAR	(64)

#define IGN_PORT	(PORTE)
#define IGN_PIN		(PIN4)
#define IGNITION_CUT_DELAY()	( _delay_ms(100) )
#define IGNITION_CUT_ON()		( BIT_SET(IGN_PORT, IGN_PIN) )
#define IGNITION_CUT_OFF()		( BIT_CLEAR(IGN_PORT, IGN_PIN) )

#define SERVER_DELAY()			( _delay_ms(500) )

#define NEUT_PORT	(PORTE)
#define NEUT_PIN	(PIN7)
#define GEAR_IS_NEUTRAL()	( !DIGITAL_READ(NEUT_PORT, NEUT_PIN) )

#define MOTOR_PWM_PORT	(PORTB)
#define MOTOR_PWM_PIN	(PIN5)
#define GEAR_PWM_ON() 	(BIT_SET(MOTOR_PWM_PORT, MOTOR_PWM_PIN))
#define GEAR_PWM_OFF() 	(BIT_CLEAR(MOTOR_PWM_PORT, MOTOR_PWM_PIN))

#define MOTOR_PORT		(PORTA)
#define MOTOR_UP_PIN	(PIN0)
#define MOTOR_DOWN_PIN	(PIN1)
#define GEAR_MOTOR_UP() 	(BIT_SET(MOTOR_PORT, MOTOR_UP_PIN))
#define GEAR_MOTOR_DOWN() 	(BIT_SET(MOTOR_PORT, MOTOR_DOWN_PIN))
#define GEAR_MOTOR_OFF_UP() 	(BIT_CLEAR(MOTOR_PORT, MOTOR_UP_PIN))
#define GEAR_MOTOR_OFF_DOWN() 	(BIT_CLEAR(MOTOR_PORT, MOTOR_DOWN_PIN))

#define TOP_TO_HZ(top)	(F_CPU / (PRESCALAR * (1+(top))))
#define HZ_TO_MS(hz)	((1/(hz)) * 1000)

#define MS_TO_TOP(ms)	((uint16_t) \
	((((F_CPU/1000.0)/(double)PRESCALAR)*((double)(ms)))) - 1)

#define SERVO_UP				(MS_TO_TOP(1))
#define SERVO_DOWN				(MS_TO_TOP(2))
#define SERVO_MIDT				(MS_TO_TOP(1.5))
#define SERVO_NEUTRAL_FROM_1	(MS_TO_TOP(1.25))
#define SERVO_NEUTRAL_FROM_2	(MS_TO_TOP(1.75))

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

static void init_pwm16_OC3C_prescalar64(uint16_t count_to) {
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

static inline void set_servo_duty_from_pos(uint16_t duty_top) {
	OCR3CH = HIGH_BYTE(duty_top);
	OCR3CL = LOW_BYTE(duty_top);
}

static void init_shift_gear(){
	SET_PIN_MODE(PORTB, GEAR_MOTOR_UP, OUTPUT);
	SET_PIN_MODE(PORTB, GEAR_MOTOR_DOWN, OUTPUT);
}

static int shift_gear(int gear_dir) {
	
	bool err = 0;

	/**
	 * @todo: Should this still be done outside the interrupt?
	 */
	if (GEAR_IS_NEUTRAL()) current_gear = 0;

	IGNITION_CUT_ON();
	IGNITION_CUT_DELAY();
	GEAR_PWM_ON();
	switch (gear_dir) {
		case GEAR_DOWN:
			if (current_gear != 0) { // CHANGE THIS NB 1. GEAR 
				GEAR_MOTOR_DOWN();
				current_gear--;
			}
			break;
		case GEAR_NEUTRAL:
			if (current_gear >= 2) {
				GEAR_MOTOR_UP(); // CHANGE THIS !!
				//set_servo_duty_from_pos(SERVO_NEUTRAL_FROM_2);
			} else {
				
				GEAR_MOTOR_DOWN (); // CHANGE THIS !!
				//set_servo_duty_from_pos(SERVO_NEUTRAL_FROM_1);
			}

			current_gear = 0;
			break;
		case GEAR_UP:
			if (current_gear != 0) {
				GEAR_MOTOR_UP();
				//set_servo_duty_from_pos(SERVO_UP);
			} else {
				// Special case. If we are in neutral we have to shift down to
				// get to gear 1 as it is laid out as [1, 0, 2, 3, 4, 5, 6]
				GEAR_MOTOR_DOWN ();
				//set_servo_duty_from_pos(SERVO_DOWN);
			}
			current_gear++;
			break;
		default: 
			err = 1; 
			break;
	}

//	switch (gear_dir) {
//		case GEAR_DOWN:
//			if (current_gear != 0) {
//				set_servo_duty_from_pos(SERVO_DOWN);
//				current_gear--;
//			}
//			break;
//		case GEAR_NEUTRAL:
//			if (current_gear >= 2) {
//				set_servo_duty_from_pos(SERVO_NEUTRAL_FROM_2);
//			} else {
//				set_servo_duty_from_pos(SERVO_NEUTRAL_FROM_1);
//			}
//
//			current_gear = 0;
//			break;
//		case GEAR_UP:
//			if (current_gear != 0) {
//				set_servo_duty_from_pos(SERVO_UP);
//			} else {
//				// Special case. If we are in neutral we have to shift down to
//				// get to gear 1 as it is laid out as [1, 0, 2, 3, 4, 5, 6]
//				set_servo_duty_from_pos(SERVO_DOWN);
//			}
//			current_gear++;
//			break;
//		default: err = 1; break;
//	}
//
//set_servo_duty_from_pos(SERVO_MIDT);
	GEAR_PWM_OFF();
	GEAR_MOTOR_OFF_UP();	
	GEAR_MOTOR_OFF_DOWN();
	SERVER_DELAY();

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

	// (F_CPU / (Prescalar * ( 1+2047)) =
	// (11059200 Hz / (64 * (1+2047))) 	=
	// 84.375 Hz = 11.852 ms period
	// Resolution = log(2047+1)/log(2) = 11 bits
	init_pwm16_OC3C_prescalar64(2047);

	init_neutral_gear_sensor();

	init_shift_gear();

	// Set ignition cut pin to output
	SET_PIN_MODE(IGN_PORT, IGN_PIN, OUTPUT);

	IGNITION_CUT_OFF();
	sei();										//Enable interrupt


	usart1_printf("\n\n\nSTARTING\n");


	while(0){
#if 1
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
