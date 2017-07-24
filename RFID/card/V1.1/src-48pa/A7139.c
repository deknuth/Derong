#include "main.h"

static Uint8 A7139_Cal(void);
static void A7139_Config(void);
static void A7139_WriteReg(Uint8 regAddr, Uint16 regVal);
static Uint16 A7139_ReadPageA(Uint8 pagAddr);
static void A7139_WritePageA(Uint8 pagAddr, Uint16 pagVal);
static void A7139_WritePageB(Uint8 pagAddr, Uint16 pagVal);
static void A7139_SetFreq(float rfFreq);
static Uint8  A7139_RCOSC_Cal(void);
static Uint8 A7139_ReadCID(void);

const Uint16  A7139Config_PageA[];
const Uint16  A7139Config_PageB[];
const Uint16  A7139Config[];

void SPIx_WriteByte(unsigned char src)
{
    unsigned char i;
    SPI_DATA_OUT;
    for (i = 0; i < 8; i++)
    {
        if (src & 0x80)
            SPI_DATA_H;
        else
            SPI_DATA_L;
        asm("nop");
        SPI_SCK_H;
        asm("nop");
        SPI_SCK_L;
        src <<= 1;
    }
}

unsigned char SPIx_ReadByte(void)
{
    unsigned char i, tmp=0;
    SPI_DATA_IN;
    for (i = 0; i < 8; i++)
    {
        if (SPI_DATA_HL)
            tmp = (tmp << 1) | 0x01;
        else
            tmp = tmp << 1;
        asm("nop");
        SPI_SCK_H;
        asm("nop");
        SPI_SCK_L;
    }
    return tmp;
}

void SPIx_WriteWord(Uint16 src)
{
    unsigned char i = 0;
    SPI_DATA_OUT;
    for (i = 0; i < 16; i++)
    {
        if (src & 0x8000)
            SPI_DATA_H;
        else
            SPI_DATA_L;
        SPI_SCK_H;
        asm("nop");
        SPI_SCK_L;
        src <<= 1;
    }
}

Uint16 SPIx_ReadWord(void)
{
    unsigned int regVal = 0;
    unsigned char i = 0;
    SPI_DATA_IN;
    for (i = 0; i < 16; i++)
    {
        if (SPI_DATA_HL)
            regVal = (regVal << 1) | 0x01;
        else
            regVal = regVal << 1;
        SPI_SCK_H;
        asm("nop");
        SPI_SCK_L;
    }
    return regVal;
}

static void A7139_WriteReg(Uint8 regAddr, Uint16 regVal)
{
    SPI_SCS_L;
    regAddr |= CMD_Reg_W;
    SPIx_WriteByte(regAddr);
    
    SPIx_WriteWord(regVal);
    SPI_SCS_H;
}

Uint16 A7139_ReadReg(Uint8 regAddr)
{
    Uint16 regVal;
    SPI_SCS_L;
    regAddr |= CMD_Reg_R;
    SPIx_WriteByte(regAddr);
    asm("nop");
    regVal=SPIx_ReadWord();
    SPI_SCS_H;
    return regVal;
}

static void A7139_WritePageA(Uint8 address, Uint16 dataWord)
{
    Uint16 tmp;
    tmp = address;
    tmp = ((tmp << 12) | A7139Config[CRYSTAL_REG]);
    A7139_WriteReg(CRYSTAL_REG, tmp);
    A7139_WriteReg(PAGEA_REG, dataWord);
}

static void A7139_WritePageB(Uint8 address, Uint16 dataWord)
{
    Uint16 tmp;
    tmp = address;
    tmp = ((tmp << 7) | A7139Config[CRYSTAL_REG]);
    A7139_WriteReg(CRYSTAL_REG, tmp);
    A7139_WriteReg(PAGEB_REG, dataWord);
}

