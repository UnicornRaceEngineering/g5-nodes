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

#include <can.h>
#include <string.h>
#include <heap.h>
#include <can_transport.h>
#include <usart.h>

int main(void) {
	usart1_init(115200);
	init_heap();
	init_can_node(STEERING_NODE);

	sei();                                      //Enable interrupt

	usart1_printf("\n\n\nSTARTING\n");

	_delay_ms(5000);
	while (1) {
#if 0
		uint8_t *storage = (uint8_t*)malloc_(27);
		char str[27] = "HAS anyone really been far\n";
		strncpy((char*)&storage[0], str, 27);
		can_broadcast(PADDLE_STATUS, storage);
		_delay_ms(1000);
#else
		while(get_queue_length()) {
			struct can_message *message = read_inbox();
			usart1_printf("message of id %d and length %d : ", message->info.id, message->info.len);
			for (int i = 0; i < message->info.len; ++i)
				usart1_putc(message->data[i]);
			can_free(message);
		}
		usart1_putc('\n');
		_delay_ms(1000);
#endif
	}

	return 0;
}
