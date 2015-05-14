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
}

// Gear neutral interrupt
ISR(NEUT_INT_vect) {
	// TODO: Implement this
	while(USART1_TX_IS_BUSY());
	UDR1 = '!';
}
