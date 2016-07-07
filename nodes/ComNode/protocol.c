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

#include "protocol.h"
#include "xbee.h"
#include "ecu.h"
#include "log.h"
#include "flags.h"
#include "send_file.h"


static bool livestream(void);
static bool handle_packet(void);
static void respond_to_handshake(void);
static bool respond_to_request(struct xbee_packet *p);
static void handle_ack(const bool ack);
static void flag_do_nothing(enum state_flags flag);
static void reset_xbee_timeout(void);
static void inc_xbee_timeout(void);

static void (*flag)(enum state_flags) = flag_do_nothing;
static enum request_type ongoing_request;
static bool streaming;
static uint32_t tick;
static uint32_t ecu_timeout;
static uint32_t xbee_timeout;
static uint32_t timeout_inc = 100;


void event_loop(void) {
	streaming = false;
	set_ongoing_request(NONE);

	ecu_send_request();

	ecu_timeout = get_tick() + 300;

	/* Main work loop */
	while(1){
		tick = get_tick();
		livestream();
		handle_packet();

		if (ongoing_request != NONE) {
			if (tick > xbee_timeout) {
				if (timeout_inc == 600) {
					/*	5 retries have now been executed without a responce.
						So we drop the ongoing request. */
					ongoing_request = NONE;
				} else {
					handle_ack(false);
					inc_xbee_timeout();
				}
			}
		}
	}
}


void inc_xbee_timeout(void) {
	timeout_inc += 100;
	xbee_timeout = tick + timeout_inc;
}


void reset_xbee_timeout(void) {
	timeout_inc = 100;
	xbee_timeout = tick + timeout_inc;
}


void set_ongoing_request(enum request_type type) {
	ongoing_request = type;
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
		/* Acknolegde request */
		switch (p.type) {
		case HANDSHAKE:
			respond_to_handshake();
			break;
		case ACK:
			handle_ack(p.buf[0]);
			reset_xbee_timeout();
			break;
		case REQUEST:
			if (respond_to_request(&p)) {
				reset_xbee_timeout();
			}
			break;
		}
		return true;
	}

	return false;
}


static void respond_to_handshake(void) {
	xbee_send_ACK();
	streaming = true;
}


static void handle_ack(const bool ack) {
	if (ack == true) {
		switch (ongoing_request) {
		case REQUEST_FILE:
			eval_send_file_status();
			break;
		case NUM_LOG:
			/* TODO */
		case NONE:
			/* Do nothing. */
			break;
		}
	} else {
		switch (ongoing_request) {
		case REQUEST_FILE:
			resend_send_file();
			break;
		case NUM_LOG:
			/* TODO */
		case NONE:
			/* Do nothing. */
			break;
		}
	}
}


static bool respond_to_request(struct xbee_packet *p) {
	if (ongoing_request != NONE) {
		xbee_send_NACK();
	} else {
		xbee_send_ACK();
	}

	enum request_type type = p->buf[0];
	switch (type) {
	case REQUEST_FILE:
		return initiate_send_file(p);
	case NUM_LOG:
		/* TODO */
		return false;
	default:
		flag(INVALID_REQ_TYPE);
		xbee_send_NACK();
		return false;
	}
}


static bool livestream(void) {
	if (ecu_has_packet()) {
		ecu_send_request();

		uint16_t id = 52; // Old systime ID
		log_append(&id, sizeof(id));
		log_append(&tick, sizeof(tick));

		struct xbee_packet p = xbee_create_packet(LIVE_STREAM);
		while (1) {
			struct sensor data;
			if (!ecu_read_data(&data)) {
				break;
			}

			uint16_t tx_id = data.id;
			const uint8_t transport = get_msg_transport(tx_id);

			if (transport & XBEE) {
				/* WARNING: If lacking space in the packet, there is a chance
				that the tx_id will be appended, but not it's associated data point */
				xbee_packet_append(&p, (uint8_t*)&tx_id, sizeof(tx_id));
				xbee_packet_append(&p, (uint8_t*)&data.value, sizeof(data.value));
			}

			if (transport & SD) {
				log_append(&tx_id, sizeof(tx_id));
				log_append(&data.value, sizeof(data.value));
			}
		}
		if (streaming) {
			xbee_send_packet(&p);
		}
		ecu_timeout = tick + 300;
		return true;
	} else {
		if (tick > ecu_timeout) {
			ecu_init();
			ecu_send_request();
			ecu_timeout = tick + 300;
		}
		return false;
	}
}
