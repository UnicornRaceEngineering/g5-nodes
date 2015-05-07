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
*   Used for setting up the CAN subsystem
*   and sending or receiving via the CAN.
*   This header file also contains many
*   driver functions in the form of macros
*/

#ifndef CAN_H
#define CAN_H

#include <stdint.h>

/**
 * Interrupt callback function pointer for can receive interrupt.
 * @param id	ID of incomming message.
 * @param len	Length of incomming message.
 * @param *msg	Pointer to message on the heap.
 */

typedef uint8_t (*canrec_callback_t)(uint16_t id, uint16_t len, uint8_t *msg);

typedef struct can_filter_t {
	uint16_t lower_bound;
	uint16_t upper_bound;
} can_filter_t;

enum can_counters{
	SUCCES,
	DLCW_ERR,
	RX_COMP,
	TX_COMP,
	ACK_ERR,
	FORM_ERR,
	CRC_ERR,
	STUFF_ERR,
	BIT_ERR,
	NO_MOB_ERR,
	ALLOC_ERR,
	TOTAL_ERR,
};


void can_init(can_filter_t, can_filter_t);
uint8_t can_send(const uint16_t id, const uint16_t len, const uint8_t* msg);
void set_canrec_callback(canrec_callback_t callback);
uint16_t get_counter(enum can_counters counter);

#endif /* CAN_H */
