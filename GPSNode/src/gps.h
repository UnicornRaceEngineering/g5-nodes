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


/**
 * Callback that should return a new byte from the GPS module when run.
 * @return      The next byte from the GPS module.
 */
typedef uint8_t (*gps_getc_t)(void);

//!< A geographic coordinate in DMS (Degrees, Minutes Seconds) format.
typedef struct gps_coordinate_t {
	char direction; //!< 'N'/'S' or 'E'/'W'
	int16_t degrees;
	uint8_t minutes;
	uint8_t seconds;
} gps_coordinate_t;

typedef struct gps_fix_t {
	gps_coordinate_t latitude;
	gps_coordinate_t longitude;
	int16_t speed; //!< speed in km/h
	bool valid; //!< if valid the fix quality is within international standards.
} gps_fix_t;

void gps_set_getc(gps_getc_t getc);
int gps_get_fix(gps_fix_t *fix);

#endif /* GPS_H */
