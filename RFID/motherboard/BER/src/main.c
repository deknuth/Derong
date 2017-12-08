#include "main.h"

#define	D1_BLINK	PORTD^=0x20
#define D1_OFF		PORTD&=0x20
#define	D2_BLINK	PORTD^=0x40
#define D2_OFF		PORTD&=0x40
#define	D3_BLINK	PORTD^=0x80
#define D3_OFF		PORTD&=0x80
#define	D4_BLINK	PORTB^=0x01
#define D4_OFF		PORTB&=0x01

#define WG_A1_0		PORTC&=~0x01
#define WG_A1_1		PORTC|=0x01
#define WG_B1_0		PORTC&=~0x02
#define WG_B1_1		PORTC|=0x02

#define WG_A2_0		PORTC&=~0x04
#define WG_A2_1		PORTC|=0x04
#define WG_B2_0		PORTC&=~0x08
#define WG_B2_1		PORTC|=0x08

#define WG_A3_0		PORTC&=~0x10
#define WG_A3_1		PORTC|=0x10
#define WG_B3_0		PORTC&=~0x20
#define WG_B3_1		PORTC|=0x20

#define WG_A4_0		PORTB&=~0x02
#define WG_A4_1		PORTB|=0x02
#define WG_B4_0		PORTB&=~0x04
#define WG_B4_1		PORTB|=0x04

#define FEED_DOG	PORTD^=0x10

#define L_PLUS_TIME		100
#define INTERVAL_TIME	1000

volatile unsigned char dir = 0;

void PortInit(void)
{
    DDRB = 0B00000111;
    PORTB= 0B00000110;
    PINB = 0x00;

    DDRD = 0B11110000;
    PORTD= 0B00000000;
    PIND = 0x00;

    DDRC = 0B00111111;
    PORTC= 0B00111111;
    PINC = 0x00;
}

void UartInit(void)
{
    UBRR0H = (F_CPU / BAUD / 16 - 1) / 256;
    UBRR0L = (F_CPU / BAUD / 16 - 1) % 256;
    UCSR0B = 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0;
    UCSR0C = 1<<UCSZ00 | 1<<UCSZ01;
}

void SendStr(unsigned char* data,unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        while(!(UCSR0A & (1 << UDRE0)));
        UDR0 = *(data++);
    }
}

unsigned char word = 0;
volatile unsigned char start = 0;
volatile unsigned char zLen = 0;
volatile unsigned char U0Ready = 0;
unsigned char U0Buf[32] = {0};
volatile unsigned char U0Count = 0;

ISR(USART_RX_vect)
{
	word = UDR0;
	if(word == 0xA0)
	{
		if(!U0Ready)
			start = 1;
	}
	if(start)
	{
		U0Buf[U0Count++] = word;
		if(U0Count == zLen)
		{	
			if(U0Count > 15)
				U0Ready = 1;
			else
				U0Count = 0;
			start = zLen = 0;
		}
		else if(U0Count == 2)
		{
			if(word < 20)		// 协议最大长度
				zLen = (word+2);
			else
			{
				U0Count = 0;
				start = 0;
			}
		}
	}
}

void WG_send_bit_1(unsigned char channel)
{
	switch(channel)
	{
		case 0:
			WG_A1_0;
			_delay_us(L_PLUS_TIME);
			WG_A1_1;
			_delay_us(INTERVAL_TIME);
			D1_BLINK;
			break;
		case 1:
			WG_A2_0;
			_delay_us(L_PLUS_TIME);
			WG_A2_1;
			_delay_us(INTERVAL_TIME);
			D2_BLINK;
			break;
		case 2:
			WG_A3_0;
			_delay_us(L_PLUS_TIME);
			WG_A3_1;
			_delay_us(INTERVAL_TIME);
			D3_BLINK;
			break;
		case 3:
			WG_A4_0;
			_delay_us(L_PLUS_TIME);
			WG_A4_1;
			_delay_us(INTERVAL_TIME);
			D4_BLINK;
			break;
		default:
			break;
				
	}
}

