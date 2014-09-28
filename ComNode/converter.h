#ifndef CONVERTER_H
#define CONVERTER_H

#include <stdint.h>


enum id {
	EMPTY 				= 0,
	FUEL_PRESSURE 		= 1,
	STATUS_LAP_COUNT 	= 2,
	STATUS_INJ_SUM 		= 3,
	LAST_GEAR_SHIFT 	= 4,
	MOTOR_OILTEMP 		= 5,
	OIL_PRESSURE 		= 6,
	STATUS_TIME			= 7,
	STATUS_LAP_TIME 	= 8,
	GEAR_OIL_TEMP 		= 9,
	STATUS_TRACTION 	= 10,
	STATUS_GAS 			= 11,
	STATUS_LAMBDA_V2 	= 12,
	STATUS_CAM_TRIG_P1 	= 13,
	STATUS_CAM_TRIG_P2 	= 14,
	STATUS_CHOKER_ADD 	= 15,
	STATUS_LAMBDA_PWM 	= 16,
	WATER_TEMP 			= 17,
	MANIFOLD_AIR_TEMP 	= 18,
	POTMETER 			= 19,
	RPM 				= 20,
	TRIGGER_ERR 		= 21,
	CAM_ANGLE1 			= 22,
	CAM_ANGLE2 			= 23,
	ROAD_SPEED 			= 24,
	MAP_SENSOR 			= 25,
	BATTERY_V 			= 26,
	LAMBDA_V 			= 27,
	LOAD 				= 28,
	INJECTOR_TIME 		= 29,
	IGNITION_TIME 		= 30,
	DWELL_TIME 			= 31,
	GX 					= 32,
	GY 					= 33,
	GZ 					= 34,
	MOTOR_FLAGS 		= 35,
	OUT_BITS 			= 36,
	TIME 				= 37,
};

struct sensor {
   const char *name;			// Human readable name
   enum id id;					// ID
   double value; 				// Value of the sensor
};



double convert(enum id id, uint16_t raw_value);

#endif /* CONVERTER_H */
