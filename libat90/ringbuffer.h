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
 * @file ringbuffer.h
 * @brief
 * 	A simple always keep one slop open ring buffer of size RB_BUFFER_SIZE. The
 * 	buffer size is determined at compile time on a per module basis. That is a.c
 * 	can use a size of 64 but c.c cannot have both a 32 and 64 length buffer. The
 *  buffer size can be altered by defining RB_BUFFER_SIZE as the desired buffer
 *  size before including ringbuffer.h The default value for RB_BUFFER_SIZE is
 *  32
 *
 * 	The datatype of the buffer is uint8_t by default but this can also be
 * 	overridden by defining RB_DATA_t to another type before including
 * 	ringbuffer.h
 *
 * 	The buffer size is recommended to a power of 2 value as this allows for
 * 	faster calculating the next index values. However non power of 2 values can
 * 	be used.
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint8_t
#include <string.h>  // for memset

#include "utils.h"   // for IS_POW2

// #define RB_BUFFER_MASK	(RB_BUFFER_SIZE-1)
#define RB_BUFFER_MASK(B)	((B)->size-1)

typedef struct ringbuffer_t{
	uint8_t *buffer; //!< Pointer to the actual buffer
	size_t start; //!< Index of the start of valid data in the buffer
	size_t end; //!< Index of the end of valid data in the buffer
	size_t size; //!< Size of the buffer @note muster be pow2 value
} ringbuffer_t;


#define rb_nextStart(B)		(((B)->start+1) & RB_BUFFER_MASK((B)))
#define rb_nextEnd(B)		(((B)->end+1) & RB_BUFFER_MASK((B)))

/**
 * Check if the ring buffer is empty.
 * @param  B pointer to the ringbuffer_t
 * @return   Boolean true if empty
 */
#define rb_isEmpty(B)			((B)->end == (B)->start)


/**
 * Check if the ring buffer is full.
 * @param  B pointer to the ringbuffer_t
 * @return   Boolean true if full
 */
#define rb_isFull(B)			(rb_nextEnd((B)) == (B)->start)


/**
 * Number of bytes used in buffer.
 * @param  B pointer to the ringbuffer_t
 * @return   size_t bytes
 */
#define rb_bytesUsed(B)			(((b)->end - (b)->start) + (-((int)((b)->end < (b)->start)) & (b)->size))


/**
 * @brief
 * Initializes the ring buffer to it's initial values and zero the buffer.
 * @param buffer The buffer to initialize
 */
static inline int rb_init(ringbuffer_t* const rb, uint8_t *buf, size_t size){
	if (!IS_POW2(size)) return -1;

	rb->buffer = buf;
	rb->size = size;

	rb->end = rb->start = 0;
	memset(rb->buffer, 0, sizeof(rb->size));

	return 0;
}

/**
 * @brief Inserts a byte to the ring buffer.
 * @notice This will overwrite the oldest element in the buffer so the caller
 * should check if the buffer is full using the rb_isFull() call.
 * @param  buffer The ring buffer
 * @param  data   The data point that is added to buffer
 * @return        1 on overflow error and 0 on success
 */
static inline void rb_push(ringbuffer_t *rb, uint8_t data) {
	rb->buffer[rb->end] = data;
	rb->end = rb_nextEnd(rb);
	if (rb_isEmpty(rb)) {
		rb->start = rb_nextStart(rb);
	}
}

/**
 * @breief
 * Get a byte from ring buffer.
 * @param  buffer The ring buffer
 * @param  data   pointer where returned byte is stored
 * @return        1 if no data is available and 0 on success
 */
static inline int rb_pop(ringbuffer_t *rb, uint8_t *data) {
	if (rb_isEmpty(rb)) return -1; // No data available

	*data = rb->buffer[rb->start];
	rb->start = rb_nextStart(rb);

	return 0; // Success
}

static inline unsigned rb_left(ringbuffer_t *rb) {
	return ((rb->start > rb->end) ? (rb->start - rb->end) : (rb->size - (rb->end - rb->start))) - 1;
}

/**
 * @brief
 * Get the next byte in the ring buffer without removing it from the buffer.
 * This will work the same as rb_pop except, successive calls to rb_peek will
 * return the same byte.
 * @param  buffer The ring buffer
 * @param  data   pointer where returned byte is stored
 * @return        1 if no data is available and 0 on success
 */
static inline int rb_peek(ringbuffer_t *rb, size_t index, uint8_t *data) {
	if (rb_isEmpty(rb)) return -1; // No data available

	// *data = rb->buffer[rb->start];
	*data = (rb->start + index) & RB_BUFFER_MASK(rb);
	return 0; // Success
}

#endif /* RINGBUFFER_H */
