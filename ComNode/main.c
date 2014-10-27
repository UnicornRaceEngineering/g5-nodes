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

#include <avr/interrupt.h> // sei()
#include <util/delay.h>
#include <stdbool.h>

#include "ecu.h"
#include "xbee.h"

#include <usart.h>
#include <spi.h>
#include <mmc_sdcard.h>
#include <stdio.h>

int main(void) {
	xbee_init();
	ecu_init();

	spi_init_master(false);

	sei();										//Enable interrupt

	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\r');


	if (sd_spi_mode_initialization() != 0) {
		usart1_printf("SD Connection error\n");
	} else {
		usart1_printf("SD connected\n");
		char tx[SD_BLOCKSIZE];
		uint8_t buff[SD_BLOCKSIZE];

		bool found_char = false;
		for (int block = 0; block < 50; ++block) {
			const int tx_len = snprintf(tx, SD_BLOCKSIZE, "This is a test string on the SD card written to block address %#x", block);
			if (sd_write_block((uint8_t*)tx, block, tx_len) != 0) {
				usart1_printf("Write error\n");
				continue;
			}
			if (sd_read_block(buff, block, 0, SD_BLOCKSIZE) == 0) {
				for (int i = 0; i < SD_BLOCKSIZE; ++i) {
					if (buff[i] >= ' ' && buff[i] <= 'z') {
						usart1_putc(buff[i]);
						found_char = true;
					}
				}
			if (found_char) usart1_printf("\n---\n");
			found_char = false;
			} else {
				usart1_printf("Read error\n");
			}
		}
	}

	while(1){
		// Main work loop

		ecu_parse_package();
	}

    return 0;
}
