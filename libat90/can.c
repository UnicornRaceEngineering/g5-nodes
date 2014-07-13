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
*   Used for setting up the CAN subsystem
*   and sending or receiving via the CAN
*/

#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "bitwise.h"
#include "can.h"

static int8_t set_can_config(const uint32_t baudrate);
static int8_t set_can_timings(const uint8_t prescalar, const uint8_t Tbit);

static canit_callback_t canit_callback[NB_CANIT_CB] = {NULL};
static ovrit_callback_t ovrit_callback = NULL;


static int8_t set_can_config(const uint32_t baudrate) {
	if ((F_CPU % baudrate) != 0) return 1;

	const uint16_t clks_pr_bit = F_CPU / baudrate;

	// As per CAN spec Tbit must be must at least from 8 to 25
	for (uint8_t Tbit = 8; Tbit <= 25; ++Tbit) {

		// Make sure the prescalar is a whole integer with no remainder
		if ((clks_pr_bit % Tbit) != 0) continue;
		const uint8_t prescalar = clks_pr_bit / Tbit;

		// Prescalar (BRP[5..0]) is a 6 bit value so it cant be bigger than 2^6
		if (prescalar > (1<<6)) continue;

		return set_can_timings(prescalar, Tbit);
	}

	return 1;
}

static int8_t set_can_timings(const uint8_t prescalar, const uint8_t Tbit) {
	const uint8_t Tsyns = 1; // Tsyns is always 1 TQ
	const uint8_t Tprs = IS_ODD(Tbit)				? ((Tbit-1)/2) : (Tbit/2);
	const uint8_t Tph1 = IS_ODD(Tbit-Tprs-Tsyns)	? ((Tprs/2)+1) : (Tprs/2);
	const uint8_t Tph2 = Tprs/2; // We round down to nearest int
	const uint8_t Tsjw = 1; // can vary from 1 to 4 but is 1 in all avr examples

	// Sanity check
	if (Tbit != Tsyns+Tprs+Tph1+Tph2
		|| !(1 <= Tprs && Tprs <= 8)
		|| !(1 <= Tph1 && Tph1 <= 8)
		|| !(2 <= Tph2 && Tph2 <= Tph1)) return 1;

	SET_REGISTER_BITS(CANBT1, (prescalar-1)<<BRP0,
		(1<<BRP5|1<<BRP4|1<<BRP3|1<<BRP2|1<<BRP1|1<<BRP0));
	SET_REGISTER_BITS(CANBT2, (Tprs-1)<<PRS0, (1<<PRS0|1<<PRS1|1<<PRS2));
	SET_REGISTER_BITS(CANBT2, (Tsjw-1)<<SJW0, (1<<SJW0|1<<SJW1));
	SET_REGISTER_BITS(CANBT3, (Tph1-1)<<PHS10, (1<<PHS10|1<<PHS11|1<<PHS12));
	SET_REGISTER_BITS(CANBT3, (Tph2-1)<<PHS20, (1<<PHS20|1<<PHS21|1<<PHS22));

	return 0;
}

void set_canit_callback(enum can_int_t interrupt, canit_callback_t callback) {
	canit_callback[interrupt] = callback;
}

uint8_t can_init(void) {
	CAN_RESET();

	/*
	The CPU freq is 11059200 so with a baud-rate of 204800 one get exactly
	11059200 % 204800 = 0 which means the CPU freq is divisible with the
	baud-rate.
	CPU freq / baud-rate = clock cycles per bit transmitted = 54
	setting the prescalar to 6
	and gives Tbit value of 9
	because: (clock cycles per bit transmitted) / Tbit = prescalar
	and because Tbit = Tsync + Tprs + Tph1 + Tph2
	we get:
	Tprs = 4, Tph1 = 2 and Tph2 = 2
	which is set in the follow register values.
	(because 11059200 % 204800 = 0 we get a timing error = 0)
	 */
	if (set_can_config(CAN_BAUDRATE) != 0) return 1;

	// It reset CANSTMOB, CANCDMOB, CANIDTx & CANIDMx and clears data FIFO of
	// MOb[0] upto MOb[LAST_MOB_NB].
	for (uint8_t mob_number = 0; mob_number < NB_MOB; ++mob_number) {
		CANPAGE = (mob_number << 4);	// Page index
		MOB_CLEAR_STATUS();				// All MOb Registers=0
	}

	CAN_ENABLE();
	return 0;
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
		MOB_SET_DLC(msg->dlc); // Set the expected payload length
		MOB_EN_RX();
		CAN_ENABLE_MOB_INTERRUPT(msg->mob);
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

int can_receive(can_msg_t *msg) {
	msg->dlc = MOB_GET_DLC();           // Fill in the msg dlc
	MOB_RX_DATA(msg->data, msg->dlc);   // Fill in the msg data
	MOB_CLEAR_INT_STATUS();     // and reset MOb status
	MOB_EN_RX();                // re-enable reception. We keep listning for this msg
	return 0;
}

int can_send(can_msg_t *msg) {
	CAN_SET_MOB(msg->mob);
	MOB_SET_STD_ID(msg->id);
	MOB_SET_DLC(msg->dlc); // Set the expected payload length
	MOB_TX_DATA(msg->data, msg->dlc);
	MOB_EN_TX();
	CAN_ENABLE_MOB_INTERRUPT(msg->mob);
	return CANSTMOB;
}

/*
 * The Can_clear_mob() function clears the following registers:
 * CANSTMOB             -- Contains interrupt status
 * CANCDMOB             -- Defines MOB mode and msg length
 * CANIDT1 ... CANIDT4  -- CAN Identifier Tag Registers
 * CANIDM1 ... CANIDT4  -- CAN Identifier Mask Registers
 */
	//Can_clear_rtr();                          /* no remote transmission request */
	//Can_set_rtrmsk();                         /* Remote Transmission Request - comparison true forced */
	//Can_set_idemsk();                         /* Identifier Extension - comparison true forced */
	//clear_mob_status(mob);                    /* Described above */

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
