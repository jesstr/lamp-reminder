/*
 * soft_timer.c
 *
 *  Created on: 24.04.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#include "soft_timer.h"

void SoftTimer_TimerReg(soft_timer_t *timer_p) {

	if ( soft_timers_count < MAX_SOFT_TIMERS_COUNT ) {
		soft_timers_queue[soft_timers_count++] = timer_p;
	}
}
