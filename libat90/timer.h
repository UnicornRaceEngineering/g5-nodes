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
 * @file timer.h
 * @brief
 * Provides a simplified interface to the various available timers.
 */

#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <stdint.h>

/**
 * @name timer_prescalar
 * CSn2, CSn1 and CSn0 (Clock Select) sets the source used by the Timer/Counter.
 * @{
 */

/** For more information see the data-sheet page 111 */
enum timer0_prescalar_t {
	TIMER0_PRESCALAR_NO_SOURCE 	=		(0      |0      |0      ), //!< No clock source (Timer/Counter stopped)
	TIMER0_PRESCALAR_1 			=		(0      |0      |1<<CS00), //!< clkI/O/(No prescaling)
	TIMER0_PRESCALAR_8 			=		(0      |1<<CS01|0      ), //!< clkI/O/8 (From prescaler)
	TIMER0_PRESCALAR_64 		=		(0      |1<<CS01|1<<CS00), //!< clkI/O/64 (From prescaler)
	TIMER0_PRESCALAR_256 		=		(1<<CS02|0      |0      ), //!< clkI/O/256 (From prescaler)
	TIMER0_PRESCALAR_1024 		=		(1<<CS02|0      |1<<CS01), //!< clkI/O/1024 (From prescaler)

	TIMER0_PRESCALAR_EXTERNAL_FALLING = (1<<CS02|1<<CS01|0      ), //!< External clock source on T0 pin. Clock on falling edge.
	TIMER0_PRESCALAR_EXTERNAL_RISING  = (1<<CS02|1<<CS01|1<<CS00)  //!< External clock source on T0 pin. Clock on rising edge.
};

/** For more information see the data-sheet page 139 */
enum timer1_prescalar_t {
	TIMER1_PRESCALAR_NO_SOURCE 	=		(0      |0      |0      ), //!< No clock source (Timer/Counter stopped)
	TIMER1_PRESCALAR_1 			=		(0      |0      |1<<CS10), //!< clkI/O/(No prescaling)
	TIMER1_PRESCALAR_8 			=		(0      |1<<CS11|0      ), //!< clkI/O/8 (From prescaler)
	TIMER1_PRESCALAR_64 		=		(0      |1<<CS11|1<<CS10), //!< clkI/O/64 (From prescaler)
	TIMER1_PRESCALAR_256 		=		(1<<CS12|0      |0      ), //!< clkI/O/256 (From prescaler)
	TIMER1_PRESCALAR_1024 		=		(1<<CS12|0      |1<<CS11), //!< clkI/O/1024 (From prescaler)

	TIMER1_PRESCALAR_EXTERNAL_FALLING = (1<<CS12|1<<CS11|0      ), //!< External clock source on T1 pin. Clock on falling edge.
	TIMER1_PRESCALAR_EXTERNAL_RISING  = (1<<CS12|1<<CS11|1<<CS10)  //!< External clock source on T1 pin. Clock on rising edge.
};

/** For more information see the data-sheet page 159 */
enum timer2_prescalar_t {
	TIMER2_PRESCALAR_NO_SOURCE 	= (0      |0      |0      ), //!< No clock source (Timer/Counter stopped)
	TIMER2_PRESCALAR_1 			= (0      |0      |1<<CS20), //!< clkT2S/(No prescaling)
	TIMER2_PRESCALAR_8 			= (0      |1<<CS21|0      ), //!< clkT2S/8 (From prescaler)
	TIMER2_PRESCALAR_32 		= (0      |1<<CS21|1<<CS20), //!< clkT2S/32 (From prescaler)
	TIMER2_PRESCALAR_64 		= (1<<CS22|0      |0      ), //!< clkT2S/64 (From prescaler)
	TIMER2_PRESCALAR_128 		= (1<<CS22|0      |1<<CS20), //!< clkT2S/128 (From prescaler)
	TIMER2_PRESCALAR_256 		= (1<<CS22|1<<CS21|0      ), //!< clkT2S/256 (From prescaler)
	TIMER2_PRESCALAR_1024 		= (1<<CS22|1<<CS21|1<<CS20)  //!< clkT2S/1024 (From prescaler)
};

