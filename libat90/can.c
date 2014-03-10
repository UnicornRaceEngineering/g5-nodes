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
* @file can.c
* @brief
*	Used for setting up the CAN subsystem
*	and sending or receiving via the CAN
*/

#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "bitwise.h"
#include "can.h"

static canit_callback_t canit_callback[NB_CANIT_CB] = {NULL};
static ovrit_callback_t ovrit_callback = NULL;

void set_canit_callback(enum can_int_t interrupt, canit_callback_t callback) {
	canit_callback[interrupt] = callback;
}

int can_setup(can_msg_t *msg) {
	CAN_SET_MOB(msg->mob); // Move CANPAGE point the the given mob
	switch(msg->mode) {
		case MOB_DISABLED:
			MOB_ABORT();
			break;
		case MOB_TRANSMIT:

			break;
		case MOB_RECIEVE:
			MOB_SET_STD_ID(msg->id);
			MOB_SET_STD_FILTER_FULL();
			//MOB_CONFIG_RX(); // OSBS!! we are configuring specifically for mode MOB_RECIEVE
			Can_config_rx();
			//CAN_ENABLE_MOB_INTERRUPT(msg->mob);
			Can_set_mob_int(msg->mob);
			break;
		case MOB_AUTOMATIC_REPLY:
			break;
		case MOB_FRAME_BUFF_RECEIVE:
			break;
		default:
			return 1;
			break;
	}
	return 0;
}

/**
* @todo
*	We should consider whether it's necessary
*	to should disable can interrupts during
*	sending and receiving to not mess up message
*	handling.
*/
int can_receive(can_msg_t *msg){

	//!< @todo The CANSTMOB is checked already in ISR. Does it make sense to clear interrupt status?
	if ( !((CANSTMOB == MOB_RX_COMPLETED_DLCW) || (CANSTMOB == MOB_RX_COMPLETED)) ) {
		MOB_CLEAR_INT_STATUS();
		return 2;
	}

	/**
	* @todo
	*	This works well enough when we know length
	*	of the message.
	*	But what about when it is different from
	*	what we expect?
	*/
	msg->dlc = MOB_GET_DLC(); 			// Fill in the msg dlc
	MOB_RX_DATA(msg->data, msg->dlc);	// Fill in the msg data
	MOB_CLEAR_INT_STATUS(); 	// and reset MOb status
	MOB_EN_RX(); 				// re-enable reception. We keep listning for this msg
	return 0;
}

int can_send(can_msg_t *msg) {
	CAN_SET_MOB(msg->mob);
	MOB_SET_STD_ID(msg->id);
	MOB_SET_DLC(msg->dlc); // Set the expected payload length
	MOB_TX_DATA(msg->data, msg->dlc);
	Can_config_tx();
	CAN_ENABLE_MOB_INTERRUPT(msg->mob);
	return CANSTMOB;
}

/*
 * The Can_clear_mob() function clears the following registers:
 * CANSTMOB 			-- Contains interrupt status
 * CANCDMOB 			-- Defines MOB mode and msg length
 * CANIDT1 ... CANIDT4	-- CAN Identifier Tag Registers
 * CANIDM1 ... CANIDT4	-- CAN Identifier Mask Registers
 */
	//Can_clear_rtr();							/* no remote transmission request */
	//Can_set_rtrmsk();							/* Remote Transmission Request - comparison true forced */
	//Can_set_idemsk();							/* Identifier Extension - comparison true forced */
	//clear_mob_status(mob);					/* Described above */

ISR (CANIT_vect) {
	uint8_t mob;

	// Loop over each MOB and check if it have pending interrupt
	for (mob = 0; mob <= LAST_MOB_NB; mob++) {
		if (MOB_HAS_PENDING_INT(mob)) { /* True if mob have pending interrupt */
			CAN_SET_MOB(mob); // Switch to mob

			switch (CANSTMOB) {
				case MOB_RX_COMPLETED_DLCW:
					if ( canit_callback[CANIT_RX_COMPLETED_DLCW] != NULL )
						(*canit_callback[CANIT_RX_COMPLETED_DLCW])(mob);
					// Fall through to MOB_RX_COMPLETED on purpose
					//!< @todo NEEDS TESTING. !!!URGENT!!!
				case MOB_RX_COMPLETED:
					if ( canit_callback[CANIT_RX_COMPLETED] != NULL )
						(*canit_callback[CANIT_RX_COMPLETED])(mob);
					break;
				case MOB_TX_COMPLETED:
					if ( canit_callback[CANIT_TX_COMPLETED] != NULL )
						(*canit_callback[CANIT_TX_COMPLETED])(mob);
					break;
				case MOB_ACK_ERROR:
					if ( canit_callback[CANIT_ACK_ERROR] != NULL )
						(*canit_callback[CANIT_ACK_ERROR])(mob);
					break;
				case MOB_FORM_ERROR:
					if ( canit_callback[CANIT_FORM_ERROR] != NULL )
						(*canit_callback[CANIT_FORM_ERROR])(mob);
					break;
				case MOB_CRC_ERROR:
					if ( canit_callback[CANIT_CRC_ERROR] != NULL )
						(*canit_callback[CANIT_CRC_ERROR])(mob);
					break;
				case MOB_STUFF_ERROR:
					if ( canit_callback[CANIT_STUFF_ERROR] != NULL )
						(*canit_callback[CANIT_STUFF_ERROR])(mob);
					break;
				case MOB_BIT_ERROR:
					if ( canit_callback[CANIT_BIT_ERROR] != NULL )
						(*canit_callback[CANIT_BIT_ERROR])(mob);
					break;
				default:
					if ( canit_callback[CANIT_DEFAULT] != NULL )
						(*canit_callback[CANIT_DEFAULT])(mob);
					break;
			}
		}
	}
}

ISR (OVRIT_vect) {
	if (ovrit_callback != NULL)
		(*ovrit_callback)();
}
