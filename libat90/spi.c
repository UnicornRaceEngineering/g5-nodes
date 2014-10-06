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
 * @files spi.c
 * Low level interface for SPI
 */

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include "io.h"
#include "spi.h"

void spi_init_master(const bool enable_interrupts) {
	SET_PIN_MODE(SPI_PORT, MOSI_PIN, OUTPUT);
	SET_PIN_MODE(SPI_PORT, SCK_PIN, OUTPUT);

	SET_PIN_MODE(SPI_PORT, SS_PIN, OUTPUT);

	// SPIE = SPI interrupt enable
	// SPE = SPI Enable
	// MSTR = Master mode
	SPCR |= (1<<SPE)|(1<<MSTR);

	if (enable_interrupts) {
		SPCR |= (1<<SPIE);
	}

	// Set prescaler to F_CPU/16
	SPCR |= (1<<SPR0);
}

uint8_t spi_tranceive(const uint8_t data) {
	// Load data into the buffer
	SPDR = data;

	// Wait for transmission to complete
	while (!((SPSR) & (1<<SPIF)));

	// As SPI is full duplex we also receive data
	return(SPDR);
}
