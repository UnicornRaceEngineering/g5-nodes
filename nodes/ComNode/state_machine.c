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
#include <util/delay.h>
#include <stdio.h>
#include <utils.h>
#include <string.h>
#include <system_messages.h>
#include <sysclock.h>

#include "state_machine.h"
#include "xbee.h"
#include "ecu.h"
#include "log.h"
#include "flags.h"


enum request_type {
	REQUEST_FILE,
	NONE,
};


static bool livestream(void);
static bool handle_packet(void);
static void respond_to_handshake(void);
static void respond_to_request(struct xbee_packet *p);
static void handle_ack(struct xbee_packet *p);
static void send_file(struct xbee_packet *p);
static void flag_do_nothing(enum state_flags flag);

static void (*flag)(enum state_flags) = flag_do_nothing;
static enum request_type ongoing_request;
static bool streaming;
static FIL logfile;
static uint32_t tick;


void state_machine(void) {
	streaming = false;
	ongoing_request = NONE;

	create_file(&logfile);
	ecu_send_request();

	/* Main work loop */
	while(1){
		tick = get_tick();
		(void)tick;
		handle_packet();
		xbee_send_ACK();
		//livestream();
		_delay_ms(15);
	}
}


void state_set_flag_callback(void(*func)(enum state_flags)) {
	flag = func;
}


static void flag_do_nothing(enum state_flags flag) {
	(void)flag;
}


static bool handle_packet(void) {
	struct xbee_packet p;
	if(xbee_read_packet(&p)) {
		switch (p.type) {
			case HANDSHAKE:		respond_to_handshake();	break;
			case ACK:			handle_ack(&p);			break;
			case REQUEST:		respond_to_request(&p);	break;
		}
		return true;
	}

	return false;
}


static void respond_to_handshake(void) {
	xbee_send_ACK();
	streaming = true;
}


static void handle_ack(struct xbee_packet *p) {
	//const bool ack = p->buf[0];
	if (ongoing_request == REQUEST_FILE) {send_file(p);}
}


static void respond_to_request(struct xbee_packet *p) {
	if (ongoing_request != NONE) {
		xbee_send_NACK();
	}

	enum request_type type = p->buf[0];
	switch (type) {
	case REQUEST_FILE:
		send_file(p);
		break;
	default:
		flag(INVALID_REQ_TYPE);
		xbee_send_NACK();
		break;
	}
}


//static struct t_request_file {
//	uint32_t bytes_left = 0;
//	uint32_t bytes_sent = 0;
//	FIL file;
//}


static void send_file(struct xbee_packet *p) {
	static uint32_t bytes_left = 0;
	static FIL file;

	if (ongoing_request == NONE) {
		if (p->len < 3) {
			flag(MISSING_LOGNR);
			xbee_send_NACK();
			return;
		}
		uint16_t log_nr;
		memcpy(&log_nr, &p->buf[1], sizeof(log_nr));
		if (open_file(&file, log_nr, FA_READ|FA_OPEN_EXISTING)) {
			if (!file_seek(&file, 0)) {
				f_close(&file);
				xbee_send_NACK();
				return;
			}

			ongoing_request = REQUEST_FILE;
			/* Acknolegde request */
			xbee_send_ACK();

			/* Get file size and send it in the first packet */
			bytes_left = size_of_file(&file);
			struct xbee_packet p = { .len = sizeof(bytes_left), .type = RESPONCE, };
			memcpy(p.buf, &bytes_left, sizeof(bytes_left));
			xbee_send_packet(&p);
			return;
		} else {
			ongoing_request = NONE;
			xbee_send_NACK();
			return;
		}
	}

	if (ongoing_request == REQUEST_FILE) {
		const uint8_t len = bytes_left > XBEE_PAYLOAD_LEN ? XBEE_PAYLOAD_LEN : bytes_left;
		if(!len) {
			ongoing_request = NONE;
			struct xbee_packet p = { .len = 0, .type = RESPONCE, };
			xbee_send_packet(&p);
			f_close(&file);
			return;
		}

		struct xbee_packet p = { .len = len, .type = RESPONCE, };
		read_file(&file, p.buf, len);
		xbee_send_packet(&p);

		/* TODO: bytes_left should not be counted down before we get an ACK. */
		bytes_left -= len;
	}
}


static bool livestream(void) {
	if (ecu_has_packet()) {
		struct xbee_packet p = { .len = 0, .type = LIVE_STREAM, };
		while (1) {
			struct sensor data;
			if (!ecu_read_data(&data)) {
				break;
			}

			const uint16_t tx_id = data.id;
			enum medium transport = can_msg_transport(tx_id);

			/* TODO: This should not be part of a final solution. */
			if (p.len + sizeof(tx_id) + sizeof(data.value)) {
				xbee_send_packet(&p);
				p.len = 0;
			}

			if (transport & XBEE) {
				memcpy(p.len + p.buf, &tx_id, sizeof(tx_id));
				memcpy(p.len + p.buf + sizeof(tx_id), &data.value, sizeof(data.value));
				p.len = sizeof(tx_id) + sizeof(data.value);
			}

			if (transport & SD) {
				const size_t buflen = sizeof(tx_id) + sizeof(data.value);
				uint8_t buf[buflen];
				memcpy(buf, &tx_id, sizeof(tx_id));
				memcpy(buf + sizeof(tx_id), &data.value, sizeof(data.value));
				file_write(&logfile, buf, buflen);
			}
		}
		ecu_send_request();
		f_sync(&logfile);
		if (streaming) {
			xbee_send_packet(&p);
		}
		return true;
	}

	return false;
}
