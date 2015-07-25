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
 * @file main.c
 * Main entry for the GPS node. This continuously receives data from the GPS and
 * broadcasts it via CAN.
 */

#include <avr/interrupt.h>
#include <can_transport.h>    // for can_broadcast, can_malloc, etc
#include <stdint.h>           // for uint8_t
#include <usart.h>            // for usart1_init, usart1_io
#include <util/delay.h>
#include <utils.h>            // for ARR_LEN, HIGH_BYTE, LOW_BYTE

#include "gps.h"              // for gps_fix, GPS_DMS_TO_DD, gps_get_fix, etc
#include "system_messages.h"  // for message_id::GPS_DATA, etc

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static void init(void) {
	gps_set_getc(&usart1_io);
	usart1_init(GPS_BAUDRATE, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));

	init_can_node(GPS_NODE);
	sei();	//Enable interrupt
}

int main(void) {
	init();

	struct gps_fix fix;

	// Main work loop
	while(1){
		if (gps_get_fix(&fix) == 0 ) {
			float dd = GPS_DMS_TO_DD(&(fix.latitude));
			uint8_t *dd_ptr = (uint8_t*)&dd; // We need a pointer to the float
											 // to split it up into 4 bytes

			uint8_t *data = (uint8_t*)can_malloc(13);
			data[0] = 1;
			data[1] = *(dd_ptr + 0);
			data[2] = *(dd_ptr + 1);
			data[3] = *(dd_ptr + 2);
			data[4] = *(dd_ptr + 3);
			data[5] = fix.valid;

			dd = GPS_DMS_TO_DD(&(fix.longitude));

			data[6] = 2;
			data[7] = *(dd_ptr + 0);
			data[8] = *(dd_ptr + 1);
			data[9] = *(dd_ptr + 2);
			data[10] = *(dd_ptr + 3);
			data[11] = HIGH_BYTE(fix.speed);
			data[12] = LOW_BYTE(fix.speed);

			can_broadcast(GPS_DATA, &data[0]);
		}
	}

    return 0;
}
