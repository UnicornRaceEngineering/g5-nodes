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


#include <avr/io.h>
#include <util/atomic.h>
#include <stdint.h>

#include "watchdog.h"
#include "utils.h"


static inline void enable_watchdog(void);

static uint8_t watchdog_timeout = 0xF;


void watchdog_init(uint8_t timeout) {
	watchdog_timeout = timeout & 0xF;
	enable_watchdog();
}


void kick_watchdog(void) {
	enable_watchdog();
}


void disable_watchdog(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		WDTCR = (1 << WDE) | (1 << WDCE);
		BIT_CLEAR(WDTCR, WDE);
	}
}


static inline void enable_watchdog(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		WDTCR = (1 << WDE) | (1 << WDCE);
		WDTCR = (1 << WDE) | watchdog_timeout;
	}
}
