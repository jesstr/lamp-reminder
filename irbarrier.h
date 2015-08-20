/*
 * irbarrier.h
 *
 *  Created on: 21.11.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#ifndef IRBARRIER_H_
#define IRBARRIER_H_

#include <avr/io.h>


#define	IRLED1_DDR 	DDRD
#define	IRLED2_DDR 	DDRD

#define	IRLED1_PORT	PORTD
#define	IRLED2_PORT	PORTD

#define	IRLED1_PIN	6
#define	IRLED2_PIN	7

#define IRLED1_ON	IRLED1_PORT |= (1 << IRLED1_PIN)
#define IRLED1_OFF 	IRLED1_PORT &= ~(1 << IRLED1_PIN)

#define IRLED2_ON	IRLED2_PORT |= (1 << IRLED2_PIN)
#define IRLED2_OFF 	IRLED2_PORT &= ~(1 << IRLED2_PIN)

#define IRLED1_XOR	IRLED1_PORT ^= (1 << IRLED1_PIN)
#define IRLED2_XOR 	IRLED2_PORT ^= (1 << IRLED2_PIN)


/* IR leds IO pins initialization */
void IRBarrier_IO_Init();

#endif /* IRBARRIER_H_ */
