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

#ifndef ECU_H
#define ECU_H

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

void ecu_init(void);
void ecu_parse_package(void);

#endif /* ECU_H */
