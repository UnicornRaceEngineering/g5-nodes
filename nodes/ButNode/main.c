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
 #include <io.h>                           // for IO_SET_HIGH, IO_SET_LOW, etc
 #include <stdbool.h>                      // for bool, false, true
 #include <stddef.h>                       // for size_t
 #include <stdint.h>                       // for uint8_t, uint16_t, etc
 #include <stdio.h>                        // for printf
 #include <usart.h>                        // for usart1_init
 #include <util/delay.h>
 #include <utils.h>                        // for ARR_LEN
 #include <can.h>
 #include <sysclock.h>  // for get_tick


static uint8_t buf_in[64];
static uint8_t buf_out[256];


static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	can_init();
	sysclock_init();

	SET_PIN_MODE(PORTE, PIN5, INPUT);
	SET_PIN_MODE(PORTE, PIN6, INPUT);

	sei();
	puts_P(PSTR("Init complete\n\n"));

//	// Generate synchronous interrupt on rising edge
//	EICRB |= ((1 << ISC51) | (1 << ISC50));
//	BIT_SET(EIMSK, INT5); // Interrupt enable Paddle up
//
//	// Generate synchronous interrupt on rising edge
//	EICRB |= ((1 << ISC61) | (1 << ISC60));
//	BIT_SET(EIMSK, INT6); // Interrupt enable Paddle up
}


int main(void) {
	init();

	int button = 0;
	while (1) {
		if (DIGITAL_READ(PORTE, PIN5) == LOW) {
			button = 5;
			can_broadcast(BUTTON, (void*)&button);
			_delay_ms(5);
		}

		if (DIGITAL_READ(PORTE, PIN6) == LOW) {
			button = 6;
			can_broadcast(BUTTON, (void*)&button);
			_delay_ms(5);
		}
	}

	return 0;
}