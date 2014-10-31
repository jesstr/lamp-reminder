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

#define ADR_ALARM1_SEC		0x07
#define ADR_ALARM1_MIN		0x08
#define ADR_ALARM1_HOUR		0x09
#define ADR_ALARM1_DAY		0x0A
#define ADR_ALARM1_DATE		0x0A

#define ADR_ALARM2_MIN		0x0B
#define ADR_ALARM2_HOUR		0x0C
#define ADR_ALARM2_DAY		0x0D
#define ADR_ALARM2_DATE		0x0D

#define CONTROL_REG			0x0E
#define CONTROLSTATUS_REG	0x0E

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

/* Alarm period masks */
#define ONCE_PER_MON	0x0000000
#define ONCE_PER_WEEK	0x0000000
#define ONCE_PER_DAY	0x0001000
#define ONCE_PER_HOUR	0x0001100
#define ONCE_PER_MIN	0x0001110
#define ONCE_PER_SEC	0x0001111

/* Alarm period masks bits */
#define A1M1	0
#define A1M2	1
#define A1M3	2
#define A1M4	3
#define A2M1	0
#define A2M2	1
#define A2M3	2
#define A2M4	3

/* Control and Control/Status register bits */
#define A1IE	0
#define A2IE	1
#define INTCN	2
#define RS1		3
#define RS2		4
#define CONV	5
#define BBSQW	6
#define EOSC	7

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
time_t alarm1;
time_t alarm2;

/* Init clock */
void TWI_Init(void);
/* Set current time */
void TWI_SetTime(time_t *time);
/* Get current time */
void TWI_GetTime(time_t *time);
/* Receive byte from slave */
unsigned char TWI_GetByte(unsigned char Adr);
/* Print current date and time */
char *TWI_TimeToStr(time_t *time, char *buf);
/* Set Alarm1  */
void TWI_SetAlarm1(time_t *alarm, unsigned char period_mask);
/* Set Alarm2  */
void TWI_SetAlarm1(time_t *alarm, unsigned char period_mask);
/* Get current alarm1 time */
void TWI_GetAlarm1(time_t *alarm);
/* Get current alarm2 time */
void TWI_GetAlarm2(time_t *alarm);


#endif /* TWICLOCK_H_ */


