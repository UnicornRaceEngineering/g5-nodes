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

#ifndef BSON_H
#define BSON_H

#include <stdint.h>
#include <stdbool.h>

enum element_identifier {
	ID_DOUBLE 			= 0x01,
	ID_STRING 			= 0x02,
	ID_EMBEDED_DOCUMENT = 0x03,
	ID_ARRAY			= 0x04,
	ID_BINARY			= 0x05,
	ID_UNDEFINED		= 0x06,
	ID_OBJECTID			= 0x07,
	ID_BOOL				= 0x08,
	ID_UTC_DATETIME		= 0x09,
	ID_NULL_VALUE		= 0x0A,
	ID_REGEX			= 0x0B,
	ID_DBPOINTER		= 0x0C,
	ID_JS_CODE			= 0x0D,

	// 0x0E Deprecated

	ID_JS_CODE_WS		= 0x0F,
	ID_32_INTEGER		= 0x10,
	ID_TIMESTAMP		= 0x11,
	ID_64_INTEGER		= 0x12,
	ID_MIN_KEY			= 0xFF,
	ID_MAX_KEY			= 0x7F,
};

enum binary_subtype {
	SUB_GENERIC			= 0x00,
	SUB_FUNCTION		= 0x01,
	SUB_BIN_OLD			= 0x02,
	SUB_UUID_OLD		= 0x03,
	SUB_UUID 			= 0x04,
	SUB_MD5				= 0x05,
	SUB_USER_DEFINED 	= 0x80,
};

struct bson {
	uint8_t *data;
};

struct bson_element {
	enum element_identifier e_id;
	char *key;
	union {
		char *str;
		uint8_t *arr;
		bool boolean;
		int64_t utc_datetime;
		int32_t int32;
		int64_t timestamp;
		int64_t int64;
		struct {
			enum binary_subtype subtype;
			uint8_t *data;
			int32_t len;
		} binary;
	};
};

int32_t serialize_element(uint8_t *buff, struct bson_element *e, int32_t len);

#endif /* BSON_H */