/** For more information see the data-sheet page 139 */
enum timer3_prescalar_t {
	TIMER3_PRESCALAR_NO_SOURCE 	=		(0      |0      |0      ), //!< No clock source (Timer/Counter stopped)
	TIMER3_PRESCALAR_1 			=		(0      |0      |1<<CS30), //!< clkI/O/(No prescaling)
	TIMER3_PRESCALAR_8 			=		(0      |1<<CS31|0      ), //!< clkI/O/8 (From prescaler)
	TIMER3_PRESCALAR_64 		=		(0      |1<<CS31|1<<CS30), //!< clkI/O/64 (From prescaler)
	TIMER3_PRESCALAR_256 		=		(1<<CS32|0      |0      ), //!< clkI/O/256 (From prescaler)
	TIMER3_PRESCALAR_1024 		=		(1<<CS32|0      |1<<CS31), //!< clkI/O/1024 (From prescaler)

	TIMER3_PRESCALAR_EXTERNAL_FALLING = (1<<CS32|1<<CS31|0      ), //!< External clock source on T3 pin. Clock on falling edge.
	TIMER3_PRESCALAR_EXTERNAL_RISING  = (1<<CS32|1<<CS31|1<<CS30)  //!< External clock source on T3 pin. Clock on rising edge.
};

/** @} */

/**
 * @name timer_waveform_generation_mode
 * @{
 */

/** See data-sheet page 110 table 12-1 */
enum timer0_waveform_generation_mode_t {
	TIMER0_WGM_NORMAL				= (0       |0       ), //!< TOP=0xFF,  Update of OCR0A at: Immediate, TOV0 Flag Set on: MAX
	TIMER0_WGM_PWM_PHASE_CORRECT 	= (0       |1<<WGM00), //!< TOP=0xFF,  Update of OCR0A at: TOP, 		TOV0 Flag Set on: BOTTON
	TIMER0_WGM_CTC					= (1<<WGM01|0       ), //!< TOP=OCR0A, Update of OCR0A at: Immediate, TOV0 Flag Set on: MAX
	TIMER0_WGM_FAST_PWM				= (1<<WGM01|1<<WGM00)  //!< TOP=0xFF,  Update of OCR0A at: TOP, 		TOV0 Flag Set on: MAX
};

/**
 * Because WGM11 and WGM10 is on TCCR1A while WGM13 and WGM12 is on TCCR1B we
 * will run into trouble. Instead we shift them and store them in a 16 bit value
 * where we have shifted WGM13 and WGM12 into the high bit so we later can
 * reverse the process and enter apply it.
 * @todo: enums are signed int by default in C. Will this cause problems?
 * See data-sheet page 138 table 13-4
 */
enum timer1_waveform_generation_mode_t {
	TIMER1_WGM_NORMAL 							= (0<<8    		|0<<8         |0       |0       ), //!< TOP=0xFFFF, Update of OCR1nx at: Immediate, TOVn Flag Set on: MAX

	TIMER1_WGM_PWM_PHASE_CORRECT_8BIT 			= (0<<8    		|0<<8         |0       |1<<WGM10), //!< TOP=0x00FF, Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM
	TIMER1_WGM_PWM_PHASE_CORRECT_9BIT 			= (0<<8    		|0<<8         |1<<WGM11|0       ), //!< TOP=0x01FF, Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM
	TIMER1_WGM_PWM_PHASE_CORRECT_10BIT 			= (0<<8    		|0<<8         |1<<WGM11|1<<WGM10), //!< TOP=0x03FF, Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM

	TIMER1_WGM_CTC_OCR 							= (0<<8    		|(1<<WGM12)<<8|0       |0       ), //!< TOP=OCRnA,  Update of OCRnx at: Immediate,  TOVn Flag Set on: MAX

