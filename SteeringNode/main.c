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

#include <can.h>
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

#define ARR_LEN(arr)	(sizeof(arr) / sizeof(arr[0]))

#define SHIFT_LIGHT_PORT	PORTE
#define SHIFT_LIGHT_R		PIN4 // Red rgb light
#define SHIFT_LIGHT_B		PIN3 // Blue rgb light

static void rx_complete(uint8_t mob);
static void tx_complete(uint8_t mob);
static void can_default(uint8_t mob);


int main(void) {
	set_canit_callback(CANIT_RX_COMPLETED, rx_complete);
	set_canit_callback(CANIT_TX_COMPLETED, tx_complete);
	set_canit_callback(CANIT_DEFAULT, can_default);

	can_init();

	CAN_SEI();
	CAN_EN_RX_INT();
	CAN_EN_TX_INT();

	usart1_init(115200);
	paddle_init();
	statuslight_init();

	seg7_init();

	// Disable RPM counter for debuggning
	SET_PIN_MODE(PORTB, PIN4, OUTPUT);
	IO_SET_LOW(PORTB, PIN4);

	// Shift light RGB LED
	SET_PIN_MODE(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B, OUTPUT);
	IO_SET_LOW(SHIFT_LIGHT_PORT, SHIFT_LIGHT_B);

	usart1_printf("\n\n\nSTARTING\n");

	sei();										//Enable interrupt

	while(1){
		// Main work loop

		// First lets store the current status of the paddleshifters
		{
			const bool paddle_up_is_pressed = paddle_up_status();
			const bool paddle_down_is_pressed = paddle_down_status();

			if (paddle_up_is_pressed) {
				//!< @TODO: broadcast this event on the can.
			} else if (paddle_down_is_pressed) {
				//!< @TODO: broadcast this event on the can.
			}
		}
#if 1
		// Test the 7seg
		{
			// Test the raw diplay
			for (int num = 0; num <= 9; ++num) {
				for (int digit = 0; digit < 7; ++digit){
					const char ascii_num = num + '0'; // Raw number to ascii
					seg7_disp_char(digit, ascii_num, (num%2 == 0) ?
						true : false);
				}
					_delay_us(5000);
			}

			// Test the high level display string with dot
			for (int i = 0; i < 150; ++i){
				char buff[3+1+1] = {'\0'}; // 3 digits 1 dot and 1 \0
				snprintf(buff, ARR_LEN(buff), "%d.", i);
				seg7_disp_str(buff, 0, 2);
				_delay_us(5000);
			}
		}

#endif

#if 1
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
			for (int i = 0; i < ARR_LEN(leds); ++i) {
				// When setting one of these low we allow current to flow from
				// the LED to GND thus the LED will light up.
				// In short LOW == ON

				dmux_set_y_low(leds[i]);

				{
					set_rgb_color(RED); 		_delay_us(2500);
					set_rgb_color(GREEN); 		_delay_us(2500);
					set_rgb_color(BLUE); 		_delay_us(2500);

					set_rgb_color(YELLOW); 		_delay_us(2500);
					set_rgb_color(MAGENTA); 	_delay_us(2500);
					set_rgb_color(CYAN); 		_delay_us(2500);

					set_rgb_color(WHITE); 		_delay_us(2500);
					set_rgb_color(COLOR_OFF); 	_delay_us(2500);
				}
			}
		}
#endif
	}

    return 0;
}

static void rx_complete(uint8_t mob) {
	can_msg_t msg = {
		.mob = mob
	};
	can_receive(&msg);
	usart1_printf("Received id: %d on mob %d :: ", msg.id, msg.mob);
#if 0
	// Print ascii data
	usart1_putn(msg.dlc, msg.data);
#else
	// Print binary data as hex
	for (int i = 0; i < msg.dlc; ++i) {
		usart1_printf("0x%02x ", msg.data[i]);
	}
#endif
	usart1_putc('\n');
}

static void tx_complete(uint8_t mob) {
	MOB_ABORT();					// Freed the MOB
	MOB_CLEAR_INT_STATUS();			// and reset MOb status
	CAN_DISABLE_MOB_INTERRUPT(mob);	// Unset interrupt
}

static void can_default(uint8_t mob) {
	MOB_CLEAR_INT_STATUS(); 		// and reset MOb status
}
