/**
 * @filename httpclient.c
 * @brief for http download
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author zhangshenglin
 * @date 2012-12-07
 * @version 1.0
 * @ref 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "md5.h"

extern int bNeedCheckFile;

#define HOST_ADDR_LEN 256
#define HOST_FILE_LEN 1024

//////////////////////////////httpclient.c 开始///////////////////////////////////////////

/********************************************
功能：搜索字符串右边起的第一个匹配字符
********************************************/
static char * Rstrchr(char * s, char x)
{
	int i = strlen(s);
  if(!(*s))  
  	return 0;
  while(s[i-1]) 
  	if(strchr(s + (i - 1), x))  
  		return (s + (i - 1));  
  	else i--;
  return 0;
}

/********************************************
功能：把字符串转换为全小写
********************************************/
static void ToLowerCase(char * s)
{
	while(*s)
	{
		printf("%c\n", *s);
		*s=tolower(*s);
		s++;
	}
}

/**************************************************************
功能：从字符串src中分析出网站地址和端口，并得到用户要下载的文件
***************************************************************/
static void GetHost(char * src, char * web, int web_len, char * file, 
					int file_len, int * port)  {
  char * pA;
  char * pB;
  
  memset(web, 0, web_len);
  memset(file, 0, file_len);
  
  *port = 0;
  if(!(*src))  
  	return;
  pA = src;
  
  if(!strncmp(pA, "http://", strlen("http://")))  
  	pA = src+strlen("http://");
  else if(!strncmp(pA, "https://", strlen("https://")))  
  	pA = src+strlen("https://");
  pB = strchr(pA, '/');
  
  if(pB)
  {
    memcpy(web, pA, strlen(pA) - strlen(pB));
    if(pB+1)
    {
      memcpy(file, pB + 1, strlen(pB) - 1);
      file[strlen(pB) - 1] = 0;
    }
  }
  else  
  	memcpy(web, pA, strlen(pA));
  
  if(pB)  
  	web[strlen(pA) - strlen(pB)] = 0;
  else  
  	web[strlen(pA)] = 0;
  pA = strchr(web, ':');
  
  if(pA)  
  	*port = atoi(pA + 1);
  else 
  	*port = 80;
}

/*****************************************************************
 *@brief:download http file
 *@author:zhangshenglin
 *@date:2013-3-07
 *@param :url:the http url
 					filenane:file name
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int HttpDownload(char* url, char* filename)
{
	int sockfd;
	char buffer[1024];
	int portnumber,nbytes;
	char host_addr[HOST_ADDR_LEN];
	char host_file[HOST_FILE_LEN];
	char local_file[256];
	char local_file_tmp[256];
	FILE * fp;
	char request[1024];
	int send, totalsend;
	int i;
	char * pt;

	printf("parameter.1 is: %s\n", url);
	ToLowerCase(url);/*将参数转换为全小写*/
	printf("lowercase parameter.1 is: %s\n", url);
	/* parse url, get host, portnumber, host file name and so on */
	GetHost(url, host_addr, HOST_ADDR_LEN, host_file, HOST_FILE_LEN, 
			&portnumber);/*分析网址、端口、文件名等*/
	printf("webhost:%s\n", host_addr);
	printf("hostfile:%s\n", host_file);
	printf("portnumber:%d\n\n", portnumber);

	int rv = -1;
    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo(host_addr, NULL, &hints, &res)) != 0) {
        printf("getaddrinfo error for %s: %s\n", host_addr, gai_strerror(rv));
        return -1;
    }

	struct addrinfo *ressave = res;
    do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(sockfd < 0) {
		    continue;
		}       

		if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
		    break;
		}

		close(sockfd);
    }while((res = res->ai_next) != NULL);

    if(res == NULL) {
        printf("tcp connect error for %s: %s\n", host_addr, gai_strerror(rv));
        return -1;
    }
	freeaddrinfo(ressave);
	
	sprintf(request, "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\n\
	User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n\
	Host: %s:%d\r\nConnection: Close\r\n\r\n", host_file, host_addr, portnumber);
	printf("%s", request);/*准备request，将要发送给主机*/

	/*取得真实的文件名*/
	if(/*host_file && */*host_file)  
		  pt = Rstrchr(host_file, '/');
	else 
		  pt = 0;

	memset(local_file, 0, sizeof(local_file));
	if(pt && *pt)
	{
		if((pt + 1) && *(pt+1))  
			strcpy(local_file, pt + 1);
		else  
			memcpy(local_file, host_file, strlen(host_file) - 1);
	}
	else if(/*host_file && */*host_file)  
	{ 
		strcpy(local_file, host_file);
	} 
	else  
		strcpy(local_file, "index.html");
	  
		sprintf(local_file_tmp, "%s.tmp", local_file);
	printf("local filename to write:%s\n\n", local_file_tmp);
	unlink(local_file_tmp);

	/*发送http请求request*/
	send = 0;totalsend = 0;
	nbytes=strlen(request);
	while(totalsend < nbytes)
	{
		send = write(sockfd, request + totalsend, nbytes - totalsend);
		if(send==-1)  
		{
			printf("send error!%s\n", strerror(errno));
			return -1;
			//exit(0);
		}
		totalsend+=send;
		printf("%d bytes send OK!\n", totalsend);
	}

	fp = fopen(local_file_tmp, "a");
	if(!fp)
	{
		printf("create file error! %s\n", strerror(errno));
		return -1;
	}
	printf("\nThe following is the response header:\n");
	i=0;
	/* 连接成功了，接收http响应，response */
	while((nbytes=read(sockfd,buffer,1))==1)
	{
		if(i < 4)
		{
			if(buffer[0] == '\r' || buffer[0] == '\n')
				i++;
			else 
				i = 0;
			printf("%c", buffer[0]);/*把http头信息打印在屏幕上*/
		}
		else
		{
			fwrite(buffer, 1, 1, fp);/*将http主体信息写入文件*/
			i++;
			if(i%1024 == 0)
			{
				printf("download %d bytes\n", i);
				fflush(fp);/*每1K时存盘一次*/
			}
		}
	}
	fclose(fp);
	/* 结束通讯 */
	close(sockfd);
	if(bNeedCheckFile != 0)
	{
		if(CheckFileMd5(local_file_tmp) == 0)
		{
			char cmd[256];
			sprintf(cmd, "mv %s %s", local_file_tmp, local_file);
			system(cmd);
			strcpy(filename, local_file);
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return 0;
	}
}
