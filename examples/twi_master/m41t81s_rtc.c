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
#include <stdbool.h>
#include <util/delay.h>

#include <bitwise.h>
#include <twi.h>
#include <usart.h>
#include "m41t81s_rtc.h"

#define ARR_LEN(arr)	(sizeof(arr) / sizeof(arr[0]))

static uint8_t register_map[NUMBER_OF_REGISTERS] = {0};

/** @name Flags
 * @{
 */
#define OF			 (2) //!< Oscillator fail flag
#define BL			 (4) //!< Battery low bit
#define AF			 (6) //!< Alarm flag (read only)
#define WDF			 (7) //!< Watchdog flag (read only)
#define READ_OF		 (BIT_CHECK(register_map[FLAGS_REG], OF))
#define READ_BL		 (BIT_CHECK(register_map[FLAGS_REG], BL))
#define READ_AF		 (BIT_CHECK(register_map[FLAGS_REG], AF))
#define READ_WDF	 (BIT_CHECK(register_map[FLAGS_REG], WDF))
#define WRITE_OF(x)  (BITMASK_SET_OR_CLEAR(register_map[FLAGS_REG], (1 << OF), (x)))
#define WRITE_BL(x)  (BITMASK_SET_OR_CLEAR(register_map[FLAGS_REG], (1 << BL), (x)))
#define WRITE_AF(x)  (BITMASK_SET_OR_CLEAR(register_map[FLAGS_REG], (1 << AF), (x)))
#define WRITE_WDF(x) (BITMASK_SET_OR_CLEAR(register_map[FLAGS_REG], (1 << WDF), (x)))
/** @} */

#define ST			(7) //!< Stop bit
#define READ_ST		(BIT_CHECK(register_map[SECONDS_REG], ST))
#define WRITE_ST(x) (BITMASK_SET_OR_CLEAR(register_map[SECONDS_REG], (1 << ST), (x)))

#if 0
static uint8_t dec2bcd(uint8_t dec) {
	return ((dec/10 * 16) + (dec % 10));
}

#endif
static uint8_t bcd2dec(uint8_t bcd) {
	return ((bcd/16 * 10) + (bcd % 16));
}


static void update_registers(void) {
	usart0_printf("reading array from twi\n");
#if 0
	while(1) {
		for (uint8_t dev_id = 0; dev_id <= (1 << 7); dev_id++) {
			_delay_ms(1000);
			usart0_printf("\n\ntrying dev_id 0x%X\n", dev_id);
			twi_read_array(dev_id, TENTH_HUNDREDTHS_SECONDS_REG,
				register_map, (YEAR_REG+1));
		}
		//while(1);
	}
#else
	twi_read_array(RTC_SLAVE_ADDR, TENTH_HUNDREDTHS_SECONDS_REG,
		register_map, NUMBER_OF_REGISTERS);
#endif

}

static void reset_oscilator(void) {
	update_registers();

	//WRITE_ST(1);
	//twi_write_array(RTC_SLAVE_ADDR, &register_map[SECONDS_REG], 1);
	//WRITE_ST(0);
	//twi_write_array(RTC_SLAVE_ADDR, &register_map[SECONDS_REG], 1);

	//WRITE_ST(1);
	//twi_write_array(RTC_SLAVE_ADDR, &register_map[SECONDS_REG], 1);

}

void rtc_init(void) {
	reset_oscilator();
}

void rtc_get_time_fix(struct rtc_time* t) {
	usart0_printf("mapping twi registers\n");
	update_registers();
#if 1
	t->tenth_sec 		= bcd2dec(
		register_map[TENTH_HUNDREDTHS_SECONDS_REG] & 0xFF);
	t->hundredths_sec 	= bcd2dec(
		register_map[TENTH_HUNDREDTHS_SECONDS_REG] & 0xFF);

	t->seconds 			= bcd2dec(register_map[SECONDS_REG] & 0x7F);
	t->minutes 			= bcd2dec(register_map[MINUTES_REG] & 0x7F);
	t->hours 			= bcd2dec(register_map[CENTURY_HOURS_REG] & 0x3F);
	t->day_of_week 		= bcd2dec(register_map[DAY_REG] & 0x07);
	t->day_of_month		= bcd2dec(register_map[DATE_REG] & 0x3F);
	t->month 			= bcd2dec(register_map[MONTH_REG] & 0x1F);
	t->year 			= bcd2dec(register_map[YEAR_REG] & 0xFF);
#else
	t->tenth_sec 		= (register_map[TENTH_HUNDREDTHS_SECONDS_REG] );
	t->hundredths_sec 	= (register_map[TENTH_HUNDREDTHS_SECONDS_REG] );

	t->seconds 			= (register_map[SECONDS_REG] );
	t->minutes 			= (register_map[MINUTES_REG] );
	t->hours 			= (register_map[CENTURY_HOURS_REG] );
	t->day_of_week 		= (register_map[DAY_REG] );
	t->day_of_month		= (register_map[DATE_REG] );
	t->month 			= (register_map[MONTH_REG] );
	t->year 			= (register_map[YEAR_REG] );
#endif
}
