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

#ifndef XBEE_H
#define XBEE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


#define XBEE_PAYLOAD_LEN	(63)

enum xbee_packet_type {
	HANDSHAKE = 0,
	ACK = 1,
	NACK = 1,
	RESEND = 1,
	REQUEST = 2,
	RESPONCE = 2,
	LIVE_STREAM = 3,
};

enum xbee_flags {
	WRONG_START_SEQ,
	WRONG_CHECKSUM,
	INVALID_LENGTH,
	INVALID_TYPE,

	N_XBEE_FLAGS,
};

struct xbee_packet {
	uint8_t buf[XBEE_PAYLOAD_LEN];
	uint8_t len : 6;
	enum xbee_packet_type type : 2;
};

void xbee_init(void);
void xbee_send_ACK(void);
void xbee_send_NACK(void);
void xbee_send_RESEND(void);
void xbee_send_packet(struct xbee_packet *p);
bool xbee_read_packet(struct xbee_packet *p);
void xbee_set_flag_callback(void(*func)(enum xbee_flags));
bool xbee_packet_append(struct xbee_packet *p, uint8_t *buf, size_t len);


#endif /* XBEE_H */
