#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"

#define ARR_LEN(x)  (sizeof(x) / sizeof(x[0]))
#define MIN(x, y)	((x < y) ? x : y)
#define MAX(x, y)	((x > y) ? x : y)

struct config {
   const char *name;			// Human readable name
   int id;						// Can ID
   int rounddec;				// Round to this many decimals
   int bit_length;				// Length in bits
   int max;						// Maximum expected value
   int min;						// Minimum expected value
   double (*conv)(int, int);	// Convert to human numbers
};

static double roundn(double value, int to) {
	//double places = pow(10.0, to);
	//return round(value * places) / places;
	return value;
}

/**
 * As C does not support any form of lambda functions each convertion function
 * must be declared here and then added to the list(array) of sensor configs.
 * @{
 */
static double stdConv(int x, int rounddec) {
	return roundn((x*1+0), rounddec);
}

static double StatusLambdaV2Conv(int x, int rounddec) {
	double val;
	if(x > 32768){
		x = -(65535-x);
	}
	val = 70-x/64.0;
	return roundn(val, rounddec);
}

static double airAndWaterTempConv(int x, int rounddec) {
	double val = (x * (-150.0/3840) + 120);
	return roundn(val, rounddec);
}

static double potmeterConvert(int x, int rounddec) {
	double val = ((x-336)/26.9);
	return roundn(val, rounddec);
}

static double rpmConv(int x, int rounddec) {
	double val = (x*0.9408);
	return roundn(val, rounddec);
}

static double mBarConv(int x, int rounddec) {
	double val = (x*0.75);
	return roundn(val, rounddec);
}

static double batteryConv(int x, int rounddec) {
	double val = (x*(1.0/210)+0);
	return roundn(val, rounddec);
}

static double StatusLambdaV2Conv2(int x, int rounddec) {
	double val;
	if(x > 32768){
		x = -(65535-x);
	}
	val = 70-x/64.0;
	return roundn(val/100, rounddec);
}

static double InjectorAndIgnitionTimeConv(int x, int rounddec) {
	double val = -0.75*x+120;
	return roundn(val, rounddec);
}

static double GXGYGZconv(int x, int rounddec) {
	double val;
	if(x > 32768){
		x = -(65535 - x);
	}
	val = x * (1.0/16384);
	return roundn(val, rounddec);
}

static double gearboardTempConv(int x, int rounddec) {
	double val;
	double resistance = ((10240000/(1024 - x)) - 10000);
	double temp = log(resistance);
	temp = 1 / (0.001129148 + (0.000234125 * temp) + (0.0000000876741 * temp * temp * temp));
	val = temp - 273.15;
	return roundn(val, rounddec);
}

static double waterInOutletTemoConv(int x, int rounddec) {
	double val = 127.5 * exp(-0.003286*x);
	return roundn(val, rounddec);
}

static double gearNeutralConv(int x, int rounddec) {
	double val;
	if( x > 100){
		x=1;
	}
	val = x;
	return roundn(val, rounddec);
}
/** @} */


static int get_config_from_ID(int id, const struct config *config,
	size_t cfg_length)
{
	for (int i = 0; i < cfg_length; ++i){
		if(id == config[i].id){
			return i; // index in the config list
		}
	}
	return -1; // No ID was found
}

int parse_next(const uint8_t data_byte, struct sensor *sensor, struct parser *p) {
	const uint8_t startSequence[] = {255, 123, 10};
	const struct config cfg[] = {
		#include "config.h"
	};

	p->sensor_found = false; // we dont yet know if a sensor is found

	if((p->package_start_counter == 0) && (data_byte == startSequence[0]))
		p->package_start_counter = 1;
	else if((p->package_start_counter == 1) && (data_byte == startSequence[1]))
		p->package_start_counter = 2;
	else if((p->package_start_counter == 2) && (data_byte == startSequence[2])){
		p->package_start_counter = 0;
		p->package_start = true;
		return PARSER_NEED_NEXT; // we are ready for next byte
	}

	if (p->package_start) {
		// Reset
		p->package_start = false;
		p->bytes_to_read = -1;
		p->val_out = 0;

		p->cfg_index = get_config_from_ID(data_byte, cfg, ARR_LEN(cfg));
		if(p->cfg_index == -1){
			// Invalid id found at currByte !
			return -(int)data_byte; // return the negative value as all other return codes are unsigned
		}
		p->bytes_to_read = cfg[p->cfg_index].bit_length/8;
		return PARSER_NEED_NEXT; // Ready for next byte
	}

	if (p->bytes_to_read > 0) {
		p->val_out = p->val_out + (data_byte << (8*(p->bytes_to_read-1)));
		p->bytes_to_read -= 1; // We have read a byte so we obviously have one less to read
		return PARSER_NEED_NEXT; // We have read and added the byte, So ready for the next
	}

	if (p->bytes_to_read == 0) {
		const char* name = cfg[p->cfg_index].name;
		double value = cfg[p->cfg_index].conv(p->val_out, cfg[p->cfg_index].rounddec);
		value = MIN(value, cfg[p->cfg_index].max);
		value = MAX(value, cfg[p->cfg_index].min);

		// Copy the value into the sensor object
		sensor->name = name;
		sensor->id = cfg[p->cfg_index].id;
		sensor->cfg_index = p->cfg_index;
		sensor->value = value;

		p->sensor_found = true; // we have found a sensor

		// Reset
		p->bytes_to_read = -1;
		p->val_out = 0;

		// Are the a next data byte?
		p->cfg_index = get_config_from_ID(data_byte, cfg, ARR_LEN(cfg));
		if (p->cfg_index == -1) {
			// No more data
			return PARSER_NO_MORE_DATA;
		}

		p->bytes_to_read = cfg[p->cfg_index].bit_length/8;
		return PARSER_NEED_NEXT;

	}
	return PARSER_NO_THING_TODO;
}


#if 0
static inline char* stradd(const char* a, const char* b){
	size_t len = strlen(a) + strlen(b);
	char *ret = (char*)malloc(len * sizeof(char) + 1);
	*ret = '\0';
	return strcat(strcat(ret, a) ,b);
}

void canfile2csv(const char *path){
	int i;
	FILE *fp = fopen(path, "rb"); // open first argument as read binary

	if( fp == NULL ){
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}

	fseek(fp, 0L, SEEK_END); // Seek to the end
	size_t fSize = ftell(fp); // get the file size
	fseek(fp, 0L, SEEK_SET); // Reset the seeker to the beginning of the file

	// open the output file
	char *outname = stradd(path, ".csv"); // remember to free
	FILE *outfp = fopen(outname, "w"); // open outputfile as the input.csv
	if(outfp == NULL){
		perror("Failed to open outputfile");
		exit(EXIT_FAILURE);
	}

	config_t cfg[] = INIT_CONFIG;
	parser_t p = INIT_PARSER(cfg);
	fprintf(outfp, "id,name,value\n");
	for (i = 0; i < fSize; ++i){
		int rc = getc(fp);
		if(rc == EOF){
			printf("An error occurred while reading the file \"%s\" (Unexpected EOF).\n", path);
			exit(EXIT_FAILURE);
		}

		sensor_t s;
		int rv = parseNext((uint8_t)rc, &s, &p);
		if(p.sensorFound){
			fprintf(outfp, "%d,\"%s\",%.2f\n", s.id, s.name, s.value);
		}else if( rv < 0){
			printf("Invalid ID found: %d in \"%s\" at offset %d\n", -rv, path, i);
		}
	}
	fclose(fp);
	fclose(outfp); free(outname);
}
#endif

