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


//_____ I N C L U D E S ________________________________________________________

#include <avr/interrupt.h>
#include <stdint.h>
#include <util/atomic.h>
#include "utils.h"
#include "heap.h"
#include "can_baud.h"
#include "can.h"
#include "sysclock.h"
#include <util/delay.h>


//_____ D E F I N I T I O N S __________________________________________________
#define CAN_RESET()       ( CANGCON  =  (1<<SWRES) )
#define CAN_ENABLE()      ( CANGCON |=  (1<<ENASTB))
#define CAN_DISABLE()     ( CANGCON &= ~(1<<ENASTB))
#define CAN_FULL_ABORT()  { CANGCON |=  (1<<ABRQ); CANGCON &= ~(1<<ABRQ); }

#define NB_MOB          ( 15        ) //!< Number of MOB's
#define DATA_MAX        ( 8         ) //!< The can can max transmit a payload of 8 uint8_t
#define LAST_MOB_NB     ( NB_MOB-1  ) //!< Index of the last MOB. This is useful when looping over all MOB's
#define NB_SPYMOB       ( 2         ) //!< Number of spymobs used.
#define NO_MOB          ( 0xFF      )

#define DLC_MSK         ( (1<<DLC3)|(1<<DLC2)|(1<<DLC1)|(1<<DLC0)   ) //!< Mask for Data Length Coding bits in CANCDMOB
#define MOB_CONMOB_MSK  ( (1 << CONMOB1) | (1 << CONMOB0)           ) //!< Mask for Configuration MOB bits in CANCDMOB

// Number of message allowed between every control message.
// This function is currently not fully implemented and tested.
#define MAX_BLOCK_SIZE 15


enum FC_flag {
	CLEAR_TO_SEND = 0,
	WAIT =          1,
	ABORT =         2
};

/**
* @brief
*   Different states that CANSTMOB can take.
*   This is very useful for fx. making a conditional
*   switch on the given status of the MOB
*/
enum mob_status_t {
	MOB_TX_COMPLETED        = ( 1<<TXOK ),             //!< 0x40
	MOB_RX_COMPLETED        = ( 1<<RXOK ),             //!< 0x20
	MOB_RX_COMPLETED_DLCW   = ( (1<<RXOK)|(1<<DLCW) ), //!< 0xA0
	MOB_ACK_ERROR           = ( 1<<AERR ),             //!< 0x01
	MOB_FORM_ERROR          = ( 1<<FERR ),             //!< 0x02
	MOB_CRC_ERROR           = ( 1<<CERR ),             //!< 0x04
	MOB_STUFF_ERROR         = ( 1<<SERR ),             //!< 0x08
	MOB_BIT_ERROR           = ( 1<<BERR ),             //!< 0x10
 };

typedef struct can_msg_t can_msg_t;

struct can_msg_t {
	uint16_t id;
	uint16_t len;
	uint16_t idx;
	uint16_t msg_num;
	uint8_t *data;
	uint8_t on_mob;
	uint8_t waiting;
	uint32_t age;
};


//_____ M A C R O S ____________________________________________________________
//!< @name MOB Transmit and Receive
//!< Transmit or receive data on the current MOB
//!< @{
#define MOB_TX_DATA(data) do { \
	for (uint8_t i = 0; i < 8; ++i) { CANMSG = data[i]; } \
} while (0)

#define MOB_RX_DATA(data) do { \
	for (uint8_t i = 0; i < 8; ++i) { data[i] = CANMSG;} \
} while (0)
//!< @} ----------

#define TX_BUSY() ( BIT_CHECK(CANGSTA, TXBSY) )

//!< @name CAN status Interrupt register
//!< @{
#define CANSIT_16                   ( CANSIT2 + (CANSIT1 << 8)      ) //!< The CANSIT holds information about what mob has fired an interrupt. This combines it into a single 16 bit value.
#define MOB_HAS_PENDING_INT(mob)    ( BIT_CHECK(CANSIT_16, (mob))   ) //!< Check if the given mob has a pending interrupt.
//!< @} ----------


