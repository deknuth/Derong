/*
 * network.h
 *
 *  Created on: 2015年12月26日
 *      Author: Lovenix
 */

#ifndef INC_NETWORK_H_
#define INC_NETWORK_H_

#define ROBOT_PORT		"7777"			// robot client port

#define _FILE_OFFSET_BITS 64
#define MAX_REC	50000
#define MAXBUF 8192
#define MAXEPOLLSIZE MAX_REC
#define MAXLISTEN 20
#define PORT 5555
#define	ENC		"lovenix"
#define JAVA_STR "update"
#define	TEST	1111
#define MAXRPL	64					// MAX packet length of robot
#define MAXMPL	2048				// MAX packet length of mobile

#define MAX_EQU	512
#define ROBOT_HEART_TIME 	300
#define ROBOT_DEAD_TIME 	45

#define MHI	(MAX_REC<<1)+1			// max hash index
#define NANOSECOND 50000000UL
#define UNS 	90000000UL
#define MWU	5						// max watch user
#define SHI	8192

typedef struct ri{
	int tfd;			// Robot TCP handle
	int tflag;			// TCP	连接标志
	int cflag;			// client close flag
	int hiv;					// hash index value
//	char rst[17];		// 机器人状态		0--离线  1--在线   2--3g在线
	char id[17];		// 设备id
	char alias[17];		// PUSH SERVER ALIAS
	char ip[16];		// client IP
	char mPhone[12];			// main phone
//	NINFO rnInfo;
	unsigned char lip[4];		// locale IP
	unsigned char wc;
	unsigned char uFlag;		// update flag
	unsigned int uPort;
	unsigned int cntime;			// TCP 	持续时间
	unsigned char version;
}RINFO,*PRINFO;//[MAX_REC];

typedef struct{
	int radius;
	int LRTime;				// last alarm time
	double base_lon;		// 基准经纬度
	double base_lat;
	double lon;				// 测量所得经纬度
	double lat;
	unsigned char on;		// on off
	unsigned char cycle;	// fence cycle
	unsigned char flag;		// 定位标志	1->已定位,0->未定位
	double last_lon;
	double last_lat;
}LON_LAT,*P_LON_LAT;

typedef struct hi{
	int rhiv;			// robot client socket handle index hash
	int mcun;			// mobile connect user number
//	int whiv;			// watch client socket handle index hash
	int pValid;			// 是否有图片传送请求
	int mfd;			// save mobile socket id
	int wfd;			// watch socket handle
}HIV[MHI];


struct cli_info{
	int sockfd;
};


extern PRINFO rInfo;
extern HIV hInfo;


extern void* RClientManage(void *arg);
extern void UpdateRClient(int rfd);
extern void ResetRClient(int rfd);
extern void *RobotServer(void *arg);
extern int setnonblocking(int sockfd);
extern int kdpfd;
extern struct epoll_event ev;

extern int rsfd;

#endif /* INC_NETWORK_H_ */
