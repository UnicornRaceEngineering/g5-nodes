#include <stdint.h>
#include <avr/interrupt.h>

#include "pwm.h"
#include "timer.h"
#include "bitwise.h"
#include "io.h"


void pwm_PE5_init(void) {
	// OC3C, Output Compare Match C output (counter 3 output compare)

	// Set prescalar to 64
#if PWM_PRESCALAR == 64
	timer3_set_prescalar(TIMER3_PRESCALAR_64);
#else
#	error undefined PWM_PRESCALAR
#endif

	// Count to the specified value
	const uint16_t count_to = PWM_TOP;
	ICR3H = HIGH_BYTE(count_to);
	ICR3L = LOW_BYTE(count_to);

	// Set Wave Generation Mode to Fast PWM counting to ICR
	timer3_set_waveform_generation_mode(TIMER3_WGM_FAST_PWM_ICR);

	// Clear on Compare Match
	SET_REGISTER_BITS(TCCR3A, (1<<COM3C1|1<<0    ), (1<<COM3C1|1<<COM3C0));
	SET_PIN_MODE(PORTE, PIN5, OUTPUT);
}
