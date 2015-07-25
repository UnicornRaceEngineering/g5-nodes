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

	SENSOR_FRONT_NODE,

	SPY_NODE,
	TEST_NODE,

	N_NODES,
};


struct can_filter {
	uint16_t lower_bound;
	uint16_t upper_bound;
};


#define filter_info(node_id) ((const struct can_filter []) { \
	[PUBLIC]        = { .lower_bound =    0	, .upper_bound =  255 }, \
	[COM_NODE]      = { .lower_bound =  256	, .upper_bound = 2047 }, \
	[GEAR_NODE]     = { .lower_bound =  512	, .upper_bound =  770 }, \
	[STEERING_NODE] = { .lower_bound =  257	, .upper_bound = 1023 }, \
	[GPS_NODE]      = { .lower_bound = 1024	, .upper_bound = 1275 }, \
	[SENSOR_FRONT_NODE] = { .lower_bound = 1276	, .upper_bound = 1276+255 }, \
	[TEST_NODE]     = { .lower_bound = 1275	, .upper_bound = 1530 }, \
	[SPY_NODE]      = { .lower_bound =  256	, .upper_bound = 2047 }, \
}[node_id])


enum message_id {
	// Com node
	ECU_PKT,
	LAST_ECU_PKT = N_ECU_IDS - 1,

	HEARTBEAT,
	GPS_DATA,
	NODE_STATUS,

	// Gear node
	PADDLE_STATUS,

	// Steerinng node
	CURRENT_GEAR,
	NEUTRAL_ENABLED,

	// WHEEL Sensors
	FRONT_RIGHT_WHEEL_SPEED,
	FRONT_LEFT_WHEEL_SPEED,

	// Public
	TRANSPORT_TEST_SHORT,
	TRANSPORT_TEST_LONG,
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
	const uint16_t can_id; // Should this be CAN_id?
	const uint8_t len;
	const uint8_t transport;
};


#define MESSAGE_INFO(type) ((const struct message_detail []) { \
	[TRANSPORT_TEST_SHORT]         = { .can_id =   1,    .len =  6,    .transport = 0 | CAN             }, \
	[TRANSPORT_TEST_LONG]          = { .can_id =   2,    .len = 27,    .transport = 0 | CAN             }, \
	[TIME_SYNC]                    = { .can_id =   3,    .len =  4,    .transport = 0 | CAN             }, \
	\
	[ECU_PKT + EMPTY]              = { .can_id = 257,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + FUEL_PRESSURE]      = { .can_id = 258,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_LAP_COUNT]   = { .can_id = 259,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_INJ_SUM]     = { .can_id = 260,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + LAST_GEAR_SHIFT]    = { .can_id = 261,    .len =  4,    .transport = 0 |      XBEE       }, \
	[ECU_PKT + MOTOR_OILTEMP]      = { .can_id = 262,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + OIL_PRESSURE]       = { .can_id = 263,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_TIME]        = { .can_id = 264,    .len =  4,    .transport = 0 |              SD }, \
	[ECU_PKT + STATUS_LAP_TIME]    = { .can_id = 265,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + GEAR_OIL_TEMP]      = { .can_id = 266,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_TRACTION]    = { .can_id = 267,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_GAS]         = { .can_id = 268,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_LAMBDA_V2]   = { .can_id = 269,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_CAM_TRIG_P1] = { .can_id = 270,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_CAM_TRIG_P2] = { .can_id = 271,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + STATUS_CHOKER_ADD]  = { .can_id = 272,    .len =  4,    .transport = 0 | CAN | XBEE      }, \
	[ECU_PKT + STATUS_LAMBDA_PWM]  = { .can_id = 273,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + WATER_TEMP]         = { .can_id = 274,    .len =  4,    .transport = 0 | CAN | XBEE | SD }, \
	[ECU_PKT + MANIFOLD_AIR_TEMP]  = { .can_id = 275,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + SPEEDER_POTMETER]   = { .can_id = 276,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + RPM]                = { .can_id = 277,    .len =  4,    .transport = 0 | CAN | XBEE | SD }, \
	[ECU_PKT + TRIGGER_ERR]        = { .can_id = 278,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + CAM_ANGLE1]         = { .can_id = 279,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + CAM_ANGLE2]         = { .can_id = 280,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + ROAD_SPEED]         = { .can_id = 281,    .len =  4,    .transport = 0       | XBEE | SD }, /* Check if this contains any data */ \
	[ECU_PKT + MAP_SENSOR]         = { .can_id = 282,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + BATTERY_V]          = { .can_id = 283,    .len =  4,    .transport = 0 | CAN | XBEE | SD }, \
	[ECU_PKT + LAMBDA_V]           = { .can_id = 284,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + LOAD]               = { .can_id = 285,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + INJECTOR_TIME]      = { .can_id = 286,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + IGNITION_TIME]      = { .can_id = 287,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + DWELL_TIME]         = { .can_id = 288,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + GX]                 = { .can_id = 289,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + GY]                 = { .can_id = 290,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + GZ]                 = { .can_id = 291,    .len =  4,    .transport = 0       | XBEE | SD }, \
	[ECU_PKT + MOTOR_FLAGS]        = { .can_id = 292,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + OUT_BITS]           = { .can_id = 293,    .len =  4,    .transport = 0                   }, \
	[ECU_PKT + TIME]               = { .can_id = 294,    .len =  4,    .transport = 0                   }, \
	\
	[GPS_DATA]                     = { .can_id = 295,    .len = 13,    .transport = 0 | CAN | XBEE | SD }, \
	[HEARTBEAT]                    = { .can_id = 296,    .len =  1,    .transport = 0 | CAN | XBEE | SD }, \
	[NODE_STATUS]                  = { .can_id = 297,    .len =  2,    .transport = 0       | XBEE | SD }, \
	[PADDLE_STATUS]                = { .can_id = 512,    .len =  1,    .transport = 0 | CAN             }, \
	[CURRENT_GEAR]                 = { .can_id = 768,    .len =  1,    .transport = 0 | CAN | XBEE | SD }, \
	[NEUTRAL_ENABLED]              = { .can_id = 769,    .len =  1,    .transport = 0 | CAN | XBEE | SD }, \
	[FRONT_RIGHT_WHEEL_SPEED]      = { .can_id = 800,    .len =  4,    .transport = 0 | CAN | XBEE | SD }, \
	[FRONT_LEFT_WHEEL_SPEED]       = { .can_id = 801,    .len =  4,    .transport = 0 | CAN | XBEE | SD }, \
	\
	[END_OF_LIST]                  = { .can_id =   0,    .len =  0,    .transport = 0                   }, \
}[type])

#endif /* SYSTEM_MESSAGES_H */
