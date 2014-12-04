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
 * @file ecu.c
 * High level interface for the receiving data from the ECU.
 * The ECU communicates via. UART, but require a keep-alive heart beat to
 * actually send any data. Therefor we must periodically send the heartbeat.
 *
 * @note that the frequency we send the heartbeat is *NOT* the frequency with
 * which we receive data from the ECU.
 *
 * The structure of the data we receive from the ECU can be found in
 * "ecu_package_layout.inc".
 *
 */

#include <stdbool.h>
#include <avr/interrupt.h>
#include <usart.h>
#include <timer.h>
#include <m41t81s_rtc.h>

#include <pff.h>

#include "ecu.h"
#include "xbee.h"
#include "bson.h"

#define ECU_BAUD	(19200)

#define ECU_HEARTBEAT_ISR_VECT	TIMER0_COMP_vect

#define HEARTBEAT_DELAY_LENGTH	8 // How many times we delay the heartbeat timer

#define ARR_LEN(x)  (sizeof(x) / sizeof(x[0]))


static inline uint32_t clamp(uint32_t value) {
	return (value > (1<<15)) ? -(0xFFFF - value) : value;
}

void ecu_parse_package(void) {

	struct ecu_package {
		struct sensor sensor;
		uint32_t raw_value; // The raw data received from the ECU
		size_t length; 		// length of the data in bytes
	} pkt[] = {
#		include "ecu_package_layout.inc"
	};

	// We loop over the package and extract the number of bytes element contains
	for (size_t i = 0; i < ARR_LEN(pkt); ++i) {
		while (pkt[i].length--) {
			const uint8_t ecu_byte = usart0_getc_unbuffered();
			if (pkt[i].sensor.id == EMPTY ) continue;

			pkt[i].raw_value += (ecu_byte << (8 * pkt[i].length));
		}

		// Convert the raw data to usable data
		{
			switch (pkt[i].sensor.id) {
			case STATUS_LAMBDA_V2:
				pkt[i].sensor.value = (70-clamp(pkt[i].raw_value)/64.0);
				break;
			case WATER_TEMP:
			case MANIFOLD_AIR_TEMP:
				pkt[i].sensor.value = (pkt[i].raw_value * (-150.0/3840) + 120);
				break;
			case POTMETER:
				pkt[i].sensor.value = ((pkt[i].raw_value-336)/26.9);
				break;
			case RPM:
				pkt[i].sensor.value = (pkt[i].raw_value*0.9408);
				break;
			case MAP_SENSOR:
				pkt[i].sensor.value = (pkt[i].raw_value*0.75);
				break;
			case BATTERY_V:
				pkt[i].sensor.value = (pkt[i].raw_value*(1.0/210)+0);
				break;
			case LAMBDA_V:
				pkt[i].sensor.value = ((70-clamp(pkt[i].raw_value)/64.0)/100);
				break;
			case INJECTOR_TIME:
			case IGNITION_TIME:
				pkt[i].sensor.value = (-0.75*pkt[i].raw_value+120);
				break;
			case GX:
			case GY:
			case GZ:
				pkt[i].sensor.value = (clamp(pkt[i].raw_value) * (1.0/16384));
				break;
			case FUEL_PRESSURE:
			case STATUS_LAP_COUNT:
			case STATUS_INJ_SUM:
			case LAST_GEAR_SHIFT:
			case MOTOR_OILTEMP:
			case OIL_PRESSURE:
			case STATUS_TIME:
			case STATUS_LAP_TIME:
			case GEAR_OIL_TEMP:
			case STATUS_TRACTION:
			case STATUS_GAS:
			case STATUS_CAM_TRIG_P1:
			case STATUS_CAM_TRIG_P2:
			case STATUS_CHOKER_ADD:
			case STATUS_LAMBDA_PWM:
			case TRIGGER_ERR:
			case CAM_ANGLE1:
			case CAM_ANGLE2:
			case ROAD_SPEED:
			case LOAD:
			case DWELL_TIME:
			case MOTOR_FLAGS:
			case OUT_BITS:
			case TIME:
			case EMPTY:
			default:
				// No conversion
				pkt[i].sensor.value = (double)pkt[i].raw_value;
				break;
			}
		}

		if (pkt[i].sensor.id != EMPTY) {
			// Ready sensor for serialization
			struct bson_element sensor[] = {
				{
					.e_id 			= ID_DOUBLE,
					.key 			= "val",
					.floating_val 	= pkt[i].sensor.value,
				},
				{
					.e_id 			= ID_UTC_DATETIME,
					.key 			= "ts",
					.utc_datetime 	= rtc_utc_datetime(),
				},
			};

			// Make an element that contains the two others
			struct bson_element sensor_doc = {
				.e_id 				= ID_EMBEDED_DOCUMENT,
				.key 				= (char*)pkt[i].sensor.name,
				.elements.e			= sensor,
				.elements.n_elem 	= ARR_LEN(sensor),
			};

			uint8_t bson[128];
			int32_t bson_len = serialize(bson, &sensor_doc, 1, ARR_LEN(bson));
			unsigned bytes_written = 0;
			if (bson_len > 0) {
				pf_write(bson, bson_len, &bytes_written);
				xbee_send(bson, bson_len);
			}
		}
	}
}

void ecu_init(void) {
	usart0_init(ECU_BAUD);	// ECU

	// setup timer 0 which periodically sends heartbeat to the ECU
	{
		// 1/((F_CPU/Prescaler)/n_timersteps)
		// 1/((11059200/1024)/256) = approx 23.7 ms or about 42 Hz
		OCR0A = 100;					// Set start value
		TIMSK0 |= 1<<OCIE0A; 			// Enable timer compare match interrupt
		TCCR0A |= 1<<CS02 | 1<<CS00;    // Set prescaler 1024
	}
}


ISR(ECU_HEARTBEAT_ISR_VECT) {
	static int delay = HEARTBEAT_DELAY_LENGTH;

	// We have to send a start sequence to the ECU to force it respond with data
	// but if we ask too often it crashes
	if (!delay--) {
		const uint8_t heart_beat[] = {0x12,0x34,0x56,0x78,0x17,0x08,0,0,0,0};
		for (size_t i = 0; i < ARR_LEN(heart_beat); ++i) {
			usart0_putc_unbuffered(heart_beat[i]);
		}
		delay = HEARTBEAT_DELAY_LENGTH; // Reset the delay
	}

}

