/*
 * tty.c
 *
 *  Created on: 07.11.2014
 *      Author: Pavel Cherstvov
 */

#include "tty.h"
#include <string.h>

unsigned char i = 0;


char *tty_history_look_down(void) {
	char *buf;

	if (i < TTY_HISTORY_SIZE - 1) {
		i++;
	}
	else {
		i = 0;
	}
	buf = tty_history[i];
	return buf;
}

char *tty_history_look_up(void) {
	char *buf;

	buf = tty_history[i];
	if (i > 0) {
		i--;
	}
	else {
		i = TTY_HISTORY_SIZE - 1;
	}
	return buf;
}

char *tty_history_pop(void) {
	return tty_history[history_pos];
}

void tty_history_add(char *str) {

	if (history_pos < TTY_HISTORY_SIZE - 1) {
		history_pos++;
	}
	else {
		history_pos = 0;
	}

	i = history_pos;

	strcpy(tty_history[history_pos], str);
}
