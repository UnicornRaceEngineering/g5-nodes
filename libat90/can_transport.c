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
 *
 * @TODO maybe we need some rolling buffer for incoming and outgoing msgs.
 * @note Only reliably handles messages that fits in a single can message.
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include "can.h"
#include "can_transport.h"
#include "system_messages.h"


/**
 * Initializes the node so it is ready to communicate on the network
 * @note this should only be called once.
 * @param  id ID of the node that is initialized
 * @return    A pointer to the node handle
 */
void init_can_node(enum node_id node) {
	can_filter_t filter1 = {.lower_bound = filter_info(PUBLIC).lower_bound,
							.upper_bound = filter_info(PUBLIC).upper_bound };
	can_filter_t filter2 = {.lower_bound = filter_info(node).lower_bound,
							.upper_bound = filter_info(node).upper_bound };
	can_init(filter1, filter2);
}


/**
 * Broadcast a message to the network
 * @param  id   The id of the message that should be send
 * @param  data a pointer to the raw data that should be send
 * @return      Non zero on error.
 */
uint8_t can_broadcast(const enum message_id type, void * const data) {
	return can_send(MESSAGE_INFO(type).can_id, MESSAGE_INFO(type).len, data);
}


struct can_message read_inbox() {
	struct can_message msg;
	read_message(&msg.id, &msg.len, msg.data);
	uint16_t index = 0;
	do {
		const uint16_t can_id = MESSAGE_INFO(index).can_id;
		if (can_id == msg.id) {
			msg.id = index;
			break;
		}
	} while(index++ != END_OF_LIST);

	return msg;
}


bool can_has_data() {
	return !inbox_empty();
}
