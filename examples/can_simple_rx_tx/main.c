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

#include <stdint.h>
#include <util/delay.h>

#include <can.h>
#include <usart.h>
#include <bitwise.h>

static void rx_complete(uint8_t mob);
static void tx_complete(uint8_t mob);
static void can_default(uint8_t mob);


int main(void) {
	set_canit_callback(CANIT_RX_COMPLETED, rx_complete);
	set_canit_callback(CANIT_TX_COMPLETED, tx_complete);
	set_canit_callback(CANIT_DEFAULT, can_default);

	usart1_init(115200);

	can_init();

	//CAN_SEI();
	//CAN_EN_RX_INT();
	//CAN_EN_TX_INT();
	can_init();
	CAN_INIT_ALL();

	sei();										//Enable interrupt

	usart1_printf("\n\n\nSTARTING\n");

	// Set MOB 8 to listen for messeges with id 4 and length 7
	can_msg_t rx_msg = {
		.mob = 8,
		.id = 4,
		.dlc = 7,

		.mode = MOB_RECIEVE
	};
	can_setup(&rx_msg);


	usart1_printf("Listning for id %d on mob %d with a msg length %d\n",
		rx_msg.id,
		rx_msg.mob,
		rx_msg.dlc
	);


	while(1){
		// Main work loop
		_delay_ms(250);

#if 0
		// send a message with id 4 on MOB 10
		can_msg_t tx_msg = {
			.mob = 10,
			.id = 4,
			.data = {'H', 'E', 'L', 'L', 'O', '!', '!'},
			.dlc = 7,
			.mode = MOB_TRANSMIT
		};

		can_send(&tx_msg);
		usart1_printf("CAN Tx\t id %d, mob %d, :: ", tx_msg.id, tx_msg.mob);

		// As tx_msg.data is a byte array we cant treat it as a string
		usart1_putn(sizeof(tx_msg.data)/sizeof(tx_msg.data[0]), tx_msg.data);
		usart1_putc('\n');
#endif
	}

    return 0;
}

// Callback to be run when rx comletes on the CAN
static void rx_complete(uint8_t mob) {
	usart1_printf("double: %d float: %d\n", sizeof(double), sizeof(float));
	can_msg_t msg = {
		.mob = mob // mob is the MOB that fired the interrupt
	};
	can_receive(&msg); // Fetch the message and fill out the msg struct

#if 1
	// Print out the received data. Please dont print inside can callbacks
	// in real code as these are run inside the can ISR
	usart1_printf("CAN Rx\t id: %d on mob %d :: ", msg.id, msg.mob);
	usart1_putn(msg.dlc, msg.data); usart1_putc('\n');
#endif

#if 0
	// Print data from GPSNode. You might want to insert a delay in the GPS node
	// or we might not pick up the second msg with the longitude data as we are
	// busy printing the latitude data.
	if (msg.data[0] == 1) {
		float lat_dd;
		uint8_t *lat_ddptr = (uint8_t*)&lat_dd;
		*(lat_ddptr + 0) = msg.data[1];
		*(lat_ddptr + 1) = msg.data[2];
		*(lat_ddptr + 2) = msg.data[3];
		*(lat_ddptr + 3) = msg.data[4];

		int valid = msg.data[5];
		usart1_printf("Validity: %d\n", valid);
		usart1_printf("lat: %lf\n", lat_dd);
	} else if(msg.data[0] == 2) {
		float lon_dd;
		uint8_t *lon_ddptr = (uint8_t*)&lon_dd;
		*(lon_ddptr + 0) = msg.data[1];
		*(lon_ddptr + 1) = msg.data[2];
		*(lon_ddptr + 2) = msg.data[3];
		*(lon_ddptr + 3) = msg.data[4];

		int speed =  MERGE_BYTE(msg.data[5], msg.data[6]);
		usart1_printf("lon: %lf\n", lon_dd);
		usart1_printf("Speed = %d\n", speed);
	} else {
		usart1_printf("ERROR: data[0] = %d\n", msg.data[0]);
	}
#endif
}

static void tx_complete(uint8_t mob) {
	// We clear the mob so it can be used again
	MOB_ABORT();					// Freed the MOB
	MOB_CLEAR_INT_STATUS();			// and reset MOB status
	CAN_DISABLE_MOB_INTERRUPT(mob);	// Unset interrupt
}

static void can_default(uint8_t mob) {
	MOB_CLEAR_INT_STATUS(); 		// reset MOB status
}
