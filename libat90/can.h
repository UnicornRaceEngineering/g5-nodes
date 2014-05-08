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
* @file can.h
* @brief
* Used for setting up the CAN subsystem and sending or receiving via the CAN.
* This header file also contains many driver functions in the form of macros.
*/

#ifndef CAN_H
#define CAN_H


//_____ I N C L U D E S ________________________________________________________

#include <stdint.h>
#include "bitwise.h"
#include <avr/interrupt.h>

#ifndef UINT16_MAX
#define UINT16_MAX	((uint16_t)(~0))
#endif

#ifndef UINT32_MAX
#define UINT32_MAX	((uint32_t)(~0))
#endif

/**
 * Can interrupt callback function pointer.
 * @param  mob The mob that caused the interrupt
 */
typedef void (*canit_callback_t)(uint8_t mob);

/**
 * Interrupt callback function pointer for can overflow interrupt.
 */
typedef void (*ovrit_callback_t)(void);


//_____ D E F I N I T I O N S __________________________________________________
#ifndef F_CPU
#error	"You must define F_CPU"
#endif

#ifndef CAN_BAUDRATE
#error	"You must define CAN_BAUDRATE"
#endif

#if F_CPU == 16000000				//!< Fclkio = 16 MHz, Tclkio = 62.5 ns
#if		CAN_BAUDRATE == 100		//!< -- 100Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x12	// Tscl  =	0x Tclkio = 625 ns
#		define CONF_CANBT2	0x0C	// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x37	// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 125		//!< -- 125Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x0E	// Tscl  = 8x Tclkio = 500 ns
#		define CONF_CANBT2	0x0C	// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x37	// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 200Kb	//!< -- 200Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x08	// Tscl  = 5x Tclkio = 312.5 ns
#		define CONF_CANBT2	0x0C	// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x37	// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 250		//!< -- 250Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x06	// Tscl  = 4x Tclkio = 250 ns
#		define CONF_CANBT2	0x0C	// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x37	// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 500		//!< -- 500Kb/s, 8x Tscl, sampling at 75%
#		define CONF_CANBT1	0x06	// Tscl = 4x Tclkio = 250 ns
#		define CONF_CANBT2	0x04	// Tsync = 1x Tscl, Tprs = 3x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x13	// Tpsh1 = 2x Tscl, Tpsh2 = 2x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 1000	//!< -- 1 Mb/s, 8x Tscl, sampling at 75%
#		define CONF_CANBT1	0x02	// Tscl  = 2x Tclkio = 125 ns
#		define CONF_CANBT2	0x04	// Tsync = 1x Tscl, Tprs = 3x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x13	// Tpsh1 = 2x Tscl, Tpsh2 = 2x Tscl, 3 sample points
#else
#error	"This CAN_BAUDRATE value is not defined"
#endif

// Timing changed to work with 11059200 Hz!
#elif F_CPU == 11059200			//!< Fclkio = 12 MHz, Tclkio = 83.333 ns
#if		CAN_BAUDRATE == 100		//!< -- 100Kb/s, 20x Tscl, sampling at 75%
#		define CONF_CANBT1	0x0A	// Tscl  = 6x Tclkio = 500 ns
#		define CONF_CANBT2	0x0E	// Tsync = 1x Tscl, Tprs = 8x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x4B	// Tpsh1 = 6x Tscl, Tpsh2 = 5x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 125		//!< -- 125Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x0A	// Tscl  = 6x Tclkio = 500 ns
#		define CONF_CANBT2	0x0C	// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x37	// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 200		//!< -- 200Kb/s, 20x Tscl, sampling at 75%
#		define CONF_CANBT1	0x04	// Tscl  = 3x Tclkio = 250 ns
#		define CONF_CANBT2	0x0E	// Tsync = 1x Tscl, Tprs = 8x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x4B	// Tpsh1 = 6x Tscl, Tpsh2 = 5x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 250		//!< -- 250Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x06	// Tscl  = 3x Tclkio = 250 ns
#		define CONF_CANBT2	0x06	// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x24	// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 500		//!< -- 500Kb/s, 12x Tscl, sampling at 75%
#		define CONF_CANBT1	0x02	// Tscl  = 2x Tclkio = 166.666 ns
#		define CONF_CANBT2	0x08	// Tsync = 1x Tscl, Tprs = 5x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x25	// Tpsh1 = 3x Tscl, Tpsh2 = 3x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 1000	//!< -- 1 Mb/s, 12x Tscl, sampling at 75%
#		define CONF_CANBT1	0x00	// Tscl  = 1x Tclkio = 83.333 ns
#		define CONF_CANBT2	0x08	// Tsync = 1x Tscl, Tprs = 5x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x25	// Tpsh1 = 3x Tscl, Tpsh2 = 3x Tscl, 3 sample points
#else
#error "This CAN_BAUDRATE value is not defined"
#endif

