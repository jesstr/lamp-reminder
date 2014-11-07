#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdlib.h>
#include "uart.h"


/* UART initialization */
void UART_Init(unsigned int ubrr1)
{
	/* UBRRH = (unsigned char)(ubrr1>>8); */	/* Setting operation frequency hight byte */
	UBRRL = (unsigned char) ubrr1;				/* Setting operation frequency low byte */
	UCSRB = (1<<RXEN)|(1<<TXEN); 				/* RX enabled, TX enabled */
	UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);	/* Asynchronous mode, 8 bit, 1 stop bit, no parity check */
	UCSRB |= (1<<RXCIE);						/* RX interrupt enabled */

	uart_tx_buff = NULL;		/* UART TX buffer */
	uart_tx_buff_p = NULL;

	command = NULL;

	n_bytes = 0;	// Ñ÷åò÷èê ïðèíÿòûõ ïî UART áàéò
	n_lex = 0;	// Ñ÷åò÷èê ëåêñåì

	global_state = 0; // Ïåðåìåííàÿ ôëàãîâ ñîñòîÿíèÿ
}

/* Send a byte over UART */
void UART_SendByte(unsigned char byte1)
{
	while (!( UCSRA & (1<<UDRE)));
	UDR = byte1;
};

/* Send text string over UART */
void UART_SendString(char *buffer)
{
	while (*buffer!=0) {
		 UART_SendByte(*buffer++);
		 //buffer++;
	}
}

/* Send data over UART */
void UART_SendData(char *buffer, unsigned short nbytes)
{
	while (nbytes > 0) {
		UART_SendByte(*buffer++);
		nbytes--;
	}
}

/* Send text string from program memory over UART */
void UART_PgmSendString(char *str){
	int i = 0;
	char buf;

	while( (buf = pgm_read_byte(str++)) ) {
			i++;
			UART_SendByte(buf);
		}
}

/* RX Interrupt routine (has to be moved to the main file) */
/*
ISR(USART1_RX_vect)
{
	unsigned char buff=UDR1;
}
*/
