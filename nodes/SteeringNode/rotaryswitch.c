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
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <utils.h>
#include <io.h>

#include "rotaryswitch.h"

void rot_init(void) {
	MCUCR |= (1 << JTD); // Disable JTAG (as JTAG is on ROT_PORT (PORTF))
	MCUCR |= (1 << JTD); // Must be run twice (safety measure)

	SET_PIN_MODE(ROT_PORT, ROT_B1, INPUT);
	SET_PIN_MODE(ROT_PORT, ROT_B2, INPUT);
	SET_PIN_MODE(ROT_PORT, ROT_B3, INPUT);
	SET_PIN_MODE(ROT_PORT, ROT_B4, INPUT);
}

/**
 * @note For some reason the AVR chip wont read ROT_B1 This results in only even
 * numbers are read (This effectively halves the number of stages of the switch).
 * @return  rotary switch value
 */
uint8_t rot_read(void) {
	const uint8_t rot_bcd = (PIN_PORT(ROT_PORT) & ROT_B_MASK) >> 4;
	const uint8_t bcd[] = {
		0b0000, 0b1000, 0b0100, 0b1100, 0b0010, 0b1010, 0b0110, 0b1110,
		0b0001, 0b1001, 0b0101, 0b1101, 0b0011, 0b1011, 0b0111, 0b1111,
	};

	uint8_t num;
	for (num = 0; num < ARR_LEN(bcd); ++num) {
		if (bcd[num] == rot_bcd) break;
	}

	return num;
}