#elif F_CPU == 8000000				//!< Fclkio = 8 MHz, Tclkio = 125 ns
#if 	CAN_BAUDRATE == 100		//!< -- 100Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x08		// Tscl  = 5x Tclkio = 625 ns
#		define CONF_CANBT2	0x0C		// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x37		// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 125		//!< -- 125Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x06		// Tscl  = 4x Tclkio = 500 ns
#		define CONF_CANBT2	0x0C		// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x37		// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 200		//!< -- 200Kb/s, 20x Tscl, sampling at 75%
#		define CONF_CANBT1	0x02		// Tscl  = 2x Tclkio = 250 ns
#		define CONF_CANBT2	0x0E		// Tsync = 1x Tscl, Tprs = 8x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x4B		// Tpsh1 = 6x Tscl, Tpsh2 = 5x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 250		//!< -- 250Kb/s, 16x Tscl, sampling at 75%
#		define CONF_CANBT1	0x02		// Tscl  = 2x Tclkio = 250 ns
#		define CONF_CANBT2	0x0C		// Tsync = 1x Tscl, Tprs = 7x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x37		// Tpsh1 = 4x Tscl, Tpsh2 = 4x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 500		//!< -- 500Kb/s, 8x Tscl, sampling at 75%
#		define CONF_CANBT1	0x02		// Tscl  = 2x Tclkio = 250 ns
#		define CONF_CANBT2	0x04		// Tsync = 1x Tscl, Tprs = 3x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x13		// Tpsh1 = 2x Tscl, Tpsh2 = 2x Tscl, 3 sample points
#elif	CAN_BAUDRATE == 1000	//!< -- 1 Mb/s, 8x Tscl, sampling at 75%
#		define CONF_CANBT1	0x00		// Tscl  = 1x Tclkio = 125 ns
#		define CONF_CANBT2	0x04		// Tsync = 1x Tscl, Tprs = 3x Tscl, Tsjw = 1x Tscl
#		define CONF_CANBT3	0x13		// Tpsh1 = 2x Tscl, Tpsh2 = 2x Tscl, 3 sample points
#else
#error "This CAN_BAUDRATE value is not defined"
#endif

#else
#error "This FOSC value is not defined"
#endif

#define CAN_RESET()		( CANGCON  =  (1<<SWRES) )
#define CAN_ENABLE()	( CANGCON |=  (1<<ENASTB))
#define CAN_DISABLE()	( CANGCON &= ~(1<<ENASTB))
#define CAN_FULL_ABORT(){ CANGCON |=  (1<<ABRQ); CANGCON &= ~(1<<ABRQ); }

#define NB_CANIT_CB (9) //!< Number of canit callbacks.

#define NB_MOB		( 15		) //!< Number of MOB's
#define NB_DATA_MAX	( 8			) //!< The can can max transmit a payload of 8 uint8_t
#define LAST_MOB_NB	( NB_MOB-1	) //!< Index of the last MOB. This is useful when looping over all MOB's

#define MOB_Tx_ENA	( 1 << CONMOB0 ) //!< Mask for Enabling Tx on the current MOB
#define MOB_Rx_ENA	( 2 << CONMOB0 ) //!< Mask for Enabling Rx on the current MOB
#define MOB_Rx_BENA	( 3 << CONMOB0 ) //!< Mask for Enabling Rx with buffer enabled for the current MOB

#define DLC_MSK			((1<<DLC3)|(1<<DLC2)|(1<<DLC1)|(1<<DLC0)) //!< Mask for Data Length Coding bits in CANCDMOB
#define MOB_CONMOB_MSK	((1 << CONMOB1) | (1 << CONMOB0)		) //!< Mask for Configuration MOB bits in CANCDMOB

enum can_int_t {
	CANIT_RX_COMPLETED_DLCW =	0, //!< Data length code warning.
	CANIT_RX_COMPLETED =		1, //!< Receive completed.
	CANIT_TX_COMPLETED =		2, //!< Transmit completed.
	CANIT_ACK_ERROR =			3, //!< No detection of the dominant bit in the acknowledge slot.

	/**
	* @brief
	* The form error results from one or more violations of the fixed form in
	* the following bit fields:
	*  + CRC delimiter
	*  + Acknowledgment delimiter
	*  + EOF
	*/
	CANIT_FORM_ERROR =			4,

