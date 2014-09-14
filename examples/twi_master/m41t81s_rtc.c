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

/**
 * @files m41t81s_rtc.c
 * @brief
 * Driver for the M41T81S RTC (Real Time Clock).
 */

#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>

#include <bitwise.h>
#include <twi.h>
#include <usart.h>
#include "m41t81s_rtc.h"

#define DEBUG(str) usart0_printf("%s:%d %s\n", __FILE__, __LINE__, str)

#define ARR_LEN(arr)	(sizeof(arr) / sizeof(arr[0]))

#define HIGH	1
#define LOW		0

/**
 * @name Bits
 * The individual bits in the clock register map
 * @TODO add better error reports
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

/**
 * Hold the local representation of the M41T81S's internal registers
 *
 * @TODO consider putting this in progmem to save memory
 */
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
 *
 * @TODO maybe there is a better way to generate these rather than having so
 * many copy paste macro here
 * @{
 */
#define OF				(2) //!< Oscillator fail flag
#define OF_REG			FLAGS_REG
#define READ_OF()		READ_FLAG_FROM_REGISTER_MAP(OF, OF_REG)
#define WRITE_OF(x)		WRITE_FLAG_TO_REGISTER_MAP(OF, OF_REG, (x))

#define BL				(4) //!< Battery low bit
#define BL_REG			FLAGS_REG
#define READ_BL()		READ_FLAG_FROM_REGISTER_MAP(BL, BL_REG)
#define WRITE_BL(x)		WRITE_FLAG_TO_REGISTER_MAP(BL, BL_REG, (x))

#define AF				(6) //!< Alarm flag (read only)
#define AF_REG			FLAGS_REG
#define READ_AF()		READ_FLAG_FROM_REGISTER_MAP(AF, AF_REG)
#define WRITE_AF(x)		WRITE_FLAG_TO_REGISTER_MAP(AF, AF_REG, (x))

#define WDF				(7) //!< Watchdog flag (read only)
#define WDF_REG			FLAGS_REG
#define READ_WDF()		READ_FLAG_FROM_REGISTER_MAP(WDF, WDF_REG)
#define WRITE_WDF(x)	WRITE_FLAG_TO_REGISTER_MAP(WDF, WDF_REG, (x))

#define ST				(7) //!< Stop bit
#define ST_REG			SECONDS_REG
#define READ_ST()		READ_FLAG_FROM_REGISTER_MAP(ST, ST_REG)
#define WRITE_ST(x)		WRITE_FLAG_TO_REGISTER_MAP(ST, ST_REG, (x))

#define HT				(6) //!< Halt update bit
#define HT_REG			ALARM_REG2
#define READ_HT()		READ_FLAG_FROM_REGISTER_MAP(HT, HT_REG)
#define WRITE_HT(x)		WRITE_FLAG_TO_REGISTER_MAP(HT, HT_REG, (x))

#define CEB				(7)
#define CEB_REG			CENTURY_HOURS_REG
#define READ_CEB()		READ_FLAG_FROM_REGISTER_MAP(CEB, CEB_REG)
#define WRITE_CEB(x)	WRITE_FLAG_TO_REGISTER_MAP(CEB, CEB_REG, (x))

#define OUT				(7)
#define OUT_REG			CENTURY_HOURS_REG
#define READ_OUT()		READ_FLAG_FROM_REGISTER_MAP(OUT, OUT_REG)
#define WRITE_OUT(x)	WRITE_FLAG_TO_REGISTER_MAP(OUT, OUT_REG, (x))

#define FT				(6)
#define FT_REG			CENTURY_HOURS_REG
#define READ_FT()		READ_FLAG_FROM_REGISTER_MAP(FT, FT_REG)
#define WRITE_FT(x)		WRITE_FLAG_TO_REGISTER_MAP(FT, FT_REG, (x))

#define S				(6)
#define S_REG			CENTURY_HOURS_REG
#define READ_S()		READ_FLAG_FROM_REGISTER_MAP(S, S_REG)
#define WRITE_S(x)		WRITE_FLAG_TO_REGISTER_MAP(S, FT_REG, (x))

#define OFIE			(7)
#define OFIE_REG		CENTURY_HOURS_REG
#define READ_OFIE()		READ_FLAG_FROM_REGISTER_MAP(OFIE, OFIE_REG)
#define WRITE_OFIE(x)	WRITE_FLAG_TO_REGISTER_MAP(OFIE, OFIE_REG, (x))

#define AFIE			(7)
#define AFIE_REG		CENTURY_HOURS_REG
#define READ_AFIE()		READ_FLAG_FROM_REGISTER_MAP(AFIE, AFIE_REG)
#define WRITE_AFIE(x)	WRITE_FLAG_TO_REGISTER_MAP(AFIE, AFIE_REG, (x))

#define SQW				(6)
#define SQW_REG			CENTURY_HOURS_REG
#define READ_SQW()		READ_FLAG_FROM_REGISTER_MAP(SQW, SQW_REG)
#define WRITE_SQW(x)	WRITE_FLAG_TO_REGISTER_MAP(SQW, SQW_REG, (x))

#define ABE				(5)
#define ABE_REG			CENTURY_HOURS_REG
#define READ_ABE()		READ_FLAG_FROM_REGISTER_MAP(ABE, ABE_REG)
#define WRITE_ABE(x)	WRITE_FLAG_TO_REGISTER_MAP(ABE, ABE_REG, (x))

#define AI				(4)
#define AI_REG			CENTURY_HOURS_REG
#define READ_AI()		READ_FLAG_FROM_REGISTER_MAP(AI, AI_REG)
#define WRITE_AI(x)		WRITE_FLAG_TO_REGISTER_MAP(AI, AI_REG, (x))
/** @} */

/* PROTOTYPES */
static uint8_t dec2bcd_tens(uint8_t dec);
static uint8_t dec2bcd_ones(uint8_t dec);
static uint8_t bcd2dec(uint8_t bcd);
static int16_t update_registers(void);
static void rtc_set_flag(uint8_t reg, uint8_t bit_index, uint8_t value);
static void zero_unused_bits(void);
static void configure_flags(void);
static void stop_watch_dog_timer(void);
static void set_and_send_time_unit(uint8_t reg, uint8_t tens_mask,
	uint8_t ones_mask, uint8_t value);

static uint8_t dec2bcd_tens(uint8_t dec) {
	return dec / 10;
}

static uint8_t dec2bcd_ones(uint8_t dec) {
	return dec - (dec2bcd_tens(dec) * 10);
}

static uint8_t bcd2dec(uint8_t bcd) {
	return LOW_NIBBLE(bcd) + (10 * HIGH_NIBBLE(bcd));
}

static int16_t update_registers(void) {
	return twi_read_array(RTC_SLAVE_ADDR, HUNDREDTH_SECONDS_REG,
		register_map, NUMBER_OF_REGISTERS-1);
}

static void rtc_set_flag(uint8_t reg, uint8_t bit_index, uint8_t value) {
	WRITE_FLAG_TO_REGISTER_MAP(bit_index, reg, value);
	twi_write_register(RTC_SLAVE_ADDR, reg, register_map[reg]);
}

/**
 * Zeros all bits that must be set to 0 on the RTC according to the data sheet
 */
static void zero_unused_bits(void) {
	// We first set the values in the local reg map
	SET_REGISTER_BITS(register_map[0x02], 0, (D7));
	SET_REGISTER_BITS(register_map[0x04], 0, (D7|D6|D5|D4|D3));
	SET_REGISTER_BITS(register_map[0x05], 0, (D7|D6));
	SET_REGISTER_BITS(register_map[0x06], 0, (D7|D6|D5));
	SET_REGISTER_BITS(register_map[0x0F], 0, (D5|D3|D1|D0));
	register_map[0x10] = 0; // Entire register must be null
	register_map[0x11] = 0;
	register_map[0x12] = 0;
	SET_REGISTER_BITS(register_map[0x13], 0, (D3|D2|D1|D0));

	// We then send each relevant register with the updated values
	twi_write_register(RTC_SLAVE_ADDR, 0x02, register_map[0x02]);
	twi_write_register(RTC_SLAVE_ADDR, 0x04, register_map[0x04]);
	twi_write_register(RTC_SLAVE_ADDR, 0x05, register_map[0x05]);
	twi_write_register(RTC_SLAVE_ADDR, 0x06, register_map[0x06]);
	twi_write_register(RTC_SLAVE_ADDR, 0x0F, register_map[0x0F]);
	twi_write_register(RTC_SLAVE_ADDR, 0x10, register_map[0x10]);
	twi_write_register(RTC_SLAVE_ADDR, 0x12, register_map[0x12]);
	twi_write_register(RTC_SLAVE_ADDR, 0x13, register_map[0x13]);
}

/**
 * Configure the M41T81S to the desired operation.
 */
static void configure_flags(void) {
	rtc_set_flag(CEB_REG, CEB, LOW);
	rtc_set_flag(OUT_REG, OUT, LOW);
	rtc_set_flag(FT_REG, FT, LOW);
	rtc_set_flag(S_REG, S, LOW);
	rtc_set_flag(OFIE_REG, OFIE, LOW);
	rtc_set_flag(AFIE_REG, AFIE, LOW);
	rtc_set_flag(SQW_REG, SQW, LOW);
	rtc_set_flag(ABE_REG, ABE, LOW);
	rtc_set_flag(AI_REG, AI, LOW);
}

static void stop_watch_dog_timer(void) {
	SET_REGISTER_BITS(register_map[WATCHDOG_REG], 0,
		(BMB4_TO_BMB0_MASK|RB1_TO_RB0_MASK));

	twi_write_register(RTC_SLAVE_ADDR, WATCHDOG_REG,
		register_map[WATCHDOG_REG]);
}

/**
 * Initializes the RTC to the correct default value
 * @return  [description]
 */
int16_t rtc_init(void) {
	update_registers();

	zero_unused_bits();
	stop_watch_dog_timer();
	configure_flags();

	/**
	 * @TODO we need to handle this better. We need to check if the HT bit is
	 * HIGH, and if it is get the current (power down) timestamp for when the
	 * clock stopped and store it somewhere.
	 */
	rtc_set_flag(HT_REG, HT, LOW);

	struct rtc_time t = {
		.seconds = 16,
		.minutes = 15,
		.hours = 14,
		.day_of_month = 13,
		.month = 12,
		.year = 11
	};
	rtc_set_time(&t);
	rtc_set_flag(ST_REG, ST, LOW);

	return 0;
}


static void set_and_send_time_unit(uint8_t reg, uint8_t tens_mask,
	uint8_t ones_mask, uint8_t value) {
	// The tens value is always in the high nipple so we shift it 4
	SET_REGISTER_BITS(register_map[reg], (dec2bcd_tens(value) << 4), tens_mask);
	SET_REGISTER_BITS(register_map[reg], dec2bcd_ones(value), ones_mask);
	twi_write_register(RTC_SLAVE_ADDR, reg, register_map[reg]);
}

void rtc_set_seconds(uint8_t seconds) {
	set_and_send_time_unit(SECONDS_REG, SECONDS_TENS_MASK, SECONDS_ONES_MASK,
		seconds);
}

void rtc_set_minutes(uint8_t minutes) {
	set_and_send_time_unit(MINUTES_REG, MINUTES_TENS_MASK, MINUTES_ONES_MASK,
		minutes);
}

void rtc_set_hours(uint8_t hours) {
	set_and_send_time_unit(CENTURY_HOURS_REG, HOURS_TENS_MASK, HOURS_ONES_MASK,
		hours);
}

void rtc_set_day_of_week(uint8_t day_of_week) {
	set_and_send_time_unit(DAY_REG, 0x00, DAY_OF_WEEK_MASK, day_of_week);
}

void rtc_set_day_of_month(uint8_t day_of_month) {
	set_and_send_time_unit(DATE_REG, DAY_OF_MONTH_TENS_MASK,
		DAY_OF_MONTH_ONES_MASK, day_of_month);
}

void rtc_set_month(uint8_t month) {
	set_and_send_time_unit(MONTH_REG, MONTH_TENS_MASK, MONTH_ONES_MASK,
		month);
}

void rtc_set_year(uint8_t year) {
	set_and_send_time_unit(YEAR_REG, YEAR_TENS_MASK, YEAR_ONES_MASK,
		year);
}


/**
 * Set the RTC time to the specified time
 * @param t pointer to the rtc_time struct with time we want to set
 */
void rtc_set_time(struct rtc_time *t) {
	rtc_set_seconds(t->seconds);
	rtc_set_minutes(t->minutes);
	rtc_set_hours(t->hours);
	rtc_set_day_of_week(t->day_of_week);
	rtc_set_day_of_month(t->day_of_month);
	rtc_set_month(t->month);
	rtc_set_year(t->year);
}

/**
 * Reads the current time from the RTC
 * @param  t pointer to struct where the read time is stored
 * @return   Negative value if an error occurred
 */
int16_t rtc_get_time(struct rtc_time *t) {
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
