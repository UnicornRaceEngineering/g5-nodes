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

#ifndef STATUSLIGHT_H
#define STATUSLIGHT_H

#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>

#define STATUS_LED_PORT PORTB
#define STATUS_LED_R    PIN6 // The schematic mixes red and green
#define STATUS_LED_G    PIN5
#define STATUS_LED_B    PIN7
#define STATUS_LED_MASK ((1<<STATUS_LED_R)|(1<<STATUS_LED_G)|(1<<STATUS_LED_B))

enum color_masks {
	COLOR_OFF   = 0x00,

	RED         = 1 << STATUS_LED_R,
	GREEN       = 1 << STATUS_LED_G,
	BLUE        = 1 << STATUS_LED_B,

	YELLOW      = RED | GREEN,
	CYAN        = GREEN | BLUE,
	MAGENTA     = RED | BLUE,

	WHITE       = RED | GREEN | BLUE,
};

void statuslight_init(void);
void set_rgb_color(int led, enum color_masks color);

#endif /* STATUSLIGHT_H */
