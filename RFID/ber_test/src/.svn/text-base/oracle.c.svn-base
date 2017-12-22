#include "../inc/main.h"
#include "../inc/oracle.h"
static OraText *poolName;
static sb4 poolNameLen;			// poll名长度
static text *database = (text *)"(DESCRIPTION =(ADDRESS = (PROTOCOL = TCP)(HOST = 127.0.0.1)(PORT = 1521))(CONNECT_DATA =(SERVER = DEDICATED)(SERVICE_NAME = lovenix)))";
static text *username =(text *)"rongle";
static text *password =(text *)"lovenix1202";
static text *PoolUser =(text *)"rongle";
static text *PoolPasswd =(text *)"lovenix1202";
static ub4 conMin = 2;
static ub4 conMax = 10;
static ub4 conIncr = 1;
static OCIError   *errhp;		// 错误句柄
static OCIEnv     *envhp;		// 环境句柄
static OCICPool   *poolhp;		// 连接池句柄
sem_t sem_or;

static int CheckERR(OCIError *errhp, sword status,char* context);

int OciIE(OCI_ENV *env)		// OCI event initialization
{
	OCISvcCtx *svchp;
	OCIStmt *stmthp;

	if(OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0, (dvoid * (*)(dvoid *, size_t))0, (dvoid * (*)(dvoid *, dvoid *, size_t))0, (void (*)(dvoid *, dvoid *)) 0))
	{
		lprintf(lfd, FATAL, "OCI: OCI_INITIALIZE_FALSE!");
		return 0;
	}

	if(OCIEnvInit((OCIEnv **) &envhp, OCI_DEFAULT, (size_t) 0, (dvoid **) 0))
	{
		lprintf(lfd, FATAL, "OCI: OCI_OCIENVINIT_FALSE!");
		return 0;
	}

	/***** OCIHandleAlloc() 分配错误句柄 *****/
	(void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &errhp,OCI_HTYPE_ERROR,(size_t) 0, (dvoid **) 0);

	/***** OCIHandleAlloc() ,poolhp,分配连接池服务句柄 *****/
	(void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &poolhp,OCI_HTYPE_CPOOL,(size_t) 0, (dvoid **) 0);

	/************ OCIConnectionPoolCreate() 创建连接池 *************/
	if(CheckERR (errhp, OCIConnectionPoolCreate(envhp,
                   errhp,poolhp, &poolName, &poolNameLen,
                   database,strlen((char *)database),
                   conMin, conMax, conIncr,
                   PoolUser,strlen((char *)PoolUser),
                   PoolPasswd,strlen((char *)PoolPasswd),OCI_DEFAULT),"OCIConnectionPoolCreate"))
		return 0;

	/********************OCILogon2() 登陆数据库 ***********************/

	if(OCILogon2(envhp, errhp, &svchp, (CONST OraText *)username, strlen((char *)username),(CONST OraText *)password, strlen((char *)password),(CONST OraText *)poolName, poolNameLen,OCI_CPOOL))
	{
		OCIHandleFree((dvoid *) svchp, OCI_HTYPE_SVCCTX);
		OCIHandleFree((dvoid *) errhp, OCI_HTYPE_ERROR);
		OCIHandleFree((dvoid *) envhp, OCI_HTYPE_ENV);
		lprintf(lfd, FATAL, "OCI: OCI_OCILOGON2_FALSE!");
		return 0;
	}

	if(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&stmthp, OCI_HTYPE_STMT,(size_t)NULL, (dvoid **)NULL))
	{
		OCILogoff(svchp, errhp);
		OCIHandleFree((dvoid *) svchp, OCI_HTYPE_SVCCTX);
		OCIHandleFree((dvoid *) errhp, OCI_HTYPE_ERROR);
		OCIHandleFree((dvoid *) envhp, OCI_HTYPE_ENV);
		lprintf(lfd, FATAL, "OCI: OCI_OCIHANLDALLOC_FALSE!");
		return 0;
	}

	env->envhp = envhp;
	env->errhp = errhp;
	env->svchp = svchp;
	env->stmthp = stmthp;
	env->defhp = NULL;
	env->bindhp = NULL;
	return 1;
}

int OciInsert(char *query)
{
	sem_wait(&sem_or);
	if(OciExcu(oracle_env,query,1))
	{
		sem_post(&sem_or);
		return 1;
	}
	sem_post(&sem_or);
	return 0;
}