#define CAN_SET_MOB(mob)                ( CANPAGE = ((mob) << 4)    ) //!< Set the can the the specified MOB

//@name MOB interrupt
//!< Enable or disable interrupts on the specified MOB
//!< @{
#define CAN_ENABLE_MOB_INTERRUPT(mob) do { \
	CANIE2 |= ((1 << mob) & 0xff); \
	CANIE1 |= (((1 << mob) >> 8) & 0x7f); \
} while (0)

#define CAN_DISABLE_MOB_INTERRUPT(mob) do { \
	CANIE2 &= ~((1 << mob) & 0xff); \
	CANIE1 &= ~(((1 << mob) >> 8) & 0x7f); \
} while (0)
//!< @} ----------


//!< @name Can interrupt
//!< enable the can interrupt
//!< @{
#define CAN_SEI()          ( BIT_SET(CANGIE, ENIT) ) //!< Enable global CAN interrupts
#define CAN_EN_TX_INT()    ( BIT_SET(CANGIE, ENTX) ) //!< Enable CAN Tx interrupts
#define CAN_EN_RX_INT()    ( BIT_SET(CANGIE, ENRX) ) //!< Enable CAN Rx interrupts
//!< @} ----------


//!< @name Data Length Code
//!< Getter and setter for the length of data that the given MOB holds
//!< @{
#define MOB_GET_DLC()       ( BITMASK_CHECK(CANCDMOB, DLC_MSK) >> DLC0  ) //!< Calculates the DLC that is set for the current MOB. @return The DLC sat for the current MOB
#define MOB_SET_DLC(dlc)    ( BITMASK_SET(CANCDMOB, dlc)                ) //!< Set the DLC for the current MOB
//!< @} ----------

#define PRIORITY_MOB()		( (CANHPMOB & 0xF0) >> 4 )


//!< @name MOB ID
//!< @{
#define MOB_SET_STD_ID_10_4(id)         (   ((*((uint8_t *)(&(id)) + 1)) << 5) + \
											((*(uint8_t *)(&(id))) >> 3)            )

#define MOB_SET_STD_ID_3_0(id)          (   (*(uint8_t *)(&(id))) << 5              )

#define MOB_SET_STD_ID(id) do { \
	CANIDT1 = MOB_SET_STD_ID_10_4((id)); \
	CANIDT2 = MOB_SET_STD_ID_3_0((id)); \
	CANCDMOB &= (~(1<<IDE)); \
} while (0)

#define MOB_GET_STD_ID()	( ((CANIDT2 & 0xE0) >> 5) + (CANIDT1 << 3) )

#define MOB_SET_STD_MASK_FILTER(mask) do { \
	CANIDM1 = MOB_SET_STD_ID_10_4(mask); \
	CANIDM2 = MOB_SET_STD_ID_3_0( mask); \
} while (0)

// makes a filter of UINT32_MAX
#define MOB_SET_STD_FILTER_FULL() do { \
	uint32_t __filterMask_ = (uint32_t)(~0); \
	MOB_SET_STD_MASK_FILTER(__filterMask_); \
} while (0)

#define MOB_SET_STD_FILTER_NONE() do { \
	uint32_t __filterMask_ = 0; \
	MOB_SET_STD_MASK_FILTER(__filterMask_); \
} while (0)
//!< @} ----------


//!< @name MOB status
//!< @{
#define MOB_CLEAR_STATUS() do { \
	uint8_t  volatile *__i_; \
	for (__i_ =& CANSTMOB; __i_ < &CANSTML; __i_++) { *__i_= 0x00; } \
} while (0)
#define MOB_CLEAR_INT_STATUS()          ( CANSTMOB = 0x00   ) //!< Clears the interrupt status for the current MOB
//!< @} ----------

