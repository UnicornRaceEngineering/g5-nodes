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
#include <can_transport.h>    // for can_broadcast, can_cleanup, etc
#include <heap.h>             // for smalloc
#include <stdint.h>           // for uint8_t
#include <stdio.h>            // for printf
#include <string.h>           // for strncpy
#include <sysclock.h>         // for sysclock_init
#include <usart.h>            // for usart1_init
#include <util/delay.h>

#include "system_messages.h"  // for message_id::TRANSPORT_TEST_LONG, etc
#include "utils.h"            // for ARR_LEN

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static void init(void) {
	sysclock_init();
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	init_can_node(STEERING_NODE);

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();

	while (1) {
#if 1
		uint8_t err = 0;
		// Sending a short message (1 frame)
		uint8_t msg[6] = {'H', 'e', 'l', 'l', 'o', '\n'};
		err = can_broadcast(TRANSPORT_TEST_SHORT, msg);
		if (err) printf("err: %d\n",  err);
		_delay_ms(100);

		// Sending a long message (4 frames)
		uint8_t *storage = (uint8_t*)smalloc(27);
		if (storage) {
			char str[27] = "HAS anyone really been far\n";
			strncpy((char*)&storage[0], str, 27);
			err = can_broadcast(TRANSPORT_TEST_LONG, storage);
			if (err)
				printf("err: %d\n",  err);
		}
		_delay_ms(1000);
#else
		// recieving
		while(get_queue_length()) {
			struct can_message *message = read_inbox();
			printf("message of id %4d and length %3d : ", MESSAGE_INFO(message->index).id, MESSAGE_INFO(message->index).len);
			for (int i = 0; i < MESSAGE_INFO(message->index).len; ++i)
				putchar(message->data[i]);
			can_free(message);
		}
		putchar('\n');
		_delay_ms(1000);
#endif
		can_cleanup();
	}

	return 0;
}
