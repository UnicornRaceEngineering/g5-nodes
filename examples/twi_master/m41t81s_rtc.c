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

/**
 * @name Bits
 * The individual bits in the clock register map
 * @{
 */
#define D0	(1 << 0)
#define D1	(1 << 1)
#define D2	(1 << 2)
#define D3	(1 << 3)
#define D4	(1 << 4)
#define D5	(1 << 5)
#define D6	(1 << 6)
#define D7	(1 << 7)
/** @} */

/**
 * @name Bit masks
 * Taken directly from the m41t81s rtc datasheet
 * @{
 */
#define HUNDREDTH_OF_SECONDS_TENS_MASK	(D7|D6|D5|D4)
#define HUNDREDTH_OF_SECONDS_ONES_MASK	(D3|D2|D1|D0)
#define HUNDREDTH_OF_SECONDS_MASK \
	(HUNDREDTH_OF_SECONDS_TENS_MASK|HUNDREDTH_OF_SECONDS_ONES_MASK)

#define ST_MASK							(D7)

#define SECONDS_TENS_MASK				(D6|D5|D4)
#define SECONDS_ONES_MASK				(D3|D2|D1|D0)
#define SECONDS_MASK \
	(SECONDS_TENS_MASK|SECONDS_ONES_MASK)

#define MINUTES_TENS_MASK				(D6|D5|D4)
#define MINUTES_ONES_MASK				(D3|D2|D1|D0)
#define MINUTES_MASK \
	(MINUTES_TENS_MASK|MINUTES_ONES_MASK)

#define CEB_MASK						(D7)
#define CB_MASK							(D6)

#define HOURS_TENS_MASK					(D5|D4)
#define HOURS_ONES_MASK					(D3|D2|D1|D0)
#define HOURS_MASK \
	(HOURS_TENS_MASK|HOURS_ONES_MASK)

#define DAY_OF_WEEK_MASK				(D2|D1|D0)

#define DAY_OF_MONTH_TENS_MASK			(D5|D4)
#define DAY_OF_MONTH_ONES_MASK			(D3|D2|D1|D0)
#define DAY_OF_MONTH_MASK \
	(DAY_OF_MONTH_TENS_MASK|DAY_OF_MONTH_ONES_MASK)

#define MONTH_TENS_MASK					(D4)
#define MONTH_ONES_MASK					(D3|D2|D1|D0)
#define MONTH_MASK \
	(MONTH_TENS_MASK|MONTH_ONES_MASK)

#define YEAR_TENS_MASK					(D7|D6|D5|D4)
#define YEAR_ONES_MASK					(D3|D2|D1|D0)
#define YEAR_MASK \
	(YEAR_TENS_MASK|YEAR_ONES_MASK)

#define OUT_MASK						(D7)
#define FT_MASK							(D6)
#define S_MASK							(D5)

#define CALIBARTION_MASK				(D4|D3|D2|D1|D0)

#define OFIE_MASK						(D7)

#define BMB4_TO_BMB0_MASK				(D5|D4|D3|D2)
#define RB1_TO_RB0_MASK					(D1|D0)

#define AFIE_MASK						(D7)
#define SQW_MASK						(D6)
#define ABE_MASK						(D5)
#define AI_MASK							(D4)

#define ALARM_MONTH_MASK				(D3|D2|D1|D0)

#define ALARM_DAY_OF_MONTH_TENS_MASK	(D5|D4)
#define ALARM_DAY_OF_MONTH_ONES_MASK	(D3|D2|D1|D0)
#define ALARM_DAY_OF_MONTH_MASK \
	(ALARM_DAY_OF_MONTH_TENS_MASK|ALARM_DAY_OF_MONTH_ONES_MASK)

#define ALARM_HOUR_TENS_MASK			(D5|D4)
#define ALARM_HOUR_ONES_MASK			(D3|D2|D1|D0)
#define ALARM_HOUR_MASK \
	(ALARM_HOUR_TENS_MASK|ALARM_HOUR_ONES_MASK)

#define ALARM_MINUTES_TENS_MASK			(D6|D5|D4)
#define ALARM_MINUTES_ONES_MASK			(D3|D2|D1|D0)
#define ALARM_MINUTES_MASK \
	(ALARM_MINUTES_TENS_MASK|ALARM_MINUTES_ONES_MASK)

#define ALARM_SECONDS_TENS_MASK			(D6|D5|D4)
#define ALARM_SECONDS_ONES_MASK			(D3|D2|D1|D0)
#define ALARM_SECONDS_MASK \
	(ALARM_SECONDS_TENS_MASK|ALARM_SECONDS_ONES_MASK)

#define RPT4_MASK						(D7)
#define RPT5_MASK						(D6)
#define RPT3_MASK						(D7)
#define RPT2_MASK						(D7)
#define RPT1_MASK						(D7)

#define HT_MASK							(D6)

#define WDF_MASK						(D7)
#define AF_MASK							(D6)
#define BL_MASK							(D4)
#define OF_MASK							(D2)

#define RS3_TO_RS0_MASK					(D7|D6|D5|D4)
/** @} */

static uint8_t register_map[NUMBER_OF_REGISTERS] = {0};

/**
 * Read a flag from a specified register in the register map
 * @param  flag The bit index of the flag we want to read
 * @param  reg  The register number of the flag we want to read
 * @return      A Bool indicating if the flag is high or low
 */
#define READ_FLAG_FROM_REGISTER_MAP(flag, reg) \
	((bool)BIT_CHECK(register_map[(reg)], (flag)))

