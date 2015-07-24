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
#include <string.h> // memcpy() memset()

#include "xbee.h"

#define XBEE_BAUD 	(115200)

static FILE *xbee_out = &usart1_byte_output;

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static struct payload {
	// the buffer is the size of uart buffer with package overhead substracted.
	// This is to avoid filling the uart buffer.
	uint8_t buf[64 - 3 - 1 - 1];
	size_t i;
} p;

void xbee_init(void) {
	memset(&p, 0, sizeof(p));
	usart1_init(XBEE_BAUD, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
}

void xbee_send(const uint8_t *arr, uint8_t len) {
	// Check if buffer has room for data
	if (p.i + len >= ARR_LEN(p.buf)) {
		// If not then flush the buffer

		const uint8_t start_seq[] = {0xA1, 0xB2, 0xC3};
		for (size_t i = 0; i < ARR_LEN(start_seq); ++i) fputc(start_seq[i], xbee_out);

		fputc(p.i+1, xbee_out); // size of payload + chksum

		uint8_t chksum = 0;
		for (size_t i = 0; i < p.i; ++i) {
			fputc(p.buf[i], xbee_out);
			chksum ^= p.buf[i];
		}
		fputc(chksum, xbee_out);

		p.i = 0; // Reset payload index
	}

	// Then add it to payload
	memcpy(&p.buf[p.i], arr, len);
	p.i += len;
}
