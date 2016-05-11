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
#include "send_file.h"


/*
TODO: Timeout niveauer.
Timeout niveau start at 0 cuts at 5.
Every niv prolong with 100ms. start at 100ms.
react to both timeout and NACK.
*/

enum request_type {
	REQUEST_FILE,
	NONE,
};


static bool livestream(void);
static bool handle_packet(void);
static void respond_to_handshake(void);
static void respond_to_request(struct xbee_packet *p);
static void handle_ack(struct xbee_packet *p);
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
		livestream();

		/* Delay so xbee can keep up */
		/* TODO: It seems resonable from tests that this break should equal to
		adding about 200us delay after sending every byte. */
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
	const bool ack = p->buf[0];
	if (ack == true) {
		switch (ongoing_request) {
		case REQUEST_FILE:
			eval_send_file_status();
			break;
		case NONE:
			/* Do nothing. */
			break;
		}
	} else {
		switch (ongoing_request) {
		case REQUEST_FILE:
			resend_send_file();
			break;
		case NONE:
			/* Do nothing. */
			break;
		}
	}
}


static void respond_to_request(struct xbee_packet *p) {
	if (ongoing_request != NONE) {
		xbee_send_NACK();
	}

	int ret = -1;
	enum request_type type = p->buf[0];
	switch (type) {
	case REQUEST_FILE:
		ret = initiate_send_file(p);
		break;
	default:
		flag(INVALID_REQ_TYPE);
		xbee_send_NACK();
		return;
	}

	if (!ret) {
		ongoing_request = type;
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
			const uint8_t transport = get_msg_transport(tx_id);

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
