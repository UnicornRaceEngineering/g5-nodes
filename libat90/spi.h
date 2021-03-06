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
 * @files spi.h
 * Low level interface for SPI
 */

#ifndef SPI_H
#define SPI_H

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>

#define SPI_PORT	PORTB
#define MISO_PIN	PIN3
#define MOSI_PIN	PIN2
#define SCK_PIN		PIN1
#define SS_PIN		PIN0

enum spi_prescaler {
	SPI_PRESCALER_4 	= (0<<SPR1)|(0<<SPR0),
	SPI_PRESCALER_16 	= (0<<SPR1)|(1<<SPR0),
	SPI_PRESCALER_64 	= (1<<SPR1)|(0<<SPR0),
	SPI_PRESCALER_128 	= (1<<SPR1)|(1<<SPR0),
};

void spi_init_master(const bool enable_interrupts, const enum spi_prescaler);
uint8_t spi_tranceive(const uint8_t data);
void spi_transmit_buf(uint8_t *buf, size_t len);


#endif /* SPI_H */
