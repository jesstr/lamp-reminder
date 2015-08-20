/*
 * main.c
 *
 *  Created on: 12.09.2014
 *      Author: Pavel Cherstvov
 *    Compiler: avr-gcc 4.5.3
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rgb.h"
#include "uart.h"
#include "int.h"
#include "soft_timer.h"
#include "twiclock.h"
#include "tty.h"
#include "irbarrier.h"

/* Uncomment this line to enable debug functions and extended output messages.
 * This macros is unique in each source file, which has debug functions, so it should
 * be commented/uncommented individually! */
#define _DEBUG_	1


/* Flag, new UART command received */
volatile unsigned char new_command;
/* UART RX buffer */
volatile unsigned char uart_rx_buf;
/* Color cursor */
unsigned char color_to_set;
/* TODO add a comment */
char buf[24];
/* TODO add a comment */
char line[80];

/* Device state flags */
unsigned char device_state;

/* SysTimer ticks, incremented on every timer irq */
unsigned long sys_ticks;

/* Device state bits */
#define power_on		0
#define pwr_button_hold	1

/* Device state defines */
#define IS_POWER_ON			(device_state & (1 << power_on))
#define IS_PWR_BUTTON_HOLD	(device_state & (1 << pwr_button_hold))

#define SET_POWER_ON		device_state |= (1 << power_on)
#define CLR_POWER_ON		device_state &= ~(1 << power_on)
#define SET_PWR_BUTTON_HOLD	device_state |= (1 << pwr_button_hold)
#define CLR_PWR_BUTTON_HOLD device_state &= ~(1 << pwr_button_hold)

volatile unsigned int l_sensor = 0;
volatile unsigned int r_sensor = 0;

char version[] = {"1.0"};

/* Banner image */
char banner[] PROGMEM = {
#ifdef _DEBUG_
'\0'};
#else
"\n\r\
Wedding Anniversary Lamp\n\r\
\n\r\
        {@}  _|=|_\n\r\
       /(\")\\  (\")\n\r\
      /((~))\\/<X>\\\n\r\
      ~~/@\\~~\\|_|/\n\r\
       /   \\  |||\n\r\
      /~@~@~\\ |||\n\r\
_____/_______\\|||_______\n\r\
\n\r\
  September 19th 2014\n\r\
\n\r\
    Andrew and Lina\n\r\
________________________\n\r\
\n\r"
};
#endif

/* Help text (placed to eeprom to save program memory) */
char help[] EEMEM = {
"\n\r\
date -  Print date and time\n\r\
alarm -	Print alarm time\n\r\
color - Set lamp color in RGB format\n\r\
	Usage: color <red, 0-255>,<green, 0-255>,<blue, 0-255>\n\r\
setdate - Set current date and time\n\r\
	Usage: setdate DDMMYYHHMMSS\n\r\
setalarm - Set alarm time\n\r\
	Usage: setalarm DDMMYYYYHHMMSS\n\r\
version - Print version info\n\r\
	Usage: version\n\r\
help - 	Show help\n\r"
#ifdef _DEBUG_
"getreg - Print register\n\r\
	Usage - getreg <addr, 0-255>\n\r\
setreg - Set register\n\r\
	Usage - setreg <addr, 0-255>, <val, 0-255>\n\r"
#endif
};

char err_msg_timeformat[] PROGMEM = {
#ifdef _DEBUG_
"err"};
#else
"Wrong time format, use DDMMYYHHMMSS.\n\r\
For example: 281014105000 for 28th October 2014 10:50:00\n\r"
};
#endif

char err_msg_alarmset[] PROGMEM = {
#ifdef _DEBUG_
"err"};
#else
"Alarm is not set. To set use \"setalarm\"\n\r"
};
#endif

char err_msg_arg[] PROGMEM = {
#ifdef _DEBUG_
"err"};
#else
"Too few arguements\n\r"
};
#endif

char err_msg_nocommand[] EEMEM = {
"Not found. Try \"help\"\n\r"
};