static Uint16 A7139_ReadPageA(Uint8 address)
{
    Uint16 tmp;
    tmp = address;
    tmp = ((tmp << 12) | A7139Config[CRYSTAL_REG]);
    A7139_WriteReg(CRYSTAL_REG, tmp);
    tmp = A7139_ReadReg(PAGEA_REG);
    return tmp;
}

static void A7139_Config(void)
{
    Uint8 i;
    for(i=0; i<8; i++)
        A7139_WriteReg(i, A7139Config[i]);
    for(i=10; i<16; i++)
        A7139_WriteReg(i, A7139Config[i]);
    for(i=0; i<16; i++)
        A7139_WritePageA(i, A7139Config_PageA[i]);
    for(i=0; i<5; i++)
        A7139_WritePageB(i, A7139Config_PageB[i]);
}

static Uint8 A7139_Cal(void)
{
    Uint8 fb, fcd, fbcf;	//IF Filter
    Uint8 vb,vbcf;			//VCO Current
    Uint8 vcb, vccf;		//VCO Band
    Uint16 tmp;
    //IF calibration procedure @STB state
    A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0802);			//IF Filter & VCO Current Calibration
    do{
        tmp = A7139_ReadReg(MODE_REG);
    }while(tmp & 0x0802);
    //for check(IF Filter)
    tmp = A7139_ReadReg(CALIBRATION_REG);
    fb = tmp & 0x0F;
    fcd = (tmp>>11) & 0x1F;
    fbcf = (tmp>>4) & 0x01;
    if(fbcf)
    {
        return ERR_CAL;
    }
    //for check(VCO Current)
    tmp = A7139_ReadPageA(VCB_PAGEA);
    vcb = tmp & 0x0F;
    vccf = (tmp>>4) & 0x01;
    if(vccf)
    {
        return ERR_CAL;
    }
    //RSSI Calibration procedure @STB state
    A7139_WriteReg(ADC_REG, 0x4C00);									//set ADC average=64
    A7139_WritePageA(WOR2_PAGEA, 0xF800);								//set RSSC_D=40us and RS_DLY=80us
    A7139_WritePageA(TX1_PAGEA, A7139Config_PageA[TX1_PAGEA] | 0xE000);	//set RC_DLY=1.5ms
    A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x1000);			//RSSI Calibration
    do{
        tmp = A7139_ReadReg(MODE_REG);
    }while(tmp & 0x1000);
    A7139_WriteReg(ADC_REG, A7139Config[ADC_REG]);
    A7139_WritePageA(WOR2_PAGEA, A7139Config_PageA[WOR2_PAGEA]);
    A7139_WritePageA(TX1_PAGEA, A7139Config_PageA[TX1_PAGEA]);
    //VCO calibration procedure @STB state
    A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0004);		// VCO Band Calibration
    do{
        tmp = A7139_ReadReg(MODE_REG);
        _delay_us(1);
    }while(tmp & 0x0004);	// for check(VCO Band)

    tmp = A7139_ReadReg(CALIBRATION_REG);
    vb = (tmp >>5) & 0x07;
    vbcf = (tmp >>8) & 0x01;
    if(vbcf)				// VCO Band Auto Calibration failed
    {
        return ERR_CAL;
    }
    return 0;
}

static void A7139_SetFreq(float rfFreq)
{
    float  divFreq = rfFreq / 12.800f;
    Uint8  intFreq = (Uint8)(divFreq); //integer part
    float  fltFreq = divFreq - intFreq * 1.000f; //fraction part
    Uint16 fpFreg	= (Uint16)(fltFreq * 65536);  //FP register val
    Uint16 orgVal;
    StrobeCmd(CMD_STBY); //enter stand-by mode
    //AFC[15:15] = 0
    orgVal = A7139Config[PLL3_REG] & 0x7FFF;
    A7139_WriteReg(PLL3_REG,orgVal);
    //RFC[15:12] = 0000
    orgVal = A7139Config[PLL6_REG] & 0x0FFF;
    A7139_WriteReg(PLL6_REG,orgVal);
    //MD1[12:12]=0,1
    if(rfFreq < 860)	//433-510
        orgVal = A7139Config[PLL4_REG] & 0xEFFF;
    else	 //868-915
        orgVal = A7139Config[PLL4_REG] | 0x1000;
    A7139_WriteReg(PLL4_REG,orgVal);
    //IP[8:0] = intg
    orgVal = A7139Config[PLL1_REG] & 0xFF00;
    A7139_WriteReg(PLL1_REG,orgVal|intFreq);
    //FP[15:0] =  fpFreg
    A7139_WriteReg(PLL2_REG,fpFreg);
    //FPA[15:0] = 0x0000
    A7139_WritePageB(IF2_PAGEB,0x0000);
}

