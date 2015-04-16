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

#include "dewalt.h"
#include "vnh2sp30.h"

#include <stdint.h>

void dewalt_init(void) {
	vnh2sp30_init();
}

void dewalt_set_direction_A(void) {
	vnh2sp30_clear_INB();
	vnh2sp30_set_INA();
}

void dewalt_set_direction_B(void) {
	vnh2sp30_clear_INA();
	vnh2sp30_set_INB();
}

void dewalt_set_pwm_dutycycle(uint8_t dutycycle) {
	vnh2sp30_set_PWM_dutycycle(dutycycle);
}

void dewalt_kill(void) {
	vnh2sp30_active_break_to_GND();
}