/* USART RX interrupt routine */
ISR(USART_RXC_vect)
{
	unsigned char data = UDR;

	static unsigned escape_key = 0;
	static unsigned escape_chain = 0;

		if ((n_bytes < UART_RX_BUFF_SIZE - 1) && (data != END_OF_COMMAND)) {

			/* If "Escape" key was pressed */
			if (data == 0x1B) {
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
					strcpy(uart_rx_buff, tty_history_look_up());
					n_bytes = strlen(uart_rx_buff);

					/* Print command */
					sprintf(line, "\r%s%s", PROMPTLINE, uart_rx_buff);
					uart_tx_buff = line;
					DATA_SEND_READY;

				}
				/* "DOWN" key pressed */
				if (data == 'B') {
					/* Restore command from history */
					strcpy(uart_rx_buff, tty_history_look_down());
					n_bytes = strlen(uart_rx_buff);

					/* Print command */
					sprintf(line, "\r%s%s", PROMPTLINE, uart_rx_buff);
					uart_tx_buff = line;
					DATA_SEND_READY;

				}
				escape_chain = 0;
				return;
			}

			/* Separate keys processing */
			else {
				/* "Backspace" key processed */
				if (data == '\b') {
					if (n_bytes > 0) {
						uart_rx_buff[n_bytes--] = '\0';
						UART_SendString("\b \b");
					}
					return;
				}

				/* Put char on screen */
				uart_rx_buff[n_bytes++] = data;
				UART_SendByte(data);
			}
		}

		else {
			/* Buffer overflowed */
			if (n_bytes >= UART_RX_BUFF_SIZE) {
				global_state |= (1<<UART_buffoverflow_bit);
			}
			else {
				/* "Enter" key pressed */
				if ((global_state & (1 << UART_rx_complete_bit)) == 0) {
					uart_rx_buff[n_bytes] = '\0';
					global_state |= (1 << UART_rx_complete_bit);
					global_state &= ~(1 << UART_wrong_package_bit);
					strcpy(uart_rx_packet, uart_rx_buff);

					UART_SendString("\n\r");

					sprintf(line, "\r%s", PROMPTLINE);
					uart_tx_buff = line;
					DATA_SEND_READY;

					/* Save command to history */
					if (n_bytes > 0) {
						tty_history_add(uart_rx_buff);
					}
				}
				else {
					/* Wrong package received */
					global_state |= (1<<UART_wrong_package_bit);
				}
			}
		n_bytes = 0;
		}
}

/* TODO INT0 interrupt routine (INT0 external IRQ) */
ISR(INT0_vect) {

}

/* TODO INT1 interrupt routine (INT1 external IRQ) */
ISR(INT1_vect) {

}

/* SysTimer timer initialization (ATmega8 Timer0) */
static void SysTimer_Init(void)
{
	/* no div, Normal mode */
	//TCCR0 = (1 << CS00);
	/* div 8, Normal mode */
	TCCR0 = (1 << CS01);
	TIMSK |= (1 << TOIE0);
	TCNT0 = 0;
}

/* _Timer timer initialization (ATmega8 Timer1) */
static void IR_Timer_Init(void)
{
	/* no div, CTC mode */
	TCCR1B = (1 << CS10) | (1 << WGM12);
	//TCCR1B = (1 << WGM12);
	/* Set IRQ freq is 72072Гц on 8MHz */
	//OCR1A = 0x006F;
	OCR1A = 0x00DC;
	TIMSK |= (1 << OCIE1A);
	TCNT1 = 0x0000;
}

#if 0
inline void SoftPWM_Timer_Tick(void) {
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
#endif

static inline void SysTimer_TimerTick(void) {

	signed char i;

	sys_ticks++;

	for (i = soft_timers_count - 1; i >= 0; i--) {
		/* Soft timer check */
		if ( soft_timers_queue[i]->is_running ) {
			if (soft_timers_queue[i]->counter < soft_timers_queue[i]->top) {
				if (soft_timers_queue[i]->counter == soft_timers_queue[i]->load) {
					/* Run compare handler function */
					soft_timers_queue[i]->compare_handler();
				}
				soft_timers_queue[i]->counter++;
			}
			else {
				soft_timers_queue[i]->counter = 0;
				/* Run overflow handler function */
				soft_timers_queue[i]->overflow_handler();
			}
		}
	}
}

/*  IR_Timer interrupt routine (Timer1 compare match IRQ) */
ISR(TIMER1_COMPA_vect)
{
	IRLED1_XOR;
	IRLED2_XOR;
}

/* SysTimer interrupt routine (Timer0 overflow IRQ) */
ISR(TIMER0_OVF_vect)
{
	/* Enable nested interrupts, be careful! */
	sei();
	SysTimer_TimerTick();
}

/* IO pins initialization */
static inline void IO_Init(void) {
	/* IR barrier input pins initialization */
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);
	DDRD &= ~(1 << PD3);
	PORTD |= (1 << PD3);

	/* TWI irq input pin initialization */
	DDRC &= ~(1 << PC3);
	PORTC |= (1 << PC3);

	/* TWI clock reset pin initialization */
	DDRB &= ~(1 << PB0);
	PORTB |= (1 << PB0);
}

