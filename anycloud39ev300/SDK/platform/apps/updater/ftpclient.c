/**
 * @filename ftpclient.c
 * @brief for ftp download
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author zhangshenglin
 * @date 2012-12-07
 * @version 1.0
 * @ref 
 */
/*FtpGetRun*/  
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <fcntl.h>  
#include <unistd.h>  
#include <stdarg.h> 
#include <stdio.h>  
#include <netdb.h>  
#include "md5.h"

/*FtpGetRun Variable*/    
FILE *pFtpIOFile = NULL;  
FILE *pFileCmdChmod;        //使用popen方式修改文件属性为可执行的文件指针  
FILE *pRunGetFile;  //使用popen方式执行文件的文件指针  
char aFtpBuffer[4096];  //数据buffer
/*Http Variable*/  
FILE *pFileCmdChmod;  
FILE *pRunGetFile; 
char aRequestHead[1000]; //request buffer 
char aResponseHead[1000]; //response buffer
//int iGetRunMark;//用来标记是get模式，还是getrun模式1为get模式，2为getrun模式     
char acChmodCmd[50];//用来使用chmode命令行     
char acRunCmdLine[50];//用来运行程序     

extern int bNeedCheckFile;//是否需要进行md5校验
static int FtpCmd(int iSockFtpCmd,char *cFmt,...)    
{     
	va_list vVaStartUse;     
	int iFtpCmdReturn;    
	int iFtpLength;      
	
	if (pFtpIOFile == NULL)     
	{     
		pFtpIOFile = fdopen(iSockFtpCmd,"r");    
		if (pFtpIOFile == NULL)    
		{    
			printf("The ERROR of pointer of pFtpIOFile");    
			return -1;    
		}     
	}      
	if (cFmt)     
	{     
		va_start(vVaStartUse,cFmt);     
		iFtpLength = vsprintf(aFtpBuffer,cFmt,vVaStartUse);     
		aFtpBuffer[iFtpLength++] = '\r';     
		aFtpBuffer[iFtpLength++]='\n';     
		write(iSockFtpCmd,aFtpBuffer,iFtpLength); //如同send     
	}      
	do     
	{     
		if (fgets(aFtpBuffer,sizeof(aFtpBuffer),pFtpIOFile) == NULL)     
		{    
			return -1;    
		}      
	} while(aFtpBuffer[3] == '-');    

	sscanf(aFtpBuffer,"%d",&iFtpCmdReturn);      
	return iFtpCmdReturn;    
}     

/*****************************************************************
 *@brief:download http file
 *@author:zhangshenglin
 *@date:2013-3-07
 *@param :host:the  ftp url
 					port:file name
 					user:ftp server username
 					pass:ftp server password
 					filename:file name in ftp server
 					pcSaveFile:file name in local
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
 
int FtpGet(char *host,unsigned short port, char *user,char *pass,char *filename,char *pcSaveFile)    
{     
	int iSockFtpCmd = -1;//用来socket接受调用后返回的套接口描述符号     
	int iSockFtpData = -1;//datasocket建立后返回的套接口描述符号     
	int iSockAccept = -1;     
	struct sockaddr_in addr;//定义socket结构   
	socklen_t socklen;   
	unsigned long hostip;//存放主机地址的变量      
	int iFtpLength;     
	int iFtpCmdReturn;     
	int retval = -1;     
	int iOpenReturn; //接收open函数的返回值     
	unsigned char *c;//用来指向data连接时候的主机地址     
	unsigned char *p;//用来指向data连接时候的端口     
	hostip = inet_addr(host); //转换主机地址为网络排序模式     
	if (hostip == -1)     
	{     
		printf("\nHostIP is ERROR!!\n");    
	}     
  
	//建立socket     
	//设定相应的socket协议和地址     
	/**********************************************************/    
	iSockFtpCmd = socket(AF_INET,SOCK_STREAM,0);      

	if (iSockFtpCmd == -1)
	{
		retval = -2;		
		goto out;  
	}	

	addr.sin_family = PF_INET;     
	addr.sin_port = htons(port);     
	addr.sin_addr.s_addr = hostip;      

	/**********************************************************/     
	/*connect*/    
	if (connect(iSockFtpCmd,(struct sockaddr *)&addr,sizeof(addr)) == -1)     
	{
		retval = -3;
		goto out;   
	}	
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,NULL);     
	if (iFtpCmdReturn != 220)     
	{
		retval = -4;
		goto out;
	}	
 
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"USER %s",user);     
	if (iFtpCmdReturn != 331)     
	{
		retval = -5;
		goto out;
	}	
     
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"PASS %s",pass);     
	if (iFtpCmdReturn != 230)     
	{
		retval = -6;
		goto out;   
	}	
     
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"TYPE I");     
	if (iFtpCmdReturn != 200)     
	{
		retval = -7;
		goto out;   
	}	
      
	/*建立data socket*/    
	iSockFtpData = socket(AF_INET,SOCK_STREAM,0);     
      
	if (iSockFtpData == -1)     
	{
		retval = -8;
		goto out;  
	}	   
    
	getsockname(iSockFtpCmd,(struct sockaddr *)&addr,&socklen);     
	addr.sin_port = 0;     
	   
	/*绑定*/    
	if (bind(iSockFtpData,(struct sockaddr *)&addr,sizeof(addr)) == -1)     
	{
		retval = -9;
		goto out;   
	}	
  //监听
	if (listen(iSockFtpData,1) == -1)     
	{
		retval = -10;
		goto out;    
	}	
        
	getsockname(iSockFtpData,(struct sockaddr *)&addr,&socklen);     
	c = (unsigned char *)&addr.sin_addr;     
	p = (unsigned char *)&addr.sin_port;     
    
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"PORT %d,%d,%d,%d,%d,%d", c[0],c[1],c[2],c[3],p[0],p[1]);     
 
	if (iFtpCmdReturn != 200)     
	{
		retval = -11;
		goto out;
	}		
     
	iFtpCmdReturn = FtpCmd(iSockFtpCmd,"RETR %s",filename);     
	if (iFtpCmdReturn != 150)     
	{
		retval = -12;
		goto out;   
	}	
         
	iSockAccept = accept(iSockFtpData,(struct sockaddr *)&addr,&socklen);     
      
	if (iSockAccept == -1)     
	{
		retval = -13;
		goto out;    
	}	
	//创建一个本地文件
	iOpenReturn = open(pcSaveFile,O_WRONLY|O_CREAT,0644);     
	if (iOpenReturn == -1)     
	{
		retval = -14;
		goto out;    
	}	
	        
	retval = 0;    
	
	//循环请求数据然后写入文件
	while ((iFtpLength=read(iSockAccept,aFtpBuffer,sizeof(aFtpBuffer)))>0)     
	{     
		write(iOpenReturn,aFtpBuffer,iFtpLength);     
		retval += iFtpLength;     
	};     
    
	close(iOpenReturn);    
 
   	//md5 check
	if(bNeedCheckFile != 0)
		if(CheckFileMd5(pcSaveFile) < 0)
		{
			retval = -1;
		}	
out:     
	close(iSockAccept);     
	close(iSockFtpData);     
	close(iSockFtpCmd);     
	if (pFtpIOFile)     
	{     
		fclose(pFtpIOFile);     
		pFtpIOFile = NULL;     
	}      
	return retval;    
}