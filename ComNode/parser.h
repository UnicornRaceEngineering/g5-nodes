#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

enum parser_state {
	// PARSER_FOUND,
	PARSER_NEED_NEXT,
	PARSER_NO_MORE_DATA,
	PARSER_NO_THING_TODO,
};

struct parser {
	int package_start_counter; // how many of the start sequence bytes have we seen
	bool package_start; // Is this the beginning of a package?
	int bytes_to_read; // How many bytes do we need to read in this package?
	int val_out; // The final value when all bytes are read

	int cfg_index; // index in the sensor config

	bool sensor_found; // Have we found a sensor?
};

struct sensor {
   const char *name;			// Human readable name
   int id;						// Can ID
   int cfg_index; 				// Index number in the config array
   double value; 				// Value of the sensor
};



int parse_next(uint8_t data_byte, struct sensor *sensor, struct parser *p);

#endif /* PARSER_H */
