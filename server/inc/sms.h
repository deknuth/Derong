#ifndef INC_SMS_H_
#define INC_SMS_H_

#define SMS_USER	"rlkj"
#define SMS_PASSWD	"rlkj123"
#define SMS_EPID	"121366"
#define DestIp "114.255.71.158"
#define DestPort 8061
//#define REQ "GET /?username=%s&password=%s&message=%s&phone=%s&epid=%s&linkid=&subcode= HTTP/1.1\r\nHost:114.255.71.158:\r\nConnection: Close\r\n\r\n"

#define MAX_BODY 2048
#define APP_KEY "e5a2e90c607a41f60ce80cfc"//AppKey
#define API_MASTER_SECRECT "7718d91257c32c99bbca7399"//API MasterSecert
#define USE_OPENSSL

typedef struct vc{
	unsigned int VerifCode;
	unsigned char phone[13];
	unsigned char flag;			// 1-> already request verification code
	unsigned int mfd;
	unsigned char master;
}VECODE[8192];
extern VECODE sInfo;
extern int SendMSG(int num,const char *context,const char* phone, ...);
extern int PushMessageV3(int num, int type, char* alias, char* context, int value, ...);
extern int PushMessage(char* alias,char* context,int type);
extern int GisCommit(char *lat,char *lon,char *time,int alarm);
#endif

