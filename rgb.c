/*
 * rgb.c
 *
 *  Created on: 16.09.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include "rgb.h"


/* Uncomment this line to enable debug functions and extended output messages.
 * This macros is unique in each source file, which has debug functions, so it should
 * be commented/uncommented individually! */
//#define _DEBUG_	1

#ifdef _DEBUG_
#include "uart.h"
#endif

#define mod(x)		((x) < 0 ? 0 - (x) : (x))
#define max(x, y)	((x) > (y) ? (x) : (y))


/* Soft timers for soft PWM */
soft_timer_t red_timer = {0, 0, 127, 0, RGB_RedCompHndlr, RGB_RedOvfHndlr};
soft_timer_t blue_timer = {0, 0, 127, 0, RGB_BlueCompHndlr, RGB_BlueOvfHndlr};
soft_timer_t green_timer = {0, 0, 127, 0, RGB_GreenCompHndlr, RGB_GreenOvfHndlr};

/* Soft timers for dimmer */
soft_timer_t dimmer_timer = {0, 0, 0, 0, RGB_DimmerCompHndlr, RGB_DimmerOvfHndlr};

/* Color values stored in eeprom */
unsigned int _eemem_red_color EEMEM;
unsigned int _eemem_green_color EEMEM;
unsigned int _eemem_blue_color EEMEM;

/* Buffers */
unsigned int red_color;
unsigned int blue_color;
unsigned int green_color;

/*
struct {
	signed int r_target;
	signed int g_target;
	signed int b_target;

	signed int r_load;
	signed int g_load;
	signed int b_load;

	signed int r_step;
	signed int g_step;
	signed int b_step;

	unsigned char iter;
	unsigned char nsteps;
} dimmer;
*/
dimmer_t dimmer;

/* RGB led IO pins initialization */
void RGB_IO_Init(void) {

	RED_DDR |= (1 << RED_PIN);
	BLUE_DDR |= (1 << BLUE_PIN);
	GREEN_DDR |= (1 << GREEN_PIN); //defected pin

	RED_PORT &= ~(1 << RED_PIN);
	BLUE_PORT &= ~(1 << BLUE_PIN);
	GREEN_PORT &= ~(1 << GREEN_PIN); //defected pin

	SoftTimer_TimerReg(&red_timer);
	SoftTimer_TimerReg(&blue_timer);
	SoftTimer_TimerReg(&green_timer);

	RGB_TIMERS_START;

	//SoftTimer_TimerReg(&dimmer_timer);

	//SOFT_TIMER_START(dimmer_timer);

}

void RGB_SetColor(unsigned char r, unsigned char g, unsigned char b)
{
	red_timer.load = r;
	green_timer.load = g;
	blue_timer.load = b;
}

void RGB_RedOvfHndlr() {
	RED_PORT |= (1 << RED_PIN);
	/* Update register */
	red_timer.load = red_color;
}

void RGB_BlueOvfHndlr() {
	BLUE_PORT |= (1 << BLUE_PIN);
	/* Update register */
	blue_timer.load = blue_color;
}

void RGB_GreenOvfHndlr() {
	GREEN_PORT |= (1 << GREEN_PIN);
	/* Update register */
	green_timer.load = green_color;
}

void RGB_RedCompHndlr() {
	RED_PORT &= ~(1 << RED_PIN);
}

void RGB_BlueCompHndlr() {
	BLUE_PORT &= ~(1 << BLUE_PIN);
}

void RGB_GreenCompHndlr() {
	GREEN_PORT &= ~(1 << GREEN_PIN);
}

void RGB_DimmerCompHndlr() {

#if 0

	if (dimmer.r_load != dimmer.r_target )
		dimmer.r_load += (dimmer.r_step);
	if (dimmer.g_load != dimmer.g_target )
		dimmer.g_load += (dimmer.g_step);
	if (dimmer.b_load != dimmer.b_target )
		dimmer.b_load += (dimmer.b_step);

	/* moved to overflow routine */
	/*
	red_timer.load = dimmer.r_load / 10;
	green_timer.load = dimmer.g_load / 10;
	blue_timer.load = dimmer.b_load / 10;
	*/

	if (dimmer.iter == dimmer.nsteps ) {
		//red_timer.load = dimmer.r_target / 10;
		//green_timer.load = dimmer.g_target / 10;
		//blue_timer.load = dimmer.b_target / 10;
		SOFT_TIMER_STOP(dimmer_timer);
		dimmer.iter = 0;
		#ifdef _DEBUG_
		UART_SendString("stop\n\r");
		#endif
	}
	else {
		dimmer.iter++;
	}

#endif

}

