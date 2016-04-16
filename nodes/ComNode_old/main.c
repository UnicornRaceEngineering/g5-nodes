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

#include <avr/interrupt.h> // sei()
#include <avr/pgmspace.h>
#include <m41t81s_rtc.h>           // for rtc_init
#include <stddef.h>                // for size_t
#include <stdio.h>                 // for puts_p
#include <stdint.h>                // for uint8_t, uint16_t
#include <sysclock.h>              // for sysclock_init
#include <system_messages.h>       // for message_detail, MESSAGE_INFO, etc
#include <util/delay.h>
#include <stdbool.h>
#include <can.h>
#include <string.h>
#include <utils.h>

#include "ecu.h"  // for ecu_init, ecu_parse_package
#include "xbee.h"                  // for xbee_init, xbee_send
#include "log.h"

static void init(void) {
	rtc_init();
	ecu_init();
	xbee_init();
	log_init();
	sysclock_init();
	can_init();

	can_subscribe_all();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();

	while(1){
		// Main work loop

		uint32_t tick_timer = 0;
		const uint32_t tick = get_tick();
		if (tick > tick_timer) {
			uint8_t buf[sizeof(uint16_t) + sizeof(uint32_t)] = {0};
			memcpy(buf, (uint8_t*)&((uint16_t){SYSTIME}), 2);
			memcpy(buf+sizeof(uint16_t), (uint8_t*)&tick, 4);
			xbee_send(buf, ARR_LEN(buf));
			log_append(buf, ARR_LEN(buf));
			tick_timer = tick + 10;
		}

		while (can_has_data()) {
			struct can_message msg;
			read_message(&msg);

			const uint8_t transport = can_msg_transport(msg.id);

			if (transport & SD) {
				log_append(&msg.id, sizeof(msg.id));
				log_append(msg.data, msg.len);
			}

			if (transport & XBEE) {
				uint8_t buf[sizeof(msg.id) + msg.len];
				size_t buf_i = 0;

				for (size_t i = 0; i < sizeof(msg.id); i++) {
					buf[buf_i++] = ((uint8_t*)&msg.id)[i];
				}

				for (size_t i = 0; i < msg.len; i++) {
					buf[buf_i++] = msg.data[i];
				}

				xbee_send(buf, sizeof(msg.id) + msg.len);
			}
		}

		ecu_parse_package();
		xbee_check_request();
	}

    return 0;
}