int OciExcu(OCI_ENV *env,char *sql,int flag)
{
	/******** OCIStmtPrepare() 解析SQL语句 **********/
	if(CheckERR(errhp,OCIStmtPrepare(env->stmthp, env->errhp, (text *)sql, (ub4)strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT),"OCIStmtPrepare"))
		return 0;
	/******** OCIStmtExecute() 执行SQL语句 **********/
	if(CheckERR(errhp,OCIStmtExecute(env->svchp, env->stmthp, env->errhp, (ub4) 1, (ub4) 0,(CONST OCISnapshot *) NULL,(OCISnapshot *) NULL, OCI_DEFAULT | OCI_COMMIT_ON_SUCCESS),"OCIStmtExecute"))
		return 0;
	/******** OCITransCommit() 进行事务的提交,不提交回滚 **********/
	if(flag)
	{
		if(OCITransCommit(env->svchp,env->errhp,(ub4)0))
			return 0;
	}
	return 1;
}

int CountCheck(OCI_ENV *env, char *query, char* result)
{
	int num_col=0, i;
	int status = 0;
	char** buf_col;
	int* len_col;
	OCIParam* colhp;
	OCIDefine** defhp;
	OCI_DATA *data = (OCI_DATA *)malloc(sizeof(OCI_DATA));
	memset(data, 0x00, sizeof(OCI_DATA));

	if (env == NULL || (text *)query == NULL || data == NULL)
	{
		FreeData(data);
		return 0;
	}

	status = OCIStmtPrepare(env->stmthp, env->errhp, (text *)query, (ub4)strlen(query), OCI_NTV_SYNTAX, OCI_DEFAULT);
	if(CheckERR(errhp,status,"OCIStmtPrepare"))
	{
		FreeData(data);
		return 0;
	}
	status = OCIStmtExecute(env->svchp, env->stmthp, env->errhp, (ub4) 0, (ub4) 0,(CONST OCISnapshot *) NULL,(OCISnapshot *) NULL, OCI_DESCRIBE_ONLY);
	if(CheckERR(errhp,status,"OCIStmtExecute"))
	{
		FreeData(data);
		return 0;
	}
	if (OCIAttrGet(env->stmthp, OCI_HTYPE_STMT, &num_col, 0,OCI_ATTR_PARAM_COUNT, env->errhp))
	{
		FreeData(data);
		return 0;
	}
	if (num_col == 0)		//
	{
		FreeData(data);
		return 0;
	}
	len_col = (int *) malloc(sizeof(int) * num_col);
	buf_col = (char **) malloc(sizeof(char *) * num_col);
	defhp = (OCIDefine **) malloc(sizeof(OCIDefine *) * num_col);
	data->num_column = num_col;		//
	data->num_row = 0;			//
	data->buf = buf_col;
	data->buf_len = len_col;
	memset(len_col, 0, sizeof(int) * num_col);
	memset(buf_col, 0, sizeof(char *) * num_col);
	for (i = 1; i <= num_col; i++)
	{
		OCIParamGet(env->stmthp, OCI_HTYPE_STMT, env->errhp,(void **) &colhp, i);
		OCIAttrGet(colhp, OCI_DTYPE_PARAM, (len_col + i - 1), 0,OCI_ATTR_DATA_SIZE, env->errhp);
		*(len_col + i - 1) = *(len_col + i - 1) + 1;
		*(buf_col + i - 1) = (char *) malloc((int) (*(len_col + i - 1)));
		if (*(buf_col + i - 1) == NULL)
		{
			FreeData(data);
			return 0;
		}
		memset(*(buf_col + i - 1), 0, (int) (*(len_col + i - 1)));
		if (OCIDefineByPos(env->stmthp, (defhp + i - 1), env->errhp, i,(ub1 *) (*(buf_col + i - 1)), *(len_col + i - 1), SQLT_STR,NULL, (dvoid *) 0, (ub2 *) 0, OCI_DEFAULT))
		{
			FreeData(data);
			return 0;
		}
	}
	status = OCIStmtExecute(env->svchp, env->stmthp, env->errhp, (ub4) 0, (ub4) 0,(CONST OCISnapshot *) NULL,(OCISnapshot *) NULL, OCI_DEFAULT);
	if(CheckERR(errhp,status,"OCIStmtExecute"))
	{
		FreeData(data);
		return 0;
	}

	OCIStmtFetch(env->stmthp, env->errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT);
	if(strlen(data->buf[0]) == 0)
	{
		FreeData(data);
		return 2;
	}
	else if(strlen(data->buf[0]) < 16)
	{
		sprintf(result,"%s", data->buf[0]);
		FreeData(data);
		return 1;
	}
	else
	{
		FreeData(data);
		return 0;
	}
}

