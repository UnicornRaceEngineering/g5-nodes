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
 * @files bson.c
 * Simplfied version of the BSON (Binary JSON) format.
 * See the spec at http://bsonspec.org/spec.html
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <usart.h>

#include "bson.h"

int32_t serialize_element(uint8_t *buff, struct bson_element *e, int32_t len) {

	int32_t buff_i = 0;
	buff[buff_i++] = e->e_id;

	if (e->key == NULL) return -buff_i;
	size_t key_len = strnlen(e->key, 32);
	if (len < buff_i+key_len+1) return -buff_i;
	for (int i = 0; i < key_len; ++i) {
		buff[buff_i++] = e->key[i];
	}

	buff[buff_i++] = '\0'; // Add null terminator to the key string

	switch (e->e_id) {
		case ID_STRING:
			{
				int32_t str_len = strnlen(e->str, len)+1;
				if (len < buff_i+str_len+sizeof(int32_t)) return -buff_i;

				for (int i = 0; i < sizeof(int32_t); ++i) {
					buff[buff_i++] = ((uint8_t*)&str_len)[i];
				}

				for (int32_t i = 0; i < str_len; ++i) {
					buff[buff_i++] = e->str[i];
				}
			}
			break;

		case ID_EMBEDED_DOCUMENT:
		case ID_ARRAY:
			//!> @TODO implement this
			return -buff_i;
			break;

		case ID_BOOL:
			buff[buff_i++] = e->boolean ? 0x01 : 0x00;
			break;

		case ID_MIN_KEY:
		case ID_MAX_KEY:
		case ID_UNDEFINED:
		case ID_NULL_VALUE:
			// Insert nothing
			break;

		case ID_32_INTEGER:
			// Integers is LSB first
			if (len < sizeof(int32_t)) return -buff_i;
			for (int i = 0; i < sizeof(int32_t); ++i) {
				buff[buff_i++] = ((uint8_t*)&e->int32)[i];
			}
			break;

		case ID_UTC_DATETIME:
		case ID_TIMESTAMP:
		case ID_64_INTEGER:
			// Integers is LSB first
			if (len < sizeof(uint64_t)) return -buff_i;
			for (int i = 0; i < sizeof(int64_t); ++i) {
				buff[buff_i++] = ((uint8_t*)&e->int64)[i];
			}
			break;
		case ID_BINARY:
			if (len < sizeof(uint32_t) + 1 + e->binary.len) return -buff_i;
			for (int i = 0; i < sizeof(int32_t); ++i) {
				buff[buff_i++] = ((uint8_t*)&e->binary.len)[i];
			}
			buff[buff_i++] = e->binary.subtype;
			for (int i = 0; i < e->binary.len; ++i) {
				buff[buff_i++] = ((uint8_t*)e->binary.data)[i];
			}
			break;

		default:
			return -buff_i; // error
			break;
	}

	return buff_i;
}
