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
#include <stddef.h>
#include <stdbool.h>
#include <system_messages.h>


#define ECU_ID_NAME(ecu_id) ((char const* const[]) { \
	[ECU_EMPTY] = "", \
	[ECU_FUEL_PRESSURE] = "Fuel Press(bar).", \
	[ECU_STATUS_LAP_COUNT] = "Status Lap Counter", \
	[ECU_STATUS_INJ_SUM] = "Status Injection Sum", \
	[ECU_LAST_GEAR_SHIFT] = "Last Gear Shift", \
	[ECU_MOTOR_OILTEMP] = "Motor Oil Temp(cel)", \
	[ECU_OIL_PRESSURE] = "Oil Pressure(on/off)", \
	[ECU_STATUS_TIME] = "Status Time", \
	[ECU_STATUS_LAP_TIME] = "Status Lap Time", \
	[ECU_GEAR_OIL_TEMP] = "Gear Oil Temp", \
	[ECU_STATUS_TRACTION] = "Status Traction", \
	[ECU_STATUS_GAS]  = "Status Gas", \
	[ECU_STATUS_LAMBDA_V2] = "Status LambdaV2", \
	[ECU_STATUS_CAM_TRIG_P1] = "Status Cam shaft Trig P1", \
	[ECU_STATUS_CAM_TRIG_P2] = "Status Cam shaft Trig P2", \
	[ECU_STATUS_CHOKER_ADD] = "Status Choker Add (open/closed air intake)", \
	[ECU_STATUS_LAMBDA_PWM] = "Status Lambda PWM", \
	[ECU_WATER_TEMP] = "WaterMotor temp", \
	[ECU_MANIFOLD_AIR_TEMP] = "ManifoldAir temp", \
	[ECU_SPEEDER_POTMETER] = "Speeder Potmeter (0-100%)", \
	[ECU_RPM] = "RPM", \
	[ECU_TRIGGER_ERR] = "Trigger Err", \
	[ECU_CAM_ANGLE1] = "Cam Angle1", \
	[ECU_CAM_ANGLE2] = "Cam Angle2", \
	[ECU_ROAD_SPEED] = "RoadSpeed (km/h)", \
	[ECU_MAP_SENSOR] = "Manifold press. (mBar)", \
	[ECU_BATTERY_V] = "Batt. volt", \
	[ECU_LAMBDA_V] = "Lambda (fuel mix) (<1 => Rich)", \
	[ECU_LOAD] = "Load (motor resistance procentage)", \
	[ECU_INJECTOR_TIME] = "Injector Time", \
	[ECU_IGNITION_TIME] = "Ignition Time", \
	[ECU_DWELL_TIME] = "Dwell Time", \
	[ECU_GX] = "GX", \
	[ECU_GY] = "GY", \
	[ECU_GZ] = "GZ", \
	[ECU_MOTOR_FLAGS] = "Motor Flags", \
	[ECU_OUT_BITS] = "Out Bits", \
	[ECU_TIME] = "Time", \
}[ecu_id])


struct sensor {
	enum message_id id;
	float value;
};

void ecu_init(void);
void ecu_send_request(void);
bool ecu_has_packet(void);
bool ecu_read_data(struct sensor *data);

#endif /* ECU_H */
