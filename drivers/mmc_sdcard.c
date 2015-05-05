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

#include <stddef.h> // size_t
#include <string.h> // memcpy()
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <spi.h>
#include <io.h>

#include "mmc_sdcard.h"

/**
 * @file mmc_sdcard.c
 * Low level driver for reading and writing SD cards using SPI. Information on
 * the specification can be found at:
 * https://www.sdcard.org/downloads/pls/simplified_specs/
 * In the "Physical Layer Simplified Specification" document.
 */

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
	SEND_CSD			= CMD(9),  //!< arg stuffbit, Response R1
	SEND_CID			= CMD(10),  //!< arg stuffbit, Response R1
	STOP_TRANSMISSION	= CMD(12), //!< arg: 32-bit stuff bits,	  Response: R1b
	SET_BLOCKLEN 		= CMD(16), //!< arg: 32-bit block length, Response: R1
	READ_SINGLE_BLOCK	= CMD(17), //!< arg: 32-bit block adress, Response: R1
	READ_MULTIPLE_BLOCK	= CMD(18), //!< arg: 32-bit block adress, Response: R1
	WRITE_BLOCK			= CMD(24), //!< arg: 32-bit block adress, Response: R1
	WRITE_MULTIPLE_BLOCK= CMD(25), //!< arg: 32-bit block adress, Response: R1
	SEND_ACMD			= CMD(55), //!< arg: None 				, Response: R1
	READ_OCR			= CMD(58), //!< arg: None 				, Response: R3

	/**
	 * Sends host capacity support information and activates the card's
	 * initialization process. Reserved bits shall be set to '0'
	 * args:
	 *  [31]   Reserved
	 *  [30]   HCS
	 *  [29:0] Reserved
	 *  Response R1
	 */
	SD_SEND_OP_COND		= ACMD(41),

	/**
	 * Set the number of write blocks to be preerased before writing (to be used
	 * for faster Multiple Block WR com mand).
	 * args:
	 * [31:23]	stuff bits
	 * [22:0]	Number of blocks
	 * Response: R1
	 */
	SET_WR_BLK_ERASE_COUNT = ACMD(23),
};

#define R_TYPE(cmd) ((const enum response_type[]) { \
	[GO_IDLE_STATE] 			= R1, \
	[SEND_IF_COND] 				= R7, \
	[STOP_TRANSMISSION] 		= R1B, \
	[SET_BLOCKLEN] 				= R1, \
	[READ_SINGLE_BLOCK]			= R1, \
	[READ_MULTIPLE_BLOCK]		= R1, \
	[WRITE_BLOCK]				= R1, \
	[WRITE_MULTIPLE_BLOCK]		= R1, \
	[SEND_ACMD]					= R1, \
	[READ_OCR]					= R3, \
	[SD_SEND_OP_COND]			= R1, \
	[SET_WR_BLK_ERASE_COUNT]	= R1, \
	[SEND_CSD] 					= R1, \
	[SEND_CID] 					= R1, \
}[cmd])

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

enum OCR_masks {
	OCR_RESERVED 					= (0x1E007F7F),
	OCR_RESERVED_FOR_LOW_VOLTAGE 	= (1UL<<7),
	OCR_27_28						= (1UL<<15),
	OCR_28_29						= (1UL<<16),
	OCR_29_30						= (1UL<<17),
	OCR_30_31						= (1UL<<18),
	OCR_31_32						= (1UL<<19),
	OCR_32_33						= (1UL<<20),
	OCR_33_34						= (1UL<<21),
	OCR_34_35						= (1UL<<22),
	OCR_35_36						= (1UL<<23),
	OCR_S18A						= (1UL<<24), // Switching to 1.8V Accepted
	OCR_UHS_II						= (1UL<<29),
	OCR_CCS							= (1UL<<30), // Card Capacity Status (CCS)
	OCR_BUSY						= (1UL<<31), // Is LOW if card has not
												 // powered up yet
};

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

enum card_type card_type = 0;

/**
 * Transmit a byte to the sd card
 * @param x Byte that is trasmitted
 */
static inline void tx(uint8_t x) {
	spi_tranceive(x);
}

/**
 * Receive a byte from the sd card
 * @return  Byte received
 */
static inline uint8_t rx(void) {
	return spi_tranceive(IDLE_BYTE);
}

/**
 * SDSC Card (CCS=0) uses byte unit address and SDHC and SDXC Cards (CCS=1) use
 * block unit address (512 bytes unit). Therefor we have to convert to the
 * correct sector.
 * @param  sector Input sector
 * @return        Sector in adjusted form that the card type accepts
 */
static inline uint32_t adjust_sector(uint32_t sector) {
	return (!USES_BLOCK_ADDRESSING(card_type)) ? sector * SD_BLOCKSIZE : sector;
}

/**
 * Sends an idle byte to the sd card. This is usefull when we know we need to
 * delay the sd card a certain number of cyckles.
 * @note if we delay 10 times we will send 10 idle bytes giving us 8*10 = 80
 * delay cyckles.
 * @param n Number of delay bytes to send
 */
static void idle_clock(int n) {
	SS_L();
	for (int i = 0; i < n; ++i) tx(IDLE_BYTE);
	SS_H();
}

static int wait_ready(uint16_t ms) {
	for (ms *= 10; rx() != IDLE_BYTE; --ms) _delay_us(100);
	return (ms == 0) ? -1: 0;
}

