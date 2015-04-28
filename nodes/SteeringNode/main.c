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

#include <heap.h>
#include <can_transport.h>
#include <usart.h>
#include <io.h>
#include <utils.h>
#include <tick.h>

// Drivers
#include <74ls138d_demultiplexer.h>
#include <max7221_7seg.h>

#include "../ComNode/ecu.h"

#include "paddleshift.h"
#include "statuslight.h"
#include "rpm.h"
#include "shiftlight.h"
#include "neutral.h"
#include <heap.h>

#if 0
#include <avr/fuse.h>
FUSES = {.low = 0xFF, .high = 0xD9, .extended = 0xFD};
#endif



static void handle_ecu_data(uint8_t *data) {
	const enum ecu_id id = *data++;
	const float val = *(float*)data;

	switch (id) {
		case RPM:
			set_rpm((int16_t)val); break;

		case BATTERY_V: set_rgb_color(0, (val < 12.7) ? GREEN : RED); break;
		case WATER_TEMP: set_rgb_color(1, (val < 100) ? GREEN : RED); break;
		case MOTOR_OILTEMP: set_rgb_color(2, (val < 100) ? GREEN : RED); break;
		case OIL_PRESSURE: set_rgb_color(3, ((int)val) ? GREEN : RED); break;
		case MANIFOLD_AIR_TEMP: set_rgb_color(4, (val < 100) ? GREEN : RED); break;
		case MAP_SENSOR: set_rgb_color(5, (val < 100) ? GREEN : RED); break;
		case FUEL_PRESSURE: set_rgb_color(6, ((int)val) ? GREEN : RED); break;
		/* TODO handle lauch control */
			break;

		default: break;
	}
}

static void display_gear(uint8_t gear) {
	seg7_disp_char(3, (gear != 0) ? ('0' + gear) : 'n', false);
}

static void init(void) {
	init_can_node(STEERING_NODE);
	init_heap();
	usart1_init(115200);
	tick_init();
	paddle_init();
	statuslight_init();
	seg7_init();
	rpm_init();
	shiftlight_init();
	neutral_btn_init();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

int main(void) {
	init();

	while (1) {
		// Main work loop

		// Handle incomming CAN messages
		while (get_queue_length()) {
			struct can_message *msg = read_inbox();

			switch(msg->info.id) {
				case ECU_DATA_PKT:
					handle_ecu_data(msg->data);
					break;
				case CURRENT_GEAR:
					display_gear(*(uint8_t*)msg->data);
					break;

				default:
					fprintf(stderr, "Unknown can id %d\n", msg->info.id);
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
			const enum dmux_y_values leds[] = {
				DMUX_Y0,
				DMUX_Y1,
				DMUX_Y2,
				DMUX_Y3,
				DMUX_Y4,
				DMUX_Y5,
				DMUX_Y6,
				DMUX_Y7,
			};
			for (int i = 0; i < (int)ARR_LEN(leds); ++i) {
				// When setting one of these low we allow current to flow from
				// the LED to GND thus the LED will light up.
				// In short LOW == ON

				seg7_disp_char(3, i + '0', false);

				{
					set_rgb_color(leds[i], RED);         _delay_ms(50);
					set_rgb_color(leds[i], GREEN);       _delay_ms(50);
					set_rgb_color(leds[i], BLUE);        _delay_ms(50);

					set_rgb_color(leds[i], YELLOW);      _delay_ms(50);
					set_rgb_color(leds[i], MAGENTA);     _delay_ms(50);
					set_rgb_color(leds[i], CYAN);        _delay_ms(50);

					set_rgb_color(leds[i], WHITE);       _delay_ms(50);
					set_rgb_color(leds[i], COLOR_OFF);   _delay_ms(50);
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
