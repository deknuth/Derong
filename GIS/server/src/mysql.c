/*
 * mysql.c
 *
 *  Created on: 2017-3-24
 *      Author: Administrator
 */
#include "../inc/main.h"
#include "../inc/mysql.h"

MYSQL* DbClient(void)
{
	MYSQL* env = (MYSQL *)malloc(sizeof(MYSQL));
	mysql_init(env);
	if (!mysql_real_connect(env,DB_IP, DB_USER, DB_PSW, DB_EXP ,0,NULL,0))
	{
		lprintf(lfd, FATAL, "DB: %s",mysql_error(env));
		return 0;
	}
	else
	{
		lprintf(lfd, INFO, "DB: Database init success!");
		return env;
	}
}

int DbInsert(MYSQL* env,char* sql)
{
	if(mysql_real_query(env,sql,strlen(sql)))
		return 0;
	else
	{
		if(mysql_affected_rows(env))
			return 1;
		else
			return 0;
	}
}

int DbClose(MYSQL *env)
{
	mysql_close(env);
	free(env);
	return 1;
}

/*
int main(void)
{
    MYSQL mysql;    //mysql连接
    MYSQL_RES *res; //这个结构代表返回行的一个查询结果集
    MYSQL_ROW row; //一个行数据的类型安全(type-safe)的表示
    char *query;  //查询语句
    int t,r;
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql,"127.0.0.1", "root", "rhYxjWtcc682dPEy", "tgjhw",0,NULL,0))
        printf( "Error connecting to database: %s",mysql_error(&mysql));
    else
        printf("Connected...\n");
    query=" select * from pre_saas_thing";
    t=mysql_real_query(&mysql,query,(unsigned int)strlen(query));//执行指定为计数字符串的SQL查询。
    if(t)
        printf("执行显示时出现异常: %s",mysql_error(&mysql));
    res = mysql_store_result(&mysql);//检索完整的结果集至客户端。
    while(row = mysql_fetch_row(res))
    {
        for(t=0; t<mysql_num_fields(res); t++)
        {
            printf("%s\t" ,row[t]);
        }
        printf("\n");
    }
    mysql_free_result(res);//释放结果集使用的内存。
    mysql_close(&mysql);
    return 0;
}
*/
