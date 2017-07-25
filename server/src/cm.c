#include "../inc/main.h"
#include "../inc/network.h"

void UpdateRClient(int rfd)			// update robot client
{
	struct timeval untime;
	gettimeofday(&untime, NULL);
	rInfo[rfd].cntime = untime.tv_sec;
}

void *RClientManage(void *arg)			// robot client manage
{
	struct timeval ntime;
	struct timespec sleeptm;
	int i,count=0;
	sleeptm.tv_sec = 0;
	sleeptm.tv_nsec = NANOSECOND;
	while(1)
	{
//		sem_wait(&sem_rcm);     // robot manage signal
		gettimeofday(&ntime, NULL);
		for(i=0; i<MAXEPOLLSIZE; i++)
		{
			if(rInfo[i].tfd & rInfo[i].tflag)						// old client ->client and through  authentication
			{
				if((ntime.tv_sec - rInfo[i].cntime) > ROBOT_HEART_TIME)		// old client timeout set
				{
					if(rInfo[i].cflag == -1)
						lprintf(lfd, INFO, "Robot: [%s] active closed!",rInfo[i].id);
					else if(rInfo[i].cflag == -2)
						lprintf(lfd, INFO, "Robot: [%s] last client closed!",rInfo[i].id);
					else
						lprintf(lfd, INFO, "Robot: [%s] timeout closed!",rInfo[i].id);
				 	epoll_ctl(kdpfd, EPOLL_CTL_DEL, rInfo[i].tfd, &ev);
				 	close(rInfo[i].tfd);
					if(rInfo[i].cflag != -2)
						hInfo[rInfo[i].hiv].rhiv = 0;
					rInfo[i].tfd = 0;
					rInfo[i].tflag = 0;
					rInfo[i].cflag = 0;
					rInfo[i].hiv = 0;
//					DelHashTablePos(cInfo[i].id, RidHash,MHI);			// delete hash value
                    memset(rInfo[i].ip,0x00,16);
					memset(rInfo[i].id,0x00,17);
					atomic_dec(R_CON_NUM);
				}
			}
			else if(rInfo[i].tfd | rInfo[i].tflag)					// new client
			{
				if((ntime.tv_sec - rInfo[i].cntime) > ROBOT_DEAD_TIME)		//	new client timeout setting
				{
					lprintf(lfd, INFO, "Robot: External client [%s] close!",rInfo[i].ip);
					epoll_ctl(kdpfd, EPOLL_CTL_DEL, rInfo[i].tfd, &ev);
					close(rInfo[i].tfd);
					rInfo[i].tfd = 0;
					rInfo[i].tflag = 0;
					memset(rInfo[i].ip,0x00,16);
				}
			}
		}
//		sem_post(&sem_rtcp);
		nanosleep(&sleeptm,NULL);
		count++;
		if(count == 32766)
		{
			r_printf("Robot connect number: %d\n",atomic_read(R_CON_NUM));
			count = 0;
		}
	}
}


void ResetRClient(int rfd)
{
	struct timeval untime;
	gettimeofday(&untime, NULL);
	if (rInfo[rfd].tflag != 0)
		rInfo[rfd].cntime = (untime.tv_sec - ROBOT_HEART_TIME);	// Reduce the timeout
	else
		rInfo[rfd].cntime = (untime.tv_sec - ROBOT_DEAD_TIME);	// Reduce the timeout
	rInfo[rfd].cflag = -1;
}
