#include "main.h"
#define DIS_EXTI0	GICR&=~(1<<INT0)
#define ENA_EXTI0	GICR|=(1<<INT0)

#define GSM_ON	{ PORTC|=0x04; _delay_ms(2200); PORTC&=~(0x04); }			// PC2->GSM_PW
#define D1_BLINK	PORTA^=0x08

struct latlon{
    unsigned char latFlag;
    unsigned char lonFlag;
    unsigned char valid;
    unsigned char lat[12];
    unsigned char lon[13];
    unsigned char time[12];
};

struct latlon coor;

Uint8 tmpbuf[R_LEN] = {0}; //数组用于存放接收到的数据

void PortInit(void)
{
    DDRA = 0B00001111;			// PA0->SCS_7139 PA1->SCK_7139 PA2->SDA_7139 PA3->D1 PA7-VSC
    PORTA= 0B00000000;
    PINA = 0x00;

    DDRB = 0B00000010;			// PB1->LED
    PORTB= 0B00000010;
    PINB = 0x00;

    DDRC = 0B00011111;
    PORTC= 0B00000000;
    PINC = 0x00;

    DDRD = 0B00000000;
    PORTD= 0B00000000;
    PIND = 0x00;
}
/*
void Uart0Init(void)
{
    UBRR0H = (F_CPU / U0BAUD / 16 - 1) / 256;
    UBRR0L = (F_CPU / U0BAUD / 16 - 1) % 256;
    UCSR0B = 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0;
    UCSR0C = 1<<UCSZ00 | 1<<UCSZ01;
}

void Uart1Init(void)
{
    UBRR1H = (F_CPU / U1BAUD / 16 - 1) / 256;
    UBRR1L = (F_CPU / U1BAUD / 16 - 1) % 256;
    UCSR1B = 1<<RXEN1 | 1<<TXEN1 | 1<<RXCIE1;
    UCSR1C = 1<<UCSZ10 | 1<<UCSZ11;
}

void EI6Init(void)
{
    EICRB |= 1<<ISC61;
    EIMSK |= 1<<INT6;
}

void T0Init(void)
{
    TCCR0 |= 1<<CS02 | 1<<CS00 | 1<<CS01;		// 1024 divide
    TIMSK |= 1<<TOIE0;
}

void T3Init(void)
{
    TCCR3A = 0;
    TCCR3B |= 1<<CS32 ;		// 256 divide
    ETIMSK |= 1<<TOIE3;
}

void A7139Send(unsigned char *buf,unsigned char len)
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

void U1Send(unsigned char* data,unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        while(!(UCSR1A & (1 << UDRE1)));
        UDR1 = *(data++);
    }
}

unsigned char len = 0;
unsigned char count = 0;
volatile unsigned char flag = 0;
volatile unsigned char T0Flag = 1;
volatile unsigned char total = 5;
volatile unsigned char T1Count = 0;
volatile unsigned char SendFlag = 1;


ISR(SIG_OVERFLOW0)		// 32.64ms
{
    if(count++ > total)
    {
        count = 0;
        if(T0Flag)			// 接收间隔
            total = 4;
        else
            total = 5;
        T0Flag ^= 0x01;
    }
}

ISR(SIG_OVERFLOW1)			// 9.1s one time 
{
    if(++T1Count > 5)			// 45S
    {
        SendFlag = 1;
        T1Count = 0;
        T1_OFF;
    }
}
*/
const unsigned char ocHex[58]={
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,
    8,9
};

void StringToHex(const unsigned char *src,unsigned char *dst,unsigned char sLen)	// ACII to hex 	only 0-F
{
    int i,j;
    const unsigned ASCValue[128] = {
        0,0,0,0,0,0,0,0,0,0,		// 0-9
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,1,
        2,3,4,5,6,7,8,9,0,0,
        0,0,0,0,0,10,11,12,13,14,
        15,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,10,11,12,
        13,14,15,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0
    };
    for(i = 0,j = 0; i < sLen;)
    {
        dst[j] = ASCValue[src[i++]];
        dst[j] <<= 4;
        dst[j++] += ASCValue[src[i++]];
    }
}

