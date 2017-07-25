#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IPSTR "120.25.205.94"
#define PORT 80
#define BUFSIZE 1024

int _write(int fd,void *buffer,int length)
{
    if(fd < 3)
        return 0;
    int left;
    int wLen;
    char *ptr;
    fd_set fds;
    struct timeval tv={3,0};
    ptr = buffer;
    left = length;
    while(left > 0)
    {
        FD_ZERO(&fds);
        FD_SET(fd,&fds);
        tv.tv_sec = 2;
        switch(select(fd+1,0,&fds,NULL,&tv))
        {
        case -1:
            return 0;
        case 0:
            break;
        default:
            if(FD_ISSET(fd,&fds))
            {
                wLen = write(fd,ptr,left);
                if(wLen <= 0)
                {
                    if(errno == EINTR)
                        wLen = 0;
                    else if(errno == EAGAIN)
                        wLen = 0;
                    else
                    {
                        printf("Write: %s",strerror(errno));
                        return 0;
                    }
                }
                left -= wLen;
                ptr += wLen;
            }
            break;
        }
    }
    return 1;
}
/*120.77.159.8:8080*/
int post(char* context)
{
    int sockfd,index;
    char url[1024] = {0};
    char rBuf[2048] = {0};
    char ip[20] = {"120.25.205.94"};
    struct sockaddr_in address;
    char a[] = "POST http://120.25.205.94/cgi-bin/ds/test HTTP/1.1\r\n";
    char b[] = "Host: 120.25.205.94\r\n";
    char c[] = "Content-Length: %d\r\n\r\n";
    char d[128] = {0};
    sprintf(d,c,strlen(context));
    sprintf(url,"%s%s%s%s",a,b,d,context);
    printf("%s\n",url);
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
    printf("LBS read: %s\n",rBuf);
    close(sockfd);
    return index;
}

int main(void)
{
    char context[512] = { "{\"gateId\":\"1095\",\"deviceId\":\"A116\"}"};

    post(context);
}
