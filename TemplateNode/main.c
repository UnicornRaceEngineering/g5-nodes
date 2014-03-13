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
#include <stdio.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include <can.h>
#include <usart.h>
#include <adc.h>
#include <io.h>

static void rx_complete(uint8_t mob);
static void tx_complete(uint8_t mob);
static void can_default(uint8_t mob);


int main(void)
{
	set_canit_callback(CANIT_RX_COMPLETED, rx_complete);
	set_canit_callback(CANIT_TX_COMPLETED, tx_complete);
	set_canit_callback(CANIT_DEFAULT, can_default);

	//Initialise the Gear node
	usart1_init(115200);

	can_init();

	CAN_SEI();
	CAN_EN_RX_INT();
	CAN_EN_TX_INT();

	sei();										//Enable interrupt

	adc_setChannel(1);
	adc_setVref(INTERNAL);
	ADC_ENABLE();
	adc_setPrescaler(ADC_PRESCALAR_128);

	usart1_printf("\n\n\nSTARTING\n");


	can_msg_t rx_msg = {
		.mob = 8,
		.id = 4, // 133 = gpsnode
		.dlc = 7,

		.mode = MOB_RECIEVE
	};
	can_setup(&rx_msg);

	SET_PIN_MODE(PORTF, PIN0, OUTPUT);
	SET_PIN_MODE(PORTF, PIN1, OUTPUT);
	SET_PIN_MODE(PORTF, PIN2, OUTPUT);
	SET_PIN_MODE(PORTF, PIN3, OUTPUT);
	SET_PIN_MODE(PORTF, PIN4, OUTPUT);
	SET_PIN_MODE(PORTF, PIN5, OUTPUT);
	SET_PIN_MODE(PORTF, PIN6, OUTPUT);
	SET_PIN_MODE(PORTF, PIN7, OUTPUT);

	DIGITAL_WRITE(PORTF, PIN0, HIGH); 	// takes 6 instruktions
	IO_SET_HIGH(PORTF, PIN1); 			// takes 1 instruktion
	DIGITAL_WRITE(PORTF, PIN2, HIGH);
	DIGITAL_WRITE(PORTF, PIN3, HIGH);
	DIGITAL_WRITE(PORTF, PIN4, HIGH);
	DIGITAL_WRITE(PORTF, PIN5, HIGH);
	DIGITAL_WRITE(PORTF, PIN6, HIGH);
	IO_SET_HIGH(PORTF, PIN7);

	int i=0;
	while(1){
		// Main work loop
		_delay_ms(250);
		can_msg_t tx_msg = {
			.mob = 10,
			.id = 4,
			.data = {'H', 'E', 'Y', ' ', 'J', 'O', 'E'},
			.dlc = 7,
			.mode = MOB_TRANSMIT
		};
		can_send(&tx_msg);
		usart1_printf("HEJ %d\n", i);


		//uint16_t res = 3;adc_readChannel(i);
		//uart1_printf("ADC channel %d = %d \n", i, res);

		DIGITAL_TOGGLE(PORTF, i); // This expands to 6 instruktions

		if(++i == 8){
			i = 0;
			usart1_putc('\n');
		}
	}

    return 0;
}

static void rx_complete(uint8_t mob) {
	can_msg_t msg = {
		.mob = mob
	};
	can_receive(&msg);
	usart1_printf("Received id: %d on mob %d :: ", msg.id, msg.mob);
	usart1_putn(msg.dlc, msg.data);
	// The below should be used for binary data
	/* int i;
	for (i = 0; i < msg.dlc; ++i){
		uart1_printf("%d ", msg.data[i]);
		//uart1_printf("0x%02X ", msg.data[i]);
	} */
	usart1_printf("\n\n");
}

static void tx_complete(uint8_t mob) {
	MOB_ABORT();					// Freed the MOB
	MOB_CLEAR_INT_STATUS();			// and reset MOb status
	CAN_DISABLE_MOB_INTERRUPT(mob);	// Unset interrupt
}

static void can_default(uint8_t mob) {
	MOB_CLEAR_INT_STATUS(); 		// and reset MOb status
}
