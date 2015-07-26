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
#include <avr/sfr_defs.h>
#include <max7221_7seg.h>     // for seg7_disp_str, seg7_disp_char, etc
#include <stdbool.h>          // for false
#include <stddef.h>           // for size_t
#include <stdint.h>           // for uint8_t, uint16_t, int16_t
#include <stdio.h>            // for snprintf
#include <string.h>           // for memset
#include <sysclock.h>         // for sysclock_init
#include <usart.h>            // for usart1_init
#include <util/delay.h>
#include <utils.h>            // for ARR_LEN
#include <can.h>

#include "../ComNode/ecu.h"   // for ecu_id::BATTERY_V, ecu_id::RPM, etc
#include "dipswitch.h"        // for dip_init, dip_read
#include "neutral.h"          // for neutral_btn_init, neutral_is_enabled, etc
#include "paddleshift.h"      // for paddle_init, paddle_state
#include "rotaryswitch.h"     // for rot_init, rot_read
#include "rpm.h"              // for rpm_init, set_rpm
#include "shiftlight.h"       // for shiftlight_init, shiftlight_off
#include "statuslight.h"      // for color_masks, color_masks::COLOR_OFF, etc
#include "system_messages.h"  // for message_id::ECU_PKT, etc


#define WARN_GOOD	COLOR_OFF
#define WARN_BAD	RED
#define WARN_CRIT	BLUE

static uint8_t buf_in[64];
static uint8_t buf_out[64];

enum warn_leds {
	WARN_BATTERY_VOLT_LED,
	WARN_WATER_TEMP_LED,
};
static enum color_masks led_warn[8] = {WARN_GOOD};

static void update_warning_light(enum warn_leds led, float value) {
	enum color_masks c = WARN_GOOD;
	switch(led) {
		case WARN_BATTERY_VOLT_LED:
			if (value < 11.0) {
				c = WARN_CRIT;
			} else if (value < 12.7) {
				c = WARN_BAD;
			} else {
				c = WARN_GOOD;
			}
			break;
		case WARN_WATER_TEMP_LED:
			if (value > 115.0) {
				c = WARN_CRIT;
			} else if (value > 110.0) {
				c = WARN_BAD;
			} else {
				c = WARN_GOOD;
			}
			break;

		default: return;
	}
	led_warn[led] = c;
}

static void display_gear(uint8_t gear) {
	seg7_disp_char(3, (gear != 0) ? ('0' + gear) : 'n', false);
}

static void display_right(uint16_t v) {
	char str[8] = {'\0'};
	snprintf(str, ARR_LEN(str), "%u", v);
	seg7_disp_str(str, 4, 6);
}

static void display_left(uint16_t v) {
	char str[8] = {'\0'};
	snprintf(str, ARR_LEN(str), "%u", v);
	seg7_disp_str(str, 0, 2);
}

static void display_left_f(float f) {
	char str[8]= {'\0'};
	// add the 0.01 to avoid rounding errors
	uint8_t d = (uint8_t)((f + 0.01 - (int)f) * 10);
	snprintf(str, ARR_LEN(str), "%u.%u", (uint16_t)f, d);
	seg7_disp_str(str, 0, 2);
}