//!< @name Configuration of Message Object
//!< These bits set the communication to be performed (no initial value after RESET).
//!< These bits are *NOT* cleared once communication is performed.
//!< The user must re-write the configuration to enable new communication.
//!< @{
#define MOB_ABORT()             ( BITMASK_CLEAR(CANCDMOB, MOB_CONMOB_MSK)                   ) //!< Disable MOB
#define MOB_EN_TX()             do { BIT_CLEAR(CANCDMOB, CONMOB1); BIT_SET(CANCDMOB, CONMOB0); } while (0) //!< Enable MOB Transmission
#define MOB_EN_RX()             do { BIT_SET(CANCDMOB, CONMOB1); BIT_CLEAR(CANCDMOB, CONMOB0); } while (0) //!< Enable MOB Reception
#define MOB_EN_FRM_BUFF_RX()    ( BITMASK_SET(CANCDMOB, MOB_CONMOB_MSK)                     ) //!< Enable MOB Frame Buffer Reception
//!< @} ----------

//Please insert comment !
#define CAN_INT_ALL()  do {CAN_SEI(); CAN_EN_RX_INT(); CAN_EN_TX_INT();} while (0)
#define CAN_INT_RX()   do {CAN_SEI(); CAN_EN_RX_INT();                 } while (0)
#define CAN_INT_TX()   do {CAN_SEI(); CAN_EN_TX_INT();                 } while (0)


//_____ D E C L A R A T I O N S ________________________________________________

static inline uint8_t finnish_receive(uint8_t mob);
static void continue_sending (uint8_t mob);
static inline int8_t find_me_a_mob(void);
static inline void enable_spy_mob(uint8_t mob);
static uint8_t initiate_receive(uint8_t old_mob, can_msg_t *msg);
static inline uint8_t receive_on_mob(uint8_t old_mob, can_msg_t *msg);
static inline void mob_receive(uint8_t mob, uint16_t id);
static inline void mob_send(uint8_t mob, uint16_t id, uint8_t data[8]);
static uint8_t receive_frame(uint8_t mob);
static void reset_counters(void);

static volatile can_msg_t *msg_list[15] = {0};
static volatile uint16_t mob_on_job;
static canrec_callback_t canrec_callback = 0;
static volatile can_filter_t filter1, filter2;

static volatile uint16_t dlcw_err;
static volatile uint16_t rx_comp;
static volatile uint16_t tx_comp;
static volatile uint16_t ack_err;
static volatile uint16_t form_err;
static volatile uint16_t crc_err;
static volatile uint16_t stuff_err;
static volatile uint16_t bit_err;
static volatile uint16_t no_mob_err;
static volatile uint16_t alloc_err;


//______________________________________________________________________________

void set_canrec_callback(canrec_callback_t callback) {
	canrec_callback = callback;
}


static void reset_counters() {
	dlcw_err  = 0;
	rx_comp   = 0;
	tx_comp   = 0;
	ack_err   = 0;
	form_err  = 0;
	crc_err   = 0;
	stuff_err = 0;
	bit_err   = 0;
}


uint16_t get_counter(enum can_counters counter) {
	switch (counter) {
		case DLCW_ERR: 	return dlcw_err;
		case RX_COMP: 	return rx_comp;
		case TX_COMP: 	return tx_comp;
		case ACK_ERR: 	return ack_err;
		case FORM_ERR:	return form_err;
		case CRC_ERR: 	return crc_err;
		case STUFF_ERR: return stuff_err;
		case BIT_ERR: 	return bit_err;
		case NO_MOB_ERR:return no_mob_err;
		case ALLOC_ERR: return alloc_err;
		case TOTAL_ERR: return dlcw_err + ack_err + form_err + crc_err +
								stuff_err + bit_err + no_mob_err + alloc_err;
		default: 		return 0;
	}
}

