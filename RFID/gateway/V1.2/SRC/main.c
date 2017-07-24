#include "main.h"
#define DIS_EXTI0	GICR&=~(1<<INT0)
#define ENA_EXTI0	GICR|=1<<INT0

#define T2_ON	{ TCCR2|=1<<CS22|1<<CS20; TIMSK|=1<<TOIE2; TCNT2=0;}	// 128 divide  4.4ms
#define T2_OFF	{ TCCR2=0; TIMSK&=~1<<TOIE2; }
#define T1_ON	{ TCCR1B|=1<<CS12|1<<CS10; TIMSK|=1<<TOIE1; TCNT1=0x8000;}
#define T1_OFF	{ TCCR1B=0; TIMSK&=~1<<TOIE1;}
#define T0_ON	{ TCCR0|=1<<CS12; TIMSK|=1<<TOIE0;}
#define T0_OFF	{ TCCR0=0; TIMSK&=~1<<TOIE0;}

void PortInit(void)
{
    DDRB = 0B00000100;		// PB0->GIO2
    PORTB= 0B00000000;
    PINB = 0x00;

    DDRD = 0B00000000;		// PD2->GIO1
    PORTD= 0B00000000;
    PIND = 0x00;

    DDRC = 0B00000111;			
    PORTC= 0B00000000;
    PINC = 0x00;
}

void UartInit(void)
{
    UBRRH = (F_CPU / BAUD / 16 - 1) / 256;
    UBRRL = (F_CPU / BAUD / 16 - 1) % 256;
    UCSRB = 1<<RXEN | 1<<TXEN | 1<<RXCIE;
    UCSRC = 1<<UCSZ0 | 1<<UCSZ1 | 1<<URSEL;
}

void A7139Send(unsigned char *buf,unsigned char len)
{
    A7139_WriteFIFO(buf,len);
    StrobeCmd(CMD_TX);
    _delay_us(10);
    while(GIO2);
}

unsigned char RecvData(unsigned char *buf)
{
    StrobeCmd(CMD_RX);
    _delay_us(10);
    while(GIO2);
    return A7139_ReadFIFO(buf);
}

void U0Send(unsigned char* data,unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        while(!(UCSRA & (1 << UDRE)));
        UDR = *(data++);
    }
}

#define MAX_R_LEN	16
unsigned char uBuf[20] = {0};
volatile unsigned char U0Count = 0;
volatile unsigned char sFlag = 0;
volatile unsigned char U0Ready = 0;

ISR(SIG_OVERFLOW2)			// UART0 timeout
{
    T2_OFF;
	U0Ready = 1;
}

ISR(USART_RXC_vect)
{
    unsigned char tmp;
    tmp = UDR;
    uBuf[U0Count++] = tmp;
	T2_ON;
    if(U0Count >= 16)
        U0Count = 0;
}

volatile unsigned char total = 15;
unsigned char count = 0;
volatile unsigned char T0Flag = 0;

ISR(SIG_OVERFLOW0)		// 8.85ms
{
    if(count++ > total)
    {
        count = 0;
        if(T0Flag)			// 接收间隔
		{
            total = 6;
			LED_OFF;
		}
        else
            total = 15;
        T0Flag ^= 0x01;
    }
}

ISR(SIG_OVERFLOW1)		// 9s
{
	sFlag = 0;
	T1_OFF;
	T0Flag = 0;
	T0_OFF;
}

int main(void)
{
	unsigned char rBuf[16] = {0};
    unsigned char tmp = 0;
	unsigned char rLen = 0;
	unsigned char cid[4] = {0};
	unsigned char tBuf[5] = {0xFE,0x05,0xFF,0xFF,0xFF};
    cli();
    PortInit();
    UartInit();
//	T0Init();
	LED_BLINK;
	_delay_ms(200);
	LED_BLINK;
    do
    {
        tmp = A7139_Init(470.001f); 		// A7139初始化
        _delay_ms(100);
    }while(tmp);
    A7139_SetPowerLevel(8);		// 功率设置函数，参数范围0-8，0最小功率，8功率最大
    A7139_SetPackLen(5);
	A7139_ReadCID(cid);
	U0Send(cid,4);
	sei();

    while(1)
    {
		if(sFlag)
		{
			while(T0Flag)
			{
				StrobeCmd(CMD_RX);
				while(!GIO1 && T0Flag);
				{
					if(T0Flag)
					{
						if((rLen=RecvData(rBuf))>0)
						{
							LED_ON;
							U0Send(rBuf,rLen);
							memset(rBuf,0x00,rLen);
						}
					}
				}
				StrobeCmd(CMD_STBY);
			}
			A7139Send(tBuf,5);
		}
		if(U0Ready)
		{
			if(U0Count==5)
			{
				if(uBuf[0]==0xFE && uBuf[4]==0xFF && uBuf[1]==0x05)
				{
					memcpy(tBuf,uBuf,5);
					T1_ON;
					T0_ON;
					memset(uBuf,0x00,U0Count);
					U0Count = 0;
					sFlag = 1;
				}
			}
			else
			{
				memset(uBuf,0x00,U0Count);
				U0Count = 0;
			}
			U0Ready = 0;
		}
    }
}