static void init(void) {
	can_init();
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	sysclock_init();
	paddle_init();
	statuslight_init();
	seg7_init();
	rpm_init();
	shiftlight_init();
	neutral_btn_init();
	rot_init();
	dip_init();

	can_subscribe(CURRENT_GEAR);
	can_subscribe(ECU_PKT + RPM);
	can_subscribe(ECU_PKT + BATTERY_V);
	can_subscribe(ECU_PKT + WATER_TEMP);

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

static struct foreign_state {
	float battery_volt;
	float water_temp;
	uint8_t gear;
	int16_t rpm;
} fstate;

int main(void) {
	init();
	memset(&fstate, 0, sizeof(fstate));
	display_gear(fstate.gear);

	while (1) {
		while (can_has_data()) {
			struct can_message msg;
			read_message(&msg);
			void* data = &msg.data[0];
			switch (msg.id) {
				case CURRENT_GEAR:
					fstate.gear = *(uint8_t*)msg.data;
					display_gear(fstate.gear);
					break;
				case ECU_PKT + RPM:
					fstate.rpm = (int16_t)*(float*)data;
					set_rpm(fstate.rpm);
					break;
				case ECU_PKT + BATTERY_V:
					fstate.battery_volt = *(float*)data;
					update_warning_light(WARN_BATTERY_VOLT_LED, fstate.battery_volt);
					break;
				case ECU_PKT + WATER_TEMP:
					fstate.water_temp = *(float*)data;
					update_warning_light(WARN_WATER_TEMP_LED, fstate.water_temp);
					break;
			}
		}

		// First lets store the current status of the paddleshifters
		{
			const uint8_t state = paddle_state();
			if (state) {
				can_broadcast(PADDLE_STATUS, (uint8_t [1]) {state} );
				shiftlight_off();
			}
		}

		// Check if neutral enable button has changed state and broadcast the
		// current state
		if (neutral_state_has_changed()) {
			can_broadcast(NEUTRAL_ENABLED, (uint8_t [1]) {neutral_is_enabled()});
		}

		// Set the warning lights to their values
		for (size_t i = 0; i < ARR_LEN(led_warn); ++i) {
			set_rgb_color(i, led_warn[i]);
		}

		display_right((uint16_t)fstate.water_temp);
		switch (rot_read()) {
			case 0:
			case 1:
				display_left_f(fstate.battery_volt);
				break;

			case 2:
			case 3:
				// Cooler temp difference (no sensor yet)
				break;

			case 4:
			case 5:
				// Potmeter
				break;

			case 6:
			case 7:
				// Lambda
				break;

			case 8:
			case 9:
				// Error codes
				break;

			case 10:
			case 11:
				// Mode (switch on back of board)
				display_left(dip_read());
				break;

			case 12:
			case 13:
				// RPM/100
				display_left(fstate.rpm/100);
				break;
		}

#if 0
		// Test rotary switch
		{
			char buf[8] = {'\0'};
			snprintf(buf, ARR_LEN(buf), "%u", rot_read());
			seg7_disp_str(buf, 0, 2);
		}
#endif

#if 0
		// Test dip switch
		{
			display_left(dip_read());
		}
#endif

#if 0
		// Test the 7seg
		{
			// Test the raw diplay
			for (int num = 0; num <= 9; ++num) {
				for (int digit = 0; digit < 7; ++digit) {
					const char ascii_num = num + '0'; // Raw number to ascii
					seg7_disp_char(digit, ascii_num, (num % 2 == 0) ?
					               true : false);
				}
				_delay_ms(125);
			}

			// Test the high level display string with dot
			for (int i = 0; i < 150; ++i) {
				char buff[3 + 1 + 1] = {'\0'}; // 3 digits 1 dot and 1 \0
				snprintf(buff, ARR_LEN(buff), "%d.", i);
				seg7_disp_str(buff, 0, 2);
				_delay_ms(25);
			}
		}

#endif

#if 0
		// Test the status LEDS
		{
			for (int i = 0; i < 8; ++i) {
				// When setting one of these low we allow current to flow from
				// the LED to GND thus the LED will light up.
				// In short LOW == ON

				seg7_disp_char(3, i + '0', false);

				{
					set_rgb_color(i, RED);         _delay_ms(50);
					set_rgb_color(i, GREEN);       _delay_ms(50);
					set_rgb_color(i, BLUE);        _delay_ms(50);

					set_rgb_color(i, YELLOW);      _delay_ms(50);
					set_rgb_color(i, MAGENTA);     _delay_ms(50);
					set_rgb_color(i, CYAN);        _delay_ms(50);

					set_rgb_color(i, WHITE);       _delay_ms(50);
					set_rgb_color(i, COLOR_OFF);   _delay_ms(50);
				}
			}
		}
#endif

#if 0
		// Test the RPM meter
		{
			char buff[14] = {'\0'};
			// Rev up
			for (int16_t rpm = 0; rpm <= RPM_MAX_VALUE; rpm += 100) {
				set_rpm(rpm);

				snprintf(buff, ARR_LEN(buff), "%d.", rpm / 100);
				seg7_disp_str(buff, 4, 6);
				_delay_ms(25);
			}
			_delay_ms(100);
			// Rev Down
			for (int16_t rpm = RPM_MAX_VALUE; rpm >= 0; rpm -= 100) {
				set_rpm(rpm);

				snprintf(buff, ARR_LEN(buff), "%d.", rpm / 100);
				seg7_disp_str(buff, 4, 6);
				_delay_ms(25);
			}
		}
#endif
	}

	return 0;
}