static Uint8 A7139_RCOSC_Cal(void)
{
    Uint8  retry = 0xFF;
    Uint16 calbrtVal,t_retry=0xFFFF;
    //RCOSC_E[4:4] = 1,enable internal RC Oscillator
    A7139_WritePageA(WOR2_PAGEA, A7139Config_PageA[WOR2_PAGEA] | 0x0010);
    do{
        //ENCAL[0:0] = 1,then start RC OSC Calbrt
        A7139_WritePageA(WCAL_PAGEA, A7139Config_PageA[WCAL_PAGEA] | 0x0001);
        do{
            calbrtVal = A7139_ReadPageA(WCAL_PAGEA) & 0x0001;
        }while(calbrtVal && t_retry--);
        //read NUMLH[9:1]
        calbrtVal = (A7139_ReadPageA(WCAL_PAGEA) & 0x03FF) >> 1;
        if(calbrtVal>186 && calbrtVal<198)
            return OK_RCOSC_CAL;
    }while(retry--);
    return ERR_RCOSC_CAL;
}

void StrobeCmd(Uint8 cmd)
{
    SPI_SCS_L;
    SPIx_WriteByte(cmd);
    SPI_SCS_H;
}

Uint8 A7139_Init(float rfFreq)
{
    SPI_SCS_H;
    SPI_SCK_L;
    SPI_DATA_H;
    StrobeCmd(CMD_RF_RST);	  //reset A7139 chip
    _delay_ms(10);
    A7139_Config();			  //config A7139 chip
    _delay_ms(10);			  //for crystal stabilized
    A7139_SetCID(0x0A7139A8); //set CID code
    A7139_ReadCID();
    _delay_ms(1);
    A7139_SetFreq(rfFreq);	  //set Freq
    //A7139_SetDataRate(10);
    _delay_ms(15);
    return A7139_Cal();		  //IF and VCO calibration
}

Uint8 A7139_SetCID(Uint32 id)
{
    SPI_SCS_L;
    SPIx_WriteByte(CMD_CID_W);
    SPIx_WriteByte((Uint8)(id>>24));
    SPIx_WriteByte((Uint8)(id>>16));
    SPIx_WriteByte((Uint8)(id>>8));
    SPIx_WriteByte((Uint8)id);
    SPI_SCS_H;
    return 0;
}

Uint8 A7139_ReadCID(void)
{
    unsigned char id[4] = {0};
    SPI_SCS_L;
    SPIx_WriteByte(CMD_CID_R);
    id[0] = SPIx_ReadByte();
    id[1] = SPIx_ReadByte();
    id[2] = SPIx_ReadByte();
    id[3] = SPIx_ReadByte();
//    SendStr(id,4);
    SPI_SCS_H;
    return 0;
}

Uint16 A7139_ReadPID(void)
{
    Uint16 pid;
    Uint16 pagAddr = TX2_PAGEB << 7;
    pagAddr|=A7139Config[CRYSTAL_REG] & 0xF7CF;
    A7139_WriteReg(CRYSTAL_REG, pagAddr);
    pid = A7139_ReadReg(PAGEB_REG);
    return pid;
}

