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
	[ECU_EMPTY]                    = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_FUEL_PRESSURE]            = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_LAP_COUNT]         = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_INJ_SUM]           = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_LAST_GEAR_SHIFT]          = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_MOTOR_OILTEMP]            = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_OIL_PRESSURE]             = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_TIME]              = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_LAP_TIME]          = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_GEAR_OIL_TEMP]            = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_TRACTION]          = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_GAS]               = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_LAMBDA_V2]         = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_CAM_TRIG_P1]       = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_CAM_TRIG_P2]       = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_CHOKER_ADD]        = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_STATUS_LAMBDA_PWM]        = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_WATER_TEMP]               = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_MANIFOLD_AIR_TEMP]        = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_SPEEDER_POTMETER]         = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_RPM]                      = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_TRIGGER_ERR]              = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_CAM_ANGLE1]               = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_CAM_ANGLE2]               = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_ROAD_SPEED]               = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_MAP_SENSOR]               = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_BATTERY_V]                = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_LAMBDA_V]                 = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_LOAD]                     = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_INJECTOR_TIME]            = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_IGNITION_TIME]            = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_DWELL_TIME]               = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_GX]                       = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_GY]                       = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_GZ]                       = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_MOTOR_FLAGS]              = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_OUT_BITS]                 = { .subscribed = false,    .len = 4,    .transport = 0 },
	[ECU_TIME]                     = { .subscribed = false,    .len = 4,    .transport = 0 },

	[GPS_DATA]                     = { .subscribed = false,    .len = 8,    .transport = 0 },
	[PADDLE_STATUS]                = { .subscribed = false,    .len = 1,    .transport = 0 },
	[FRONT_RIGHT_WHEEL_SPEED]      = { .subscribed = false,    .len = 4,    .transport = 0 },
	[FRONT_LEFT_WHEEL_SPEED]       = { .subscribed = false,    .len = 4,    .transport = 0 },

	[HEARTBEAT]                    = { .subscribed = false,    .len = 1,    .transport = 0 },
	[NODE_STATUS]                  = { .subscribed = false,    .len = 2,    .transport = 0 },
	[CURRENT_GEAR]                 = { .subscribed = false,    .len = 1,    .transport = 0 },
	[NEUTRAL_ENABLED]              = { .subscribed = false,    .len = 1,    .transport = 0 },
	[SYSTIME]                      = { .subscribed = false,    .len = 4,    .transport = 0 },
};


void can_subscribe(enum message_id id) {
	message_info[(uint16_t)id].subscribed = true;
}


void can_unsubscribe(enum message_id id) {
	message_info[(uint16_t)id].subscribed = false;
}


/**
 * Subscribe to to all messages.
 */
void can_subscribe_all(void) {
	for (uint16_t i = 0; i < END_OF_LIST; ++i) {
		message_info[i].subscribed = true;
	}
}


/**
 * Unsubscribe to to all messages.
 */
void can_unsubscribe_all(void) {
	for (uint16_t i = 0; i < END_OF_LIST; ++i) {
		message_info[i].subscribed = false;
	}
}


bool can_is_subscribed(enum message_id id) {
	return message_info[(uint16_t)id].subscribed;
}


uint8_t can_msg_length(enum message_id id) {
	return message_info[(uint16_t)id].len;
}


uint8_t can_msg_transport(enum message_id id) {
	return message_info[(uint16_t)id].transport;
}


void set_msg_transport(enum message_id id, enum medium t) {
	message_info[(uint16_t)id].transport |= t;
}


void clear_msg_transport(enum message_id id, enum medium t) {
	message_info[(uint16_t)id].transport &= ~t;
}
