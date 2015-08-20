/*
 * rgb.h
 *
 *  Created on: 16.09.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#ifndef RGB_H_
#define RGB_H_

#include "soft_timer.h"
#include "rgb.h"


#define RED_DDR 	DDRD
#define	BLUE_DDR 	DDRC
#define	GREEN_DDR 	DDRC

#define	RED_PORT	PORTD
#define	BLUE_PORT	PORTC
#define	GREEN_PORT	PORTC

#define	RED_PIN		5
#define	BLUE_PIN	1
#define	GREEN_PIN	0

/* Soft timers for soft PWM */
extern soft_timer_t red_timer, blue_timer, green_timer, dimmer_timer;
//soft_timer_t blue_timer;
//soft_timer_t green_timer;

/* Buffers */
extern unsigned int red_color;
extern unsigned int blue_color;
extern unsigned int green_color;

typedef struct {
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

	unsigned char enabled;
} dimmer_t;

extern dimmer_t dimmer;


#if 0
/* RED led timer */
#define RED_TIMER_RESET			red_timer.counter = 0

#define RED_TIMER_START 		do {						\
								RED_TIMER_RESET;			\
								red_timer.is_running = 1;			\
								} while(0)

#define RED_TIMER_STOP			do {						\
								RED_TIMER_RESET;			\
								red_timer.is_running = 0;			\
								} while(0)

/* BLUE led timer */
#define BLUE_TIMER_RESET		blue_timer.counter = 0

#define BLUE_TIMER_START 		do {						\
								BLUE_TIMER_RESET;			\
								blue_timer.is_running = 1;		\
								} while(0)

#define BLUE_TIMER_STOP			do {						\
								BLUE_TIMER_RESET;			\
								blue_timer.is_running = 0;		\
								} while(0)

/* GREEN led timer */
#define GREEN_TIMER_RESET		green_timer.counter = 0

#define GREEN_TIMER_START 		do {						\
								GREEN_TIMER_RESET;			\
								green_timer.is_running = 1;		\
								} while(0)

#define GREEN_TIMER_STOP		do {						\
								GREEN_TIMER_RESET;			\
								green_timer.is_running = 0;		\
								} while(0)


/* Start group of timers */
#define RGB_TIMERS_START		do {						\
								RED_TIMER_START;			\
								BLUE_TIMER_START;			\
								GREEN_TIMER_START;			\
								} while(0)

/* Stop group of timers */
#define RGB_TIMERS_STOP			do {						\
								RED_TIMER_STOP;				\
								BLUE_TIMER_STOP;			\
								GREEN_TIMER_STOP;			\
								} while(0)
#endif

/* Start group of timers */
#define RGB_TIMERS_START		do {							\
								SOFT_TIMER_START(red_timer);	\
								SOFT_TIMER_START(blue_timer);	\
								SOFT_TIMER_START(green_timer);	\
								} while(0)

/* Stop group of timers */
#define RGB_TIMERS_STOP			do {							\
								SOFT_TIMER_STOP(red_timer);		\
								SOFT_TIMER_STOP(blue_timer);	\
								SOFT_TIMER_STOP(green_timer);	\
								} while(0)

/* RGB led IO pins initialization */
void RGB_IO_Init();
void RGB_SetColor(unsigned char r, unsigned char g, unsigned char b);

void RGB_RedOvfHndlr();
void RGB_BlueOvfHndlr();
void RGB_GreenOvfHndlr();

void RGB_RedCompHndlr();
void RGB_BlueCompHndlr();
void RGB_GreenCompHndlr();

void RGB_DimmerCompHndlr();
void RGB_DimmerOvfHndlr();

void RGB_EememSaveColor(void);
void RGB_EememRestoreColor(void);

void RGB_DimColor(unsigned char r, unsigned char g, unsigned char b, unsigned int ms);
/* 33 ms - 8 sec (8415 ms) */
void RGB_ShortDimColor(signed int r, signed int g, signed int b, unsigned int ms);
/* 1 min - 1 year (4294967296) */
void RGB_LongDimColor(unsigned char r, unsigned char g, unsigned char b, unsigned int s);


#endif /* RGB_H_ */
