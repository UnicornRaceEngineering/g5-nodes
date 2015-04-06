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

#include <serialize.h>
#include <string.h>

#define ECU_BAUD    (19200)

#define ECU_HEARTBEAT_ISR_VECT  TIMER0_COMP_vect

#define HEARTBEAT_DELAY_LENGTH  8 // How many times we delay the heartbeat timer

#define ARR_LEN(x)  (sizeof(x) / sizeof(x[0]))


FILE *ecu_out = &usart0_byte_output;
FILE *ecu_in = &usart0_input;

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
			const uint8_t ecu_byte = fgetc(ecu_in);
			if (pkt[i].sensor.id == EMPTY) continue;

			pkt[i].raw_value += (ecu_byte << (8 * pkt[i].length));
		}

		bool is_float = true;

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
				pkt[i].sensor.int_val = pkt[i].raw_value;
				is_float = false;
				break;
			}
		}

		if (pkt[i].sensor.id != EMPTY) {
			uint8_t buff[4 + 1 + 4 + 1 + 8];
			int s = 0;

			// The first 4 byte in the buffer
			buff[s++] = DT_INT8;
			buff[s++] = ECU_DATA;
			buff[s++] = DT_INT8;
			buff[s++] = pkt[i].sensor.id;

			// next 1 + 4 bytes
			int len;
			if (is_float) {
				buff[s++] = DT_FLOAT32;
				len = sizeof(pkt[i].sensor.value);
				memcpy(buff + s, &pkt[i].sensor.value, len);
			} else {
				buff[s++] = DT_INT32;
				len = sizeof(pkt[i].sensor.int_val);
				memcpy(buff + s, &pkt[i].sensor.int_val, len);
			}
			s += len;

			// 1 + 8 bytes
			buff[s++] = DT_UTC_DATETIME;
			const int64_t ts = rtc_utc_datetime();
			len = sizeof(ts);
			memcpy(buff + s, &ts, len);
			s += len;

			xbee_send(buff, s);
		}
	}
}

#if 0
void ecu_send_schema(void) {
	uint8_t schema[1024 / 2] = {'\0'};
	int s = 0;
	schema[s++] = ECU_DATA;
	s += add_to_schema(schema + s, "ECU data", DT_SCHEMA, 512 - s);
	for (int i = 0; i < 2; ++i) {
		s += add_to_schema(schema + s, (char*)ECU_ID_NAME(i), DT_SCHEMA, 512 - s);
		enum datatype dt;
		switch (i) {
		case STATUS_LAMBDA_V2:
		case WATER_TEMP:
		case MANIFOLD_AIR_TEMP:
		case POTMETER:
		case RPM:
		case MAP_SENSOR:
		case BATTERY_V:
		case LAMBDA_V:
		case INJECTOR_TIME:
		case IGNITION_TIME:
		case GX:
		case GY:
		case GZ:
			dt = DT_FLOAT32;
			break;
		default:
			dt = DT_INT32;
			break;
		}

		s += add_to_schema(schema + s, "value", dt, 512 - s);
		s += add_to_schema(schema + s, "timestamp", DT_UTC_DATETIME, 512 - s);
		s += add_to_schema(schema + s, "", DT_SCHEMA_END, 512 - s);
	}
	s += add_to_schema(schema + s, "", DT_SCHEMA_END, 512 - s);
	xbee_send(schema, s);
}
#endif

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
			fputc(heart_beat[i], ecu_out);
		}
		delay = HEARTBEAT_DELAY_LENGTH; // Reset the delay
	}

}

