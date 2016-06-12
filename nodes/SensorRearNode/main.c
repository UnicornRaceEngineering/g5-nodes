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


void setup_wheelsensor(const uint8_t ch1, const uint8_t ch2);
double Thermistor(int RawADC);


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
	setup_wheelsensor(ch1, ch2);

	while(1) {
		const float ch1_v = Thermistor(adc_readChannel(ch1));
		const float ch2_v = Thermistor(adc_readChannel(ch2));
		printf("ADC5: %5.3f | ADC6: %5.3f\n", ch1_v, ch2_v);
		_delay_ms(100);
	}

	return 0;
}


void setup_wheelsensor(const uint8_t ch1, const uint8_t ch2) {
	DDRF &= ~(1<<ch1); // configure PB as an input
	PORTF |= (1<<ch1); // enable the pull-up on PB
	//channel ch1, internal vref, ADC_PRESCALAR_16
	adc_init(ch1, INTERNAL, ADC_PRESCALAR_16);

	DDRF &= ~(1<<ch2); // configure PB as an input
	PORTF |= (1<<ch2); // enable the pull-up on PB
	//channel ch2, internal vref, ADC_PRESCALAR_16
	adc_init(ch2, INTERNAL, ADC_PRESCALAR_16);
}


double Thermistor(int RawADC) {
	double Temp;
	Temp = log(10000.0*((1024.0/RawADC-1)));
	//         =log(10000.0/(1024.0/RawADC-1)) // for pull-up configuration
	Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
	Temp = Temp - 273.15;            // Convert Kelvin to Celcius
	Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
	return Temp;
}
