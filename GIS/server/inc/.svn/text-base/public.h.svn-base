#ifndef PUBLIC_H_
#define PUBLIC_H_
#include "kmp.h"

typedef struct a{
	double longitude;
	double latitude;
}CLLocationCoordinate2D;

/*********** public.h ***************/
extern int HexToString(char *dst,const unsigned char *src,int sLen);
extern void StringToHex(unsigned char *dst,const char *src);
extern int StringToDEC(unsigned char *dst,const char *src);

extern double atod(char *temp, int len);
extern void dtoa(double n, char s[]);
extern double power(double x, int y);
extern int _write(int fd,void *buffer,int length);
extern void gtime(char *date);				// 获取当前时间
extern char *SubLeft(char *dst,char *src, int n);
extern int stop_process(char* pName);
extern void stop(int signo);
extern char *SubMid(char *dst,char *src, int n,int m);
extern int IdCrc(unsigned char* id,int len);
extern void gdate(char *date);
extern int _m_write(int fd,void *buffer,int length);
extern int CodeConvert(char **src, size_t *sLen, char **dst, size_t *dLen);
extern void DECToString(char *dst,const unsigned char *src,int sLen);
extern int raCopy(unsigned char* dst,const unsigned char *src,int len);		// right aligned copy
extern void tprintf(void);
extern int CodeConv(char *src,char *dst,char *from,char *to);
extern int GetBit(int num);
extern void m_printf(const char *format,...);
extern void r_printf(const char *format,...);
extern void w_printf(const char *format,...);
extern void u_printf(const char *format,...);
extern void s_printf(const char *format,...);
extern int FileClear(char *path);
extern CLLocationCoordinate2D Bd2Wgs(CLLocationCoordinate2D coordinate);
#define HG_LARGE_STR_LEN 256
#define RDEBUG
#define MDEBUG
//#define WDEBUG
#define ERRPRT
//#define SQL
#define UDP

#endif
