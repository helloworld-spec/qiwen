#include "StdAfx.h"
#include "ClientSocket.h"
#include "Config_test.h"


typedef struct TCP_KEEPALIVE_INFO 
{

	u_long onoff;

	u_long keepalivetime;

	u_long keepaliveinterval;

} TCP_KEEPALIVE_INFO; 


int g_hSocket_hanlde[UPDATE_MAX_NUM] = {INVALID_SOCKET};
int g_hSocket_heat[UPDATE_MAX_NUM] = {INVALID_SOCKET};
int g_hSocket_rev[UPDATE_MAX_NUM] = {INVALID_SOCKET};
int g_hSocket_update_finish_hanlde[UPDATE_MAX_NUM] = {INVALID_SOCKET};

int g_hSocket_server = INVALID_SOCKET;
int g_hSocket_server_new = INVALID_SOCKET;

typedef UINT   SlTime_t;
typedef UINT   SlSuseconds_t;

typedef struct SlTimeval_t
{
	SlTime_t          tv_sec;             /* Seconds      */
	SlSuseconds_t     tv_usec;            /* Microseconds */
}SlTimeval_t;


CClientSocket::CClientSocket(void)
{
	WSADATA wsaD;
	int result;
	UINT i = 0;
	WORD wVersionRequested;


	// m_LastError是类内字符串变量,初始化用来存放最后错误说明的字符串；  	
	// 初始化类内sockaddr_in结构变量，前者存放客户端地址，后者对应于服务器端地址; 

	memset(&m_sockaddr, 0, sizeof(m_sockaddr));  
	memset(&m_rsockaddr, 0, sizeof(m_rsockaddr)); 

	wVersionRequested = MAKEWORD( 2, 2 );


	result = WSAStartup(0x0202, &wsaD); //初始化WinSocket动态连接库
	if(result != 0) // 初始化失败；  	
	{ 
		return; 
	} 
}

CClientSocket::~CClientSocket(void)
{
	WSACleanup( );
}

BOOL CClientSocket::Socket_Create(UINT idex)  
{
	//int status;
	//unsigned long cmd = 0;

	// m_hSocket是类内Socket对象，创建一个基于TCP/IP的Socket变量，并将值赋给该变量； 
	if(g_hSocket_hanlde[idex] != INVALID_SOCKET)
	{
		closesocket(g_hSocket_hanlde[idex]);
		g_hSocket_hanlde[idex] = INVALID_SOCKET;
	}

	g_hSocket_hanlde[idex] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_hSocket_hanlde[idex] == INVALID_SOCKET) 
	{ 
		return FALSE; 
	}

	unsigned long ul = 1;
	int result;
	result = ioctlsocket(g_hSocket_hanlde[idex], FIONBIO, &ul);
	if (result == SOCKET_ERROR) 
	{
		return FALSE;
	}

	return TRUE;  
}

BOOL CClientSocket::Socket_Close(UINT idex)//关闭Socket对象； 
{  
	if (g_hSocket_hanlde[idex] != INVALID_SOCKET)
	{
		if (closesocket(g_hSocket_hanlde[idex]) == SOCKET_ERROR) 	
		{  
			return FALSE; 
		} 
	}
	g_hSocket_hanlde[idex] = INVALID_SOCKET;
	//file://重置sockaddr_in 结构变量；  
	memset(&m_sockaddr, 0, sizeof(sockaddr_in));  
	memset(&m_rsockaddr, 0, sizeof(sockaddr_in)); 

	return TRUE; 
}  

