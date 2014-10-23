
// DS3232 - Extremely Accurate i2c RTC with Integrated Crystal and SRAM

#include <avr/eeprom.h>
#include "twiclock.h"
#include <stdio.h>

void TWI_Init()
{
	/* Set base year */
	ee_year = 2000;
	/* Set bit rate (100kHz on 8Mhz) */
	TWBR = 0x00;
	TWSR |= (1<<TWPS1);					// divider 16
	TWDR = 0xFF;						// release bus
	TWCR = (1<<TWEN)|					// TWI on
			(0<<TWIE)|(0<<TWINT)|
			(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|
			(0<<TWWC); 
}

/* Receive byte from slave */
unsigned char TWI_GetByte(unsigned char Adr)
{
	unsigned char data;

	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //START
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_START) //Status: START
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }

	TWDR = SLA_W; //Slave address + write flag
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_MTX_ADR_ACK) //Status: SLA+W_ACK
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }

	TWDR = Adr;	//data (address in slave memory)
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_MTX_DATA_ACK) //Status: DATA_ACK
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }
	
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //REPEATED START
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_REP_START) //Status: REPEATED START
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }
	
	TWDR = SLA_R; //Slave address + read flag
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_MRX_ADR_ACK) //Status: SLA+R_ACK
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }

	TWCR = (1<<TWINT) | (1<<TWEN) | (0<<TWEA); //ACK off 
	while (!(TWCR & (1<<TWINT)))
	;
	data=TWDR;
	if ((TWSR & 0xF8) != TWI_MRX_DATA_NACK) //Status: DATA_NACK
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }

	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); //STOP

	return data;
}

/* Sent byte to Slave */
unsigned char TWI_SetByte(unsigned char Adr, unsigned char data)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //START
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_START) //START
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }

	TWDR = SLA_W; //Slave address + write flag
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_MTX_ADR_ACK) //Status: SLA+W_ACK
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }

	TWDR = Adr;	//data (address in slave memory)
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_MTX_DATA_ACK) //Status: DATA_ACK
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }

	TWDR=data;
	TWCR = (1<<TWINT) | (1<<TWEN) | (0<<TWEA); //ACK off 
	while (!(TWCR & (1<<TWINT)))
	;
	if ((TWSR & 0xF8) != TWI_MRX_DATA_NACK) //Status: DATA_NACK
	{TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); return 0xff; }

	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO); //STOP

	return 0;
}

/* Get current time */
void TWI_GetTime(time_t *time)
{
	unsigned char tmp;
	tmp = TWI_GetByte(ADR_SEC);
	time->sec = (tmp / 16) * 10 + tmp % 16; //bcd2hex
	tmp = TWI_GetByte(ADR_MIN);
	time->min = (tmp / 16) * 10 + tmp % 16;
	tmp = TWI_GetByte(ADR_HOUR);
	time->hour = (tmp / 16) * 10 + tmp % 16;
	tmp = TWI_GetByte(ADR_DATE);
	time->date = (tmp / 16) * 10 + tmp % 16;
	tmp = TWI_GetByte(ADR_MON);
	time->mon = (tmp / 16) * 10 + tmp % 16;
	tmp = TWI_GetByte(ADR_YEAR);
	time->year = (tmp / 16) * 10 + tmp % 16;
}

/* Set current time */
void TWI_SetTime(time_t *time)
{
	//eeprom_write_word(&ee_year,year);
	TWI_SetByte(ADR_YEAR, time->year - 2000);	// offset
	TWI_SetByte(ADR_MON, (time->mon / 10) * 16 + time->mon % 10);
	TWI_SetByte(ADR_DATE, (time->date / 10) * 16 + time->date % 10);
	TWI_SetByte(ADR_HOUR, (time->hour / 10) * 16 + time->hour % 10);
	TWI_SetByte(ADR_MIN, (time->min / 10) * 16 + time->min % 10);
	TWI_SetByte(ADR_SEC, (time->sec / 10) * 16 + time->sec % 10);
}

/* Print current date and time */
char *TWI_PrintDateTime(char *buf)
{
	sprintf(buf, "Current time: %02d.%02d.%02d %02d:%02d:%02d\n",
			time.date, time.mon, time.year, time.hour, time.min, time.sec);

	return buf;
}
