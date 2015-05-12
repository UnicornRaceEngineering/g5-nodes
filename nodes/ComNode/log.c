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

#include <stdint.h>
#include <fatfs/ff.h>
#include <stddef.h>
#include <stdio.h>

#include "log.h"

static FATFS fs;
static FIL file;

void log_init(void) {
	f_mount(&fs, "", 1);

	// increment filename until we have a new file that does not already exists.
	char file_name[32] = {'\0'};
	unsigned i = 0;
	do {
		sprintf(file_name, "log%u.dat", i++);
	} while (f_open(&file, file_name, FA_CREATE_NEW|FA_WRITE) == FR_EXIST);
}

int log_append(void *buf, size_t n) {
	unsigned bw;
	if(f_write(&file, buf, n, &bw) != FR_OK) return -1;
	return (bw == n) ? 0 : -1;
}

void log_sync(void) {
	f_sync(&file);
}

