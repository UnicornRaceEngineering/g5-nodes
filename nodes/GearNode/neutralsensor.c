#include <avr/interrupt.h>
#include <io.h>

#include "neutralsensor.h"

#define NEUT_PORT		PORTE
#define NEUT_PIN		PIN7

#define NEUT_EICR		EICRA
#define NEUT_EIMSK		EIMSK

#define NEUT_ISC1		ISC71
#define NEUT_ISC0		ISC70

#define NEUT_INT		INT7
#define NEUT_INT_vect	INT7_vect

#define GEAR_IS_NEUTRAL()	( !DIGITAL_READ(NEUT_PORT, NEUT_PIN) )

void init_neutral_gear_sensor(void) {
	SET_PIN_MODE(NEUT_PORT, NEUT_PIN, INPUT_PULLUP);

	// Interrupt on any logical change on port E pin 7
	SET_REGISTER_BITS(NEUT_EICR,
		(0<<NEUT_ISC1|1<<NEUT_ISC0),
		(1<<NEUT_ISC1|1<<NEUT_ISC0));

	BIT_SET(NEUT_EIMSK, NEUT_INT); // Enables external interrupt request
}

// Gear neutral interrupt
ISR(NEUT_INT_vect) {
	// TODO: Implement this
}
