#include "main.h"
#define DIS_EXTI0	GICR&=~(1<<INT0)
#define ENA_EXTI0	GICR|=1<<INT0

#define T2_ON	{ TCCR2B|=1<<CS22|1<<CS20; TIMSK2|=1<<TOIE2; TCNT2=0;}	// 128 divide  4.4ms
#define T2_OFF	{ TCCR2B=0; TIMSK2&=~1<<TOIE2; }

#define T1_ON	{ TCCR1B|=1<<CS12|1<<CS10; TIMSK1|=1<<TOIE1; TCNT1=0x0000;}
#define T1_OFF	{ TCCR1B=0; TIMSK1&=~1<<TOIE1;}

#define T0_ON	{ TCCR0B|=1<<CS02; TIMSK0|=1<<TOIE0;}
#define T0_OFF	{ TCCR0B=0; TIMSK0&=~1<<TOIE0;}

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
    UBRR0H = (F_CPU / BAUD / 16 - 1) / 256;
    UBRR0L = (F_CPU / BAUD / 16 - 1) % 256;
    UCSR0B = 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0;
    UCSR0C = 1<<UCSZ00 | 1<<UCSZ01;
}

void A7139Send(unsigned char *buf,unsigned char len)
{
    A7139_WriteFIFO(buf,len);
    StrobeCmd(CMD_TX);
    _delay_us(10);
    while(GIO2);
}
volatile unsigned char sFlag = 0;
unsigned char RecvData(unsigned char *buf)
{
    StrobeCmd(CMD_RX);
    _delay_us(10);
    while(GIO2)
	{
		if(sFlag==0)
			return 0;
	}
    return A7139_ReadFIFO(buf);
}

void U0Send(unsigned char* data,unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        while(!(UCSR0A & (1 << UDRE0)));
        UDR0 = *(data++);
    }
}

#define MAX_R_LEN	16
unsigned char uBuf[20] = {0};
unsigned char tBuf[7] = {0xFE,0x07,0xFF,0xFF,0xFF,0xFF,0xFF};
volatile unsigned char U0Count = 0;

volatile unsigned char U0Ready = 0;

ISR(TIMER2_OVF_vect)			// UART0 timeout
{
    T2_OFF;
	if(U0Count == 7)
	{
		memcpy(tBuf,uBuf,7);
		U0Ready = 1;
	}
	else
		memset(uBuf,0x00,U0Count);
	U0Count = 0;
}

ISR(USART_RX_vect)
{
	T2_ON;
    unsigned char tmp;
    tmp = UDR0;
    uBuf[U0Count++] = tmp;
    if(U0Count >= 16)
        U0Count = 0;
	
}

volatile unsigned char total = 15;
unsigned char count = 0;
volatile unsigned char T0Flag = 0;

ISR(TIMER0_OVF_vect)		// 8.85ms
{
    if(count++ > total)
    {
        count = 0;
        if(T0Flag)			// 接收间隔
            total = 6;
        else
            total = 15;
        T0Flag ^= 0x01;
		LED_BLINK;
    }
}

volatile unsigned char t1Count = 0;
void StopScan(void)
{
	T1_OFF;
	T0_OFF;
	T0Flag = sFlag = 0;
	LED_OFF;
	t1Count=0;
}

ISR(TIMER1_OVF_vect)		// 18s
{
	if(t1Count++ > 0)
		StopScan();
}

int main(void)
{
	unsigned char rBuf[16] = {0};
    unsigned char tmp = 0;
	unsigned char rLen = 0;
	unsigned char cid[4] = {0};
	unsigned char bcf = 0;
    cli();
    PortInit();
    UartInit();
	LED_BLINK;
	_delay_ms(200);
	LED_BLINK;
	
    do
    {
        tmp = A7139_Init(470.001f); 		// A7139初始化
        _delay_ms(100);
    }while(tmp);
    A7139_SetPowerLevel(8);		// 功率设置函数，参数范围0-8，0最小功率，8功率最大
    A7139_SetPackLen(7);
	A7139_ReadCID(cid);
//	U0Send(cid,4);
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
							if(rLen==7)
							{
								if(rBuf[0]==0xFC && rBuf[6]==0xFF)
								{
									U0Send(rBuf,rLen);
									memset(rBuf,0x00,rLen);
						//			if(bcf)
						//				StopScan();
								}
							}
						}
					}
				}
				StrobeCmd(CMD_STBY);
			}
			A7139Send(tBuf,7);
		}
		if(U0Ready)
		{
			if(tBuf[0]==0xFE && tBuf[6]==0xFF && tBuf[1]==0x07)
			{
				if(tBuf[2]==0xFF && tBuf[3]==0xFF && tBuf[4]==0xFF && tBuf[5]==0xFF)
					bcf = 0;
				else
					bcf = 1;
				T1_ON;
				T0_ON;
				sFlag = 1;
			}
			U0Ready = 0;
		}
    }
}

