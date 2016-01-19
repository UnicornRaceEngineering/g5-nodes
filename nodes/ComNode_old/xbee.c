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
#include <util/delay.h>
#include <system_messages.h>

#include "log.h"
#include "xbee.h"

#define XBEE_BAUD 	(115200)

#define MAX_LABEL_SIZE	(32)


static FILE *xbee_out = &usart1_byte_output;
static FILE *xbee_in = &usart1_io;
#define xbee_has_data()	usart1_has_data()

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static uint32_t n_multi = 1; // Number of multi packages send,
static enum data_request req;

static struct payload {
	// the buffer is the size of uart buffer with package overhead substracted.
	// This is to avoid filling the uart buffer.
	uint8_t buf[64 - 3 - 1 - 1];
	size_t i;
} p;

static void prepare_multi_package(enum data_request r) {
	n_multi = 1;
	req = REQUEST_OFFSET+r;
	xbee_flush();
}

static unsigned send_multi_pkt(const uint8_t *buf, unsigned n) {
	if (n == 0) return 1;

	if (n + sizeof(n_multi) + sizeof(uint16_t) > ARR_LEN(p.buf)) return 0;

	xbee_send((uint8_t*)&((uint16_t){req}), sizeof(uint16_t));
	xbee_send((uint8_t*)&n_multi, sizeof(n_multi));
	n_multi++;
	xbee_send(buf, n);
	xbee_flush();

	_delay_ms(15); // We have to delay so we dont murder the xbee hardware buffer (not our software buffer)

	return n;
}
static void end_multi_pkt(void) {
	n_multi = 0; // zero means eof

	xbee_send((uint8_t*)&((uint16_t){req}), sizeof(uint16_t));
	xbee_send((uint8_t*)&n_multi, sizeof(n_multi));
	xbee_flush();
}

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

static int request_log(void) {
	const uint8_t lo = fgetc(xbee_in);
	const uint8_t hi = fgetc(xbee_in);

	const uint16_t lognr = MERGE_BYTE(hi, lo);

	prepare_multi_package(REQUEST_LOG);
	int rc = log_read(lognr, &send_multi_pkt);
	end_multi_pkt();
	return rc;
}

static int request_num_logs(void) {
	prepare_multi_package(REQUEST_NUM_LOGS);

	uint16_t n_logs = log_get_num_logs();
	send_multi_pkt((uint8_t*)&n_logs, sizeof(n_logs));

	end_multi_pkt();
	return 0;
}

static int request_insert_label(void) {
	char label[MAX_LABEL_SIZE] = {'\0'};
	size_t len;
	for (len = 0; len < ARR_LEN(label); len++) {
		label[len] = fgetc(xbee_in);
		if (label[len] == '\0') break;
	}

	// If a label that is longer than what is allowed, drop the rest.
	while(xbee_has_data() && len >= ARR_LEN(label)) fgetc(xbee_in);

	const uint16_t id = REQUEST_OFFSET + REQUEST_INSERT_LABEL;
	log_append((uint8_t*)&id, sizeof(id));
	log_append(label, len);
	return 0;
}

int xbee_check_request(void) {
	if (!xbee_has_data()) return 0;

	const enum data_request r = (int)fgetc(xbee_in);

	int rc;
	switch (r) {
		case REQUEST_LOG:           rc = request_log();          break;
		case REQUEST_NUM_LOGS:      rc = request_num_logs();     break;
		case REQUEST_INSERT_LABEL:  rc = request_insert_label(); break;

		case REQUEST_NONE:
		default:
			rc = 0; break;
	}

	xbee_flush();
	return rc;
}
