#include "../src/gps.c"

#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

char test_data[] = "blablablabla$GPRMC,092750.000,A,5321.6802,N,00630.3372,W,0.02,31.66,280511,,,A*43\r\nfewfioewjiods";
int test_data_index = 0;

uint8_t get_testdata(void) {
	uint8_t c = test_data[test_data_index++];
	if (test_data_index > strlen(test_data)) {
		test_data_index = 0;
	}
	return c;
}

int main(int argc, char const *argv[]) {

	gps_set_getc(get_testdata);

	sentence_t s1;
	gps_fix_t fix1 = {{0}};

	assert(build_sentence(&s1) == 0);
	assert(s1.s[0] == '$');
	assert(s1.s[strlen(s1.s) - 2] == '\r');
	assert(s1.s[strlen(s1.s) - 1] == '\n');
	assert(s1.s[strlen(s1.s)] == '\0');

	assert(valid_sentence(&s1));
	assert(from_rmc(&s1, &fix1) == 0);

	assert(fix1.valid == true);
	assert(fix1.latitude.direction == 'N');
	assert(fix1.latitude.degrees == 53);
	assert(fix1.latitude.minutes == 21);
	assert(fix1.latitude.seconds == (uint8_t)round(0.6802*60));

	assert(fix1.longitude.direction == 'W');
	assert(fix1.longitude.degrees == 6);
	assert(fix1.longitude.minutes == 30);
	assert(fix1.longitude.seconds == (uint8_t)round(0.3372*60));

	assert(fix1.speed == (int16_t)round(KNOTS_TO_KM(0.02)));

	gps_fix_t fix2 = {{0}};
	assert(gps_get_fix(&fix2) == 0);

	assert(fix2.valid == true);
	assert(fix2.latitude.direction == 'N');
	assert(fix2.latitude.degrees == 53);
	assert(fix2.latitude.minutes == 21);
	assert(fix2.latitude.seconds == (uint8_t)round(0.6802*60));

	assert(fix2.longitude.direction == 'W');
	assert(fix2.longitude.degrees == 6);
	assert(fix2.longitude.minutes == 30);
	assert(fix2.longitude.seconds == (uint8_t)round(0.3372*60));

	assert(fix2.speed == (int16_t)round(KNOTS_TO_KM(0.02)));

	printf("%s: all tests passed\n", __FILE__);
	return 0;
}