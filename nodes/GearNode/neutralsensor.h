#ifndef NEUTRALSENSOR_H
#define NEUTRALSENSOR_H

#include <io.h>
#include <stdint.h>

#define NEUT_PORT		PORTE
#define NEUT_PIN		PIN5

#define NEUT_EICR		EICRB
#define NEUT_EIMSK		EIMSK

#define NEUT_ISC1		ISC51
#define NEUT_ISC0		ISC50

#define NEUT_INT		INT5
#define NEUT_INT_vect	INT5_vect

#define GEAR_IS_NEUTRAL()	( !DIGITAL_READ(NEUT_PORT, NEUT_PIN) )

void init_neutral_gear_sensor(void);

enum {
	UP,
	DOWN,
};

volatile uint8_t gear_shift_dir;
volatile uint8_t real_gear_pos;

#endif /* NEUTRALSENSOR_H */
