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
#include "utils.h"
#include "can.h"
#include "heap.h"


//_____ D E F I N I T I O N S __________________________________________________
#define CAN_RESET()       ( CANGCON  =  (1<<SWRES) )
#define CAN_ENABLE()      ( CANGCON |=  (1<<ENASTB))
#define CAN_DISABLE()     ( CANGCON &= ~(1<<ENASTB))
#define CAN_FULL_ABORT()  { CANGCON |=  (1<<ABRQ); CANGCON &= ~(1<<ABRQ); }

#define NB_MOB          ( 15        ) //!< Number of MOB's
#define NB_DATA_MAX     ( 8         ) //!< The can can max transmit a payload of 8 uint8_t
#define LAST_MOB_NB     ( NB_MOB-1  ) //!< Index of the last MOB. This is useful when looping over all MOB's
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
	MOB_TX_COMPLETED        = ( 1<<TXOK ),                                              //!< 0x40
	MOB_RX_COMPLETED        = ( 1<<RXOK ),                                              //!< 0x20
	MOB_RX_COMPLETED_DLCW   = ( (1<<RXOK)|(1<<DLCW) ),                                  //!< 0xA0
	MOB_ACK_ERROR           = ( 1<<AERR ),                                              //!< 0x01
	MOB_FORM_ERROR          = ( 1<<FERR ),                                              //!< 0x02
	MOB_CRC_ERROR           = ( 1<<CERR ),                                              //!< 0x04
	MOB_STUFF_ERROR         = ( 1<<SERR ),                                              //!< 0x08
	MOB_BIT_ERROR           = ( 1<<BERR ),                                              //!< 0x10
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

static void finnish_receive(uint8_t mob);
static int8_t set_can_config(const uint32_t baudrate);
static int8_t set_can_timings(const uint8_t prescalar, const uint8_t Tbit);
static void can_transmit (uint8_t mob, uint8_t type);
static int8_t find_me_a_mob(void);
static void enable_spy_mob(uint8_t mob);
static void initiate_receive(uint8_t old_mob, can_msg_t *msg);
static uint8_t count_free_mobs(void);
static void send_response(enum FC_flag flag, uint8_t block_size,
							uint8_t seperation_time, uint8_t id);
static void receive_on_mob(uint8_t old_mob, can_msg_t *msg);

static volatile can_msg_t *msg_list[15] = {0};
static volatile uint16_t mob_on_job;
static canrec_callback_t canrec_callback = 0;


//______________________________________________________________________________

void set_canrec_callback(canrec_callback_t callback) {
	canrec_callback = callback;
}

uint8_t can_init(uint16_t mask) {
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
	for (uint8_t mob = 0; mob < NB_MOB; ++mob) {
		CAN_SET_MOB(mob);
		MOB_CLEAR_STATUS();				// All MOb Registers=0
		MOB_ABORT();
		CAN_DISABLE_MOB_INTERRUPT(mob);
	}

	mob_on_job = 0;
	enable_spy_mob(14);
	enable_spy_mob(13);

	CAN_ENABLE();
	CAN_INT_ALL();
	return 0;
}

uint8_t can_send(const uint16_t id, const uint16_t len, void * const msg) {
	int8_t mob = find_me_a_mob();
	BIT_SET(mob_on_job, mob);
	msg_list[mob] = (can_msg_t*)smalloc(sizeof(can_msg_t));
	msg_list[mob]->data = (uint8_t*)msg;
	msg_list[mob]->id = id;
	msg_list[mob]->len = len;
	msg_list[mob]->idx = 0;
	msg_list[mob]->msg_num = 0;

	// After initializing all the information needed for the message we start
	// transmitting from mob and with a message type that can be 0 or 1 (these
	// are the two initializing message type according to ISO_15765-2).
	// If the payload length is 7 or less the payload can be transmitted in a
	// single message (type 0). When sending longer messages is send multiple
	// frames are needed and the first of these messages will be of type 1.
	can_transmit(mob, (len > 7));
	return 0;
}


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

static void send_response(enum FC_flag flag, uint8_t block_size,
							uint8_t seperation_time, uint8_t id) {
	uint8_t msg[8];
	uint8_t type = 3;
	msg[0] = type & 0x07;
	msg[0] += ((uint8_t)flag << 3) & 0xF8;
	msg[1] = block_size;
	msg[2] = seperation_time;

	int8_t mob = find_me_a_mob();
	BIT_SET(mob_on_job, mob);

	msg_list[mob] = (can_msg_t*)smalloc(sizeof(can_msg_t));
	msg_list[mob]->idx = 3;
	msg_list[mob]->len = 3;
	msg_list[mob]->waiting = 0;

	CAN_SET_MOB(mob);
	MOB_SET_STD_ID(id);
	MOB_SET_DLC(8);
	MOB_TX_DATA(msg);
	CAN_ENABLE_MOB_INTERRUPT(mob);
	MOB_EN_TX();
}

