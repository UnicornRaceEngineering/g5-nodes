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

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint8_t
#include <stdio.h>   // for fputc, FILE
#include <string.h>  // for memcpy, memset
#include <usart.h>   // for usart1_init, usart1_byte_output
#include <utils.h>   // for ARR_LEN
#include <system_messages.h>

#include "log.h"
#include "xbee.h"

#define XBEE_BAUD 	(115200)

static FILE *xbee_out = &usart1_byte_output;
static FILE *xbee_in = &usart1_io;
#define xbee_has_data()	usart1_has_data()

static uint8_t buf_in[64];
static uint8_t buf_out[64];

enum data_request {
	REQUEST_NONE,
	REQUEST_LOG,
	REQUEST_NUM_LOGS,
};

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
		xbee_flush();
	}

	// Then add it to payload
	memcpy(&p.buf[p.i], arr, len);
	p.i += len;
}

void xbee_flush(void) {
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

int request_log(void) {
	const uint8_t lo = fgetc(xbee_in);
	const uint8_t hi = fgetc(xbee_in);

	const uint16_t lognr = MERGE_BYTE(hi, lo);

	return log_read(lognr, xbee_out); // TODO something about multi frame messages and that log_read does not use the standard package format and then breaks everything
}

int request_num_logs(void) {
	unsigned n_logs = log_get_num_logs();
	xbee_send((uint8_t*)&((uint16_t){n_logs}), sizeof(uint16_t));
	return 0;
}

int xbee_check_request(void) {
	if (!xbee_has_data()) return 0;

	const enum data_request r = (int)fgetc(xbee_in);

	xbee_flush();
	xbee_send((uint8_t*)&((uint16_t){REQUEST_OFFSET+r}), sizeof(uint16_t));

	int rc;
	switch (r) {
		case REQUEST_LOG: 		rc = request_log(); break;
		case REQUEST_NUM_LOGS: 	rc = request_num_logs(); break;

		case REQUEST_NONE:
		default:
			rc = 0; break;
	}

	xbee_flush();
	return rc;
}