void WG_send_bit_0(unsigned char channel)
{
	switch(channel)
	{
		case 0:
			WG_B1_0;
			_delay_us(L_PLUS_TIME);
			WG_B1_1;
			_delay_us(INTERVAL_TIME);
			D1_BLINK;
			break;
		case 1:
			WG_B2_0;
			_delay_us(L_PLUS_TIME);
			WG_B2_1;
			_delay_us(INTERVAL_TIME);
			D2_BLINK;
			break;
		case 2:
			WG_B3_0;
			_delay_us(L_PLUS_TIME);
			WG_B3_1;
			_delay_us(INTERVAL_TIME);
			D3_BLINK;
			break;
		case 3:
			WG_B4_0;
			_delay_us(L_PLUS_TIME);
			WG_B4_1;
			_delay_us(INTERVAL_TIME);
			D4_BLINK;
			break;
		default:
			break;
	}   
}

void send_wiegand34(unsigned char *wiegand, unsigned char channel)
{
    unsigned char i,j;
    unsigned char check_temp; 	// 韦根包奇偶效验中间暂存
    unsigned char even; 		// 韦根包前16位偶效验
    unsigned char odd; 			// 韦根包后16位齐效验
	unsigned char temp = 0;

    //--------------------------------计算前16位1的个数是否为偶数，为偶效验用
    check_temp = wiegand[1];
    check_temp ^= wiegand[0];
    check_temp ^= check_temp>>4;
    check_temp ^= check_temp>>2;
    check_temp ^= check_temp>>1;
    even = check_temp&1;
 
    //--------------------------------计算后16位1的个数是否为偶数，为奇效验用
    check_temp = wiegand[2];
    check_temp ^= wiegand[3];
    check_temp ^= check_temp>>4;
    check_temp ^= check_temp>>2;
    check_temp ^= check_temp>>1;
    odd = !(check_temp&1);
 
    if(even)
        WG_send_bit_1(channel);
    else
        WG_send_bit_0(channel);
		
    for(i=0;i<4;i++)
    {
		temp = wiegand[i];
		for(j=0; j<8; j++)
		{
			if(temp&0x80)
				WG_send_bit_1(channel);
			else
				WG_send_bit_0(channel);
			temp <<= 1;
		}
    }
	
    if(odd)
        WG_send_bit_1(channel);
    else
        WG_send_bit_0(channel);
}

unsigned char CheckSum(unsigned char *uBuff, unsigned char uBuffLen)
{
	unsigned char i,uSum=0;
	for(i=0;i<uBuffLen;i++)
		uSum += uBuff[i];
	uSum = (~uSum) + 1;
	return uSum;
}


void display(void)
{
	unsigned char i = 0;
	for(i=0; i<8; i++)
	{
		D1_BLINK;
		FEED_DOG;
		_delay_ms(150);
	}
	D1_OFF;
}

int main(void)
{
	unsigned char sum = 0;
//	unsigned char channel[4] = {0};
//	unsigned char temp = 0;
//	unsigned char wg[4][4] = {{0}};
	unsigned char wg[4] = {0};

	cli();
    PortInit();
	display();
	UartInit();
	sei();
	while(1)
    {
		if(U0Ready)
		{	
			if(U0Count>19)
			{
				sum = CheckSum(U0Buf,U0Count-1);
				if(U0Buf[U0Count-1]==sum)
				{	
//					temp = ((U0Buf[4])&0x03);
//					channel[temp] = 1;
//					memcpy(wg[temp],&U0Buf[15],4);
					memcpy(wg,&U0Buf[15],4);
					send_wiegand34(wg,(U0Buf[4])&0x03);
				}
			}
			U0Count = U0Ready = 0;
		}
		FEED_DOG;
	}
}