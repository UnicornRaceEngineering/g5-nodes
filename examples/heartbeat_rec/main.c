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
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <heap.h>
#include <can_transport.h>
#include <usart.h>
#include "sysclock.h"


void handle_heartbeat(struct can_message *msg);


uint8_t node_state[N_NODES] = {0};
uint32_t last_heartbeat[N_NODES] = {0};


static void init(void) {
	usart1_init(115200);
	init_heap();
	sysclock_init();
	init_can_node(COM_NODE);

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();

	while (1) {
		while (get_queue_length()) {
			struct can_message *msg = read_inbox();
			struct message_detail msg_info = message_info(msg->index);

			uint16_t index = 0;
			do {
				if (message_info(index).id == msg_info.id) {
					break;
				}
			} while(++index < END_OF_LIST);

			switch(index) {
				case HEARTBEAT: handle_heartbeat(msg); break;
				default: break;
			}

			can_free(msg);
		}

		for (uint8_t i = 0; i < N_NODES; ++i) {
			if (node_state[i]) {
				if ((get_tick() - last_heartbeat[i]) > 50) {
					node_state[i] = 0;
					printf("node %d is unreachable\n", i);
				}
			}
		}
	}

	return 0;
}


void handle_heartbeat(struct can_message *msg) {
	uint8_t node_id = msg->data[0];

	last_heartbeat[node_id] = get_tick();
	if (!node_state[node_id]) {
		/* State changed */
		node_state[node_id] = 1;
		printf("node %d is reachable\n", node_id);
	}
}
