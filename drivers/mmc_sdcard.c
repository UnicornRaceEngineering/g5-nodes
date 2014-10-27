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

#include <stddef.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <spi.h>
#include <io.h>

#include "mmc_sdcard.h"

#define SS_L()	IO_SET_LOW(SPI_PORT, SS_PIN);
#define SS_H()	IO_SET_HIGH(SPI_PORT, SS_PIN);

#define IDLE_BYTE	0xFF

#define CMD0_CRC	0x4A // With arg 0x00
#define CMD8_CRC	0x43 // With arg 0x1AA

#define RX_START_MSK	(1<<7)

/**
 * A SD command is a 8 bit value that aways starts with 01. The 6 remaining
 * bytes indicate the command number. So we ands the command number with a
 * mask of the remaining 6 bits and sets the sixth bit.
 * @param  x Command number
 * @return   The correctly formatted command
 */
#define CMD(x) 		((1<<6) | (x & 0x3F))

#define ACMD_FLAG	(1<<7)
#define ACMD(x)		(CMD(x) | ACMD_FLAG) // We only set bit 7 so we can check for ACMD later. This should be cleared before sending the cmd

#define IS_ACMD(cmd)	(cmd & ACMD_FLAG)

/**
 * See 7.3.1.3 Detailed Command Description in Physical Layer Simplified
 * Specification which can be found on
 * https://www.sdcard.org/downloads/pls/simplified_specs/
 */
enum command {
	GO_IDLE_STATE 		= CMD(0),  //!< arg: None 				, Respone: R1

	SEND_IF_COND		= CMD(8),  //!< arg: 32-bit [11:8] Voltage supplied (VHS) [7:0] Check pattern, Response R7
	SET_BLOCKLEN 		= CMD(16), //!< arg: 32-bit block length, Respone: R1
	READ_SINGLE_BLOCK	= CMD(17), //!< arg: 32-bit block adress, Respone: R1
	WRITE_BLOCK			= CMD(24), //!< arg: 32-bit block adress, Respone: R1
	SEND_ACMD			= CMD(55), //!< arg: None 				, Respone: R1
	READ_OCR			= CMD(58), //!< arg: None 				, Respone: R3

	/**
	 * Sends host capacity support information and activates the card's
	 * initialization process. Reserved bits shall be set to '0'
	 * args:
	 *  [31]   Reserved
	 *  [30]   HCS
	 *  [29:0] Reserved
	 */
	SD_SEND_OP_COND		= ACMD(41),
};

enum VHS_masks {
	VHS_NOT_DEFINED = 0x00,
	VHS_27_36V 		= (1<<0),
	VHS_LOW_VOLTAGE	= (1<<1),
	VHS_RESERVED	= (1<<2)|(1<<3),
};

enum card_type {
	CT_MMC 		= (1<<0), //!< MMC ver 3
	CT_SD1 		= (1<<1), //!< SD ver 1
	CT_SD2 		= (1<<2), //!< SD ver 2
	CT_HC_XC 	= (1<<3), //!< Block addressing Card is either SDHC or SDXC
};

#define USES_BLOCK_ADDRESSING(ct)	(ct & CT_HC_XC)

enum response_type {
	R1,
	R1B,
	R2,
	R3,
	R7,
};

#if 1
enum R1_masks {
	R1_START_BIT 			= (1<<7), // Always zero
	R1_PARAMETER_ERROR 		= (1<<6),
	R1_ADDRESS_ERROR		= (1<<5),
	R1_ERASE_SEQUENCE_ERROR = (1<<4),
	R1_CRC_ERROR			= (1<<3),
	R1_ILLEGA_COMMAND		= (1<<2),
	R1_ERASE_RESET			= (1<<1),
	R1_IN_IDLE_STATE		= (1<<0),
};
#endif

enum response_length {
	R1_LEN = 1,
	R2_LEN = 2,
	R3_LEN = 5,
	R7_LEN = 5,
};

struct response {
	enum response_type type;
	union {
		uint8_t r1[R1_LEN];
		uint8_t r2[R2_LEN];
		uint8_t r3[R3_LEN];
		uint8_t r7[R7_LEN];
	};
};

static int8_t send_cmd(enum command cmd, uint32_t arg, struct response *r);


enum card_type card_type = 0;

static inline void tx(uint8_t x) {
	spi_tranceive(x);
}

static inline uint8_t rx(void) {
	return spi_tranceive(IDLE_BYTE);
}

/**
 * Sends an idle byte to the sd card. This is usefull when we know we need to
 * delay the sd card a certain number of cyckles.
 * @note if we delay 10 times we will send 10 idle bytes giving us 8*10 = 80
 * delay cyckles.
 * @param n Number of delay bytes to send
 */