	/**
	* @brief
	* The receiver performs a CRC check on every de-stuffed received message
	* from the start of frame up to the data field. If this checking does not
	* match with the de-stuffed CRC field, a CRC error is set.
	*/
	CANIT_CRC_ERROR =			5,
	CANIT_STUFF_ERROR =			6, //!< Detection of more than five consecutive bits with same value.
	CANIT_BIT_ERROR =			7, //!< Bit Error (Only in Transmission).
	CANIT_DEFAULT =				8  //!< This is hopefully temporarily. Should not be possible! Needs testing.
};

enum mob_mode_t {
	MOB_DISABLED,	//!< In this mode, the MOb is disabled.
	MOB_TRANSMIT,	//!< The mob is set in Transmit mode.
	MOB_RECIEVE,	//!< The mob is set in Receive mode.

	/**
	* @brief
	* A reply (data frame) to a remote frame can be automatically sent after
	* reception of the expected remote frame.
	*/
	MOB_AUTOMATIC_REPLY,

	/**
	* @brief
	* This mode is useful to receive multi frames. The priority between MObs
	* offers a management for these incoming frames. One set MObs (including
	* non-consecutive MObs) is created when the MObs are set in this mode. Due
	* to the mode setting, only one set is possible. A frame buffer completed
	* flag (or interrupt) - BXOK - will rise only when all the MObs of the set
	* will have received their dedicated CAN frame.
	*/
	MOB_FRAME_BUFF_RECEIVE
};

/**
* @brief
* Different states that CANSTMOB can take. This is very useful for fx. making a
* conditional switch on the given status of the MOB.
*/
enum mob_status_t {
	MOB_NOT_COMPLETED 		= ( 0x00 ),													//!< 0x00
	MOB_TX_COMPLETED 		= ( 1<<TXOK ),												//!< 0x40
	MOB_RX_COMPLETED 		= ( 1<<RXOK ),												//!< 0x20
	MOB_RX_COMPLETED_DLCW 	= ( (1<<RXOK)|(1<<DLCW) ),									//!< 0xA0
	MOB_ACK_ERROR 			= ( 1<<AERR ),												//!< 0x01
	MOB_FORM_ERROR 			= ( 1<<FERR ),												//!< 0x02
	MOB_CRC_ERROR 			= ( 1<<CERR ),												//!< 0x04
	MOB_STUFF_ERROR 		= ( 1<<SERR ),												//!< 0x08
	MOB_BIT_ERROR 			= ( 1<<BERR ),												//!< 0x10
	MOB_PENDING 			= ( (1<<RXOK)|(1<<TXOK) ),									//!< 0x60
	MOB_NOT_REACHED 		= ( (1<<AERR)|(1<<FERR)|(1<<CERR)|(1<<SERR)|(1<<BERR) ),	//!< 0x1F
 };

typedef struct can_msg_t {
	uint8_t mob;				//!< Message Object to bind to
	uint16_t id;				//!< Message id / priority
	uint8_t dlc;				//!< Data Length Code
	uint8_t data[NB_DATA_MAX];	//!< The message payload. Specification states a max length of 8 regardless of dlc
	enum mob_mode_t mode;		//!< Denotes the mode that this msg is send or received
} can_msg_t;


//_____ M A C R O S ____________________________________________________________

/**
 * @name MOB Transmit and Receive
 * Transmit or receive data on the current MOB
 * @{
 */
#define MOB_TX_DATA(data, len)			{ uint8_t i; \
											for (i = 0; i < len; ++i) \
												{ CANMSG = data[i]; } } //!< Put data onto the can
#define MOB_RX_DATA(data, len)			{ uint8_t i; \
											for (i = 0; i < len; ++i) \
												{ data[i] = CANMSG;} } //!< Get data from the can
/** @} */

/**
 * @name CAN status Interrupt register
 * @{
 */
#define MOB_HAS_PENDING_INT(mob)	( BIT_CHECK(CANSIT2 + (CANSIT1 << 8), (mob))) //!< Check if the given mob has a pending interrupt.
/** @} */


#define CAN_SET_MOB(mob)			( CANPAGE = ((mob) << 4)		) //!< Set the can the the specified MOB

/**
 * @name MOB interrupt
 * Enable or disable interrupts on the specified MOB
 * @{
 */
#define CAN_ENABLE_MOB_INTERRUPT(mob)	{	CANIE2 |= ((1 << mob) & 0xff); \
											CANIE1 |= (((1 << mob) >> 8) & 0x7f); }

#define CAN_DISABLE_MOB_INTERRUPT(mob)	{	CANIE2 &= !((1 << mob) & 0xff); \
											CANIE1 &= !(((1 << mob) >> 8) & 0x7f);}
