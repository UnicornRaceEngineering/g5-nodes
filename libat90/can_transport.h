/*
The MIT License (MIT)

Copyright (c) 2015 UnicornRaceEngineering

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
 * @files can_transport.h
 * Higher level transport protocol using the CAN interface
 */

#ifndef CAN_TRANSPORT_H
#define CAN_TRANSPORT_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

enum node_id {
	COM_NODE,
	GEAR_NODE,
	STEERING_NODE,

	CAN_TRANSPORT_EXAMPLE_NODE,

	NUMBER_OF_NODES,
};

enum message_id {
	PADDLE_STATUS,
	TRANSPORT_TEST_MSG,

	ENGINE_RPM,

	NUMBER_OF_MESSAGES,
	NO_MESSAGE,
};

#define MSG_INIT_VALS(MSG_ID, LEN) {.id = MSG_ID, .payload = {.len = LEN}, .tx_cnt = 0, .rx_cnt = 0, .complete = false}
#define MESSAGE(msg_id) ((const struct can_message[]) { \
		[NO_MESSAGE] = MSG_INIT_VALS(NO_MESSAGE, 1), \
		[PADDLE_STATUS] = MSG_INIT_VALS(PADDLE_STATUS, 1), \
		[TRANSPORT_TEST_MSG] = MSG_INIT_VALS(TRANSPORT_TEST_MSG, 15), \
		[ENGINE_RPM] = MSG_INIT_VALS(ENGINE_RPM, sizeof(int16_t)), \
	}[msg_id])



#define MAX_BROADCASTABLE_MSGS   2
#define BROADCASTABLE_MESSEGES(node_id) ((const struct can_message[NUMBER_OF_NODES][MAX_BROADCASTABLE_MSGS]) { \
		[STEERING_NODE] = { \
			MESSAGE(PADDLE_STATUS), \
			MESSAGE(NO_MESSAGE), \
		}, \
		[CAN_TRANSPORT_EXAMPLE_NODE] = { \
			MESSAGE(TRANSPORT_TEST_MSG), \
			MESSAGE(NO_MESSAGE), \
		}, \
	}[node_id])

#define MAX_SUBSCRIPEABLE_MSGS 3
#define SUBSCRIPEABLE_MESSEGES(node_id) ((const struct can_message[NUMBER_OF_NODES][MAX_SUBSCRIPEABLE_MSGS]) { \
		[STEERING_NODE] = { \
			MESSAGE(ENGINE_RPM), \
			MESSAGE(NO_MESSAGE), \
		}, \
		[CAN_TRANSPORT_EXAMPLE_NODE] = { \
			MESSAGE(PADDLE_STATUS), \
			MESSAGE(TRANSPORT_TEST_MSG), \
			MESSAGE(NO_MESSAGE), \
		} \
	}[node_id])

struct can_message {
	enum message_id id;

	struct {
		uint8_t *data;
		size_t len;
	} payload;

	int remaining; //!< how many msges remaining begore we complete tthe payload

	uint8_t tx_cnt;
	uint8_t rx_cnt;

	bool complete;
};

struct can_node {
	enum node_id id;

	//!< List of messages the will be listing for
	struct can_message msg_subscriped[MAX_SUBSCRIPEABLE_MSGS];
	size_t n_msg_subscriped;

	//!< List of messages the node will be broadcasting
	struct can_message msg_broadcasting[MAX_BROADCASTABLE_MSGS];
	size_t n_msg_broadcasting;
};

struct can_node* init_can_node(enum node_id id);
int can_broadcast(enum message_id id, uint8_t *data);

#endif /* CAN_TRANSPORT_H */
