#ifndef ORACLE_H_
#define ORACLE_H_

/***** oracle *****/
#define MAXTHREAD 10
#define NOT_BIND 0
#define ALREADY_BIND 1
#define CHECK 0
#define	INSERT	1
#define MAX_COL_LEN	1024
extern sem_t sem_or;

typedef struct getdata{
	int tag;
	int num_column;
	int num_row;
	char** buf;
	char** buftemp;
	int* buf_len;
}OCI_DATA;		// OCI fetch

typedef struct trm_config{
	char title[24];
	char url[48];
	char insert_time[48];
	char contexts[48];
	char stats[16];
	char ip[24];
	struct ter_config *next;
}CON_DATA;		//

typedef struct data
{
	char buf[MAX_COL_LEN];
	struct data *next;
}CHECK_DATA;		//

typedef struct st_oci
{
	OCIEnv *envhp;
	OCIError *errhp;
	OCIServer *srvhp;
	OCISvcCtx *svchp;
	OCISession *authp;
	OCIStmt *stmthp;
	OCIBind *bindhp;
	OCIParam *colhp;
	OCIDefine **defhp;
}OCI_ENV;		//

extern OCI_ENV *oracle_env;


extern int OciIE(OCI_ENV *env);
extern int OciExcu(OCI_ENV *env,char *sql,int flag);
extern int OciCheck(char *query, CHECK_DATA *head);
extern int FreeData(OCI_DATA *data);
extern int OciEnd(OCI_ENV *env);
extern int FreeRS(CHECK_DATA * head);
extern int CountCheck(OCI_ENV *env, char *query, char* result);
extern int OciInsert(char *query);
extern int ExecSQL(CHECK_DATA *head,char *sql);
extern int CheckOneItem(char *buf,char *sql,int len);		//  单条数据
extern int CheckOneRow(char *buf,char *sql,int len);
#endif

