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
#include <math.h>
#include "bitwise.h"
#include "can.h"
#include "usart.h"

struct time_quantum_config {
	int Tbit;
	int TQ;
	double TQ_err; //!> @todo: Instead of a double just scale to a large int
	int prescalar;
};

static void set_baud(void);

static canit_callback_t canit_callback[NB_CANIT_CB] = {NULL};
static ovrit_callback_t ovrit_callback = NULL;

void set_canit_callback(enum can_int_t interrupt, canit_callback_t callback) {
	canit_callback[interrupt] = callback;
}

/**
* @fn can_init
*
* @brief
*   CAN macro initialization. Reset the CAN peripheral, initialize the bit
*   timing, initialize all the registers mapped in SRAM to put MObs in
*   inactive state and enable the CAN macro.
*
* @warning The CAN macro will be enable after seen on CAN bus a receceive
*   level as long as of an inter frame (hardware feature).
*
* @param  Mode (for "can_fixed_baudrate" param not used)
*   ==0: start CAN bit timing evaluation from faster baudrate
*   ==1: start CAN bit timing evaluation with CANBTx registers
* contents
*
* @return Baudrate Status
*   ==0: research of bit timing configuration failed
*   ==1: baudrate performed
*/

uint8_t can_init(void) {
	uint8_t mob_number;

	CAN_RESET();
	set_baud();

	//It reset CANSTMOB, CANCDMOB, CANIDTx & CANIDMx and clears data FIFO of
	// MOb[0] upto MOb[LAST_MOB_NB].
	for (mob_number = 0; mob_number < NB_MOB; mob_number++) {
		CANPAGE = (mob_number << 4);	// Page index
		MOB_CLEAR_STATUS();				// All MOb Registers=0
	}

	CAN_ENABLE();
	return (0);
}

static int find_best_time_quantum_prescalar(struct time_quantum_config *cfg) {
	int num_valid_cfg = 0;

	// As per CAN spec Tbit must be must at least from 8 to 25
	for (int Tbit = 8; Tbit <= 25; ++Tbit) {
		// Only correct settings for these Tbit are provided by avr
		if (Tbit != 20
			&& Tbit != 16
			&& Tbit != 15
			&& Tbit != 12
			&& Tbit != 10
			&& Tbit != 8) continue;

		const double TQn_sec = 1.0 / (CAN_BAUDRATE * Tbit);
		if (!(TQn_sec >= 1.0 / F_CPU)) continue;

		const int TDIVn = (F_CPU * TQn_sec) - 1;

		// BRP[5..0] is a 6bit value
		const int max_BRP_val = (1<<6) - 1;
		if (TDIVn > max_BRP_val) continue;

		const double clks_pr_TQn = TDIVn + (1 / F_CPU); // Clock ticks per TQ

		const int clks_pr_whole_TQn = round(clks_pr_TQn);
		const double TQn_err = clks_pr_whole_TQn - clks_pr_TQn;

		// Check if the new found value has a lower error rate than the prevous.
		if (fabs(TQn_err) < fabs(cfg->TQ_err)) {
			cfg->Tbit = Tbit;
			cfg->TQ = clks_pr_whole_TQn;
			cfg->TQ_err = TQn_err;
			cfg->prescalar = TDIVn;

			++num_valid_cfg;
		}
	}

	return num_valid_cfg == 0;
}

static void set_baud(void) {

	struct time_quantum_config cfg = {
		.TQ_err = 100000 // Set to a large initial value
	};

	if (find_best_time_quantum_prescalar(&cfg) != 0) {
		// Error
	}

	const int Tsyns = 1; // Tsyns is always 1 TQ

	int Tprs = 0; // PRopagation time Segment must be 1 to 8 TQ long.
	int Tph1 = 0; // PHase Segment 1 must be 1 to 8 TQ long.
	int Tph2 = 0; // PHase Segment 2 must be <= Tph1 and >= 2

	// Values taken from avr at90can128 data sheet table 19-2
	switch(cfg.Tbit) {
		case 20: Tprs = 8;  Tph1 = 6; Tph2 = 5; break;
		case 16: Tprs = 16; Tph1 = 4; Tph2 = 4; break;
		case 15: Tprs = 7;  Tph1 = 4; Tph2 = 3; break;
		case 12: Tprs = 5;  Tph1 = 3; Tph2 = 3; break;
		case 10: Tprs = 4;  Tph1 = 3; Tph2 = 2; break;
		case 8:  Tprs = 3;  Tph1 = 2; Tph2 = 2; break;

		default:
			Tprs = IS_ODD(cfg.Tbit) ? ((cfg.Tbit-1)/2) : (cfg.Tbit/2);
			Tph1 = IS_ODD(cfg.Tbit-Tprs-Tsyns) ? ((Tprs/2)+1) : (Tprs/2);
			Tph2 = Tprs/2; // Integer division. We round down to nearest int

			break;
	}

	const int Tsjw = 1; // can vary from 1 to 4 but is 1 in all avr examples.

	// Sanity check
	if (cfg.Tbit != Tsyns+Tprs+Tph1+Tph2
		|| (1 <= Tprs && Tprs <= 8)
		|| (1 <= Tph1 && Tph1 <= 8)
		|| (2 <= Tph2 && Tph2 <= Tph1))
	{
		// Error
	}


	SET_REGISTER_BITS(CANBT1, (cfg.prescalar-1)<<BRP0, (1<<BRP5|1<<BRP4|1<<BRP3|1<<BRP2|1<<BRP1|1<<BRP0));
	SET_REGISTER_BITS(CANBT2, (Tprs-1)<<PRS0, (1<<PRS0|1<<PRS1|1<<PRS2));
	SET_REGISTER_BITS(CANBT2, (Tsjw-1)<<SJW0, (1<<SJW0|1<<SJW1));
	SET_REGISTER_BITS(CANBT3, (Tph1-1)<<PHS10, (1<<PHS10|1<<PHS11|1<<PHS12));
	SET_REGISTER_BITS(CANBT3, (Tph2-1)<<PHS20, (1<<PHS20|1<<PHS21|1<<PHS22));
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
	usart1_printf("Interrupt\n");
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
					usart1_printf("ACK_ERROR\n");
					if ( canit_callback[CANIT_ACK_ERROR] != NULL )
						(*canit_callback[CANIT_ACK_ERROR])(mob);
					break;
				case MOB_FORM_ERROR:
				usart1_printf("FORM_ERROR\n");
					if ( canit_callback[CANIT_FORM_ERROR] != NULL )
						(*canit_callback[CANIT_FORM_ERROR])(mob);
					break;
				case MOB_CRC_ERROR:
				usart1_printf("CRC_ERROR\n");
					if ( canit_callback[CANIT_CRC_ERROR] != NULL )
						(*canit_callback[CANIT_CRC_ERROR])(mob);
					break;
				case MOB_STUFF_ERROR:
				usart1_printf("STUFF_ERROR\n");
					if ( canit_callback[CANIT_STUFF_ERROR] != NULL )
						(*canit_callback[CANIT_STUFF_ERROR])(mob);
					break;
				case MOB_BIT_ERROR:
				usart1_printf("BIT_ERROR\n");
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
