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
 * @file heap.c
 * @brief
 * Implements a simple heap stored in a global array.
 * Come with the capability to count the fullness of the heap
 * and roughly calculate the fragmentation of free space.
 * @TODO either add proper support or return an error
 * for a heap size > 256
 */

#include <stdint.h>
#include "heap.h"

static volatile uint8_t heap[HEAPSIZE];
static volatile uint8_t header;

/**
 * Initializes the heap.
 * The header is initializes to by to byte long.
 * A single header is placed at the very start of the heap.
 * This header is always placed pointing at the next allocation
 * or at the end of the heap.
 * The size is always zero.
 * Having a static header at the start of the heap simplifies allocation in
 * certain situations.
 * @param 	void
 * @return 	void
 */
void init_heap (void) {
	header = 2;
	heap[0] = HEAPSIZE;
	heap[1] = 0;
}

/**
 * Allocates space on the heap.
 * @param size	The size of the allocation.
 * @return		A void pointer to the first place in the allocated space on
 * 				success otherwise NULL.
 */
void* malloc_(uint8_t size) {
	uint8_t heap_ptr = 0;
	uint8_t next_blk;
	uint8_t blk_size;
	do {
		next_blk = heap[heap_ptr];
		blk_size = heap[heap_ptr + 1] + header;
		if ((next_blk - (heap_ptr + blk_size)) < (size + header)) {
			heap_ptr = next_blk;
		} else {
			uint8_t new_ptr = heap_ptr + header + heap[heap_ptr + 1];
			heap[new_ptr] = next_blk;
			heap[new_ptr + 1] = size;
			heap[heap_ptr] = new_ptr;
			return (void*)&heap[new_ptr + header];
		}
	} while(heap_ptr != HEAPSIZE);
	return 0;
}

/**
 * Frees up an allocation on the heap.
 * Does not overwrite the memory used.
 * @param *heap_ptr Pointer to the allocated space.
 * @return   		0 if success otherwise 1
 */
uint8_t free_(void *heap_ptr)
{
	uint8_t target = (heap_ptr - header) - (void*)&heap[0];
	uint8_t pointer = 0;

	do {
		if (heap[pointer] == target) {
			heap[pointer] = heap[heap[pointer]];
			return 0;
		} else {
			pointer = heap[pointer];
		}
	} while (pointer != HEAPSIZE);
	return 1;
}

/**
 * Counts the number of bytes occupied on the heap, including 2 bytes per header
 * and the static header at the start.
 * @param void
 * @return   	Number of bytes used/allocated.
 */
uint8_t count_heap(void) {
	uint8_t pointer = heap[0];
	uint8_t counter = 2;
	do {
		counter += header + heap[pointer + 1];
		pointer = heap[pointer];
	} while (pointer != HEAPSIZE);
	return counter;
}

/**
 * Calculates the fragmentation on the heap by a score of 0 to 100
 * where 0 means that all the free memory sits in one chunk and 100
 * means that all the free bytes are split up in between allocations.
 * Notice that this calculation should not be considered exact in all situations
 * and may be regarded as an approximation.
 * @param void
 * @return 		Fragmentation percentage range: 0-100
 */
uint8_t count_fragmentation(void) {
	uint8_t free_total = HEAPSIZE - count_heap();
	uint8_t free_bytes;
	uint8_t fragmentation = 100;
	uint8_t pointer = 0;
	uint8_t blk_size;
	uint8_t fraction;

	do {
		blk_size = header + heap[pointer + 1];
		free_bytes = heap[pointer] - (blk_size + pointer);
		fraction = free_bytes * 100 / free_total;
		fragmentation -= fraction * fraction / 100;
		pointer = heap[pointer];
	} while (pointer != HEAPSIZE);
	return fragmentation;
}
