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

#include <avr/io.h>

#ifndef DEMULTIPLEXER_74LS138D
#define DEMULTIPLEXER_74LS138D

enum dmux_pins {
	DMUX_A_PIN = PIN7,
	DMUX_B_PIN = PIN6,
	DMUX_C_PIN = PIN5,
};

enum dmux_masks {
	DMUX_A = (1 << DMUX_A_PIN),
	DMUX_B = (1 << DMUX_B_PIN),
	DMUX_C = (1 << DMUX_C_PIN),

	DMUX_MASK = (DMUX_A|DMUX_B|DMUX_C),
};

/**
 *
 */
enum dmux_y_values {
	DMUX_Y0 = 0x00, // All off.
	DMUX_Y1 = DMUX_A,
	DMUX_Y2 = DMUX_B,
	DMUX_Y3 = DMUX_A|DMUX_B,
	DMUX_Y4 = DMUX_C,
	DMUX_Y5 = DMUX_A|DMUX_C,
	DMUX_Y6 = DMUX_B|DMUX_C,
	DMUX_Y7 = DMUX_A|DMUX_B|DMUX_C,
};

void dmux_init(void);
void dmux_set_y_low(enum dmux_y_values mask);

#endif /* DEMULTIPLEXER_74LS138D */