void RGB_DimmerOvfHndlr() {
	/*
	red_timer.load = (red_timer.load < dimmer.r_target ?
			: red_timer.load + dimmer.r_step, red_timer.load - dimmer.r_step);
	green_timer.load = (green_timer.load < dimmer.g_target ?
			: green_timer.load + dimmer.g_step, green_timer.load - dimmer.g_step);
	blue_timer.load = (blue_timer.load < dimmer.b_target ?
			: blue_timer.load + dimmer.b_step, blue_timer.load - dimmer.b_step);
	*/

	/* Update RGB timers load */
	red_timer.load = dimmer.r_load / 10;
	green_timer.load = dimmer.g_load / 10;
	blue_timer.load = dimmer.b_load / 10;

}

/* Save current color to eeprom memory*/
inline void RGB_EememSaveColor(void) {
	eeprom_write_word(&_eemem_red_color, red_timer.load);
	eeprom_write_word(&_eemem_green_color, green_timer.load);
	eeprom_write_word(&_eemem_blue_color, blue_timer.load);
}

/* Restore saved color from eeprom memory*/
inline void RGB_EememRestoreColor(void) {
	red_timer.load = eeprom_read_word(&_eemem_red_color);
	green_timer.load = eeprom_read_word(&_eemem_green_color);
	blue_timer.load = eeprom_read_word(&_eemem_blue_color);
}

/* 33 ms - 8 sec (8415 ms) */
void RGB_ShortDimColor(signed int r, signed int g, signed int b, unsigned int ms) {
	signed int delay;


	if (dimmer_timer.is_running) {
		SOFT_TIMER_STOP(dimmer_timer);
		dimmer.iter = 0;
		#ifdef _DEBUG_
		UART_SendString("stop\n\r");
		#endif
	}

	delay = ms < 16 ? 1 : (ms / 16);
	delay = ms > 8415 ? 256 : delay;

	dimmer.r_target = r * 10;
	dimmer.g_target = g * 10;
	dimmer.b_target = b *10;

	dimmer.r_load = red_timer.load * 10;
	dimmer.g_load = green_timer.load * 10;
	dimmer.b_load = blue_timer.load * 10;

	signed char sign = r  < red_timer.load ? -1 : 1;
/*
	dimmer.r_step = (signed int)((signed int)(dimmer.r_target - (signed int)dimmer.r_load)) / delay + sign;
	dimmer.g_step = (signed int)((signed int)(dimmer.g_target - (signed int)dimmer.g_load)) / delay + sign;
	dimmer.b_step = (signed int)((signed int)(dimmer.b_target - (signed int)dimmer.b_load)) / delay + sign;
*/
	dimmer.r_step = (signed int)((signed int)(dimmer.r_target - (signed int)dimmer.r_load)) / delay;
	dimmer.g_step = (signed int)((signed int)(dimmer.g_target - (signed int)dimmer.g_load)) / delay;
	dimmer.b_step = (signed int)((signed int)(dimmer.b_target - (signed int)dimmer.b_load)) / delay;

	dimmer.nsteps = max((dimmer.r_target - dimmer.r_load) / dimmer.r_step,
					max((dimmer.g_target - dimmer.g_load) / dimmer.g_step,
						(dimmer.b_target - dimmer.b_load) / dimmer.b_step));

	#ifdef _DEBUG_
	char buf[4];

	UART_SendString(itoa(delay, buf, 10));
	UART_SendString(" delay\n\r");

	UART_SendString(itoa(dimmer.r_step, buf, 10));
	UART_SendString(" r_step\n\r");

	UART_SendString(itoa(dimmer.g_step, buf, 10));
	UART_SendString(" g_step\n\r");

	UART_SendString(itoa(dimmer.b_step, buf, 10));
	UART_SendString(" b_step\n\r");

	UART_SendString(itoa(dimmer.nsteps, buf, 10));
	UART_SendString(" nsteps\n\r");
	#endif

	dimmer_timer.load = delay - 1;
	dimmer_timer.top = delay;

	//SOFT_TIMER_START(dimmer_timer);
	dimmer.enabled = 1;

}

/* 1 min - 1 year (4294967296) */
void RGB_LongDimColor(unsigned char r, unsigned char g, unsigned char b, unsigned int s) {

}

