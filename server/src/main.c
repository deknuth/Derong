/*
 * main.c
 *
 *  Created on: 2010-3-27
 *      Author: Lovenix
 */
#include "../inc/main.h"
char LogFile[64] = {0};
log_t *lfd;
int TotalRec,sflag,CacheRec,Cache1Rec;

sem_t sem_rcm;
sem_t sem_rtcp;

tpool_t *pool;
struct HashItem* RidHash;
PRINFO 	rInfo;			// robot info struct
HIV hInfo;

atomic_t *R_CON_NUM;
atomic_t *M_CON_NUM;
int main(void)
{
	int i,j;
	struct rlimit rt;
	pthread_t id1,id2,id3;
	R_CON_NUM = malloc(sizeof(atomic_t));
	M_CON_NUM = malloc(sizeof(atomic_t));
	atomic_set(R_CON_NUM,0);
	atomic_set(M_CON_NUM,0);

	rInfo = (PRINFO)malloc(sizeof(RINFO)*MAX_REC);
	memset(rInfo,0x00,sizeof(rInfo));

	rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE; //
	if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
		printf("Kernel: Set limit failed!\n");
//	sleep(120);					//
	signal(SIGINT, stop);		// ctrl+C
//	signal(SIGTERM, stop);		// kill
//	signal(SIGHUP, stop);		// ssh close
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);
	setbuf(stdout,NULL);

	CryptTables();				// 初始化hash
	RidHash = inithashtable(MHI);

	sem_init(&sem_rcm, 0, 0);			// TCP client manager signal
	sem_init(&sem_rtcp, 0, 1);			// TCP

	pool = tpool_init(512, 512, 1); 					//
	sflag = Cache1Rec = CacheRec = TotalRec = 0;		// sflag=0

	CreatLogName();

	if((lfd = log_open(LogFile, 0)) == 0)	//
	{
		lprintf(lfd, FATAL, "Kernel: Open log file failed!");
		exit(0);
	}
	for(i = 0; i < MAX_REC; i++)
	{
		rInfo[i].cntime = 0;
		rInfo[i].tfd = 0;
		rInfo[i].tflag = 0;
		rInfo[i].cflag = 0;
		rInfo[i].hiv = 0;
		rInfo[i].uFlag = 0;

 		memset(rInfo[i].id,0x00,17);
 		memset(rInfo[i].alias,0x00,17);
 		memset(rInfo[i].mPhone,0x00,12);
 		memset(rInfo[i].ip,0x00,16);
 		memset(rInfo[i].lip,0x00,4);
	}

	for(i=0; i<MHI; i++)		// Init index hash value
	{
		hInfo[i].rhiv = 0;
		hInfo[i].mcun = 0;
		hInfo[i].pValid = 0;
		hInfo[i].wfd = 0;
	}

//	UpdateRid();			// update RID
	sleep(1);
	pthread_create(&id1, NULL, RobotServer, (void *)ROBOT_PORT);
	sleep(1);
	pthread_create(&id2, NULL, RClientManage, (void *)0);
	sleep(1);
	pthread_create(&id3, NULL, HttpServer, (void *) 0);

	pthread_join(id1, (void **) 0);
	pthread_join(id2, (void **) 0);
	pthread_join(id3, (void **) 0);

	sem_destroy(&sem_rtcp);
	sem_destroy(&sem_rcm);
	tpool_destroy(pool, 1);
	log_close(lfd);
	FreeHash(RidHash);
	free(rInfo);
	free(R_CON_NUM);
	free(M_CON_NUM);
	return 0;
}

