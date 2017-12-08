#include "main.h"
#include "Display_EPD_W21.h"
#include "ziku.h"

#define GSM_ON	{ PORTC|=0x04; _delay_ms(2200); PORTC&=~0x04; }
#define T1_OFF { TCCR1B=0; TIMSK&=~(1<<TOIE1); }
#define T1_ON { TCCR1B|=1<<CS12|1<<CS10; TIMSK|=1<<TOIE1; TCNT1=0x735F; }		// 1024 divide

#define T2_ON	{ TCCR2=1<<CS22|1<<CS20|1<<CS21; TIMSK|=1<<TOIE2; TCNT2=0;}
#define T2_OFF	{ TCCR2=0; TIMSK&=~1<<TOIE2; }

#define	LED_ON		PORTA|=(1<<3)
#define	LED_OFF		PORTA&=~(1<<3)
#define	LED_BLINK	PORTA^=(1<<3)


unsigned char U0Buf[64]= {0};
volatile unsigned char U0Count = 0;
volatile unsigned char U0Ready = 0;
struct latlon{
    unsigned char latFlag;
    unsigned char lonFlag;
    unsigned char valid;
 //   unsigned char lat[12];
 //   unsigned char lon[13];
//    unsigned char time[12];
//	unsigned char date[8];
//	unsigned char speed[8];
//	unsigned char dir[8];
	unsigned char sn[8];
};
struct latlon coor;

void PortInit(void)
{
    DDRA = 0B00001000;
    PORTA= 0B00000000;
    PINA = 0x00;
	
    DDRB = 0B10110110;		// PB1->RES	PB2->D/C PB0->BUSY PB5->SDI PB4->CS PB7->CLK
    PORTB= 0B00000000;
    PINB = 0x00;

    DDRD = 0B00000000;		// 
    PORTD= 0B00000000;
    PIND = 0x00;

    DDRC = 0B00011100;
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

void U0Send(char* data,unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        while(!(UCSRA & (1 << UDRE)));
        UDR = *(data++);
    }
} 

volatile unsigned char T1Count = 0;
ISR(TIMER1_OVF_vect)			// 5S one time 
{
	TCNT1=0x735F;
    T1Count++;
}

ISR(USART_RXC_vect)
{
	unsigned char tmp;
	T2_ON;
	tmp = UDR;
	U0Buf[U0Count++] = tmp;
	if(U0Count > 63)
		U0Count = 0;
}

ISR(TIMER2_OVF_vect)			// UART timeout
{
	U0Ready = 1;
	T2_OFF;
}

unsigned char num[504] = {0};
void display(int integ, unsigned char dec)
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;
	unsigned int ten = abs(((integ/10)%10));

	if(ten != 0)
	{
		i = 504*ten;
		j = i+504;
		for(; i<j; i++)
			num[k++] = pgm_read_byte(gImage_num+i);
		EPD_Dis_Part(40,111,170,225,num,1);			// 十位

		if(integ >= 0)		// 非负数
		{
			for(i=0;i<72;i++)					// 全白
				num[i] = 0xFF;
			EPD_Dis_Part(64,87,226,249,num,1);
		}
		else
			EPD_Dis_Part(64,87,226,249,gImage_n,1);		//负号
	}
	else		// 十位为0
	{
		for(i=0; i<504; i++)
			num[i] = 0xFF;
		EPD_Dis_Part(40,111,170,225,num,1);			// 十位涂白

		for(i=0;i<72;i++)					// 原负数位涂白
			num[i] = 0xFF;
		EPD_Dis_Part(64,87,226,249,num,1);
		
		if(integ < 0)
			EPD_Dis_Part(64,87,177,200,gImage_n,1);	//原十位填充负号
	}

	integ = abs(integ);
	k = 0;
	i = 504*(integ%10);
	j = i+504;
	for(; i<j; i++)
		num[k++] = pgm_read_byte(gImage_num+i);
	EPD_Dis_Part(40,111,119,174,num,1);			// 个位
	
	k = 0;
	i = 504*(dec%10);
	j = i+504;
	for(; i<j; i++)
		num[k++] = pgm_read_byte(gImage_num+i);
	EPD_Dis_Part(40,111,45,100,num,1);			// 小数

}

