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
 * @file max7221_7seg.c
 * Driver for the MAX7221 7 segment Serially Interfaced, 8-Digit LED Display
 * Drivers.
 */

#include <avr/io.h>
#include <io.h>       // for IO_SET_LOW, IO_SET_HIGH
#include <spi.h>      // for spi_tranceive, SPI_PORT, SS_PIN, etc
#include <stdbool.h>  // for false, bool, true
#include <stddef.h>   // for size_t
#include <stdint.h>   // for int8_t, uint8_t
#include <string.h>   // for strlen

#include "max7221_7seg.h"

#define IS_IN_RANGE(val, from, to) (val >= from && val <= to)

enum register_map {
	REG_NO_OP 			= 0x00, // Used only for daisy chaining MAX7221
	REG_DIGIT_0 		= 0x01,
	REG_DIGIT_1 		= 0x02,
	REG_DIGIT_2 		= 0x03,
	REG_DIGIT_3 		= 0x04,
	REG_DIGIT_4 		= 0x05,
	REG_DIGIT_5 		= 0x06,
	REG_DIGIT_6 		= 0x07,
	REG_DIGIT_7 		= 0x08,
	REG_DECODE_MODE 	= 0x09,
	REG_INTENSITY 		= 0x0A,
	REG_SCAN_LIMIT 		= 0x0B,
	REG_SHUTDOWN 		= 0x0C,
	REG_DISPLAY_TEST 	= 0x0F,
};

/**
 * CBD = Code B Decode
 * ND = No Decode
 *
 * If CBD bit is high for a given segment table 5 in the datasheet can be used
 * to display predefined font
 */
enum decode_modes {
	NO_DECODE_7_TO_0 		= 0x00,
	CBD_0_ND_7_TO_1 		= 0x01,
	CBD_3_TO_0_ND_7_TO_4 	= 0x0F,
	CBD_7_TO_0 				= 0xFF,
};


enum scan_limit {
	DISP_DIGIT_0 		= 0x00,
	DISP_DIGIT_0_TO_1 	= 0x01,
	DISP_DIGIT_0_TO_2 	= 0x02,
	DISP_DIGIT_0_TO_3 	= 0x03,
	DISP_DIGIT_0_TO_4 	= 0x04,
	DISP_DIGIT_0_TO_5 	= 0x05,
	DISP_DIGIT_0_TO_6 	= 0x06,
	DISP_DIGIT_0_TO_7 	= 0x07,
};

/**
 * Intensity can be anythin with in the intensity range
 */
enum intensity_range {
	MIN_INTENSITY = 0x00,
	MAX_INTENSITY = 0x0F,
};

enum display_test_mode {
	TEST_OFF 	= 0x00,
	TEST_ON 	= 0x01,
};

enum shutdown_mode {
	SHUTDOWN_OFF 	= 0x01,
	SHUTDOWN_ON 	= 0x00,
};

enum segment_lines {
	SEG_DP 	= 1<<7,
	SEG_A 	= 1<<6,
	SEG_B 	= 1<<5,
	SEG_C 	= 1<<4,
	SEG_D	= 1<<3,
	SEG_E 	= 1<<2,
	SEG_F 	= 1<<1,
	SEG_G 	= 1<<0,
};

static void write_reg(uint8_t reg, uint8_t data) {
	IO_SET_LOW(SPI_PORT, SS_PIN); 	// Mark beginning
	spi_tranceive(reg);
	spi_tranceive(data);
	IO_SET_HIGH(SPI_PORT, SS_PIN); 	// Latch data in
	IO_SET_LOW(SPI_PORT, SS_PIN); 	// Mark end
}


void seg7_init(void) {
	spi_init_master(false, SPI_PRESCALER_16); // No interrupts
	write_reg(REG_SCAN_LIMIT, 	DISP_DIGIT_0_TO_7);
	write_reg(REG_DECODE_MODE, 	NO_DECODE_7_TO_0);
	write_reg(REG_SHUTDOWN, 	SHUTDOWN_OFF);
	write_reg(REG_DISPLAY_TEST, TEST_OFF);

	seg7_clear_disp();

	write_reg(REG_INTENSITY, MAX_INTENSITY);
}

void seg7_clear_disp(void) {
	for (int digit = REG_DIGIT_0; digit < REG_DIGIT_7+1; ++digit) {
		write_reg(digit, 0x00);
	}
}

