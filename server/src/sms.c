/*
 * sms.c
 *
 *  Created on: 2016年5月12日
 *      Author: lovenix
 */

#include "../inc/main.h"

int GetIp(char* url,char* ip)
{
    struct hostent* host;
    char* p = NULL;
    if((host = gethostbyname(url)) != NULL)
	{
		p = inet_ntoa(*((struct in_addr *)host->h_addr_list[0]));
		memcpy(ip,p,strlen(p));
		return 1;
	}
	else
		return 0;
}

int GisCommit(char *lat,char *lon,char *time,int alarm)
{
    int sockfd,index;
    char url[1024] = {0};
    char rBuf[2048] = {0};
    char ip[20] = {0};
    struct sockaddr_in address;
    char *REQ = "GET /index.php?d=api&c=saas_gw_thing&m=action&thingsn=&lat=%s&lon=%s&time=%s&alarm=%d HTTP/1.1\r\nHost:api.cellocation.com:\r\nConnection: Close\r\n\r\n";
    snprintf(url,sizeof(url),REQ,lat,lon,time,alarm);
    if(GetIp("www.tgj168.com",ip) == 0)
        return 0;
  //  printf("LBS[%d]: %s\n",strlen(url),url);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(80);
    if(connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        close(sockfd);
        return 0;
    }
    if(_write(sockfd,url,strlen(url)) == 0)
    {
        close(sockfd);
        return 0;
    }
    while(1)
    {
        if(recv(sockfd, rBuf, sizeof(rBuf), 0) <= 0)
            break;
    }
 //   printf("LBS read: %s\n",rBuf);
    /*
    if((index = kmp(rBuf,"utf-8")) != -1)
    {
        index += 5;
        if(strlen(&rBuf[index]) > 20)
        {
            if(sscanf(&rBuf[index],"%*[^,],%11[^,]",pWf->sGi.llat) == 1)
            {
                sscanf(&rBuf[index],"%*[^,],%*[^,],%11[^,]",pWf->sGi.llon);
                index = 1;
            }
        }
        else
        {
            m_printf("lbs get len: %d\n",strlen(&rBuf[index]));
            index = 0;
        }
    }
    else
        index = 0;
        */
    close(sockfd);
    return index;
}
/*
int GetLBS(WINF *pWf)
{
	int sockfd,index;
	char url[1024] = {0};
	char rBuf[2048] = {0};
	char ip[20] = {0};
	struct sockaddr_in address;
	char *LBS_REQ_A = "GET /cell/?mcc=460&mnc=%s&lac=%s&ci=%s&output=csv HTTP/1.1\r\nHost:api.cellocation.com:\r\nConnection: Close\r\n\r\n";
	char *LBS_REQ_B	= "GET /loc/?cl=460,%s,%s,%s,%s;460,%s,%s,%s,%s&output=csv HTTP/1.1\r\nHost:api.cellocation.com:\r\nConnection: Close\r\n\r\n";
	char *LBS_REQ_C	= "GET /loc/?cl=460,%s,%s,%s,%s;460,%s,%s,%s,%s;460,%s,%s,%s,%s&output=csv HTTP/1.1\r\nHost:api.cellocation.com:\r\nConnection: Close\r\n\r\n";
	if(GetIp("api.cellocation.com",ip) == 0)
		return 0;
	w_printf("lbn: %d\n",pWf->lbn);
	switch(pWf->lbn)
	{
	case 1:
		snprintf(url,sizeof(url),LBS_REQ_A,
				pWf->mnc,pWf->lac[0],pWf->cellid[0]);
		break;
	case 2:
		snprintf(url,sizeof(url),LBS_REQ_B,
				pWf->mnc,pWf->lac[0],pWf->cellid[0],pWf->rssi[0],
				pWf->mnc,pWf->lac[1],pWf->cellid[1],pWf->rssi[1]);
		break;
	default:
		snprintf(url,sizeof(url),LBS_REQ_C,
				pWf->mnc,pWf->lac[0],pWf->cellid[0],pWf->rssi[0],
				pWf->mnc,pWf->lac[1],pWf->cellid[1],pWf->rssi[1],
				pWf->mnc,pWf->lac[2],pWf->cellid[2],pWf->rssi[2]);
		break;
	}
	w_printf("LBS[%d]: %s\n",strlen(url),url);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port = htons(80);
	if(connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		close(sockfd);
		return 0;
	}
	if(_write(sockfd,url,strlen(url)) == 0)
	{
		close(sockfd);
		return 0;
	}
	while(1)
	{
		if(recv(sockfd, rBuf, sizeof(rBuf), 0) <= 0)
			break;
	}
//	m_printf("LBS read: %s\n",rBuf);
	if((index = kmp(rBuf,"utf-8")) != -1)
	{
		index += 5;
		if(strlen(&rBuf[index]) > 20)
		{
			if(sscanf(&rBuf[index],"%*[^,],%11[^,]",pWf->sGi.llat) == 1)
			{
				sscanf(&rBuf[index],"%*[^,],%*[^,],%11[^,]",pWf->sGi.llon);
				index = 1;
			}
		}
		else
		{
			m_printf("lbs get len: %d\n",strlen(&rBuf[index]));
			index = 0;
		}
	}
	else
		index = 0;
	close(sockfd);
	return index;
}

int SendMSG(int num,const char *context,const char* phone, ...)
{
	int sockfd,rs;
	char number[512] = {0};
	char url[2048] = {0};
	struct sockaddr_in address;
	if(num > 31 || num < 1)
		return 0;
	va_list mark;
	va_start(mark,phone);
	strcat(number,phone);
	while(--num)
	{
		strcat(number,",");
		phone = va_arg(mark,const char *);
		strncat(number,phone,11);
	}
	va_end(mark);
	snprintf(url,sizeof(url),REQ,SMS_USER,SMS_PASSWD,context,number,SMS_EPID);

	m_printf("SMS: %s\n",url);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(DestIp);
	address.sin_port = htons(DestPort);
	if(connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		close(sockfd);
		return 0;
	}
	rs = _write(sockfd,url,strlen(url));
#ifdef BUG
	char rcvBuf[2048] = {0};
    while(1)
    {
    	num = recv(sockfd, rcvBuf, 2048, 0);
    	if((0 == num) || (-1 == num))
    		break ;
    }
	printf("SMS socket recv: %s\n", rcvBuf);
#endif
	close(sockfd);
	return rs;
}
*/

