
#include <stdint.h>
#include "converter.h"

static uint16_t clamp(uint16_t value) {
	if(value > (1<<15)){
		value = -(0xFFFF - value);
	}
	return value;
}

double convert(enum id id, uint16_t raw_value) {
	double value;

	switch (id) {
	case FUEL_PRESSURE:
	case STATUS_LAP_COUNT:
	case STATUS_INJ_SUM:
	case LAST_GEAR_SHIFT:
	case MOTOR_OILTEMP:
	case OIL_PRESSURE:
	case STATUS_TIME:
	case STATUS_LAP_TIME:
	case GEAR_OIL_TEMP:
	case STATUS_TRACTION:
	case STATUS_GAS:
	case STATUS_CAM_TRIG_P1:
	case STATUS_CAM_TRIG_P2:
	case STATUS_CHOKER_ADD:
	case STATUS_LAMBDA_PWM:
	case TRIGGER_ERR:
	case CAM_ANGLE1:
	case CAM_ANGLE2:
	case ROAD_SPEED:
	case LOAD:
	case DWELL_TIME:
	case MOTOR_FLAGS:
	case OUT_BITS:
	case TIME:
		value = (raw_value);
		break;
	case STATUS_LAMBDA_V2:
		value = (70-clamp(raw_value)/64.0);
		break;
	case WATER_TEMP:
	case MANIFOLD_AIR_TEMP:
		value = (raw_value * (-150.0/3840) + 120);
		break;
	case POTMETER:
		value = ((raw_value-336)/26.9);
		break;
	case RPM:
		value = (raw_value*0.9408);
		break;
	case MAP_SENSOR:
		value = (raw_value*0.75);
		break;
	case BATTERY_V:
		value = (raw_value*(1.0/210)+0);
		break;
	case LAMBDA_V:
		value = ((70-clamp(raw_value)/64.0)/100);
		break;
	case INJECTOR_TIME:
	case IGNITION_TIME:
		value = (-0.75*raw_value+120);
		break;
	case GX:
	case GY:
	case GZ:
		value = (clamp(raw_value) * (1.0/16384));
		break;

	case EMPTY:
	default:
		// Do nothing
		value = (double)raw_value;
		break;
	}

	return value;
}

