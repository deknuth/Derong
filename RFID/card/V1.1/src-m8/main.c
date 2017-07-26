#include "main.h"
#define DIS_EXTI0	GICR&=~(1<<INT1)
#define ENA_EXTI0	GICR|=(1<<INT1)
#define BLINK	PORTD^=0x04
#define LED_ON	PORTD|=0x04
#define LED_OFF	PORTD&=~0x04

#define T1_ON	{ TCCR1B|=1<<CS12; TIMSK|=1<<TOIE1; TCNT1=0xC000;}
#define T1_OFF	{ TCCR1B=0; TIMSK&=~1<<TOIE1;}

volatile unsigned char cFlag = 1;		// count Flag
void PortInit(void)
{
    DDRB = 0B00000111;
    PORTB= 0B11111000;
    PINB = 0x00;

    DDRD = 0B00000100;	
    PORTD= 0B11111011;
    PIND = 0x00;

    DDRC = 0B00000000;			
    PORTC= 0B11111111;
    PINC = 0x00;
}

void UartInit(void)
{
    UBRRH = (F_CPU / BAUD / 16 - 1) / 256;
    UBRRL = (F_CPU / BAUD / 16 - 1) % 256;
    UCSRB = 1<<RXEN | 1<<TXEN | 1<<RXCIE;
    UCSRC = 1<<UCSZ0 | 1<<UCSZ1 | 1<<URSEL;
}

void SendData(unsigned char *buf,unsigned char len)
{
    A7139_WriteFIFO(buf,len);
    StrobeCmd(CMD_TX);
    _delay_us(10);
    while(GIO2);
}

unsigned char RecvData(unsigned char* buf)
{
    StrobeCmd(CMD_RX);
    _delay_us(10);
    while(GIO2);
    return A7139_ReadFIFO(buf);
}

void SendStr(unsigned char* data,unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        while(!(UCSRA & (1 << UDRE)));
        UDR = *(data++);
    }
}

void SetSleep(void)
{
    MCUCR |= 1<<SM1;
    MCUCR |= 1<<SE;
    asm("sleep");
}

void EI0Init(void)
{
    MCUCR = 0;
    GICR = 1<<INT1;
}

volatile unsigned char len = 0;
volatile unsigned char flag = 0;
unsigned int value = 0;
unsigned char rBuf[16] = {0};

ISR(INT1_vect)		// GIO1 中断
{
    //	value= A7139_GetRSSI();
    len = RecvData(rBuf);
    flag = 1;
}

/*
ISR(SIG_OVERFLOW1)		// 9s
{
	cFlag = 0;
	T1_OFF;
	ENA_EXTI0;
}
*/
unsigned int nId = 0x05;
int main(void)
{
    unsigned char tBuf[7] = {0xFC,0x07,0x00,0x00,0x00,0x00,0xFF};
    unsigned char tmp = 0;
	unsigned char i = 0;
    unsigned int count = 0;
	unsigned int rId = 0;
	tBuf[4] = (nId>>8);
	tBuf[5] = (nId&0xFF);
    PortInit();
	LED_ON;
	_delay_ms(300);
	LED_OFF;
	LED_ON;
	_delay_ms(300);
	LED_OFF;
    EI0Init();
    do
    {
        tmp = A7139_Init(470.001f); 		// A7139初始化
        _delay_ms(100);
    }while(tmp);

    A7139_SetPowerLevel(6);		// 功率设置函数，参数范围0-8，0最小功率，8功率最大
    A7139_SetPackLen(16);
    A7139_WOR_ByPreamble();
    sei();		
    StrobeCmd(CMD_RX);
    SetSleep();	
    while(1)
    {
        if(flag)
        {
            if(len==7)
            {
				rId = (rBuf[4]<<8)+rBuf[5];
				if(rId==nId)
				{
					DIS_EXTI0;
					_delay_ms(60+nId);
					tBuf[2] = rBuf[2];
					tBuf[3] = rBuf[3];
					SendData(tBuf,7);
					if(rBuf[2] == 0)
					{
						for(i=0;i<27;i++)
						{
							if(i%2)
							{
								LED_ON;
								_delay_ms(30);
							}
							else
							{
								LED_OFF;
								_delay_ms(1300);
							}
						}
						_delay_ms(3000);
						LED_OFF;
					}
					else
						_delay_ms(2000);
					ENA_EXTI0;
				}

				else if(rId==65535)		// broadcast
				{
					DIS_EXTI0;
					_delay_ms(60+nId);
					SendData(tBuf,7);
					_delay_ms(3000);
					_delay_ms(9000);
					_delay_ms(9000);
					ENA_EXTI0;
				}
            }
            flag = 0;
            A7139_WOR_ByPreamble();
            StrobeCmd(CMD_RX);
            SetSleep();
        }
        else if(count++>1000)
        {
			count = 0;
			SetSleep();
        }
    }
}


