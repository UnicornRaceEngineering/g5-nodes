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
#include <stdint.h>    // for uint8_t, uint16_t, int8_t, uint32_t
#include <util/atomic.h>
#include <stdbool.h>

#include "can.h"       // for can_counters::NO_MOB_ERR, etc
#include "can_baud.h"  // for CANBT1_VALUE, CANBT2_VALUE, CANBT3_VALUE
#include "utils.h"     // for BIT_CLEAR, BIT_SET, BITMASK_SET, BIT_CHECK, etc
#include "ringbuffer.h"
#include "system_messages.h"


//_____ D E F I N I T I O N S __________________________________________________
#define CAN_RESET()       ( CANGCON  =  (1<<SWRES) )
#define CAN_ENABLE()      ( CANGCON |=  (1<<ENASTB))
#define CAN_DISABLE()     ( CANGCON &= ~(1<<ENASTB))
#define CAN_FULL_ABORT()  { CANGCON |=  (1<<ABRQ); CANGCON &= ~(1<<ABRQ); }

#define NB_MOB          ( 15        ) //!< Number of MOB's
#define DATA_MAX        ( 8         ) //!< The can can max transmit a payload of 8 uint8_t
#define LAST_MOB_NB     ( NB_MOB-1  ) //!< Index of the last MOB. This is useful when looping over all MOB's
#define NB_SPYMOB       ( 10        ) //!< Number of spymobs used.
#define NO_MOB          ( 0xFF      )

#define DLC_MSK         ( (1<<DLC3)|(1<<DLC2)|(1<<DLC1)|(1<<DLC0)   ) //!< Mask for Data Length Coding bits in CANCDMOB
#define MOB_CONMOB_MSK  ( (1 << CONMOB1) | (1 << CONMOB0)           ) //!< Mask for Configuration MOB bits in CANCDMOB

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


//_____ M A C R O S ____________________________________________________________
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

static inline int8_t find_me_a_mob(void);
static inline void enable_spy_mob(uint8_t mob);
static void receive_frame(const uint8_t mob, const uint16_t id);
static void reset_counters(void);

static volatile uint16_t mob_on_job;
static volatile ringbuffer_t rb;
static uint8_t buff[64];

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


void can_init() {
	rb_init((ringbuffer_t*)&rb, buff, 64);

	CAN_RESET();
	reset_counters();

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


uint8_t can_broadcast(const enum message_id id, const void* msg) {
	const uint16_t can_id = (uint16_t)id;
	int8_t mob = find_me_a_mob();
	if (mob == -1) {
		return NO_MOB_ERR;
	}

	uint8_t len = can_msg_length(id);

	BIT_SET(mob_on_job, mob);
	CAN_SET_MOB(mob);
	MOB_SET_STD_ID(can_id);
	MOB_SET_DLC(len);
	for (uint8_t i = 0; i < len; ++i) {
		CANMSG = ((uint8_t*)msg)[i];
	}

	MOB_EN_TX();
	CAN_ENABLE_MOB_INTERRUPT(mob);
	return SUCCES;
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


static void receive_frame(const uint8_t mob, const uint16_t id) {
	const uint8_t len = MOB_GET_DLC();

	if (rb_left((ringbuffer_t*)&rb) > (3 + len)) {
		rb_push((ringbuffer_t*)&rb, HIGH_BYTE(id));
		rb_push((ringbuffer_t*)&rb, LOW_BYTE(id));
		rb_push((ringbuffer_t*)&rb, len);
		for (uint8_t i = 0; i < len; ++i) {
			rb_push((ringbuffer_t*)&rb, CANMSG);
		}

		CAN_ENABLE_MOB_INTERRUPT(mob);
		MOB_EN_RX();
		++rx_comp;
	} else {
		CAN_ENABLE_MOB_INTERRUPT(mob);
		MOB_EN_RX();
		++alloc_err;
	}
}


void read_message(struct can_message* msg) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (can_has_data()) {
			uint8_t c;
			rb_pop((ringbuffer_t*)&rb, &c);
			msg->id = c << 8;
			rb_pop((ringbuffer_t*)&rb, &c);
			msg->id += c;
			rb_pop((ringbuffer_t*)&rb, &msg->len);

			for (uint8_t i = 0; i < msg->len; ++i) {
				rb_pop((ringbuffer_t*)&rb, &msg->data[i]);
			}
		}
	}
}


bool can_has_data() {
	return (rb_left((ringbuffer_t*)&rb) < 63) ? true : false;
}


ISR (CANIT_vect) {
	while (PRIORITY_MOB() != NB_MOB) { /* True if mob have pending interrupt */
		const uint8_t mob = PRIORITY_MOB();
		CAN_SET_MOB(mob);
		const uint8_t canst = CANSTMOB;
		const uint16_t id = MOB_GET_STD_ID();
		MOB_CLEAR_INT_STATUS();
		sei();

		switch (canst) {
			case MOB_RX_COMPLETED_DLCW:
				++dlcw_err;
			case MOB_RX_COMPLETED:

				if (!can_is_subscribed(id)) {
					CAN_ENABLE_MOB_INTERRUPT(mob);
					MOB_EN_RX();
					continue;
				} else {
					receive_frame(mob, id);
				}
				break;

			case MOB_TX_COMPLETED:
				++tx_comp;
				BIT_CLEAR(mob_on_job, mob);
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