int OciCheck(char *query, CHECK_DATA *head)
{
	CHECK_DATA *DataLink;
	int num_col, count, i,len;
	int flag = 0;				// 1 -> Single row  2 -> Multiple row
	char** buf_col;
	int* len_col;
	int status = 0;
	OCIParam* colhp;
	OCIDefine** defhp;

	if ((text *)query == NULL)
		return -1;
	num_col = 0;
	status = OCIStmtPrepare(oracle_env->stmthp, oracle_env->errhp, (text *)query, (ub4) strlen(query),OCI_NTV_SYNTAX, OCI_DEFAULT);
	if(CheckERR(errhp,status,"OCIStmtPrepare"))
		return -1;
	status = OCIStmtExecute(oracle_env->svchp, oracle_env->stmthp, oracle_env->errhp, (ub4) 0, (ub4) 0,(CONST OCISnapshot *) NULL,(OCISnapshot *) NULL, OCI_DESCRIBE_ONLY);
	if(CheckERR(errhp,status,"OCIStmtExecute"))
		return -1;
	if (OCIAttrGet(oracle_env->stmthp, OCI_HTYPE_STMT, &num_col, 0,OCI_ATTR_PARAM_COUNT, oracle_env->errhp))
		return -1;

//	OCIAttrGet((dvoid*)param, OCI_DTYPE_PARAM,(dvoid*)&dtype, (ub4*)0, OCI_ATTR_DATA_TYPE, errHandle); // get select type

	if (num_col == 0)		// 查询无内容
		return 0;
	len_col = (int *) malloc(sizeof(int) * num_col);
	buf_col = (char **) malloc(sizeof(char *) * num_col);
	defhp = (OCIDefine **) malloc(sizeof(OCIDefine *) * num_col);

	OCI_DATA *data = (OCI_DATA *)malloc(sizeof(OCI_DATA));
	memset(data, 0x00, sizeof(OCI_DATA));

	data->num_column = num_col;		// 列
	data->num_row = 0;			// 行
	data->buf = buf_col;
	data->buf_len = len_col;
	memset(len_col, 0, sizeof(int) * num_col);
	memset(buf_col, 0, sizeof(char *) * num_col);
	for (i = 1; i <= num_col; i++)
	{
		OCIParamGet(oracle_env->stmthp, OCI_HTYPE_STMT, oracle_env->errhp,(void **) &colhp, i);
		OCIAttrGet(colhp, OCI_DTYPE_PARAM, (len_col + i - 1), 0,OCI_ATTR_DATA_SIZE, oracle_env->errhp);
		*(len_col + i - 1) = *(len_col + i - 1) + 1;
		*(buf_col + i - 1) = (char *) malloc((int) (*(len_col + i - 1)));
		if (*(buf_col + i - 1) == NULL)
		{
			FreeData(data);
			return 0;
		}
		memset(*(buf_col + i - 1), 0, (int) (*(len_col + i - 1)));

		if (OCIDefineByPos(oracle_env->stmthp, (defhp + i - 1), oracle_env->errhp, i,(ub1 *) (*(buf_col + i - 1)), *(len_col + i - 1), SQLT_STR,NULL, (dvoid *) 0, (ub2 *) 0, OCI_DEFAULT))
		{
			FreeData(data);
			return -1;
		}
	}
	status = OCIStmtExecute(oracle_env->svchp, oracle_env->stmthp, oracle_env->errhp, (ub4)0, (ub4)0,(CONST OCISnapshot *) NULL,(OCISnapshot *) NULL, OCI_DEFAULT);
	if(CheckERR(errhp,status,"OCIStmtExecute"))
	{
		FreeData(data);
		return -1;
	}

	for (i = 0; i < data->num_column; i++)
		memset(data->buf[i], 0, data->buf_len[i]);

	while (OCIStmtFetch(oracle_env->stmthp, oracle_env->errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT) != OCI_NO_DATA)
	{
		count = data->num_column;
		DataLink = (CHECK_DATA *) malloc(sizeof(CHECK_DATA));
		len = 0;
		if(data->num_column > 1)
		{
			while (count--)
			{
				sprintf(&((DataLink->buf)[len]),"%s%c", data->buf[count],0x01);
				len += strlen(data->buf[count])+1;
			}
			flag = 2;
		}
		else
		{
			sprintf(DataLink->buf, "%s", data->buf[0]);
			flag = 1;
		}
		DataLink->next = NULL;
		DataLink->next = head->next;
		head->next = DataLink;
		data->num_row++;
	}

	if ((data->num_row) > 0)
	{
		FreeData(data);
		return flag;
	}
	else
	{
		FreeData(data);
		data->tag = NOT_BIND;
		return 0;
	}
}

int FreeData(OCI_DATA *data)
{
	int i;
	if(data == NULL)
		return 1;
	if(data->buf_len)
		free(data->buf_len);
	if(data->buf)
	{
		for(i=0; i<data->num_column; i++)
		{
			if(*(data->buf+i))
				free(*(data->buf+i));
		}
		free(data->buf);
	}
	return 1;
}

