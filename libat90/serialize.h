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

#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <stdint.h>
#include <stdlib.h>

enum datatype {
	DT_BOOLEAN,

	DT_UINT8,
	DT_INT8,
	DT_UINT16,
	DT_INT16,
	DT_UINT32,
	DT_INT32,
	DT_UINT64,
	DT_INT64,

	DT_FLOAT32,
	DT_FLOAT64,

	DT_CSTRING, // Nullterminated C string

	DT_UTC_DATETIME, // int64 containing the offset from unix epoch in ms

	DT_SCHEMA, // Nested schema
	DT_SCHEMA_END,
};

enum schemas {
	ECU_DATA,

	SCHEMA_DEFENITION,
	TEST_SCHEMA,
};

#if 0
int add_to_schema(uint8_t *schema, char* key, enum datatype dt, size_t n);
#endif

#endif /* SERIALIZE_H */
