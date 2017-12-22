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
int p2p_sfd;
sem_t sem_pms;		// mobile control signal
sem_t sem_rcm;
sem_t sem_rtcp;
sem_t sem_udp;
sem_t sem_ud;
sem_t sem_mtcp;
sem_t sem_mcm;		// mobile client manager signal
sem_t sem_wtcp;
sem_t sem_wcm;		// watch client manager signal
sem_t sem_uf;		// update file
sem_t sem_pcm;		// photo client manager signal
sem_t sem_ptcp;
sem_t sem_p2p;		// p2p control signal


tpool_t *pool;
OCI_ENV  *oracle_env;
struct HashItem* RidHash;
struct HashItem* WidHash;
struct HashItem* SmsHash;
struct HashItem* P2PHash;
//struct HashItem* UDPHash;

PINFO 	pInfo;
PRINFO 	rInfo;			// robot info struct
//ROBOT rInfo;			// robot info struct
MINFO	mInfo;			// mobile info struct
WINFO	wInfo;			// watch info struct
VECODE	sInfo;			// sms info struct
OT_INFO *otInfo;		// other info
HIV hInfo;
WHIV whInfo;
P2P* uInfo;
UDPI* ucInfo;
volatile unsigned char test;
atomic_t *R_CON_NUM;
atomic_t *M_CON_NUM;
int main(void)
{
	int i,j;
	struct rlimit rt;
	pthread_t id[13] = {0};
	R_CON_NUM = malloc(sizeof(atomic_t));
	M_CON_NUM = malloc(sizeof(atomic_t));
	atomic_set(R_CON_NUM,0);
	atomic_set(M_CON_NUM,0);
	rInfo = (PRINFO)malloc(sizeof(RINFO)*MAX_REC);
	uInfo = (P2P*)malloc(sizeof(P2P)*MAX_REC);
	ucInfo = (UDPI*)malloc(sizeof(UDPI)*MAX_REC);
	otInfo = (OT_INFO*)malloc(sizeof(OT_INFO)*MHI);
	memset(otInfo,0x00,sizeof(otInfo));
	memset(uInfo,0x00,sizeof(uInfo));
	memset(rInfo,0x00,sizeof(rInfo));
	memset(ucInfo,0x00,sizeof(ucInfo));
	memset(whInfo,0x00,sizeof(whInfo));

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

	if(FileCheck(CONFIG_FILE) == 0)		// 检测配置文件
	{
		if(!DefaultConfigSet())
		{
			printf("Create configure file error!\n");
			exit(0);
		}
	}

	CryptTables();				// 初始化hash
	RidHash = inithashtable(MHI);
	WidHash = inithashtable(MHI);
	P2PHash = inithashtable(MAX_REC);
	SmsHash = inithashtable(SHI);
//	UDPHash = inithashtable(MAX_REC);

	srand((unsigned) time(NULL));

//	buffer = (unsigned char *)malloc(sizeof(unsigned char));

	/***openssl curl **********/
	curl_global_init(CURL_GLOBAL_ALL);	// In windows, this will init the winsock stuff
	init_locks();
	/**************************/

	sem_init(&sem_rcm, 0, 0);			// TCP client manager signal
	sem_init(&sem_mcm, 0, 0);			// mobile TCP client manager signal
	sem_init(&sem_wcm, 0, 0);
	sem_init(&sem_pcm, 0, 0);

	sem_init(&sem_p2p, 0, 1);
	sem_init(&sem_rtcp, 0, 1);			// TCP
	sem_init(&sem_mtcp, 0, 1);			// mobile TCP signal
	sem_init(&sem_wtcp, 0, 1);
	sem_init(&sem_ptcp, 0, 1);

	sem_init(&sem_udp, 0, 0);			// UDP
//	sem_init(&sem_tp, 0, 1);			// TCP
	sem_init(&sem_ud, 0, 1);			//
	sem_init(&sem_or, 0, 1);			//
	sem_init(&sem_uf, 0, 1);
	sem_init(&sem_pms, 0, 1);
	pool = tpool_init(512, 512, 1); 					//
	sflag = Cache1Rec = CacheRec = TotalRec = 0;		// sflag=0

	oracle_env = (OCI_ENV *)malloc(sizeof(OCI_ENV));
	memset(oracle_env, 0x00, sizeof(OCI_ENV));

	CreatLogName();				//
//	if(!FileCheck(CONFIG_FILE))	//
//		DefaultConfigSet();
	
	if((lfd = log_open(LogFile, 0)) == 0)	//
	{
		lprintf(lfd, FATAL, "Kernel: Open log file failed!");
		exit(0);
	}

	if(!OciIE(oracle_env))	//
	{
		lprintf(lfd, FATAL, "Kernel: Connect oracle failed!");
		return 0;
	}
	memset(wInfo,0x00,sizeof(wInfo));
	for(i = 0; i < MAX_REC; i++)
	{
		rInfo[i].cntime = 0;
		rInfo[i].tfd = 0;
		rInfo[i].tflag = 0;
		rInfo[i].cflag = 0;
		rInfo[i].hiv = 0;
		rInfo[i].uFlag = 0;

 		mInfo[i].cntime = 0;
 		mInfo[i].mfd = 0;
 		mInfo[i].tflag = 0;
 		mInfo[i].cflag = 0;
 		mInfo[i].hiv = 0;
 		mInfo[i].tmpFile.ffd = 0;
 		mInfo[i].tmpFile.wo = 0;

 		pInfo[i].cflag = 0;
 		pInfo[i].pfd = 0;
 		pInfo[i].tflag = 0;
 		pInfo[i].dsf = 0;
 		pInfo[i].count = 0;
 		pInfo[i].index = 0;
 		pInfo[i].mfd = 0;
 		pInfo[i].wFlag = 0;

 		memset(rInfo[i].id,0x00,17);
 		memset(rInfo[i].alias,0x00,17);
 		memset(rInfo[i].mPhone,0x00,12);
 		memset(rInfo[i].ip,0x00,16);
 		memset(rInfo[i].lip,0x00,4);

 		memset(mInfo[i].aKey,0x00,8);
 		memset(mInfo[i].id,0x00,17);
 		memset(mInfo[i].ip,0x00,16);
 		memset(mInfo[i].user,0x00,17);
 		memset(mInfo[i].tmpFile.path,0x00,128);

 		memset(wInfo[i].rid,0x00,17);
 		memset(wInfo[i].id,0x00,17);
 		memset(wInfo[i].ip,0x00,17);
 		wInfo[i].ef_flag = 0;
	}

	for(i = 0; i < MAX_REC; i++)
	{
		ucInfo[i].ufd = 0;
		ucInfo[i].cfm = ucInfo[i].last_cfm = 0;
		ucInfo[i].p2p_index = 0;
		ucInfo[i].count = 0;
	}
	for(i=0; i<MHI; i++)		// Init index hash value
	{
		hInfo[i].rhiv = 0;
		hInfo[i].mcun = 0;
		hInfo[i].pValid = 0;
		hInfo[i].wfd = 0;
		for(j=0; j<MUN; j++)
			hInfo[i].mhiv[j] = 0;
	}

	test = 1;			// 测试
	unsigned char buf[12] = {0};
	buf[0] = 0xFE;
	buf[2] = 0x0C;
	buf[11] = 0xFF;
	p2p_sfd = ClientP2P(P2P_SERVER_IP);

	UpdateRid();			// update RID
	UpdateWid();			// update WID
	pthread_create(&id[0], NULL, CoreUp, (void *) 0);
	sleep(6);
	pthread_create(&id[1], NULL, RobotServer, (void *)ROBOT_PORT); 		//
	pthread_create(&id[2], NULL, UdpServer, (void *)KER_UDP_PORT); 		// UDP
	sleep(1);
	pthread_create(&id[3], NULL, PhotoServer, (void *) PHOTO_PORT); 			//
	sleep(1);
	pthread_create(&id[4], NULL, MobServer, (void *) MOBILE_PORT);		// mobile client thread
	sleep(1);
	pthread_create(&id[5], NULL, WatchServer, (void *)WATCH_PORT);
	sleep(1);
	pthread_create(&id[6], NULL, RClientManage, (void *)0);				// TCP
	pthread_create(&id[7], NULL, PClientManage, (void *)0);
	pthread_create(&id[8], NULL, MClientManage, (void *) 0);
	pthread_create(&id[9], NULL, WClientManage, (void *)0);
	pthread_create(&id[10], NULL, LogManage, (void *)0);
	pthread_create(&id[11], NULL, EFServer, (void *)0);
	pthread_create(&id[12], NULL, UClientManage, (void *)0);

	for(i=0; i<13; i++)
		pthread_join(id[i], (void **) 0);

	sem_destroy(&sem_udp);
	sem_destroy(&sem_ud);
	sem_destroy(&sem_or);
	sem_destroy(&sem_uf);

	sem_destroy(&sem_mtcp);
	sem_destroy(&sem_rtcp);
	sem_destroy(&sem_wtcp);
	sem_destroy(&sem_ptcp);

	sem_destroy(&sem_rcm);
	sem_destroy(&sem_mcm);
	sem_destroy(&sem_wcm);
	sem_destroy(&sem_pcm);
	sem_destroy(&sem_pms);
	sem_destroy(&sem_p2p);

	curl_global_cleanup();
	kill_locks();

	tpool_destroy(pool, 1);
	OciEnd(oracle_env);
	free(oracle_env);
	free(uInfo);
	log_close(lfd);
	close(p2p_sfd);
//	free(buffer);
	FreeHash(SmsHash);
	FreeHash(RidHash);
	FreeHash(WidHash);
	FreeHash(P2PHash);
//	FreeHash(UDPHash);
	free(rInfo);
	free(R_CON_NUM);
	free(M_CON_NUM);
	free(otInfo);
	return 0;
}