/** @} */

/**
 * @name Data Length Code
 * Getter and setter for the length of data that the given MOB holds
 * @{
 */
#define MOB_GET_DLC()		( BITMASK_CHECK(CANCDMOB, DLC_MSK) >> DLC0	) //!< Calculates the DLC that is set for the current MOB. @return The DLC sat for the current MOB
#define MOB_SET_DLC(dlc)	( BITMASK_SET(CANCDMOB, dlc)				) //!< Set the DLC for the current MOB
/** @} */

/**
 * @name MOB ID
 * @{
 */
#define MOB_SET_STD_ID_10_4(id) 		(	((*((uint8_t *)(&(id)) + 1)) << 5) + \
											((*(uint8_t *)(&(id))) >> 3)			)

#define MOB_SET_STD_ID_3_0(id) 			(	(*(uint8_t *)(&(id))) << 5 				)

#define MOB_SET_STD_ID(id) 				{	CANIDT1 = MOB_SET_STD_ID_10_4((id)); \
											CANIDT2 = MOB_SET_STD_ID_3_0((id)); \
											CANCDMOB &= (~(1<<IDE));				}

#define MOB_SET_STD_MASK_FILTER(mask)	{	CANIDM1 = MOB_SET_STD_ID_10_4(mask); \
											CANIDM2 = MOB_SET_STD_ID_3_0( mask);	}

#define MOB_SET_STD_FILTER_FULL()		{	uint32_t __filterMask_ = UINT32_MAX; \
											MOB_SET_STD_MASK_FILTER(__filterMask_); }

#define MOB_SET_STD_FILTER_NONE()		{	uint32_t __filterMask_ = 0; \
											MOB_SET_STD_MASK_FILTER(__filterMask_); }
/** @} */

/**
 * @name Clear MOB status
 * Clear all MOB registers for a single MOB and
 * clears interrupt status register.
 * @{
 */
#define MOB_CLEAR_STATUS()				{   uint8_t  volatile *__i_; \
											for (__i_ =& CANSTMOB; __i_ < &CANSTML; ++__i_) \
												{ *__i_= 0x00; }					}
#define MOB_CLEAR_INT_STATUS()			( CANSTMOB = 0x00	) //!< Clears the interrupt status for the current MOB
/** @} */

/**
 * @name Configuration of Message Object
 * These bits set the communication to be performed (no initial value after
 * RESET).
 * @note These bits are *NOT* cleared once communication is performed. The user
 * must re-write the configuration to enable new communication.
 * @{
 */
#define MOB_ABORT()				( BITMASK_CLEAR(CANCDMOB, MOB_CONMOB_MSK)					) //!< Disable MOB
#define MOB_EN_TX()				{ BIT_CLEAR(CANCDMOB, CONMOB1); BIT_SET(CANCDMOB, CONMOB0);	} //!< Enable MOB Transmission
#define MOB_EN_RX()				{ BIT_SET(CANCDMOB, CONMOB1); BIT_CLEAR(CANCDMOB, CONMOB0);	} //!< Enable MOB Reception
#define MOB_EN_FRM_BUFF_RX()	( BITMASK_SET(CANCDMOB, MOB_CONMOB_MSK)						) //!< Enable MOB Frame Buffer Reception
/** @} */

/**
 * @name Can interrupt mode
 * Enables full function mode, recieve mode and transmit mode respectivly.
 * @{
 */
#define CAN_INIT_ALL()	BITMASK_SET(CANGIE, (1<<ENIT|1<<ENRX|1<<ENTX)
#define CAN_INIT_RX()	BITMASK_SET(CANGIE, (1<<ENIT|1<<ENRX)
#define CAN_INIT_TX()	BITMASK_SET(CANGIE, (1<<ENIT|1<<ENTX)
/** @} */

/**
 * @name CAN Bit Timing Register
 * Set Baud Rate Prescaler, Re-Synchronization Jump Width,
 * Propagation Time Segment, Sample Point(s) and Phase Segments.
 * Depends on FOSC and CAN_BAUDRATE.
 * @{
 */
#define CAN_CONF_CANBT()	{ CANBT1=CONF_CANBT1; CANBT2=CONF_CANBT2; CANBT3=CONF_CANBT3; }
/** @} */


//_____ D E C L A R A T I O N S ________________________________________________

void set_canit_callback(enum can_int_t interrupt, canit_callback_t callback);
uint8_t can_init(void);
int can_setup(can_msg_t *msg);
int can_receive(can_msg_t *msg);
int can_send(can_msg_t *msg);
void can_clear_all_mob(void);

#endif /* CAN_H */
