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
#include "rgb.h"
#include "uart.h"
#include "int.h"
#include "soft_timer.h"
#include "twiclock.h"


/* Flag, new UART command received */
volatile unsigned char new_command;
/* UART RX buffer */
volatile unsigned char uart_rx_buf;


/* USART RX interrupt routine */
ISR(USART_RXC_vect)
{
	uart_rx_buf = UDR;
	new_command = 1;
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
	TCCR0 |= (1<<CS02)|(1<<CS00);
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
			}
			red_timer.counter++;
		}
		else {
			RED_TIMER_RESET;
		}
	}
	/* blue_timer check */
	if ( blue_timer.is_running ) {
		if (blue_timer.counter < 255) {
			if (blue_timer.counter == blue_timer.load) {
				/* Set blue LED pin */
			}
			blue_timer.counter++;
		}
		else {
			BLUE_TIMER_RESET;
		}
	}
	/* green_timer check */
	if ( green_timer.is_running ) {
		if (green_timer.counter < 255) {
			if (green_timer.counter == green_timer.load) {
				/* Set green LED pin */
			}
			green_timer.counter++;
		}
		else {
			GREEN_TIMER_RESET;
		}
	}
}

/* TODO IO pins initialization */
inline void IO_Init(void) {
	//AD1_DDR |= (1 << AD1_PIN);

}

/* Main routine */
int main(void)
{
	RGB_IO_Init();

	red_timer.load = 100;
	blue_timer.load = 100;
	green_timer.load = 100;

	RGB_TIMERS_START;

	TWI_Init();
	year = eeprom_read_word(&year);
	UART_Init(MYUBRR);
	INT_Init();
	sei();

	/* Set time  */
	time.year = 1;
	time.mon = 1;
	time.date = 1;
	time.hour = 1;
	time.min = 1;
	time.sec = 1;

	TWI_SetTime();

	/* Get time */
	unsigned char tmp;

 	tmp=TWI_GetByte(ADR_SEC);
	time.sec=(tmp/16)*10+tmp%16;
	tmp=TWI_GetByte(ADR_MIN);
	time.min=(tmp/16)*10+tmp%16;
	tmp=TWI_GetByte(ADR_HOUR);
	time.hour=(tmp/16)*10+tmp%16;


	while (1) {
		if (new_command) {
			switch (uart_rx_buf) {
				/* Reset LED color (switch LEDs off) */
				case 0xC0:

					new_command = 0;
					break;
				/* Self test */
				case 0xCC:
					UART_SendByte(0xC3);
					new_command = 0;
					break;
				/* Set LED color */
				case 0x80 ... 0x9F:

					new_command = 0;
					break;
				default:
					break;
			}
		}
	}
}
