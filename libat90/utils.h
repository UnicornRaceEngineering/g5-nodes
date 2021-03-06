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

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define ARR_LEN(arr)    (sizeof(arr) / sizeof(arr[0]))

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
#define BITMASK_FLIP(x,y) 	( (x) ^= (y)	) // Use: ( (x) = ~(y)	) instead?
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
#define MERGE_BYTE(h, l) 	( ((uint16_t)(((h) << 8)) | (l)) 	) //!< Merges two 8 bit bytes into one 16 bit byte where h is the High byte and l is the Low byte

#define MERGE_U32(msb_h, msb_l, lsb_h, lsb_l)	( ((uint32_t)(MERGE_BYTE(msb_h, msb_l)) << 16) | MERGE_BYTE(lsb_h, lsb_l) )

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

#define BYTE_POS(nbytes, bitindex)		(((((nbytes)*8)-1)-(bitindex)) / 8)
#define BYTE_POS_MSB(nbytes, bitindex)	(BYTE_POS((nbytes), (((nbytes)*8)-1)-(bitindex)))
#define MSK_HIGHEST(x)	(((0xFF + 1) >> (x)) - 1)
#define MSK_LOWEST(x)	((1 << (x)) - 1)

/**
 *	Extract byte value from the given array. This is usefull if we are operating
 *	on a register that is bigger than any normal C datatype.
 *	@param arr       The register byte array
 *	@param nbytes    The number of bytes in the byte array
 *	@param bit_index Index of the first bit of the value that should be extracted
 *	@param width     The bit width if the value that should be extracted
 */
#define EXTRACT_AT_INDEX(arr, nbytes, bit_index, width) ((arr[BYTE_POS((nbytes), (bit_index))] >> ((bit_index) - (BYTE_POS_MSB((nbytes), (bit_index)) * 8))) & MSK_LOWEST((width)))


/**
 * Jump at the addresse 0x0000 (not a reset !)
 * @warning not a reset!
 */
#define Direct_jump_to_zero()   { asm ("jmp 0"::); }

/**
 * RESET device with Watchdog Timer.
 */
#define Hard_reset()    { WDTCR |= 1<<WDE;  while(1); }

#define DEBUG()	do {printf("%s:%d\n", __FUNCTION__, __LINE__);} while(0)

int32_t map(int32_t x,
            const int32_t from_low, const int32_t from_high,
            const int32_t to_low, const int32_t to_high);

#endif /* UTILS_H */
