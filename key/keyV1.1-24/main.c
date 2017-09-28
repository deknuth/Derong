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
#include <avr/iom16.h>

#define LOW           0
#define HIGH          (!LOW)
#define FALSE         0
#define TRUE		  (!FALSE)

#define setbit(x,y) x |= (1<<y)
#define clrbit(x,y) x &= ~(1<<y)

#define SPI_SCS_H	PORTB|=1		// PB0
#define	SPI_SCS_L	PORTB&=~1

#define	SPI_SCK_H	PORTB|=1<<1
#define	SPI_SCK_L	PORTB&=~(1<<1)

#define	SPI_DATA_OUT	DDRB|=1<<3
#define	SPI_DATA_IN   	DDRB&=~(1<<3)
#define	SPI_DATA_H		PORTB|=1<<3
#define	SPI_DATA_L		PORTB&=~(1<<3)
#define	SPI_DATA_HL		PINB&(1<<3)

#define	LED_ON		PORTB|=(1<<1)
#define	LED_OFF		PORTB&=~(1<<1)
#define	LED_BLINK	PORTB^=(1<<1)

#define BAUD	38400

#define T1_OFF { TCCR1B=0; TIMSK&=~(1<<TOIE1); TCNT1=0; }
#define T1_ON { TCCR1B|=1<<CS11; TIMSK|=1<<TOIE1; TCNT1=0; }	// 8 divide
#define R_LEN	16

#define T2_ON	{ TCCR2=1<<CS22|1<<CS20; TIMSK|=1<<TOIE2; TCNT2=0;}	// 128 divide  2.1ms
#define T2_OFF	{ TCCR2=0; TIMSK&=~1<<TOIE2; }


#define RS_595		{ PORTA&=~1; _delay_us(5); PORTA|=1; }	
#define SCK_595_H	PORTA|=(1<<1)
#define SCK_595_L	PORTA&=~(1<<1)
#define RCK_595_H	PORTA|=(1<<2)
#define RCK_595_L	PORTA&=~(1<<2)
#define DATA_595_H	PORTA|=(1<<3)
#define DATA_595_L	PORTA&=~(1<<3)

#define TX_ENA	PORTD|=0x04
#define RX_ENA	PORTD&=~0x04
#define MAX_R_LEN 16

#define BEEP_100MS	{ PORTC|=1<<5; _delay_ms(200); PORTC&=~1<<5; }
#define FEED_DOG	PORTB^=1<<4;
//#define TEST

volatile unsigned char  U0Ready = 0;
volatile unsigned char  U0Count = 0;
unsigned char rBuf[MAX_R_LEN] = {0};
unsigned char sBuf[6] = {0xFB,0x00,0x00,0x00,0x00,0xFF};
unsigned char cBuf[6] = {0xFC,0x00,0x00,0x00,0x00,0xFF};

void PortInit(void)
{
    DDRA = 0B00001111;		// PA4-PA7 (key encode)
    PORTA= 0B00000000;
    PINA = 0x00;

    DDRB = 0B00010100;		// PB4->dog
    PORTB= 0B00000000;		// PB2->595 en
    PINB = 0x00;

    DDRC = 0B00111111;		// PC5->beep  (COL1->COL5 <=> PC0>PC4)
    PORTC= 0B00011111;
    PINC = 0x00;

    DDRD = 0B00000100;		// (ROW1->ROW5 <=> PD3->PD7)	// PD2->485-DR
    PORTD= 0B11111000;
    PIND = 0x00;

}

void UartInit(void)
{
    UBRRH = (F_CPU / BAUD / 16 - 1) / 256;
    UBRRL = (F_CPU / BAUD / 16 - 1) % 256;
    UCSRB = 1<<RXEN | 1<<TXEN | 1<<RXCIE;
    UCSRC = 1<<UCSZ0 | 1<<UCSZ1 | 1<<URSEL;
}


void R485Send(unsigned char* data,unsigned char len)
{
    unsigned char i;
  //  TX_ENA;
    _delay_us(20);
    for(i=0; i<len; i++)
    {
        while(!(UCSRA & (1 << UDRE)));
        UDR = *(data++);
    }
 //   RX_ENA;
}

//volatile unsigned char flag = 0;
/*
ISR(SIG_OVERFLOW1)		//71ms
{
    T1_OFF;
    flag = 1;
}
*/

