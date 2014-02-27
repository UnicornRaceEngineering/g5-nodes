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
* @file io.h
* @brief
*	Used for simple digital pin IO operations
*/

#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <bitwise.h>

#define DDR_PORT(port)	(*(&(port) - 0x01)) //!< Maps a given PORT to the corresponding DDR eg. PORTA -> DDRA. The magic number 0x01 is the offset from the PORT register to the DDR register
#define PIN_PORT(port)	(*(&(port) - 0x02)) //!< Maps a given PORT to the corresponding PIN eg. PORTA -> PINA. The magic number 0x02 is the offset from the PORT register to the PIN register


/**
* @brief
*	Represents the two possible
*	digital values
*/
enum io_digital_t {
	LOW 	= 0,
	HIGH 	= 1
};

/**
* @brief
*	Different modes that the pins
*	can be set to
*/
enum io_pinmode_t {
	INPUT, 			//!< Useful for reading a sensor but not powering an LED, You would most likely want the INPUT pin to reference GND
	OUTPUT, 		//!< Sets the pin in a low impedance state. Thus the pin can power a LED but not read a sensor
	INPUT_PULLUP 	//!< Use the internal pull-up resistors. This effectively inverts its behavior, where HIGH means off and LOW means on
};

//!< @name digital read/write
//!< Digitally read, write or toggle a specified IO pin
//!< @{
#define DIGITAL_READ(port, pin)			( BIT_CHECK((PIN_PORT(port)), (pin)) != 0 ? HIGH : LOW 			) //!< Same as digitalRead
#define DIGITAL_WRITE(port, pin, value)	( BITMASK_SET_OR_CLEAR((port), (1 << (pin)), (value)) 			) //!< Same as digitalWrite. Use only if value is run time dependent
#define DIGITAL_TOGGLE(port, pin)		( DIGITAL_WRITE((port), (pin), !DIGITAL_READ((port), (pin))) 	) //!< Toggles a pin
//!< @}

//!< @name IO_SET
//!< These should be used over DIGITAL_WRITE() if
//!< the state you want to set is known at compile time
//!< @{
#define IO_SET_HIGH(port, pin)	( BIT_SET((port), (pin)) 	) //!< Set the value on the specified pin to HIGH
#define IO_SET_LOW(port, pin)	( BIT_CLEAR((port), (pin)) 	) //!< Set the value on the specified pin to LOW
//!< @}

/**
* @brief
*	Same as pinMode
*/
#define SET_PIN_MODE(port, pin, mode){ \
	if (mode == OUTPUT){ \
		BIT_SET( DDR_PORT( (port) ), (pin) ); \
	} else if ( ((mode) == INPUT) || ((mode) == INPUT_PULLUP) ){ \
		BIT_CLEAR( DDR_PORT( (port) ), (pin) ); \
		if ((mode) == INPUT_PULLUP) BIT_SET( (port), (pin) ); \
	} \
}

enum io_digital_t digitalRead(volatile uint8_t *inputPinRegister, int pin);
void digitalWrite(volatile uint8_t *port, int pin, enum io_digital_t value);

int pinMode(volatile uint8_t *port, int pin, enum io_pinmode_t mode);


#endif /* IO_H */
