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
 * @files can_messages.h
 * Defines the message types for the high-level can transmission.
 */

#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

#include <stdint.h>

enum node_id {
	COM_NODE,
	GEAR_NODE,
	STEERING_NODE,
	GPS_NODE,
};

enum message_id {
	TRANSPORT_TEST_MSG 	= 2,
	ENGINE_RPM 			= 28,
	GPS_DATA 			= 4,
	PADDLE_STATUS,
	ECU_DATA_PKT
};

struct message_detail {
	enum message_id id;
	uint16_t len;
};

#define message_info(type) ((const struct message_detail []) { \
	{ .id = PADDLE_STATUS			, .len = 1 }, \
	{ .id = TRANSPORT_TEST_MSG		, .len =  4 }, \
	{ .id = GPS_DATA				, .len = 13 }, \
	{ .id = 0						, .len =  0 }, \
}[type])

#endif /* CAN_TRANSPORT_H */
