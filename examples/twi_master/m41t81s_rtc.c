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
	return twi_read_array(RTC_SLAVE_ADDR, MILI_SECONDS_REG,
		register_map, NUMBER_OF_REGISTERS);

}

int16_t rtc_disable_halt_update(void) {
	if (READ_HT() != LOW) {
		WRITE_HT(LOW);
		twi_start_write(RTC_SLAVE_ADDR);
		twi_write(HT_REG);
		twi_write(register_map[HT_REG]);
		twi_send_stop_condition();
	}

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
	if ((rc = update_registers()) < 0) return rc;

	t->hundredth_seconds = bcd2dec(register_map[MILI_SECONDS_REG] & 0xFF);
	t->seconds			 = bcd2dec(register_map[SECONDS_REG] & 0x7F);
	t->minutes 			 = bcd2dec(register_map[MINUTES_REG] & 0x7F);
	t->hours 			 = bcd2dec(register_map[CENTURY_HOURS_REG] & 0x3F);
	t->day_of_week 		 = bcd2dec(register_map[DAY_REG] & 0x07);
	t->day_of_month		 = bcd2dec(register_map[DATE_REG] & 0x3F);
	t->month 			 = bcd2dec(register_map[MONTH_REG] & 0x1F);
	t->year 			 = bcd2dec(register_map[YEAR_REG] & 0xFF);

	return rc;
}
