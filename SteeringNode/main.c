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

#include <can_transport.h>
#include <usart.h>
#include <io.h>

// Drivers
#include <74ls138d_demultiplexer.h>
#include <max7221_7seg.h>

#include "paddleshift.h"
#include "statuslight.h"

#if 0
#include <avr/fuse.h>
FUSES = {.low = 0xFF, .high = 0xD9, .extended = 0xFD};
#endif

#define ARR_LEN(arr)    (sizeof(arr) / sizeof(arr[0]))

#define SHIFT_LIGHT_PORT    PORTE
#define SHIFT_LIGHT_R       PIN4 // Red rgb light
#define SHIFT_LIGHT_B       PIN3 // Blue rgb light

#define RPM_PORT        PORTB
#define RPM_PIN         PIN4
#define RPM_TCCR        TCCR2A
#define RPM_OCR         OCR2A
#define RPM_WGM1        WGM21
#define RPM_WGM0        WGM20
#define RPM_CS1         CS21
#define RPM_CS0         CS20
#define RPM_COMA1       COM0A1
#define RPM_COMA0       COM0A0

#define RPM_MAX_VALUE   13000
#define RPM_MIN_VALUE   3300

enum paddle_status {PADDLE_DOWN, PADDLE_UP};

static int32_t map(int32_t x,
                   const int32_t from_low, const int32_t from_high,
                   const int32_t to_low, const int32_t to_high) {
	if (x < from_low) {
		x = from_low;
	} else if (x > from_high) {
		x = from_high;
	}

	return (x - from_low) * (to_high - to_low) / (from_high - from_low) + to_low;
}

static void set_rpm(int16_t rpm) {
	// Because the PWM signal goes through a low pass filter we loose some
	// granularity so we must lower the max value or the RPM meter will max out
	// too soon. The value is determined by increasing the calibration value
	// until it "looked right".
	const int8_t calibration = 80;
	RPM_OCR = map(rpm, RPM_MIN_VALUE, RPM_MAX_VALUE, 0, 0xFF - calibration);
}

int main(void) {
	usart1_init(115200);

	paddle_init();
	statuslight_init();

	seg7_init();

	// init Timer0 PWM PB4 for the RPM-counter
	{
		RPM_TCCR |= (1 << RPM_WGM1) | (1 << RPM_WGM0); // Fast PWM mode
		RPM_TCCR |= (1 << RPM_CS1) | (1 << RPM_CS1); // F_CPU/64 prescalar
		// Clear RPM_OCR on compare match. Set RPM_OCR at TOP.
		RPM_TCCR |= (1 << RPM_COMA1) | (0 <<  RPM_COMA0);

		set_rpm(0);
		SET_PIN_MODE(RPM_PORT, RPM_PIN, OUTPUT);
	}

	// Shift light RGB LED
	{
		SET_PIN_MODE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B, OUTPUT);
		SET_PIN_MODE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_R, OUTPUT);
		IO_SET_LOW(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B);
		IO_SET_LOW(SHIFT_LIGHT_PORT, SHIFT_LIGHT_R);
	}

	printf("\n\n\nSTARTING\n");

	sei();                                      //Enable interrupt

	while (1) {
		// Main work loop

		// First lets store the current status of the paddleshifters
		{
			const bool paddle_up_is_pressed = paddle_up_status();
			const bool paddle_down_is_pressed = paddle_down_status();

			if (paddle_up_is_pressed) {
				//can_broadcast(PADDLE_STATUS, (uint8_t *) & (const enum paddle_status) {
				//	PADDLE_UP
				//});
				DIGITAL_TOGGLE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B);

			} else if (paddle_down_is_pressed) {
				//can_broadcast(PADDLE_STATUS, (uint8_t *) & (const enum paddle_status) {
				//	PADDLE_DOWN
				//});
				DIGITAL_TOGGLE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_R);
			}
		}

		// for (size_t n_msg = 0; n_msg < node->n_msg_subscriped; ++n_msg) {
		// 	if (node->msg_subscriped[n_msg].complete) {
		// 		struct can_message *msg = &node->msg_subscriped[n_msg];
		// 		msg->complete = false;

		// 		switch (msg->id) {
		// 		case ENGINE_RPM: set_rpm((int16_t) * (int16_t *)msg->payload.data); break;
		// 		/**
		// 		 * @TODO The steering wheel should recv a lot of other
		// 		 * values that should handled here
		// 		 */
		// 		default: break;
		// 		}
		// 	}
		// }

		// Print the values of relevant can register to the 7seg display
		{
			char buff[7] = {'\0'};
			seg7_disp_char(0, PADDLE_STATUS + '0', false);
			if (CANGSTA != (1 << ENFG)) {
				snprintf(buff, ARR_LEN(buff), "%d", CANGSTA);
				seg7_disp_str(buff, 0, 2);
				seg7_disp_char(3, '0', false);
			} else {
				seg7_disp_char(3, '1', false);
			}
			// snprintf(buff, ARR_LEN(buff), "%d", err_mob);
			snprintf(buff, ARR_LEN(buff), "%d", CANGIT);
			seg7_disp_str(buff, 4, 6);
			// _delay_ms(500);
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

				dmux_set_y_low(leds[i]);
				seg7_disp_char(3, i + '0', false);

				{
					set_rgb_color(RED);         _delay_ms(50);
					set_rgb_color(GREEN);       _delay_ms(50);
					set_rgb_color(BLUE);        _delay_ms(50);

					set_rgb_color(YELLOW);      _delay_ms(50);
					set_rgb_color(MAGENTA);     _delay_ms(50);
					set_rgb_color(CYAN);        _delay_ms(50);

					set_rgb_color(WHITE);       _delay_ms(50);
					set_rgb_color(COLOR_OFF);   _delay_ms(50);
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
