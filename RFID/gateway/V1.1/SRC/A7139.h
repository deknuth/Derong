#ifndef A7139_H_
#define A7139_H_

typedef unsigned char Uint8;
typedef unsigned int  Uint16;
typedef unsigned long Uint32;
typedef signed   char Sint8;
typedef signed   int  Sint16;
typedef signed   long Sint32; 

#define SYSTEMCLOCK_REG 	0x00
#define PLL1_REG 			0x01
#define PLL2_REG 			0x02
#define PLL3_REG 			0x03
#define PLL4_REG			0x04
#define PLL5_REG			0x05
#define PLL6_REG			0x06
#define CRYSTAL_REG			0x07
#define PAGEA_REG	  		0x08
#define PAGEB_REG			0x09
#define RX1_REG  			0x0A
#define RX2_REG  			0x0B
#define ADC_REG  			0x0C
#define PIN_REG 			0x0D
#define CALIBRATION_REG  	0x0E
#define MODE_REG  			0x0F

#define TX1_PAGEA           0x00
#define WOR1_PAGEA          0x01
#define WOR2_PAGEA          0x02
#define RFI_PAGEA           0x03
#define PM_PAGEA            0x04
#define RTH_PAGEA           0x05
#define AGC1_PAGEA          0x06
#define AGC2_PAGEA          0x07
#define GIO_PAGEA           0x08
#define CKO_PAGEA           0x09
#define VCB_PAGEA           0x0A
#define CHG1_PAGEA          0x0B
#define CHG2_PAGEA          0x0C
#define FIFO_PAGEA			0x0D
#define CODE_PAGEA			0x0E
#define WCAL_PAGEA			0x0F

#define TX2_PAGEB			0x00
#define	IF1_PAGEB			0x01
#define IF2_PAGEB			0x02
#define	ACK_PAGEB			0x03
#define	ART_PAGEB			0x04


#define CMD_Reg_W			0x00	//000x,xxxx	control register write
#define CMD_Reg_R			0x80	//100x,xxxx	control register read
#define CMD_CID_W			0x20	//001x,xxxx	ID write
#define CMD_CID_R			0xA0	//101x,xxxx	ID Read
#define CMD_FIFO_W			0x40	//010x,xxxx	TX FIFO Write
#define CMD_FIFO_R			0xC0	//110x,xxxx	RX FIFO Read
#define CMD_RF_RST			0x70	//x111,xxxx	RF reset
#define CMD_TFR				0x60	//0110,xxxx	TX FIFO address pointrt reset
#define CMD_RFR				0xE0	//1110,xxxx	RX FIFO address pointer reset

#define CMD_SLEEP			0x10	//0001,0000	SLEEP mode
#define CMD_IDLE			0x12	//0001,0010	IDLE mode
#define CMD_STBY			0x14	//0001,0100	Standby mode
#define CMD_PLL				0x16	//0001,0110	PLL mode
#define CMD_RX				0x18	//0001,1000	RX mode
#define CMD_TX				0x1A	//0001,1010	TX mode
//#define CMD_DEEP_SLEEP	0x1C	//0001,1100 Deep Sleep mode(tri-state)
#define CMD_DEEP_SLEEP		0x1F	//0001,1111 Deep Sleep mode(pull-high)

#define ERR_PARAM 			0x01
#define ERR_PLATFORM		0x02
#define ERR_UNK				0x03
#define ERR_CAL				0x04
#define ERR_TMO				0xFF		
#define ERR_RCOSC_CAL		0x04
#define OK_RCOSC_CAL		0x05
#define ERR_GET_RSSI		0x00

extern void	StrobeCmd(Uint8 cmd);
extern Uint8  A7139_Init(float rfFreq);
extern Uint16 A7139_ReadPID(void);
extern Uint8  A7139_SetCID(Uint32 id);
extern Uint8  A7139_SetDataRate(Uint8 datRate);
extern Uint8  A7139_SetPackLen(Uint8 len);
extern Uint8 A7139_GetRSSI(void);
extern Uint8  A7139_WOR_ByPreamble(void);
extern Uint8  A7139_WOR_BySync(void);
extern Uint8  A7139_IsBatteryLow(Uint8 low2_x);
extern Uint8  A7139_SetDataRate(Uint8 datRat);
extern Uint8  A7139_SetPowerLevel(Uint8 pwrLev);
extern Uint8 A7139_ReadCID(unsigned char *id);

extern void SPIx_WriteByte(unsigned char src);
extern unsigned char SPIx_ReadByte(void);
extern void SPIx_WriteWord(Uint16 src);
extern Uint16 SPIx_ReadWord(void);
extern unsigned char A7139_ReadFIFO(unsigned char* buf);
extern void A7139_WriteFIFO(unsigned char *buf,unsigned char len);

#endif

