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
 * @files system_messages.c
 * Contains the actual CAN message type information.
 */

#include <stdbool.h>
#include <stdint.h>

#include "system_messages.h"


struct message_detail message_info[END_OF_LIST] = {
	[TIME_SYNC]                    = { .subscribed = false,    .len = 4,    .transport = 0 | CAN             },
	[ECU_PKT + EMPTY]              = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + FUEL_PRESSURE]      = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_LAP_COUNT]   = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_INJ_SUM]     = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + LAST_GEAR_SHIFT]    = { .subscribed = false,    .len = 4,    .transport = 0 |      XBEE       },
	[ECU_PKT + MOTOR_OILTEMP]      = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + OIL_PRESSURE]       = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_TIME]        = { .subscribed = false,    .len = 4,    .transport = 0 |              SD },
	[ECU_PKT + STATUS_LAP_TIME]    = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + GEAR_OIL_TEMP]      = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_TRACTION]    = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_GAS]         = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_LAMBDA_V2]   = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_CAM_TRIG_P1] = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_CAM_TRIG_P2] = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + STATUS_CHOKER_ADD]  = { .subscribed = false,    .len = 4,    .transport = 0 | CAN | XBEE      },
	[ECU_PKT + STATUS_LAMBDA_PWM]  = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + WATER_TEMP]         = { .subscribed = false,    .len = 4,    .transport = 0 | CAN | XBEE | SD },
	[ECU_PKT + MANIFOLD_AIR_TEMP]  = { .subscribed = false,    .len = 4,    .transport = 0       | XBEE | SD },
	[ECU_PKT + SPEEDER_POTMETER]   = { .subscribed = false,    .len = 4,    .transport = 0       | XBEE | SD },
	[ECU_PKT + RPM]                = { .subscribed = false,    .len = 4,    .transport = 0 | CAN | XBEE | SD },
	[ECU_PKT + TRIGGER_ERR]        = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + CAM_ANGLE1]         = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + CAM_ANGLE2]         = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + ROAD_SPEED]         = { .subscribed = false,    .len = 4,    .transport = 0       | XBEE | SD }, /* Check if this contains any data */
	[ECU_PKT + MAP_SENSOR]         = { .subscribed = false,    .len = 4,    .transport = 0       | XBEE | SD },
	[ECU_PKT + BATTERY_V]          = { .subscribed = false,    .len = 4,    .transport = 0 | CAN | XBEE | SD },
	[ECU_PKT + LAMBDA_V]           = { .subscribed = false,    .len = 4,    .transport = 0       | XBEE | SD },
	[ECU_PKT + LOAD]               = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + INJECTOR_TIME]      = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + IGNITION_TIME]      = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + DWELL_TIME]         = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + GX]                 = { .subscribed = false,    .len = 4,    .transport = 0       | XBEE | SD },
	[ECU_PKT + GY]                 = { .subscribed = false,    .len = 4,    .transport = 0       | XBEE | SD },
	[ECU_PKT + GZ]                 = { .subscribed = false,    .len = 4,    .transport = 0       | XBEE | SD },
	[ECU_PKT + MOTOR_FLAGS]        = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + OUT_BITS]           = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[ECU_PKT + TIME]               = { .subscribed = false,    .len = 4,    .transport = 0                   },
	[GPS_DATA]                     = { .subscribed = false,    .len = 8,    .transport = 0 | CAN | XBEE | SD },
	[HEARTBEAT]                    = { .subscribed = false,    .len = 1,    .transport = 0 | CAN | XBEE | SD },
	[NODE_STATUS]                  = { .subscribed = false,    .len = 2,    .transport = 0       | XBEE | SD },
	[PADDLE_STATUS]                = { .subscribed = false,    .len = 1,    .transport = 0 | CAN             },
	[CURRENT_GEAR]                 = { .subscribed = false,    .len = 1,    .transport = 0 | CAN | XBEE | SD },
	[NEUTRAL_ENABLED]              = { .subscribed = false,    .len = 1,    .transport = 0 | CAN | XBEE | SD },
	[FRONT_RIGHT_WHEEL_SPEED]      = { .subscribed = false,    .len = 4,    .transport = 0 | CAN | XBEE | SD },
	[FRONT_LEFT_WHEEL_SPEED]       = { .subscribed = false,    .len = 4,    .transport = 0 | CAN | XBEE | SD },
};


void can_subscribe(enum medium id) {
	message_info[id].subscribed = true;
}


void can_unsubscribe(enum medium id) {
	message_info[id].subscribed = false;
}


void can_subscribe_all(void) {
	for (uint16_t i = 0; i < END_OF_LIST; ++i) {
		message_info[i].subscribed = true;
	}
}


void can_unsubscribe_all(void) {
	for (uint16_t i = 0; i < END_OF_LIST; ++i) {
		message_info[i].subscribed = false;
	}
}


bool can_is_subscribed(enum medium id) {
	return message_info[id].subscribed;
}


uint8_t can_msg_length(enum medium id) {
	return message_info[id].len;
}


uint8_t can_msg_transport(enum medium id) {
	return message_info[id].transport;
}