	TIMER1_WGM_FAST_PWM_8BIT 					= (0<<8    		|(1<<WGM12)<<8|0       |1<<WGM10), //!< TOP=0x00FF, Update of OCRnx at: TOP,        TOVn Flag Set on: TOP
	TIMER1_WGM_FAST_PWM_9BIT 					= (0<<8         |(1<<WGM12)<<8|1<<WGM11|0       ), //!< TOP=0x01FF, Update of OCRnx at: TOP,        TOVn Flag Set on: TOP
	TIMER1_WGM_FAST_PWM_10BIT 					= (0<<8         |(1<<WGM12)<<8|1<<WGM11|1<<WGM10), //!< TOP=0x03FF, Update of OCRnx at: TOP,        TOVn Flag Set on: TOP

	TIMER1_WGM_PWM_PHASE_FREQUENCY_CORRECT_ICR 	= ((1<<WGM13)<<8|0<<8         |0       |0       ), //!< TOP=ICRn,   Update of OCRnx at: BOTTOM,     TOVn Flag Set on: BOTTOM
	TIMER1_WGM_PWM_PHASE_FREQUENCY_CORRECT_OCR 	= ((1<<WGM13)<<8|0<<8         |0       |1<<WGM10), //!< TOP=OCRnA,  Update of OCRnx at: BOTTOM,     TOVn Flag Set on: BOTTOM

	TIMER1_WGM_PWM_PHASE_CORRECT_ICR 			= ((1<<WGM13)<<8|0<<8         |1<<WGM11|0       ), //!< TOP=ICRn,   Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM
	TIMER1_WGM_PWM_PHASE_CORRECT_OCR 			= ((1<<WGM13)<<8|0<<8         |1<<WGM11|1<<WGM10), //!< TOP=OCRnA,  Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM

	TIMER1_WGM_CTC_ICR 							= ((1<<WGM13)<<8|(1<<WGM12)<<8|0       |0       ), //!< TOP=ICRn,   Update of OCRnx at: Immediate,  TOVn Flag Set on: MAX

	TIMER1_WGM_RESERVED 						= ((1<<WGM13)<<8|(1<<WGM12)<<8|0       |1<<WGM10), //!< @note reserved

	TIMER1_WGM_FAST_PWM_ICR 					= ((1<<WGM13)<<8|(1<<WGM12)<<8|1<<WGM11|0       ), //!< TOP=ICRn,   Update of OCRnx at: TOP,        TOVn Flag Set on: TOP
	TIMER1_WGM_FAST_PWM_OCR 					= ((1<<WGM13)<<8|(1<<WGM12)<<8|1<<WGM11|1<<WGM10)  //!< TOP=OCRnA,  Update of OCRnx at: TOP,        TOVn Flag Set on: TOP
};

/** See data-sheet page 158 table 14-1 */
enum timer2_waveform_generation_mode_t {
	TIMER2_WGM_NORMAL				= (0       |0       ), //!< TOP=0xFF,  Update of OCR2A at: Immediate, TOV0 Flag Set on: MAX
	TIMER2_WGM_PWM_PHASE_CORRECT 	= (0       |1<<WGM20), //!< TOP=0xFF,  Update of OCR2A at: TOP, 		TOV0 Flag Set on: BOTTON
	TIMER2_WGM_CTC					= (1<<WGM21|0       ), //!< TOP=OCR2A, Update of OCR2A at: Immediate, TOV0 Flag Set on: MAX
	TIMER2_WGM_FAST_PWM				= (1<<WGM21|1<<WGM20)  //!< TOP=0xFF,  Update of OCR2A at: TOP, 		TOV0 Flag Set on: MAX
};

/**
 * See timer1_waveform_generation_mode_t
 */
enum timer3_waveform_generation_mode_t {
	TIMER3_WGM_NORMAL 							= (0<<8         |0<<8         |0       |0       ), //!< TOP=0xFFFF, Update of OCR1nx at: Immediate, TOVn Flag Set on: MAX

