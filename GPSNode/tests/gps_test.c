#include "../src/gps.c"

#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <usart.h>

char test_data[] = "blablablabla$GPRMC,092750.000,A,5321.6802,N,00630.3372,W,0.02,31.66,280511,,,A*43\r\nfewfioewjiods";
int test_data_index = 0;

bool floatcmp(double a, double b, double epsilon) {
	return fabs(a - b) <= ( (fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

uint8_t get_testdata(void) {
	uint8_t c = test_data[test_data_index++];
	if (test_data_index > strlen(test_data)) {
		test_data_index = 0;
	}
	return c;
}

int main(int argc, char const *argv[]) {

	gps_set_getc(get_testdata);

	struct sentence s1;
	struct gps_fix fix1 = {{0}};

	assert(build_sentence(&s1) == 0);
	assert(s1.str[0] == '$');
	assert(s1.str[strlen(s1.str) - 2] == '\r');
	assert(s1.str[strlen(s1.str) - 1] == '\n');
	assert(s1.str[strlen(s1.str)] == '\0');

	assert(valid_sentence(&s1));
	assert(from_rmc(&s1, &fix1) == 0);

	assert(fix1.valid == true);
	assert(fix1.latitude.direction == 'N');
	assert(fix1.latitude.degrees == 53);
	assert(fix1.latitude.minutes == 21);
	assert(floatcmp(fix1.latitude.seconds, (0.6802*60), 0.01));

	assert(fix1.longitude.direction == 'W');
	assert(fix1.longitude.degrees == 6);
	assert(fix1.longitude.minutes == 30);
	assert(floatcmp(fix1.longitude.seconds, (0.3372*60), 0.01));

	assert(fix1.speed == (int16_t)round(KNOTS_TO_KM(0.02)));

	struct gps_fix fix2 = {{0}};
	assert(gps_get_fix(&fix2) == 0);

	assert(fix2.valid == true);
	assert(fix2.latitude.direction == 'N');
	assert(fix2.latitude.degrees == 53);
	assert(fix2.latitude.minutes == 21);
	assert(floatcmp(fix2.latitude.seconds, (0.6802*60), 0.01));

	assert(fix2.longitude.direction == 'W');
	assert(fix2.longitude.degrees == 6);
	assert(fix2.longitude.minutes == 30);
	assert(floatcmp(fix2.longitude.seconds, (0.3372*60), 0.01));

	assert(fix2.speed == (int16_t)round(KNOTS_TO_KM(0.02)));

	float lat = GPS_DMS_TO_DD(&(fix2.latitude));
	float lon = GPS_DMS_TO_DD(&(fix2.longitude));
	usart1_printf("lat: %f\n", lat);
	usart1_printf("lat: %f\n", lon);

	usart1_printf("%s: all tests passed\n", __FILE__);
	return 0;
}
