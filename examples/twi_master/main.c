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
#include <util/delay.h>
#include <avr/interrupt.h>

#include <usart.h>
#include <twi.h>
#include "m41t81s_rtc.h"
#include <io.h>

#if 0
#define TARGET_TWI_ADDR			(0xD0)
#define TARGET_TWI_INTERNAL_REG	(0x01)
#endif

#define ARR_LEN(arr)	(sizeof(arr) / sizeof(arr[0]))

int main(void) {
	usart0_init(115200);						//Serial communication

	sei();										//Enable interrupt

	twi_init_master();
	//rtc_init();

	usart0_printf("\n\n\nSTARTING\n");

	while (1) {
#if 0
		uint8_t arr[] = {'H', 'E', 'L', 'L', 'O'};
		twi_write_array(TARGET_TWI_ADDR, arr, ARR_LEN(arr));

		twi_read_array(TARGET_TWI_ADDR, TARGET_TWI_INTERNAL_REG, arr,
			ARR_LEN(arr));

		usart1_putn(ARR_LEN(arr), arr); usart1_putc('\n');
#endif
		usart0_printf("Getting time fix\n");

		struct rtc_time t;
		rtc_get_time_fix(&t);

		usart0_printf("s/100: %u, s/10: %u s: %u m %u h %u dow %u dom %u month %u year %u\n",
			t.hundredths_sec, t.tenth_sec, t.seconds, t.minutes, t.hours,
			t.day_of_week, t.day_of_month, t.month, t.year);

		_delay_ms(500);
	}

    return 0;
}

