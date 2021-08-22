// NetCtrl.cpp: implementation of the CNetCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "ISPCTRL_TOOLDlg.h"
#include "NetCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

typedef struct _cmd_head
{
	T_U16 attr_id;
	T_U16 cmd_type;
}CMD_HEAD;

typedef struct _trans_data
{
	CMD_HEAD	head;
	T_U32		datalen;
	char* 		data;
}TRANS_DATA, *PTRANS_DATA;

CNetCtrl::CNetCtrl()
{
	WORD wVersionRequested;
    WSADATA wsaData;
    int ret = 0;
	m_bConnect = FALSE;

	m_thread_heat = INVALID_HANDLE_VALUE;

	test_pass_flag = 0;
	heat_close_flag = FALSE;
	m_hSocket_heat = INVALID_SOCKET;

	//WinSock初始化
    wVersionRequested = MAKEWORD(2, 2); //希望使用的WinSock DLL 的版本
    ret = WSAStartup(wVersionRequested, &wsaData);
    if(ret != 0)
    {
        fprintf(stderr, "WSAStartup() failed!\n");
    }
}

CNetCtrl::~CNetCtrl()
{
	WSACleanup();
}

void RecvThread(void *argv)
{
	CNetCtrl* pthis = (CNetCtrl*)argv;

	int ret = 0;
	unsigned int size = 0;
	int recvlen = 0;
	TRANS_DATA cmd;

	while (1)
	{		
		size = ISP_PACKET_HEAD_SIZE;
		memset(&cmd, 0, sizeof(TRANS_DATA));
		ret = recv(pthis->m_tcp_client, (char*)&cmd.head, size, 0);
	
		if (SOCKET_ERROR == ret)
		{
			fprintf(stderr, "recv failed! code:%d\n", WSAGetLastError());
			break;
		}

		fprintf(stderr, "recv: attr id:%d\n", cmd.head.attr_id);
		fprintf(stderr, "recv: cmd type:%d\n", cmd.head.cmd_type);

		if ((cmd.head.attr_id >= ISP_ATTR_TYPE_NUM)
			||(cmd.head.cmd_type >= CMD_TYPE_NUM))
		{
			fprintf(stderr, "recv: addr_id or cmd type err!\n");
			continue;
		}

		if ((CMD_REPLY == cmd.head.cmd_type)
			|| (CMD_REPLY_TXT == cmd.head.cmd_type)
			|| (CMD_RET == cmd.head.cmd_type))
		{
			size = ISP_PARM_LEN_SIZE;
			ret = recv(pthis->m_tcp_client, (char*)&cmd.datalen, size, 0);
		
			if (SOCKET_ERROR == ret)
			{
				fprintf(stderr, "recv failed! code:%d\n", WSAGetLastError());
				break;
			}

			recvlen = 0;
			size = cmd.datalen;
			cmd.data = (char*)malloc(size);

			if (NULL == cmd.data)
			{
				fprintf(stderr, "NULL == cmd.data\n");
				break;
			}

			memset(cmd.data, 0, size);

			while (recvlen < cmd.datalen)
			{
				ret = recv(pthis->m_tcp_client, cmd.data + recvlen, size, 0);
				if (SOCKET_ERROR == ret)
				{
					fprintf(stderr, "recv failed! code:%d\n", WSAGetLastError());
					free(cmd.data);
					cmd.data = NULL;
					return;
				}
				recvlen += ret;
				size -= ret;
			}
		}

		if (pthis->m_pRecvCB)
		{
			pthis->m_pRecvCB(cmd.head.attr_id, cmd.head.cmd_type, cmd.data, cmd.datalen, pthis->m_pRecvCBP);
		}

		if (NULL != cmd.data)
		{
			free(cmd.data);
			cmd.data = NULL;
		}
	}
}


