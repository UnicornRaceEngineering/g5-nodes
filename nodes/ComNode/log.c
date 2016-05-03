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

#include <avr/pgmspace.h>
#include <fatfs/ff.h>  // for f_mount, f_open, f_sync, f_write, ::FR_EXIST, etc
#include <stddef.h>    // for size_t
#include <stdint.h>    // for uint8_t
#include <string.h>    // for memcpy, memset
#include <util/delay.h>
#include <stdio.h>
#include <utils.h>
#include <stdbool.h>

#include "log.h"

#define FMT_LOG_NAME PSTR("LOG%u.DAT")


static void flag_do_nothing(enum log_flags flag);

static FATFS fs;
static void (*flag)(enum log_flags) = flag_do_nothing;


static void flag_do_nothing(enum log_flags flag) {
	(void)flag;
}


void log_set_flag_callback(void(*func)(enum log_flags)) {
	flag = func;
}


void log_init(void) {
	_delay_ms(1000); /* Wait for SD card to be ready */
	if (f_mount(&fs, "", 1) != FR_OK) {
		flag(MOUNT_ERR);
	}
}


void create_file(FIL *f) {
	// increment filename until we have a new file that does not already exists.
	char file_name[32] = {'\0'};
	unsigned i = 0;
	do {
		sprintf_P(file_name, FMT_LOG_NAME, i++);
		if (i > 1000) {
			flag(CREATE_FILE_ERR);
			return;
		}
	} while (f_open(f, file_name, FA_CREATE_NEW|FA_WRITE) != FR_OK); //== FR_EXIST);
}


uint32_t size_of_file(FIL *f) {
	return f_size(f);
}


bool open_file(FIL *f, uint16_t lognr, uint8_t mode) {
	char file_name[16] = {'\0'};
	sprintf_P(file_name, FMT_LOG_NAME, lognr);
	if (f_open(f, file_name, mode) != FR_OK) {
		flag(OPEN_FILE_ERR);
		return false;
	}

	return true;
}


bool read_file(FIL *f, uint8_t *buf, size_t len) {
	unsigned int bw;
	const FRESULT rc = f_read(f, buf, len, &bw);
	if ((len != bw) || (rc != FR_OK)) {
		flag(READ_FILE_ERR);
		return false;
	}

	return true;
}


bool file_seek(FIL *f, size_t offset) {
	if (f_lseek(f, offset) != FR_OK) {
		flag(ERR_SEEKING);
		return false;
	}

	return true;
}


bool file_write(FIL *f, uint8_t *buf, size_t len) {
	unsigned int bw;
	const FRESULT rc = f_write(f, buf, len, &bw);
	if((rc != FR_OK) || (bw != len)) {
		flag(WRITE_FILE_ERR);
		return false;
	}

	return true;
}


unsigned int log_get_num_logs(void) {
	char file_name[32] = {'\0'};

	unsigned int i = 0;
	while (1) {
		sprintf_P(file_name, FMT_LOG_NAME, i++);

		FILINFO info;
		if (f_stat(file_name, &info) == FR_NO_FILE) break;
	}
	return i-1; // -1 because we incremented i before we checked the string
}
