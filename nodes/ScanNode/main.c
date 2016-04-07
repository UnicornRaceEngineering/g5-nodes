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

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <stdint.h>           // for uint8_t, uint16_t
#include <stdio.h>            // for printf
#include <usart.h>            // for usart1_init
#include <stdbool.h>
#include <can.h>
#include <event_manager.h>
#include <sysclock.h>
#include "system_messages.h"  // for MESSAGE_INFO, message_detail, etc
#include "utils.h"            // for ARR_LEN
#include <util/delay.h>


void read_msg(const uint8_t load);

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static uint16_t msg_num = 0;


static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	can_init();

	can_subscribe_all();

	sei();
	puts_P(PSTR("Init complete\n\n"));
}

//_____ D E F I N I T I O N S __________________________________________________
#define CAN_RESET()       ( CANGCON  =  (1<<SWRES) )
#define CAN_ENABLE()      ( CANGCON |=  (1<<ENASTB))
#define CAN_DISABLE()     ( CANGCON &= ~(1<<ENASTB))
#define CAN_FULL_ABORT()  { CANGCON |=  (1<<ABRQ); CANGCON &= ~(1<<ABRQ); }

#define RINGBF_SIZE     ( 64        ) //!< Size of ringbuffer for recieved CAN frames.
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



static inline void enable_spy_mob(uint8_t mob) {
	CAN_SET_MOB(mob);
	MOB_SET_STD_FILTER_NONE();
	MOB_SET_DLC(8); // Set the expected payload length
	CAN_ENABLE_MOB_INTERRUPT(mob);
	MOB_EN_RX();
}

int main(void) {
	init();


	while (1) {
		uint8_t node_id = 2;
		can_broadcast(HEARTBEAT, &node_id);
		_delay_ms(1000);
		//while (can_has_data()) {
		//	struct can_message message;
		//	read_message(&message);
		//	printf("SUCCES!!!!!!: %d\n", message.id);
		//}
	}


	uint32_t suc = 0;

	while (1) {
		uint8_t CAN_PRESCALER = 1;
		uint8_t CAN_TPRS = 1;
		uint8_t CAN_TPH1 = 1;
		uint8_t CAN_TPH2 = 1;
		uint8_t CAN_SJW = 1;

		for (CAN_PRESCALER = 1; CAN_PRESCALER < 64; CAN_PRESCALER++) {
			for (CAN_TPRS = 1; CAN_TPRS < 8; CAN_TPRS++) {
				for (CAN_TPH1 = 1; CAN_TPH1 < 8; CAN_TPH1++) {
					for (CAN_TPH2 = 1; CAN_TPH2 < 8; CAN_TPH2++) {
						for (CAN_SJW = 1; CAN_SJW < 4; CAN_SJW++) {

							CAN_RESET();

							CANBT1 = ((CAN_PRESCALER-1)<<BRP0);
							CANBT2 = (((CAN_TPRS-1)<<PRS0) | ((CAN_SJW-1)<<SJW0));
							CANBT3 = (((CAN_TPH1-1)<<PHS10) | ((CAN_TPH2-1)<<PHS20));

							for (uint8_t mob = 0; mob < NB_MOB; ++mob) {
								CAN_SET_MOB(mob);
								MOB_CLEAR_STATUS();				// All MOb Registers=0
								MOB_ABORT();
								CAN_DISABLE_MOB_INTERRUPT(mob);
							}

							// Enable all spymobs
							uint8_t i = NB_SPYMOB;
							while (i--) {
								enable_spy_mob(LAST_MOB_NB - i);
							}

							CAN_ENABLE();
							CAN_INT_ALL();
							//_delay_ms(10);
							//printf("PRESCAL %d|TPRS %d|TPH1 %d|TPH2 %d|SJW %d|TOTAL_ERR: %6u\n", CAN_PRESCALER, CAN_TPRS, CAN_TPH1, CAN_TPH2, CAN_SJW, get_counter(TOTAL_ERR));
							//uint8_t pakke[8] = {2, 9, 0, 0x55, 0x55, 0x55, 0x55, 0x55};
							//can_broadcast((enum message_id)0x7DF, &pakke);
							//_delay_ms(5);

							while (can_has_data()) {
								struct can_message message;
								read_message(&message);
								++suc;
								printf("PRESCAL %d|TPRS %d|TPH1 %d|TPH2 %d|SJW %d|SUCCES: %lu  RX: %u   TOTAL_ERR: %6u\n", CAN_PRESCALER, CAN_TPRS, CAN_TPH1, CAN_TPH2, CAN_SJW, suc, get_counter(RX_COMP), get_counter(TOTAL_ERR));
								printf("SUCCES!!!!!!!!!!!!!!!!!!!!!!: %d\n", message.id);
							}
						}
					}
				}
			}
		}
		printf("DONE\n");
	}

	return 0;
}


void read_msg(const uint8_t load) {
	while(can_has_data()) {
		struct can_message msg;
		read_message(&msg);
		printf("Load:%3u | MSG:%5u | recieved: %4u | error: %4u |  Got id: %4u and length: %1u\t",
			load, msg_num++, get_counter(RX_COMP), get_counter(TOTAL_ERR), msg.id, msg.len);
		for (uint8_t i = 0; i < msg.len; ++i) {
			printf("%3d; 0x%02x  |  ", msg.data[i], msg.data[i]);
		}
		printf("\n");
	}
}
