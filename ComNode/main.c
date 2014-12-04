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
#include "bson.h"

#include <usart.h>
#include <string.h>
#include <spi.h>
#include <mmc_sdcard.h>
#include <stdio.h>

#include <pff.h>
#include <diskio.h>

#define ARR_LEN(x)  (sizeof(x) / sizeof(x[0]))

#define PUT_RC(func) usart1_printf("rc=%d at line %d\n", func, __LINE__)

static FRESULT scan_files (char* path) {
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;


    res = pf_opendir(&dir, path);
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = pf_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fattrib & AM_DIR) {
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);
                if (res != FR_OK) break;
                path[i] = 0;
            } else {
                usart1_printf("%s/%s\n", path, fno.fname);
            }
        }
    }

    return res;
}
#if 0

static void test_fs(void) {
	FATFS fs; 		// File system object

	// PUT_RC(disk_initialize());
	PUT_RC(pf_mount(&fs));

	PUT_RC(scan_files("/LOGS"));
	PUT_RC(pf_open("TESTFILE.TXT"));

	// Read what is current in testfile.txt
	{
		unsigned bytes_read = 0;
		uint8_t read_buffer[128] = {'\0'};
		PUT_RC(pf_read(&read_buffer, 128-1, &bytes_read));

		usart1_printf("read %u bytes:\n", bytes_read);
		usart1_printf("%s\n", read_buffer);
	}

	// Write something new in the file
	{
		PUT_RC(pf_lseek(0));

		unsigned bytes_written = 0;
		char wbuff[] = "123456789";

		PUT_RC(pf_write(wbuff, strlen(wbuff), &bytes_written));
		usart1_printf("bytes written=%d\n", bytes_written);
		PUT_RC(pf_write(0, 0, &bytes_written));
		usart1_printf("bytes written=%d\n", bytes_written);
	}
		PUT_RC(pf_lseek(0));

	// Read the new data written
	{
		unsigned bytes_read = 0;
		uint8_t read_buffer[128] = {'\0'};
		PUT_RC(pf_read(&read_buffer, 128-1, &bytes_read));

		usart1_printf("read %u bytes:\n", bytes_read);
		usart1_printf("%s\n", read_buffer);
	}
}
#endif

int seek_to_log_end(FATFS *fs) {
	int32_t document_size = 0;
	uint8_t buff[sizeof(document_size)];
	bool end_found = false;

	while (!end_found) {
		unsigned bytes_read;
		if (pf_read(buff, sizeof(document_size), &bytes_read) != FR_OK) {
			//!< @TODO Handle fs error
			usart1_printf("Read error\n");
		}

		// Read document size
		for (size_t i = 0; i < sizeof(document_size); ++i) {
			((uint8_t*)&document_size)[i] = buff[i];
		}
usart1_printf("doc_size=%d\n", document_size);
		if (document_size == 0) end_found = true; // end found

		pf_lseek((fs->fptr - sizeof(document_size)) + document_size );
	}

usart1_printf("fptr=%u\n", fs->fptr);
	unsigned int log_end_pos = fs->fptr; //(fs->fptr > sizeof(document_size)) ?
		//fs->fptr - sizeof(document_size) : 0;
	pf_lseek(log_end_pos);
usart1_printf("End of log is at %u\n", log_end_pos);
	return log_end_pos;
}

int main(void) {
	xbee_init();
	ecu_init();

	sei();										//Enable interrupt

	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\r');

	usart1_printf("Starting\n\n\n");

	FATFS fs;
	{
		if (pf_mount(&fs) != FR_OK) {
			//!< @TODO Handle fs error
			usart1_printf("Error mounting fs\n");
		}

		char scan_dir[64] = "/LOGS";
		scan_files(scan_dir);

		const char logfile[] = "LOG";
		if (pf_open(logfile) != FR_OK) {
			//!< @TODO Handle fs error
			usart1_printf("Error opening file \"%s\"\n", logfile);
		}
		// seek_to_log_end(&fs);
	}
	// _delay_ms(1000*10);
	while(1){
		// Main work loop
		ecu_parse_package();
	}

    return 0;
}
