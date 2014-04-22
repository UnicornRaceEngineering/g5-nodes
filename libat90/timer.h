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
 * @file timer.h
 * @brief
 * Provides a simplified interface to the various available timers.
 */

#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <stdint.h>

/**
 * CS02, CS01 and CS00 (Clock Select) sets the source used by the Timer/Counter
 * If external pin modes are used for the Timer/Counter0, transitions on the T0
 * pin will clock the counter even if the pin is configured as an output. This
 * feature allows software control of the counting.
 *
 * For more infomation see the datasheet page 111
 */
enum timer0_prescalar_t {
	TIMER0_PRESCALAR_NO_SOURCE 	=		(0   |0   |0   ), //!< No clock source (Timer/Counter stopped)
	TIMER0_PRESCALAR_1 			=		(0   |0   |CS00), //!< clkI/O/(No prescaling)
	TIMER0_PRESCALAR_8 			=		(0   |CS01|0   ), //!< clkI/O/8 (From prescaler)
	TIMER0_PRESCALAR_64 		=		(0   |CS01|CS00), //!< clkI/O/64 (From prescaler)
	TIMER0_PRESCALAR_256 		=		(CS02|0   |0   ), //!< clkI/O/256 (From prescaler)
	TIMER0_PRESCALAR_1024 		=		(CS02|0   |CS01), //!< clkI/O/1024 (From prescaler)

	TIMER0_PRESCALAR_EXTERNAL_FALLING = (CS02|CS01|0   ), //!< External clock source on T0 pin. Clock on falling edge.
	TIMER0_PRESCALAR_EXTERNAL_RISING  = (CS02|CS01|CS00)  //!< External clock source on T0 pin. Clock on rising edge.
};

/**
 * CS12, CS11 and CS10 (Clock Select) sets the source used by the Timer/Counter
 * If external pin modes are used for the Timer/Counter0, transitions on the T1
 * pin will clock the counter even if the pin is configured as an output. This
 * feature allows software control of the counting.
 *
 * For more infomation see the datasheet page 139
 */
enum timer1_prescalar_t {
	TIMER1_PRESCALAR_NO_SOURCE 	=		(0   |0   |0   ), //!< No clock source (Timer/Counter stopped)
	TIMER1_PRESCALAR_1 			=		(0   |0   |CS10), //!< clkI/O/(No prescaling)
	TIMER1_PRESCALAR_8 			=		(0   |CS11|0   ), //!< clkI/O/8 (From prescaler)
	TIMER1_PRESCALAR_64 		=		(0   |CS11|CS10), //!< clkI/O/64 (From prescaler)
	TIMER1_PRESCALAR_256 		=		(CS12|0   |0   ), //!< clkI/O/256 (From prescaler)
	TIMER1_PRESCALAR_1024 		=		(CS12|0   |CS11), //!< clkI/O/1024 (From prescaler)

	TIMER1_PRESCALAR_EXTERNAL_FALLING = (CS12|CS11|0   ), //!< External clock source on T1 pin. Clock on falling edge.
	TIMER1_PRESCALAR_EXTERNAL_RISING  = (CS12|CS11|CS10)  //!< External clock source on T1 pin. Clock on rising edge.
};

/**
 * CS12, CS11 and CS10 (Clock Select) sets the source used by the Timer/Counter.
 *
 * For more infomation see the datasheet page 159
 */
enum timer2_prescalar_t {
	TIMER2_PRESCALAR_NO_SOURCE 	= (0   |0   |0   ), //!< No clock source (Timer/Counter stopped)
	TIMER2_PRESCALAR_1 			= (0   |0   |CS20), //!< clkT2S/(No prescaling)
	TIMER2_PRESCALAR_8 			= (0   |CS21|0   ), //!< clkT2S/8 (From prescaler)
	TIMER2_PRESCALAR_32 		= (0   |CS21|CS20), //!< clkT2S/32 (From prescaler)
	TIMER2_PRESCALAR_64 		= (CS22|0   |0   ), //!< clkT2S/64 (From prescaler)
	TIMER2_PRESCALAR_128 		= (CS22|0   |CS20), //!< clkT2S/128 (From prescaler)
	TIMER2_PRESCALAR_256 		= (CS22|CS21|0   ), //!< clkT2S/256 (From prescaler)
	TIMER2_PRESCALAR_1024 		= (CS22|CS21|CS20)  //!< clkT2S/1024 (From prescaler)
};

/**
 * CS32, CS31 and CS30 (Clock Select) sets the source used by the Timer/Counter
 * If external pin modes are used for the Timer/Counter0, transitions on the T3
 * pin will clock the counter even if the pin is configured as an output. This
 * feature allows software control of the counting.
 *
 * For more infomation see the datasheet page 139
 */
enum timer3_prescalar_t {
	TIMER3_PRESCALAR_NO_SOURCE 	=		(0   |0   |0   ), //!< No clock source (Timer/Counter stopped)
	TIMER3_PRESCALAR_1 			=		(0   |0   |CS30), //!< clkI/O/(No prescaling)
	TIMER3_PRESCALAR_8 			=		(0   |CS31|0   ), //!< clkI/O/8 (From prescaler)
	TIMER3_PRESCALAR_64 		=		(0   |CS31|CS30), //!< clkI/O/64 (From prescaler)
	TIMER3_PRESCALAR_256 		=		(CS32|0   |0   ), //!< clkI/O/256 (From prescaler)
	TIMER3_PRESCALAR_1024 		=		(CS32|0   |CS31), //!< clkI/O/1024 (From prescaler)

	TIMER3_PRESCALAR_EXTERNAL_FALLING = (CS32|CS31|0   ), //!< External clock source on T1 pin. Clock on falling edge.
	TIMER3_PRESCALAR_EXTERNAL_RISING  = (CS32|CS31|CS30)  //!< External clock source on T1 pin. Clock on rising edge.
};

void timer0_set_prescalar(uint8_t prescalar);
void timer1_set_prescalar(uint8_t prescalar);
void timer2_set_prescalar(uint8_t prescalar);
void timer3_set_prescalar(uint8_t prescalar);

#endif /* TIMER_H */
