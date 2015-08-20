/*
 * tty.h
 *
 *  Created on: 07.11.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#ifndef TTY_H_
#define TTY_H_

#include "uart.h"

/* */
#define STR_BUFF_SIZE 		UART_RX_BUFF_SIZE
/* */
#define TTY_HISTORY_SIZE	5
/* */
unsigned char history_pos;
/* */
char tty_history[TTY_HISTORY_SIZE][STR_BUFF_SIZE];




char *tty_history_look_down(void);

char *tty_history_look_up(void);

char *tty_history_pop(void);

void tty_history_add(char *str);

#endif /* TTY_H_ */
