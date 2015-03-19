/**
 * @file ecu_package_layout.h
 * The ECU spits out data in the following format. The important values here is
 * the .id that signifies what kind of sensor data we receive, and the .length
 * which is how many bytes of that sensor type we should exspect.
 * EMPTY packages are just that, empty, The ECU will put out the specified
 * length of garbage.
 *
 * @note
 * This file is *ONLY* supposed to be included when declaring an ecu_package
 * array.
 * <code>
 * 	struct ecu_package pkt[] = {
 *	#	include "ecu_package_layout.h"
 *	};
 * </code>
 */

{.sensor = {.id = FUEL_PRESSURE,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = STATUS_LAP_COUNT,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.id = STATUS_INJ_SUM,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.id = LAST_GEAR_SHIFT,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.id = MOTOR_OILTEMP,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = OIL_PRESSURE,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = STATUS_TIME,}, .raw_value = 0, 			.length = 4,},
{.sensor = {.id = STATUS_LAP_TIME,}, .raw_value = 0, 		.length = 4,},
{.sensor = {.id = GEAR_OIL_TEMP,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = STATUS_TRACTION,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.id = STATUS_GAS,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = STATUS_LAMBDA_V2,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.id = STATUS_CAM_TRIG_P1,}, .raw_value = 0, 	.length = 2,},
{.sensor = {.id = STATUS_CAM_TRIG_P2,}, .raw_value = 0, 	.length = 2,},
{.sensor = {.id = STATUS_CHOKER_ADD,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.id = STATUS_LAMBDA_PWM,}, .raw_value = 0, 		.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 					.length = 10,},

{.sensor = {.id = WATER_TEMP,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = MANIFOLD_AIR_TEMP,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.id = POTMETER,}, .raw_value = 0, 				.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 					.length = 2,},

{.sensor = {.id = RPM,}, .raw_value = 0, 					.length = 2,},
{.sensor = {.id = TRIGGER_ERR,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = CAM_ANGLE1,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = CAM_ANGLE2,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = ROAD_SPEED,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = MAP_SENSOR,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = BATTERY_V,}, .raw_value = 0, 				.length = 2,},
{.sensor = {.id = LAMBDA_V,}, .raw_value = 0, 				.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 					.length = 4,},

{.sensor = {.id = LOAD,}, .raw_value = 0, 					.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 					.length = 2,},

{.sensor = {.id = INJECTOR_TIME,}, .raw_value = 0, 			.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 					.length = 2,},

{.sensor = {.id = IGNITION_TIME,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.id = DWELL_TIME,}, .raw_value = 0, 			.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 					.length = 10,},

{.sensor = {.id = GX,}, .raw_value = 0, 					.length = 2,},
{.sensor = {.id = GY,}, .raw_value = 0, 					.length = 2,},
{.sensor = {.id = GZ,}, .raw_value = 0, 					.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 					.length = 9,},

{.sensor = {.id = MOTOR_FLAGS,}, .raw_value = 0, 			.length = 1,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 					.length = 1,},

{.sensor = {.id = OUT_BITS,}, .raw_value = 0, 				.length = 1,},
{.sensor = {.id = TIME,}, .raw_value = 0, 					.length = 1,},