bool CNetCtrl::TcpClientConnect(unsigned long remote_ip, UINT remote_port)
{
	int ret = 0;
	struct sockaddr_in saServer;
	unsigned long ul = 1;
	struct timeval tv;  
	fd_set wset;  

	m_tcp_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_tcp_client == INVALID_SOCKET)
	{
		fprintf(stderr, "socket() failed!\n");
		return FALSE;
	}

	//构建服务器地址信息
	saServer.sin_family = AF_INET; //地址家族
	saServer.sin_port = htons(remote_port); //注意转化为网络字节序
	saServer.sin_addr.S_un.S_addr = remote_ip; 

	

	ioctlsocket(m_tcp_client, FIONBIO, (unsigned long*)&ul);
	ret = connect(m_tcp_client, (struct sockaddr *)&saServer, sizeof(saServer));

	if(ret < 0)  
	{
		tv.tv_sec = 2;  
		tv.tv_usec = 0;  
		
		FD_ZERO(&wset);  
		FD_SET(m_tcp_client,&wset);  
		ret = select(0,NULL,&wset,NULL,&tv);  
		if(ret < 0)  
		{ // select出错  
			fprintf(stderr, "connect() failed!\n");
			closesocket(m_tcp_client);
			return FALSE;
		}  
		else if (0 == ret)  
		{ // 超时  
			fprintf(stderr, "connect() timeout!\n");
			closesocket(m_tcp_client);
			return FALSE;
		}  
		else  
		{  
			int error = -1;
			int optLen = sizeof(int);
			getsockopt(m_tcp_client, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen); 

			if (0 != error)
			{
				fprintf(stderr, "connect() failed!\n");
				closesocket(m_tcp_client);
				return FALSE;
			}

			// 连接成功  
			fprintf(stderr, "connect() OK!\n");
			m_bConnect = TRUE;
			//AfxMessageBox("连接成功！", 0, 0);

			ul = 0;
			ioctlsocket(m_tcp_client, FIONBIO, (unsigned long*)&ul);

			m_recvtask = CreateThread(NULL, TRANS_STACKSIZE, (LPTHREAD_START_ROUTINE)RecvThread, this, 0, NULL);
			return TRUE;
		}  
	}
	else
	{
		// 连接成功  
			fprintf(stderr, "connect() OK!\n");
			m_bConnect = TRUE;
			//AfxMessageBox("连接成功！", 0, 0);

			ul = 0;
			ioctlsocket(m_tcp_client, FIONBIO, (unsigned long*)&ul);

			m_recvtask = CreateThread(NULL, TRANS_STACKSIZE, (LPTHREAD_START_ROUTINE)RecvThread, this, 0, NULL);
			return TRUE;
	}

}


bool CNetCtrl::TcpClientClose()
{
	int ret = 0;

	ret = closesocket(m_tcp_client);
	
	if (ret == SOCKET_ERROR)
	{
		fprintf(stderr, "closesocket() failed!\n");
		AfxMessageBox("断开失败！", 0, 0);
		return FALSE;
	}
	else 
	{
		fprintf(stderr, "closesocket() OK!\n");
		m_bConnect = FALSE;
		AfxMessageBox("连接已断开！", 0, 0);
		TerminateThread(m_recvtask, 0);
		return TRUE;
	}
}

bool CNetCtrl::IsConnected()
{
	return m_bConnect;
}

BOOL decode_command(char *lpBuf, char *commad_type, char *file_name, char *param)
{
	//重新定义结构
	int len_temp = 0;
	int param_len_temp = 0;
	int nFlags = 0;
	T_NET_INFO trance = {0};
	int ret = 0, i = 0;
	char auto_test_flag = 0;
	short check_sum = 0;
	short check_sum_temp = 0;
	
	
	//lpBuf结构是按T_NET_INFO结构排放的
	strncpy((char *)&trance.len, lpBuf, 2);
	
	strncpy(&trance.commad_type, &lpBuf[2], 1);
	*commad_type = trance.commad_type;
	
	strncpy((char *)&trance.auto_test_flag, &lpBuf[3], 1);
	auto_test_flag = trance.auto_test_flag;
	
	strncpy((char *)&len_temp, &lpBuf[4], 2);
	if (len_temp > 0 && file_name != NULL)
	{
		strncpy(file_name, &lpBuf[6], len_temp);
	}
	
	strncpy((char *)&param_len_temp, &lpBuf[6 + len_temp], 2);
	if (param_len_temp > 0 && param != NULL)
	{
		strncpy(param, &lpBuf[8 + len_temp], param_len_temp);
	}
	
	strncpy((char *)&check_sum, &lpBuf[8 + len_temp + param_len_temp], 2);
	
	//检测包
	for (i = 2; i < trance.len - 2; i++)
	{
		check_sum_temp += lpBuf[i];
	}
	
	if (check_sum != check_sum_temp)
	{
		return FALSE;
	}
	
	return TRUE;
}