static void can_transmit (uint8_t mob, uint8_t type) {
	if (msg_list[mob]->waiting)
		return;

	if (msg_list[mob]->len == msg_list[mob]->idx) {
		CAN_DISABLE_MOB_INTERRUPT(mob);
		sfree((void *)msg_list[mob]->data);
		sfree((void *)msg_list[mob]);
		msg_list[mob] = 0;
		BIT_CLEAR(mob_on_job, mob);
		return;
	}

	uint8_t msg[8] = {0};
	uint8_t msg_length;
	uint8_t header;

	msg_list[mob]->waiting = 0;

	switch (type) {
		case 0:
			msg[0] = type & 0x07;
			msg[0] += (msg_list[mob]->len << 3) & 0xF8;
			header = 1;
			break;

		case 1:
			msg[0] = type & 0x07;
			msg[0] += LOW_BYTE(msg_list[mob]->len << 3);
			msg[1] = HIGH_BYTE(msg_list[mob]->len << 3);
			header = 2;

			int8_t new_mob = find_me_a_mob();
			BIT_SET(mob_on_job, new_mob);
			CAN_SET_MOB(new_mob);
			MOB_SET_STD_ID(msg_list[mob]->id);
			MOB_SET_STD_FILTER_FULL();
			MOB_SET_DLC(8); // Set the expected payload length
			MOB_EN_RX();
			CAN_ENABLE_MOB_INTERRUPT(new_mob);
			msg_list[mob]->on_mob = mob;
			msg_list[mob]->waiting = 1;
			msg_list[new_mob] = msg_list[mob];
			break;

		case 2:
			msg[0] = type & 0x07;
			msg_list[mob]->msg_num++;
			msg[0] += (msg_list[mob]->msg_num << 3) & 0xF8;
			header = 1;
			break;

		default:
			// Error: unsupported type
			return;
	}


	// If message is at the end with sending the mesage,
	// the length will be ajusted
	msg_length = (msg_list[mob]->len >= (msg_list[mob]->idx + (8 - header)))
					? ((type == 1) ? 6 : 7)
					: (msg_list[mob]->len - msg_list[mob]->idx);

	for (uint8_t i = 0; i < msg_length; i++)
		msg[i + header] = msg_list[mob]->data[msg_list[mob]->idx++];

	CAN_SET_MOB(mob);
	//The ID has to be set as the first parameter otherwise the it will be
	//the ID of the next message sent by the MOB.
	MOB_SET_STD_ID(msg_list[mob]->id);
	MOB_SET_DLC(8);
	MOB_TX_DATA(msg);
	CAN_ENABLE_MOB_INTERRUPT(mob);
	MOB_EN_TX();
}

static int8_t find_me_a_mob(void) {
	for (uint8_t i = 0; i <= 15; i++)
		if (!BIT_CHECK(mob_on_job, i))
			return i;

	return -1;
}

static void enable_spy_mob(uint8_t mob) {
	BIT_SET(mob_on_job, mob);
	CAN_SET_MOB(mob);
	MOB_SET_STD_FILTER_NONE();
	MOB_SET_DLC(8); // Set the expected payload length
	CAN_ENABLE_MOB_INTERRUPT(mob);
	MOB_EN_RX();
}

static uint8_t count_free_mobs(void) {
	uint8_t free_mobs = 0;
	for (uint8_t i = 0; i < 15; i++)
		free_mobs += !BIT_CHECK(mob_on_job, i);
	return free_mobs;
}

static void initiate_receive(uint8_t mob, can_msg_t *msg) {
	uint16_t number_of_frames = ((msg->len - msg->idx) + (7 - 1)) / 7;
	uint8_t free_mobs = count_free_mobs();

	uint8_t block_size = (number_of_frames > MAX_BLOCK_SIZE) ?
							MAX_BLOCK_SIZE : number_of_frames;
	block_size = (free_mobs > block_size) ? block_size : free_mobs;

	uint8_t FC_flag = !block_size;
	uint8_t seperation_time = 1; // 1 millisecond;

	if (FC_flag == 0)
		receive_on_mob(mob, (can_msg_t*)msg);

	send_response(FC_flag, block_size, seperation_time, msg->id);
}