	TIMER3_WGM_PWM_PHASE_CORRECT_8BIT 			= (0<<8         |0<<8         |0       |1<<WGM30), //!< TOP=0x00FF, Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM
	TIMER3_WGM_PWM_PHASE_CORRECT_9BIT 			= (0<<8         |0<<8         |1<<WGM31|0       ), //!< TOP=0x01FF, Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM
	TIMER3_WGM_PWM_PHASE_CORRECT_10BIT 			= (0<<8         |0<<8         |1<<WGM31|1<<WGM30), //!< TOP=0x03FF, Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM

	TIMER3_WGM_CTC_OCR 							= (0<<8         |(1<<WGM32)<<8|0       |0       ), //!< TOP=OCRnA,  Update of OCRnx at: Immediate,  TOVn Flag Set on: MAX

	TIMER3_WGM_FAST_PWM_8BIT 					= (0<<8         |(1<<WGM32)<<8|0       |1<<WGM30), //!< TOP=0x00FF, Update of OCRnx at: TOP,        TOVn Flag Set on: TOP
	TIMER3_WGM_FAST_PWM_9BIT 					= (0<<8         |(1<<WGM32)<<8|1<<WGM31|0       ), //!< TOP=0x01FF, Update of OCRnx at: TOP,        TOVn Flag Set on: TOP
	TIMER3_WGM_FAST_PWM_10BIT 					= (0<<8         |(1<<WGM32)<<8|1<<WGM31|1<<WGM30), //!< TOP=0x03FF, Update of OCRnx at: TOP,        TOVn Flag Set on: TOP

	TIMER3_WGM_PWM_PHASE_FREQUENCY_CORRECT_ICR 	= ((1<<WGM33)<<8|0<<8         |0       |0       ), //!< TOP=ICRn,   Update of OCRnx at: BOTTOM,     TOVn Flag Set on: BOTTOM
	TIMER3_WGM_PWM_PHASE_FREQUENCY_CORRECT_OCR 	= ((1<<WGM33)<<8|0<<8         |0       |1<<WGM30), //!< TOP=OCRnA,  Update of OCRnx at: BOTTOM,     TOVn Flag Set on: BOTTOM

	TIMER3_WGM_PWM_PHASE_CORRECT_ICR 			= ((1<<WGM33)<<8|0<<8         |1<<WGM31|0       ), //!< TOP=ICRn,   Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM
	TIMER3_WGM_PWM_PHASE_CORRECT_OCR 			= ((1<<WGM33)<<8|0<<8         |1<<WGM31|1<<WGM30), //!< TOP=OCRnA,  Update of OCRnx at: TOP,        TOVn Flag Set on: BOTTOM

	TIMER3_WGM_CTC_ICR 							= ((1<<WGM33)<<8|(1<<WGM32)<<8|0       |0       ), //!< TOP=ICRn,   Update of OCRnx at: Immediate,  TOVn Flag Set on: MAX

	TIMER3_WGM_RESERVED 						= ((1<<WGM33)<<8|(1<<WGM32)<<8|0       |1<<WGM30), //!< @note reserved

	TIMER3_WGM_FAST_PWM_ICR 					= ((1<<WGM33)<<8|(1<<WGM32)<<8|1<<WGM31|0       ), //!< TOP=ICRn,   Update of OCRnx at: TOP,        TOVn Flag Set on: TOP
	TIMER3_WGM_FAST_PWM_OCR 					= ((1<<WGM33)<<8|(1<<WGM32)<<8|1<<WGM31|1<<WGM30)  //!< TOP=OCRnA,  Update of OCRnx at: TOP,        TOVn Flag Set on: TOP
};

/** @} */

/**
 * todo: implement the same as above for COMnC0 and COMnC1 so we can set the
 * Compare Output Mode in a more high level way.
 */

void timer0_set_prescalar(uint8_t prescalar);
void timer1_set_prescalar(uint8_t prescalar);
void timer2_set_prescalar(uint8_t prescalar);
void timer3_set_prescalar(uint8_t prescalar);

void timer0_set_waveform_generation_mode(uint8_t wgm);
void timer1_set_waveform_generation_mode(uint16_t wgm);
void timer2_set_waveform_generation_mode(uint8_t wgm);
void timer3_set_waveform_generation_mode(uint16_t wgm);

#endif /* TIMER_H */
