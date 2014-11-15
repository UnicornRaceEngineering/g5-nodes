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
 * @file mmc_sdcard.c
 * Low level driver for reading and writing SD cards using SPI. Information on
 * the specification can be found at:
 * https://www.sdcard.org/downloads/pls/simplified_specs/
 * In the "Physical Layer Simplified Specification" document.
 */

#ifndef MMC_SDCARD_H
#define MMC_SDCARD_H

#include <stdint.h>
#include <stddef.h>

#define SD_BLOCKSIZE 512

int8_t sd_init(void);
int8_t sd_read_block(uint8_t *buff, uint32_t sector, int16_t offset,
					 int16_t n);
int8_t sd_write_block(uint8_t *data, uint32_t sector, size_t n);
#endif /* MMC_SDCARD_H */