Uint8 A7139_SetDataRate(Uint8 datRate)
{
    //enter stand by mode
    StrobeCmd(CMD_STBY);
    //set xs[0:0] = 0 to disable XTAL
    A7139_WriteReg(CRYSTAL_REG,A7139Config[CRYSTAL_REG] & 0xFFFE );
    switch(datRate)
    {
    case 2:
    {
        //CSC[2:0]=[011],Fcsck=3.2MHz
        //IFBW[3:2]=[00],50kHz
        //SDR[15:9]=0x18,DCK=2Kps
        A7139_WriteReg(SYSTEMCLOCK_REG,0x3023);
        A7139_WriteReg(RX1_REG,0x18D0);
    }
        break;
    case 10:
    {
        //CSC[2:0]=[011],Fcsck=3.2MHz
        //IFBW[3:2]=[00],50kHz
        //SDR[15:9]=0x04,DCK=10Kps
        A7139_WriteReg(SYSTEMCLOCK_REG,0x0823);
        A7139_WriteReg(RX1_REG,0x18D0);
    }
        break;
    case 50:
    {
        //CSC[2:0]=[011],Fcsck=3.2MHz
        //IFBW[3:2]=[00],100Kps
        //SDR[15:9]=0x00,DCK=50kHz
        A7139_WriteReg(SYSTEMCLOCK_REG,0x0023);
        A7139_WriteReg(RX1_REG,0x18D0);
    }
        break;
    case 100:
    {
        //CSC[2:0]=[001] ,Fcsck=6.4MHz
        //IFBW[3:2]=[01],50kHz
        //SDR[15:9]=0x00,DCK=100Kps
        A7139_WriteReg(SYSTEMCLOCK_REG,0x0021);
        A7139_WriteReg(RX1_REG,0x18D4);
    }
        break;
    case 150:
    {
        //Cause LO is 12.8MHz
        //must use Pll clk gen,complement in detail later
        //also can be implemented by the users themselves
        //IFBW=[10] <=> 150kHz
        //DCK=150K
        //CSC=000,fcsck=9.6MHz
        //SDR=0x00
        //DMOS=1,IFBW=150KHz
    }
        break;
    default:
        // set xs[0:0] = 1 to open XTAL
        A7139_WriteReg(CRYSTAL_REG,A7139Config[CRYSTAL_REG] | 0x0001 );
        return ERR_PARAM;
    }
    // set xs[0:0] = 1 to enable XTAL
    A7139_WriteReg(CRYSTAL_REG,A7139Config[CRYSTAL_REG] | 0x0001 );
    _delay_ms(10);
    return 0;
}

void A7139_WriteFIFO(unsigned char *buf,unsigned char len)
{
    Uint8 i;
    StrobeCmd(CMD_TFR);			// 重置Tx数据指针位置
    SPI_SCS_L;
    SPIx_WriteByte(CMD_FIFO_W);	// 送出“写FIFO缓存区”的命令
    for(i=0; i <len; i++)
        SPIx_WriteByte(buf[i]);
    SPI_SCS_H;
}

unsigned char A7139_ReadFIFO(unsigned char* buf)
{
    unsigned char i = 0;
    unsigned char len = 0;
    unsigned char temp = 0;
    StrobeCmd(CMD_RFR);				// 重置Rx数据指针位置
    _delay_us(5);
    SPI_SCS_L;
    SPIx_WriteByte(CMD_FIFO_R);		// 送出“读FIFO缓存区”的命令
    temp = SPIx_ReadByte();			// pack head
    if(temp==0xfe)
    {
		buf[i++] = temp;
        len = SPIx_ReadByte();		// pack data lenght
		buf[i++] = len;
        if(len > 0 && len < 16)
        {
            for(; i <len; i++)
                buf[i] = SPIx_ReadByte();	// 从A7139内部的FIFO缓冲区中读取数据
        }
    }
    SPI_SCS_H;
    return len;
}

