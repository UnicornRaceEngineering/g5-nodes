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

#if 1
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

static void verify_bson(int32_t encode_len, uint8_t *buff, int32_t ref_len, uint8_t *ref) {
	if (ref_len != encode_len)
		usart1_printf("Encoded string not same length as reference string\n");

	usart1_printf("serialize len=%u\n", encode_len);
	for (int i = 0; i < encode_len; ++i) {
		usart1_printf("%#2x ", buff[i]);
		if (buff[i] != ref[i]) {
			usart1_printf("Error expected %#2x\n", ref[i]);
			break;
		}
	}
	usart1_printf("\n");
}

int main(void) {
	xbee_init();
	ecu_init();

	sei();										//Enable interrupt

	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\n');
	usart1_putc_unbuffered('\r');

	test_fs();


	{
		uint8_t buff[128];

		// Test String serializing
		usart1_printf("\n\nTest String serializing\n");
		{
			struct bson_element e = {.e_id = ID_STRING, .key = "str", .str = "This is a string"};
			int32_t encode_len = serialize_element(buff, &e, 128);
			uint8_t ref[] = {0x2,0x73,0x74,0x72,0,0x11,0,0,0,0x54,0x68,0x69,0x73,0x20,0x69,0x73,0x20,0x61,0x20,0x73,0x74,0x72,0x69,0x6e,0x67,0};
			int32_t ref_len = ARR_LEN(ref);
			verify_bson(encode_len, buff, ref_len, ref);
		}

		// Test int32 serializing
		usart1_printf("\n\nTest int32 serializing\n");
		{
			struct bson_element e = {.e_id = ID_32_INTEGER, .key = "integer", .int32 = 42};
			int32_t encode_len = serialize_element(buff, &e, 128);
			uint8_t ref[] = {0x10, 0x69, 0x6e, 0x74, 0x65, 0x67, 0x65, 0x72, 0x00, 0x2a, 0x00, 0x00, 0x00};
			verify_bson(encode_len, buff, ARR_LEN(ref), ref);
		}

		// Test binary serializing
		usart1_printf("\n\nTest binary serializing\n");
		{
			uint8_t bin[] = {1,2,3,4};
			struct bson_element e = {.e_id = ID_BINARY, .binary.subtype=SUB_GENERIC, .key="buff", .binary.data=bin, .binary.len=ARR_LEN(bin)};
			int32_t encode_len = serialize_element(buff, &e, 128);
			uint8_t ref[] = {0x05, 0x62, 0x75, 0x66, 0x66, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,};
			verify_bson(encode_len, buff, ARR_LEN(ref), ref);
		}

		usart1_printf("\n\nDone testing BSON\n");
	}

	while(1){
		// Main work loop

		ecu_parse_package();
	}

    return 0;
}
