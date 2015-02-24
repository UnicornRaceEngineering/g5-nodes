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
#include <stdlib.h>
#include <string.h> // memcpy()
#include <stdbool.h>
#include <avr/sfr_defs.h> // loop_until_bit_is_clear()

#include "usart.h"

#include "can.h"
#include "can_transport.h"

#define ARR_LEN(arr)    (sizeof(arr) / sizeof(arr[0]))


#define MAX_DLC         NB_DATA_MAX //!< @TODO remove this !?
#define FREE_DATA_MAX   (MAX_DLC - NUM_RESERVED_INDEXES)

#define MSGS_NEEDED_TO_COMPLETE(msg) ((msg->payload.len / FREE_DATA_MAX) + 1)

enum reserved_indexes {
	RXTX_CNT_IDX,

	NUM_RESERVED_INDEXES,
};

static void rx_complete_cb(uint8_t mob);
static void tx_complete(uint8_t mob);
static void can_default(uint8_t mob);

static struct can_node this_node;
static int available_mobs = LAST_MOB_NB;


/**
 * Setsup the message by filling it with relevant default values and attaching
 * it to the node.
 */
static int setup_message(struct can_message *msg) {
	if (msg->id == NO_MESSAGE) return 1;
	msg->remaining = MSGS_NEEDED_TO_COMPLETE(msg);
	msg->payload.data = calloc(msg->payload.len, sizeof(uint8_t));
	if (msg->payload.data == NULL) return -1; // Out of memory

	return 0;
}

/**
 * Initializes the node so it is ready to communicate on the network
 * @note this should only be called once.
 * @param  id ID of the node that is initialized
 * @return    A pointer to the node handle
 */
struct can_node *init_can_node(enum node_id id) {
	this_node.id = id;
	can_init();

	// Set Broadcasting
	for (int i = 0; i < MAX_BROADCASTABLE_MSGS; ++i) {
		struct can_message *msg = (struct can_message *)&BROADCASTABLE_MESSEGES(id)[i];
		if (setup_message(msg) != 0) break;

		memcpy(&this_node.msg_broadcasting[this_node.n_msg_broadcasting++], msg,
		       sizeof(struct can_message));
	}

	// Set Subscriptions
	for (int i = 0; i < MAX_SUBSCRIPEABLE_MSGS; ++i) {
		struct can_message *msg = (struct can_message *)&SUBSCRIPEABLE_MESSEGES(id)[i];
		if (setup_message(msg) != 0) break;

		can_setup(&(can_msg_t) {
			.mob = available_mobs--, .id = msg->id, .mode = MOB_RECIEVE,
		});

		memcpy(&this_node.msg_subscriped[this_node.n_msg_subscriped++], msg,
		       sizeof(struct can_message));
	}

	set_canit_callback(CANIT_TX_COMPLETED, tx_complete);
	set_canit_callback(CANIT_DEFAULT, can_default);
	set_canit_callback(CANIT_RX_COMPLETED, rx_complete_cb);

	CAN_SEI();
	CAN_EN_RX_INT();
	CAN_EN_TX_INT();

	return &this_node;
}

static void broadcast_msg(struct can_message *msg) {
	int remaining = msg->remaining;

	while (remaining) {
		can_msg_t can_msg = {
			.mob = available_mobs,
			.id = msg->id,
			.data = {
				[RXTX_CNT_IDX] = msg->tx_cnt++,
			},
			.dlc = (remaining > 1) ? MAX_DLC : MAX_DLC - ((MSGS_NEEDED_TO_COMPLETE(msg) * FREE_DATA_MAX) - msg->payload.len),

			.mode = MOB_TRANSMIT
		};

		memcpy(&can_msg.data[NUM_RESERVED_INDEXES],
		       &msg->payload.data[(MSGS_NEEDED_TO_COMPLETE(msg) - remaining--) *
		                          FREE_DATA_MAX],
		       can_msg.dlc * sizeof(can_msg.data[0]));

		loop_until_bit_is_clear(CANGSTA, TXBSY); // Blocking untill msg is send
		can_send(&can_msg);
		// Debug output
#if 0
		{
			usart1_printf("remaining=%d, mob: %d, id: %d, dlc: %d msg.len: %d\n",
			              remaining + 1, can_msg.mob, can_msg.id, can_msg.dlc, msg->payload.len);
			for (int i = 0; i < can_msg.dlc; ++i) {
				usart1_printf("0x%0X ", can_msg.data[i]);
			}
			usart1_printf("\n");
		}
#endif
	}
}

/**
 * Broadcast a message to the network
 * @param  id   The id of the message that should be send
 * @param  data a pointer to the raw data that should be send
 * @return      Non zero on error.
 */
int can_broadcast(enum message_id id, uint8_t *data) {
	for (size_t i = 0; i < this_node.n_msg_broadcasting; ++i) {
		if (this_node.msg_broadcasting[i].id == id) {
			this_node.msg_broadcasting[i].payload.data = data;
			broadcast_msg(&this_node.msg_broadcasting[i]);
			return 0;
		}
	}
	return 1;
}


static void rx_complete_cb(uint8_t mob) {
	can_msg_t can_msg = {.mob = mob};
	can_receive(&can_msg);

	// Debug output
#if 0
	{
		usart1_printf("mob: %d, id: %d, dlc: %d\n", can_msg.mob, can_msg.id,
		              can_msg.dlc);
		for (int i = 0; i < can_msg.dlc; ++i) {
			usart1_printf("0x%0X ", can_msg.data[i]);
		}
		usart1_printf("\n");
	}
#endif

	for (size_t i = 0; i < this_node.n_msg_subscriped; ++i) {
		if (this_node.msg_subscriped[i].id == can_msg.id) {
			struct can_message *msg = &this_node.msg_subscriped[i];
			const int n_to_complete = MSGS_NEEDED_TO_COMPLETE(msg);

			msg->rx_cnt++;
			msg->tx_cnt = can_msg.data[RXTX_CNT_IDX];

			//!> @TODO handle missed packages

			memcpy(&msg->payload.data[(n_to_complete - msg->remaining--) *
			                          FREE_DATA_MAX],
			       &can_msg.data[NUM_RESERVED_INDEXES],
			       can_msg.dlc * sizeof(can_msg.data[0]));

			if (msg->remaining == 0) {
				msg->remaining = n_to_complete;
				msg->complete = true;
			}

			break; // No need to keep looping
		}
	}
}

static void tx_complete(uint8_t mob) {
	MOB_ABORT();                    // Freed the MOB
	MOB_CLEAR_INT_STATUS();         // and reset MOb status
	CAN_DISABLE_MOB_INTERRUPT(mob); // Unset interrupt
}

static void can_default(uint8_t mob) {
	MOB_CLEAR_INT_STATUS();         // and reset MOb status
}