static void receive_on_mob(uint8_t old_mob, can_msg_t *msg) {
	int8_t mob = find_me_a_mob();
	msg_list[mob] = (can_msg_t*)msg;
	BIT_SET(mob_on_job, mob);
	CAN_SET_MOB(mob);
	MOB_EN_RX();
	MOB_SET_DLC(8); // Set the expected payload length
	MOB_SET_STD_ID(msg->id);
	MOB_SET_STD_FILTER_FULL();
	CAN_ENABLE_MOB_INTERRUPT(mob);
	CAN_SET_MOB(old_mob);
}

static void finnish_receive(uint8_t mob) {
	if (canrec_callback != 0)
		(*canrec_callback)(msg_list[mob]->id, msg_list[mob]->len,
							(uint8_t*)&msg_list[mob]->data[0]);

	sfree((void *)msg_list[mob]);
	msg_list[mob] = 0;
	BIT_CLEAR(mob_on_job, mob);
}

static void can_get(uint8_t mob) {
	uint8_t msg[8];
	MOB_RX_DATA(msg);

	const uint8_t type = msg[0] & 0x07;

	switch (type) {
		case 0:
			msg_list[mob] = (can_msg_t*)smalloc(sizeof(can_msg_t));
			msg_list[mob]->len = ((msg[0] & 0xF8) >> 3);
			msg_list[mob]->data = (uint8_t*)smalloc(msg_list[mob]->len);
			msg_list[mob]->id = MOB_GET_STD_ID();
			msg_list[mob]->msg_num = 0;
			msg_list[mob]->idx = msg_list[mob]->len;
			for (uint8_t i = 0; i < msg_list[mob]->len; i++)
				msg_list[mob]->data[i] = msg[i + 1];
			finnish_receive(mob);
			break;

		case 1:
			msg_list[mob] = (can_msg_t*)smalloc(sizeof(can_msg_t));
			msg_list[mob]->len = ((msg[0] & 0xF8) >> 3) + ((msg[1] * 256) >> 3);
			msg_list[mob]->data = (uint8_t*)smalloc(msg_list[mob]->len);
			msg_list[mob]->id = MOB_GET_STD_ID();
			msg_list[mob]->msg_num = 0;
			msg_list[mob]->idx = 6;
			msg_list[mob]->waiting = 0;
			for (uint8_t i = 2; i < 8; i++)
				msg_list[mob]->data[i - 2] = msg[i];

			initiate_receive(mob, (can_msg_t*)msg_list[mob]);
			CAN_SET_MOB(mob);
			msg_list[mob] = 0;
			break;

		case 2:
			msg_list[mob]->msg_num = ((msg[0] & 0xF8) >> 3);
			uint8_t i = 1;
			while ( (msg_list[mob]->idx < msg_list[mob]->len) && (i < 8))
				msg_list[mob]->data[msg_list[mob]->idx++] = msg[i++];

			if (msg_list[mob]->len == msg_list[mob]->idx && type != 3) {
				finnish_receive(mob);
				return;
			}

			if (msg_list[mob]->len != msg_list[mob]->idx) {
				receive_on_mob(mob, (can_msg_t*)msg_list[mob]);

				msg_list[mob] = 0;
				BIT_CLEAR(mob_on_job, mob);
			}
			return;

		case 3:
			MOB_ABORT();
			CANSTMOB = 0;
			msg_list[mob]->waiting = 0;
			can_transmit(msg_list[mob]->on_mob, 2);
			BIT_CLEAR(mob_on_job, mob);
			return;

		default:
			// Error: unsupported type
			break;
	}

	CAN_ENABLE_MOB_INTERRUPT(mob);
	MOB_EN_RX();
	return;
}

ISR (CANIT_vect) {
	while (PRIORITY_MOB() != 15) { /* True if mob have pending interrupt */
		uint8_t mob = PRIORITY_MOB();
		CAN_SET_MOB(mob);
		switch (CANSTMOB) {
			case MOB_RX_COMPLETED_DLCW:
				// Fall through to MOB_RX_COMPLETED on purpose.
			case MOB_RX_COMPLETED:
				can_get(mob);
				break;

			case MOB_TX_COMPLETED:
				can_transmit(mob, 2);
				break;

		// 	case MOB_ACK_ERROR:
		// 		break;

		// 	case MOB_FORM_ERROR:
		// 		break;

		// 	case MOB_CRC_ERROR:
		// 		break;

		// 	case MOB_STUFF_ERROR:
		// 		break;

		// 	case MOB_BIT_ERROR:
		// 		break;
		}
		CANSTMOB = 0;
	}
}
