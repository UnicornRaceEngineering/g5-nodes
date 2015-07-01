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

#include <stdint.h>
#include <utils.h>
#include <io.h>

#include "dipswitch.h"

void dip_init(void) {
	SET_PIN_MODE(DIP_PORT, PIN7, INPUT);
	SET_PIN_MODE(DIP_PORT, PIN6, INPUT);
	SET_PIN_MODE(DIP_PORT, PIN5, INPUT);
	SET_PIN_MODE(DIP_PORT, PIN4, INPUT);
	SET_PIN_MODE(DIP_PORT, PIN3, INPUT);
	SET_PIN_MODE(DIP_PORT, PIN2, INPUT);
	SET_PIN_MODE(DIP_PORT, PIN1, INPUT);
	SET_PIN_MODE(DIP_PORT, PIN0, INPUT);
}

uint8_t dip_read(void) {
	return PIN_PORT(DIP_PORT);
}
