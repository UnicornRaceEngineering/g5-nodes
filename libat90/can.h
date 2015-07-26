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
#include <stdbool.h>

#include "system_messages.h"


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
	ID_ERR,
	TOTAL_ERR,
};


struct can_message {
	uint16_t id;
	uint8_t len;
	uint8_t data[8];
};


void can_init(void);
uint8_t can_broadcast(const enum message_id id, const void* msg);
uint16_t get_counter(enum can_counters counter);
void read_message(struct can_message* msg);
bool can_has_data(void);

#endif /* CAN_H */
