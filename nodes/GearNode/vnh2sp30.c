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

#include "vnh2sp30.h"

#include <adc.h>      // for ADC_ENABLE, adc_setPrescaler, adc_setVref, etc
#include <pwm.h>      // for pwm_PB5_init
#include <stdbool.h>  // for bool

#include "io.h"       // for SET_PIN_MODE, io_pinmode_t::INPUT, etc

/**
 * @name vnh2sp30_init
 * Initializing
 * @{
 */
#define vnh2sp30_init_INA() \
	SET_PIN_MODE(VNH2SP30_INA_PORT, VNH2SP30_INA_PIN, OUTPUT)
#define vnh2sp30_init_INB() \
	SET_PIN_MODE(VNH2SP30_INB_PORT, VNH2SP30_INB_PIN, OUTPUT)

#define vnh2sp30_init_DIAGA() \
	SET_PIN_MODE(VNH2SP30_DIAGA_PORT, VNH2SP30_DIAGA_PIN, INPUT)
#define vnh2sp30_init_DIAGB() \
	SET_PIN_MODE(VNH2SP30_DIAGA_PORT, VNH2SP30_DIAGA_PIN, INPUT)

#define vnh2sp30_init_CS() do { \
	adc_setVref(AVCC); \
	adc_setPrescaler(ADC_PRESCALAR_128); \
	ADC_ENABLE(); \
} while (0)


void vnh2sp30_init(void) {
	vnh2sp30_init_INA();
	vnh2sp30_init_INB();

	vnh2sp30_init_DIAGA();
	vnh2sp30_init_DIAGB();

	vnh2sp30_init_CS();

	pwm_PB5_init();
}

void vnh2sp30_active_break_to_GND(void) {
	vnh2sp30_set_PWM_dutycycle(100);
	vnh2sp30_clear_INB();
	vnh2sp30_clear_INA();
}

void vnh2sp30_active_break_to_Vcc(void) {
	vnh2sp30_set_PWM_dutycycle(100);
	vnh2sp30_set_INB();
	vnh2sp30_set_INA();
}

bool vnh2sp30_is_faulty(void) {
	return !vnh2sp30_read_DIAGA() || !vnh2sp30_read_DIAGB();
}

void vnh2sp30_reset(void) {
	SET_PIN_MODE(VNH2SP30_DIAGA_PORT, VNH2SP30_DIAGA_PIN, OUTPUT);
	SET_PIN_MODE(VNH2SP30_DIAGB_PORT, VNH2SP30_DIAGB_PIN, OUTPUT);

	vnh2sp30_clear_INA();
	vnh2sp30_clear_INB();
	vnh2sp30_set_PWM_dutycycle(0);

	IO_SET_LOW(VNH2SP30_DIAGA_PORT, VNH2SP30_DIAGA_PIN);
	IO_SET_LOW(VNH2SP30_DIAGB_PORT, VNH2SP30_DIAGB_PIN);

	vnh2sp30_init_DIAGA();
	vnh2sp30_init_DIAGB();

}
