/*
 * soft_timer.h
 *
 *  Created on: 24.04.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#ifndef SOFT_TIMER_H_
#define SOFT_TIMER_H_


#define MAX_SOFT_TIMERS_COUNT	4

/* RED led timer */
#define SOFT_TIMER_RESET(soft_timer)	soft_timer.counter = 0

#define SOFT_TIMER_START(soft_timer) 	do {							\
										SOFT_TIMER_RESET(soft_timer);	\
										soft_timer.is_running = 1;		\
										} while(0)

#define SOFT_TIMER_STOP(soft_timer)		do {							\
										SOFT_TIMER_RESET(soft_timer);	\
										soft_timer.is_running = 0;		\
										} while(0)


typedef struct {
	unsigned int counter;
	unsigned int load;
	unsigned int top;
	unsigned char is_running;
	void (*compare_handler)();
	void (*overflow_handler)();
} soft_timer_t;

soft_timer_t *soft_timers_queue[MAX_SOFT_TIMERS_COUNT];

unsigned char soft_timers_count;


void SoftTimer_TimerReg(soft_timer_t *timer);


#endif /* SOFT_TIMER_H_ */
