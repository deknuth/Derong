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


#define SPI_SCS_H	PORTB|=0x04
#define	SPI_SCS_L	PORTB&=~0x04

#define	SPI_SCK_H	PORTB|=0x02
#define	SPI_SCK_L	PORTB&=~0x02

#define	SPI_DATA_OUT	DDRB|=1
#define	SPI_DATA_IN   	DDRB&=~1
#define	SPI_DATA_H	PORTB|=1
#define	SPI_DATA_L	PORTB&=~1
#define	SPI_DATA_HL	(PINB&0x01)

#define	LED_ON		PORTD|=(1<<5)
#define	LED_OFF		PORTD&=~(1<<5)
#define	LED_BLINK	PORTD^=(1<<5)

#define GIO2   (PIND&0x40)
#define GIO1   (PIND&0x80)

#define BAUD	115200

extern void SendStr(unsigned char* data,unsigned char len);
extern Uint8 tmpbuf[];
#define R_LEN	16
#endif



