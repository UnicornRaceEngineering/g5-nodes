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
 * @file ecu.h
 * High level interface for the receiving data from the ECU.
 * The ECU communicates via. UART, but require a keep-alive heart beat to
 * actually send any data. Therefor we must periodically send the heartbeat.
 *
 * @note that the frequency we send the heartbeat is *NOT* the frequency with
 * which we receive data from the ECU.
 *
 * The structure of the data we receive from the ECU can be found in
 * "ecu_package_layout.inc".
 *
 */

#ifndef ECU_H
#define ECU_H

#include <stdint.h>

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

	N_ECU_IDS
};

#define ECU_ID_NAME(ecu_id) ((char const* const[]) { \
	[EMPTY] = "", \
	[FUEL_PRESSURE] = "Fuel Press.", \
	[STATUS_LAP_COUNT] = "Status Lap Counter", \
	[STATUS_INJ_SUM] = "Status Injection Sum", \
	[LAST_GEAR_SHIFT] = "Last Gear Shift", \
	[MOTOR_OILTEMP] = "Motor Oil Temp", \
	[OIL_PRESSURE] = "Oil Pressure", \
	[STATUS_TIME] = "Status Time", \
	[STATUS_LAP_TIME] = "Status Lap Time", \
	[GEAR_OIL_TEMP] = "Gear Oil Temp", \
	[STATUS_TRACTION] = "Status Traction", \
	[STATUS_GAS]  = "Status Gas", \
	[STATUS_LAMBDA_V2] = "Status LambdaV2", \
	[STATUS_CAM_TRIG_P1] = "Status Cam Trig P1", \
	[STATUS_CAM_TRIG_P2] = "Status Cam Trig P2", \
	[STATUS_CHOKER_ADD] = "Status Choker Add", \
	[STATUS_LAMBDA_PWM] = "Status Lambda PWM", \
	[WATER_TEMP] = "WaterMotor temp", \
	[MANIFOLD_AIR_TEMP] = "ManifoldAir temp", \
	[POTMETER] = "Potmeter (0-100%)", \
	[RPM] = "RPM", \
	[TRIGGER_ERR] = "Trigger Err", \
	[CAM_ANGLE1] = "Cam Angle1", \
	[CAM_ANGLE2] = "Cam Angle2", \
	[ROAD_SPEED] = "RoadSpeed (km/h)", \
	[MAP_SENSOR] = "Manifold press. (mBar)", \
	[BATTERY_V] = "Batt. volt", \
	[LAMBDA_V] = "Lambda (<1 => Rich)", \
	[LOAD] = "Load", \
	[INJECTOR_TIME] = "Injector Time", \
	[IGNITION_TIME] = "Ignition Time", \
	[DWELL_TIME] = "Dwell Time", \
	[GX] = "GX", \
	[GY] = "GY", \
	[GZ] = "GZ", \
	[MOTOR_FLAGS] = "Motor Flags", \
	[OUT_BITS] = "Out Bits", \
	[TIME] = "Time", \
}[ecu_id])

struct sensor {
	enum ecu_id id;				// ID

	//!< @TODO is all sensor data really of type double? perhabs we should use
	//!< a union here instead with a double and int field
	union {
		float value;
		int32_t int_val;
	};
};

void ecu_init(void);
void ecu_parse_package(void);
void ecu_send_schema(void);

#endif /* ECU_H */