#include <stdio.h>
void can_init(can_filter_t fil1, can_filter_t fil2) {
	CAN_RESET();
	reset_counters();

	// Set the CAN ID filters.
	filter1 = fil1;
	filter2 = fil2;

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
	CANBT1 = CANBT1_VALUE;
	CANBT2 = CANBT2_VALUE;
	CANBT3 = CANBT3_VALUE;

	// It reset CANSTMOB, CANCDMOB, CANIDTx & CANIDMx and clears data FIFO of
	// MOb[0] upto MOb[LAST_MOB_NB].
	for (uint8_t mob = 0; mob < NB_MOB; ++mob) {
		CAN_SET_MOB(mob);
		MOB_CLEAR_STATUS();				// All MOb Registers=0
		MOB_ABORT();
		CAN_DISABLE_MOB_INTERRUPT(mob);
	}

	mob_on_job = 0;

	// Enable all spymobs
	uint8_t i = NB_SPYMOB;
	while (i--) {
		enable_spy_mob(LAST_MOB_NB - i);
	}

	CAN_ENABLE();
	CAN_INT_ALL();
}


uint8_t can_send(const uint16_t id, const uint16_t len, const uint8_t* msg) {

	// After initializing all the information needed for the message we start
	// transmitting from mob and with a message type that can be 0 or 1 (these
	// are the two initializing message type according to ISO_15765-2).
	// If the payload length is 7 or less the payload can be transmitted in a
	// single message (type 0). When sending longer messages is send multiple
	// frames are needed and the first of these messages will be of type 1.
	if (len > 7) {
		int8_t mob = find_me_a_mob();
		if (mob == -1) {
			return NO_MOB_ERR;
		}
		BIT_SET(mob_on_job, mob);

		int8_t new_mob = find_me_a_mob();
		if (new_mob == -1) {
			BIT_CLEAR(mob_on_job, mob);
			return NO_MOB_ERR;
		}
		BIT_SET(mob_on_job, new_mob);

		msg_list[mob] = (can_msg_t*)smalloc(sizeof(can_msg_t));
		if (!msg_list[mob]) {
			BIT_CLEAR(mob_on_job, new_mob);
			BIT_CLEAR(mob_on_job, mob);
			++alloc_err;
			return ALLOC_ERR;
		}

		msg_list[mob]->data = (uint8_t*)msg;
		msg_list[mob]->id = id;
		msg_list[mob]->len = len;
		msg_list[mob]->idx = 0;
		msg_list[mob]->msg_num = 0;
		msg_list[mob]->age = get_tick();

		uint8_t data[DATA_MAX] = {0};
		data[0] = 1;
		data[0] += LOW_BYTE(msg_list[mob]->len << 3);
		data[1] = HIGH_BYTE(msg_list[mob]->len << 3);
		uint8_t header = 2;

		CAN_SET_MOB(new_mob);
		MOB_SET_STD_ID(msg_list[mob]->id);
		MOB_SET_STD_FILTER_FULL();
		MOB_SET_DLC(DATA_MAX); // Set the expected payload length
		MOB_EN_RX();
		CAN_ENABLE_MOB_INTERRUPT(new_mob);
		msg_list[mob]->on_mob = mob;
		msg_list[mob]->waiting = new_mob;
		msg_list[new_mob] = msg_list[mob];

		for (uint8_t i = header; i < DATA_MAX; i++) {
			data[i] = msg_list[mob]->data[msg_list[mob]->idx++];
		}

		CAN_SET_MOB(mob);
		mob_send(mob, msg_list[mob]->id, data);
	} else {
		int8_t mob = find_me_a_mob();
		if (mob == -1) {
			return NO_MOB_ERR;
		}

		msg_list[mob] = 0;

		uint8_t data[8] = {0};
		data[0] = (len << 3) & 0xF8;
		for (uint8_t i = 0; i < len; ++i)
			data[i + 1] = msg[i];

		BIT_SET(mob_on_job, mob);
		CAN_SET_MOB(mob);
		MOB_SET_STD_ID(id);
		MOB_SET_DLC(8);
		MOB_TX_DATA(data);
		MOB_EN_TX();
		CAN_ENABLE_MOB_INTERRUPT(mob);
	}
	return SUCCES;
}


