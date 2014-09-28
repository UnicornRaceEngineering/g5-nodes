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

enum ecu_id {
	EMPTY,
	FUEL_PRESSURE,
	STATUS_LAP_COUNT,
	STATUS_INJ_SUM,
	LAST_GEAR_SHIFT,
	MOTOR_OILTEMP,
	OIL_PRESSURE,
	STATUS_TIME,
	STATUS_LAP_TIME,
	GEAR_OIL_TEMP,
	STATUS_TRACTION,
	STATUS_GAS,
	STATUS_LAMBDA_V2,
	STATUS_CAM_TRIG_P1,
	STATUS_CAM_TRIG_P2,
	STATUS_CHOKER_ADD,
	STATUS_LAMBDA_PWM,
	WATER_TEMP,
	MANIFOLD_AIR_TEMP,
	POTMETER,
	RPM,
	TRIGGER_ERR,
	CAM_ANGLE1,
	CAM_ANGLE2,
	ROAD_SPEED,
	MAP_SENSOR,
	BATTERY_V,
	LAMBDA_V,
	LOAD,
	INJECTOR_TIME,
	IGNITION_TIME,
	DWELL_TIME,
	GX,
	GY,
	GZ,
	MOTOR_FLAGS,
	OUT_BITS,
	TIME,
};

struct sensor {
	const char *name;			// Human readable name
	enum ecu_id id;					// ID
	double value; 				// Value of the sensor
};

void ecu_init(void);
void ecu_parse_package(void);

#endif /* ECU_H */
