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
 * @file gps.c
 * Parses input data from a connected GPS module. @ref gps_set_getc() must be
 * called before use to register where to get GPS input from. @ref gps_get_fix()
 * can then be used to extract a GPS fix.
 */

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h> // round()
#include <stdio.h>

#include "gps.h"


#define MAX_SENTENCE_LEN		(128)
#define STARTS_WITH(pre, str)	(strncmp(pre, str, strlen(pre)) == 0)
#define KNOTS_TO_KM(d) 			((d)*1.852)


typedef struct sentence_t {
	char s[MAX_SENTENCE_LEN]; //!< The string representation of the sentence
	int i; //!< The index of the sentence
} sentence_t;


//!< Pointer to the function that returns a new byte from the GPS module.
gps_getc_t gps_getc = NULL;


/**
 * Calculate the checksum of a string up the the specified length.
 * @param  s   The string to run the checksum on.
 * @param  len Length of the string to be checked.
 * @return     The checksum of the string.
 */
static unsigned int checksum(const char *s, int len) {
	unsigned int c = 0;
	while (len--) {
		c ^= *s++;
	}
	return c;
}

/**
 * Convert as string of length n to unsigned 16 bit integer.
 * @param  s The string containing a numeric ascii value.
 * @param  n How many characters of the string that should be converted.
 * @return   The converted integral value as an unsigned 16 bit integer.
 */
static uint16_t str2uint(const char *s, size_t n) {
	int rc = 0;

	uint16_t base = 1;
	do {
		rc += (s[--n] - '0') * base;
		base *= 10;
	} while (n);

	return rc;
}


/**
 * @brief
 * Parses a nmea_0183 RMC (Recommended Minimum Navigation Information) sentence
 * and extracts latitude, longitude and speed.
 *
 * <pre>
 * Following ascii graph is taken from: http://www.catb.org/gpsd/NMEA.html
 *
 *                                                           12
 *        1         2 3       4 5        6  7   8   9    10 11|  13
 *        |         | |       | |        |  |   |   |    |  | |   |
 * $--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a,m,*hh<CR><LF>
 * </pre>
 *
 * Field Number:
 *  1. UTC Time
 *  2. Status, V=Navigation receiver warning A=Valid
 *  3. Latitude
 *  4. N or S
 *  5. Longitude
 *  6. E or W
 *  7. Speed over ground, knots
 *  8. Track made good, degrees true
 *  9. Date, ddmmyy
 *  10. Magnetic Variation, degrees
 *  11. E or W
 *  12. FAA mode indicator (NMEA 2.3 and later)
 *  13. Checksum
 *
 * @param  s   pointer to the sentence that is parsed.
 * @param  fix pointer to the GPS fix where parsed data is stored.
 * @return     0 on success 1 on error
 */
static int from_rmc(sentence_t *s, gps_fix_t *fix) {
	if (fix == NULL) return 1;

	const int status_pos = 18;
	fix->valid = s->s[status_pos] == 'A' ? true : false;

	const int lat_pos = 20;
	fix->latitude.degrees = str2uint(&(s->s[lat_pos]), 2);
	fix->latitude.minutes = str2uint(&(s->s[lat_pos+2]), 2);
	if (s->s[lat_pos+4] != '.') return 1;
	fix->latitude.seconds = (uint8_t)round(
		((double)str2uint(&(s->s[lat_pos+5]), 2) / 100.0) * 60.0);
	if (s->s[lat_pos+9] != ',') return 1;
	fix->latitude.direction = s->s[lat_pos+10];

	const int lon_pos = 32;
	fix->longitude.degrees = str2uint(&(s->s[lon_pos]), 3);
	fix->longitude.minutes = str2uint(&(s->s[lon_pos+3]), 2);
	if (s->s[lon_pos+5] != '.') return 1;
	fix->longitude.seconds = (uint8_t)round(
		((double)str2uint(&(s->s[lon_pos+6]), 2) / 100.0) * 60.0);
	if (s->s[lon_pos+10] != ',') return 1;
	fix->longitude.direction = s->s[lon_pos+11];

	const int speed_pos = 45;
	fix->speed = (int16_t)round(
		KNOTS_TO_KM(strtod(&(s->s[speed_pos]), NULL)));

	return 0;
}

/**
 * Builds a complete sentence by continuously asking the GPS module for a new
 * char. The sentence is saved in the global value aptly named sentence. The
 * length of the sentence including null terminator is stored in  the global
 * value s_index.
 * @param  s Pointer to where the build sentence is stored.
 * @return   0 if success 1 if error
 */
static int build_sentence(sentence_t *s) {
	if (gps_getc == NULL) return 1;

	char c;
	while ((c = gps_getc()) != '$'); // Wait for start sentence;

	int i = 0;

	do {
		if (i > MAX_SENTENCE_LEN) return 1;
		s->s[i++] = c;
	} while ((c = gps_getc()) != '\n');

	s->s[i++] = c;
	s->s[i] = '\0';
	s->i = i;

	return 0;
}

/**
 * Verifies the sentence checksum.
 * @param  s Pointer to the sentence that is verified.
 * @return   true if valid, false if invalid.
 */
static bool valid_sentence(sentence_t *s) {
	// Extract the checksum
	const int chksum_pos = s->i-4;
	unsigned int chksum = strtoul(&(s->s[chksum_pos]), NULL, 16);

	// Verify that the checksum is valid. As the checksum should not include the
	// sentence start specifier ('$') we give the checksum function a string
	// where the start specifier is not included.
	const int len_without_chksum = s->i-6;
	return (chksum == checksum(&(s->s[1]), len_without_chksum));
}

/**
 * Set the function that is used to get a character from the GPS module
 * @param getc The gps getc function pointer.
 */
void gps_set_getc(gps_getc_t getc) {
	gps_getc = getc;
}

/**
 * Receive fix data fro the GPS module and save it for later use.
 * @param  fix Where the GPS fix data is saved.
 * @return     0 on success 1 on error.
 */
int gps_get_fix(gps_fix_t *fix) {
	sentence_t sentence = {
		.s = {'\0'},
		.i = 0
	};

	if (build_sentence(&sentence) != 0) return 1;
	if (!STARTS_WITH("$GPRMC", sentence.s)) return 1;
	if (!valid_sentence(&sentence)) return 1;
	if (from_rmc(&sentence, fix) != 0) return 1;

	return 0;
}