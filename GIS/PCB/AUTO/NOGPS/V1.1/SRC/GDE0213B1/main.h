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
#define LOW           0
#define HIGH          (!LOW)
#define FALSE         0
#define TRUE		  (!FALSE)	

#define setbit(x,y) x |= (1<<y)
#define clrbit(x,y) x &= ~(1<<y)

#define BAUD	115200
extern void SendStr(unsigned char* data,unsigned char len);
extern int kmp(const char *Text,const char* Pattern);
extern void StringToHex(const unsigned char *src,unsigned char *dst,unsigned char sLen)	;
extern void GetTemper(void);
extern void display(int integ , unsigned char dec);
extern unsigned char num[];
extern unsigned char DS18B20_Init(void);
extern void Write_18b20(unsigned char dat);

#endif

