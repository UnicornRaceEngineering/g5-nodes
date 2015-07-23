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

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <usart.h>
#include <can_transport.h>


void read_message(struct can_message *message);


static uint16_t msg_num;


static void init(void) {
	usart1_init(115200);
	init_can_node(COM_NODE);

	msg_num = 0;

	sei();
	puts_P(PSTR("Init complete\n\n"));
}


int main(void) {
	init();

	while (1) {
		while(get_queue_length()) {
			struct can_message *message = read_inbox();
			read_message(message);
			can_free(message);
		}
	}

	return 0;
}


void read_message(struct can_message *msg) {
	const uint8_t len = MESSAGE_INFO(msg->index).len;
	const uint16_t id  = MESSAGE_INFO(msg->index).id;

	float fdata = 0;
	if (len >= 4) {
		fdata = (float)*(float*)msg->data;
	}

	printf("MSG:%4d  Got id: %4d and length: %1d\t", msg_num++, id, len);
	for (uint8_t i = 0; i < len; ++i) {
		printf("%3d; 0x%02x  |  ", msg->data[i], msg->data[i]);
	}

	if (len >= 4) {
		printf("\tas float: %10.3f", (double)fdata);
	}
	printf("\n");
}
