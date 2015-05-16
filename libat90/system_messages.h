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
	// Com node
	ECU_PKT,
	LAST_ECU_PKT = N_ECU_IDS - 1,

	HEARTBEAT,
	GPS_DATA,

	// Gear node
	PADDLE_STATUS,

	// Steerinng node
	CURRENT_GEAR,
	NEUTRAL_ENABLED,

	// Public
	TRANSPORT_TEST_SHORT,
	TRANSPORT_TEST_LONG,
	ANNOUNCE,
	TIME_SYNC,

	// end
	END_OF_LIST,
};


enum medium {
	NONE = 0,
	CAN  = 1 << 0,
	XBEE = 1 << 1,
	SD   = 1 << 2,
};


struct message_detail {
	const uint16_t id;
	const uint8_t len;
	const uint8_t transport;
};


#define message_info(type) ((const struct message_detail []) { \
	[TRANSPORT_TEST_SHORT]         = { .id =   1,    .len =  6,    .transport = 0 | CAN             }, \
	[TRANSPORT_TEST_LONG]          = { .id =   2,    .len = 27,    .transport = 0 | CAN             }, \
	[GPS_DATA]                     = { .id = 256,    .len = 13,    .transport = 0 | CAN | XBEE | SD }, \
	\
	[ECU_PKT + EMPTY]              = { .id = 257,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + FUEL_PRESSURE]      = { .id = 258,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_LAP_COUNT]   = { .id = 259,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_INJ_SUM]     = { .id = 260,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + LAST_GEAR_SHIFT]    = { .id = 261,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + MOTOR_OILTEMP]      = { .id = 262,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + OIL_PRESSURE]       = { .id = 263,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_TIME]        = { .id = 264,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_LAP_TIME]    = { .id = 265,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + GEAR_OIL_TEMP]      = { .id = 266,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_TRACTION]    = { .id = 267,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_GAS]         = { .id = 268,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_LAMBDA_V2]   = { .id = 269,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_CAM_TRIG_P1] = { .id = 270,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_CAM_TRIG_P2] = { .id = 271,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_CHOKER_ADD]  = { .id = 272,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_LAMBDA_PWM]  = { .id = 273,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + WATER_TEMP]         = { .id = 274,    .len =  4,    .transport = 0 | CAN | XBEE | SD }, \
	[ECU_PKT + MANIFOLD_AIR_TEMP]  = { .id = 275,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + SPEEDER_POTMETER]   = { .id = 276,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + RPM]                = { .id = 277,    .len =  4,    .transport = 0 | CAN | XBEE | SD }, \
	[ECU_PKT + TRIGGER_ERR]        = { .id = 278,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + CAM_ANGLE1]         = { .id = 279,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + CAM_ANGLE2]         = { .id = 280,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + ROAD_SPEED]         = { .id = 281,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + MAP_SENSOR]         = { .id = 282,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + BATTERY_V]          = { .id = 283,    .len =  4,    .transport = 0 | CAN | XBEE | SD }, \
	[ECU_PKT + LAMBDA_V]           = { .id = 284,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + LOAD]               = { .id = 285,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + INJECTOR_TIME]      = { .id = 286,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + IGNITION_TIME]      = { .id = 287,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + DWELL_TIME]         = { .id = 288,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + GX]                 = { .id = 289,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + GY]                 = { .id = 290,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + GZ]                 = { .id = 291,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + MOTOR_FLAGS]        = { .id = 292,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + OUT_BITS]           = { .id = 293,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + TIME]               = { .id = 294,    .len =  4,    .transport = 0                   }, \
	\
	[PADDLE_STATUS]                = { .id = 512,    .len =  1,    .transport = 0 | CAN             }, \
	[CURRENT_GEAR]                 = { .id = 768,    .len =  1,    .transport = 0 | CAN | XBEE | SD }, \
	[NEUTRAL_ENABLED]              = { .id = 769,    .len =  1,    .transport = 0 | CAN | XBEE | SD }, \
	\
	[END_OF_LIST]                  = { .id =   0,    .len =  0,    .transport = 0                   }, \
}[type])

#endif /* SYSTEM_MESSAGES_H */
