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
#include <m41t81s_rtc.h>  // for rtc_time, rtc_get_time, rtc_init
#include <stdint.h>       // for uint8_t, int16_t
#include <stdio.h>        // for printf
#include <usart.h>        // for usart1_init
#include <util/delay.h>

#include "utils.h"        // for ARR_LEN

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	rtc_init();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();

	while (1) {
		int16_t rc;

		struct rtc_time t;
		if (!((rc = rtc_get_time(&t)) < 0)) {
			printf("Got time: ");
		} else {
			printf("failed to get fix rc: %x ", rc);
		}
		printf("%u/%u/%u %u:%u:%u.%u         \n",
				t.year, t.month, t.day_of_month,
				t.hours, t.minutes, t.seconds, t.hundredth_seconds);

		_delay_ms(518);
	}

    return 0;
}
