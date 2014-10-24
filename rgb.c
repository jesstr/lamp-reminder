/*
 * rgb.c
 *
 *  Created on: 16.09.2014
 *      Author: Pavel Cherstvov
 */

#include <avr/io.h>
#include "rgb.h"


/* RGB led IO pins initialization */
void RGB_IO_Init(void) {

	RED_DDR |= (1 << RED_PIN);
	BLUE_DDR |= (1 << BLUE_PIN);
	GREEN_DDR |= (1 << GREEN_PIN); //defected pin

	RED_PORT &= ~(1 << RED_PIN);
	BLUE_PORT &= ~(1 << BLUE_PIN);
	GREEN_PORT &= ~(1 << GREEN_PIN); //defected pin

}

