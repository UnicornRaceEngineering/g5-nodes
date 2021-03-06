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
#include "protocol.h"


static void send_size_responce(uint32_t size);


static uint32_t bytes_left;
static uint32_t bytes_sent;
static FIL file;


bool initiate_send_file(struct xbee_packet *p) {
	if (p->len < 3) {
		send_size_responce(0);
		return false;
	}
	uint16_t log_nr;
	memcpy(&log_nr, &p->buf[1], sizeof(log_nr));

	if (open_file(&file, log_nr, FA_READ|FA_OPEN_EXISTING)) {
		if (!file_seek(&file, 0)) {
			f_close(&file);
			send_size_responce(0);
			return false;
		}

		/* Get file size and send it in the first packet */
		bytes_left = size_of_file(&file);
		if (!bytes_left) {
			send_size_responce(0);
			return false;
		}

		bytes_sent = 0;
		set_ongoing_request(REQUEST_FILE);

		send_size_responce(bytes_left);
		return true;
	} else {
		send_size_responce(0);
		return false;
	}
}


static void send_size_responce(uint32_t size) {
	struct xbee_packet p = xbee_create_packet(RESPONCE);
	xbee_packet_append(&p, (uint8_t*)&size, sizeof(size));
	xbee_send_packet(&p);
}


void continue_send_file(void) {
	const uint8_t len = bytes_left > XBEE_PAYLOAD_LEN ? XBEE_PAYLOAD_LEN : bytes_left;
	if(!len) {
		struct xbee_packet p = xbee_create_packet(RESPONCE);
		xbee_send_packet(&p);
		f_close(&file);
		set_ongoing_request(NONE);
		return;
	} else {
		struct xbee_packet p = { .len = len, .type = RESPONCE, };
		read_file(&file, p.buf, len);
		xbee_send_packet(&p);

		bytes_sent += len;
	}
}


void eval_send_file_status(void) {
	if (bytes_sent) {
		const uint8_t len = bytes_left > XBEE_PAYLOAD_LEN ? XBEE_PAYLOAD_LEN : bytes_left;
		bytes_left -= len;
	}

	continue_send_file();
}


void resend_send_file(void) {
	if (bytes_sent) {
		continue_send_file();
	} else {
		struct xbee_packet p = { .len = sizeof(bytes_left), .type = RESPONCE, };
		memcpy(p.buf, &bytes_left, sizeof(bytes_left));
		xbee_send_packet(&p);
	}
}
