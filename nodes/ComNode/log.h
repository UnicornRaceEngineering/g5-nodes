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

#ifndef LOG_H
#define LOG_H

#include <stddef.h>
#include <stdint.h>
#include <fatfs/ff.h>


enum log_flags {
	MOUNT_ERR,
	CREATE_FILE_ERR,
	WRITE_FILE_ERR,
	OPEN_FILE_ERR,
	BUFFER_TOO_LARGE,
	READ_FILE_ERR,
	ERR_SEEKING,

	N_LOG_FLAGS,
};


void log_init(void);
uint32_t size_of_file(FIL *file);
void create_file(FIL *file);
bool open_file(FIL *f, uint16_t lognr, uint8_t mode);
bool read_file(FIL *f, uint8_t *buf, size_t len);
bool file_seek(FIL *f, size_t offset);
bool file_write(FIL *f, uint8_t *buf, size_t len);
void log_set_flag_callback(void(*func)(enum log_flags));
unsigned log_get_num_logs(void);

#endif /* LOG_H */
