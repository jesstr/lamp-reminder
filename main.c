/*
 * main.c
 *
 *  Created on: 12.09.2014
 *      Author: Pavel Cherstvov
 */

//#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>
#include "rgb.h"
#include "uart.h"
#include "int.h"
#include "soft_timer.h"
#include "twiclock.h"
#include "parce.h"

/* Flag, new UART command received */
volatile unsigned char new_command;
/* UART RX buffer */
volatile unsigned char uart_rx_buf;
/* Color cursor */
unsigned char color_to_set;
/*  */
char buf[32];

/* Banner image */
char banner[] PROGMEM = {
"											\n\r\
Wedding Anniversary Lamp					\n\r\
               _							\n\r\
        {@}  _|=|_							\n\r\
       /(\")\\  (\") 						\n\r\
      /((~))\\/<X>\\						\n\r\
      ~~/@\\~~\\|_|/						\n\r\
       /   \\  |||							\n\r\
      /~@~@~\\ |||							\n\r\
_____/_______\\|||_______					\n\r\
						  					\n\r\
  September 19th 2014						\n\r\
											\n\r\
    Andrew and Lina							\n\r\
________________________					\n\r\
											\n\r"
};


/* USART RX interrupt routine */
ISR(USART_RXC_vect)
{
	UART_FillRxBuf(UDR);
}

/* TODO INT0 interrupt routine (INT0 external IRQ) */
ISR(INT0_vect) {

}

/* TODO INT1 interrupt routine (INT1 external IRQ) */
ISR(INT1_vect) {

}

/* SoftPwm_Timer timer initialization (ATmega8 Timer0) */
void SoftPwm_Timer_Init(void)
{
	/* 1024 divider, (~30Гц) on 8MHz */
	//TCCR0 |= (1<<CS02)|(1<<CS00);
	TCCR0 |= (1<<CS00); //no div
	TIMSK |= (1<<TOIE0);
	TCNT0 = 0;
}

/* SoftPwm_Timer interrupt routine (Timer0 overflow IRQ) */
ISR(TIMER0_OVF_vect)
{
	/* red_timer check */
	if ( red_timer.is_running ) {
		if (red_timer.counter < 255) {
			if (red_timer.counter == red_timer.load) {
				/* Set red LED pin */
				RED_PORT &= ~(1 << RED_PIN);
			}
			red_timer.counter++;
		}
		else {
			RED_TIMER_RESET;
			RED_PORT |= (1 << RED_PIN);
		}
	}
	/* blue_timer check */
	if ( blue_timer.is_running ) {
		if (blue_timer.counter < 255) {
			if (blue_timer.counter == blue_timer.load) {
				/* Set blue LED pin */
				BLUE_PORT &= ~(1 << BLUE_PIN);
			}
			blue_timer.counter++;
		}
		else {
			BLUE_TIMER_RESET;
			BLUE_PORT |= (1 << BLUE_PIN);
		}
	}
	/* green_timer check */
	if ( green_timer.is_running ) {
		if (green_timer.counter < 255) {
			if (green_timer.counter == green_timer.load) {
				/* Set green LED pin */
				GREEN_PORT &= ~(1 << GREEN_PIN);
			}
			green_timer.counter++;
		}
		else {
			GREEN_TIMER_RESET;
			GREEN_PORT |= (1 << GREEN_PIN);
		}
	}
}

/* TODO IO pins initialization */
inline void IO_Init(void) {
	//AD1_DDR |= (1 << AD1_PIN);

}


/* */
void inline PrintString(char *str) {
	uart_tx_buff = str;
	DATA_SEND_READY;
}


/* Main routine */
int main(void)
{
	RGB_IO_Init();
	SoftPwm_Timer_Init();

	red_timer.load = 100;
	blue_timer.load = 100;
	green_timer.load = 100;

	RGB_TIMERS_START;

	TWI_Init();
	year = eeprom_read_word(&year);
	UART_Init(MYUBRR);
	INT_Init();
	sei();

	/* Print banner image */
	UART_PgmSendString(banner);
	UART_SendString(PROMPTLINE);

	/* Time debug */
	time.year = 14;
	time.mon = 10;
	time.date = 21;
	time.hour = 17;
	time.min = 00;
	time.sec = 00;

	TWI_SetTime(&time);

	TWI_GetTime(&time);
	PrintString(TWI_TimeToStr(buf));
	/* Time debug end */

	  while(1) {
			if (IS_NEW_COMMAND) {
				command = strtok(uart_rx_packet, "=, ");
				lex_p[n_lex++] = command;
				while( (command = strtok(NULL, "=,.: ")) ) {
					lex_p[n_lex++] = command;
				}
				/* Set current date and time: "date" */
				if (strcmp(lex_p[0], "date") == 0) {
					TWI_GetTime(&time);
					PrintString(TWI_TimeToStr(buf));
					COMMAND_DONE;
				}
				/* Set current date and time: "setdate DDMMYYHHMMSS" */
				if (strcmp(lex_p[0], "setdate") == 0) {
					/* Check input data and time format */
					if (strlen(lex_p[1]) != 12) {
						PrintString("Wrong time format, use DDMMYYHHMMSS.\n\r \
For example: 281014105000 for 28th October 2014 10:50:00\n\r");
					}
					/* Set date and time */
					else {
						char tmpbuf[3];
						char *ch_p;

						ch_p = lex_p[1];
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						time.date = atoi(tmpbuf);

						ch_p += 2;
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						time.mon = atoi(tmpbuf);

						ch_p += 2;
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						time.year = atoi(tmpbuf);

						ch_p += 2;
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						time.hour = atoi(tmpbuf);

						ch_p += 2;
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						time.min = atoi(tmpbuf);

						ch_p += 2;
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						time.sec = atoi(tmpbuf);

						TWI_SetTime(&time);
					}
					COMMAND_DONE;
				}
				/* Set color:  "color <red, 0-255>,<green, 0-255>,<blue, 0-255>" */
				else if (strcmp(lex_p[0], "color") == 0) {
					red_timer.load = atoi(lex_p[1]);
					green_timer.load = atoi(lex_p[2]);
					blue_timer.load = atoi(lex_p[3]);
					COMMAND_DONE;
				}
				/* Set alarm date and time: "alarm DDMMYYYYHHMMSS" */
				else if (strcmp(lex_p[0], "alarm") == 0) {

					COMMAND_DONE;
				}
				/* TODO develop ping-pong functionality */
				else if (strcmp(lex_p[0], "ping") == 0) {
					uart_tx_buff = "pong\n\r";
					DATA_SEND_READY;
					COMMAND_DONE;
				}
				/* TODO develop print usage info */
				else if (strcmp(lex_p[0], "help") == 0) {
					/* TODO print usage info */
					PrintString("Help: [under constraction]\n\r");
					COMMAND_DONE;
				}
				/* Unrecognized command */
				else {
					//uart_tx_buff = "\n\r";
					//DATA_SEND_READY;
					COMMAND_DONE;
				}
			}
			if (IS_DATA_TO_SEND) {
				UART_SendString(uart_tx_buff);
				UART_SendString(PROMPTLINE);
				DATA_SEND_DONE;
			}
			_delay_us(2);
		}
}