void Str2Hex(unsigned char *src,unsigned char *dst)
{
    int i;
    for(i=0;i<12;i++)
    {
        if(*(src+i) == '.')
            break;
    }
    if(i==5)
    {
        dst[0] = ocHex[src[0]]*100 + ocHex[src[1]]*10 + ocHex[src[2]];
        dst[1] = (ocHex[src[3]]<<4) + ocHex[src[4]];
        dst[2] = (ocHex[src[6]]<<4) + ocHex[src[7]];
        dst[3] = (ocHex[src[8]]<<4) + ocHex[src[9]];
        dst[4] = (ocHex[src[10]]<<4) + ocHex[src[11]];
    }
    else if(i==4)
    {
        dst[0] = (ocHex[src[0]])*10 + ocHex[src[1]];
        dst[1] = (ocHex[src[2]]<<4) + ocHex[src[3]];
        dst[2] = (ocHex[src[5]]<<4) + ocHex[src[6]];
        dst[3] = (ocHex[src[7]]<<4) + ocHex[src[8]];
        dst[4] = (ocHex[src[9]]<<4) + ocHex[src[10]];
    }
}

volatile unsigned char T3Count = 0;
unsigned char info[18] = {	0xFE,0x12,0x02,0x00,0x00,0x00,
                                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF };
/*
ISR(SIG_OVERFLOW3)
{
    T3Count++;
    if(T3Count > 14)
    {
        //   if(coor.valid == 'A')
        {
            Str2Hex(coor.lat,&info[3]);
            Str2Hex(coor.lon,&info[8]);
            StringToHex(coor.time,&info[13],6);
            U1Send(info,18);
            memset(&coor,0x00,sizeof(coor));
            info[16] = 0x00;
        }
        T3Count = 0;
        UCSR0B &= ~(1<<RXCIE0);
    }
    else if(T3Count > 13)
    {
        UCSR0B |= 1<<RXCIE0;
    }

}

unsigned char SegCount = 0;
unsigned char headFlag = 0;
unsigned char sbit = 0;
char head[6] = {0};
unsigned char lac,loc,tc,hc = 0;

ISR(USART1_RX_vect)
{
    unsigned char tmp;
    tmp = UDR1;
}


ISR(USART0_RX_vect)
{
    unsigned char tmp;
    tmp = UDR0;
    switch(tmp)
    {
    case '$':
        headFlag = 1;
        hc = 0;
        SegCount = 0;
        break;
    case ',':
        SegCount++;
        if(SegCount == 1)
        {
            if(strcmp(head,"GNRMC") == 0)
            {
                hc = loc = lac = tc = 0;
                headFlag = 0;
                sbit = 1;
                memset(&coor,0x00,sizeof(coor));
            }
            else
            {
                SegCount = 0;
                sbit = 0;
            }
        }
        break;
    default:
    {
        if(headFlag)
        {
            if(hc<5)
                head[hc++] = tmp;
        }
        else if(sbit)
        {
            switch(SegCount)
            {
            case 1:
                coor.time[tc++] = tmp;
                break;
            case 2:
                coor.valid = tmp;
                break;
            case 3:
                coor.lat[lac++] = tmp;
                break;
            case 4:
                coor.latFlag = tmp;
                break;
            case 5:
                coor.lon[loc++] = tmp;
                break;
            case 6:
                coor.lonFlag = tmp;
                break;
            default:
                break;
            }
        }
    }
        break;
    }
}
*/

int main(void)
{
    unsigned char TX_BUF[4] = {0};
    unsigned char tmp = 0;
    cli();
    PortInit();
    //Uart0Init();
    //Uart1Init();
	GSM_ON;
	while(1)
	{
		D1_BLINK;
		_delay_ms(500);
	}

	/*
    //	EI0Init();
    TX_BUF[0]=0xFE;
    TX_BUF[1]=0x02;
    TX_BUF[2]=0xAB;
    TX_BUF[3]=0xCD;
    do
    {
        tmp = A7139_Init(470.001f); 		// A7139初始化
        _delay_ms(100);
    }while(tmp);
    A7139_SetPowerLevel(8);		// 功率设置函数，参数范围0-8，0最小功率，8功率最大
    A7139_SetPackLen(4);

    LED_BLINK;
    _delay_ms(500);
    LED_BLINK;
    _delay_ms(500);
    LED_BLINK;
    T0Init();
    sei();
    T3Init();

    while(1)
    {
        while(T0Flag)
        {
            StrobeCmd(CMD_RX);
            while(!GIO1 && T0Flag);
            {
                if(T0Flag)
                {
                    if(RecvData() == 2 && tmpbuf[0] == 0xAB && tmpbuf[1] == 0xCD)
                    {
                        T1_ON;
                        LED_BLINK;
                        SendFlag = 0;
                        info[16] = 0x01;
                        T3Count = 12;
                    }
                }
            }
            StrobeCmd(CMD_STBY);
        }
        if(SendFlag)
            A7139Send(TX_BUF,4);
    }
	*/
}


