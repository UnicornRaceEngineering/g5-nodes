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
 * @files twi.c
 * @brief
 * Provides a simple interface to communicating over the TWI (I2C) interface.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <util/twi.h> // avr libc twi bit mask definitions

#include "usart.h"
#include "bitwise.h"
#include "io.h"
#include "twi.h"

#define ARR_LEN(arr)	(sizeof(arr) / sizeof(arr[0]))

#define TWI_CMD_NOT_FINISHED	((TWCR & (1<<TWINT)) == 0)
#define RW_BIT					(1 << 0)

#define SCL_PORT	PORTD
#define SCL_PIN		PIN0
#define SDA_PORT	PORTD
#define SDA_PIN		PORT1

/**
 * Sets this node in master mode
 */
void twi_init_master(void) {
	// SCL_freq = (F_CPU / (16 + (2*TWBR) * 4^TWPS))

	SET_PIN_MODE(SCL_PORT, SCL_PIN, INPUT_PULLUP);
	SET_PIN_MODE(SDA_PORT, SDA_PIN, INPUT_PULLUP);
#if 1
	TWBR = ((((F_CPU / F_SCL) / PRESCALER) - 16 ) / 2);
#endif

#if PRESCALER == 1
	TWSR = (0<<TWPS1)|(0<<TWPS0);
#elif PRESCALER == 4
	TWSR = (0<<TWPS1)|(1<<TWPS0);
#elif PRESCALER == 16
	TWSR = (1<<TWPS1)|(0<<TWPS0);
#elif PRESCALER == 64
	TWSR = (1<<TWPS1)|(1<<TWPS0);
#else
#	error Invalid TWI prescaler value
#endif

	//TWBR = 14;

	//TWCR = (1 << TWEN);
}

int8_t twi_send_start_condition(void) {
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); // Send start condition
	while (TWI_CMD_NOT_FINISHED); // Wait for start cond to finish transmitting

	return TW_STATUS;
}

int8_t twi_send_stop_condition(void) {
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
#if 1
	/**
	 * @TODO For some reason the RTC (M41T81S) refuses to respond if not this
	 * delay is inserted after some commands. As a hack we have just put it
	 * here.
	 */
	int8_t status = TW_STATUS;
	_delay_us(5);
	return status;
#else
	return TW_STATUS;
#endif
}


/**
 * Writes a byte on the TWI bus
 * @param  data The byte that is transmitted
 * @return      0 on success (data transmitted, ACK received) else 1 for failure
 */
int8_t twi_write(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN); // Start transmission
	while (TWI_CMD_NOT_FINISHED);
	return TW_STATUS != TW_MT_DATA_ACK; // 1 = fail, 0 = success
}

/**
 * Reads a byte from the TWI bus with ACK
 * @return  The byte that was read
 */
int8_t twi_read(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA); // Read with ACK
	while (TWI_CMD_NOT_FINISHED);
	return TWDR;
}

/**
 * Reads a byte from the TWI bus with NACK
 * @return  The byte that was read
 */
int8_t twi_read_nack(void) {
	TWCR = (1<<TWINT)|(1<<TWEN); // Read with not ACK
	while (TWI_CMD_NOT_FINISHED);
	return TWDR;
}

/**
 * Set's the Read/Write bit on the address
 * @param  dev_addr The device address
 * @param  rw       'w' if you want to write to the address
 *                  'r' if you want to read from the address
 * @return          The address with the r/w bit correctly sat
 */
static inline uint8_t address_rw(uint8_t dev_addr, char rw) {
	if 		(rw == 'r') BITMASK_SET(dev_addr, RW_BIT);
	else if (rw == 'w') BITMASK_CLEAR(dev_addr, RW_BIT);
	return dev_addr;
}

/**
 * Initiates a write to the given device address.
 * twi_write() can only be called after this function has been successfully
 * called.
 * @param  dev_addr Target device address
 * @return          0 on success 1 on failure
 */
int16_t twi_start_write(uint8_t dev_addr) {
	int16_t rc;
	if ((rc = twi_send_start_condition()) != TW_START) return -rc;
	// We must shift the dev_addr 1 as it is a 8 bit value with the addr in the
	// upper 7 bytes and 1 Read/Write indicator bit in the Least Significant Bit.
	twi_write(address_rw(dev_addr, 'w')); // Start is special case so we don't use it's rc
	rc = TW_STATUS; // Instead we read directly from the status reg
	if ((rc != TW_MT_SLA_ACK) && (rc != TW_MR_SLA_ACK)) return -rc;


	return rc;
}

/**
 * Initiates a read from the given device address's internal register
 * twi_read() can only be called after this function has been successfully
 * called.
 * @param  dev_addr     Target device address
 * @param  internal_reg The internal register to read from on the target
 * @return              0 on success 1 on failure
 */
int16_t twi_start_read(uint8_t dev_addr, uint8_t internal_reg) {
	if (twi_start_write(dev_addr) < 0) return -TW_STATUS;
	if (twi_write(internal_reg) != 0) return -TW_STATUS;

	if (twi_send_start_condition() != TW_REP_START) return -TW_STATUS;
	// Set the R/W bit high
	twi_write(address_rw(dev_addr, 'r'));
	if (TW_STATUS != TW_MR_SLA_ACK) return -TW_STATUS;

	return TW_STATUS;
	//return TW_STATUS != TW_MR_SLA_ACK; // 1 failure, 0 success
}

/**
 * Writes a byte array to the target device
 * @param  dev_addr Target device address
 * @param  arr      Pointer to the byte array
 * @param  len      Length of the byte array
 * @return          0 on success 1 on failure
 */
int16_t twi_write_array(uint8_t dev_addr, uint8_t* arr, size_t len) {
	if (twi_start_write(dev_addr) < 0) return -TW_STATUS;

	while (len--)
		if (twi_write(*arr++) != 0) return -TW_STATUS;

	twi_send_stop_condition();
	return TW_STATUS;
}

int16_t twi_write_register(uint8_t dev_addr, uint8_t internal_reg,
	uint8_t value) {
	uint8_t tx[2] = {internal_reg, value};
	return twi_write_array(dev_addr, tx, ARR_LEN(tx));
}

/**
 * Reads n bytes from the target device's internal register and save it in an
 * array
 * @param  dev_addr     Target device address
 * @param  internal_reg The internal register to read from on the target
 * @param  arr          Pointer to the array where the read data is stored
 * @param  n            Number of bytes to read from the target
 * @return              0 on success 1 on failure
 */
int16_t twi_read_array(uint8_t dev_addr, uint8_t internal_reg, uint8_t* arr,
	size_t n) {
	if (twi_start_read(dev_addr, internal_reg) < 0) return -TW_STATUS;

	while (n--) {
		*arr++ = (n != 0) ? twi_read() : twi_read_nack();
		uint8_t status = TW_STATUS;
		if (status != TW_MR_DATA_ACK &&
		   (status != TW_MR_DATA_NACK && n == 0)) return -TW_STATUS;
	}

	twi_send_stop_condition();

	return TW_STATUS;
}

/**
 * Read and return the TWI status register. Read data sheet for list of status
 * codes.
 * @return  The TWI status code.
 */
int16_t twi_get_status(void) {
	return TW_STATUS;
}