Uint8 A7139_SetPackLen(Uint8 len)
{
    Uint16 pagVal;
    StrobeCmd(CMD_STBY);
    //FEP[7:0]
    pagVal = A7139Config_PageA[FIFO_PAGEA] & 0xFF00;
    A7139_WritePageA(FIFO_PAGEA,pagVal|(len-1));
    //FEP[13:8]
    pagVal = A7139Config_PageA[VCB_PAGEA] & 0xC0FF;
    A7139_WritePageA(VCB_PAGEA,pagVal);
    return 0;
}

Uint8 A7139_IsBatteryLow(Uint8 low2_x)
{
    Uint16 pagVal;
    if(low2_x<0 || low2_x>7)
        return ERR_PARAM;
    StrobeCmd(CMD_STBY);
    //BVT[3:1] BDS[0:0]
    pagVal= A7139Config[PM_PAGEA] & 0xFFF0;
    A7139_WritePageA(PM_PAGEA,pagVal | (low2_x << 1) | 0x01);
    _delay_us(10); //delay 5us at least
    //read VBD[7:7]
    return !((A7139_ReadPageA(WOR1_PAGEA) & 0x0080) >> 7);
}

Uint16 A7139_GetRSSI()
{	
    Uint16  rssi;
    Uint16 t_retry = 0xFFFF;
    StrobeCmd(CMD_RX);
    A7139_WriteReg(ADC_REG,A7139Config[ADC_REG] & 0xFCFF);
    A7139_WriteReg(MODE_REG,A7139_ReadReg(MODE_REG) | 0x0001);	// ADCM[0:0] = 1
    do{
        rssi = A7139_ReadReg(MODE_REG) & 0x0001; //ADCM auto clear when measurement done

    }while(t_retry-- && rssi);
    if(t_retry>0)
        rssi=(A7139_ReadReg(RX2_REG) & 0x01FF);  //ADCO[8:0]: RSSI value
    else
        rssi = ERR_GET_RSSI;
    StrobeCmd(CMD_STBY);
    return rssi;
}

Uint8 A7139_WOT(void)
{
    if(A7139_RCOSC_Cal()==ERR_RCOSC_CAL)
        return ERR_RCOSC_CAL;
    StrobeCmd(CMD_STBY);
    //GIO1=FSYNC, GIO2=WTR
    A7139_WritePageA(GIO_PAGEA, 0x0045);
    //setup WOT Sleep time
    A7139_WritePageA(WOR1_PAGEA, 0x027f);
    //WMODE=1 select WOT function
    A7139_WriteReg(PIN_REG, A7139Config[PIN_REG] | 0x0400);
    //WORE=1 to enable WOT function
    A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0200);
    //while(1); //注意此处为死循环，代码只为演示之用，用户必须按业务实际逻辑进行需改
    return 0;
}

Uint8 A7139_WOR_BySync(void)
{
    StrobeCmd(CMD_STBY);		//GIO1=FSYNC, GIO2=WTR
    A7139_WritePageA(GIO_PAGEA, 0x0045);
    A7139_WritePageA(WOR1_PAGEA, 0xFF06);	//setup WOR Sleep time and Rx time  active:15.6ms sleep:554ms
    if(A7139_RCOSC_Cal()==ERR_RCOSC_CAL)
        return ERR_RCOSC_CAL;
    A7139_WritePageA(WOR2_PAGEA, A7139Config_PageA[WOR2_PAGEA] | 0x0010);	//enable RC OSC & WOR by sync
    A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0200);

    return 0;
}

