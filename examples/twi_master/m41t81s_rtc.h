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

#ifndef M41T81S_RTC_H
#define M41T81S_RTC_H

#include <stdint.h>

#define RTC_SLAVE_ADDR	0xD0//(0xD0 >> 1)

enum m41t81s_registers {
	HUNDREDTH_SECONDS_REG 			= 0x00,
	SECONDS_REG 					= 0x01,
	MINUTES_REG 					= 0x02,
	CENTURY_HOURS_REG 				= 0x03,
	DAY_REG 						= 0x04,
	DATE_REG 						= 0x05,
	MONTH_REG 						= 0x06,
	YEAR_REG 						= 0x07,
	CALIBRATION_REG 				= 0x08,
	WATCHDOG_REG 					= 0x09,

	ALARM_REG0 						= 0x0A,
	ALARM_REG1 						= 0x0B,
	ALARM_REG2						= 0x0C,
	ALARM_REG3 						= 0x0D,
	ALARM_REG4 						= 0x0E,

	FLAGS_REG 						= 0x0F,

	RESERVED_REG0 					= 0x10,
	RESERVED_REG1 					= 0x11,
	RESERVED_REG2 					= 0x12,

	SQUARE_WAVE_REG 				= 0x13,

	// This indicates the number of registers given all the above are in one
	// inc order.
	NUMBER_OF_REGISTERS
};

struct rtc_time {
	uint8_t hundredth_seconds;
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day_of_week;
	uint8_t day_of_month;
	uint8_t month;
	uint8_t year;
};

void rtc_set_seconds(uint8_t seconds);
void rtc_set_minutes(uint8_t minutes);
void rtc_set_hours(uint8_t hours);
void rtc_set_day_of_week(uint8_t day_of_week);
void rtc_set_day_of_month(uint8_t day_of_month);
void rtc_set_month(uint8_t month);
void rtc_set_year(uint8_t year);

int16_t rtc_init(void);
void rtc_set_time(struct rtc_time *t);
int16_t rtc_get_time(struct rtc_time *t);


#endif /* M41T81S_RTC_H */
