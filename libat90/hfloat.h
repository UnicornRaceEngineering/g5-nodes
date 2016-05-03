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

#ifndef HFLOAT_H
#define HFLOAT_H


#include <stdint.h>


typedef uint16_t hfloat;

union float_uint {
	float asFloat;
	uint32_t asUint;
};


/*
 * Converts a 32bit floating point number to a 16bit half float.
 * This is suitable for strict memory and network requirements.
 * WARNING: This course a loss of precision and does not take special cases
 * into account (like infinities).
 */
inline hfloat float2hfloat(const float f) {
	const union float_uint fu = { .asFloat = f };
	const uint32_t u = fu.asUint;
	return ((u >> 16) & 0x8000) | ((((u & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((u >> 13) & 0x03ff);
}


/*
 * Converts a given half float back to a full 32bit floating point value.
 * This is necessary when wanting to do math on a half float.
 */
inline float hfloat2float(const hfloat f) {
	/* We put our 16bit float 'container' into a 32bit one, so that bitshfifting works correctly. */
	const uint32_t f32 = (uint32_t)f;
	const uint32_t u = ((f32 & 0x8000) << 16) | (((f32 & 0x7c00) + 0x1C000) << 13) | ((f32 & 0x03FF) << 13);
	const union float_uint fu = { .asUint = u };
	return fu.asFloat;
}


#endif /* HFLOAT_H */
