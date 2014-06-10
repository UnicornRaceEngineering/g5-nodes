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

#include <stdlib.h> // size_t
#include <avr/interrupt.h> // sei()
#include <usart.h>
#include <util/delay.h>

#define XBEE_BAUD 	(115200)
#define ECU_BAUD	(19200)

// rapidly firring this causes the ECU to crash. Therefor we need to call this
// periodically.
static void request_ecu(void) {
	const uint8_t start_seq[10] = {0x12,0x34,0x56,0x78,0x17,0x08,0,0,0,0};
	for (int i = 0; i < 10; ++i) {
		usart0_putc(start_seq[i]);
	}
}

int main(void) {
	usart1_init(XBEE_BAUD); // xbee
	usart0_init(ECU_BAUD);	// ECU

	sei();										//Enable interrupt

	usart1_puts("\nStarting:\n");

	while(1){
		// Main work loop

		usart1_puts("requesting data\n");
		request_ecu();
		_delay_ms(10);

		for (int i = 0; i < 115; ++i) {
			//usart1_printf("[%d: 0x%x], ",i, usart0_getc());
			usart1_printf("[%d: %d], ", i, usart0_getc());
		}
		usart1_putc('\n');

	}

    return 0;
}

