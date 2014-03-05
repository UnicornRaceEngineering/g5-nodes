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

#include <stdint.h>
#include <string.h> // memset()

#ifndef RB_BUFFER_SIZE
	#define RB_BUFFER_SIZE 32
#endif

#define RB_BUFFER_MASK	(RB_BUFFER_SIZE-1)

#if RB_BUFFER_SIZE < (1<<8)
 	typedef uint8_t rb_index_t;
#elif RB_BUFFER_SIZE < (1<<16)
 	typedef uint16_t rb_index_t;
#elif RB_BUFFER_SIZE < (1<<32)
 	typedef uint32_t rb_index_t;
#else
 	#error RB_BUFFER_SIZE is too big
#endif

// Determine if the buffer size is a power of 2
#if RB_BUFFER_SIZE  == (1<<2)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<3)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<4)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<5)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<6)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<7)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<8)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<9)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<10)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<11)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<12)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<13)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<14)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<15)
	#define RB_BUFFER_SIZE_IS_POW2
#elif RB_BUFFER_SIZE  == (1<<16)
	#define RB_BUFFER_SIZE_IS_POW2
#endif


#ifndef RB_DATA_t
#define RB_DATA_t uint8_t
#endif


typedef struct ringbuffer_t{
	RB_DATA_t buffer[RB_BUFFER_SIZE]; //!< The actual buffer
	rb_index_t start; //!< Index of the start of valid data in the buffer
	rb_index_t end; //!< Index of the end of valid data in the buffer
} ringbuffer_t;


#ifdef RB_BUFFER_SIZE_IS_POW2
	#define rb_nextStart(B)		(((B)->start+1) & RB_BUFFER_MASK)
	#define rb_nextEnd(B)		(((B)->end+1) & RB_BUFFER_MASK)
#else
	#define rb_nextStart(B)		((((B)->start+1) % RB_BUFFER_SIZE))
	#define rb_nextEnd(B)		((((B)->end+1) % RB_BUFFER_SIZE))
#endif

#define rb_isEmpty(B)			((B)->end == (B)->start) //!< Returns a boolean whether ring buffer is empty
#define rb_isFull(B)			(rb_nextEnd((B)) == (B)->start) //!< Returns a boolean whether ring buffer is full

#define rb_reset(B)				{(B)->end = (B)->start = 0;}

/**
 * @brief
 * Initializes the ring buffer to it's initial values and zero the buffer.
 * @param buffer The buffer to initialize
 */
static inline void rb_init(ringbuffer_t* const buffer){
	rb_reset(buffer);
	memset(buffer->buffer, 0, sizeof(buffer->buffer));
}

/**
 * @brief Inserts a byte to the ring buffer.
 * @notice This will overwrite the oldest element in the buffer so the caller
 * should check if the buffer is full using the rb_isFull() call.
 * @param  buffer The ring buffer
 * @param  data   The data point that is added to buffer
 * @return        1 on overflow error and 0 on success
 */
static inline void rb_push(ringbuffer_t *buffer, RB_DATA_t data) {
	buffer->buffer[buffer->end] = data;
	buffer->end = rb_nextEnd(buffer);
	if (rb_isEmpty(buffer)) {
		buffer->start = rb_nextStart(buffer);
	}

	//return 0; // Success
}

/**
 * @breief
 * Get a byte from ring buffer.
 * @param  buffer The ring buffer
 * @param  data   pointer where returned byte is stored
 * @return        1 if no data is available and 0 on success
 */
static inline int rb_pop(ringbuffer_t *buffer, RB_DATA_t *data) {
	if (rb_isEmpty(buffer)) {
		return 1; // No data available
	}

	*data = buffer->buffer[buffer->start];
	buffer->start = rb_nextStart(buffer);

	return 0; // Success
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
static inline int rb_peek(ringbuffer_t *buffer, RB_DATA_t *data) {
	if (rb_isEmpty(buffer)) {
		return 1; // No data available
	}

	*data = buffer->buffer[buffer->start];
	return 0; // Success
}

#endif /* RINGBUFFER_H */
