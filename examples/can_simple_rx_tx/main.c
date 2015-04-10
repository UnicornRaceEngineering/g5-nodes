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
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
#include <can.h>
#include <usart.h>
#include <heap.h>


static void rx_complete(uint16_t id, uint16_t len, uint8_t *msg);

static void init(void) {
	usart1_init(115200);
	init_heap();
	can_init(1);
	set_canrec_callback(rx_complete);
	
	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();

#if 0 // If sending messages
	uint8_t *storage1 = (uint8_t*)malloc_(28);
	char str1[28] = "Has anyone really been far\n";
	strncpy((char*)&storage1[0], str1, 28);

	uint8_t *storage2 = (uint8_t*)malloc_(29);
	char str2[29] = "even as decided to use even\n";
	strncpy((char*)&storage2[0], str2, 29);

	uint8_t *storage3 = (uint8_t*)malloc_(33);
	char str3[33] = "go want to do look more like?\n\n";
	strncpy((char*)&storage3[0], str3, 33);
#endif

	_delay_ms(2000);
	while(1) {
#if 0 // If sending messages
		can_send(1, 28, &storage1[0]);
		can_send(2, 29, &storage2[0]);
		can_send(3, 33, &storage3[0]);
#endif

		// printf("There is %d bytes allocated.\n", count_heap());
		_delay_ms(1000);

	}
    return 0;
}

// Callback to be run when rx comletes on the CAN
static void rx_complete(uint16_t id, uint16_t len, uint8_t *msg) {
	printf("id: %d len: %d\n", id, len);
	// for (uint8_t i = 0; i < len; i++) putchar(msg[i]);
}
