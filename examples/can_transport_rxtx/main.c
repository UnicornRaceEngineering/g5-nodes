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
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <can_transport.h>
#include <usart.h>

int main(void) {
	struct can_node *node = init_can_node(CAN_TRANSPORT_EXAMPLE_NODE);
	usart1_init(115200);

	sei();                                      //Enable interrupt

	usart1_printf("\n\n\nSTARTING\n");

	while (1) {
#if 1
		// Print out the payload of any received message we are subscribed to
		for (size_t n_msg = 0; n_msg < node->n_msg_subscriped; ++n_msg) {
			if (node->msg_subscriped[n_msg].complete) {
				struct can_message *msg = &node->msg_subscriped[n_msg];
				msg->complete = false;

				usart1_printf("recv id %d: ", msg->id);
				for (size_t i = 0; i < msg->payload.len; ++i) {
					usart1_printf("0x%02X ", msg->payload.data[i]);
				}
				usart1_printf("\n");
			}
		}
#else
		// Broadcast dummy data to all messages the node is aware of
		for (size_t n_msg = 0; n_msg < node->n_msg_broadcasting; ++n_msg) {
			struct can_message *msg = &node->msg_broadcasting[n_msg];
			uint8_t payload_buff[msg->payload.len]; // VLA is a c99 feature
			usart1_printf("send id %d: ", msg->id);
			for (size_t i = 0; i < msg->payload.len; ++i) {
				payload_buff[i] = i;
				usart1_printf("0x%02X ", payload_buff[i]);
			}
			usart1_printf("\n");

			can_broadcast(msg->id, (uint8_t *)&payload_buff);
		}
#endif
	}

	return 0;
}