Uint8 A7139_WOR_ByPreamble(void)
{
    StrobeCmd(CMD_STBY);
    _delay_us(20);
    //	A7139_WritePageA(GIO_PAGEA, 0x004D);	//GIO1=PMDO, GIO2=WTR
    A7139_WritePageA(GIO_PAGEA, 0x0047);	// 0043
    //Real WOR Active Period = (WOR_AC[5:0]+1) x 244us,XTAL and Regulator Settling Time
    //Note : Be aware that Xtal settling time requirement includes initial tolerance,
    //       temperature drift, aging and crystal loading.
    A7139_WritePageA(WOR1_PAGEA, 0x98B0);	//setup WOR Sleep time and Rx time	active:9.5ms sleep:758ms		// 9860->758ms  98B0->1380ms 
    //RC Oscillator Calibration
    if(A7139_RCOSC_Cal()==ERR_RCOSC_CAL)
        return ERR_RCOSC_CAL;
    A7139_WritePageA(WOR2_PAGEA, A7139Config_PageA[WOR2_PAGEA] | 0x0030);	// enable RC OSC & WOR by preamble
    A7139_WriteReg(MODE_REG, A7139Config[MODE_REG] | 0x0200);				// WORE=1 to enable WOR function

    return 0;
}

Uint8 A7139_SetPowerLevel(Uint8 pwrLev)
{
    Uint16 pagVal= A7139Config_PageB[TX2_PAGEB] & 0xFF80;
    StrobeCmd(CMD_STBY);
    _delay_us(20);
    switch(pwrLev)
    {
    case 0:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x01);	//-1.1dBm
        break;
    case 1:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x09);	//1.2dBm
        break;
    case 2:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x0A);	//4.8dBm
        break;
    case 3:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x61);	//6.7dBm
        break;
    case 4:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x2A);	//10.9dBm
        break;
    case 5:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x1C);	//13.2dBm
        break;
    case 6:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x26);	//18dBm
        break;
    case 7:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x3E);	//20dBm
        break;
    case 8:
        A7139_WritePageB(TX2_PAGEB, pagVal| 0x5F);  //21dBm
        break;
    default:
        return ERR_PARAM;
    }
    return 0;
}

const Uint16 A7139Config[]=		//470MHz, 10kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
{
        0x1221,		//SYSTEM CLOCK register,
        0x0A24,		//PLL1 register,
        0xB805,		//PLL2 register,	470.001MHz
        0x0000,		//PLL3 register,
        //0x0B30,		//PLL4 register,
        0x0A20,     //PLL4 register,
        0x0024,		//PLL5 register,
        0x0000,		//PLL6 register,
        0x0011,		//CRYSTAL register,
        0x0000,		//PAGEA,
        0x0000,		//PAGEB,
        0x18D4,		//RX1 register, 	IFBW=100KHz, ETH=1
        0x7009,		//RX2 register, 	by preamble
        0x4400,		//ADC register,
        0x0800,		//PIN CONTROL register,		Use Strobe CMD
        0x4C45,     //CALIBRATION register,
        0x20C0		//MODE CONTROL register, 	Use FIFO mode
};

const Uint16 A7139Config_PageA[]=   //470MHz, 10kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
{
        0xF706,     //TX1 register,     Fdev = 37.5kHz
        0x0000,		//WOR1 register,
        0xF800,     //WOR2 register,
        0x1107,		//RFI register, 	Enable Tx Ramp up/down
        0x9970,     //PM register,      CST=1
        0x0201,		//RTH register,
        0x400F,     //AGC1 register,	// 400F
        0x2AC0,     //AGC2 register,
        0x0045,		//GIO register, 	GIO2=WTR  Inverted output, GIO1=FSYNC
        0xD281,		//CKO register
        0x0004,		//VCB register,
        0x0825,		//CHG1 register, 	480MHz
        0x0127,		//CHG2 register, 	500MHz
        0x000F,		//FIFO register, 	FEP=63+1=64bytes
        0x155F,		//CODE register, 	Preamble=4bytes, ID=4bytes
        0x0000		//WCAL register,
};

const Uint16 A7139Config_PageB[]=   //470MHz, 10kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz
{
        0x035F,		//TX2 register,
        0x8400,		//IF1 register, 	Enable Auto-IF, IF=200KHz
        0x0000,		//IF2 register,
        0x0000,		//ACK register,
        0x0000		//ART register,
};

