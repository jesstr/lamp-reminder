/*
 * rgb.h
 *
 *  Created on: 16.09.2014
 *      Author: Pavel Cherstvov
 */

#ifndef RGB_H_
#define RGB_H_

#include "soft_timer.h"
#include "rgb.h"


#define RED_DDR 	DDRB
#define	BLUE_DDR 	DDRB
#define	GREEN_DDR 	DDRB

#define	RED_PORT	PORTB
#define	BLUE_PORT	PORTB
#define	GREEN_PORT	PORTB

#define	RED_PIN		1
#define	BLUE_PIN	2
#define	GREEN_PIN	3

/* Soft timers for soft PWM */
soft_timer_t red_timer;
soft_timer_t blue_timer;
soft_timer_t green_timer;

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


/* RGB led IO pins initialization */
void RGB_IO_Init();

#endif /* RGB_H_ */