/**
 * Sends a command to the SD card
 * @param  cmd The command send to the SD card
 * @param  arg The command arguments
 * @param  r   pointer to where the response is stored
 * @return     0: success 1: failure
 */
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

		if (R_TYPE(cmd) == R1B) {
			//!< @TODO use timer tick to make this take about 200ms
			int16_t max_idles = 20000/4;
			while (rx() == IDLE_BYTE && --max_idles);
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
		r->type = R_TYPE(cmd);

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


		int8_t num_retries = 10*10;
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
 * Reads the OCR from the sd card and places the result at the given pointer
 * @param  ocr[OUT] pointer to where the OCR value is stored
 * @return          0 on success 1 on failure
 */
static int8_t read_ocr(uint32_t *ocr) {
	struct response r;
	const int8_t rc = send_cmd(READ_OCR, 0, &r);
	if (rc != 0 || r.r1[0] != 0) return 1;

	memcpy(ocr, r.r3, sizeof(*ocr));

	return 0;
}

/**
 * Flow descriped on page 171
 * @return  0:success 1:failure
 */
static int8_t sd_spi_mode_initialization(void) {
	idle_clock(10); // Delay atleast 74 cyckles (10*8 = 80)

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

		uint32_t ocr;
		if (retries <= 0 || read_ocr(&ocr) != 0) return 1;
		card_type = (ocr & OCR_CCS) ? (CT_SD2|CT_HC_XC) : CT_SD2;
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

/**
 * Inits the sd card. This must be called before any other sd functionality can
 * be performed.
 * @return  1:success 0:failure
 */
int8_t sd_init(void) {
	spi_init_master(false, SPI_PRESCALER_64); // SPI_F < 400 kHz
	if (sd_spi_mode_initialization() != 0) return 1;
	spi_init_master(false, SPI_PRESCALER_4); // SPI_F max
	return 0;
}


static int recv_block(uint8_t *buf, size_t n) {
	SS_L();
	int16_t retries = 200; // should be ca. 200ms

	// Wait for ready response. If sdcard is not ready in time return error
	while (rx() != 0xFE && --retries);
	if (!retries) {
		SS_H();
		return -1;// Error
	}

	do {
		*buf++ = rx();
	} while (n--);

	// Discard CRC
	rx();
	rx();

	SS_H();

	return 0;
}


int8_t sd_read(uint8_t *buff, uint32_t sector, size_t n) {
	sector = adjust_sector(sector);

	const uint8_t cmd = (n > 1) ? READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK;
	if (send_cmd(cmd, sector, &(struct response){0}) != 0) return -1; // Error

	do {
		if (recv_block(buff, SD_BLOCKSIZE) != 0) break;
	} while (--n);
	if (cmd == READ_MULTIPLE_BLOCK) send_cmd(STOP_TRANSMISSION, 0, &(struct response){0});

	return n ? -1 : 0;
}

static int send_block(const uint8_t buf[SD_BLOCKSIZE], uint8_t token) {
	SS_L();

	tx(token);

	if (buf != NULL) {
		for (size_t i = 0; i < SD_BLOCKSIZE; ++i) tx(*buf++);

		// tx Dummy CRC value
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
				return -1;
		}
	}

	// Wait for end of write with a timeout of 500ms
	if (wait_ready(500) != 0) {
		SS_H();
		return -1;
	}

	SS_H();

	return 0;
}

int8_t sd_write(const uint8_t *buf, uint32_t sector, size_t n) {
	if (buf == NULL) return 1;
	sector = adjust_sector(sector);

	if (n == 1) {
		// Write single block
		struct response r = {0};
		if (send_cmd(WRITE_BLOCK, sector, &r) != 0) return -1;
		if (send_block(buf, 0xFE) != 0) return -1;
	} else {
		// Write multi block
		if (!USES_BLOCK_ADDRESSING(card_type)) {
			send_cmd(SET_WR_BLK_ERASE_COUNT, n, &(struct response){0});
		}
		if (send_cmd(WRITE_MULTIPLE_BLOCK, sector, &(struct response){0}) != 0) return -1;
		while (n--) if (send_block(buf, 0xFC) != 0) return -1;
		if (send_block(NULL, 0xFD) != 0) return -1;
	}

	SS_H();

	return 0;
}

int sd_sync(void) {
	return wait_ready(500);
}

int read_csd(uint8_t csd[static CSD_SIZE]) {

	if (send_cmd(SEND_CSD, 0, &(struct response){0}) != 0) return -1;
	if (recv_block(csd, CSD_SIZE) != 0) return -1;
	return 0;
}

int read_cid(uint8_t cid[static CID_SIZE]) {
	if (send_cmd(SEND_CID, 0, &(struct response){0}) != 0) return -1;
	if (recv_block(cid, CID_SIZE) != 0) return -1;
	return 0;
}

/**
 * Return the memory capacity of the card (in KBytes as per the specification)
 * @param  memory_capacity The cards memory capacity
 * @return                 Non zero error code
 */
int get_memory_capacity(uint32_t *memory_capacity) {
	uint8_t csd[CSD_SIZE] = {0};
	if (read_csd(csd) != 0) return -1;

	if (CSD_STRUCTURE(csd) == 1) {
		// CSD version 2.0

		const uint32_t c_size = CSD_V2_C_SIZE(csd);
		*memory_capacity = (c_size + 1) * 512; // (c_size + 1) * 512KByte
	} else {
		// CSD version 1.0

		const uint32_t c_size = EXTRACT_AT_INDEX(csd, CSD_SIZE, 73, 6);
		const uint16_t mult = 1 << (CSD_V1_C_SIZE_MULT(csd) + 2); // 2^c_size_mult+2
		const uint16_t block_len = 1 << CSD_V1_READ_BL_LEN(csd); // 2^read_bl_len
		const uint32_t blocknr = (c_size + 1) * mult;

		*memory_capacity = blocknr * block_len;
	}

	return 0;
}
