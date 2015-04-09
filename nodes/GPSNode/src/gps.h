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
 * @file gps.h
 * Parses input data from a connected GPS module. @ref gps_set_getc() must be
 * called before use to register where to get GPS input from. @ref gps_get_fix()
 * can then be used to extract a GPS fix.
 */

#ifndef GPS_H
#define GPS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define GPS_BAUDRATE	(4800)


/**
 * Converts a DMS (Degrees, Minutes Seconds) coordinate to a DD (Decimal Degree)
 * @param  dms A pointer to a geographic coordinate in DMS format.
 * @return The converted DD value
 */
#define GPS_DMS_TO_DD(dms)	((double)((dms)->degrees + \
									 ((dms)->minutes / 60.0) + \
									 ((dms)->seconds / 3600.0)) * \
										(((dms)->direction == 'N' || \
										  (dms)->direction == 'E') \
											? 1 : -1))


//!< A geographic coordinate in DMS (Degrees, Minutes Seconds) format.
struct gps_coordinate {
	char direction; //!< 'N'/'S' or 'E'/'W'
	int16_t degrees;
	uint8_t minutes;
	double seconds;
};

struct gps_fix {
	struct gps_coordinate latitude;
	struct gps_coordinate longitude;
	int16_t speed; //!< speed in km/h
	bool valid; //!< if valid the fix quality is within international standards.
};

void gps_set_getc(FILE *stream);
int gps_get_fix(struct gps_fix *fix);

#endif /* GPS_H */
