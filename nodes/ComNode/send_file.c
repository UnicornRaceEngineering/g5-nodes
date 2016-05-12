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
#include <stdbool.h>
#include <string.h>

#include "send_file.h"
#include "log.h"
#include "xbee.h"
#include "flags.h"


struct t_request_file {
	uint32_t bytes_left;
	uint32_t bytes_sent;
	FIL file;
} r;


enum req_file_flags initiate_send_file(struct xbee_packet *p) {
	if (p->len < 3) {
		xbee_send_NACK();
		return MISSING_LOGNR;
	}
	uint16_t log_nr;
	memcpy(&log_nr, &p->buf[1], sizeof(log_nr));

	if (open_file(&r.file, log_nr, FA_READ|FA_OPEN_EXISTING)) {
		if (!file_seek(&r.file, 0)) {
			f_close(&r.file);
			xbee_send_NACK();
			return FILE_ACCES_ERR;
		}

		/* Get file size and send it in the first packet */
		r.bytes_left = size_of_file(&r.file);
		if (!r.bytes_left) {
			xbee_send_NACK();
			return EMPTY_LOG_FILE;
		}

		r.bytes_sent = 0;
		/* Acknolegde request */
		xbee_send_ACK();

		struct xbee_packet p = xbee_create_packet(RESPONCE);
		xbee_packet_append(&p, (uint8_t*)&r.bytes_left, sizeof(r.bytes_left));
		xbee_send_packet(&p);
		return REQUEST_ACTIVE;
	} else {
		xbee_send_NACK();
		return FILE_ACCES_ERR;
	}
}


enum req_file_flags continue_send_file(void) {
	const uint8_t len = r.bytes_left > XBEE_PAYLOAD_LEN ? XBEE_PAYLOAD_LEN : r.bytes_left;
	if(!len) {
		struct xbee_packet p = xbee_create_packet(RESPONCE);
		xbee_send_packet(&p);
		f_close(&r.file);
		return FINISHED_REQUEST;
	}

	struct xbee_packet p = { .len = len, .type = RESPONCE, };
	read_file(&r.file, p.buf, len);
	xbee_send_packet(&p);

	r.bytes_sent += len;
	return REQUEST_ACTIVE;
}


void eval_send_file_status(void) {
	if (r.bytes_sent) {
		const uint8_t len = r.bytes_left > XBEE_PAYLOAD_LEN ? XBEE_PAYLOAD_LEN : r.bytes_left;
		r.bytes_left -= len;
	}

	continue_send_file();
}


void resend_send_file(void) {
	if (r.bytes_sent) {
		continue_send_file();
	} else {
		struct xbee_packet p = { .len = sizeof(r.bytes_left), .type = RESPONCE, };
		memcpy(p.buf, &r.bytes_left, sizeof(r.bytes_left));
		xbee_send_packet(&p);
	}
}
