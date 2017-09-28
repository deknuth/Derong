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

#define BAUD	38400

#define T1_OFF { TCCR1B=0; TIMSK&=~(1<<TOIE1); TCNT1=0; }
#define T1_ON { TCCR1B|=1<<CS11; TIMSK|=1<<TOIE1; TCNT1=0; }	// 8 divide
#define R_LEN	16

#define T2_ON	{ TCCR2B=1<<CS22|1<<CS20; TIMSK2|=1<<TOIE2; TCNT2=0;}	// 128 divide  2.1ms
#define T2_OFF	{ TCCR2B=0; TIMSK2&=~1<<TOIE2; }


#define RS_595		{ PORTC&=~8; _delay_us(5); PORTC|=8; }	
#define SCK_595_H	PORTC|=4
#define SCK_595_L	PORTC&=~4
#define RCK_595_H	PORTC|=2
#define RCK_595_L	PORTC&=~2
#define DATA_595_H	PORTC|=1			// PC0
#define DATA_595_L	PORTC&=~1
#define EN_4051		PORTC&=~0x10
#define DIS_4051	PORTC|=0x10

#define TX_ENA	PORTD|=0x04
#define RX_ENA	PORTD&=~0x04
#define MAX_R_LEN 16

#define BEEP_100MS	{ PORTB|=0x20; _delay_ms(200); PORTB&=~0x20; }
#define FEED_DOG	PORTC^=0x20;
#define TEST

volatile unsigned char  U0Ready = 0;
volatile unsigned char  U0Count = 0;
unsigned char rBuf[MAX_R_LEN] = {0};
unsigned char sBuf[6] = { 0xFB,0x00,0x00,0x00,0x00,0xFF };
unsigned char cBuf[6] = { 0xFC,0x00,0x00,0x00,0x00,0xFF };

void PortInit(void)
{
    DDRB = 0B00110111;		// PB4->595_OE PB5->beep  PB1->IN1A PB2->IN1B PB3->IN1C
    PORTB= 0B00000000;		// PB2->595_EN
    PINB = 0x00;

    DDRC = 0B00111111;		// PC5->dog PC4->4051_EN
    PORTC= 0B00001111;
    PINC = 0x00;

    DDRD = 0B00000100;		// PD2->485-DR  PD3->SIG
    PORTD= 0B00000000;
    PIND = 0x00;
}

void UartInit(void)
{
    UBRR0H = (F_CPU / BAUD / 16 - 1) / 256;
    UBRR0L = (F_CPU / BAUD / 16 - 1) % 256;
    UCSR0B = 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0;
    UCSR0C = 1<<UCSZ00 | 1<<UCSZ01;
}

void R485Send(unsigned char* data,unsigned char len)
{
    unsigned char i;
  //  TX_ENA;
    _delay_us(20);
    for(i=0; i<len; i++)
    {
        while(!(UCSR0A & (1 << UDRE0)));
        UDR0 = *(data++);
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


ISR(TIMER2_OVF_vect)			// UART0 timeout
{
    U0Ready = 1;
    T2_OFF;
}

ISR(USART_RX_vect)
{
    unsigned char tmp;
    T2_ON;
    tmp = UDR0;
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
    0x80,0x40,0x20,0x10,0x8,0x4,0x02,0x01,
    0x8000,0x4000,0x2000,0x1000,0x800,0x400,0x200,0x100,
    0x400000,0x800000,0x200000,0x100000,0x20000,0x40000,0x80000,0x10000
};
unsigned char key_stat[8] = {0};

void KeyScan(void)
{
    unsigned char i;
    EN_4051;
    PORTB &= 0xF8;
    for(i=0; i<8; i++) 
    {	
		PORTB |= i;
		_delay_us(1);
		if(PIND&0x08)
			key_stat[i] = 1;
		else
			key_stat[i] = 0;
		PORTB &= ~i;
    }
  	DIS_4051;
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
    sei();
	
	for(i=0; i<3; i++)	
	{
		BEEP_100MS;
		_delay_ms(150);
	}
	
	encode = ((PIND&0xF0)>>4);		// ВІТы
	
	if(encode == 0xF)
	{
		while(1)
		{
			for(i=0;i<8;i++)
			{
				Send595(bit[i]);
				BEEP_100MS;
				Send595(0);
				_delay_ms(100);
				FEED_DOG;
			}
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
							if(keyBit < 8)
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
							for(i=0;i<8;i++)
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

