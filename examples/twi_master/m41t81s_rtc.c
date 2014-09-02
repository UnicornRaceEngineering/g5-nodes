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

#define HIGH	1
#define LOW		0

static uint8_t register_map[NUMBER_OF_REGISTERS] = {0};

/** @name Flags
 * @{
 */
#define OF			 (2) //!< Oscillator fail flag
#define BL			 (4) //!< Battery low bit
#define AF			 (6) //!< Alarm flag (read only)
#define WDF			 (7) //!< Watchdog flag (read only)
#define READ_OF()	 ((bool)BIT_CHECK(register_map[FLAGS_REG], OF))
#define READ_BL()	 ((bool)BIT_CHECK(register_map[FLAGS_REG], BL))
#define READ_AF()	 ((bool)BIT_CHECK(register_map[FLAGS_REG], AF))
#define READ_WDF()	 ((bool)BIT_CHECK(register_map[FLAGS_REG], WDF))
#define WRITE_OF(x)  (BITMASK_SET_OR_CLEAR(register_map[FLAGS_REG], (1 << OF), (x)))
#define WRITE_BL(x)  (BITMASK_SET_OR_CLEAR(register_map[FLAGS_REG], (1 << BL), (x)))
#define WRITE_AF(x)  (BITMASK_SET_OR_CLEAR(register_map[FLAGS_REG], (1 << AF), (x)))
#define WRITE_WDF(x) (BITMASK_SET_OR_CLEAR(register_map[FLAGS_REG], (1 << WDF), (x)))
/** @} */

#define ST			(7) //!< Stop bit
#define READ_ST()	((bool)BIT_CHECK(register_map[SECONDS_REG], ST))
#define WRITE_ST(x) (BITMASK_SET_OR_CLEAR(register_map[SECONDS_REG], (1 << ST), (x)))

#define HT			(6) //!< Halt update bit
#define READ_HT()	((bool)BIT_CHECK(register_map[ALARM_REG2], HT))
#define WRITE_HT(x) (BITMASK_SET_OR_CLEAR(register_map[ALARM_REG2], (1 << HT), (x)))

#if 0
static uint8_t dec2bcd(uint8_t dec) {
	return ((dec/10 * 16) + (dec % 10));
}

#endif
static uint8_t bcd2dec(uint8_t bcd) {
	return LOW_NIBBLE(bcd) + (10 * HIGH_NIBBLE(bcd));
}


static int16_t update_registers(void) {
	//usart0_printf("reading array from twi\n");
	return twi_read_array(RTC_SLAVE_ADDR, MILI_SECONDS_REG,
		register_map, NUMBER_OF_REGISTERS);

}

int16_t rtc_disable_halt_update(void) {
	if (READ_HT() != LOW) {
		WRITE_HT(LOW);
		twi_start_write(RTC_SLAVE_ADDR);
		twi_write(ALARM_REG2);
		twi_write(register_map[ALARM_REG2]);
		twi_send_stop_condition();
	}

	return 0;
}

int16_t rtc_reset_oscilator(void) {
	int16_t rc;
	if ((rc = update_registers()) < 0) return  rc;

	WRITE_ST(HIGH);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(SECONDS_REG);
	twi_write(register_map[SECONDS_REG]);

	WRITE_ST(LOW);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(SECONDS_REG);
	twi_write(register_map[SECONDS_REG]);
	twi_send_stop_condition();

	_delay_ms(1000);
	_delay_ms(1000);
	_delay_ms(1000);
	_delay_ms(1000);
	WRITE_OF(LOW);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(FLAGS_REG);
	twi_write(register_map[FLAGS_REG]);

	twi_send_stop_condition();
	return rc;
}

int16_t rtc_set_stopbit_low(void) {
	int16_t rc;
	if ((rc = update_registers()) < 0) return  rc;

	if (READ_ST() == LOW) return 0;
	WRITE_ST(LOW);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(SECONDS_REG); // The Stop bit is in this register
	twi_write(register_map[SECONDS_REG]);
	twi_send_stop_condition();

	return 0;
}

int16_t rtc_init(void) {
	if (rtc_set_stopbit_low() != 0) return -1;

	update_registers();
	bool oscillator_fail = READ_OF();
	rtc_disable_halt_update();
	rtc_set_stopbit_low();

	if (oscillator_fail == HIGH) {
		//rtc_reset_oscilator();
	}
	return 0;
}

int16_t rtc_get_time_fix(struct rtc_time* t) {
	int16_t rc;
	//usart0_printf("mapping twi registers\n");
	if ((rc = update_registers()) < 0) return rc;

	t->hundredth_seconds = bcd2dec(register_map[MILI_SECONDS_REG] & 0xFF);
	t->seconds			 = bcd2dec(register_map[SECONDS_REG] & 0x7F);
	t->minutes 			 = bcd2dec(register_map[MINUTES_REG] & 0x7F);
	t->hours 			 = bcd2dec(register_map[CENTURY_HOURS_REG] & 0x3F);
	t->day_of_week 		 = bcd2dec(register_map[DAY_REG] & 0x07);
	t->day_of_month		 = bcd2dec(register_map[DATE_REG] & 0x3F);
	t->month 			 = bcd2dec(register_map[MONTH_REG] & 0x1F);
	t->year 			 = bcd2dec(register_map[YEAR_REG] & 0xFF);

	//usart0_printf("ST: %u HT: %u\n", READ_ST(), READ_HT());
	return rc;
}
