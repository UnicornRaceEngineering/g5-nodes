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
#include "ecu.h"

#define XBEE_BAUD 	(115200)


int main(void) {
	usart1_init(XBEE_BAUD); // xbee
	ecu_init();

	sei();										//Enable interrupt

	usart1_puts("\nStarting:\n");

	while(1){
		// Main work loop

		// Send sync package
		{
			usart1_putc_unbuffered(0xA1);
			usart1_putc_unbuffered(0xB2);
			usart1_putc_unbuffered(0xC3);
			usart1_putc_unbuffered(0xD4);
			usart1_putc_unbuffered(0xE5);
			usart1_putc_unbuffered(0xF6);
		}
		ecu_parse_package();
	}

    return 0;
}
