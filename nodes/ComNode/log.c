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

#include "log.h"

static FATFS fs;
static FIL file;

#define FMT_LOG_NAME PSTR("log%u.dat")

#define BUF_SIZE	512

static struct payload {
	uint8_t buf[BUF_SIZE];
	size_t i;
} p;

void log_init(void) {
	memset(&p, 0, sizeof(p));

	if (f_mount(&fs, "", 1) != FR_OK) {
		// Error
	}

	// increment filename until we have a new file that does not already exists.
	char file_name[32] = {'\0'};
	unsigned i = 0;
	do {
		sprintf_P(file_name, FMT_LOG_NAME, i++);
		if (i > 1000) break;
	} while (f_open(&file, file_name, FA_CREATE_NEW|FA_WRITE) != FR_OK); //== FR_EXIST);
}

static int flush_to_sd(void) {
	int err = 0;
	unsigned bw;
	if(f_write(&file, p.buf, p.i, &bw) != FR_OK) err = -1;
	p.i = 0;
	err = (bw == p.i) ? 0 : -1;
	return err;
}

int log_append(void *data, size_t n) {
	int err = 0;
	if (p.i + n > BUF_SIZE) {
		const size_t reminder = n - ((p.i + n) - BUF_SIZE);
		memcpy(&p.buf[p.i], data, reminder);
		p.i += reminder;
		n -= reminder;
		data += reminder;
		log_sync();
	}

	memcpy(&p.buf[p.i], data, n);
	p.i += n;
	return err;
}

void log_sync(void) {
	f_sync(&file);
	flush_to_sd();
}

int log_read(uint16_t lognr, FILE* fd) {
	// TODO use f_forward instead? http://elm-chan.org/fsw/ff/en/forward.html
	if (fd == NULL) return -1;

	char file_name[16] = {'\0'};
	sprintf_P(file_name, FMT_LOG_NAME, lognr);

	log_sync();

	FIL f;
	if (f_open(&f, file_name, FA_READ|FA_OPEN_EXISTING) != FR_OK) goto err;

	const uint32_t fsize = f_size(&f);
	for (size_t i = 0; i < sizeof(fsize); i++) {
		fputc(((uint8_t*)&fsize)[i], fd);
	}

	// seek to the start of file
	if (f_lseek(&f, 0) != FR_OK) goto err;

	uint32_t bytes_left = fsize;
	while (bytes_left != 0) {
		uint8_t buf[32];

		const unsigned btr = (ARR_LEN(buf) < bytes_left) ? ARR_LEN(buf) : bytes_left;
		unsigned br;
		if (f_read(&f, buf, btr, &br) != FR_OK) goto err;
		bytes_left -= br;

		for (size_t i = 0; i < br; i++) {
			fputc(buf[i], fd);
		}
	}

	log_init();
	return 0;

err:
	log_init();
	return -1;
}

unsigned log_get_num_logs(void) {
	char file_name[32] = {'\0'};

	unsigned i = 0;
	while (1) {
		sprintf_P(file_name, FMT_LOG_NAME, i++);

		FILINFO info;
		if (f_stat(file_name, &info) == FR_NO_FILE) break;
	}
	return i-1; // -1 because we incremented i before we checked the string
}
