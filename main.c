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
#include "uart.h"
#include "int.h"

/* Flag, new UART command received */
volatile unsigned char new_command;
/* UART RX buffer */
volatile unsigned char uart_rx_buf;


/* USART RX interrupt routine */
ISR(USART_RXC_vect)
{
	uart_rx_buf = UDR1;
	new_command = 1;
}

/* TODO INT0 interrupt routine (INT0 external IRQ) */
ISR(INT0_vect) {

}

/* TODO INT1 interrupt routine (INT1 external IRQ) */
ISR(INT1_vect) {

}

/* TODO IO pins initialization */
inline void IO_Init(void) {
	//AD1_DDR |= (1 << AD1_PIN);

}

/* Main routine */
int main(void)
{
	UART_Init(MYUBRR);
	INT_Init();
	sei();

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
