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
 * @file twi.h
 * @brief
 * Provides a simple interface to communicating over the TWI (I2C) interface.
 */

#ifndef TWI_H
#define TWI_H

#include <stdint.h>
#include <stdlib.h>

#define F_SCL 100000UL // SCL frequency
#define PRESCALER 1

void twi_init_master(void);
int8_t twi_send_start_condition(void);
int8_t twi_send_stop_condition(void);
int8_t twi_write(uint8_t data);
int8_t twi_read(void);
int8_t twi_read_nack(void);
int16_t twi_start_write(uint8_t dev_addr);
int16_t twi_start_read(uint8_t dev_addr, uint8_t internal_reg);
int16_t twi_write_array(uint8_t dev_addr, uint8_t* arr, size_t len);
int16_t twi_read_array(uint8_t dev_addr, uint8_t internal_reg, uint8_t* arr,
	size_t n);
int16_t twi_write_register(uint8_t dev_addr, uint8_t internal_reg,
	uint8_t value);
int16_t twi_get_status(void);

#endif /* TWI_H */