void check_rev_date_thread(void *argv)
{
	CNetCtrl* pthis = (CNetCtrl*)argv;
	char commad_type; 
	char commad_type_temp; 
	char *file_name = NULL;
	char *param = NULL;
	int ret = 0;
	char lpBuf[256] = {0};
	UINT nBufLen = 256;
	UINT time1 = 0;
	UINT time2 = 0;
	char g_param[256] = {0};
	
	//获取心跳命令
	while (1)
	{
		Sleep(100);
		if (pthis->m_hBurnThread_rev_data != INVALID_HANDLE_VALUE)
		{
			commad_type = 0;  //初始化
			ret = recv(pthis->m_tcp_client, lpBuf, nBufLen, 0);
			if (SOCKET_ERROR == ret)
			{
				pthis->test_pass_flag = 2;
				AfxMessageBox(_T("接收数据错误，请检查"));
				return;
			}
			//lpBuf结构是按T_NET_INFO结构排放的
			strncpy(&commad_type, &lpBuf[2], 1);
			if(commad_type == TEST_RESPONSE)
			{
				memset(g_param, 0, 256);
				if (!decode_command(lpBuf, &commad_type_temp, NULL, g_param))
				{
					pthis->test_pass_flag = 2;	
				}
				else
				{
					if (g_param[0] == 49)  //49 表示1
					{
						pthis->test_pass_flag = 1;
					}
					else
					{
						pthis->test_pass_flag = 2;
					}
				}
			}
		}
	}
	return;
}

BOOL CNetCtrl::creat_socket(char *lIPAddress, UINT remote_port)
{
	int ret = 0;
	struct sockaddr_in saServer;
	unsigned long ul = 1;
	struct timeval tv;  
	fd_set wset; 
	
	m_tcp_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (m_tcp_client == INVALID_SOCKET)
	{
		fprintf(stderr, "socket() failed!\n");
		return FALSE;
	}
	
	//构建服务器地址信息
	//saServer.sin_family = AF_INET; //地址家族
	//saServer.sin_port = htons(remote_port); //注意转化为网络字节序
	//saServer.sin_addr.S_un.S_addr = remote_ip; 

	saServer.sin_addr.s_addr = inet_addr(lIPAddress); 
	saServer.sin_family = AF_INET; 
	saServer.sin_port = htons(remote_port);  

	ioctlsocket(m_tcp_client, FIONBIO, (unsigned long*)&ul);
	ret = connect(m_tcp_client, (struct sockaddr *)&saServer, sizeof(saServer));

	if(ret < 0)  
	{
		tv.tv_sec = 2;  
		tv.tv_usec = 0;  
		
		FD_ZERO(&wset);  
		FD_SET(m_tcp_client,&wset);  
		ret = select(0,NULL,&wset,NULL,&tv);  
		if(ret < 0)  
		{ // select出错  
			fprintf(stderr, "connect daemon failed!\n");
			AfxMessageBox("连接daemon失败！", 0, 0);
			return FALSE;
		}  
		else if (0 == ret)  
		{ // 超时  
			fprintf(stderr, "connect daemon timeout!\n");
			AfxMessageBox("连接超时！", 0, 0);
			return FALSE;
		}  
		else  
		{  
			int error = -1;
			int optLen = sizeof(int);
			getsockopt(m_tcp_client, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen); 

			if (0 != error)
			{
				fprintf(stderr, "connect daemon failed!\n");
				AfxMessageBox("连接daemon失败！", 0, 0);
				return FALSE;
			}

			// 连接成功  
			fprintf(stderr, "connect daemon OK!\n");
			//AfxMessageBox("连接成功！", 0, 0);

			ul = 0;
			ioctlsocket(m_tcp_client, FIONBIO, (unsigned long*)&ul);
			
			m_hBurnThread_rev_data = CreateThread(NULL, TRANS_STACKSIZE, (LPTHREAD_START_ROUTINE)check_rev_date_thread, this, 0, NULL);
			return TRUE;
		}  
	}
	else
	{
		// 连接成功  
		fprintf(stderr, "connect daemon OK!\n");
		//AfxMessageBox("连接成功！", 0, 0);

		ul = 0;
		ioctlsocket(m_tcp_client, FIONBIO, (unsigned long*)&ul);
		
		m_hBurnThread_rev_data = CreateThread(NULL, TRANS_STACKSIZE, (LPTHREAD_START_ROUTINE)check_rev_date_thread, this, 0, NULL);
		return TRUE;
	}
}

