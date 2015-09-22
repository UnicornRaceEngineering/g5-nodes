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

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>           // for uint8_t, uint16_t
#include <stdio.h>            // for printf
#include <usart.h>            // for usart1_init
#include <stdbool.h>
#include <can.h>
#include <event_manager.h>
#include <sysclock.h>
#include "system_messages.h"  // for MESSAGE_INFO, message_detail, etc
#include "utils.h"            // for ARR_LEN


void read_msg(const uint8_t load);

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static uint16_t msg_num = 0;


static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	can_init();

	can_subscribe_all();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}


int main(void) {
	init();

	while (1) {
		uint32_t tick = get_tick();
		uint8_t event = 0;

		const uint8_t load = event_manager(&event, tick);

		switch (event) {
			case E_CAN_REC: read_msg(load); break;
			default: break;
		}
	}

	return 0;
}


void read_msg(const uint8_t load) {
	while(can_has_data()) {
		struct can_message msg;
		read_message(&msg);
		printf("Load:%3u | MSG:%5u | recieved: %4u | error: %4u |  Got id: %4u and length: %1u\t",
			load, msg_num++, get_counter(RX_COMP), get_counter(TOTAL_ERR), msg.id, msg.len);
		for (uint8_t i = 0; i < msg.len; ++i) {
			printf("%3d; 0x%02x  |  ", msg.data[i], msg.data[i]);
		}
		printf("\n");
	}
}