int FreeRS(CHECK_DATA *head)		// free result set
{
	if (head == NULL)
		return 0;
	CHECK_DATA  *ptr = head;
	CHECK_DATA  *curr;
	while (ptr->next != NULL)
	{
		curr = ptr;
		ptr = ptr->next;
		free(curr);
	}
	free(ptr);
	return 1;
}

int OciEnd(OCI_ENV *env)
{
	OCILogoff((dvoid *) env->svchp, env->errhp);
	OCIConnectionPoolDestroy(poolhp, errhp, OCI_DEFAULT);	// OCIConnectionPoolDestroy()销毁联接池
	OCIHandleFree((dvoid *) env->stmthp, OCI_HTYPE_STMT);
	OCIHandleFree((dvoid *) env->envhp, OCI_HTYPE_ENV);
	OCIHandleFree((dvoid *)poolhp, OCI_HTYPE_CPOOL);		// OCIHandleFree() 释放连接池句柄
	OCIHandleFree((dvoid *)errhp, OCI_HTYPE_ERROR);			// OCIHandleFree() 释放错误代码句柄
	OCIHandleFree((dvoid *) env->svchp, OCI_HTYPE_SVCCTX);
	return 1;
}

int CheckERR(OCIError *errhp, sword status,char* context)				// 出错检测函数
{
	text errbuf[512] = {0};
	sb4 errcode = 0;
	int flag = 0;
	switch (status)
	{
		case OCI_SUCCESS:
			break;
		case OCI_SUCCESS_WITH_INFO:
			lprintf(lfd, INFO, "OCI(%s): OCI_SUCCESS_WITH_INFO!",context);
			break;
		case OCI_NEED_DATA:
			lprintf(lfd, FATAL, "OCI(%s): OCI_NEED_DATA!",context);
			flag = 1;
			break;
		case OCI_NO_DATA:
			lprintf(lfd, FATAL, "OCI(%s): OCI_NODATA!",context);
			flag = 1;
			break;
		case OCI_ERROR:
			(void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
			lprintf(lfd, FATAL, "OCI(%s): %s",context,errbuf);
			flag = 1;
			break;
		case OCI_INVALID_HANDLE:
			lprintf(lfd, FATAL, "OCI(%s): OCI_INVALID_HANDLE!",context);
			flag = 1;
			break;
		case OCI_STILL_EXECUTING:
			lprintf(lfd, INFO, "OCI(%s): OCI_STILL_EXECUTE!",context);
			break;
		case OCI_CONTINUE:
			lprintf(lfd, INFO, "OCI(%s): OCI_CONTINUE!",context);
			break;
		default:
			break;
	}
	return flag;
}

int ExecSQL(CHECK_DATA *head,char *sql)
{
	int rs = 0;
	memset(head, 0x00, sizeof(CHECK_DATA));
	sem_wait(&sem_or);
	rs = OciCheck(sql, head);
	sem_post(&sem_or);
	if(rs == -1)
		lprintf(lfd, FATAL, "SQL CHECK Failed： %s", sql);
	return rs;
}

int CheckOneItem(char *buf,char *sql,int len)
{
	int rs = 0;
	CHECK_DATA *head = (CHECK_DATA *)malloc(sizeof(CHECK_DATA));
	memset(head , 0x00, sizeof(CHECK_DATA));
	sem_wait(&sem_or);
	rs = OciCheck(sql,head);
	sem_post(&sem_or);
	if(rs == 1)		// 一列一个数据
	{
		if(head != NULL)
		{
			if(head->next != NULL)
			{
				head = head->next;
				if(strlen(head->buf) == len)
					memcpy(buf,head->buf,len);
				else
					rs = 0;
			}
			else
				rs = 0;
		}
		else
			rs = 0;
	}
	else if(rs == -1)
	{
		rs = 0;
		lprintf(lfd, FATAL, "OCI: Execution [%s] failed!",sql);
	}
	else
		rs = 0;
	FreeRS(head);
	return rs;
}

int CheckOneRow(char *buf,char *sql,int len)
{
	int rs = 0;
	CHECK_DATA *head = (CHECK_DATA *)malloc(sizeof(CHECK_DATA));
	memset(head,0x00,sizeof(CHECK_DATA));
	sem_wait(&sem_or);
	rs = OciCheck(sql,head);
	sem_post(&sem_or);
	if(rs > 0)		// 一行
	{
		if(head != NULL)
		{
			if(head->next != NULL)
			{
				head = head->next;
				if(strlen(head->buf) <= len)
					memcpy(buf,head->buf,len);
				else
					rs = 0;
			}
			else
				rs = 0;
		}
		else
			rs = 0;
	}
	else if(rs == -1)
	{
		rs = 0;
		lprintf(lfd, FATAL, "OCI: Execution [%s] failed!",sql);
	}
	else
		rs = 0;
	FreeRS(head);
	return rs;
}

