#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <sys/types.h>          /* See NOTES */
#include    <sys/socket.h>
#include    <errno.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    "cjson.h"
char InputBuffer[4096] = {0};

int DecodeAndProcessData(char *input); 	//具体译码和处理数据，该函数代码略

int control(char* context)
{
    int socket_fd;
    char temp[64] = {0};
    struct sockaddr_in server_addr;
    char rBuf[128] = {0};
    if((socket_fd = socket(AF_INET,SOCK_STREAM,0)) < 0 )
    {
        printf("create socket error: %s(errno:%d)\n)",strerror(errno),errno);
        return 0;
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4444);

    if(inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr) <=0)
    {
        printf("inet_pton error!\n");
        return 0;
    }
    if(connect(socket_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
       // printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    if(send(socket_fd,context,strlen(context),0 ) < 0)
    {
        printf("send message error\n");
        return 0;
    }
    while(1)
    {
        if(recv(socket_fd, rBuf, sizeof(rBuf), 0) <= 0)
            break;
    }
    sprintf(temp,"{'isSuccess':%d}",rBuf[0]-48);
    printf("%s\n",temp);
    close(socket_fd);
    return 1;
}

int parseJson(char * pMsg)
{
    char msg[128] = {0};
    if(NULL == pMsg)
        return 0;
    cJSON * pJson = cJSON_Parse(pMsg);
    if(NULL == pJson)
    {
        printf("Invalid json format!\n");
        return 0;
    }
    
    cJSON * pSub = cJSON_GetObjectItem(pJson, "type");	// get string from json
    if(NULL == pSub)
        return 0;		
    else
    	strncat(msg,pSub->valuestring,strlen(pSub->valuestring));
    
    pSub = cJSON_GetObjectItem(pJson, "nid");
    if(NULL == pSub)
        return 0;
    else   
    	strncat(msg,pSub->valuestring,strlen(pSub->valuestring));
    
    pSub = cJSON_GetObjectItem(pJson, "gid");
    if(NULL == pSub)
        return 0;	
    else     
    	strncat(msg,pSub->valuestring,strlen(pSub->valuestring));
    
    /*
    cJSON * pSubSub = cJSON_GetObjectItem(pSub, "subjsonobj");
    if(NULL == pSubSub)
    {
        // get object from subject object faild
    }
    printf("sub_obj_1 : %s\n", pSubSub->valuestring);
*/

	if(strlen(msg) == 25)
    	control(msg);
    else
    	printf("Invalid json data!\n");
    cJSON_Delete(pJson);
}

// 创建数组，数组值是另一个JSON的item
char* makeArray(int iSize)
{
    int i = 0;
    cJSON * root =  cJSON_CreateArray();
    if(NULL == root)
    {
        printf("create json array faild\n");
        return NULL;
    }
    for(i = 0; i < iSize; i++)
    {
        cJSON_AddNumberToObject(root, "hehe", i);
    }
    char * out = cJSON_Print(root);
    cJSON_Delete(root);
    return out;
}

char *makeJson()
{
    cJSON * pJsonRoot = NULL;
    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot)
        return NULL;
    cJSON_AddStringToObject(pJsonRoot, "gatewayId", "123456");
    cJSON_AddStringToObject(pJsonRoot, "time", "2017年2月11日09: 53: 37");
    cJSON_AddStringToObject(pJsonRoot, "ver", "1.0");
    cJSON_AddStringToObject(pJsonRoot, "error", "1");
    cJSON_AddStringToObject(pJsonRoot, "msg", "请求超时");
    //   cJSON_AddBoolToObject(pJsonRoot, "bool", 1);

    /*
    cJSON * pSubJson = NULL;
    pSubJson = cJSON_CreateObject();
    if(NULL == pSubJson)
    {
        // create object faild, exit
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    cJSON_AddStringToObject(pSubJson, "subjsonobj", "a sub json string");
    cJSON_AddItemToObject(pJsonRoot, "subobj", pSubJson);
*/
    char * p = cJSON_Print(pJsonRoot);
    // else use :
    // char * p = cJSON_PrintUnformatted(pJsonRoot);
    if(NULL == p)
    {
        //convert json list to string faild, exit
        //because sub json pSubJson han been add to pJsonRoot, so just delete pJsonRoot, if you also delete pSubJson, it will coredump, and error is : double free
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    //free(p);

    cJSON_Delete(pJsonRoot);
    return p;
}


int main(int argc, char*argv[])
{
    int ContentLength;	// 数据长度
    int i,x;
    char *p;
    char *pRequestMethod; 	// METHOD属性值
    setvbuf(stdin, NULL, _IONBF, 0); // 关闭stdin的缓冲
    //   printf("Content-type:text/html\n\n"); // 从stdout中输出，告诉Web服务器返回的信息类型
    printf("Content-type:application/json\n\n");
    //    printf("\n"); // 插入一个空行，结束头部信息
    // 从环境变量REQUEST_METHOD中得到METHOD属性值
    pRequestMethod = getenv("REQUEST_METHOD");
    if (pRequestMethod == NULL)
    {
        printf("<p>request = null</p>");
        return 0;
    }
    if (strcasecmp(pRequestMethod, "POST") == 0)
    {
        //      printf("<p>OK the method is POST!\n</p>");
        p = getenv("CONTENT_LENGTH");     //从环境变量CONTENT_LENGTH中得到数据长度
        if (p != NULL)
            ContentLength = atoi(p);
        else
            ContentLength = 0;
        if (ContentLength > sizeof(InputBuffer) - 1)
            ContentLength = sizeof(InputBuffer) - 1;
        i = 0;
        while (i < ContentLength)
        {                         //从stdin中得到Form数据
            x = fgetc(stdin);
            if (x == EOF)
                break;
            InputBuffer[i++] = x;
        }
        InputBuffer[i] = '\0';
        ContentLength = i;
  //      printf("%s\n", InputBuffer);
        parseJson(InputBuffer);
        //	DecodeAndProcessData(InputBuffer);                 //具体译码和处理数据，该函数代码略
    }
    else if (strcasecmp(pRequestMethod, "GET") == 0)
    {
        //     printf("<p>OK the method is GET!\n</p>");
        p = getenv("QUERY_STRING");     //从环境变量QUERY_STRING中得到Form数据
        if (p != NULL)
        {
            strncpy(InputBuffer, p, sizeof(InputBuffer));
            DecodeAndProcessData(InputBuffer);    //具体译码和处理数据，该函数代码略
        }
    }
    //   printf("<HEAD><TITLE>Submitted OK</TITLE></HEAD>\n");    //从stdout中输出返回信息
    //   printf("<BODY>The information you supplied has been accepted.</BODY>\n");
    return 0;
}

int DecodeAndProcessData(char *input)    //具体译码和处理数据
{
    // 补充具体操作
    return 0;
}
/*
{
"gateId":"1095",
"deviceId":"A116"
}

{
	"type":	"1",
	"nid" : "FFFFFFFF",
	"gid":	"0123456789abcdef"
}
*/