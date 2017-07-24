#include "main.h"
#define DIS_EXTI0	GICR&=~(1<<INT1)
#define ENA_EXTI0	GICR|=(1<<INT1)
Uint8 tmpbuf[16] = {0}; //数组用于存放接收到的数据

void PortInit(void)
{
    DDRB = 0B00001111;
    PORTB= 0B11110000;
    PINB = 0x00;

    DDRD = 0B00000000;	
    PORTD= 0B11111111;
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

unsigned char RecvData(void)
{
    StrobeCmd(CMD_RX);
    _delay_us(10);
    while(GIO2);
    return A7139_ReadFIFO();
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
    MCUCR = 0;//1<<ISC00 ;//| 1<<ISC01;
    GICR = 1<<INT1;
}

volatile unsigned char len = 0;
volatile unsigned char flag = 0;
unsigned int value = 0;

ISR(INT1_vect)		// GIO1 中断
{
    //	LED_BLINK;
    //	value= A7139_GetRSSI();
    len = RecvData();
    /*
    if(len > 0)
    {
        SendStr(tmpbuf,len);
    }
        */
    flag = 1;
}

int main(void)
{
    unsigned char TX_BUF[4] = {0};
    unsigned char tmp = 0;
    unsigned int count = 0;
    PortInit();
    EI0Init();
    UartInit();
    do
    {
        tmp = A7139_Init(470.001f); 		// A7139初始化
        _delay_ms(100);
    }while(tmp);

    A7139_SetPowerLevel(6);		// 功率设置函数，参数范围0-8，0最小功率，8功率最大
    A7139_SetPackLen(16);
    A7139_WOR_ByPreamble();		// del
 
    sei();						// del
    StrobeCmd(CMD_RX);			// del
    SetSleep();					// del
    while(1)
    {
        if(flag)
        {
            if(len == 2)
            {
                TX_BUF[0] = 0xFE;
                TX_BUF[1] = 0x02;
                TX_BUF[2] = tmpbuf[0];
                TX_BUF[3] = tmpbuf[1];
                DIS_EXTI0;
                _delay_ms(120);
                SendData(TX_BUF,4);
                ENA_EXTI0;
                LED_BLINK;
            }
            flag = 0;
            A7139_WOR_ByPreamble();
            StrobeCmd(CMD_RX);
            SetSleep();
        }
        else if(count++ > 1000)
        {
            count = 0;
            SetSleep();
        }
    }
}


