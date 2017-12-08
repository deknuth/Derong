#include "main.h"

#define SET_DQ (PORTC)|=0x02      
#define CLR_DQ (PORTC)&=~(0x02)
#define DQ_IN  (PINC)&0x02
#define SET_OUT (DDRC)|=0x02
#define SET_IN  (DDRC)&=~(0x02)
unsigned char Temp_H,Temp_L,OK_Flag;    //温度高位，低位，复位成功标志
unsigned char DS18B20_Init(void)
{ 
	SET_OUT; 
	_delay_us(490); 
	SET_IN;     //
	_delay_us(68); 
	if(DQ_IN)   
		OK_Flag = 0;  
	else
		OK_Flag = 1; 
	_delay_us(422); 
	return OK_Flag;   
}

unsigned char Read_18b20(void)
{
	unsigned char i;  
	unsigned char dat = 0;  
	for(i = 0;i < 8;i++)   
	{  
		SET_OUT;    
		_delay_us(2);  
		SET_IN;       
   
		_delay_us(4);        
		dat = dat >> 1;       
		if(DQ_IN)       
			dat |= 0x80;   
		_delay_us(62);
	} 
	return dat;
}

void Write_18b20(unsigned char dat)
{
	unsigned char i; 
 
	for(i = 0;i < 8;i++) 
	{
		SET_OUT;
		_delay_us(2);
		if(dat & 0x01)
			SET_IN;
		else
			SET_OUT; 	
		dat >>= 1;     
		_delay_us(62); 
		SET_IN;
		_delay_us(2);
	} 
}


void GetTemper(void)
{
	unsigned int tempint,tempint1,tempint2,tempint3,tempint4;
    unsigned int temppoint; 
	DS18B20_Init();
	Write_18b20(0Xcc);
	Write_18b20(0X44);
	_delay_ms(1000);
	DS18B20_Init();
	Write_18b20(0Xcc);
	Write_18b20(0Xbe);
	Temp_L = Read_18b20();
	Temp_H = Read_18b20();
	if(Temp_H & 0x08)
	{   
		Temp_H = ~Temp_H;
		Temp_L = ~Temp_L;
  
		SREG |= ~(1 << SREG_C);
		Temp_L++;
		if(SREG & (1 << SREG_C))
			Temp_H++;
	} 
	tempint = ((Temp_H << 4) & 0x70) | (Temp_L >> 4);    
	tempint1 = tempint / 1000;
	tempint2 = tempint % 1000 / 100;
	tempint3 = tempint % 100 / 10;
	tempint4 = tempint % 10;   
	temppoint = Temp_L & 0x0f;
	temppoint = (temppoint * 625);
	display(tempint,temppoint/1000);
}