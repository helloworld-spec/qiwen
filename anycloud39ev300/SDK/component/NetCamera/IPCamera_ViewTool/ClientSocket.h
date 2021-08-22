#pragma once

class CClientSocket
{
public:
	CClientSocket(void);
	~CClientSocket(void);

public:
	SOCKADDR_IN m_sockaddr;
	SOCKADDR_IN m_rsockaddr;


	BOOL Socket_Create( UINT idex );  
	BOOL Socket_Close(  UINT idex  );//关闭Socket对象； 
	int Socket_Connect(char *lIPAddress, unsigned int iPort , UINT idex );//定义连接函数； 
	int Socket_Send( char* strData, int iLen , UINT idex);//数据发送函数；
	int Socket_Receive( char* strData, int iLen , UINT idex);//数据接收函数；

};
