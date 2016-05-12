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
#include <stdio.h>   // for fputc
#include <usart.h>   // for usart1_init, usart1_byte_output
#include <stdbool.h>
#include <string.h>

#include "xbee.h"

#define XBEE_BAUD 	(115200)


static void flag_do_nothing(enum xbee_flags flag);

static FILE *xbee_out = &usart1_byte_output;
static FILE *xbee_in = &usart1_io;

static uint8_t buf_in[128];
static uint8_t buf_out[128];

static void (*flag)(enum xbee_flags) = flag_do_nothing;


static void flag_do_nothing(enum xbee_flags flag) {
	(void)flag;
}


void xbee_set_flag_callback(void(*func)(enum xbee_flags)) {
	flag = func;
}


void xbee_init(void) {
	usart1_init(XBEE_BAUD, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
}


void xbee_send_ACK(void) {
	struct xbee_packet p = { .buf = {1}, .len = 1, .type = ACK};
	xbee_send_packet(&p);
}


void xbee_send_NACK(void) {
	struct xbee_packet p = { .buf = {0}, .len = 1, .type = NACK};
	xbee_send_packet(&p);
}


void xbee_send_RESEND(void) {
	struct xbee_packet p = { .buf = {2}, .len = 1, .type = RESEND};
	xbee_send_packet(&p);
}


bool xbee_packet_append(struct xbee_packet *p, uint8_t *buf, size_t len) {
	if (p->len + len > XBEE_PAYLOAD_LEN) {
		return false;
	}

	memcpy(p->buf + p->len, buf, len);
	p->len += len;
	return true;
}


void xbee_send_packet(struct xbee_packet *p) {
	/* Send start sequence */
	fputc(0xA1, xbee_out);

	/* Send header */
	const uint8_t header = (p->type << 6) | (0x3F & p->len);
	fputc(header, xbee_out);

	/* Send payload and calculate checksum */
	uint8_t checksum = 0;
	for (size_t i = 0; i < p->len; ++i) {
		fputc(p->buf[i], xbee_out);
		checksum ^= p->buf[i];
	}

	/* Send checksum */
	fputc(checksum, xbee_out);
}


bool xbee_read_packet(struct xbee_packet *p) {
	/* Return false if nothing has been recieved */
	/* Note that we don't wait for the complete packet to arrive */
	if (!usart1_has_data()) {
		return false;
	}

	/* Check if start sequence (single byte) is correct */
	const uint8_t start_seq = (uint8_t)fgetc(xbee_in);
	if (start_seq != 0xA1) {
		flag(WRONG_START_SEQ);
		return false;
	}

	/* Retrieve the header from the next byte */
	const uint8_t header = (uint8_t)fgetc(xbee_in);
	p->type = BITMASK_CHECK(0xC0, header) >> 6; //Check 2 most significant bits.
	p->len = BITMASK_CHECK(0x3F, header); //Check remaining 6 lowest significant bits.

	/* Read payload and calculate checksum */
	uint8_t payload_checksum = 0;
	for (uint8_t i = 0; i < p->len; ++i) {
		p->buf[i] = (uint8_t)fgetc(xbee_in);
		payload_checksum ^= p->buf[i];
	}

	/* Check if the calculated checksum is a match with the real one. */
	const uint8_t checksum = (uint8_t)fgetc(xbee_in);
	if (checksum != payload_checksum){
		flag(WRONG_CHECKSUM);
		xbee_send_RESEND();
		return false;
	}

	/* We use the type and length to check packets for their validity. */
	switch (p->type) {
	case HANDSHAKE:
		break;
	case ACK|NACK|RESEND:
		if (p->len != 1) {
			/* Packets of these type can only have length 1. */
			flag(INVALID_LENGTH);
			xbee_send_NACK();
			return false;
		}
		break;
	case REQUEST|RESPONCE:
		if (!p->len) {
			/* An empty request is invalid. */
			flag(INVALID_LENGTH);
			xbee_send_NACK();
			return false;
		}
		break;
	case LIVE_STREAM:
		/* We do not expect to encounter this type at all. */
		flag(INVALID_TYPE);
		xbee_send_NACK();
		return false;
	}

	/* Return true if message retrievel is succesful. */
	return true;
}
