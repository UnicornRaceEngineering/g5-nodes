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
// #include <spi.h>
// #include <mmc_sdcard.h>
#include <stdio.h>

#include <pff.h>
#include <diskio.h>

#define PUT_RC(func) usart1_printf("rc=%d", func)

int main(void) {
	xbee_init();
	ecu_init();

	sei();										//Enable interrupt

	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\r');

#if 0
	if (sd_init() != 0) {
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
#endif

	FATFS fs; 		// File system object
	// DIR dir;		// Directory object
	// FILINFO finfo;	// File infomation

	PUT_RC(usart1_printf("rc=%d", disk_initialize()));
	PUT_RC(pf_mount(&fs));
	PUT_RC(pf_open("testfile.txt"));

	unsigned int bytes_read = 0;
	uint8_t buff[512] = {'\0'};

	do {
		uint8_t res = pf_read(&buff, 512-1, &bytes_read);
		if (res != FR_OK) break;
	} while (bytes_read == 512-1);
	buff[512-1] = '\0';

	usart1_printf("%s", buff);

	while(1){
		// Main work loop

		ecu_parse_package();
	}

    return 0;
}
