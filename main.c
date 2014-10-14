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
#include <stdlib.h>
#include "rgb.h"
#include "uart.h"
#include "int.h"
#include "soft_timer.h"
#include "twiclock.h"


/* Flag, new UART command received */
volatile unsigned char new_command;
/* UART RX buffer */
volatile unsigned char uart_rx_buf;
/* Color cursor */
unsigned char color_to_set;


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

/* Main routine */
int main(void)
{
	RGB_IO_Init();
	SoftPwm_Timer_Init();

	red_timer.load = 100;
	blue_timer.load = 100;
	//green_timer.load = 100;

	RGB_TIMERS_START;

	//RED_PORT |= (1 << RED_PIN);
	//BLUE_PORT |= (1 << BLUE_PIN);
	//GREEN_PORT |= (1 << GREEN_PIN); //defected pin

	TWI_Init();
	year = eeprom_read_word(&year);
	UART_Init(MYUBRR);
	INT_Init();
	sei();

	UART_SendString("Start\n");


	UART_SendByte(TWI_GetByte(ADR_SEC)); //test
	//_delay_ms(500);
	UART_SendByte(TWI_GetByte(ADR_MIN)); //test
	//_delay_ms(500);
	UART_SendByte(TWI_GetByte(ADR_HOUR)); //test
	//_delay_ms(500);
	UART_SendByte(TWI_GetByte(ADR_DAY)); //test
	//_delay_ms(500);
	UART_SendByte(TWI_GetByte(ADR_DATE)); //test
	//_delay_ms(500);
	UART_SendByte(TWI_GetByte(ADR_MON)); //test
	//_delay_ms(500);
	UART_SendByte(TWI_GetByte(ADR_YEAR)); //test


	{ /* Time debug part */

		/* Debug set time  */
		time.year = 1;
		time.mon = 1;
		time.date = 1;
		time.hour = 1;
		time.min = 1;
		time.sec = 1;

#if 0
		TWI_SetTime();

		/* Debug get time */
		unsigned char tmp;

		tmp=TWI_GetByte(ADR_SEC);
		time.sec=(tmp/16)*10+tmp%16;
		tmp=TWI_GetByte(ADR_MIN);
		time.min=(tmp/16)*10+tmp%16;
		tmp=TWI_GetByte(ADR_HOUR);
		time.hour=(tmp/16)*10+tmp%16;

		/* Debug send time */
		char buf[10];

		UART_SendString(itoa(time.year, buf, 10));
		UART_SendString(itoa(time.mon, buf, 10));
		UART_SendString(itoa(time.date, buf, 10));
		UART_SendString(itoa(time.hour, buf, 10));
		UART_SendString(itoa(time.min, buf, 10));
		UART_SendString(itoa(time.sec, buf, 10));

		UART_SendString("\n");
#endif

	}

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
				//case 0x80 ... 0x9F:
				case 'C':
					color_to_set = 1;
					new_command = 0;
					break;
				default:
					switch (color_to_set) {
						case 1:
							red_timer.load = uart_rx_buf;
							color_to_set++;
							new_command = 0;
							break;
						case 2:
							green_timer.load = uart_rx_buf;
							color_to_set++;
							new_command = 0;
							break;
						case 3:
							blue_timer.load = uart_rx_buf;
							color_to_set = 0;
							new_command = 0;
							break;
						default:
							break;
					}
					break;
			}
		}

		//UART_SendByte(TWI_GetByte(ADR_SEC)); //test
		//_delay_ms(500);

	}
}
