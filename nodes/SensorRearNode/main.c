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
#include <io.h>               // for io_pinmode_t::INPUT, SET_PIN_MODE
#include <stdint.h>           // for uint8_t, uint32_t, uint16_t
#include <stdio.h>            // for printf
#include <sysclock.h>         // for get_tick, sysclock_init
#include <usart.h>            // for usart1_init
#include <util/delay.h>
#include <utils.h>            // for ARR_LEN, BIT_SET
#include <stdbool.h>
#include <can.h>
#include "system_messages.h"  // for message_id, etc
#include <adc.h>


static uint8_t buf_in[64];
static uint8_t buf_out[64];


void setup_thermistor(const uint8_t channel);
float thermistor(const uint16_t rawADC);


static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	sysclock_init();
	can_init();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}


int main(void) {
	init();

	const uint8_t ch1 = 5, ch2 = 6;
	setup_thermistor(ch1);
	setup_thermistor(ch2);

	while(1) {
		const float ch1_v = thermistor(adc_readChannel(ch1));
		const float ch2_v = thermistor(adc_readChannel(ch2));
		printf("ADC5: %5.3f | ADC6: %5.3f\n", ch1_v, ch2_v);
		_delay_ms(100);
	}

	return 0;
}


void setup_thermistor(const uint8_t channel) {
	DDRF &= ~(1 << channel); // configure PB as an input
	PORTF |= (1 << channel); // enable the pull-up on PB
	adc_init(channel, INTERNAL, ADC_PRESCALAR_16);
}


float thermistor(const uint16_t rawADC) {
	float temp;
	temp = log(10000.0 * ((1024.0 / rawADC - 1)));
	temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp)) * temp);
	temp = temp - 273.15;            // Convert Kelvin to Celcius
	//temp = (temp * 9.0) / 5.0 + 32.0; // Convert Celcius to Fahrenheit
	return temp;
}
