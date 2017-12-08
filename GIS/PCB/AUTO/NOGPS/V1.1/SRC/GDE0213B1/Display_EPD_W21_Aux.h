#ifndef _DISPLAY_EPD_W21_AUX_H_
#define _DISPLAY_EPD_W21_AUX_H_
#include "main.h"

#define EPD_W21_MOSI_0	PORTB&=~(1<<5)
#define EPD_W21_MOSI_1	PORTB|=1<<5

#define EPD_W21_CLK_0	PORTB&=~(1<<7)
#define EPD_W21_CLK_1	PORTB|=1<<7

#define EPD_W21_CS_0	PORTB&=~(1<<4)
#define EPD_W21_CS_1	PORTB|=1<<4

#define EPD_W21_DC_0	PORTB&=~(1<<2)
#define EPD_W21_DC_1	PORTB|=1<<2

#define EPD_W21_RST_0	PORTB&=~(1<<1)
#define EPD_W21_RST_1	PORTB|=1<<1

#define EPD_W21_BUSY_LEVEL 0
#define isEPD_W21_BUSY (PINB&0x1) 

void SPI_Write(unsigned char value);

#endif