bool CNetCtrl::Socket_close()
{
	int ret = 0;
	
	ret = closesocket(m_tcp_client);
	
	if (ret == SOCKET_ERROR)
	{
		if (m_hBurnThread_rev_data != INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_hBurnThread_rev_data, 0);
		}
		fprintf(stderr, "closesocket() failed!\n");
		return FALSE;
	}
	else 
	{
		fprintf(stderr, "closesocket() OK!\n");
		if (m_hBurnThread_rev_data != INVALID_HANDLE_VALUE)
		{
			TerminateThread(m_hBurnThread_rev_data, 0);
		}
		return TRUE;
	}
}


int CNetCtrl::Socket_Send( char* strData, int iLen )//数据发送函数； 
{  
	if(strData == NULL || iLen == 0) 
	{	
		return FALSE;  
	}
	
	if( send(m_tcp_client, strData, iLen, 0) == SOCKET_ERROR ) 
	{  
		return FALSE; 
	}  
	
	return TRUE; 
} 


bool CNetCtrl::SendCommand(short addr_id, short cmd_type, char* pData, int len)
{
	int size = len + ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE;
	int ret = SOCKET_ERROR;

	if ((addr_id >= ISP_ATTR_TYPE_NUM)
		|| (cmd_type >= CMD_TYPE_NUM))
	{
		fprintf(stderr, "send failed! addr_id or cmd_type err : %d, %d\n", addr_id, cmd_type);
		return FALSE;
	}

	char* senddata = new char[size];
	char* pos = senddata;

	memcpy(pos, &addr_id, ISP_ATTR_ID_SIZE);
	pos += ISP_ATTR_ID_SIZE;
	memcpy(pos, &cmd_type, ISP_CMD_TYPE_SIZE);
	if ((NULL != pData) && (len > 0))
	{
		pos += ISP_CMD_TYPE_SIZE;
		memcpy(pos, &len, ISP_PARM_LEN_SIZE);
		pos += ISP_PARM_LEN_SIZE;
		memcpy(pos, pData, len);
	}
	else
	{
		size -= ISP_PARM_LEN_SIZE;
	}
	
	ret = send(m_tcp_client, senddata, size, 0);

	delete[] senddata;
	
	if (ret == SOCKET_ERROR)
	{
		fprintf(stderr, "send failed! code:%d\n", WSAGetLastError());
		return FALSE;
	}
	else 
	{
		fprintf(stderr, "send() OK!\n");
		return TRUE;
	}
}


int CNetCtrl::SetRecvCallBack(RECVCB pCB, void *pParam)
{
	m_pRecvCB = pCB;
	m_pRecvCBP = pParam;
	return 0;
}

