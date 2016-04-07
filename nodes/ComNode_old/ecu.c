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
#include <system_messages.h>
#include <util/delay.h>
#include <can.h>

#include "log.h"
#include "xbee.h"
#include "bson.h"
#include "ecu.h"

#define FAULTY_BATTERY_V_SENSOR 0

#define ECU_BAUD    (19200)

#define ECU_HEARTBEAT_ISR_VECT  TIMER0_COMP_vect
#define HEARTBEAT_DELAY_LENGTH  8 // How many times we delay the heartbeat timer

static FILE *ecu = &usart0_io;

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static void send_request(void) {
	const uint8_t heart_beat[] = {0x12, 0x34, 0x56, 0x78, 0x17, 0x08, 0, 0, 0, 0};
	for (size_t i = 0; i < ARR_LEN(heart_beat); ++i) {
		fputc(heart_beat[i], ecu);
	}
}

static inline float clamp(float value) {
	uint32_t u32;
	memcpy(&u32, &value, sizeof(value));
	u32 = (u32 > (1 << 15)) ? -(0xFFFF - u32) : u32;
	memcpy(&value, &u32, sizeof(value));
	return value;
}

void ecu_parse_package(void) {
	send_request();

	struct ecu_package {
		struct sensor sensor;
		float raw_value; // The raw data received from the ECU
		size_t length;      // length of the data in bytes
	} pkt[] = {
#       include "ecu_package_layout.inc"
	};

	// We loop over the package and extract the number of bytes element contains
	for (size_t i = 0; i < ARR_LEN(pkt); ++i) {
		while (pkt[i].length--) {
			const uint8_t ecu_byte = fgetc(ecu);
			if (pkt[i].sensor.id == EMPTY) {
#if 0
				// We know these pkts are just zero, if they are not reset
				if ((i == ARR_LEN(pkt) - 4 || i == 0 || i == 16) && ecu_byte != 0) {
					// Reset ECU and clear the current corrupt package
					TIMSK0 &= ~(1 << OCIE0A); // Disable heartbeat isr
					_delay_ms(50); // About double the heartbeat time.
					ecu_init();
					return;
				}
#endif
				continue;
			}

			pkt[i].raw_value += (ecu_byte << (8 * pkt[i].length));
		}
	}

	// Convert the raw data to usable data
	// We have to do this after collecting an entire ECU package to avoid
	// broadcasting corrupt data
	for (size_t i = 0; i < ARR_LEN(pkt); ++i) {
		switch (pkt[i].sensor.id) {
		case STATUS_LAMBDA_V2:
			pkt[i].sensor.value = (70 - clamp(pkt[i].raw_value) / 64.0);
			break;
		case WATER_TEMP:
		case MANIFOLD_AIR_TEMP:
			pkt[i].sensor.value = (pkt[i].raw_value * (-150.0 / 3840) + 120);
			break;
		case SPEEDER_POTMETER:
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

		if (pkt[i].sensor.id != EMPTY) {
			const uint16_t tx_id = ECU_PKT + pkt[i].sensor.id;
			enum medium transport = can_msg_transport(tx_id);

			uint8_t buf[sizeof(tx_id)+sizeof(pkt[i].sensor.value)];
			memcpy(buf, &tx_id, sizeof(tx_id));
			memcpy(buf+sizeof(tx_id), &pkt[i].sensor.value, sizeof(pkt[i].sensor.value));

			if (transport & CAN) {
				// CAN sends the id as message id, so it is not needed in the
				// payload.
#if FAULTY_BATTERY_V_SENSOR
				// Hot fix faulty sensor data
				if (pkt[i].sensor.id == BATTERY_V) {
					pkt[i].sensor.value = 13.0;
				}
#endif

				can_broadcast(tx_id, &pkt[i].sensor.value);
			}

			if (transport & XBEE) xbee_send(buf, ARR_LEN(buf));
			if (transport & SD) log_append(buf, ARR_LEN(buf));
		}
	}
}

void ecu_init(void) {
	usart0_init(ECU_BAUD, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));  // ECU
}