#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <syscall.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sys/stat.h>
#include <locale.h>
#include <wchar.h>
#include <semaphore.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <limits.h>
#include <termios.h>    	// PPSIX 终端控制定义
#define j2h(x) (3.14159265359*(x)/180.0)
double sinValue[16384] = {0};
double cosValue[16384] = {0};
unsigned char GetCrcPackage(unsigned char B, unsigned char C)
{
    static unsigned char cbit[256] = {
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,
        4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,
        4,5,1,2,2,3,2,3,3,4,2,3,3,4,3,
        4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,
        5,5,6,1,2,2,3,2,3,3,4,2,3,3,4,
        3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,
        4,5,5,6,2,3,3,4,3,4,4,5,3,4,4,
        5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,
        6,5,6,6,7,1,2,2,3,2,3,3,4,2,3,
        3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,
        4,5,4,5,5,6,2,3,3,4,3,4,4,5,3,4,
        4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,
        5,6,5,6,6,7,2,3,3,4,3,4,4,5,3,4,
        4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,
        5,6,5,6,6,7,3,4,4,5,4,5,5,6,4,5,
        5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,
        6,7,6,7,7,8,
    };
    return (cbit[B]+cbit[C])&0x07;
}

/***写串口函数****/
int _write(int fd, void *buffer, int length)
{
    int bytes_left;
    int written_bytes;
    char *ptr;

    ptr = buffer;
    bytes_left = length;
    while (bytes_left > 0)
    {
        /********开始写 ***********/
        written_bytes=write(fd, ptr, bytes_left);
        if (written_bytes <= 0) // 出错了
        {
            if (errno==EINTR) // 中断错误继续写
                written_bytes = 0;
            else
                // 其他错误退出
                return (-1);
        }
        bytes_left -= written_bytes;
        ptr += written_bytes; // 从剩下的地方继续写
    }
    return (0);
}
unsigned int speed = 0;
/*************   读串口函数,非阻塞模式   **********/
int ReadDevice(int hComm)
{
    unsigned char temp;
    int count = 0;
    double axis[360][2] = {0};
    int fd;
    fd_set readset; // 设置文件描述符集
    struct timeval tv;
    int MaxFd = 0;
    int c = 0;
    int len = 0;
    int nFd; // 文件描述符个数
    unsigned int distance = 0;
    unsigned char buf[18] = {0};
    unsigned char mix[48] = {0};
    unsigned int angle = 0;
    unsigned int first=0,second=0;
    memset(buf, 0x00, 18); // 初始化
    memset(mix, 0x00, 48); // 初始化
    fd = open("/root/zb.txt", O_RDWR | O_CREAT | O_APPEND, 0644); //|O_APPEND
    while(1)
    {
        do{
            FD_ZERO( &readset); // 文件描述符置0
            if (hComm >= 0)
                FD_SET(hComm, &readset);
            MaxFd = hComm + 1; // 最大文件描述符数
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            do {
                nFd = select(MaxFd, &readset, 0, 0, &tv); // 检测状态
            } while (nFd==-1 && errno==EINTR); // 是否中断
            if (nFd <= 0)
                hComm = -1;
            else if(FD_ISSET(hComm, &readset)) // FD_ISSET检测fd是否设置
            {
                if(read(hComm, &temp, 1))
                {
                    buf[count++] = temp;
                    if(temp&0x80)
                    {
                        if(count == 4)
                        {
                            distance = buf[1];
                            distance |= ((buf[0]&0x7F)<<7);
                            angle = (buf[3]&0x7F);      // LOW 7 BIT
                            angle |= (buf[2]<<7);
                            second = first;
                            first = angle;
                            if(abs(second-first) > 10000)
                            {
                                second = first;
                                speed++;
                            }
          //                  printf("distance: %d\tangle: %d\n",distance,angle);
                            len = sprintf(mix,"%0.8f,%0.8f\n",distance*cosValue[angle],distance*sinValue[angle]);
                            if(speed == 3)
                                _write(fd,mix,len);
                            memset(mix,0x00,len);
                        }
                        count = 0;
                    }
                    if(count > 4)
                        count = 0;
                }
                else
                    break;
            }
        }while(hComm >= 0);
    }
    return 1;
}

/***@brief  设置串口通信速率
 *@param  fd     类型 int  打开串口的文件句柄
 *@param  speed  类型 int  串口速度
 *@return  void
 */
#define BAUDRATE B230400
int open_com(char *DevPath)
{
    int fd;
    struct termios newtio;
    fd = open(DevPath, O_RDWR|O_NOCTTY|O_NDELAY);
    if (fd == -1)
    {
        printf("can't open !\n");
        return (-1);
    }
    tcgetattr(fd,&newtio);
    bzero(&newtio,sizeof(newtio));
    //setting c_cflag
    newtio.c_cflag |= (CLOCAL|CREAD);
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag &= ~PARODD;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;
    newtio.c_oflag |= OPOST;
    //newtio.c_lflag = 0;
    //newtio.c_cc[VTIME] = 5;
    //newtio.c_cc[VMIN] = 0;
    //setting c_iflag
    newtio.c_iflag &=~ (IXON|IXOFF|IXANY);
    cfsetispeed(&newtio,BAUDRATE);
    cfsetospeed(&newtio,BAUDRATE);
    //printf("speed=%d\n",baud);
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
    return(fd);
}

void printMes(int signo)
{
    printf("Motor speed : %d !\n",speed);
 //   speed = 0;
}

int main(void)
{
    int cfd = 0,i;
    float step = 0.0;
    struct itimerval tick;

    for(i=0; i<16384; i++)
    {
        sinValue[i] = sin(j2h(step));
        cosValue[i] = cos(j2h(step));
        step += 0.02197265625;
    }


    signal(SIGALRM, printMes);
    memset(&tick, 0, sizeof(tick));

    //Timeout to run first time
    tick.it_value.tv_sec = 1;
    tick.it_value.tv_usec = 0;

    //After first, the Interval time for clock
    tick.it_interval.tv_sec = 1;
    tick.it_interval.tv_usec = 0;

    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
        printf("Set timer failed!\n");

    cfd = open_com("/dev/ttyUSB0");
    ReadDevice(cfd);
}
