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

/**
 * @files vnh2sp30.h
 * Driver implementation for the vnh2sp30 h-bridge.
 *
 * The h-bridge has a few pins that we should concern ourselves with, namely
 * INA, INB, DIAGA DIAGB, PWM and CS.
 *
 * INA and INB:
 *  controls the direction of the connected motor.
 *
 * DIAGA and DIAGB:
 *  TODO: WRITE THIS DISCRIPTION
 *
 * PWM:
 * 	The input PWM that controls the speed of the connected motor
 *
 * CS:
 * 	Current Sense, holds an analog value of how much current the motor is using.
 */

#ifndef VNH2SP30_H
#define VNH2SP30_H

#include <avr/io.h>
#include <pwm.h>
#include <adc.h>
#include <io.h>

#define VNH2SP30_INA_PORT	(PORTA)
#define VNH2SP30_INA_PIN	(PIN0)

#define VNH2SP30_INB_PORT	(PORTA)
#define VNH2SP30_INB_PIN	(PIN1)

#define VNH2SP30_PWM_PORT	(PORTB)
#define VNH2SP30_PWM_PIN	(PIN5)

#define VNH2SP30_DIAGA_PORT	(PORTA)
#define VNH2SP30_DIAGA_PIN	(PIN2)

#define VNH2SP30_DIAGB_PORT	(PORTA)
#define VNH2SP30_DIAGB_PIN	(PIN3)

#define VNH2SP30_CS_PORT	(PORTF)
#define VNH2SP30_CS_PIN		(PIN0)

void vnh2sp30_init(void);
void vnh2sp30_active_break_to_GND(void);
void vnh2sp30_active_break_to_Vcc(void);

#define vnh2sp30_set_INA() \
	IO_SET_HIGH(VNH2SP30_INA_PORT, VNH2SP30_INA_PIN)
#define vnh2sp30_set_INB() \
	IO_SET_HIGH(VNH2SP30_INB_PORT, VNH2SP30_INB_PIN)

#define vnh2sp30_clear_INA() \
	IO_SET_LOW(VNH2SP30_INA_PORT, VNH2SP30_INA_PIN)
#define vnh2sp30_clear_INB() \
	IO_SET_LOW(VNH2SP30_INB_PORT, VNH2SP30_INB_PIN)

#define vnh2sp30_read_DIAGA() \
	DIGITAL_READ(VNH2SP30_DIAGA_PORT, VNH2SP30_DIAGA_PIN)
#define vnh2sp30_read_DIAGB() \
	DIGITAL_READ(VNH2SP30_DIAGB_PORT, VNH2SP30_DIAGB_PIN)

#define vnh2sp30_read_CS() \
	adc_readChannel(VNH2SP30_CS_PIN)

#define vnh2sp30_set_PWM_dutycycle(dutycycle) \
	pwm_PB5_set_dutycycle((dutycycle))

#endif /* VNH2SP30_H */
