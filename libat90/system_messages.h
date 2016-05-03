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

#include <stdbool.h>

#include "../nodes/ComNode/ecu.h"
#include "../nodes/ComNode/xbee.h"


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
	TIME_SYNC,

	SYSTIME,

	// end
	END_OF_LIST,
};


enum medium {
	CAN  = 1 << 0,
	XBEE = 1 << 1,
	SD   = 1 << 2,
};


struct message_detail {
	const unsigned len : 4;
	const unsigned transport : 3;
	unsigned subscribed : 1;
};


void can_subscribe(enum message_id id);
void can_unsubscribe(enum message_id id);
void can_subscribe_all(void);
void can_unsubscribe_all(void);
bool can_is_subscribed(enum message_id id);
uint8_t can_msg_length(enum message_id id);
uint8_t can_msg_transport(enum message_id id);

#endif /* SYSTEM_MESSAGES_H */
