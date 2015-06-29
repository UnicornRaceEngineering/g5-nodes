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
#include <stdio.h>

#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>

#include <can_transport.h>
#include <usart.h>
#include <io.h>
#include <utils.h>
#include <sysclock.h>

// Drivers
#include <74ls138d_demultiplexer.h>
#include <max7221_7seg.h>

#include "../ComNode/ecu.h"

#include "paddleshift.h"
#include "statuslight.h"
#include "rpm.h"
#include "shiftlight.h"
#include "neutral.h"
#include "rotaryswitch.h"

#if 0
#include <avr/fuse.h>
FUSES = {.low = 0xFF, .high = 0xD9, .extended = 0xFD};
#endif

#define WARN_GOOD	COLOR_OFF
#define WARN_BAD	RED
#define WARN_CRIT	BLUE

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

static void init(void) {
	init_can_node(STEERING_NODE);
	usart1_init(115200);
	sysclock_init();
	paddle_init();
	statuslight_init();
	seg7_init();
	rpm_init();
	shiftlight_init();
	neutral_btn_init();
	rot_init();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();

	while (1) {
		// Main work loop

		while (get_queue_length()) {
			struct can_message *msg = read_inbox();
			switch (msg->index) {
				case CURRENT_GEAR: display_gear(*(uint8_t*)msg->data);
				case ECU_PKT + RPM: set_rpm(*(int16_t*)msg->data); break;
				case ECU_PKT + BATTERY_V:
					update_warning_light(WARN_BATTERY_VOLT_LED, *(float*)msg->data);
					break;
				case ECU_PKT + WATER_TEMP:
					update_warning_light(WARN_WATER_TEMP_LED, *(float*)msg->data);
					break;
			}
			can_free(msg);
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

#if 0
		// Test rotary switch
		{
			char buf[8] = {'\0'};
			snprintf(buf, ARR_LEN(buf), "%u", rot_read());
			seg7_disp_str(buf, 0, 2);
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
