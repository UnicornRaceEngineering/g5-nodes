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
	ECU_DATA_PKT        = 258,

	// Gear node
	PADDLE_STATUS       = 512,

	// Steerinng node
	CURRENT_GEAR        = 768,
	NEUTRAL_ENABLED     = 769,
};

struct message_detail {
	enum message_id id;
	uint16_t len;
};

#define message_info(type) ((const struct message_detail []) { \
	{ .id = TRANSPORT_TEST_SHORT	, .len =  6 }, \
	{ .id = TRANSPORT_TEST_LONG		, .len = 27 }, \
	{ .id = PADDLE_STATUS			, .len =  1 }, \
	{ .id = GPS_DATA				, .len = 13 }, \
	{ .id = ECU_DATA_PKT			, .len =  5 }, \
	{ .id = CURRENT_GEAR			, .len =  1 }, \
	{ .id = NEUTRAL_ENABLED			, .len =  1 }, \
	{ .id = 0						, .len =  0 }, \
}[type])

#endif /* CAN_TRANSPORT_H */
