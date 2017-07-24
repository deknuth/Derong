#include "main.h"
#define DIS_EXTI0	GICR&=~(1<<INT0)
#define ENA_EXTI0	GICR|=1<<INT0
void PortInit(void)
{
    DDRB = 0B00001111;		// PB3->BL-PWM	(BACK Light)
    PORTB= 0B00000000;
    PINB = 0x00;

    DDRD = 0B00100000;		// PD5->led
    PORTD= 0B00000000;
    PIND = 0x00;

    DDRC = 0B00000001;		// PCO->BL-EN,PC1->GIO1,PC2->GIO2			
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
        while(!(UCSR0A & (1 << UDRE0)));
        UDR0 = *(data++);
    }
}

void EI0Init(void)
{
    MCUCR = 1<<ISC00 | 1<<ISC01;
    EIMSK = 1<<INT0;
}

void T0Init(void)
{
    TCCR0 |= 1<<CS02;		// 256 divide
    TIMSK |= 1<<TOIE0;
}

volatile unsigned char total = 3;
unsigned char count = 0;
volatile unsigned char T0Flag = 1;
ISR(SIG_OVERFLOW0)		// 88.89ms
{
    if(count++ > total)
    {
        count = 0;
        if(T0Flag)			// 接收间隔
		{
            total = 2;
			LED_OFF;
		}
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
    unsigned char tBuf[5] = {0xFE,0x05,0x00,0x01,0xFF};
	unsigned char rBuf[16] = {0};
    unsigned char tmp = 0;
	unsigned char rLen = 0;
    cli();
    PortInit();
    UartInit();
	T0Init();
	LED_BLINK;
	_delay_ms(200);
	LED_BLINK;
	
//	EI0Init();

	unsigned char cid[4] = {0};
    do
    {
        tmp = A7139_Init(470.001f); 		// A7139初始化
        _delay_ms(100);
    }while(tmp);
    A7139_SetPowerLevel(8);		// 功率设置函数，参数范围0-8，0最小功率，8功率最大
    A7139_SetPackLen(16);
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
                    if((rLen=RecvData(rBuf))>0)
                    {
						LED_ON;
						U0Send(rBuf,rLen);
                    }
                }
            }
            StrobeCmd(CMD_STBY);
        }

        A7139Send(tBuf,5);

	/*
		if(flag)
		{
			flag = 0;
			len = RecvData(rBuf);
			if(len > 0)
				U0Send(rBuf,len);
			StrobeCmd(CMD_STBY);
		}
		*/
    }
}

