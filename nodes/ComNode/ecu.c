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
#include <string.h>
#include <utils.h>
#include <heap.h>
#include <can_transport.h>

#include "log.h"
#include "xbee.h"
#include "bson.h"
#include "ecu.h"

#define ECU_BAUD    (19200)

#define ECU_HEARTBEAT_ISR_VECT  TIMER0_COMP_vect
#define HEARTBEAT_DELAY_LENGTH  8 // How many times we delay the heartbeat timer

FILE *ecu = &usart0_io;

static inline uint32_t clamp(uint32_t value) {
	return (value > (1 << 15)) ? -(0xFFFF - value) : value;
}

void ecu_parse_package(void) {

	struct ecu_package {
		struct sensor sensor;
		uint32_t raw_value; // The raw data received from the ECU
		size_t length;      // length of the data in bytes
	} pkt[] = {
#       include "ecu_package_layout.inc"
	};

	// We loop over the package and extract the number of bytes element contains
	for (size_t i = 0; i < ARR_LEN(pkt); ++i) {
		while (pkt[i].length--) {
			const uint8_t ecu_byte = fgetc(ecu);
			if (pkt[i].sensor.id == EMPTY) continue;

			pkt[i].raw_value += (ecu_byte << (8 * pkt[i].length));
		}

		// Convert the raw data to usable data
		{
			switch (pkt[i].sensor.id) {
			case STATUS_LAMBDA_V2:
				pkt[i].sensor.value = (70 - clamp(pkt[i].raw_value) / 64.0);
				break;
			case WATER_TEMP:
			case MANIFOLD_AIR_TEMP:
				pkt[i].sensor.value = (pkt[i].raw_value * (-150.0 / 3840) + 120);
				break;
			case POTMETER:
				pkt[i].sensor.value = ((pkt[i].raw_value - 336) / 26.9);
				break;
			case RPM:
				pkt[i].sensor.value = (pkt[i].raw_value * 0.9408);
				break;
			case MAP_SENSOR:
				pkt[i].sensor.value = (pkt[i].raw_value * 0.75);
				break;
			case BATTERY_V:
				pkt[i].sensor.value = (pkt[i].raw_value * (1.0 / 210) + 0);
				break;
			case LAMBDA_V:
				pkt[i].sensor.value = ((70 - clamp(pkt[i].raw_value) / 64.0) / 100);
				break;
			case INJECTOR_TIME:
			case IGNITION_TIME:
				pkt[i].sensor.value = (-0.75 * pkt[i].raw_value + 120);
				break;
			case GX:
			case GY:
			case GZ:
				pkt[i].sensor.value = (clamp(pkt[i].raw_value) * (1.0 / 16384));
				break;

			default:
				// No conversion
				pkt[i].sensor.value = pkt[i].raw_value;
				break;
			}
		}

		if (pkt[i].sensor.id != EMPTY) {
			// Broadcast on CAN
			{
				uint8_t *buf = smalloc(1 + sizeof(pkt[i].sensor.value));
				*buf++ = pkt[i].sensor.id;
				memcpy(buf, &pkt[i].sensor.value, sizeof(pkt[i].sensor.value));
				can_broadcast(ECU_DATA_PKT, buf);
			}

			// Broadcast to Xbee
			{
				uint8_t buff[4 + 1 + 4 + 1 + 8];
				int s = 0;

				// The first 4 byte in the buffer
				buff[s++] = DT_INT8;
				buff[s++] = XBEE_ECU;
				buff[s++] = DT_INT8;
				buff[s++] = pkt[i].sensor.id;

				// next 1 + 4 bytes
				buff[s++] = DT_FLOAT32;
				memcpy(buff + s, &pkt[i].sensor.value, sizeof(pkt[i].sensor.value));
				s += sizeof(pkt[i].sensor.value);

				// 1 + 8 bytes
				buff[s++] = DT_UTC_DATETIME;
				const int64_t ts = rtc_utc_datetime();
				memcpy(buff + s, &ts, sizeof(ts));
				s += sizeof(ts);

				xbee_send(buff, s);
				log_append(buff, s);
			}
		}
	}
}

void ecu_init(void) {
	usart0_init(ECU_BAUD);  // ECU

	// setup timer 0 which periodically sends heartbeat to the ECU
	{
		// 1/((F_CPU/Prescaler)/n_timersteps)
		// 1/((11059200/1024)/256) = approx 23.7 ms or about 42 Hz
		OCR0A = 100;                     // Set start value
		TIMSK0 |= 1 << OCIE0A;           // Enable timer compare match interrupt
		TCCR0A |= 1 << CS02 | 1 << CS00; // Set prescaler 1024
	}
}


ISR(ECU_HEARTBEAT_ISR_VECT) {
	static int delay = HEARTBEAT_DELAY_LENGTH;

	// We have to send a start sequence to the ECU to force it respond with data
	// but if we ask too often it crashes
	if (!delay--) {
		const uint8_t heart_beat[] = {0x12, 0x34, 0x56, 0x78, 0x17, 0x08, 0, 0, 0, 0};
		for (size_t i = 0; i < ARR_LEN(heart_beat); ++i) {
			fputc(heart_beat[i], ecu);
		}
		delay = HEARTBEAT_DELAY_LENGTH; // Reset the delay
	}

}

