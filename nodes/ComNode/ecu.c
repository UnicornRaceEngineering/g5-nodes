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
#include <avr/pgmspace.h>
#include <usart.h>
#include <string.h>
#include <utils.h>

#include "ecu.h"

#define ECU_BAUD    	(19200)
#define ECU_PACKET_LEN	(114)


static float clamp(float value);

static const int8_t ecu_packet[][2] PROGMEM = {
	{ECU_FUEL_PRESSURE 			,2},
	{ECU_STATUS_LAP_COUNT 		,2},
	{ECU_STATUS_INJ_SUM 		,2},
	{ECU_LAST_GEAR_SHIFT 		,2},
	{ECU_MOTOR_OILTEMP 			,2},
	{ECU_OIL_PRESSURE 			,2},
	{ECU_STATUS_TIME 			,4},
	{ECU_STATUS_LAP_TIME 		,4},
	{ECU_GEAR_OIL_TEMP 			,2},
	{ECU_STATUS_TRACTION 		,2},
	{ECU_STATUS_GAS 			,2},
	{ECU_STATUS_LAMBDA_V2 		,2},
	{ECU_STATUS_CAM_TRIG_P1 	,2},
	{ECU_STATUS_CAM_TRIG_P2 	,2},
	{ECU_STATUS_CHOKER_ADD 		,2},
	{ECU_STATUS_LAMBDA_PWM 		,2},
	{ECU_EMPTY 					,10},
	{ECU_WATER_TEMP 			,2},
	{ECU_MANIFOLD_AIR_TEMP 		,2},
	{ECU_SPEEDER_POTMETER 		,2},
	{ECU_EMPTY 					,2},
	{ECU_RPM 					,2},
	{ECU_TRIGGER_ERR 			,2},
	{ECU_CAM_ANGLE1 			,2},
	{ECU_CAM_ANGLE2 			,2},
	{ECU_ROAD_SPEED 			,2},
	{ECU_MAP_SENSOR 			,2},
	{ECU_BATTERY_V 				,2},
	{ECU_LAMBDA_V 				,2},
	{ECU_EMPTY 					,4},
	{ECU_LOAD 					,2},
	{ECU_EMPTY 					,2},
	{ECU_INJECTOR_TIME 			,2},
	{ECU_EMPTY 					,2},
	{ECU_IGNITION_TIME 			,2},
	{ECU_DWELL_TIME 			,2},
	{ECU_EMPTY 					,10},
	{ECU_GX 					,2},
	{ECU_GY 					,2},
	{ECU_GZ 					,2},
	{ECU_EMPTY 					,8},
	{ECU_MOTOR_FLAGS 			,1},
	{ECU_EMPTY 					,1},
	{ECU_OUT_BITS 				,1},
	{ECU_TIME 					,1},

	/* End state, not an actual part of the data recieved */
	{ECU_EMPTY 					,0},
};


static FILE *ecu = &usart0_io;

static uint8_t buf_in[128];
static uint8_t buf_out[16];


void ecu_init(void) {
	usart0_init(ECU_BAUD, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));  // ECU
}


void ecu_send_request(void) {
	const uint8_t heart_beat[] = {0x12, 0x34, 0x56, 0x78, 0x17, 0x08, 0, 0, 0, 0};
	for (size_t i = 0; i < ARR_LEN(heart_beat); ++i) {
		fputc(heart_beat[i], ecu);
	}
}


bool ecu_has_packet(void) {
	// Every full data responce we get following a request is 114 bytes long.
	return usart0_input_buffer_bytes() == ECU_PACKET_LEN;
}


static float clamp(float value) {
	uint32_t u32;
	memcpy(&u32, &value, sizeof(value));
	u32 = (u32 > (1 << 15)) ? -(0xFFFF - u32) : u32;
	memcpy(&value, &u32, sizeof(value));
	return value;
}


bool ecu_read_data(struct sensor *data) {
	static uint8_t data_count = 0;
	float raw_value = 0;

	data->id = pgm_read_byte(&(ecu_packet[data_count][0]));
	uint8_t len = pgm_read_byte(&(ecu_packet[data_count][1]));

	if (!len) {
		data_count = 0;
		return false;
	}

	if (data->id == ECU_EMPTY) {
		for (uint8_t i = 0; i < len; ++i) {
			fgetc(ecu);
		}
		++data_count;
		data->id = pgm_read_byte(&(ecu_packet[data_count][0]));
		len = pgm_read_byte(&(ecu_packet[data_count][1]));
	}

	while (len--) {
		raw_value += fgetc(ecu) << (8 * len);
	}

	switch (data->id ) {
	case ECU_STATUS_LAMBDA_V2:
		raw_value = (70 - clamp(raw_value) / 64.0);
		break;
	case ECU_WATER_TEMP:
	case ECU_MANIFOLD_AIR_TEMP:
		raw_value = (raw_value * (-150.0 / 3840) + 120);
		break;
	case ECU_SPEEDER_POTMETER:
		raw_value = ((raw_value - 336) / 26.9);
		break;
	case ECU_RPM:
		raw_value = (raw_value * 0.9408);
		break;
	case ECU_MAP_SENSOR:
		raw_value = (raw_value * 0.75);
		break;
	case ECU_BATTERY_V:
		raw_value = (raw_value * (1.0 / 210) + 0);
		break;
	case ECU_LAMBDA_V:
		raw_value = ((70 - clamp(raw_value) / 64.0) / 100);
		break;
	case ECU_INJECTOR_TIME:
	case ECU_IGNITION_TIME:
		raw_value = (-0.75 * raw_value + 120);
		break;
	case ECU_GX:
	case ECU_GY:
	case ECU_GZ:
		raw_value = (clamp(raw_value) * (1.0 / 16384));
		break;

	default:
		// No conversion
		break;
	}

	data->value = raw_value;

	++data_count;
	return true;
}
