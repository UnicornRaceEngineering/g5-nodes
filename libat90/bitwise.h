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
* @file bitwise.h
* @brief
*	Provides some abstraction for
*	some bitwise operations.
*	Useage should help code readability.
*/

#ifndef BITWISE_H
#define BITWISE_H

/** @name Bit operations
* Does operation on the specified bit
* where a=target, b=bit number to act on
* @{
*/
#define BIT_SET(a,b) 		( (a) |= (1<<(b))	)
#define BIT_CLEAR(a,b) 		( (a) &= ~(1<<(b))	)
#define BIT_FLIP(a,b) 		( (a) ^= (1<<(b))	)
#define BIT_CHECK(a,b) 		( (a) & (1<<(b))	)
/**@}*/

/** @name Bit mask operations
* Does operation on the specified target
* according to the mask where
* x=target, y=mask
* @{
*/
#define BITMASK_SET(x,y) 	( (x) |= (y)	)
#define BITMASK_CLEAR(x,y) 	( (x) &= (~(y))	)
#define BITMASK_FLIP(x,y) 	( (x) ^= (y)	)
#define BITMASK_CHECK(x,y) 	( (x) & (y)		)

/**
* @brief
*	Conditionally set or clear bit without branching
*
* @param[out] w
*	The word to modify:  if (f) w |= m; else w &= ~m;
* @param[in] m
*	The bit mask
* @param[in] f
*	Conditional flag
*/
#define BITMASK_SET_OR_CLEAR(w, m, f) ( (w) ^= (-(f) ^ (w)) & (m) )
/**@}*/

#define IS_POW2(x) 				( (x) && !((x) & ((x) - 1))	) //!< Check if x is a power of 2
#define HAS_OPPSITE_SIGN(x, y)	( (((x) ^ (y)) < 0) ) //!< check if x and y has opposite sign

/**
 * @name Byte splitting
 * Provides splitting and merging byte and word operations.
 * @{
 */
#define LOW_BYTE(w) 		( (uint8_t) ((w) & 0xFF)		) //!< Extracts the low-order (rightmost) byte of a variable.
#define HIGH_BYTE(w) 		( (uint8_t) ((w) >> 8)			) //!< Extracts the high-order (leftmost) byte of a word (or the second lowest byte of a larger data type).
#define MERGE_BYTE(h, l) 	( (uint16_t)(((h) << 8) | (l)) 	) //!< Merges two 8 bit bytes into one 16 bit byte where h is the High byte and l is the Low byte

#define LOW_NIBBLE(b)		( (b) & 0xF 		)
#define HIGH_NIBBLE(b)		( ((b) >> 4) & 0xF 	)
#define MERGE_NIBBLES(h, l) ( ((h) << 4) | (l) 	)
/** @} */

#define IS_ODD(n)	(((n) & 1) == 1)

/**
 * Sets the bitset on the given control register. This works by first
 * filtering out invalid bitset values according to the mask. The register is
 * cleared and then we can finally write the bitset value to the register.
 * @param  ctrl_register The timer control register
 * @param  bitset        The bitset that should be written to the register
 * @param  mask          Mask of the bits in the register that should be
 *                       modified.
 */
#define SET_REGISTER_BITS(ctrl_register, bitset, mask) do { \
	const uint8_t normalized_bitset = BITMASK_CHECK((bitset), (mask)); \
	BITMASK_CLEAR((ctrl_register), (mask)); /* Clear register before writing */\
	BITMASK_SET((ctrl_register), (normalized_bitset)); \
} while (0)

/**
 * Jump at the addresse 0x0000 (not a reset !)
 * @warning not a reset!
 */
#define Direct_jump_to_zero()   { asm ("jmp 0"::); }

/**
 * RESET device with Watchdog Timer.
 */
#define Hard_reset()    { WDTCR |= 1<<WDE;  while(1); }

#endif /* BITWISE_H */
