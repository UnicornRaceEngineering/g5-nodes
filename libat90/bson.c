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

#include "bson.h"

#define MAX_KEY_LEN	32


#define DOUBLE_FRACTION_S	52 // Fraction is 52 bits
#define DOUBLE_EXPONENT_S	11 // Exponent is 11 bits

#define FLOAT_FRACTION_S	23 // Fraction is 23 bits
#define FLOAT_EXPONENT_S	8  // Exponent is  8 bits

/**
 * represents a double as a 64-bit double.
 * @param  v double value we want to represent as a double
 * @return   64bit value that represents v as a double
 */
static uint64_t repr_double(double v) {
	if (sizeof(v) == sizeof(uint64_t)) {
		uint64_t d;
		memcpy(&d, &v, sizeof(d));
		return d;
	}

	// We have to manually do the cast
	uint32_t f;
	memcpy(&f, &v, sizeof(f));

	if (!(f&0x7fffffff)) return (uint64_t)f<<32;

	uint32_t s = f>>31; // get sign
	uint32_t e = ((f&0x7f800000)>>FLOAT_FRACTION_S) - 128; // get exponent and unbias from 128
	uint32_t m = f&0x007fffff; // get mantisa (the exponant)

	if (e == -128) {
		// handle denormals
		while (!(m&0x00800000)) {
			m <<= 1;
			e--;
		}

		// remove implicit 1
		m &= 0x007fffff;
		e++;
	} else if (e == 127) {
		// +/-infinity
		e = 1023;
	}

	uint64_t d = s; // store sign (in lowest bit)

	d <<= DOUBLE_EXPONENT_S; // make space for exponent
	d |= e + 1024;   // store rebiased exponent

	d <<= FLOAT_FRACTION_S; // add space for 23 most significant bits of mantisa
	d |= m;   // store 23 bits of mantisa

	d <<= DOUBLE_FRACTION_S-FLOAT_FRACTION_S; // trail zeros in place of lower significant bit of mantisa

	return d;
}


int32_t serialize_element(uint8_t *buff, struct bson_element *e, size_t len) {
	if (buff == NULL || e == NULL || len < 1) return -1;

	int32_t buff_i = 0;

	// Insert element id (type identifier)
	buff[buff_i++] = e->e_id;

	// Insert the key
	if (e->key == NULL) return -buff_i;
	size_t key_len = strnlen(e->key, MAX_KEY_LEN);
	if (len < buff_i+key_len+1) return -buff_i; // Check buffer is big enough
	for (size_t i = 0; i < key_len; ++i) {
		buff[buff_i++] = e->key[i];
	}
	buff[buff_i++] = '\0'; // Add null terminator to the key string

	switch (e->e_id) {
		case ID_STRING:
			{
				if (e->str == NULL) return -buff_i;
				int32_t str_len = strnlen(e->str, len)+1;
				if (len < (int32_t)buff_i+str_len+sizeof(int32_t))
					return -buff_i;

				for (size_t i = 0; i < sizeof(int32_t); ++i) {
					buff[buff_i++] = ((uint8_t*)&str_len)[i];
				}

				for (int32_t i = 0; i < str_len; ++i) {
					buff[buff_i++] = e->str[i];
				}
			}
			break;

		case ID_ARRAY:
			//!< @TODO implement this
			return -buff_i;
			break;

		case ID_EMBEDED_DOCUMENT:
			if (e->elements.e == NULL) return -buff_i;
			{
				int32_t doc_len = serialize(&buff[buff_i], e->elements.e,
					e->elements.n_elem, len);
				if (doc_len < 0) return -buff_i;
				buff_i += doc_len;
			}
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
			for (size_t i = 0; i < sizeof(int32_t); ++i) {
				buff[buff_i++] = ((uint8_t*)&e->int32)[i];
			}
			break;

		case ID_UTC_DATETIME:
		case ID_TIMESTAMP:
		case ID_64_INTEGER:
			// Integers is LSB first
			if (len < sizeof(uint64_t)) return -buff_i;
			for (size_t i = 0; i < sizeof(int64_t); ++i) {
				buff[buff_i++] = ((uint8_t*)&e->int64)[i];
			}
			break;
		case ID_BINARY:
			if (len < sizeof(uint32_t) + 1 + e->binary.len) return -buff_i;
			for (size_t i = 0; i < sizeof(int32_t); ++i) {
				buff[buff_i++] = ((uint8_t*)&e->binary.len)[i];
			}
			buff[buff_i++] = e->binary.subtype;
			for (int32_t i = 0; i < e->binary.len; ++i) {
				buff[buff_i++] = ((uint8_t*)e->binary.data)[i];
			}
			break;

		case ID_DOUBLE:
			{
				uint64_t d = repr_double(e->floating_val);
				if (len < sizeof(d)) return -buff_i;
				for (size_t i = 0; i < sizeof(d); ++i) {
					buff[buff_i++] = ((uint8_t*)&d)[i];
				}
			}
			break;

		default:
			return -buff_i; // error
			break;
	}

	return buff_i;
}

