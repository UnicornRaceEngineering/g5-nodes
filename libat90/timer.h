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

void timer0_set_prescalar(uint8_t prescalar);

#endif /* TIMER_H */
