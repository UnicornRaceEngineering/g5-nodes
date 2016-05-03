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

#include "flags.h"
#include "xbee.h"
#include "log.h"
#include "state_machine.h"


static uint32_t all_the_flags;

static void xbee_callback(enum xbee_flags flag);
static void log_callback(enum log_flags flag);
static void state_callback(enum state_flags flag);


void flags_init(void) {
	reset_all_flags();
	xbee_set_flag_callback(xbee_callback);
	log_set_flag_callback(log_callback);
	state_set_flag_callback(state_callback);
}


void reset_all_flags(void) {
	all_the_flags = 0;
}


uint32_t get_all_flags(void) {
	return all_the_flags;
}


static void xbee_callback(enum xbee_flags flag) {
	all_the_flags |= 1 << flag;
}


static void log_callback(enum log_flags flag) {
	all_the_flags |= 1 << (flag + N_XBEE_FLAGS);
}


static void state_callback(enum state_flags flag) {
	all_the_flags |= 1 << (flag + N_XBEE_FLAGS + N_LOG_FLAGS);
}