static void delay(int n) {
	SS_L();
	for (int i = 0; i < n; ++i) tx(IDLE_BYTE);
	SS_H();
}

#if 0
static void check_r1(uint8_t r1) {
	usart1_printf("r1: %#02x ", r1);

	if (r1 & R1_START_BIT) 				usart1_printf("Start bit not zero ");
	if (r1 & R1_PARAMETER_ERROR) 		usart1_printf("Parmeter error ");
	if (r1 & R1_ADDRESS_ERROR) 			usart1_printf("Address error ");
	if (r1 & R1_ERASE_SEQUENCE_ERROR) 	usart1_printf("Erase sequence error ");
	if (r1 & R1_CRC_ERROR) 				usart1_printf("CRC error ");
	if (r1 & R1_ILLEGA_COMMAND) 		usart1_printf("Illegal command ");
	if (r1 & R1_ERASE_RESET) 			usart1_printf("Erase reset ");
	if (r1 & R1_IN_IDLE_STATE) 			usart1_printf("In idle state ");
	usart1_putc('\n');
}
#endif

static int8_t send_cmd(enum command cmd, uint32_t arg, struct response *r) {
	SS_L();

	// Transmit the cmd
	{
		if (IS_ACMD(cmd)) {
			SS_H();
			struct response res = {.type = R1};
			if (send_cmd(SEND_ACMD, 0, &res) != 0) return 1;
			BITMASK_CLEAR(cmd, ACMD_FLAG); // Clear the ACMD bit so we have a valid cmd
			SS_L();
		}

		// Send the command
		tx((uint8_t)cmd);

		// Send the arguments MSB first
		for (int8_t i = sizeof(arg)-1; i >= 0; --i) tx(((uint8_t*)&arg)[i]);

		// Send the CRC value. This is only mandetory for CMD0 unless we
		// specificly enable CRC checking.
		uint8_t crc7 = 0x00;
		if ((cmd == CMD(0)) && (arg == 0x00)) 	crc7 = CMD0_CRC;
		if ((cmd == CMD(8)) && (arg == 0x1AA)) 	crc7 = CMD8_CRC;
		crc7 = (crc7<<1) | (1<<0); // Set the stop bit
		tx(crc7);
	}

	// Wait for a valid response and then store it
	{
		switch (cmd) {
			case GO_IDLE_STATE: 		r->type = R1; break;
			case SEND_IF_COND: 			r->type = R7; break;
			case SET_BLOCKLEN:			r->type = R1; break;
			case READ_SINGLE_BLOCK: 	r->type = R1; break;
			case WRITE_BLOCK: 			r->type = R1; break;
			case SEND_ACMD:				r->type = R1; break;
			case READ_OCR:				r->type = R3; break;
			case SD_SEND_OP_COND:		r->type = R1; break;

			default: 					r->type = R1; break;
		}

		int8_t response_len;
		uint8_t *res_buff; // Will be pointer to the response buffer
		switch (r->type) {
			case R1: 	// Fall through
			case R1B: 	response_len = R1_LEN; res_buff = r->r1; break;
			case R2: 	response_len = R2_LEN; res_buff = r->r2; break;
			case R3: 	response_len = R3_LEN; res_buff = r->r3; break;
			case R7:	response_len = R7_LEN; res_buff = r->r7; break;
			default: return 1; // No response type given.
		}

		int8_t num_retries = 10;
		do {
			res_buff[response_len-1] = rx(); // We recv MSB first
		} while(((res_buff[response_len-1] & RX_START_MSK) != 0) && num_retries--);

		if (!(num_retries >= 0)) {
			SS_H();
			return 1; // Error no response received before timeout
		}

		// Recv MSB first. We already stored the first byte so substract one more
		for (int8_t i = response_len-2; i >= 0; --i) res_buff[i] = rx();
	}

	SS_H();

	return 0;
}

/**
 * Flow descriped on page 171
 * @return  0 on success 1 on failure
 */