void seg7_disp_char(int8_t digit, const char c, const bool decimal_point) {
	uint8_t patteren; // segment patteren

	digit += REG_DIGIT_0; // Convert digit to correct register
	if (!IS_IN_RANGE(digit, REG_DIGIT_0, REG_DIGIT_7)) return; // out of range

	switch (c) {
	case '0': patteren = (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F); 		break;
	case '1': patteren = (SEG_B|SEG_C); 								break;
	case '2': patteren = (SEG_A|SEG_B|SEG_D|SEG_E|SEG_G); 				break;
	case '3': patteren = (SEG_A|SEG_B|SEG_C|SEG_D|SEG_G); 				break;
	case '4': patteren = (SEG_B|SEG_C|SEG_F|SEG_G); 					break;
	case '5': patteren = (SEG_A|SEG_C|SEG_D|SEG_F|SEG_G); 				break;
	case '6': patteren = (SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G); 		break;
	case '7': patteren = (SEG_A|SEG_B|SEG_C); 							break;
	case '8': patteren = (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G); 	break;
	case '9': patteren = (SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G); 		break;

	case 'a':
	case 'A': patteren = (SEG_A|SEG_B|SEG_C|SEG_E|SEG_F|SEG_G); 		break;

	case 'b':
	case 'B': patteren = (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G); 	break;

	case 'c':
	case 'C': patteren = (SEG_A|SEG_D|SEG_E|SEG_F); 					break;

	case 'D':
	case 'd': patteren = (SEG_B|SEG_C|SEG_D|SEG_E|SEG_G); 				break;

	case 'e':
	case 'E': patteren = (SEG_A|SEG_D|SEG_E|SEG_F|SEG_G);				break;

	case 'f':
	case 'F': patteren = (SEG_A|SEG_E|SEG_F|SEG_G); 					break;

	case 'g':
	case 'G': patteren = (SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G); 		break;

	case 'h':
	case 'H': patteren = (SEG_B|SEG_C|SEG_E|SEG_F|SEG_G); 				break;

	case 'i':
	case 'I': patteren = (SEG_E|SEG_F); 								break;

	case 'j':
	case 'J': patteren = (SEG_B|SEG_C|SEG_D); 							break;

	case 'k':
	case 'K': patteren = 0x00; /* TODO */ 								break;

	case 'l':
	case 'L': patteren = (SEG_D|SEG_E|SEG_F); 							break;

	case 'm':
	case 'M': patteren = 0x00; /* TODO */ 								break;

	case 'N':
	case 'n': patteren = (SEG_A|SEG_B|SEG_C|SEG_E|SEG_F); 				break;

	case 'o':
	case 'O': patteren = (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F); 		break;

	case 'p':
	case 'P': patteren = (SEG_A|SEG_B|SEG_E|SEG_F|SEG_G); 				break;

	case 'Q':
	case 'q': patteren = (SEG_A|SEG_B|SEG_C|SEG_F|SEG_G); 				break;

	case 'R':
	case 'r': patteren = (SEG_A|SEG_E|SEG_F); 							break;

	case 's':
	case 'S': patteren = (SEG_A|SEG_F|SEG_G|SEG_C|SEG_D); 				break;

	case 't':
	case 'T': patteren = (SEG_F|SEG_G|SEG_E|SEG_D); 					break;

	case 'u':
	case 'U': patteren = (SEG_B|SEG_C|SEG_D|SEG_E|SEG_F); 				break;

	case 'v':
	case 'V': patteren = 0x00; /* TODO */ 								break;

	case 'w':
	case 'W': patteren = 0x00; /* TODO */ 								break;

	case 'x':
	case 'X': patteren = 0x00; /* TODO */ 								break;

	case 'Y':
	case 'y': patteren = (SEG_F|SEG_B|SEG_G|SEG_C); 					break;

	case 'z':
	case 'Z': patteren = (SEG_A|SEG_B|SEG_G|SEG_E|SEG_D); 				break;

	case '-': patteren = (SEG_G); 										break;

	case '\0':
	case ' ':
	default: patteren = 0x00;											break;
	}

	write_reg(digit, ((decimal_point) ? (patteren | SEG_DP) : patteren));
}

void seg7_disp_str(char *str, int8_t start, int8_t end) {
	size_t len = strlen(str);
#if 1
	while (end >= start) {
		if (len) {
			if (str[--len] == '.') continue;
			seg7_disp_char(end--, str[len], ((str[len+1] == '.') ?
				true : false));
		} else {
			seg7_disp_char(end--, '0', false);
		}
	}
#else
	while (end >= start) {
		seg7_disp_char(end--, (len ? str[--len] : '0'), true);
	}
#endif
}
