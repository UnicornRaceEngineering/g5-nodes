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
 * @file ecu.c
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

#include <stdbool.h>
#include <avr/interrupt.h>
#include <usart.h>
#include <timer.h>
#include "ecu.h"

#define ECU_BAUD	(19200)

#define ECU_HEARTBEAT_ISR_VECT	TIMER0_COMP_vect

#define HEARTBEAT_DELAY_LENGTH	8 // How many times we delay the heartbeat timer

#define ARR_LEN(x)  (sizeof(x) / sizeof(x[0]))


static uint32_t clamp(uint32_t value) {
	if(value > (1<<15)){
		value = -(0xFFFF - value);
	}
	return value;
}

//!< @TODO Move this into its own module as it doesn't belong here
static void send_xbee_array(const uint8_t id, const uint8_t *arr, uint16_t len) {
	usart1_putc_unbuffered(id);
	usart1_putc_unbuffered(HIGH_BYTE(len));
	usart1_putc_unbuffered(LOW_BYTE(len));

	for (int i = 0; i < len; ++i) {
		usart1_putc_unbuffered(arr[i]);
	}
}

void ecu_parse_package(void) {

	struct ecu_package {
		struct sensor sensor;
		uint32_t raw_value; // The raw data received from the ECU
		size_t length; 		// length of the data in bytes
	} pkt[] = {
#		include "ecu_package_layout.inc"
	};

	// We loop over the package and extract the number of bytes element contains
	for (int i = 0; i < ARR_LEN(pkt); ++i) {
		while (pkt[i].length--) {
			const uint8_t ecu_byte = usart0_getc_unbuffered();
			if (pkt[i].sensor.id == EMPTY ) continue;

			pkt[i].raw_value += (ecu_byte << (8 * pkt[i].length));
		}

		// Convert the raw data to usable data
		{
			switch (pkt[i].sensor.id) {
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
			case EMPTY:
			default:
				// No conversion
				pkt[i].sensor.value = (double)pkt[i].raw_value;
				break;
			}
		}

		// usart1_printf("%s: %d\n", pkt[i].sensor.name, (int)(pkt[i].sensor.value*1000));

		// Send the individual sensor data
		send_xbee_array(pkt[i].sensor.id, (uint8_t*)&(pkt[i].sensor.value),
			sizeof(pkt[i].sensor.value));
#if 0
		{
			const uint8_t *value_ptr = (uint8_t*)&(pkt[i].sensor.value);
			usart1_putc_unbuffered(pkt[i].sensor.id);
			usart1_putc_unbuffered(HIGH_BYTE((uint16_t)sizeof(pkt[i].sensor.value)));
			usart1_putc_unbuffered(LOW_BYTE((uint16_t)sizeof(pkt[i].sensor.value)));
			usart1_putc_unbuffered(value_ptr[0]);
			usart1_putc_unbuffered(value_ptr[1]);
			usart1_putc_unbuffered(value_ptr[2]);
			usart1_putc_unbuffered(value_ptr[3]);
		}
#endif
	}
}

void ecu_init(void) {
	usart0_init(ECU_BAUD);	// ECU

	// setup timer 0 which periodically sends heartbeat to the ECU
	{
		// 1/((F_CPU/Prescaler)/n_timersteps)
		// 1/((11059200/1024)/256) = approx 23.7 ms or about 42 Hz
		OCR0A = 100;					// Set start value
		TIMSK0 |= 1<<OCIE0A; 			// Enable timer compare match interrupt
		TCCR0A |= 1<<CS02 | 1<<CS00;    // Set prescaler 1024
	}
}


ISR(ECU_HEARTBEAT_ISR_VECT) {
	static int delay = HEARTBEAT_DELAY_LENGTH;

	// We have to send a start sequence to the ECU to force it respond with data
	// but if we ask too often it crashes
	if (!delay--) {
		const uint8_t heart_beat[] = {0x12,0x34,0x56,0x78,0x17,0x08,0,0,0,0};
		for (int i = 0; i < ARR_LEN(heart_beat); ++i) {
			usart0_putc_unbuffered(heart_beat[i]);
		}
		delay = HEARTBEAT_DELAY_LENGTH; // Reset the delay
	}

}

