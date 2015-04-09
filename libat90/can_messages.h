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

enum node_id {
	COM_NODE,
	GEAR_NODE,
	STEERING_NODE,
	GPS_NODE,
};

enum message_id {
	PADDLE_STATUS,
	TRANSPORT_TEST_MSG,
	ENGINE_RPM,
	GPS_DATA,
};

struct message_detail {
	uint16_t id;
	uint16_t len;
};

#define message_info(type) ((const struct message_detail []) { \
	[PADDLE_STATUS] 	 = {.id =  1, .len = 27}, \
	[TRANSPORT_TEST_MSG] = {.id =  2, .len =  4}, \
	[ENGINE_RPM] 	 	 = {.id = 28, .len =  1}, \
	[GPS_DATA]			 = {.id =  4, .len = 13}, \
}[type])

#endif /* CAN_TRANSPORT_H */
