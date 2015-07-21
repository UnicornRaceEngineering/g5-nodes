#include <avr/interrupt.h>
#include <io.h>
#include <stdio.h>
#include "usart.h"

#include "neutralsensor.h"


void init_neutral_gear_sensor(void) {
	SET_PIN_MODE(NEUT_PORT, NEUT_PIN, INPUT_PULLUP);

	// Interrupt on any logical change on port E pin 7
	NEUT_EICR |= (0<<NEUT_ISC1|1<<NEUT_ISC0);

	BIT_SET(NEUT_EIMSK, NEUT_INT); // Enables external interrupt request

	gear_shift_dir = 0;
	real_gear_pos = 0;
}

// Gear neutral interrupt
ISR(NEUT_INT_vect) {
	if (gear_shift_dir == DOWN) {
		real_gear_pos = 1;
	} else if (gear_shift_dir == UP) {
		real_gear_pos = 2;
	}
}