int8_t sd_spi_mode_initialization(void) {
	delay(100); // Delay atleast 74 cyckles

	struct response res = {0};
	int16_t retries = 0;

	// Step 1 tell the card to go into idle state to activate SPI mode
	if (send_cmd(GO_IDLE_STATE, 0, &res) != 0) return 1;

	// Build the if condition args checking if the card can work at vdd range
	uint32_t if_cond_args = 0x00;
	((uint8_t*)&if_cond_args)[0] = 0xAA; 		// Echo value
	((uint8_t*)&if_cond_args)[1] = VHS_27_36V; 	// Voltage acceptence range
	card_type = (send_cmd(SEND_IF_COND, if_cond_args, &res) == 0) ? CT_SD2 : 0;

	if (card_type == CT_SD2) {
		// SDv2 because CMD8 is supported

		const uint8_t echo_back = res.r7[0];
		const uint8_t voltage_accepted = LOW_NIBBLE(res.r7[1]);
		if (!(voltage_accepted == VHS_27_36V
			&& echo_back == ((uint8_t*)&if_cond_args)[0])) return 1;

		// Wait for the card to leave idle state and activate it. We have High
		// Capacity Support (HCS) so set HCS bit high.
		for (retries = 10000; retries != 0; --retries) {
			send_cmd(SD_SEND_OP_COND, (1UL<<30), &res);
			if (!(res.r1[0] & R1_IN_IDLE_STATE)) break;
			_delay_us(100);
		}

		if (retries <= 0 || (send_cmd(READ_OCR, 0, &res) != 0)) return 1;

		uint32_t *ocr = (uint32_t*)&res.r3[1]; // R3 byte[1:4] is OCR
		card_type = (*ocr & (1UL<<29)) ? (CT_SD2|CT_HC_XC) : CT_SD2;
	} else {
		// SDv1 or MMCv3
		if (send_cmd(SD_SEND_OP_COND, 0, &res) != 0) return 1;

		// Check if any error has occourd to determain between SDv1 and MMC
		card_type = (res.r1[0] <= 1) ? CT_SD1 : CT_MMC;
		const enum command cmd = (CT_SD1) ? ACMD(41) : CMD(1);

		// Wait for card to leave idle state
		for (retries = 10000; retries != 0; --retries) {
			send_cmd(cmd, 0, &res);
			if (!(res.r1[0] & R1_IN_IDLE_STATE)) break;
			_delay_us(100);
		}

		if ((retries <= 0)
			|| (send_cmd(SET_BLOCKLEN, SD_BLOCKSIZE, &res) != 0)) return 1;
	}

	return 0;
}

int8_t sd_read_block(uint8_t *buff, int32_t sector, int16_t offset,
					 int16_t n) {
	if (buff == NULL) return 1;
	if (!(offset+n <= SD_BLOCKSIZE)) return 1;

	// Check if card uses block or byte addressing
	if (!USES_BLOCK_ADDRESSING(card_type)) sector *= SD_BLOCKSIZE;

	struct response r;
	if (send_cmd(READ_SINGLE_BLOCK, sector, &r) != 0) return 1;

	SS_L();

	int16_t retries = 40000;
	uint8_t rc;
	// Wait for data packet
	do rc = rx(); while (rc == IDLE_BYTE && --retries);

	if (rc != 0xFE) {
		SS_H();
		return 1;
	}

	int16_t remaining = (SD_BLOCKSIZE+sizeof(uint16_t)) - offset - n;

	// Throw away everything before the offset
	if (offset != 0) do rx(); while (--offset);

	// Save n bytes in the buffer
	do *buff++ = rx(); while (--n);

	// Throw away trailing bytes and the CRC value (the sizeof(uint16_t))
	do rx(); while(--remaining);

	SS_H();
	return 0;
}

int8_t sd_write_block(uint8_t *data, int32_t sector, size_t n) {
	if (data == NULL) return 1;

	// Initiate sector write
	{
		struct response r;
		// Check if card uses block or byte addressing
		if (!USES_BLOCK_ADDRESSING(card_type)) sector *= SD_BLOCKSIZE;
		if (send_cmd(WRITE_BLOCK, sector, &r) != 0) return 1;
		if (r.r1[0] != 0) return 1;
		SS_L();

		const uint16_t header = 0xFFFE;
		tx(((uint8_t*)&header)[1]); // Send datablock header MSB
		tx(((uint8_t*)&header)[0]);
	}

	int16_t remaining = SD_BLOCKSIZE; // remaining bytes in this block

	// Send data
	{
		while (n-- && remaining--) tx(*data++);
	}

	// Finalize the write
	{
		while(remaining--) tx(0x00);
		const uint16_t crc16 = 0;
		tx(((uint8_t*)&crc16)[1]); // MSB first
		tx(((uint8_t*)&crc16)[0]);

		// Check response
		// bits: [7|6|5|4| 3|2|1  |0]
		//       [x|x|x|0| Status |1]
		switch (((rx() & 0x1F))) {
			case 0x05:  // [3|2|1] = [010] Data Accepted.
				break;  // Continue execution

			case 0x0B: // [3|2|1] = [101] Data rejected due to a CRC error
			case 0x0D: // [3|2|1] = [110] Data Rejected due to a Write Error
			default:
				SS_H();
				return 1;
		}
		// Wait for end of write with a timeout of 500ms
		for (int timeout = 5000; rx() != IDLE_BYTE; --timeout) _delay_us(100);

		SS_H();
	}

	return 0;
}
