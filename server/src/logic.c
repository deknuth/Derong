#include "../inc/main.h"

void RobotProcess(int rfd) 	// robot logic processing
{
    int rLen, i = 0,j=0;
    unsigned char pLen = 0;
    unsigned char rBuf[1024] = { 0 };
    unsigned char pack[MAXRPL] = { 0 };
    while ((rLen = read(rfd, rBuf, 1024)) > 0)
    {
    	/*	
        for(j=0;j<rLen;j++)
            printf("%x\n",rBuf[j]);
        printf("\n");
        */
        pLen = i = 0;
        while (1)
        {
            while (i < rLen)
            {
                if (rBuf[i] == 0xfe)		// FIND head
                    break;
                i++;
            }
            if ((i + 5) >= rLen)
                break;
            pLen = rBuf[i+1];
            if ((i + pLen) <= rLen && pLen < MAXRPL && pLen >= 6)
            {
                memset(pack, 0x00, MAXRPL);
                memcpy(pack, &rBuf[i], pLen);
                if (pack[pLen - 1] == 0xff)		// control packet
                {
                    switch (pack[2])
                    {
                    case 0:
                        if(pLen == 6)
                        {
                   //         printf("heart pack!\n");
                            UpdateRClient(rfd);
                        }
                        break;
                    case 1:				// online register
                        if (pLen == 12)
                        {
                  //          printf("register pack!\n");
                            if (rInfo[rfd].tflag == 0)
                            {
                                if (RobotInit(rfd, pack) == 0x101)
                                    UpdateRClient(rfd);
                            }
                            else	// already register
                            {
//                                rRegBack(rfd, 0x101);//robot register back
                                UpdateRClient(rfd);
                            }
                        }
                        break;

                    case 2:
                        GisInfo(pack);
                        UpdateRClient(rfd);
                        break;

                    default:
                        break;
                    }

                }
                else	// protocol error
                    lprintf(lfd, INFO,"Robot: Get a rubbish pack of robot client!");
                memset(pack, 0x00, pLen);
                i += (pLen - 1);
            }
            else
                i++;
        }
        memset(rBuf,0x00,rLen);
    }
    if (rLen == -1 && errno != EAGAIN)			// socket closed
        lprintf(lfd, WARN, "Robot: socket read error!");
    else if (rLen == 0)
        ResetRClient(rfd);
    else	// normal exit
        ;
    if(errno == EINTR)
        lprintf(lfd, WARN, "Robot: AAAAAAAAAAAAAAAAAAAAAAAAAAAAA!");
}

