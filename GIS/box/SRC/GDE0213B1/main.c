#include "main.h"
#include "Display_EPD_W21.h"
#include "ziku.h"
void PortInit(void)
{
    DDRA = 0B00001000;
    PORTA= 0B00000000;
    PINA = 0x00;
	
    DDRB = 0B10110110;		// PB1->RES	PB2->D/C PB0->BUSY PB5->SDI PB4->CS PB7->CLK
    PORTB= 0B00000000;
    PINB = 0x00;

    DDRD = 0B00000000;		// PD5->RES	PD6->D/C PD7->BUSY PD4->SDI PD3->CS PD2->CLK
    PORTD= 0B00000000;
    PIND = 0x00;

    DDRC = 0B00000000;
    PORTC= 0B00000000;
    PINC = 0x00;
}
/*

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
*/

unsigned char num[640] = {0};
#define DELAYTIME 2000
#define xDot 128
#define yDot 250

int main(void)
{
 	unsigned int i,m;
 	PortInit();
	EPD_init_Full();
	_delay_ms(500);
	m = 0xFF;
	EPD_Dis_Full((unsigned char *)&m,0x00);  //all white
	_delay_ms(4000);
	
/*
	EPD_Dis_Full((unsigned char *)first,1);  //	pic
	_delay_ms(DELAYTIME);
 	EPD_Dis_Full((unsigned char *)second,1);  //	pic
	_delay_ms(DELAYTIME);
	EPD_Dis_Full((unsigned char *)third,1);  //	pic
	_delay_ms(DELAYTIME);
	EPD_Dis_Full((unsigned char *)logo,1);
	_delay_ms(DELAYTIME);

    EPD_init_Part();
	_delay_ms(200);
    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)logo,1);
	*/
	EPD_init_Part();
	_delay_ms(1000);
	/*
	for(i=0; i<648; i++)
		num[i] = pgm_read_byte(d+i);
	EPD_Dis_Part(32,103,0,71,num,1);
	*/
	for(i=0; i<240; i++)
		num[i] = pgm_read_byte(gImage_t48+i);
	EPD_Dis_Part(64,111,0,39,num,1);

	for(i=0; i<415; i++)
		num[i] = pgm_read_byte(gImage_DC6+i);
	EPD_Dis_Part(0,39,0,82,num,1);
	
//	EPD_Dis_Part(56,79,0,23,gImage_ddd,1);		// 度

	EPD_Dis_Part(64,87,225,250,gImage_n,1);		//负号
	for(i=0; i<504; i++)
		num[i] = pgm_read_byte(gImage_8+i);
	EPD_Dis_Part(40,111,119,174,num,1);
	
		
	for(i=0; i<504; i++)
		num[i] = pgm_read_byte(gImage_2+i);
	EPD_Dis_Part(40,111,170,225,num,1);
	
	EPD_Dis_Part(88,111,102,114,gImage_dot,1);
	
	while(1)
	{
	//	EPD_Dis_Full((unsigned char *)&m,0x00);  //all white
		m++;
		EPD_init_Part();
		_delay_ms(1000);

		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_0+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		
		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_1+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		
		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_2+i);
		EPD_Dis_Part(40,111,45,100,num,1);

		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_3+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_4+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_5+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_6+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_7+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_8+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		_delay_ms(1000);
		
		for(i=0; i<504; i++)
			num[i] = pgm_read_byte(gImage_9+i);
		EPD_Dis_Part(40,111,45,100,num,1);
		PORTA^=0x08;
	}
/*
	EPD_init_Full();
	_delay_ms(200);
	
	m = 0xFF;
	EPD_Dis_Full((unsigned char *)&m,0x00);  //all white
	_delay_ms(1000);
	
	EPD_init_Part();
	_delay_ms(200);
*/
//	PORTC|=0x02;
	while(1)
	{
		PORTA^=0x08;
		_delay_ms(3000);
	}
/*
  	while(1)
  	{		
	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)first,1); //pic
	  	driver_delay_xms(1000);	

	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)second,1); //pic
	  	driver_delay_xms(1000);
			
	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)third,1); //pic
	  	driver_delay_xms(900);
			
	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)fourth,1); //pic
	  	driver_delay_xms(900);	

	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)fifth,1); //pic
	  	driver_delay_xms(800);
			
	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)sixth,1); //pic
	  	driver_delay_xms(800);	

	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)senventh,1); //pic
	  	driver_delay_xms(700);
			
	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)eighth,1); //pic
	  	driver_delay_xms(700);
			
	    EPD_Dis_Part(0,xDot-1,0,yDot-1,(unsigned char *)ninth,1); //pic
	  	driver_delay_xms(600);	
  	}
  	*/
}
