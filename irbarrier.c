/*
 * irbarrier.c
 *
 *  Created on: 21.11.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#include "irbarrier.h"


void IRBarrier_IO_Init(){

	IRLED1_DDR |= (1 << IRLED1_PIN);
	IRLED2_DDR |= (1 << IRLED2_PIN);

	IRLED1_PORT &= ~(1 << IRLED1_PIN);
	IRLED2_PORT &= ~(1 << IRLED2_PIN);
}
