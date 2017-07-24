#include "main.h"
#define DIS_EXTI0	GICR&=~(1<<INT0)
#define ENA_EXTI0	GICR|=1<<INT0
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
    UCSRB = 1<<RXEN | 1<<TXEN ;//| 1<<RXCIE;
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

void EI0Init(void)
{
    MCUCR = 1<<ISC00 | 1<<ISC01;
    GICR = 1<<INT0;
}

void T0Init(void)
{
    TCCR0 |= 1<<CS02;		// 256 divide
    TIMSK |= 1<<TOIE0;
}

unsigned char total = 0;
unsigned char count = 0;
unsigned char T0Flag = 0;
ISR(SIG_OVERFLOW0)		// 88.89ms
{
    if(count++ > total)
    {
        count = 0;
        if(T0Flag)			// 接收间隔
            total = 2;
        else

            total = 3;
        T0Flag ^= 0x01;
    }
}

volatile unsigned char len = 0;
volatile unsigned char flag = 0;
unsigned char value[4] = { 0 };

/*
ISR(INT0_vect)		// GIO1 中断
{
    flag = 1;
}
*/

int main(void)
{
    unsigned char tBuf[4] = {0};
	unsigned char rBuf[16] = {0};
    unsigned char tmp = 0;
    cli();
    PortInit();
    UartInit();
	T0Init();
	LED_BLINK;
	_delay_ms(200);
	LED_BLINK;
	_delay_ms(200);
//	EI0Init();
	sei();
    tBuf[0]=0xFE;
    tBuf[1]=0x02;
    tBuf[2]=0xAB;
    tBuf[3]=0xCD;
	unsigned char cid[4] = {0};
    do
    {
        tmp = A7139_Init(470.001f); 		// A7139初始化
        _delay_ms(100);
    }while(tmp);
    A7139_SetPowerLevel(8);		// 功率设置函数，参数范围0-8，0最小功率，8功率最大
    A7139_SetPackLen(4);
	A7139_ReadCID(cid);
	U0Send(cid,4);
    sei();
//	DIS_EXTI0;
    while(1)
    {
		while(T0Flag)
        {
            StrobeCmd(CMD_RX);
            while(!GIO1 && T0Flag);
            {
                if(T0Flag)
                {
                    if(RecvData(rBuf)>0)
                    {
//						U0Send("read:",5);
						U0Send(rBuf,2);
                    }
                }
            }
            StrobeCmd(CMD_STBY);
        }
        A7139Send(tBuf,4);
		U0Send(tBuf,4);
    }
}

