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


#ifndef WATCHDOG_H
#define WATCHDOG_H


#include <stdint.h>


/*
 * The watchdog_init function takes a single integer input (0-7) and sets the
 * timeout accordingly. It then enables the watchdog timer.
 * Asuming Vcc = 5.0V integer inputs between 0 and 7 will give the following
 * timeouts:
 *
 * 0 - 16.3 ms
 * 1 - 32.5 ms
 * 2 - 65 ms
 * 3 - 0.13 s
 * 4 - 0.26 s
 * 5 - 0.52 s
 * 6 - 1.0 s
 * 7 - 2.1 s
 */
void watchdog_init(uint8_t timeout);
void disable_watchdog(void);
void kick_watchdog(void);


#endif /* WATCHDOG_H */
