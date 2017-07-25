#ifndef ROBOT_H_
#define ROBOT_H_
#define V_PATH	"/home/oracle/live/mediaServer/video/"

typedef struct xx{
	unsigned char r_type;			// 设备类型
	unsigned char r_mac[17];		// 设备mac地址
	unsigned char r_resever[5];		// 预留
//	NINFO rnInfo;
}ROBOT;

extern int CodeValid(unsigned char* plainBlock,unsigned char* cipherBlock);
extern int RobotReg(int rfd,unsigned char* rBuf);		// robot online register
extern int RobotAlive(int rfd,unsigned char* rBuf);
extern int DESCrc(unsigned char* plainBlock,unsigned char* cipherBlock,int len);
extern int RobotInit(int rfd,unsigned char* rBuf);		// robot initialization
extern int rRegBack(int rfd,int stat);		//robot register back
extern int rCrtBack(int rfd,unsigned char* rBuf,unsigned char len);
extern int GisInfo(unsigned char *buf);
extern ROBOT *RBInfo;
#endif	// ROBOT_H_ 