int32_t serialize(uint8_t *buff, struct bson_element *elements, size_t n_elem,
				  size_t len) {
	if (buff == NULL || elements == NULL || len < sizeof(int32_t)+1) return -1;

	int32_t document_size = sizeof(int32_t);
	uint8_t *buff_start = buff; // Reference to the start of the buffer

	buff += document_size;
	for (size_t i = 0; i < n_elem; ++i) {
		const int32_t buffer_left = len - document_size;
		if (buffer_left < 0) return -document_size;
		const int32_t e_size = serialize_element(buff, &elements[i],
			buffer_left);
		if (e_size < 0) return -document_size;
		document_size += e_size;
		buff += e_size;
	}

	document_size += 1; // Remeber we add the zero byte at the end
	for (size_t i = 0; i < sizeof(int32_t); ++i) {
		buff_start[i] = ((uint8_t*)&document_size)[i];
	}

	*buff = 0x00;

	return document_size;
}

int32_t find_key(struct bson_element *e, uint8_t *bson, int32_t n) {
	if (e == NULL || e->key == NULL || bson == NULL) return -1;

	const size_t key_len = strnlen(e->key, MAX_KEY_LEN);
	int32_t seek_pos = 0;
	int32_t document_size;
	for (size_t i = 0; i < sizeof(int32_t); ++i) {
		((uint8_t*)&document_size)[i] = bson[seek_pos++];
	}

	if (n < seek_pos) return -seek_pos;

	bool match_found = false;
	while (n > seek_pos && document_size > seek_pos && !match_found) {
		const enum  element_identifier e_id = bson[seek_pos++];

		const size_t bson_key = strnlen((char*)bson+seek_pos, MAX_KEY_LEN);
		seek_pos += bson_key + 1; // plus null byte
		if (n < seek_pos) return -seek_pos;
		if (bson_key == key_len && strncmp(e->key,
											(char*)&bson[seek_pos-bson_key-1],
											key_len) == 0) match_found = true;

		if (match_found) e->e_id = e_id;
		switch (e_id) {
			case ID_STRING:
				{
					if (n < seek_pos+(int32_t)sizeof(int32_t)) return -seek_pos;
					int32_t str_len;
					for (size_t i = 0; i < sizeof(int32_t); ++i) {
						((uint8_t*)&str_len)[i] = bson[seek_pos++];
					}
					if (n < seek_pos+str_len) return -seek_pos;
					if (match_found) strncpy(e->str, (char*)&bson[seek_pos],
											str_len);
					seek_pos += str_len;
				}
				break;

			case ID_ARRAY:
				//!< @TODO implement this
				break;

			case ID_EMBEDED_DOCUMENT:
				{
					if (n < seek_pos+(int32_t)sizeof(int32_t)) return -seek_pos;
					e->raw_elements.offset = seek_pos;
					int32_t doc_len;
					for (size_t i = 0; i < sizeof(int32_t); ++i) {
						((uint8_t*)&doc_len)[i] = bson[seek_pos++];
					}
					seek_pos += e->raw_elements.doc_len = doc_len;
				}
				break;

			case ID_BOOL:
				if (match_found) e->boolean = bson[seek_pos++] == 0x01
					? true : false;
				else seek_pos++;
				if (n < seek_pos) return -seek_pos;
				break;

			case ID_MIN_KEY:
			case ID_MAX_KEY:
			case ID_UNDEFINED:
			case ID_NULL_VALUE:
				// Do nothing
				break;

			case ID_32_INTEGER:
				if (n < seek_pos+(int32_t)sizeof(int32_t)) return -seek_pos;
				if (match_found) {
					for (size_t i = 0; i < sizeof(int32_t); ++i) {
						((uint8_t*)&e->int32)[i] = bson[seek_pos++];
					}
				} else {
					seek_pos += sizeof(int32_t);
				}
				break;

			case ID_UTC_DATETIME:
			case ID_TIMESTAMP:
			case ID_64_INTEGER:
			if (n < seek_pos+(int32_t)sizeof(int64_t)) return -seek_pos;
				if (match_found) {
					for (size_t i = 0; i < sizeof(int64_t); ++i) {
						((uint8_t*)&e->int64)[i] = bson[seek_pos++];
					}
				} else {
					seek_pos += sizeof(int64_t);
				}
				break;

			case ID_BINARY:
				if (n < seek_pos+(int32_t)sizeof(int32_t)+1) return -seek_pos;
				for (size_t i = 0; i < sizeof(int32_t); ++i) {
					((uint8_t*)&e->binary.len)[i] = bson[seek_pos++];
				}
				e->binary.subtype = bson[seek_pos++];
				if (n < seek_pos + e->binary.len) return -seek_pos;
				if (match_found) {
					for (int32_t i = 0; i < e->binary.len; ++i) {
						e->binary.data[i] = bson[seek_pos++];
					}
				} else {
					seek_pos += e->binary.len;
				}
				break;

			case ID_DOUBLE:
				//!< @TODO implement this

			default:
				return -seek_pos;
		}
	}

	return seek_pos;
}