void eLinkInit(void)
{
	unsigned int i,m;
	EPD_init_Full();
	_delay_ms(500);
	m = 0xFF;
	EPD_Dis_Full((unsigned char *)&m,0x00);  //all white
	_delay_ms(4000);
	EPD_init_Part();
	EPD_Dis_Part(0,249,0,127,(unsigned char *)&m,1);
	_delay_ms(700);
	
	for(i=0; i<240; i++)						
		num[i] = pgm_read_byte(gImage_t48+i);	
	EPD_Dis_Part(64,111,0,39,num,1);

	for(i=0; i<415; i++)						
		num[i] = pgm_read_byte(gImage_DC6+i);	
	EPD_Dis_Part(0,39,0,82,num,1);
	
	for(i=0; i<300; i++)						
		num[i] = pgm_read_byte(gImage_signal3+i);	
	EPD_Dis_Part(0,39,190,249,num,1);				// 信号
	EPD_Dis_Part(88,111,102,113,gImage_dot,1);		// 小数点
	display(0,0);
}

unsigned char macReady = 0;
unsigned char cStat = 0;	// client state	
unsigned char s_mac[16] = {0};

char REG_PACK[12] = { 0xFE,0xC,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xFF };
char AT_INS[6][43] = {
	"AT+DSCADDR=0,\"TCP\",\"114.215.158.69\",7777\r",
	"AT&W\r",
	"AT+CFUN=1,1\r",
	"AT+GSN\r",
	"ATO\r",
	"+++"
};

enum at{ 
	CON_SER, 
	SAVE, 
	RESTART, 
	GET_MAC,
	DATA_MODE,
	CMD_MODE
};

void ClearU0Buf(void)
{
	memset(U0Buf,0x00,U0Count);
	U0Ready = U0Count = 0;
}

void PowerConfir(void)
{
	while(1)
	{
		if(U0Ready)
		{
			if(kmp(U0Buf,"AST_POWERON") != -1)
				break;
			else
				ClearU0Buf();
		}
	}
	ClearU0Buf();
}

void WaitBack(void)
{
	while(1)
	{
		if(U0Ready)
		{
			ClearU0Buf();
			break;
		}
	}
}

void InitGSM(void)
{
	U0Send(AT_INS[CON_SER],strlen(AT_INS[CON_SER]));
	WaitBack();
	U0Send(AT_INS[SAVE],strlen(AT_INS[SAVE]));
	WaitBack();
	U0Send(AT_INS[RESTART],strlen(AT_INS[RESTART]));
	PowerConfir();
}

void GetMac(void)
{
	ClearU0Buf();
	U0Send(AT_INS[GET_MAC],strlen(AT_INS[GET_MAC]));
	T1_ON;
	
	while(1)	// get mac address
	{
		if(U0Ready)
		{
			if(kmp(U0Buf,"CONNECT") != -1)
			{
				U0Send(AT_INS[CMD_MODE],strlen(AT_INS[CMD_MODE]));
				WaitBack();
				_delay_ms(400);
				U0Send(AT_INS[GET_MAC],strlen(AT_INS[GET_MAC]));
			}
			else if(U0Count==25)
			{
				memcpy(&s_mac[1],&U0Buf[2],15);
				s_mac[0] = '0';
				StringToHex(s_mac,coor.sn,16);
				memcpy(&REG_PACK[3],coor.sn,8);
				ClearU0Buf();
				macReady = 1;
				break;
			}
			ClearU0Buf();
		}
		
		if(T1Count > 4)		// 20S
		{
			U0Send(AT_INS[DATA_MODE],strlen(AT_INS[DATA_MODE]));
			_delay_ms(100);
			U0Send(AT_INS[CMD_MODE],strlen(AT_INS[CMD_MODE]));
			WaitBack();
			_delay_ms(400);
			InitGSM();		// reset mode
			T1Count = 0;
		}
	}
	U0Send(AT_INS[DATA_MODE],strlen(AT_INS[DATA_MODE]));
	_delay_ms(200);	
	U0Send(REG_PACK,12);		// reg pack
	cStat = 1;
	_delay_ms(200);	
	T1_OFF;
}

int main(void)
{

	cli();
 	PortInit();
	LED_BLINK;
	_delay_ms(200);
	LED_BLINK;
	_delay_ms(200);
	LED_BLINK;
	_delay_ms(200);
	LED_BLINK;
	_delay_ms(1500);
	eLinkInit();
	UartInit();
	sei();
	GSM_ON;
	PowerConfir();
	LED_ON;
	InitGSM();
	GetMac();
	
	while(1)
	{
		if(U0Ready)
		{
//			U0Send(U0Buf,U0Count);
			if(kmp(U0Buf,"CONNECT") != -1)
			{
				_delay_ms(150);
				U0Send(REG_PACK,12);		// reg pack
				cStat = 1;
			}
			else if(kmp(U0Buf,"CLOSED") != -1)
				cStat = 0;	
			ClearU0Buf();
		}
		GetTemper();
	}
}
