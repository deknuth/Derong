#include "../inc/main.h"
#include "../inc/robot.h"

char date[15] = {0};

int RobotInit(int rfd,unsigned char* rBuf)
{
	int rb,index;
	char sn[17] = {0};
    HexToString(sn,&rBuf[3],8);

//	if(CodeValid(plainBlock,cipherBlock))		// Matching ID
	{
//		index = GetHashTablePos(sn,RidHash,MHI);
//		if(index == 0)
		{
//			snprintf(sql,sizeof(sql),"insert into t_robot_info (n_mac,n_prefix,n_id,n_push_alias,n_arm_ver,n_reg_time) values ('%s',Floor((select count(n_mac) from t_robot_info)/40),seq_robot.nextVal,'%s','%d',to_char(sysdate,'yyyy-mm-dd hh24:mi:ss'))",sn,alias,rBuf[3]);
//			s_printf("robot register SQL: %s\n",sql);

//			if(OciInsert(sql))
			{
                rb = 0x101;		// success
//				index = InsertHash(sn,RidHash,MHI);
//				rInfo[rfd].hiv = index; 				// insert hash table
//				hInfo[index].rhiv = rfd;				// storage rfd hash
				rInfo[rfd].tflag = rfd;					// set socket online flag
				lprintf(lfd, INFO, "Robot: ID [%s] initialization success!",sn);
                memcpy(rInfo[rfd].id,sn,16);
                printf("reg sn: %s\n",sn);
				atomic_inc(R_CON_NUM);
			}
//			else
//			{
//				rb = 0x106;		// database problem
//				lprintf(lfd, FATAL, "Robot: initialization failed, insert abnormal!");
//			}
        }
//		else				// already register
//			rb = 0x109;		// already initialization
    }
//	else	// ID encryption error
//	{
//		rb = 0x108;		//ID error
//		lprintf(lfd, WARN, "Robot: Receive a abnormal initialization [%s] from robot client!",sn);
//	}

//	BackPacket(rb,pack,1,14,0);
//	_write(rfd,pack,14);				// robot initialization back packet
/*
	if(rb == 0x107)
	{
		UpdateRid();			// update ID
	}
 */
	return rb;
}

const unsigned char tab[16]={ 48,49,50,51,52,53,54,55,56,57,48,48,48,48,48,48 };
const unsigned char tab_time[36]={
		0x8,0x9,0x10,0x11,0x12,0x13,0x14,0x15,
		0x16,0x17,0x0,0x0,0x0,0x0,0x0,0x0,
		0x18,0x19,0x20,0x21,0x22,0x23,0x00,0x01,
		0x2,0x3,0x0,0x0,0x0,0x0,0x0,0x0,
		0x4,0x5,0x6,0x7
};
int GisInfo(unsigned char *buf,int rfd)
{
    int index=0,i=0,j;
    LOVENIX gps_info;
    memset(&gps_info,0x00,sizeof(gps_info));
    unsigned char lat[5] = {0};
    unsigned char lon[5] = {0};
    unsigned char date[6] = {0};
    unsigned char temp[12] = {0};
    unsigned char speed[2] = {0};
    int dir = 0;
    float min = 0;

    memcpy(lat,&buf[3],5);
    memcpy(lon,&buf[8],5);
    memcpy(date,&buf[13],6);
    memcpy(speed,&buf[19],2);
    i = 0;
    temp[i++] = tab[(lon[1])>>4];
    temp[i++] = tab[(lon[1])&0xf];
    temp[i++] = '.';
    for(j=2; j<5; j++)
    {
        temp[i++] = tab[(lon[j])>>4];
        temp[i++] = tab[(lon[j])&0xf];
    }
    min=atod(temp,9);
    min /= 60.0;
    min += lon[0];
    sprintf(gps_info.lon,"%.8f",min);

    i = 0;
    memset(temp,0x00,12);
    temp[i++] = tab[(lat[1])>>4];
    temp[i++] = tab[(lat[1])&0xf];
    temp[i++] = '.';
    for(j=2; j<5; j++)
    {
        temp[i++] = tab[(lat[j])>>4];
        temp[i++] = tab[(lat[j])&0xf];
    }
    min=atod(temp,9);
    min /= 60.0;
    min += lat[0];
    sprintf(gps_info.lat,"%.8f",min);
    gps_info.speed[0] = (speed[0]>>4)+48;
    gps_info.speed[1] = (speed[0]&0xf)+48;
    gps_info.speed[2] = (speed[1]>>4)+48;
    gps_info.speed[3] = '.';
    gps_info.speed[4] = (speed[1]&0xf)+48;
 //   printf("speed: %s\n",gps_info.speed);
    dir = (buf[21]>>4)*100 + (buf[21]&0xf)*10 + (buf[22]>>4);
    sprintf(gps_info.dir,"%d",dir);
//    printf("dir: %s\n",gps_info.dir);
    struct tm *p;
    time_t timep;
    time(&timep);
    p = localtime(&timep);
    p->tm_year = ((date[5])>>4)*10 + ((date[5])&0xf) + 100;		// year
    p->tm_mon = ((date[4])>>4)*10 + ((date[4])&0xf);
    p->tm_mday = ((date[3])>>4)*10 + ((date[3])&0xf);
    p->tm_hour = ((date[0])>>4)*10 + ((date[0])&0xf);
    p->tm_min = ((date[1])>>4)*10 + ((date[1])&0xf);
    p->tm_sec = ((date[2])>>4)*10 + ((date[2])&0xf);
    timep = mktime(p)+28800;
    p = localtime(&timep);
    strftime(gps_info.date, 100, "%Y%m%d", p);
    strftime(gps_info.time, 100, "%H%M%S", p);
    memcpy(gps_info.id,rInfo[rfd].id,16);
    GisCommit(gps_info);
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