/*
void IR_LedFreg() {
	IRLED1_XOR;
	IRLED2_XOR;
	//_delay_us(5);
	IRLED1_XOR;
	IRLED2_XOR;
}
*/

/* Main routine */
int main(void)
{
	IO_Init();

	/* Power on bounce protection delay */
	//_delay_ms(300);

	UART_Init(MYUBRR);
	RGB_IO_Init();
	SysTimer_Init();
	IR_Timer_Init();

	TWI_Init();
	year = eeprom_read_word(&year);

	IRBarrier_IO_Init();

	sei();

	/* Print banner image */
	UART_PgmSendString(banner);
	//UART_SendString(PROMPTLINE);

	TWI_GetTime(&time);
	sprintf(line, "Current date: %s\n\r%s", TWI_TimeToStr(&time, buf), PROMPTLINE);
	uart_tx_buff = line;
	DATA_SEND_READY;

	/*
	#define max(x, y)	((x) > (y) ? (x) : (y))

	dimmer.r_target = 200 * 10;
	dimmer.g_target = 200 * 10;
	dimmer.b_target = 200 *10;

	dimmer.r_load = red_timer.load * 10;
	dimmer.g_load = green_timer.load * 10;
	dimmer.b_load = blue_timer.load * 10;

	signed char sign = 1;

	dimmer.r_step = (signed int)((signed int)(dimmer.r_target - (signed int)dimmer.r_load)) / 200 + sign;
	dimmer.g_step = (signed int)((signed int)(dimmer.g_target - (signed int)dimmer.g_load)) / 200 + sign;
	dimmer.b_step = (signed int)((signed int)(dimmer.b_target - (signed int)dimmer.b_load)) / 200 + sign;

	dimmer.nsteps = max((dimmer.r_target - dimmer.r_load) / dimmer.r_step,
					max((dimmer.g_target - dimmer.g_load) / dimmer.g_step,
						(dimmer.b_target - dimmer.b_load) / dimmer.b_step));

*/
	unsigned long d = 0;
	unsigned long time_stamp = 0;
	unsigned long delay = 1000;

	while(1) {
			if (IS_NEW_COMMAND) {
				command = strtok(uart_rx_packet, "=, ");
				lex_p[n_lex++] = command;
				while( (command = strtok(NULL, "=, ")) ) {
					lex_p[n_lex++] = command;
				}
				/* Get current date and time: "date" */
				if (strcmp(lex_p[0], "date") == 0) {
					TWI_GetTime(&time);
					sprintf(line, "Current time: %s\n\r%s", TWI_TimeToStr(&time, buf), PROMPTLINE);
					uart_tx_buff = line;
					DATA_SEND_READY;
					COMMAND_DONE;
				}
				/* Get current alarm time: "alarm" */
				if (strcmp(lex_p[0], "alarm") == 0) {

					if (ALARM1_IS_ON) {

						TWI_GetAlarm1(&alarm1);
						sprintf(line, "Alarm time: %s\n\r%s", TWI_TimeToStr(&alarm1, buf), PROMPTLINE);
						uart_tx_buff = line;
						DATA_SEND_READY;
						COMMAND_DONE;
					}
					else {
						UART_PgmSendString(err_msg_alarmset);
						UART_SendString(PROMPTLINE);
						COMMAND_DONE;
					}

				}
				/* Set current date and time: "setdate DDMMYYHHMMSS" */
				if (strcmp(lex_p[0], "setdate") == 0) {
					/* Check input data and time format */
					/* TODO Add non digit check */
					if (strlen(lex_p[1]) != 12) {
						UART_PgmSendString(err_msg_timeformat);
						UART_SendString(PROMPTLINE);
						COMMAND_DONE;

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
						/*
						TWI_GetTime(&time);
						uart_tx_buff = TWI_TimeToStr(&time, buf);
						DATA_SEND_READY;
						*/
					}
					COMMAND_DONE;
				}
#if 0
				/* Set color:  "color <red, 0-255>,<green, 0-255>,<blue, 0-255>" */
				else if (strcmp(lex_p[0], "color") == 0) {
					if (n_lex < 4) {
						UART_PgmSendString(err_msg_arg);
						UART_SendString(PROMPTLINE);
						COMMAND_DONE
					}
					else {
						SET_POWER_ON;
						RGB_SetColor(atoi(lex_p[1]),atoi(lex_p[2]),atoi(lex_p[3]));
					}
					COMMAND_DONE;
				}
#endif
				/* Change color smooth "color <red, 0-255>,<green, 0-255>,<blue, 0-255>,<ms, 0 - 8415>" */
				else if (strcmp(lex_p[0], "color") == 0) {
					if (n_lex < 4) {
						UART_PgmSendString(err_msg_arg);
						UART_SendString(PROMPTLINE);
/*
						sprintf(line, "Color: %d,%d,%d\n\r%s", red_timer.load, PROMPTLINE);
						uart_tx_buff = line;
						DATA_SEND_READY;
*/
						COMMAND_DONE;
					}
					else {
						if (n_lex == 4) {
							/* Set color with no smooth */
							//RGB_ShortDimColor(atoi(lex_p[1]), atoi(lex_p[2]), atoi(lex_p[3]), 0);
							RGB_SetColor(atoi(lex_p[1]), atoi(lex_p[2]), atoi(lex_p[3]));
						}
						else {
							/* Set color smooth */
							//TIMSK &= ~(1 << OCIE1A);
							//PORTD |= (1 << PD2);
							RGB_ShortDimColor(atoi(lex_p[1]), atoi(lex_p[2]), atoi(lex_p[3]), atoi(lex_p[4]));

						}
					SET_POWER_ON;
					}
					COMMAND_DONE;
				}
#if 0
				else if (strcmp(lex_p[0], "dimcolor") == 0) {
					if (n_lex > 3) {
						if (n_lex == 4) {
							/* Set color with no smooth */
							RGB_ShortDimColor(atoi(lex_p[1]), atoi(lex_p[2]), atoi(lex_p[3]), 0);
						}
						else {
							/* Set color smooth */
							RGB_ShortDimColor(atoi(lex_p[1]), atoi(lex_p[2]), atoi(lex_p[3]), atoi(lex_p[4]));
						}
						SET_POWER_ON;
					}
					else {
						UART_PgmSendString(err_msg_arg);
						UART_SendString(PROMPTLINE);
						COMMAND_DONE;
					}
					COMMAND_DONE;
				}
#endif
				/* Set alarm time: "setalarm DDMMYYHHMMSS" */
				else if (strcmp(lex_p[0], "setalarm") == 0) {
					/* Check alarm time format */
					if (strlen(lex_p[1]) != 12) {
						UART_PgmSendString(err_msg_timeformat);
						UART_SendString(PROMPTLINE);
						COMMAND_DONE;
					}
					/* Set alarm */
					else {
						char tmpbuf[3];
						char *ch_p;

						ch_p = lex_p[1];
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						alarm1.date = atoi(tmpbuf);

						/* Skip mon and year field TODO */
						ch_p += 4;

						ch_p += 2;
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						alarm1.hour = atoi(tmpbuf);

						ch_p += 2;
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						alarm1.min = atoi(tmpbuf);

						ch_p += 2;
						strncpy(tmpbuf, ch_p, 2);
						tmpbuf[2] = '\0';
						alarm1.sec = atoi(tmpbuf);

						TWI_SetAlarm1(&alarm1, ONCE_PER_MON);

						#ifdef _DEBUG_
						/* Debug output */
						sprintf(line, "CONTROL_REG: %x\n\r", TWI_GetByte(CONTROL_REG));
						UART_SendString(line);

						sprintf(line, "CONTROLSTATUS_REG: %x\n\r", TWI_GetByte(CONTROLSTATUS_REG));
						UART_SendString(line);
						#endif

						/* TODO Alarm is set message needed */
						/*
						uart_tx_buff = TWI_TimeToStr(&alarm1, buf);
						DATA_SEND_READY;
						*/
					}
					COMMAND_DONE;
				}
				/* Print usage info */
				else if (strcmp(lex_p[0], "help") == 0) {
					UART_EememSendString(help);
					UART_SendString(PROMPTLINE);
					COMMAND_DONE;
				}
				/* Print version info */
				else if (strcmp(lex_p[0], "version") == 0) {
					UART_PgmSendString(banner);

					/*
					//sprintf(line, "Firmware: %s\n\r%s", version, PROMPTLINE);
					sprintf(line, "Firmware: %s\n\r", version);
					UART_SendString(line);
					UART_SendString(PROMPTLINE);
					*/

					/*
					sprintf(line, "Date: %s\n\r", __DATE__);
					UART_SendString(line);
					sprintf(line, "Time: %s\n\r", __TIME__);
					UART_SendString(line);
					sprintf(line, "Compiler: %s\n\r", __VERSION__);
					UART_SendString(line);
					*/
					COMMAND_DONE;
				}
				#ifdef _DEBUG_
				/* Commands for debugging */
				/* Print register value: getreg <addr, 0-255> */
				else if (strcmp(lex_p[0], "getreg") == 0) {
					sprintf(line, "REG %d (0x%02X): %d\n\r", atoi(lex_p[1]), atoi(lex_p[1]), TWI_GetByte(atoi(lex_p[1])));
					UART_SendString(line);
					COMMAND_DONE;
				}
				/* Set register value: setreg <addr, 0-255>, <val, 0-255> */
				else if (strcmp(lex_p[0], "setreg") == 0) {
					TWI_SetByte(atoi(lex_p[1]), atoi(lex_p[2]));
					COMMAND_DONE;
				}
				#endif
				/* Unrecognized command */
				else {
					//uart_tx_buff = "\n\r";
					//DATA_SEND_READY;
					/*
					if () {
						UART_EememSendString(err_msg_nocommand);
						UART_SendString(PROMPTLINE);
					}
					*/
					COMMAND_DONE;
				}
			}
			if (IS_DATA_TO_SEND) {
				UART_SendString("\r");
				UART_SendString(uart_tx_buff);
				DATA_SEND_DONE;
			}

			/* Check if power on button (ir sensor) is pressed */
			if ((PIND & (1 << PD2)) == 0) {
				_delay_ms(5); /* bounce protection */
				if ((PIND & (1 << PD2)) == 0) {
					/* Check if power on button (ir sensor) is holded */
					if (IS_PWR_BUTTON_HOLD) {
						#ifdef _DEBUG_
						/* Debug mesages */
						UART_SendString("HOLD");
						#endif
						_delay_ms(500);
					}
					else {
						if (IS_POWER_ON) {
							#ifdef _DEBUG_
							/* Debug mesages */
							UART_SendString("OFF");
							#endif
							/* Save current color to eeprom */
							//RGB_EememSaveColor();
							//RGB_SetColor(0,0,0);
							CLR_POWER_ON;
						}
						else {
							#ifdef _DEBUG_
							/* Debug mesages */
							UART_SendString("ON");
							#endif
							/* Restore current color from eeprom */
							//RGB_EememRestoreColor();
							SET_POWER_ON;
						}
						SET_PWR_BUTTON_HOLD;
						_delay_ms(500);
					}
				}
			}
			/* Check if power on button (ir sensor) is released  */
			if (PIND & (1 << PD2)) {
				CLR_PWR_BUTTON_HOLD;
			}

			/* Check alarm flag */
			if ((PINC & (1 << PC3)) == 0) {
				UART_SendString("Alarm!\n\r");
				UART_SendString(PROMPTLINE);
				ALARM1_RESET;
			}

			if (dimmer.enabled ) {
				if ( sys_ticks >= time_stamp + delay ) {

					time_stamp = sys_ticks;

					//sprintf(line, "%d:%d:%d\n\r", dimmer.r_load, dimmer.r_target, dimmer.r_step);
					//	UART_SendString(line);

					if (dimmer.r_load != dimmer.r_target )
						dimmer.r_load += (dimmer.r_step);
					if (dimmer.g_load != dimmer.g_target )
						dimmer.g_load += (dimmer.g_step);
					if (dimmer.b_load != dimmer.b_target )
						dimmer.b_load += (dimmer.b_step);

					red_color = dimmer.r_load < 0 ? 0 : dimmer.r_load / 10;
					green_color = dimmer.g_load < 0 ? 0 : dimmer.g_load / 10;
					blue_color = dimmer.b_load < 0 ? 0 : dimmer.b_load / 10;

					if (dimmer.iter == dimmer.nsteps ) {

						dimmer.iter = 0;
						dimmer.enabled = 0;

					//	red_color = dimmer.r_target / 10;
					//	green_color = dimmer.g_target / 10;
					//	blue_color = dimmer.b_target / 10;

						#ifdef _DEBUG_
						//UART_SendString("stop\n\r");
						#endif
					}
					else {
						dimmer.iter++;
					}

				}
			}

			_delay_us(2);
		}
}
