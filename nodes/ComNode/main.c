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

#include <avr/interrupt.h> // sei()
#include <avr/pgmspace.h>
#include <m41t81s_rtc.h>           // for rtc_init
#include <stdio.h>                 // for puts_p
#include <sysclock.h>              // for sysclock_init
#include <util/delay.h>
#include <usart.h>

#include "ecu.h"  // for ecu_init, ecu_parse_package
#include "xbee.h"                  // for xbee_init, xbee_send
#include "log.h"
#include "protocol.h"


static void set_msg_transport_rules(void);


static void init(void) {
	rtc_init();
	sysclock_init();
	ecu_init();
	xbee_init();
	log_init();
	set_msg_transport_rules();

	sei();
}

int main(void) {
	init();
	event_loop();

	return 0;
}


static void set_msg_transport_rules(void) {
	set_msg_transport(ECU_MOTOR_OILTEMP    , XBEE + SD);
	set_msg_transport(ECU_OIL_PRESSURE     , XBEE + SD);
	set_msg_transport(ECU_STATUS_CHOKER_ADD, XBEE + SD);
	set_msg_transport(ECU_WATER_TEMP       , XBEE + SD);
	set_msg_transport(ECU_MANIFOLD_AIR_TEMP,        SD);
	set_msg_transport(ECU_SPEEDER_POTMETER , XBEE + SD);
	set_msg_transport(ECU_RPM              , XBEE + SD);
	set_msg_transport(ECU_TRIGGER_ERR      ,        SD);
	set_msg_transport(ECU_CAM_ANGLE1       ,        SD);
	set_msg_transport(ECU_CAM_ANGLE2       ,        SD);
	set_msg_transport(ECU_MAP_SENSOR       ,        SD);
	set_msg_transport(ECU_BATTERY_V        , XBEE + SD);
	set_msg_transport(ECU_LAMBDA_V         , XBEE + SD);
	set_msg_transport(ECU_LOAD             ,        SD);
	set_msg_transport(ECU_INJECTOR_TIME    ,        SD);
	set_msg_transport(ECU_IGNITION_TIME    ,        SD);
	set_msg_transport(ECU_GX               ,        SD);
	set_msg_transport(ECU_GY               ,        SD);
	set_msg_transport(ECU_GZ               ,        SD);
}