unsigned char dis[4] = {0};
volatile unsigned char count = 0;
unsigned char tmp;

ISR(SIG_OVERFLOW2)			// UART0 timeout
{
    U0Ready = 1;
    T2_OFF;
}

ISR(USART_RXC_vect)
{
    unsigned char tmp;
    T2_ON;
    tmp = UDR;
    rBuf[U0Count++] = tmp;
    if(U0Count >= MAX_R_LEN)
        U0Count = 0;
}

void Send595(unsigned long data)
{
    int i;
    for(i=0; i<25; i++)
    {
        SCK_595_H;
        if(data&0x800000)
            DATA_595_H;
        else
            DATA_595_L;
        data <<= 1;
        SCK_595_L;
    }
    RCK_595_H;
    _delay_us(1);
    RCK_595_L;
    DATA_595_L;
}

const unsigned long bit[26] = {
    0x01,0x80,0x40,0x20,0x10,0x8,0x4,0x02,
    0x8000,0x4000,0x2000,0x1000,0x800,0x400,0x200,0x100,
    0x400000,0x800000,0x200000,0x100000,0x20000,0x40000,0x80000,0x10000
};
unsigned char key_stat[26] = {0};
unsigned char scan[5] = {0};
unsigned long key_stat_l = 0;
void KeyScan(void)
{
    unsigned char i,j;
    unsigned char temp;
    for(i=0; i<5; i++) //(ROW1->ROW5 <=> PD3->PD7)  (COL1->COL5 <=> PC0>PC4)
    {
        PORTC &= ~(1<<i);
        _delay_ms(5);
        temp = ((PIND&0xF8)>>3);
        _delay_ms(5);
        scan[i] = temp;
        PORTC |= (1<<i);
    }

	for(i=0; i<5; i++)	// scan over
    {
        for(j=0; j<5; j++)
            key_stat[i*5+j] = ((scan[i]>>j)&0x01);
    }
}

int main(void)
{
    unsigned int keyBit;
	unsigned char encode = 0;
	unsigned char i = 0;
    cli();
    PortInit();
	RS_595;
    Send595(0);
    UartInit();
    //    T0Init();
    sei();
	
	PORTC |= 1<<5;
    _delay_ms(150);
	PORTC &= ~1<<5;
	_delay_ms(150);
	PORTC |= 1<<5;
    _delay_ms(150);
	PORTC &= ~1<<5;
	_delay_ms(150);
	PORTC |= 1<<5;
    _delay_ms(150);
	PORTC &= ~1<<5;
	_delay_ms(150);
	
	encode = (PINA>>7)+((PINA&0x40)>>5)+((PINA&0x20)>>3)+((PINA&0x10)>>1);
	FEED_DOG;
	if(encode == 0x0F)
	{
		unsigned char i;
		while(1)
		{
			for(i=0;i<16;i++)
			{
				Send595(bit[i]);
				BEEP_100MS;
				Send595(0);
				_delay_ms(200);
				FEED_DOG;
			}
			_delay_ms(600);
			FEED_DOG;
			_delay_ms(600);
			FEED_DOG;
			_delay_ms(600);
			FEED_DOG;
			
		}
	}
    while(1)
    {	
		FEED_DOG;
        if(U0Ready)
        {
            //if(U0Count == 5)
            {
				if(rBuf[1] == encode)		// encode right
				{
					if(rBuf[0]==0xFA && rBuf[4]==0xFF)
					{
						switch(rBuf[2])
						{
						case 1:			// control
						{
							keyBit = rBuf[3];
							if(keyBit < 24)
							{
								Send595(bit[keyBit]);
								BEEP_100MS;
								Send595(0);
								KeyScan();
								sBuf[1] = rBuf[1];
								sBuf[2] = rBuf[2];
								sBuf[3] = rBuf[3];		// key number
								sBuf[4] = key_stat[keyBit];
								R485Send(sBuf,6);
							}
							break;
						}
						case 2:			// check key state
						{
							cBuf[1] = rBuf[1];		// key board code
							memset(&cBuf[2],0x00,3);
							
							for(i=0;i<24;i++)
								cBuf[4-i/8] |= (key_stat[i]<<(i%8));
								
							R485Send(cBuf,6);
							break;
						}
						default:
							break;
						}
					}
				}
            }
            U0Ready = U0Count = 0;
        }
        KeyScan();
    }
}

