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
 * @files hbridge_vnh2sp30.h
 * This implement the basic driver interface to the vnh2sp30 h-bridge.
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

#include <avr/io.h>

#define HBRIDGE_INA_PORT	(PORTA)
#define HBRIDGE_INA_PIN		(PIN0)

#define HBRIDGE_INB_PORT	(PORTA)
#define HBRIDGE_INB_PIN		(PIN1)

#define HBRIDGE_PWM_PORT	(PORTB)
#define HBRIDGE_PWM_PIN		(PIN5)

#define HBRIDGE_DIAGA_PORT	(PORTA)
#define HBRIDGE_DIAGA_PIN	(PIN2)

#define HBRIDGE_DIAGB_PORT	(PORTA)
#define HBRIDGE_DIAGB_PIN	(PIN3)

#define HBRIDGE_CS_PORT		(PORTF)
#define HBRIDGE_CS_PIN		(PIN0)