void mob_cleanup(uint32_t time_now) {
	for (uint8_t mob = 0; mob < NB_MOB; ++mob) {
		if (BIT_CHECK(mob_on_job, mob) && msg_list[mob]) {
			printf("mob %2d, waiting %2d, has age %4lu\n", mob, msg_list[mob]->waiting, get_tick() - msg_list[mob]->age);
		}
		if ((get_tick() - msg_list[mob]->age) > 10) {
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				sfree((void *)msg_list[mob]->data);
				sfree((void *)msg_list[mob]);
				if (msg_list[mob]->waiting <= NO_MOB) {
					uint8_t receiving_mob = msg_list[mob]->waiting;
					msg_list[receiving_mob] = 0;
					BIT_CLEAR(mob_on_job, receiving_mob);
				}
				uint8_t sending_mob = msg_list[mob]->on_mob;
				msg_list[sending_mob] = 0;
				BIT_CLEAR(mob_on_job, sending_mob);
			}
		}
	}
}


static inline void mob_receive(uint8_t mob, uint16_t id) {
	CAN_SET_MOB(mob);
	MOB_EN_RX();
	MOB_SET_DLC(8); // Set the expected payload length
	MOB_SET_STD_ID(id);
	MOB_SET_STD_FILTER_FULL();
	CAN_ENABLE_MOB_INTERRUPT(mob);
}


static inline void mob_send(uint8_t mob, uint16_t id, uint8_t data[8]) {
	CAN_SET_MOB(mob);
	//The ID has to be set as the first parameter otherwise the it will be
	//the ID of the next message sent by the MOB.
	MOB_SET_STD_ID(id);
	MOB_SET_DLC(8);
	MOB_TX_DATA(data);
	CAN_ENABLE_MOB_INTERRUPT(mob);
	MOB_EN_TX();
}


static inline int8_t find_me_a_mob(void) {
	for (uint8_t i = 0; i < 15; i++)
		if (!BIT_CHECK(mob_on_job, i))
			return i;

	++no_mob_err;
	return -1;
}


static inline void enable_spy_mob(uint8_t mob) {
	BIT_SET(mob_on_job, mob);
	CAN_SET_MOB(mob);
	MOB_SET_STD_FILTER_NONE();
	MOB_SET_DLC(8); // Set the expected payload length
	CAN_ENABLE_MOB_INTERRUPT(mob);
	MOB_EN_RX();
}


static uint8_t initiate_receive(uint8_t mob, can_msg_t *msg) {
	uint16_t number_of_frames = ((msg->len - msg->idx) + (7 - 1)) / 7;
	uint8_t free_mobs = 0;
	for (uint8_t i = 0; i < 15; i++)
		free_mobs += !BIT_CHECK(mob_on_job, i);

	uint8_t block_size = (number_of_frames > MAX_BLOCK_SIZE) ?
							MAX_BLOCK_SIZE : number_of_frames;
	block_size = (free_mobs > block_size) ? block_size : free_mobs;

	uint8_t FC_flag = !block_size;
	uint8_t seperation_time = 1; // 1 millisecond;

	if (FC_flag == 0) {
		if (receive_on_mob(mob, (can_msg_t*)msg)) {
			return NO_MOB_ERR;
		}
	}

	uint8_t data[8] = {0};
	uint8_t type = 3;
	data[0] = type & 0x07;
	data[0] += ((uint8_t)FC_flag << 3) & 0xF8;
	data[1] = block_size;
	data[2] = seperation_time;

	int8_t res_mob = find_me_a_mob();
	if (res_mob == -1) {
		BIT_CLEAR(mob_on_job, mob);
		return NO_MOB_ERR;
	}
	BIT_SET(mob_on_job, res_mob);

	msg_list[res_mob] = (can_msg_t*)smalloc(sizeof(can_msg_t));
	if (!msg_list[res_mob]) {
		BIT_CLEAR(mob_on_job, res_mob);
		BIT_CLEAR(mob_on_job, mob);
		return ALLOC_ERR;
	}

	msg_list[res_mob]->idx = 3;
	msg_list[res_mob]->len = 3;
	msg_list[res_mob]->waiting = NO_MOB;

	CAN_SET_MOB(res_mob);
	mob_send(res_mob, msg->id, data);
	return SUCCES;
}


