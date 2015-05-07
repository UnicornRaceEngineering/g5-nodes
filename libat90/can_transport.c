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
#include "heap.h"
#include "can.h"
#include "can_transport.h"
#include "can_messages.h"


static struct can_message *oldest_message;
static struct can_message *newest_message;
static volatile uint8_t queue_length;

static uint8_t rx_complete(uint16_t id, uint16_t len, uint8_t *msg);

/**
 * Initializes the node so it is ready to communicate on the network
 * @note this should only be called once.
 * @param  id ID of the node that is initialized
 * @return    A pointer to the node handle
 */
void init_can_node(enum node_id node) {
	oldest_message = 0;
	newest_message = 0;
	queue_length = 0;

	set_canrec_callback(rx_complete);

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
uint8_t can_broadcast(const enum message_id receiver, void * const data) {
	uint16_t i = 0;
	do {
		if (message_info(i).id == receiver)
			return can_send(message_info(i).id, message_info(i).len, data);
	} while (message_info(++i).id);
	return 1;
}


// Callback to be run when rx comletes on the CAN
static uint8_t rx_complete(uint16_t id, uint16_t len, uint8_t *msg) {
	if (queue_length) {
		struct can_message *temp = newest_message;
		newest_message = (struct can_message*)smalloc(sizeof(struct can_message));
		if (!newest_message) {
			return ALLOC_ERR;
		}
		newest_message->info.id = id;
		newest_message->info.len = len;
		newest_message->data = msg;
		newest_message->older_message = temp;
		newest_message->newer_message = 0;
		temp->newer_message = newest_message;
	} else {
		newest_message = (struct can_message*)smalloc(sizeof(struct can_message));
		if (!newest_message) {
			return ALLOC_ERR;
		}
		oldest_message = newest_message;
		newest_message->info.id = id;
		newest_message->info.len = len;
		newest_message->data = msg;
		newest_message->newer_message = 0;
		newest_message->older_message = 0;
	}
	++queue_length;
	return SUCCES;
}

struct can_message* read_inbox(void) {
	if (queue_length) {
		struct can_message *return_message = oldest_message;
		oldest_message = return_message->newer_message;
		oldest_message->older_message = 0;
		--queue_length;
		return return_message;
	} else {
		return 0;
	}
}

uint8_t get_queue_length(void) {
	return queue_length;
}

void can_free(struct can_message* message) {
	sfree((void *) message->data);
	sfree((void *) message);
}
