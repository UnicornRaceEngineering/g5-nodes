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

#include <stdbool.h>
#include <avr/interrupt.h>
#include <usart.h>
#include <timer.h>
#include "ecu.h"

#define ECU_BAUD	(19200)

#define ECU_TIMER				0
#define ECU_REQUEST_ISR_VECT	TIMER0_COMP_vect


#define ARR_LEN(x)  (sizeof(x) / sizeof(x[0]))


static uint32_t clamp(uint32_t value) {
	if(value > (1<<15)){
		value = -(0xFFFF - value);
	}
	return value;
}

void ecu_parse_package(void) {

	struct ecu_package {
		struct sensor sensor;
		uint32_t raw_value; // The raw data received from the ECU
		size_t length; // length of the data in bytes
	} pkt[] = {
#		include "ecu_package_layout.inc"
	};

	for (int i = 0; i < ARR_LEN(pkt); ++i) {
		while (pkt[i].length--) {
			const uint8_t ecu_byte = usart0_getc();
			if (pkt[i].sensor.id == EMPTY ) continue;

			pkt[i].raw_value += (ecu_byte << (8 * pkt[i].length));
		}

		// Convert the raw data to usable data
		{
			switch (pkt[i].sensor.id) {
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
				// No conversion
				pkt[i].sensor.value = (double)pkt[i].raw_value;
				break;
			case STATUS_LAMBDA_V2:
				pkt[i].sensor.value = (70-clamp(pkt[i].raw_value)/64.0);
				break;
			case WATER_TEMP:
			case MANIFOLD_AIR_TEMP:
				pkt[i].sensor.value = (pkt[i].raw_value * (-150.0/3840) + 120);
				break;
			case POTMETER:
				pkt[i].sensor.value = ((pkt[i].raw_value-336)/26.9);
				break;
			case RPM:
				pkt[i].sensor.value = (pkt[i].raw_value*0.9408);
				break;
			case MAP_SENSOR:
				pkt[i].sensor.value = (pkt[i].raw_value*0.75);
				break;
			case BATTERY_V:
				pkt[i].sensor.value = (pkt[i].raw_value*(1.0/210)+0);
				break;
			case LAMBDA_V:
				pkt[i].sensor.value = ((70-clamp(pkt[i].raw_value)/64.0)/100);
				break;
			case INJECTOR_TIME:
			case IGNITION_TIME:
				pkt[i].sensor.value = (-0.75*pkt[i].raw_value+120);
				break;
			case GX:
			case GY:
			case GZ:
				pkt[i].sensor.value = (clamp(pkt[i].raw_value) * (1.0/16384));
				break;

			case EMPTY:
			default:
				// Do nothing as this should never be called
				pkt[i].sensor.value = (double)pkt[i].raw_value;
				break;
			}
		}

		usart1_printf("%s: %d\n", pkt[i].sensor.name, (int)(pkt[i].sensor.value*1000));
	}
}

void ecu_init(void) {
	usart0_init(ECU_BAUD);	// ECU

	// init_ECU_request_timer
	{
		// 1/((F_CPU/Prescaler)/n_timersteps)
		// 1/((11059200/1024)/256) = approx 23.7 ms or about 42 Hz
		OCR0A = 100;					// Set start value
		TIMSK0 |= 1<<OCIE0A; 			// Enable timer compare match interrupt
		TCCR0A |= 1<<CS02 | 1<<CS00;    // Set prescaler 1024
	}
}

ISR(ECU_REQUEST_ISR_VECT) {
	// We have to send a start sequence to the ECU to force it respond with data
	// but if we ask too often it crashes
	const uint8_t start_seq[] = {0x12,0x34,0x56,0x78,0x17,0x08,0,0,0,0};
	for (int i = 0; i < ARR_LEN(start_seq); ++i) {
		usart0_putc_unbuffered(start_seq[i]);
	}

}

