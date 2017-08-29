#include "../inc/main.h"
#include "../inc/robot.h"

char date[15] = {0};

int RobotInit(int rfd,unsigned char* rBuf)
{
	int rb,index;
	char sn[17] = {0};
	char sql[512] = {0};
	unsigned char plainBlock[8] = {0};
	unsigned char cipherBlock[8] = {0};
	unsigned char decrypt[8] = {0};
	unsigned char pack[12] = {0};
	struct timeval ntime;

    memcpy(plainBlock,&rBuf[3],8);
    HexToString(sn,plainBlock,8);

	printf("lsn: %s\n",sn);
	index = GetHashTablePos(sn,RidHash,MHI);
	if(index == 0)
	{
		index = InsertHash(sn,RidHash,MHI);
		rInfo[rfd].tflag = rfd;				// set new client info or set socket online flag
		hInfo[index].rhiv = rfd;			// change  hash index value or storage rfd hash
		rInfo[rfd].hiv = index;
       	memcpy(rInfo[rfd].id,sn,16);
		atomic_inc(R_CON_NUM);
    }
    else
    {
		if(rInfo[hInfo[index].rhiv].tfd == 0)			// first login
			lprintf(lfd, INFO, "Robot: [%s] [%s] link up success!",sn,rInfo[rfd].ip);
		else											// login again
		{
			rInfo[hInfo[index].rhiv].cflag = -2;		// close last socket
			gettimeofday(&ntime, NULL);
			rInfo[hInfo[index].rhiv].cntime = (ntime.tv_sec - ROBOT_HEART_TIME);
			lprintf(lfd, INFO, "Robot: [%s] connection update success!",sn);
		}
      	rInfo[rfd].tflag = rfd;				// set new client info or set socket online flag
		hInfo[index].rhiv = rfd;			// change  hash index value or storage rfd hash
		rInfo[rfd].hiv = index;
       	memcpy(rInfo[rfd].id,sn,16);
       	atomic_inc(R_CON_NUM);
    }
    rb = 0x101;		// success
	return rb;
}

int post(char* context)
{
    int sockfd,index;
    char url[1024] = {0};
    char rBuf[2048] = {0};
    char ip[20] = {"120.77.159.8"};
    char con_base64[1024] = {0};
    struct sockaddr_in address;
    char a[] = "POST http://120.77.159.8:8080/spd-web/mapper/Car/addOrUpdataCar HTTP/1.1\r\n";
    char b[] = "Host: 120.77.159.8\r\n";
    char c[] = "Content-Length: %d\r\n\r\n";
    char d[128] = {0};
    sprintf(d,c,strlen(context));
    base64Encoder(context,con_base64);
    sprintf(url,"%s%s%s%s",a,b,d,con_base64);
    printf("%s\n",url);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(8080);
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
    printf("LBS read: %s\n",rBuf);
    close(sockfd);
    return index;
}

const unsigned char tab[16]={ 48,49,50,51,52,53,54,55,56,57,48,48,48,48,48,48 };
int GisInfo(unsigned char *buf)
{
		int len = 0;
		int i;
		char req[512] = {0};
		char dst[8] = {0};
		unsigned char temp[4] = {0};
		len = buf[1]-4;			// ID³¤¶È
		for(i=0; i<len; i+=4)
		{
				memcpy(temp,&buf[3+i],4);
				HexToString(dst,temp,4);	// hex to ASCII
				strcat(req,dst);
				strcat(req,",");
		}
		req[strlen(req)-1] = 0x00;
		printf("%s\n",req);
		post(req);
    //GisCommit(lat_s,lon_s,time_s,buf[16]);
    return 1;
}

/*
int RobotAlive(int rfd,unsigned char* rBuf)		// robot Keep Alive
{
	char sn[17] = {0};
	unsigned char plainBlock[8] = {0};
	unsigned char cipherBlock[8] = {0};

	memcpy(plainBlock,&rBuf[9],8);
	memcpy(cipherBlock,&rBuf[17],8);

	HexToString(sn,plainBlock,8);
	if(CodeValid(plainBlock,cipherBlock))
	{
		if(rInfo[rfd].tflag != 0)		// already register
			if (rInfo[rfd].hiv == GetHashTablePos(sn,RidHash,MHI))	// ID already insert into hash table
				return 1;
	}
	return 0;
}
*/