static inline uint8_t receive_on_mob(uint8_t old_mob, can_msg_t *msg) {
	int8_t mob = find_me_a_mob();
	if (mob == -1) {
		return NO_MOB_ERR;
	}

	msg_list[mob] = (can_msg_t*)msg;
	msg_list[mob]->on_mob = mob;
	BIT_SET(mob_on_job, mob);
	mob_receive(mob, msg->id);
	CAN_SET_MOB(old_mob);
	return SUCCES;
}


static inline uint8_t finnish_receive(uint8_t mob) {
	uint8_t err = (*canrec_callback)(msg_list[mob]->id,	(uint8_t*)&msg_list[mob]->data[0]);

	sfree((void *)msg_list[mob]);
	msg_list[mob] = 0;
	BIT_CLEAR(mob_on_job, mob);
	if (err) {
		return err;
	}
	return SUCCES;
}


static void continue_sending(uint8_t mob) {
	if (msg_list[mob] == 0) {
		BIT_CLEAR(mob_on_job, mob);
		return;
	}

	if (msg_list[mob]->waiting != NO_MOB)
		return;

	if (msg_list[mob]->len == msg_list[mob]->idx) {
		CAN_DISABLE_MOB_INTERRUPT(mob);
		sfree((void *)msg_list[mob]->data);
		sfree((void *)msg_list[mob]);
		msg_list[mob] = 0;
		BIT_CLEAR(mob_on_job, mob);
		return;
	}

	msg_list[mob]->waiting = NO_MOB;
	msg_list[mob]->msg_num++;
	uint8_t msg[8] = {0};
	msg[0] = 2;
	msg[0] += (msg_list[mob]->msg_num << 3) & 0xF8;
	const uint8_t header = 1;

	// If message is at the end with sending the mesage,
	// the length will be ajusted
	uint8_t msg_length = (msg_list[mob]->len >= (msg_list[mob]->idx + (DATA_MAX - header)))
					? DATA_MAX - header : (msg_list[mob]->len - msg_list[mob]->idx);

	for (uint8_t i = 0; i < msg_length; i++)
		msg[i + header] = msg_list[mob]->data[msg_list[mob]->idx++];

	CAN_SET_MOB(mob);
	_delay_us(100);
	mob_send(mob, msg_list[mob]->id, msg);
}


