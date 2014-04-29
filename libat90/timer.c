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
 * @file timer.c
 * @brief
 * Provides a simplified interface to the various available timers.
 */

#include <avr/io.h>
#include <stdint.h>
#include "timer.h"
#include "bitwise.h"


/**
 * @name set_prescalar
 * @param prescalar see timer0-3_prescalar_t for valid input
 * @{
 */

/**
 * Sets the prescalar for timer0
 * @param  prescalar see timer0_prescalar_t for valid input
 */
void timer0_set_prescalar(uint8_t prescalar) {
	const uint8_t mask = CS02|CS01|CS00;
	SET_REGISTER_BITS(TCCR0A, prescalar, mask);
}

/**
 * Sets the prescalar for timer1
 * @param prescalar see timer1_prescalar_t for valid input
 */
void timer1_set_prescalar(uint8_t prescalar) {
	const uint8_t mask = CS12|CS11|CS10;
	SET_REGISTER_BITS(TCCR1B, prescalar, mask);
}

/**
 * Sets the prescalar for timer2
 * @param prescalar See timer2_prescalar_t for valid input
 */
void timer2_set_prescalar(uint8_t prescalar) {
	const uint8_t mask = CS22|CS21|CS20;
	SET_REGISTER_BITS(TCCR2A, prescalar, mask);
}

/**
 * Sets the prescalar for timer3
 * @param prescalar see timer3_prescalar_t for valid input
 */
void timer3_set_prescalar(uint8_t prescalar) {
	const uint8_t mask = CS32|CS31|CS30;
	SET_REGISTER_BITS(TCCR3B, prescalar, mask);
}

/** @} */


/**
 * @name set_waveform_generation_mode
 * @param wgm The desired Waveform Generation Mode
 * @{
 */

void timer0_set_waveform_generation_mode(uint8_t wgm) {
	const uint8_t mask = WGM01|WGM00;
	SET_REGISTER_BITS(TCCR0A, wgm, mask);
}

void timer1_set_waveform_generation_mode(uint16_t wgm) {
	const uint8_t maskA = WGM11|WGM10;
	const uint8_t maskB = WGM13|WGM12;

	const uint8_t wgmA = LOW_BYTE(wgm);
	const uint8_t wgmB = HIGH_BYTE(wgm);

	SET_REGISTER_BITS(TCCR1A, wgmA, maskA);
	SET_REGISTER_BITS(TCCR1B, wgmB, maskB);
}

void timer2_set_waveform_generation_mode(uint8_t wgm) {
	const uint8_t mask = WGM21|WGM20;
	SET_REGISTER_BITS(TCCR2A, wgm, mask);
}

void timer3_set_waveform_generation_mode(uint16_t wgm) {
	const uint8_t maskA = WGM31|WGM30;
	const uint8_t maskB = WGM33|WGM32;

	const uint8_t wgmA = LOW_BYTE(wgm);
	const uint8_t wgmB = HIGH_BYTE(wgm);

	SET_REGISTER_BITS(TCCR3A, wgmA, maskA);
	SET_REGISTER_BITS(TCCR3B, wgmB, maskB);
}

/** @} */
