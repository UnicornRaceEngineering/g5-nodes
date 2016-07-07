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
#include <io.h>                           // for IO_SET_HIGH, IO_SET_LOW, etc
#include <stdbool.h>                      // for bool, false, true
#include <stddef.h>                       // for size_t
#include <stdint.h>                       // for uint8_t, uint16_t, etc
#include <stdio.h>                        // for printf
#include <usart.h>                        // for usart1_init
#include <util/delay.h>
#include <utils.h>                        // for ARR_LEN
#include <can.h>
#include <sysclock.h>

#include "adc.h"                          // for adc_init, adc_vref_t::AVCC
#include "neutralsensor.h"                // for GEAR_IS_NEUTRAL, NEUT_PIN, etc
#include "system_messages.h"              // for message_id::CURRENT_GEAR, etc
#include "vnh2sp30.h"                     // for vnh2sp30_is_faulty, etc

#define IGN_PORT	(PORTE)
#define IGN_PIN		(PIN4)
#define IGNITION_CUT()			( IO_SET_HIGH(IGN_PORT, IGN_PIN) )
#define IGNITION_UNCUT()		( IO_SET_LOW(IGN_PORT, IGN_PIN) )


enum gear_dir {
	STOP = 0,
	UP = 1,
	DOWN = 2,
};

static void gear(enum gear_dir dir);

static void gear_up(void);
static void gear_down(void);

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	sysclock_init();
	adc_init(1, AVCC, 4);
	can_init();

	vnh2sp30_init();
	vnh2sp30_active_break_to_Vcc();

	SET_PIN_MODE(NEUT_PORT, NEUT_PIN, INPUT_PULLUP);
	SET_PIN_MODE(IGN_PORT, IGN_PIN, OUTPUT);
	IGNITION_UNCUT();

	can_subscribe(PADDLE_STATUS);
	can_subscribe(NEUTRAL_ENABLED);
	can_subscribe(GEAR_STOP_BUTTON);
	can_subscribe(RESET_GEAR_ESTIMATE);

	sei();
	puts_P(PSTR("Init complete\n\n"));
}


int main(void) {
	init();

	while (1) {
//		if(usart1_has_data()) {
//			char c = getchar();
//
//			switch (c) {
//			case 27:
//			case 91:
//				continue;
//				break;
//			case 65:
//				gear_up();
//				printf("\n");
//				break;
//			case 66:
//				gear_down();
//				printf("\n");
//				break;
//			case '\r':
//				printf("\r\n");
//				break;
//			default:
//				break;
//			}
//		}

		if (can_has_data()) {
			struct can_message message;
			read_message(&message);
			switch (message.id) {
			case PADDLE_STATUS:
				if (message.data[0] == 1) {
					gear_up();
				}

				if (message.data[0] == 0) {
					gear_down();
				}
				break;
			case NEUTRAL_ENABLED:
				if (message.data[0] == 1) {
					printf("NEUTRAL BUTTON ACTIVATED\n");
					while(1) {
						if (can_has_data()) {
							struct can_message message;
							read_message(&message);
							switch (message.id) {
							case PADDLE_STATUS:
								if (message.data[0] == 1) {
									gear(UP);
									_delay_ms(30);
									gear(STOP);
								}

								if (message.data[0] == 0) {
									gear(DOWN);
									_delay_ms(30);
									gear(STOP);
								}
								break;
							case NEUTRAL_ENABLED:
								if (message.data[0] == 0) {
									printf("NEUTRAL BUTTON DEACTIVATED\n");
									goto NO_NEUTRAL;
								}
								break;
							default:
								break;
							}
						}
					}
				}
				NO_NEUTRAL:
				break;
			default:
				break;
			}
			can_init();
		}

	}

	return 0;
}


static void gear_up(void) {
	printf("SHIFT UP\n");
	IGNITION_CUT();
	gear(UP);

	uint32_t timer = get_tick() + 500;

	while(1) {
		if (get_tick() > timer) {
			gear(STOP);
			IGNITION_UNCUT();
			gear(DOWN);
			_delay_ms(100);
			gear(STOP);
			printf("DIDN'T REACH END\n");
			return;
		}

		if (can_has_data()) {
			struct can_message message;
			read_message(&message);
			if ((message.id == GEAR_STOP_BUTTON) && (message.data[0] == 2)) {
				gear(STOP);
				IGNITION_UNCUT();
				_delay_ms(50);
				gear(DOWN);

				uint32_t timer2 = get_tick() + 200;
				while(1) {
					if (get_tick() > timer2) {
						gear(STOP);
						printf("FAILED TO RELEASE AFTER SHIFT\n");
						return;
					}

					if (can_has_data()) {
						struct can_message message;
						read_message(&message);
						if ((message.id == GEAR_STOP_BUTTON) && (message.data[0] == 0)) {
							_delay_ms(100);
							gear(STOP);
							printf("PERFECT GEARSHIFT\n");
							return;
						}
					}
				}
			}
		}
	}
}


static void gear_down(void) {
	printf("SHIFT DOWN\n");
	gear(DOWN);

	uint32_t timer = get_tick() + 300;

	while(1) {
		if (get_tick() > timer) {
			gear(STOP);
			gear(UP);
			_delay_ms(100);
			gear(STOP);
			printf("DIDN'T REACH END\n");
			return;
		}

		if (can_has_data()) {
			struct can_message message;
			read_message(&message);
			if ((message.id == GEAR_STOP_BUTTON) && (message.data[0] == 1)) {
				gear(STOP);
				_delay_ms(50);
				gear(UP);

				uint32_t timer2 = get_tick() + 200;
				while(1) {
					if (get_tick() > timer2) {
						gear(STOP);
						printf("FAILED TO RELEASE AFTER SHIFT\n");
						return;
					}

					if (can_has_data()) {
						struct can_message message;
						read_message(&message);
						if ((message.id == GEAR_STOP_BUTTON) && (message.data[0] == 0)) {
							gear(STOP);
							gear(UP);
							_delay_ms(150);
							gear(STOP);
							printf("PERFECT GEARSHIFT\n");
							return;
						}
					}
				}
			}
		}
	}
}


static void gear(enum gear_dir dir) {
	switch (dir) {
	case STOP:
		vnh2sp30_active_break_to_Vcc();
		break;
	case UP:
		vnh2sp30_clear_INA();
		vnh2sp30_set_INB();
		vnh2sp30_set_PWM_dutycycle(100);
		break;
	case DOWN:
		vnh2sp30_clear_INB();
		vnh2sp30_set_INA();
		vnh2sp30_set_PWM_dutycycle(100);
		break;
	}
}
