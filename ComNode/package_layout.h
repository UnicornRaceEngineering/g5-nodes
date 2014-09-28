/**
 * The ECU spits out data in the following format. The important values here is
 * the .id that signifies what kind of sensor data we receive, and the .length
 * which is how many bytes of that sensor type we should exspect.
 * EMPTY packages are just that, empty, The ECU will put out the specified
 * length of garbage.
 */

{.sensor = {.name = "Fuel Press.", 				.id = FUEL_PRESSURE,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Status Lap Counter", 		.id = STATUS_LAP_COUNT,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.name = "Status Injection Sum", 	.id = STATUS_INJ_SUM,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.name = "Last Gear Shift", 			.id = LAST_GEAR_SHIFT,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.name = "Motor Oil Temp", 			.id = MOTOR_OILTEMP,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Oil Pressure", 			.id = OIL_PRESSURE,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Status Time", 				.id = STATUS_TIME,}, .raw_value = 0, 			.length = 4,},
{.sensor = {.name = "Status Lap Time", 			.id = STATUS_LAP_TIME,}, .raw_value = 0, 		.length = 4,},
{.sensor = {.name = "Gear Oil Temp", 			.id = GEAR_OIL_TEMP,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Status Traction", 			.id = STATUS_TRACTION,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.name = "Status Gas", 				.id = STATUS_GAS,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Status LambdaV2", 			.id = STATUS_LAMBDA_V2,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.name = "Status Cam Trig P1", 		.id = STATUS_CAM_TRIG_P1,}, .raw_value = 0, 	.length = 2,},
{.sensor = {.name = "Status Cam Trig P2", 		.id = STATUS_CAM_TRIG_P2,}, .raw_value = 0, 	.length = 2,},
{.sensor = {.name = "Status Choker Add", 		.id = STATUS_CHOKER_ADD,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.name = "Status Lambda PWM", 		.id = STATUS_LAMBDA_PWM,}, .raw_value = 0, 		.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 														.length = 10,},

{.sensor = {.name = "WaterMotor temp", 			.id = WATER_TEMP,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "ManifoldAir temp", 		.id = MANIFOLD_AIR_TEMP,}, .raw_value = 0, 		.length = 2,},
{.sensor = {.name = "Potmeter (0-100%)", 		.id = POTMETER,}, .raw_value = 0, 				.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 														.length = 2,},

{.sensor = {.name = "RPM", 						.id = RPM,}, .raw_value = 0, 					.length = 2,},
{.sensor = {.name = "Trigger Err", 				.id = TRIGGER_ERR,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Cam Angle1", 				.id = CAM_ANGLE1,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Cam Angle2", 				.id = CAM_ANGLE2,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "RoadSpeed (km/h)", 		.id = ROAD_SPEED,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Manifold press. (mBar)", 	.id = MAP_SENSOR,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Batt. volt", 				.id = BATTERY_V,}, .raw_value = 0, 				.length = 2,},
{.sensor = {.name = "Lambda (<1 => Rich)", 		.id = LAMBDA_V,}, .raw_value = 0, 				.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 														.length = 4,},

{.sensor = {.name = "Load", 					.id = LOAD,}, .raw_value = 0, 					.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 														.length = 2,},

{.sensor = {.name = "Injector Time", 			.id = INJECTOR_TIME,}, .raw_value = 0, 			.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 														.length = 2,},

{.sensor = {.name = "Ignition Time", 			.id = IGNITION_TIME,}, .raw_value = 0, 			.length = 2,},
{.sensor = {.name = "Dwell Time", 				.id = DWELL_TIME,}, .raw_value = 0, 			.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 														.length = 10,},

{.sensor = {.name = "GX", 						.id = GX,}, .raw_value = 0, 					.length = 2,},
{.sensor = {.name = "GY", 						.id = GY,}, .raw_value = 0, 					.length = 2,},
{.sensor = {.name = "GZ", 						.id = GZ,}, .raw_value = 0, 					.length = 2,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 														.length = 9,},

{.sensor = {.name = "Motor Flags", 				.id = MOTOR_FLAGS,}, .raw_value = 0, 			.length = 1,},

{.sensor = {.id = EMPTY,}, .raw_value = 0, 														.length = 1,},

{.sensor = {.name = "Out Bits", 				.id = OUT_BITS,}, .raw_value = 0, 				.length = 1,},
{.sensor = {.name = "Time", 					.id = TIME,}, .raw_value = 0, 					.length = 1,},

