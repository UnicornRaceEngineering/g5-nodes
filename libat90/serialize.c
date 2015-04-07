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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "serialize.h"


#if 0
int add_to_schema(uint8_t *schema, char* key, enum datatype dt, size_t n) {
	size_t len = strlen(key) + 1; // remember the null terminator
	if (len > n) return -1;
	strcpy((char*)schema, key);
	schema[len] = (int8_t)dt;
	return len+1;
}



#include <stdio.h>
int main(void){
	struct test_pkg {
		uint8_t key;
		int8_t key2;
	} pkg;

	uint8_t schema[1024*1024] = {'\0'};
	int s = 0;
	schema[s++] = TEST_SCHEMA;

	s += add_to_schema(schema+s, "some int", DT_UINT8, 1024*1024-s);

	s += add_to_schema(schema+s, "schema", DT_SCHEMA, 1024*1024-s);

		s += add_to_schema(schema+s, "key", DT_UINT8, 1024*1024-s);
		s += add_to_schema(schema+s, "key2", DT_INT8, 1024*1024-s);

	s += add_to_schema(schema+s, "", DT_SCHEMA_END, 1024*1024-s);

	s += add_to_schema(schema+s, "another schema", DT_SCHEMA, 1024*1024-s);
		s += add_to_schema(schema+s, "something", DT_UINT8, 1024*1024-s);
			s += add_to_schema(schema+s, "Nested schema", DT_SCHEMA, 1024*1024-s);
				s += add_to_schema(schema+s, "nested1", DT_UINT8, 1024*1024-s);
				s += add_to_schema(schema+s, "nested2", DT_UINT8, 1024*1024-s);
		s += add_to_schema(schema+s, "", DT_SCHEMA_END, 1024*1024-s);
	s += add_to_schema(schema+s, "", DT_SCHEMA_END, 1024*1024-s);

	for (int i = 0; i < s; ++i) {
		// printf("%c", schema[i]);
		printf("%d, ", schema[i]);
	}
	printf("\n");

	uint8_t data[] = {(uint8_t)TEST_SCHEMA, 0xFF, -1};

	return 0;
}

#endif