//***************************************************************************************
//如下是创建心跳的socket
BOOL CNetCtrl::Heat_Socket_Create(void)  
{
	//int status;
	//unsigned long cmd = 0;
	
	// m_hSocket是类内Socket对象，创建一个基于TCP/IP的Socket变量，并将值赋给该变量； 
	if(m_hSocket_heat != INVALID_SOCKET)
	{
		closesocket(m_hSocket_heat);
	}
	
	if ((m_hSocket_heat = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) 
	{ 
		return FALSE; 
	} 
	//status = ioctlsocket(m_hSocket_Client, FIONBIO, &cmd);
	
	return TRUE;  
}

BOOL CNetCtrl::Heat_Socket_Close(void)//关闭Socket对象； 
{ 
	close_thread_heat();
	if (m_hSocket_heat != INVALID_SOCKET)
	{
		if (closesocket(m_hSocket_heat) == SOCKET_ERROR) 	
		{  
			return FALSE; 
		} 
	}
	
	//file://重置sockaddr_in 结构变量；  
	//memset(&m_sockaddr, 0, sizeof(sockaddr_in));  
	//memset(&m_rsockaddr, 0, sizeof(sockaddr_in)); 

	return TRUE; 
}  

/////////////////////////////////////////   
int CNetCtrl::Heat_Socket_Connect(char *lIPAddress, unsigned int iPort)//定义连接函数； 
{
	m_sockaddr.sin_addr.s_addr = inet_addr(lIPAddress); 
	m_sockaddr.sin_family = AF_INET; 
	m_sockaddr.sin_port = htons( iPort );  
	
	if(connect(m_hSocket_heat, (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr)) == SOCKET_ERROR ) 	
	{  
		return FALSE; 
		
	}  
	
	return TRUE; 
}  
 
///////////////////////////////////////////////////////  
 
int CNetCtrl::Heat_Socket_Bind(char* strIP, unsigned int iPort)//绑定函数； 
{  
	
	if(strlen(strIP) == 0 || iPort == 0) 
	{
		return FALSE;  
	}
	
	memset(&m_sockaddr,0, sizeof(m_sockaddr)); 
	
	m_sockaddr.sin_family = AF_INET;  
	
	m_sockaddr.sin_addr.s_addr = inet_addr(strIP); 
	
	m_sockaddr.sin_port = htons(iPort);  
	
	if (bind(m_hSocket_heat, (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr)) == SOCKET_ERROR ) 	
	{  
		return FALSE; 
	}  
	
	return TRUE; 
}  


////////////////////////////////////////////////////  
int CNetCtrl::Heat_Socket_Send( char* strData, int iLen )//数据发送函数； 
{  
	if(strData == NULL || iLen == 0) 
	{	
		return FALSE;  
	}
 
	if( send(m_hSocket_heat, strData, iLen, 0) == SOCKET_ERROR ) 
	{  
		return FALSE; 
	}  
 
	return TRUE; 
}  

/////////////////////////////////////////////////////  
int CNetCtrl::Heat_Socket_Receive(SOCKET hSocket_heat, char* strData, int iLen )//数据接收函数； 
{  
	int len = 0; 
	int ret = 0;  

	if(strData == NULL) 
	{
		return -1; 
	}

	ret = recv(hSocket_heat, strData, iLen, 0); 
	return ret; 
}  

void check_heartbeat_thread(void *argv)
{
	CNetCtrl* pthis = (CNetCtrl*)argv;
	char commad_type; 
	char *file_name = NULL;
	char *param = NULL;
	CNetCtrl NetCtrl;
	int ret = 0;
	char lpBuf[256] = {0};
	UINT nBufLen = 256;
	UINT time1 = 0;
	UINT time2 = 0;
	char g_param[256] = {0};
	
	time1 = GetTickCount();
	time2 = GetTickCount();
	pthis->heat_close_flag = FALSE;
	//获取心跳命令
	while (1)
	{
		Sleep(500);
		if (pthis->m_thread_heat != INVALID_HANDLE_VALUE && pthis->g_test_flag)
		{
			commad_type = 0;  //初始化
			ret = NetCtrl.Heat_Socket_Receive(pthis->m_hSocket_heat, lpBuf, nBufLen);
			if (ret == -1)
			{
				if (pthis->m_thread_heat == INVALID_HANDLE_VALUE)
				{
					return;
				}
				pthis->heat_close_flag = TRUE;
				pthis->test_pass_flag = 2;
				AfxMessageBox(_T("接收数据错误，请检查"));
				return;
			}
			strncpy(&commad_type, &lpBuf[2], 1);
			if (commad_type == TEST_HEARTBEAT)
			{
				time2 = GetTickCount();
			}
			else
			{
				time1 = GetTickCount();
				if (time1 - time2 > 5000)
				{
					pthis->heat_close_flag = TRUE;
					pthis->test_pass_flag = 2;
					AfxMessageBox(_T("小机没有正常动行，请检查,并重新连接测试"));
					return;
				}
				continue;
			}
		}
	}
	
	
	return ;
}

BOOL CNetCtrl::create_thread_heat(char *ipaddr, UINT net_uPort)
{
	
	USES_CONVERSION;
	
	//创建一个心跳线程的socket
	Heat_Socket_Create();
	
	if(!Heat_Socket_Connect(ipaddr, net_uPort))	
	{
		return FALSE;
	}
	
	if (m_thread_heat != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_thread_heat);
		m_thread_heat = INVALID_HANDLE_VALUE;
	}

	m_thread_heat = CreateThread(NULL, TRANS_STACKSIZE, (LPTHREAD_START_ROUTINE)check_heartbeat_thread, this, 0, NULL);

	if (m_thread_heat == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;
}

void CNetCtrl::close_thread_heat() 
{
	if(m_thread_heat != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_thread_heat);
		m_thread_heat = INVALID_HANDLE_VALUE;
	}
}