#ifdef TEST_BSON

#include <stdio.h>

static bool verify_serialized_bson(int32_t encode_len, uint8_t *buff,
								   int32_t ref_len, uint8_t *ref) {
	bool test_passed = true;

	if (encode_len < 0) {
		printf("Enocde returned error %d\n", encode_len);
		return false;
	}

	if (ref_len != encode_len) {
		printf("Encoded string not same length as reference string\n");
		test_passed = false;
	}

	printf("serialize len=%u\n", encode_len);
	for (int i = 0; i < encode_len; ++i) {
		printf("%#.2x, ", buff[i]);
		if (buff[i] != ref[i]) {
			printf("Error expected %#.2x\n", ref[i]);
			test_passed = false;
			// break;
		}
	}
	printf("\n%s\n", test_passed ? "Test passed" : "Test failed");
	return test_passed;
}

int main(void) {
	uint8_t buff[128];
	bool test_passed = true;

	uint8_t bin[] = {1,2,3,4};
	struct bson_element e[] = {
		{.e_id = ID_STRING, .key = "str", .str = "This is a string"},
		{.e_id = ID_32_INTEGER, .key = "integer", .int32 = 42},
		{.e_id = ID_BINARY, .binary.subtype=SUB_GENERIC, .key="buff", .binary.data=bin, .binary.len=ARR_LEN(bin)},
	};

	// Test serializing single element
	{
		// Test String serializing
		printf("\n\nTest String serializing\n");
		{
			int32_t encode_len = serialize_element(buff, &e[0], ARR_LEN(buff));
			uint8_t ref[] = {0x2,0x73,0x74,0x72,0,0x11,0,0,0,0x54,0x68,0x69,0x73,0x20,0x69,0x73,0x20,0x61,0x20,0x73,0x74,0x72,0x69,0x6e,0x67,0};
			int32_t ref_len = ARR_LEN(ref);
			if (verify_serialized_bson(encode_len, buff, ref_len, ref) == false)
				test_passed = false;
		}

		// Test int32 serializing
		printf("\n\nTest int32 serializing\n");
		{
			int32_t encode_len = serialize_element(buff, &e[1], ARR_LEN(buff));
			uint8_t ref[] = {0x10, 0x69, 0x6e, 0x74, 0x65, 0x67, 0x65, 0x72, 0x00, 0x2a, 0x00, 0x00, 0x00};
			if (verify_serialized_bson(encode_len, buff, ARR_LEN(ref), ref) == false)
				test_passed = false;
		}

		// Test binary serializing
		printf("\n\nTest binary serializing\n");
		{
			int32_t encode_len = serialize_element(buff, &e[2], ARR_LEN(buff));
			uint8_t ref[] = {0x05, 0x62, 0x75, 0x66, 0x66, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,};
			if (verify_serialized_bson(encode_len, buff, ARR_LEN(ref), ref) == false)
				test_passed = false;
		}
	}

	printf("\n\nTest serialize array of elements\n");

	int32_t encode_len = serialize(buff, e, ARR_LEN(e), ARR_LEN(buff));
	uint8_t ref[] = {0x3b, 0x00, 0x00, 0x00, 0x02, 0x73, 0x74, 0x72, 0x00, 0x11, 0x00, 0x00, 0x00, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x00, 0x10, 0x69, 0x6e, 0x74, 0x65, 0x67, 0x65, 0x72, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x05, 0x62, 0x75, 0x66, 0x66, 0x00, 0x04, 0x00, 0, 0, 0, 1, 2, 3, 4, 0};
	if (verify_serialized_bson(encode_len, buff, ARR_LEN(ref), ref) == false)
		test_passed = false;

	printf("\n\nTest finding key\n");
		// Find the str key
	{
		char str_buff[64] = {'\0'};
		struct bson_element found_e = {.key="str", .str=str_buff};
		int32_t rc = find_key(&found_e, buff, ARR_LEN(buff));
		if (rc < 0) test_passed = false;
		if (strcmp(found_e.str, e[0].str) != 0) test_passed = false;
	}

	// Find the integer key
	{
		struct bson_element found_e = {.key="integer"};
		int32_t rc = find_key(&found_e, buff, ARR_LEN(buff));
		if (rc < 0) test_passed = false;
		if (found_e.int32 != e[1].int32) test_passed = false;
	}

	// Find the buff key containing binary data
	{
		uint8_t bin_buff[64] = {'\0'};
		struct bson_element found_e = {.key="buff", .binary.data=bin_buff};
		int32_t rc = find_key(&found_e, buff, ARR_LEN(buff));
		if (rc < 0) test_passed = false;
		if (memcmp(found_e.binary.data, e[2].binary.data, ARR_LEN(bin)) != 0) test_passed = false;
	}

	printf("\n\nTest serializing embedded doc\n");
	// Test serialize nested elements
	{
		struct bson_element embed_e = {.e_id=ID_EMBEDED_DOCUMENT, .key="doc", .elements.e=e, .elements.n_elem=ARR_LEN(e)};
		int32_t encode_len = serialize(buff, &embed_e, 1, ARR_LEN(buff));
		uint8_t ref[] = {69, 0, 0, 0, 3, 100, 111, 99, 0, 59, 0, 0, 0, 2, 115, 116, 114, 0, 17, 0, 0, 0, 84, 104, 105, 115, 32, 105, 115, 32, 97, 32, 115, 116, 114, 105, 110, 103, 0, 16, 105, 110, 116, 101, 103, 101, 114, 0, 42, 0, 0, 0, 5, 98, 117, 102, 102, 0, 4, 0, 0, 0, 0, 1, 2, 3, 4, 0, 0,};
		if (verify_serialized_bson(encode_len, buff, ARR_LEN(ref), ref) == false) test_passed = false;
	}

	printf("\n\n%s testing BSON\n", test_passed ? "SUCCESS" : "FAILED");
}

#endif