/**
 * Write a flag to the specified register in the register map
 * @param  flag The bit index of the flag we want to read
 * @param  reg  The register number of the flag we want to read
 * @param  x    The value to set the specified bit. Must be HIGH or LOW
 */
#define WRITE_FLAG_TO_REGISTER_MAP(flag, reg, x) \
	(BITMASK_SET_OR_CLEAR(register_map[(reg)], (1 << (flag)), (x)))

/** @name Flags
 * Read and write values to different flags
 * @{
 */
#define OF			 (2) //!< Oscillator fail flag
#define OF_REG		 FLAGS_REG
#define READ_OF()	 READ_FLAG_FROM_REGISTER_MAP(OF, OF_REG)
#define WRITE_OF(x)  WRITE_FLAG_TO_REGISTER_MAP(OF, OF_REG, (x))

#define BL			 (4) //!< Battery low bit
#define BL_REG		 FLAGS_REG
#define READ_BL()	 READ_FLAG_FROM_REGISTER_MAP(BL, BL_REG)
#define WRITE_BL(x)  WRITE_FLAG_TO_REGISTER_MAP(BL, BL_REG, (x))

#define AF			 (6) //!< Alarm flag (read only)
#define AF_REG		 FLAGS_REG
#define READ_AF()	 READ_FLAG_FROM_REGISTER_MAP(AF, AF_REG)
#define WRITE_AF(x)  WRITE_FLAG_TO_REGISTER_MAP(AF, AF_REG, (x))

#define WDF			 (7) //!< Watchdog flag (read only)
#define WDF_REG		 FLAGS_REG
#define READ_WDF()	 READ_FLAG_FROM_REGISTER_MAP(WDF, WDF_REG)
#define WRITE_WDF(x) WRITE_FLAG_TO_REGISTER_MAP(WDF, WDF_REG, (x))

#define ST			(7) //!< Stop bit
#define ST_REG		SECONDS_REG
#define READ_ST()	READ_FLAG_FROM_REGISTER_MAP(ST, ST_REG)
#define WRITE_ST(x) WRITE_FLAG_TO_REGISTER_MAP(ST, ST_REG, (x))

#define HT			(6) //!< Halt update bit
#define HT_REG		ALARM_REG2
#define READ_HT()	READ_FLAG_FROM_REGISTER_MAP(HT, HT_REG)
#define WRITE_HT(x) WRITE_FLAG_TO_REGISTER_MAP(HT, HT_REG, (x))
/** @} */

#if 0
static uint8_t dec2bcd(uint8_t dec) {
	return ((dec/10 * 16) + (dec % 10));
}

#endif
static uint8_t bcd2dec(uint8_t bcd) {
	return LOW_NIBBLE(bcd) + (10 * HIGH_NIBBLE(bcd));
}


static int16_t update_registers(void) {
	return twi_read_array(RTC_SLAVE_ADDR, HUNDREDTH_SECONDS_REG,
		register_map, NUMBER_OF_REGISTERS);

}

int16_t rtc_disable_halt_update(void) {
	if (READ_HT() == LOW) return 0;

	WRITE_HT(LOW);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(HT_REG);
	twi_write(register_map[HT_REG]);
	twi_send_stop_condition();

	return 0;
}

int16_t rtc_reset_oscilator(void) {
	int16_t rc;
	if ((rc = update_registers()) < 0) return  rc;

	WRITE_ST(HIGH);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(ST_REG);
	twi_write(register_map[ST_REG]);

	WRITE_ST(LOW);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(ST_REG);
	twi_write(register_map[ST_REG]);
	twi_send_stop_condition();

	// The rtc datasheet says that we must wait atleast 4 seconds after setting
	// OF low.
	_delay_ms(1000);
	_delay_ms(1000);
	_delay_ms(1000);
	_delay_ms(1000);
	WRITE_OF(LOW);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(OF_REG);
	twi_write(register_map[OF_REG]);

	twi_send_stop_condition();
	return rc;
}

int16_t rtc_set_stopbit_low(void) {
	int16_t rc;
	if ((rc = update_registers()) < 0) return  rc;
	if (READ_ST() == LOW) return 0;

	WRITE_ST(LOW);
	twi_start_write(RTC_SLAVE_ADDR);
	twi_write(ST_REG);
	twi_write(register_map[ST_REG]);
	twi_send_stop_condition();

	return 0;
}

int16_t rtc_init(void) {
	if (rtc_set_stopbit_low() != 0) return -1;

	update_registers();
	rtc_disable_halt_update();
	rtc_set_stopbit_low();

	if (READ_OF() == HIGH) {
		//rtc_reset_oscilator();
	}
	return 0;
}

int16_t rtc_get_time_fix(struct rtc_time* t) {
	int16_t rc;
	if ((rc = update_registers()) < 0) return rc;

	t->hundredth_seconds = bcd2dec(register_map[HUNDREDTH_SECONDS_REG] & HUNDREDTH_OF_SECONDS_MASK);
	t->seconds			 = bcd2dec(register_map[SECONDS_REG] & SECONDS_MASK);
	t->minutes 			 = bcd2dec(register_map[MINUTES_REG] & MINUTES_MASK);
	t->hours 			 = bcd2dec(register_map[CENTURY_HOURS_REG] & HOURS_MASK);
	t->day_of_week 		 = bcd2dec(register_map[DAY_REG] & DAY_OF_WEEK_MASK);
	t->day_of_month		 = bcd2dec(register_map[DATE_REG] & DAY_OF_MONTH_MASK);
	t->month 			 = bcd2dec(register_map[MONTH_REG] & MONTH_MASK);
	t->year 			 = bcd2dec(register_map[YEAR_REG] & YEAR_MASK);

	return rc;
}
