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

/**
 * @files paddleshift.h
 * Implements a basic interface to paddleshift controls connected to board
 */

#ifndef PADDLESHIFT_H
#define PADDLESHIFT_H

#include <avr/io.h>
#include <stdbool.h>

/**
 * @name Pin layout
 * @{
 */
#define PADDLE_UP_PORT			PORTE
#define PADDLE_UP_PIN			PIN7
#define PADDLE_UP_INT			INT7
#define PADDLE_UP_ISR_VECT		INT7_vect
#define PADDLE_UP_ISC1			ISC71
#define PADDLE_UP_ISC0			ISC70

#define PADDLE_DOWN_PORT		PORTE
#define PADDLE_DOWN_PIN			PIN6
#define PADDLE_DOWN_INT			INT6
#define PADDLE_DOWN_ISR_VECT	INT6_vect
#define PADDLE_DOWN_ISC1		ISC61
#define PADDLE_DOWN_ISC0		ISC60
/** @} */

/**
 * @name Function prototypes
 * @{
 */
void paddle_init(void);
bool paddle_up_status(void);
bool paddle_down_status(void);
/** @} */

#endif /* PADDLESHIFT_H */
