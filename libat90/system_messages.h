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
 * @files system_messages.h
 * Defines the message types for the high-level can transmission.
 */

#ifndef SYSTEM_MESSAGES_H
#define SYSTEM_MESSAGES_H

#include <stdint.h>
#include "../nodes/ComNode/ecu.h"

enum node_id {
	PUBLIC,
	COM_NODE,
	GEAR_NODE,
	STEERING_NODE,
	GPS_NODE,
};

struct can_filter {
	uint16_t lower_bound;
	uint16_t upper_bound;
};

#define filter_info(node_id) ((const struct can_filter []) { \
	{ .lower_bound =    0	, .upper_bound =  255 }, /* PUBLIC */ \
	{ .lower_bound =  256	, .upper_bound = 2047 }, /* COM_NODE */ \
	{ .lower_bound =  512	, .upper_bound =  767 }, /* GEAR_NODE */ \
	{ .lower_bound =  768	, .upper_bound = 1023 }, /* STEERING_NODE */ \
	{ .lower_bound = 1024	, .upper_bound = 1275 }, /* GPS_NODE */ \
}[node_id])


enum message_id {
	// Public
	TRANSPORT_TEST_SHORT= 1,
	TRANSPORT_TEST_LONG = 2,
	ANNOUNCE            = 3,
	TIME_SYNC           = 4,

	// Com node
	HEARTBEAT           = 256,
	GPS_DATA            = 257,
	ECU_DATA_PKT        = 258, /* Every ID between ECU_DATA_PKT and
								* LAST_ECU_DATA_PKT is reserved */
	LAST_ECU_DATA_PKT   = ECU_DATA_PKT + N_ECU_IDS,

	// Gear node
	PADDLE_STATUS       = 512,

	// Steerinng node
	CURRENT_GEAR        = 768,
	NEUTRAL_ENABLED     = 769,
};

enum medium {
	NONE = 0,
	CAN  = 1 << 0,
	XBEE = 1 << 1,
	SD   = 1 << 2,
};

struct message_detail {
	enum message_id id;
	uint8_t len;
	uint8_t transport;
};

#define message_info(type) ((const struct message_detail []) { \
	{ .id = TRANSPORT_TEST_SHORT,    .len =  6,    .transport = CAN             }, \
	{ .id = TRANSPORT_TEST_LONG,     .len = 27,    .transport = CAN             }, \
	{ .id = PADDLE_STATUS,           .len =  1,    .transport = CAN             }, \
	{ .id = GPS_DATA,                .len = 13,    .transport = CAN | XBEE | SD }, \
	\
	{ .id = ECU_DATA_PKT,                      .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + FUEL_PRESSURE,      .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_LAP_COUNT,   .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_INJ_SUM,     .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + LAST_GEAR_SHIFT,    .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + MOTOR_OILTEMP,      .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + OIL_PRESSURE,       .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_TIME,        .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_LAP_TIME,    .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + GEAR_OIL_TEMP,      .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_TRACTION,    .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_GAS,         .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_LAMBDA_V2,   .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_CAM_TRIG_P1, .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_CAM_TRIG_P2, .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_CHOKER_ADD,  .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + STATUS_LAMBDA_PWM,  .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + WATER_TEMP,         .len =  4, .transport = CAN | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + MANIFOLD_AIR_TEMP,  .len =  4, .transport = 0   | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + SPEEDER_POTMETER,   .len =  4, .transport = 0   | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + RPM,                .len =  4, .transport = CAN | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + TRIGGER_ERR,        .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + CAM_ANGLE1,         .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + CAM_ANGLE2,         .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + ROAD_SPEED,         .len =  4, .transport = 0   | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + MAP_SENSOR,         .len =  4, .transport = 0   | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + BATTERY_V,          .len =  4, .transport = CAN | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + LAMBDA_V,           .len =  4, .transport = 0   | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + LOAD,               .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + INJECTOR_TIME,      .len =  4, .transport = 0 }, \
	{ .id = ECU_DATA_PKT + IGNITION_TIME,      .len =  4, .transport = 0 }, \
	{ .id = ECU_DATA_PKT + DWELL_TIME,         .len =  4, .transport = 0 }, \
	{ .id = ECU_DATA_PKT + GX,                 .len =  4, .transport = 0   | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + GY,                 .len =  4, .transport = 0   | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + GZ,                 .len =  4, .transport = 0   | XBEE | SD }, \
	{ .id = ECU_DATA_PKT + MOTOR_FLAGS,        .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + OUT_BITS,           .len =  4, .transport = NONE }, \
	{ .id = ECU_DATA_PKT + TIME,               .len =  4, .transport = NONE }, \
	\
	{ .id = CURRENT_GEAR,            .len =  1,    .transport = CAN | XBEE | SD }, \
	{ .id = NEUTRAL_ENABLED,         .len =  1,    .transport = CAN | XBEE | SD }, \
	{ .id = 0,                       .len =  0,    .transport = NONE            }, \
}[type])

#endif /* SYSTEM_MESSAGES_H */