/////////////////////////////////////////  
int CClientSocket::Socket_Connect(char *lIPAddress, unsigned int iPort, UINT idex)//定义连接函数； 
{
	int result;
	unsigned long ul = 1;

	m_sockaddr.sin_addr.s_addr = inet_addr(lIPAddress); 
	m_sockaddr.sin_family = AF_INET; 
	m_sockaddr.sin_port = htons( iPort );  
#if 0
	//connect非阻塞方式
	ul = 1;
	result = ioctlsocket(g_hSocket_hanlde, FIONBIO, &ul);
	if (result == SOCKET_ERROR) 
	{
		return FALSE;
	}

#endif
	result = connect(g_hSocket_hanlde[idex], (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr));
	if (1)//result < 0) 
	{
		DWORD ret = GetLastError();
		ret = ret;
		//m_bDisConnect = TRUE;
	}

#if 1
	INT try_times = 0;
	do 
	{
		struct timeval timeout;
		fd_set fd_con, fd_con_w;

		FD_ZERO(&fd_con);
		FD_ZERO(&fd_con_w);
		FD_SET(g_hSocket_hanlde[idex], &fd_con);
		FD_SET(g_hSocket_hanlde[idex], &fd_con_w);
		timeout.tv_sec = 1; //2s 连接超时
		timeout.tv_usec = 100*1000;
		result = select(g_hSocket_hanlde[idex] + 1, &fd_con_w, &fd_con, 0, &timeout);
		try_times ++;
		//	Sleep(100);
	} while(0); // ((result == 0) && (try_times < 10));
	if (result <= 0) 
	{
		DWORD ret = WSAGetLastError();
		return FALSE;
	}
	else
	{
		DWORD ret = WSAGetLastError();
		ret = FALSE;

	}

	//m_stSockInfo.recv_fd = m_stSockInfo.send_fd;

#if 1
	//设回阻塞方式
	ul = 0;
	if (ioctlsocket(g_hSocket_hanlde[idex], FIONBIO, &ul) < 0)
	{
		return FALSE;	
	}
#endif
#endif

	return TRUE; 
}  

////////////////////////////////////////////////////  
int CClientSocket::Socket_Send( char* strData, int iLen,  UINT idex )//数据发送函数； 
{  
	if(strData == NULL || iLen == 0) 
	{	
		return FALSE;  
	}

	if( send(g_hSocket_hanlde[idex], strData, iLen, 0) == SOCKET_ERROR ) 
	{  
		return FALSE; 
	}  

	return TRUE; 
}  

/////////////////////////////////////////////////////  
int CClientSocket::Socket_Receive( char* strData, int iLen, UINT idex )//数据接收函数； 
{  
	int len = 0; 
	int ret = 0; 
	CString str;

	if(strData == NULL) 
	{
		return -1; 
	}

	ret = recv(g_hSocket_hanlde[idex], strData, iLen, 0); 
	return ret; 
} 



