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

#include <stdbool.h>
#include <avr/interrupt.h>
#include <usart.h>
#include <timer.h>
#include "parser.h"
#include "ecu.h"

#define ECU_BAUD	(19200)

#define ECU_TIMER				0
#define ECU_REQUEST_ISR_VECT	TIMER0_COMP_vect

#define ECU_RECV_PKT_SIZE	115 //!< Legacy code uses this package size

static void init_ECU_request_timer(void);


static void init_ECU_request_timer(void) {
	// 1/((F_CPU/Prescaler)/n_timersteps)
	// 1/((11059200/1024)/256) = approx 23.7 ms or about 42 Hz
	OCR0A = 100;					// Set start value
	TIMSK0 |= 1<<OCIE0A; 			// Enable timer compare match interrupt
	TCCR0A |= 1<<CS02 | 1<<CS00;    // Set prescaler 1024
}

void ecu_parse_package(void) {
	struct parser p = {
		.package_start_counter = 0,
		.package_start = false,
		.bytes_to_read = -1,
		.val_out = 0,
		.cfg_index = -1,
		.sensor_found = false,
	};

	// Apprently the ECU sends packages of a fixed sized length
	for (int i = 0; i < ECU_RECV_PKT_SIZE; ++i) {
		struct sensor s;

		// usart1_printf("[%d: 0x%x], ",i, usart0_getc());
		// usart1_printf("[%d: %d], ", i, usart0_getc());

		usart1_printf("1\n");
		const uint8_t ecu_byte = usart0_getc();
		usart1_printf("2\n");
		parse_next(ecu_byte, &s, &p); //!< @TODO do something with return value?
		usart1_printf("3\n");
		if (p.sensor_found) {
			//!< @TODO do something with the sensor data
			usart1_printf("sensor: %s, id: %d, value*1000: %d\n", s.name, s.id,
				(int)(s.value*1000));
		} else {
			usart1_printf("Nothing found\n");
		}
	}
}

void ecu_init(void) {
	usart0_init(ECU_BAUD);	// ECU
	init_ECU_request_timer();
}

ISR(ECU_REQUEST_ISR_VECT) {
	// We have to send a start sequence to the ECU to force it respond with data
	// but if we ask too often it crashes
	const uint8_t start_seq[10] = {0x12,0x34,0x56,0x78,0x17,0x08,0,0,0,0};
	for (int i = 0; i < 10; ++i) {
		usart0_putc_unbuffered(start_seq[i]);
	}

}

