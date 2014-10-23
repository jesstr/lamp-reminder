/*
 * twiclock.h
 *
 *  Created on: 16.09.2012
 *      Author: Pavel Cherstvov
 */

#ifndef TWICLOCK_H_
#define TWICLOCK_H_

#define SLA_W	0b11010000	// Clock address + write flag	
#define SLA_R	0b11010001	// Clock address + read flag

/* Memory cells address */
#define ADR_SEC		0x00
#define ADR_MIN		0x01
#define ADR_HOUR	0x02
#define ADR_DAY		0x03
#define ADR_DATE	0x04
#define ADR_MON		0x05
#define ADR_YEAR	0x06
	
/* Common state registry values */
#define TWI_START                  0x08  // START sent
#define TWI_REP_START              0x10  // START sent repeated

/* State registry values in Master Transmitter mode */
#define TWI_MTX_ADR_ACK            0x18  // Sent SLA+W and ACK received
#define TWI_MTX_ADR_NACK           0x20  // Sent SLA+W and NACK received
#define TWI_MTX_DATA_ACK           0x28  // Sent data byte and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Sent data byte and NACK received

/* State registry values in Master Receiver mode */
#define TWI_MRX_ADR_ACK            0x40  // Sent SLA+R and ACK received
#define TWI_MRX_ADR_NACK           0x48  // Sent SLA+R and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Received data byte and ACK sent
#define TWI_MRX_DATA_NACK          0x58  // Received data byte and NACK sent

//uint16_t ee_year EEMEM = 2000; // Base year in EEPROM
uint16_t ee_year; // Base year = 2000 (twiclock.c)
uint16_t year;   // Year value, global variable

/* 4 bytes struct for time store */
typedef struct
{
	unsigned char sec:6;
	unsigned char min:6;
	unsigned char hour:5;
	unsigned char date:5;
	unsigned char mon:4;
	unsigned char year:6;
} time_t;

time_t time;

/* Init clock */
void TWI_Init(void);
/* Set current time */
void TWI_SetTime(time_t *time);
/* Get current time */
void TWI_GetTime(time_t *time);
/* Receive byte from slave */
unsigned char TWI_GetByte(unsigned char Adr);
/* Print current date and time */
char *TWI_PrintDateTime(char *buf);

#endif /* TWICLOCK_H_ */