/////////////////////////////////////////////////////////////////////////////
//如下是创建心跳的socket
BOOL CClientSocket::Heat_Socket_Create(UINT idex)  
{
	//int status;
	//unsigned long cmd = 0;

	// m_hSocket是类内Socket对象，创建一个基于TCP/IP的Socket变量，并将值赋给该变量； 
	if(g_hSocket_heat[idex] != INVALID_SOCKET)
	{
		closesocket(g_hSocket_heat[idex]);
		g_hSocket_heat[idex] = INVALID_SOCKET;
	}

	if ((g_hSocket_heat[idex] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) 
	{ 
		return FALSE; 
	} 

	return TRUE;  
}

BOOL CClientSocket::Heat_Socket_Close(UINT idex)//关闭Socket对象； 
{ 
	if (g_hSocket_heat[idex] != INVALID_SOCKET)
	{
		if (closesocket(g_hSocket_heat[idex]) == SOCKET_ERROR) 	
		{  
			return FALSE; 
		} 
	}
	g_hSocket_heat[idex] = INVALID_SOCKET;
	//file://重置sockaddr_in 结构变量；  
	//memset(&m_sockaddr, 0, sizeof(sockaddr_in));  
	//memset(&m_rsockaddr, 0, sizeof(sockaddr_in)); 

	return TRUE; 
}  

/////////////////////////////////////////   


int CClientSocket::Heat_Socket_Connect(char *lIPAddress, unsigned int iPort, UINT idex)//定义连接函数； 
{
	m_sockaddr.sin_addr.s_addr = inet_addr(lIPAddress); 
	m_sockaddr.sin_family = AF_INET; 
	m_sockaddr.sin_port = htons( iPort );  

	if(connect(g_hSocket_heat[idex], (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr)) == SOCKET_ERROR ) 	
	{  
		return FALSE; 

	}  

	return TRUE; 
}  


////////////////////////////////////////////////////  
int CClientSocket::Heat_Socket_Send( char* strData, int iLen ,UINT idex)//数据发送函数； 
{  
	if(strData == NULL || iLen == 0) 
	{	
		return FALSE;  
	}

	if( send(g_hSocket_heat[idex], strData, iLen, 0) == SOCKET_ERROR ) 
	{  
		return FALSE; 
	}  

	return TRUE; 
}  

/////////////////////////////////////////////////////  
int CClientSocket::Heat_Socket_Receive( char* strData, int iLen , UINT idex)//数据接收函数； 
{  
	int len = 0; 
	int ret = 0;  

	if(strData == NULL) 
	{
		return -1; 
	}

	ret = recv(g_hSocket_heat[idex], strData, iLen, 0); 
	return ret; 
}  


//接收数据的socket
/////////////////////////////////////////////////////////////////////////////
BOOL CClientSocket::Rev_Socket_Create(UINT idex)  
{
	//int status;
	//unsigned long cmd = 0;

	// m_hSocket是类内Socket对象，创建一个基于TCP/IP的Socket变量，并将值赋给该变量； 
	if(g_hSocket_rev[idex] != INVALID_SOCKET)
	{
		closesocket(g_hSocket_rev[idex]);
		g_hSocket_rev[idex] = INVALID_SOCKET;
	}

	if ((g_hSocket_rev[idex] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) 
	{ 
		return FALSE; 
	} 

	return TRUE;  
}

BOOL CClientSocket::Rev_Socket_Close(UINT idex)//关闭Socket对象； 
{ 
	if (g_hSocket_rev[idex] != INVALID_SOCKET)
	{
		if (closesocket(g_hSocket_rev[idex]) == SOCKET_ERROR) 	
		{  
			return FALSE; 
		} 
	}
	g_hSocket_rev[idex] = INVALID_SOCKET;
	//file://重置sockaddr_in 结构变量；  
	//memset(&m_sockaddr, 0, sizeof(sockaddr_in));  
	//memset(&m_rsockaddr, 0, sizeof(sockaddr_in)); 

	return TRUE; 
}  

/////////////////////////////////////////   

int CClientSocket::Rev_Socket_Connect(char *lIPAddress, unsigned int iPort, UINT idex)//定义连接函数； 
{
	m_sockaddr.sin_addr.s_addr = inet_addr(lIPAddress); 
	m_sockaddr.sin_family = AF_INET; 
	m_sockaddr.sin_port = htons( iPort );  

	if(connect(g_hSocket_rev[idex], (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr)) == SOCKET_ERROR ) 	
	{  
		return FALSE; 

	}  

	return TRUE; 
}  


////////////////////////////////////////////////////  
int CClientSocket::Rev_Socket_Send( char* strData, int iLen ,UINT idex)//数据发送函数； 
{  
	if(strData == NULL || iLen == 0) 
	{	
		return FALSE;  
	}

	if( send(g_hSocket_rev[idex], strData, iLen, 0) == SOCKET_ERROR ) 
	{  
		return FALSE; 
	}  

	return TRUE; 
}  

/////////////////////////////////////////////////////  
int CClientSocket::Rev_Socket_Receive( char* strData, int iLen , UINT idex)//数据接收函数； 
{  
	int len = 0; 
	int ret = 0;  

	if(strData == NULL) 
	{
		return -1; 
	}

	ret = recv(g_hSocket_rev[idex], strData, iLen, 0); 
	return ret; 
}  



/////////////////////////////
BOOL CClientSocket::Socket_Create_update_finish(UINT idex)  
{
	//int status;
	//unsigned long cmd = 0;

	// m_hSocket是类内Socket对象，创建一个基于TCP/IP的Socket变量，并将值赋给该变量； 
	if(g_hSocket_update_finish_hanlde[idex] != INVALID_SOCKET)
	{
		closesocket(g_hSocket_update_finish_hanlde[idex]);
		g_hSocket_update_finish_hanlde[idex] = INVALID_SOCKET;
	}

	g_hSocket_update_finish_hanlde[idex] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_hSocket_update_finish_hanlde[idex] == INVALID_SOCKET) 
	{ 
		return FALSE; 
	}

	unsigned long ul = 1;
	int result;
	result = ioctlsocket(g_hSocket_update_finish_hanlde[idex], FIONBIO, &ul);
	if (result == SOCKET_ERROR) 
	{
		return FALSE;
	}

	return TRUE;  
}
BOOL CClientSocket::Socket_Close_update_finish(UINT idex)//关闭Socket对象； 
{  
	if (g_hSocket_update_finish_hanlde[idex] != INVALID_SOCKET)
	{
		if (closesocket(g_hSocket_update_finish_hanlde[idex]) == SOCKET_ERROR) 	
		{  
			return FALSE; 
		} 
	}
	g_hSocket_update_finish_hanlde[idex] = INVALID_SOCKET;
	//file://重置sockaddr_in 结构变量；  
	memset(&m_sockaddr, 0, sizeof(sockaddr_in));  
	memset(&m_rsockaddr, 0, sizeof(sockaddr_in)); 

	return TRUE; 
}  
int CClientSocket::Socket_Receive_update_finish( char* strData, int iLen , UINT idex)//数据接收函数； 
{  
	int len = 0; 
	int ret = 0; 
	CString str;

	if(strData == NULL) 
	{
		return -1; 
	}

	ret = recv(g_hSocket_update_finish_hanlde[idex], strData, iLen, 0); 
	return ret; 
} 

int CClientSocket::Socket_Connect_update_finish(char *lIPAddress, unsigned int iPort, UINT idex)//定义连接函数； 
{
	int result;
	unsigned long ul = 1;

	m_sockaddr.sin_addr.s_addr = inet_addr(lIPAddress); 
	m_sockaddr.sin_family = AF_INET; 
	m_sockaddr.sin_port = htons( iPort );  

	result = connect(g_hSocket_update_finish_hanlde[idex], (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr));
	if (1)//result < 0) 
	{
		DWORD ret = GetLastError();
		ret = ret;
		//m_bDisConnect = TRUE;
	}

	INT try_times = 0;
	do 
	{
		struct timeval timeout;
		fd_set fd_con, fd_con_w;

		FD_ZERO(&fd_con);
		FD_ZERO(&fd_con_w);
		FD_SET(g_hSocket_update_finish_hanlde[idex], &fd_con);
		FD_SET(g_hSocket_update_finish_hanlde[idex], &fd_con_w);
		timeout.tv_sec = 1; //2s 连接超时
		timeout.tv_usec = 100*1000;
		result = select(g_hSocket_update_finish_hanlde[idex] + 1, &fd_con_w, &fd_con, 0, &timeout);
		try_times ++;
		//	Sleep(100);
	} while(0); // ((result == 0) && (try_times < 10));
	if (result <= 0) 
	{
		DWORD ret = WSAGetLastError();
		return FALSE;
	}
	else
	{
		DWORD ret = WSAGetLastError();
		ret = FALSE;

	}
#if 0
	//设回阻塞方式
	ul = 0;
	if (ioctlsocket(g_hSocket_update_finish_hanlde[idex], FIONBIO, &ul) < 0)
	{
		return FALSE;	
	}
#endif
	return TRUE; 
}  


//////////////////////////////////////////////////////////////////////////////
//服务端 
BOOL CClientSocket::Socket_server_Create(void)  
{
	//int status;
	//unsigned long cmd = 0;

	// m_hSocket是类内Socket对象，创建一个基于TCP/IP的Socket变量，并将值赋给该变量； 
	if(g_hSocket_server != INVALID_SOCKET)
	{
		closesocket(g_hSocket_server);
	}

	if ((g_hSocket_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) 
		//if ((g_hSocket_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) 
	{ 
		return 0; 
	} 
	//status = ioctlsocket(m_hSocket_Client, FIONBIO, &cmd);

	return 1;  
}

BOOL CClientSocket::Socket_server_Close(void)//关闭Socket对象； 
{  

	shutdown(g_hSocket_server, SD_BOTH);

	if (closesocket(g_hSocket_server) == SOCKET_ERROR) 	
	{  
		return 0; 
	} 
	g_hSocket_server = INVALID_SOCKET;
	//file://重置sockaddr_in 结构变量；  
	memset(&m_sockaddr, 0, sizeof(sockaddr_in));  
	memset(&m_rsockaddr, 0, sizeof(sockaddr_in)); 

	return 1; 
}  

/////////////////////////////////////////  
int CClientSocket::Socket_server_Connect(char *lIPAddress, unsigned int iPort)//定义连接函数； 
{
	m_sockaddr.sin_addr.s_addr = inet_addr(lIPAddress); 
	m_sockaddr.sin_family = AF_INET; 
	m_sockaddr.sin_port = htons( iPort );  

	if(connect(g_hSocket_server, (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr)) == SOCKET_ERROR ) 	
	{  
		return 0; 

	}  

	return 1; 
}  


///////////////////////////////////////////////////////  
int CClientSocket::Socket_server_Bind(char* strIP, unsigned int iPort)//绑定函数； 
{  

	if(iPort == 0) 
	{
		return FALSE;  
	}

	memset(&m_sockaddr,0, sizeof(m_sockaddr)); 

	m_sockaddr.sin_family = AF_INET;  

	m_sockaddr.sin_addr.s_addr = INADDR_ANY; 

	m_sockaddr.sin_port = htons(iPort);  

	if (bind(g_hSocket_server, (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr)) == SOCKET_ERROR ) 	
	{  
		return 0; 
	}  

	return 1; 
}  

//////////////////////////////////////////  
int CClientSocket::Socket_server_Accept()//建立连接函数，S为监听Socket对象名； 
{   

	int Len = sizeof(m_rsockaddr);  

	memset(&m_rsockaddr, 0, sizeof(m_rsockaddr));  

	if((g_hSocket_server_new = accept(g_hSocket_server, (SOCKADDR*)&m_rsockaddr, &Len)) == INVALID_SOCKET) 
	{  
		return 0; 
	}  

	return 1; 
}  


/////////////////////////////////////////////////////  
int CClientSocket::Socket_server_asyncSelect(HWND hWnd, unsigned int wMsg, long lEvent) 
//file://事件选择函数； 
{  

	if(!IsWindow( hWnd ) || wMsg == 0 || lEvent == 0) 
	{
		return 0; 
	}


	if(WSAAsyncSelect( g_hSocket_server, hWnd, wMsg, lEvent) == SOCKET_ERROR ) 
	{  
		return 0; 
	}  

	return 1; 
}  


//////////////////////////////////////////////////// 
int CClientSocket::Socket_server_Listen( int iQueuedConnections )//监听函数； 
{  

	if(iQueuedConnections == 0) 
	{
		return 0; 
	}


	if(listen( g_hSocket_server, iQueuedConnections) == SOCKET_ERROR )  
	{  
		return 0; 
	}  

	return 1; 
}  

int CClientSocket::Socket_server_setsockopt( void)//数据接收函数； 
{  
	const char optval = SO_REUSEADDR;
	int err = -1;
	/*
	#if 1
	#if 0
	err = setsockopt(g_hSocket_server,SOL_SOCKET,SO_REUSEADDR, &optval,sizeof(optval) );
	if (err == -1 )
	{
	AfxMessageBox(_T("setsockopt SO_REUSEADDR fail"), MB_OK);
	return err;
	}
	#endif

	int nNetTimeout=10000;

	err = setsockopt(g_hSocket_server,SOL_SOCKET,SO_RCVTIMEO, (const char *)&nNetTimeout,sizeof(int) );

	return err;
	#else
	return 0;

	#endif
	*/

#if 1
	const DWORD SIO_KEEPALIVE_VALS   = IOC_IN | IOC_VENDOR | 4;
	TCP_KEEPALIVE_INFO inKeepAlive = {0};
	unsigned long ulInLen = sizeof(struct TCP_KEEPALIVE_INFO);
	struct TCP_KEEPALIVE_INFO outKeepAlive = {0};
	unsigned long ulOutLen = sizeof(struct TCP_KEEPALIVE_INFO);
	unsigned long ulBytesReturn = 0;

	inKeepAlive.onoff=1;
	inKeepAlive.keepaliveinterval=5000; //单位为毫秒
	inKeepAlive.keepalivetime=1000;      //单位为毫秒
	err=WSAIoctl(g_hSocket_server, SIO_KEEPALIVE_VALS, (LPVOID)&inKeepAlive, ulInLen,(LPVOID)&outKeepAlive, ulOutLen, &ulBytesReturn, NULL, NULL);
	return err;
#endif

}  


////////////////////////////////////////////////////  

//注意，使用的socket是新的
int CClientSocket::Socket_server_Send( char* strData, int iLen )//数据发送函数； 
{  
	if(strData == NULL || iLen == 0) 
	{	
		//fprintf(stderr, "000 g_hSocket_server_new:%x, iLen:%d\n", g_hSocket_server_new, iLen);
		return FALSE;  
	}

	if( send(g_hSocket_server_new, strData, iLen, 0) == SOCKET_ERROR ) 
	{  
		DWORD ret = GetLastError();
		//fprintf(stderr, "Socket_server_Send:%x, iLen:%d, ret:%d\n", g_hSocket_server_new, iLen, ret);
		return FALSE; 
	}  

	return TRUE; 
}  

/////////////////////////////////////////////////////  
int CClientSocket::Socket_server_Receive( char* strData, int iLen )//数据接收函数； 
{  
	int len = 0; 
	int ret = 0;  

	if(strData == NULL) 
	{
		return -1; 
	}

	ret = recv(g_hSocket_server_new, strData, iLen, 0); 

	//fprintf(stderr, "Socket_server_Receive:%x, iLen:%d, ret:%d\n", g_hSocket_server_new, iLen, ret);
	return ret; 
}  