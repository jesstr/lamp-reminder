/*
 * uart.h
 *
 *  Created on: 12.09.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#ifndef _UART_H_
#define _UART_H_

#define BAUDRATE 38400
#define MYUBRR F_CPU/16/BAUDRATE-1

/* Command prompt line */
#define PROMPTLINE "\r>\x1B[K" /* \x1B[K - clear screen after cursor */

/* TODO translate comments */
#define UART_RX_BUFF_SIZE		32	// Ðàçìåð áóôåðà ïðèåìà UART
#define UART_TX_BUFF_SIZE		32	// Ðàçìåð áóôåðà ïðèåìà UART
#define UART_LEX_MASS_SIZE		5	// Ðàçìåð ìàññèâà ëåêñåì

#define END_OF_COMMAND		0x0D	//Flag, that indicates the end of command

char uart_rx_buff[UART_RX_BUFF_SIZE];	// Áóôåð ïðèåìà UART
char uart_rx_packet[UART_RX_BUFF_SIZE];	// Ïðèíÿòàÿ ïî UART ïîñûëêà
//char uart_rx_history[UART_RX_HISTORY_SIZE][UART_RX_BUFF_SIZE];
//char uart_tx_buff[UART_TX_BUFF_SIZE];	// Áóôåð ïðèåìà UART

char *uart_tx_buff;				/* UART TX buffer */
char *uart_tx_buff_p;

//char lex[UART_LEX_MASS_SIZE][UART_RX_BUFF_SIZE];	// Ìàññèâ ëåêñåì
char *lex_p[UART_LEX_MASS_SIZE];	// Ìàññèâ óêàçàòåëåé íà ëåêñåìû

char *command;

unsigned char n_bytes;	// Ñ÷åò÷èê ïðèíÿòûõ ïî UART áàéò
unsigned char n_lex;	// Ñ÷åò÷èê ëåêñåì

unsigned char global_state; // Ïåðåìåííàÿ ôëàãîâ ñîñòîÿíèÿ

#define UART_rx_complete_bit 	0 // Ôëàã ïðèåìà UART ïîñûëêè
#define UART_buffoverflow_bit 	1 // Ôëàã ïåðåïîëíåíèÿ áöôåðà ïðèåìà UART
#define UART_wrong_package_bit 	2 // Ôëàã ïîòåðÿííîãî(ûõ) áàéòà(îâ) UART ïîñûëêè
#define UART_tx_ready_bit 		3 /* TX data ready flag */

#define IS_NEW_COMMAND 		global_state & (1 << UART_rx_complete_bit) // Ïðîâåðêà, íåò ëè íîâîé êîìàíäû äëÿ îáðàáîòêè

#define COMMAND_DONE 		do { \
							global_state &= ~(1 << UART_rx_complete_bit); \
							n_lex = 0; \
							} while(0)	// Ñáðîñ ôëàãà íîâîé êîìàíäû ïîñëå îáðàáîòêè, ñáðîñ èíäåêñà ìàññèâà ëåêñåì

#define IS_DATA_TO_SEND		global_state & (1 << UART_tx_ready_bit) 		/* TX data ready check */
#define DATA_SEND_DONE		global_state &= ~(1 << UART_tx_ready_bit) 	/* TX data ready flag clear */
#define DATA_SEND_READY		global_state |= (1 << UART_tx_ready_bit)		/* TX data ready flag set */

#define IS_ESCAPE_CHAIN		global_state |= (1 << UART_escape_chain_bit)		/* TX data ready flag set */


/* UART initialization */
void UART_Init(const unsigned int ubrr1);
/* Send a byte over UART */
void UART_SendByte(const unsigned char byte1);
/* Send text string over UART */
void UART_SendString(const const char *buffer);
/* Send nbytes of data over UART */
void UART_SendData(const char *buffer, unsigned short nbytes);
/* Send text string from program memory over UART */
void UART_PgmSendString(const char *str);
/* Send text string from eeprom over UART */
void UART_EememSendString(const char *str);


#endif /* _UART_H_ */
