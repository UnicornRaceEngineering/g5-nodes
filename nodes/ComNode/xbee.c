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

#include <stdint.h>
#include <usart.h>
#include <utils.h>

#include "xbee.h"

#define XBEE_BAUD 	(115200)

FILE *xbee_out = &usart1_byte_output;
FILE *xbee_in = &usart1_io;

void xbee_init(void) {
	usart1_init(XBEE_BAUD);
}

void xbee_send(const uint8_t *arr, uint8_t len) {
	const uint8_t start_seq[] = {0xA1, 0xB2, 0xC3};
	for (size_t i = 0; i < ARR_LEN(start_seq); ++i) fputc(start_seq[i], xbee_out);

	fputc(len+1, xbee_out);

	uint8_t chksum = 0;
	for (int i = 0; i < len; ++i) {
		fputc(arr[i], xbee_out);
		chksum ^= arr[i];
	}
	fputc(chksum, xbee_out);
}