static uint8_t receive_frame(uint8_t mob) {
	uint8_t msg[8];
	MOB_RX_DATA(msg);

	switch (msg[0] & 0x07) { // Switch on the message type.
		case 0:
		{
			uint16_t len = (msg[0] & 0xF8) >> 3;
			uint16_t id = MOB_GET_STD_ID();
			uint8_t *data = (uint8_t*)smalloc(len);
			if (!data) {
				return ALLOC_ERR;
			}

			for (uint8_t i = 0; i < len; i++)
				data[i] = msg[i + 1];

			uint8_t err = (*canrec_callback)(id, (uint8_t*)&data[0]);
			if (err) {
				sfree((void *) data);
				return err;
			}
		}
		break;

		case 1:
			msg_list[mob] = (can_msg_t*)smalloc(sizeof(can_msg_t));
			if (!msg_list[mob]) {
				return ALLOC_ERR;
			}

			msg_list[mob]->len = ((msg[0] & 0xF8) >> 3) + ((msg[1] * 256) >> 3);
			msg_list[mob]->data = (uint8_t*)smalloc(msg_list[mob]->len);
			if (!msg_list[mob]->data) {
				sfree((void *)msg_list[mob]);
				return ALLOC_ERR;
			}

			msg_list[mob]->id = MOB_GET_STD_ID();
			msg_list[mob]->msg_num = 0;
			msg_list[mob]->idx = 6;
			msg_list[mob]->waiting = NO_MOB;
			msg_list[mob]->age = get_tick();
			const uint8_t header = 2;
			for (uint8_t i = header; i < DATA_MAX; i++)
				msg_list[mob]->data[i - header] = msg[i];

			uint8_t err = initiate_receive(mob, (can_msg_t*)msg_list[mob]);
			if (err) {
				return err;
			}

			CAN_SET_MOB(mob);
			msg_list[mob] = 0;
			break;

		case 2:
			msg_list[mob]->age = get_tick();
			msg_list[mob]->msg_num = ((msg[0] & 0xF8) >> 3);
			uint8_t i = 1;
			while ( (msg_list[mob]->idx < msg_list[mob]->len) && (i < 8))
				msg_list[mob]->data[msg_list[mob]->idx++] = msg[i++];

			if (msg_list[mob]->len == msg_list[mob]->idx) {
				uint8_t err = finnish_receive(mob);
				if (err) {
					return err;
				}
				return SUCCES;
			}
			break;

		case 3:
			msg_list[mob]->waiting = NO_MOB;
			continue_sending(msg_list[mob]->on_mob);
			BIT_CLEAR(mob_on_job, mob);
			return SUCCES;

		default:
			// Error: unsupported type
			break;
	}

	CAN_ENABLE_MOB_INTERRUPT(mob);
	MOB_EN_RX();
	return SUCCES;
}


ISR (CANIT_vect) {
	while (PRIORITY_MOB() != NB_MOB) { /* True if mob have pending interrupt */
		const uint8_t mob = PRIORITY_MOB();
		CAN_SET_MOB(mob);
		const uint8_t canst = CANSTMOB;
		CANSTMOB = 0;
		sei();

		switch (canst) {
			case MOB_RX_COMPLETED_DLCW:
				++dlcw_err;;
				break;
			case MOB_RX_COMPLETED:
				++rx_comp;

				// Run through filter, and return if ID not in ranges.
				if (mob > (LAST_MOB_NB - NB_SPYMOB)) {
					uint16_t id = MOB_GET_STD_ID();
					if ( !((id >= filter1.lower_bound && id < filter1.upper_bound)
						|| (id >= filter2.lower_bound && id < filter2.upper_bound)) )
					{
						CAN_ENABLE_MOB_INTERRUPT(mob);
						MOB_EN_RX();
						continue;
					}
				}

				// Receive a frame and deal with errors if necessary.
				uint8_t err = receive_frame(mob);
				if (err) {
					if (mob > (LAST_MOB_NB - NB_SPYMOB)) {
						CAN_ENABLE_MOB_INTERRUPT(mob);
						MOB_EN_RX();
					}

					if (err == ALLOC_ERR) {
						++alloc_err;
					}
				}
				break;

			case MOB_TX_COMPLETED:
				++tx_comp;
				continue_sending(mob);
				break;

		 	case MOB_ACK_ERROR:
		 		++ack_err;
		 		break;

		 	case MOB_FORM_ERROR:
		 		++form_err;
		 		break;

		 	case MOB_CRC_ERROR:
		 		++crc_err;
		 		break;

		 	case MOB_STUFF_ERROR:
		 		++stuff_err;
		 		break;

		 	case MOB_BIT_ERROR:
		 		++bit_err;
		 		break;
		}
		cli();
	}
}
