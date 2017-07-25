#include "../inc/main.h"
#include "../inc/network.h"
int kdpfd;
int rsfd;
struct epoll_event ev;
struct epoll_event events[MAXEPOLLSIZE];

/******* setnonblocking - 设置句柄为非阻塞方式 *******/
int setnonblocking(int sockfd)
{
	struct timeval timeout = { 3, 0 };
	int mw_optval = 1;
	int nRecvBuf = 512*1024;		//	512K
	int nSendBuf = 512*1024;		//	512K
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,(char *)&mw_optval,sizeof(mw_optval));
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&mw_optval,sizeof(mw_optval)); 	// 设置端口多重邦定
	setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(timeout));
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout));
	setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
	setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1)
		return 0;
	return 1;
}

void *RobotServer(void *arg)
{
	int nfds, n, lisnum, rfd;
	struct timeval ntime;
	struct sockaddr_in ser_addr, cli_addr;

	socklen_t len = sizeof(cli_addr);
	lisnum = MAXLISTEN;
	if ((rsfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) 	// 开启 socket 监听
	{
		lprintf(lfd, FATAL, "Robot: Socket error!");
		exit(1);
	}
	else
		lprintf(lfd, INFO, "Robot: Socket create success!");
	bzero(&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = PF_INET;
	ser_addr.sin_port = htons((uint16_t)atoi(arg));
	ser_addr.sin_addr.s_addr = INADDR_ANY;
	setnonblocking(rsfd);		// 设置非阻塞模式
	if (bind(rsfd, (struct sockaddr *) &ser_addr, sizeof(struct sockaddr)) == -1)
	{
		lprintf(lfd, FATAL, "Robot: TCP bind error!");
		exit(1);
	}
	else
		lprintf(lfd, INFO, "Robot: TCP bind success!");
	if (listen(rsfd, lisnum) == -1)
	{
		lprintf(lfd, FATAL, "Robot: TCP listen error!");
		exit(1);
	}
	else
		lprintf(lfd, INFO, "Robot: TCP listen success!");
	kdpfd = epoll_create(MAXEPOLLSIZE);
	ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP ; 		// 注册epoll 事件
	ev.data.fd = rsfd;
	if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, rsfd, &ev) < 0)
	{
		lprintf(lfd, FATAL, "Robot: EPOLL_CTL_ADD error!");
		exit(1);
	}
	while (1)
	{
	//	sem_wait(&sem_rcm);
		nfds = epoll_wait(kdpfd, events, MAXEPOLLSIZE, 20);
		if (nfds == -1)
		{
			lprintf(lfd, FATAL,"Robot: EPOLL_WAIT %s!",strerror(errno));
			continue;
		}
		for (n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == rsfd) // 如果是连接事件
			{
				while ((rfd = accept(rsfd, (struct sockaddr *) &cli_addr,&len)) > 0)
				{
				//	lprintf(lfd, INFO, "Robot: TCP Client from [%s]\tSocket ID [%d]", inet_ntoa(cli_addr.sin_addr),rfd);
					setnonblocking(rfd);
					ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;	// 注册新连接
					ev.data.fd = rfd;
					if(epoll_ctl(kdpfd, EPOLL_CTL_ADD, rfd, &ev) < 0) // 将新连接加入EPOLL的监听队列
						lprintf(lfd, FATAL, "Robot: EPOLL_CTL_ADD error!");
					else
					{
						gettimeofday(&ntime, NULL);
						rInfo[rfd].cntime = ntime.tv_sec;		// 登记接入时间
						rInfo[rfd].tfd = rfd;
						snprintf(rInfo[rfd].ip,16,"%s",inet_ntoa(cli_addr.sin_addr));
					}
				}
				if(rfd == -1)
				{
					if(errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
						lprintf(lfd, FATAL, "Robot: TCP accept error!");
				}
			}
			else if(events[n].events & EPOLLRDHUP)		// 2.6.17 UP	0x2000
				ResetRClient(events[n].data.fd);		// close client
			else if (events[n].events & EPOLLIN)		// 0x001
				tpool_add_work(pool, RobotProcess, events[n].data.fd);		// 读取分析TCP信息
			else
			{
			 	epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
			 	close(events[n].data.fd);
			}
		}
	//	sem_post(&sem_rcm);
	}
	close(rsfd);
}

void* HttpServer(void *arg)
{
#define SERVPORT 4444 // 服务器监听端口号
#define BACKLOG 10 // 最大同时连接请求数
    int index = 0,i;
    unsigned char buf[1024] = {0};
    char sn[17] = {0};
    char nid[8] = {0};
    unsigned char hex[4] = {0};
    char back[2] = {0};
    unsigned char ctrl[4] = {0};
    unsigned char word[8] = { 0xFE,0x08,0x01,0x00,0x00,0x00,0x00,0xFF };
    int sockfd,client_fd,rLen=0;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        lprintf(lfd, INFO, "Robot: Http server socket error!");
        exit(1);
    }
    my_addr.sin_family=AF_INET;
    my_addr.sin_port=htons(SERVPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero),8);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        lprintf(lfd, INFO, "Robot: Http server bind error!");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        lprintf(lfd, INFO, "Robot: Http server listen error!");
        exit(1);
    }

    while(1)
    {
        if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size)) == -1)
        {
            lprintf(lfd, INFO, "Robot: Http server accept error!");
            continue;
        }
//        printf("received a connection from %s\n", inet_ntoa(remote_addr.sin_addr));
        if((rLen = recv(client_fd, buf, 1024, 0)) == -1)
            close(client_fd);
        else
        { 
//        	printf("%s\n",buf);
            memcpy(sn,&buf[9],16);
            index = GetHashTablePos(sn,RidHash,MHI);
//            printf("sn: %s\n",sn);
//            printf("index : %d\n",index);
//            printf("rhiv : %d\n",hInfo[index].rhiv);
            if(hInfo[index].rhiv > 0)           // 在线
            {
                memcpy(nid,&buf[1],8);
 //               printf("nid: %s\n",nid);
                StringToHex(hex,nid);
                memcpy(&word[3],hex,4);
                back[0] = '1';
                _write(hInfo[index].rhiv,word,8);
#if 0
                for(i=0; i<8; i++)
                    printf("%02x\t",word[i]);
                printf("\n");
#endif
            }
            else
                back[0] = '0';
            if(send(client_fd,back,1,0) < 0)
            	lprintf(lfd, INFO, "Robot: http send message error!");
//                printf("send message error\n");
            close(client_fd);
        }
    }
}

