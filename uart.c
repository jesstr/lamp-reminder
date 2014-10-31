#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
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

	n_butes = 0;	// Ñ÷åò÷èê ïðèíÿòûõ ïî UART áàéò
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

void UART_FillRxBuf(unsigned char data)
{
static unsigned escape_key = 0;
static unsigned escape_chain = 0;

	if ((n_butes < UART_RX_BUFF_SIZE - 1) && (data != END_OF_COMMAND)) {

		/* If "Escape" key was pressed */
		if (data == 0x1B) {
			//global_state |= (1 << UART_escape_chain_bit);
			escape_key = 1;
			return;
		}

		/* If escape chain was pressed */
		if (escape_key && data == '[') {
			escape_key = 0;
			escape_chain = 1;
			return;
		}

		/* Escape chains processing */
		if (escape_chain) {
			/* "UP" key pressed */
			if (data == 'A') {
				/* Restore command from history */
				if (history_pos > 0) {
					history_pos--;
				}
				else {
					history_pos = UART_RX_HISTORY_SIZE - 1;
				}

				n_butes = strlen(uart_rx_history[history_pos]);
				strncpy(uart_rx_buff, uart_rx_history[history_pos], n_butes);
				uart_rx_buff[n_butes] = '\0';

				char buf[8];

				/* Print command */
				UART_SendString("\r");
				UART_SendString(PROMPTLINE);
				UART_SendString(itoa(history_pos, buf, 10));
				UART_SendString(uart_rx_history[history_pos]);

				global_state &= ~(1 << UART_escape_chain_bit);
			}
			/* "DOWN" key pressed */
			if (data == 'B') {
				UART_SendString("down");
				global_state &= ~(1 << UART_escape_chain_bit);
			}
			escape_chain = 0;
			return;
		}

		/* Separate keys processing */
		else {
			/* "Backspace" key processed */
			if (data == '\b') {
				uart_rx_buff[n_butes--] = '\0';
				UART_SendString("\b \b");
				return;
			}
			/* Put char on screen */

			uart_rx_buff[n_butes++] = data;
			UART_SendByte(data);
		}
	}

	else {
		/* Buffer overflowed */
		if (n_butes >= UART_RX_BUFF_SIZE) {
			global_state |= (1<<UART_buffoverflow_bit);
		}
		else {
			/* "Enter" key pressed */
			if ((global_state & (1 << UART_rx_complete_bit)) == 0) {
				uart_rx_buff[n_butes] = '\0';
				global_state |= (1 << UART_rx_complete_bit);
				global_state &= ~(1 << UART_wrong_package_bit);
				strcpy(uart_rx_packet, uart_rx_buff);
				UART_SendString("\r\n");
				UART_SendString(PROMPTLINE);
				/* Save command to history */
				if (n_butes > 0) {

					if (history_pos < UART_RX_HISTORY_SIZE - 1) {
						history_pos++;
					}
					else {
						history_pos = 0;
					}
					strcpy(uart_rx_history[history_pos], uart_rx_buff);
				}
			}
			else {
				/* Wrong package received */
				global_state |= (1<<UART_wrong_package_bit);
			}
		}
	n_butes = 0;
	}
}

/* RX Interrupt routine (has to be moved to the main file) */
/*
ISR(USART1_RX_vect)
{
	unsigned char buff=UDR1;
}
*/
