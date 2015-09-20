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
#include <stdio.h>
#include <util/delay.h>

#include "event_manager.h"
#include "sysclock.h"


static uint8_t e = 0;
static uint16_t load_intv = 1000; // default millisec.


extern uint8_t set_load_intv(uint16_t time_intv) {
	// The time between calculating load cannot be less than 1 millisec
	// and in some situations stops making sence if less than 100 times the idle
	// wait time (WAIT_US / 10). 
	if (time_intv < 1 || time_intv < (WAIT_US / 10)) {
		return 1;
	}

	load_intv = time_intv;
	return 0;
}


extern void set_event(uint8_t event) {
	e = event;
}


extern uint8_t get_event(void) {
	return e;
}


extern uint8_t event_manager(uint8_t *event, uint32_t tick) {
	static uint8_t load = 0;
	static uint32_t tock = 0;
	static uint32_t load_timer = 0;

	if (e) {
		*event = e;
		e = 0;
	} else {
		++tock;
		_delay_us(WAIT_US);
	}

	if (tick > load_timer) {
		load = 100 - (((tock * WAIT_US) / (double)((uint32_t)1000 * load_intv)) * 100);
		load_timer = tick + load_intv;
		tock = 0;
	}

	return load;
}
