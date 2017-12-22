#include "../inc/main.h"
#include "../inc/robot.h"

int UpdateRid(void)			// update robot id
{
	int j,k,rs,index=0,pos;
	char id[17] = {0};
	char prefix[4] = {0};
	char *p = NULL;
	char *sql = "select n_arm_ver,n_prefix,n_mac from t_robot_info";
	const char pattern[2] = { 0x01 };
	CHECK_DATA *head = (CHECK_DATA *)malloc(sizeof(CHECK_DATA));
	memset(head , 0x00, sizeof(CHECK_DATA));
	sem_wait(&sem_or);
	rs = OciCheck(sql,head);
	sem_post(&sem_or);
	if(rs != -1)
	{
		if(head != NULL)
		{
			if(rs > 0)		// Multiple row
			{
				while (head->next != NULL)
				{
STAR:
					j = k = 0;
					memset(id,0x00,17);
					memset(prefix,0x00,4);
					head = head->next;
					p = head->buf;
					pos = kmp(p,pattern);
					if(pos==16)
					{
						memcpy(id,p,pos);
						p += pos+1;
						index = InsertHash(id, RidHash,MHI);
					}
					else if(pos == -1)
						goto STAR;
					pos = kmp(p,pattern);
					if(pos == -1)
						goto STAR;
					else if(pos<4)
					{
						memcpy(prefix,p,pos);
						p += pos+1;
					}
					rs = atoi(p);
					if(index != 0)
						otInfo[index].version = (rs>7)?0:rs;
//					r_printf("sn [%s] ver: %d\n",id,otInfo[index].version);
//					lprintf(lfd, INFO, "Robot: ID [%s] update success!",id);
				}
				lprintf(lfd, INFO, "Robot: ID update success!");
			}
		}
	}
	else
		lprintf(lfd, FATAL, "Robot: OCI check and robot id update failed!");
	FreeRS(head);
	return 1;
}

int DESCrc(unsigned char* plainBlock,unsigned char* cipherBlock,int len)
{
	int count=0;
	while(len--)
		count += (*(plainBlock++))^(*(cipherBlock++));
	if(count)
		return 0;
	else
		return 1;
}

int CodeValid(unsigned char* plainBlock,unsigned char* cipherBlock)
{
	unsigned char encBlock[8] = {0};
	DES_Decrypt(cipherBlock,(unsigned char *)KEY,encBlock);
	if(DESCrc(plainBlock,encBlock,8))
		return 1;
	else
		return 0;
}
