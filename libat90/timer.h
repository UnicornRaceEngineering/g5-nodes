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

enum timer_prescalar_t {
	PRESCALAR_1,
	PRESCALAR_8,
	PRESCALAR_64,
	PRESCALAR_256,
	PRESCALAR_1024
};

enum timer_16bit_ConReg {
	TIMER1,
	TIMER3
};

int Waveform_Generation_Mode;


void timer_setPrescaler(const enum timer_prescalar_t p);
void timer_setMode(const enum timer_16bit_ConReg, const unsigned int Waveform_Generation_Mode);

#endif /* TIMER_H */