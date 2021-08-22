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
	BOOL Heat_Socket_Create(UINT idex) ;  
	BOOL Heat_Socket_Close(UINT idex); //关闭Socket对象； 
	int Heat_Socket_Connect(char *lIPAddress, unsigned int iPort, UINT idex); //定义连接函数；
	int Heat_Socket_Send( char* strData, int iLen ,UINT idex); //数据发送函数； 
	int Heat_Socket_Receive( char* strData, int iLen , UINT idex); //数据接收函数； 
	BOOL Rev_Socket_Create(UINT idex);  
	BOOL Rev_Socket_Close(UINT idex);//关闭Socket对象； 
	int Rev_Socket_Connect(char *lIPAddress, unsigned int iPort, UINT idex);
	int Rev_Socket_Send( char* strData, int iLen ,UINT idex);//数据发送函数； 
	int Rev_Socket_Receive( char* strData, int iLen , UINT idex);//数据接收函数；

	int Socket_Receive_update_finish( char* strData, int iLen , UINT idex);//数据接收函数； 
	BOOL Socket_Close_update_finish(UINT idex);//关闭Socket对象； 
	BOOL Socket_Create_update_finish(UINT idex); 
	int Socket_Connect_update_finish(char *lIPAddress, unsigned int iPort, UINT idex);



	int Socket_server_Receive( char* strData, int iLen );//数据接收函数； 
	int Socket_server_Send( char* strData, int iLen );//数据发送函数； 
	int Socket_server_setsockopt( void);//数据接收函数； 
	int Socket_server_Listen( int iQueuedConnections );//监听函数； 
	int Socket_server_asyncSelect(HWND hWnd, unsigned int wMsg, long lEvent) ;
	int Socket_server_Accept();//建立连接函数，S为监听Socket对象名；
	int Socket_server_Bind(char* strIP, unsigned int iPort);//绑定函数；
	int Socket_server_Connect(char *lIPAddress, unsigned int iPort);//定义连接函数； 
	BOOL Socket_server_Close(void);//关闭Socket对象； 
	BOOL Socket_server_Create(void);

};
