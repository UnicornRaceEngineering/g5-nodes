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
#include <io.h>      // for io_pinmode_t::OUTPUT, SET_PIN_MODE, etc
#include <stdint.h>  // for uint8_t
#include <stdio.h>   // for printf
#include <usart.h>   // for usart1_init
#include <util/delay.h>

#include "utils.h"   // for ARR_LEN

#define NUM_PINS	(8)

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();

	SET_PIN_MODE(PORTF, PIN0, OUTPUT);
	SET_PIN_MODE(PORTF, PIN1, OUTPUT);
	SET_PIN_MODE(PORTF, PIN2, OUTPUT);
	SET_PIN_MODE(PORTF, PIN3, OUTPUT);
	SET_PIN_MODE(PORTF, PIN4, OUTPUT);
	SET_PIN_MODE(PORTF, PIN5, OUTPUT);
	SET_PIN_MODE(PORTF, PIN6, OUTPUT);
	SET_PIN_MODE(PORTF, PIN7, OUTPUT);

	DIGITAL_WRITE(PORTF, PIN0, HIGH); 	// takes 6 instruktions
	IO_SET_HIGH(PORTF, PIN1); 			// takes 1 instruktion
	DIGITAL_WRITE(PORTF, PIN2, HIGH);
	DIGITAL_WRITE(PORTF, PIN3, HIGH);
	DIGITAL_WRITE(PORTF, PIN4, HIGH);
	DIGITAL_WRITE(PORTF, PIN5, HIGH);
	DIGITAL_WRITE(PORTF, PIN6, HIGH);
	IO_SET_HIGH(PORTF, PIN7);

	int i = 0;
	while(1){
		// Main work loop
		_delay_ms(250); // 1 second

		DIGITAL_TOGGLE(PORTF, i); // This expands to 6 instruktions

		int pin;
		for (pin = 0; pin < NUM_PINS; ++pin){
			int pinVal = DIGITAL_READ(PORTF, pin);
			printf("pin %d is logical %d\n", pin, pinVal);
		}
		printf("\n");

		if(++i == NUM_PINS){
			i = 0;
		}
	}

    return 0;
}
