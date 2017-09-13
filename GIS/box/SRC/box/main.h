#ifndef MAIN_H_
#define MAIN_H_
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <math.h>
#include <util/delay.h>
#include <util/twi.h>
#include "A7139.h"

#define LOW           0
#define HIGH          (!LOW)
#define FALSE         0
#define TRUE		  (!FALSE)	

#define setbit(x,y) x |= (1<<y)
#define clrbit(x,y) x &= ~(1<<y)

#define SPI_SCS_H	PORTA|=1
#define	SPI_SCS_L	PORTA&=~1

#define	SPI_SCK_H	PORTA|=1<<1
#define	SPI_SCK_L	PORTA&=~(1<<1)

#define	SPI_DATA_OUT	DDRA|=1<<2
#define	SPI_DATA_IN   	DDRA&=~(1<<2)
#define	SPI_DATA_H		PORTA|=1<<2
#define	SPI_DATA_L		PORTA&=~(1<<2)
#define	SPI_DATA_HL		PINA&(1<<2)

#define	LED_ON		PORTB|=(1<<1)
#define	LED_OFF		PORTB&=~(1<<1)
#define	LED_BLINK	PORTB^=(1<<1)

#define GIO2   (PIND&0x10)
#define GIO1   (PIND&0x04)
#define U0BAUD	9600
#define U1BAUD	115200

#define T1_OFF { TCCR1B=0; TIMSK&=~(1<<TOIE1); }
#define T1_ON { TCCR1B|=1<<CS12|1<<CS10; TIMSK|=1<<TOIE1; }
#define R_LEN	16
extern Uint8 tmpbuf[];

#endif
