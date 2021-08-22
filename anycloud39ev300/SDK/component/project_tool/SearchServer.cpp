// SearchServer.cpp: implementation of the CSearchServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SearchServer.h"
#include "winsock.h"
#include "atlconv.h"
#include "config_test.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


BOOL m_bStop = FALSE;
int	m_nWorkFunction;
int m_nRecvErrorCnt;
BOOL  m_bDisConnect;
sock_info m_stSockInfo;
BOOL m_start_search = FALSE;
extern BOOL m_not_find_anyIP;
extern BOOL m_find_IP_end_flag;

extern TCHAR m_ip_address_buf[MAX_PATH][IP_ADDRE_LEN];
extern UINT m_ip_address_idex;
extern CConfig_test g_test_config;

HANDLE m_handle = INVALID_HANDLE_VALUE;

CSearchServer::CSearchServer()
{

}

CSearchServer::~CSearchServer()
{

}

int CSearchServer::SubCommandGetInfoParse(char *data, char subCommandType, UINT len, struct sockaddr_in & sAddr)
{
	char *pos = data;
	
	switch(subCommandType) 
	{
	case GET_COMM_SERVER_INFO:
		//获取ip地址

		//strcpy(m_ServerIpAddr[m_ServerIpAddr_idex], inet_ntoa(sAddr.sin_addr));
		//m_ServerIpAddr_idex++;
		break;
	default:
		fprintf(stderr, "SubCommandGetInfoParse no support %d sub command", subCommandType);
		break;
	}
	
	return 0;
}


void CSearchServer::ParseCommand(char *data, UINT len, struct sockaddr_in & sAddr)
{
	char *pos = data;
	char *EndPos = data + len;
	
	while (pos < EndPos) {
		if (len < sizeof(SYSTEM_HEADER)) {
			fprintf(stderr, "the data recv from net is wrong!\n");
			return;
		}
		
		SYSTEM_HEADER stSystemHeader = *(SYSTEM_HEADER*)(pos);
		char strSystemTag[5];
		ZeroMemory(strSystemTag, sizeof strSystemTag);
		CON_SYSTEM_TAG(&stSystemHeader.nSystemTag, strSystemTag);
		
		if (strcmp(strSystemTag, SYSTEM_TAG) != 0) {
			fprintf(stderr, "no our command!\n");
			return;
		}
		
		pos += sizeof(SYSTEM_HEADER);
		if (pos >= EndPos) {
			fprintf(stderr, "the data recv from net no any command content!\n");
			return;
		}
		
		for (unsigned int i = 0; i < stSystemHeader.nCommandCnt; ++i) {
			COMMAND_HEADER stCommandHeader = *(COMMAND_HEADER *)pos;
			pos += sizeof(COMMAND_HEADER);
			if (pos >= EndPos) {
				fprintf(stderr, "the data recv from net no any sub command!\n");
				return;
			}
			
			if (pos + stCommandHeader.nLen > EndPos) {
				fprintf(stderr, "the data recv from net have a invalid sub command type = %d!", stCommandHeader.subCommandType);
				return;
			}
			
			switch(stCommandHeader.CommandType) {
			case COMM_TYPE_OPEN_SERVICE: //server respond the command process status
				//MessageBox(NULL, L"waring", L"aaa", MB_OK); 
				//Sleep(1000);
				//ServerOpenProcessRespond(pos, stCommandHeader.subCommandType, stCommandHeader.nLen, sAddr);
				break;
			case COMM_TYPE_GET_INFO:
				SubCommandGetInfoParse(pos, stCommandHeader.subCommandType, stCommandHeader.nLen, sAddr);
				break;
			case COMM_TYPE_SERVER_PROCESS_RETURN:
				//ServerReturnParse(pos, stCommandHeader.nLen, sAddr);
				break;
			default:
				fprintf(stderr, "no support %d command", stCommandHeader.CommandType);
				break;
			}
			
			pos += stCommandHeader.nLen;
			len -= (pos - data);
		}
	}
	
	return;
}

//发送心跳包
int CSearchServer::SendHeartBeat()
{
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;
	
	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_HEART_BEAT;
	stCH.subCommandType = 0;
	stCH.nLen = sizeof(int);
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER);
	stSH.nLen = len;
	
	char * data = new char[len];
	char * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	
	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) {
		AfxMessageBox(L"send volume control error!", 0, 0);
		delete[] data;
		return -1;
	}
	
	delete[] data;
	
	return 0;
}

//Search命令发送
int CSearchServer::Search(TIME_INFO & SystemTime)
{

#if 0
	SYSTEM_HEADER stSH;
	ZeroMemory(&stSH, sizeof(SYSTEM_HEADER));
	stSH.nSystemTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	stSH.nCommandCnt = 1;
	
	COMMAND_HEADER stCH;
	ZeroMemory(&stCH, sizeof(COMMAND_HEADER));
	stCH.CommandType = COMM_TYPE_GET_INFO;
	stCH.subCommandType = GET_COMM_SERVER_INFO;
	stCH.nLen = sizeof(TIME_INFO);
	
	int len = sizeof(SYSTEM_HEADER) + sizeof(COMMAND_HEADER) + sizeof(TIME_INFO) + 4;
	
	stSH.nLen = len;
	
	char * data = new char[len];
	char * pos = data;
	memcpy(pos, &stSH, sizeof(SYSTEM_HEADER));
	pos += sizeof(SYSTEM_HEADER);
	memcpy(pos, &stCH, sizeof(COMMAND_HEADER));
	pos += sizeof(COMMAND_HEADER);
	memcpy(pos, &SystemTime, sizeof(TIME_INFO));

#endif

	char *data = "Anyka IPC ,Get IP Address!";
	UINT len = strlen(data);

	
	int ret = sendto(m_stSockInfo.send_fd, (char *)data, len, 0, (struct sockaddr *)&m_stSockInfo.send_addr, sizeof(struct sockaddr_in));
	if(ret < 0) 
	{
		fprintf(stderr, "send search info error!");
		//delete[] data;
		return -1;
	}
	m_start_search = TRUE;
	
	//delete[] data;
	return 0;
}

UINT UTF8StrToUnicode( const char* UTF8String, UINT UTF8StringLength, wchar_t* OutUnicodeString, UINT UnicodeStringBufferSize )
{
   UINT UTF8Index = 0;
   UINT UniIndex = 0;

   while ( UTF8Index < UTF8StringLength )
   {
       unsigned char UTF8Char = UTF8String[UTF8Index];

       if ( UnicodeStringBufferSize != 0 && UniIndex >= UnicodeStringBufferSize )
           break;

       if ((UTF8Char & 0x80) == 0) 
       {
           const UINT cUTF8CharRequire = 1;
           // UTF8字码不足
           if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
               break;

           if ( OutUnicodeString )
           {
               wchar_t& WideChar = OutUnicodeString[UniIndex]; 
               WideChar = UTF8Char;
           }
           UTF8Index++;
           
       } 
       else if((UTF8Char & 0xE0) == 0xC0)  ///< 110x-xxxx 10xx-xxxx
       {
           const UINT cUTF8CharRequire = 2;

           // UTF8字码不足
           if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
               break;

           if ( OutUnicodeString )
           {
               wchar_t& WideChar = OutUnicodeString[UniIndex]; 
               WideChar  = (UTF8String[UTF8Index + 0] & 0x3F) << 6;
               WideChar |= (UTF8String[UTF8Index + 1] & 0x3F);
           }
           
           UTF8Index += cUTF8CharRequire;
       }
       else if((UTF8Char & 0xF0) == 0xE0)  ///< 1110-xxxx 10xx-xxxx 10xx-xxxx
       {
           const UINT cUTF8CharRequire = 3;

           // UTF8字码不足
           if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
               break;

           if ( OutUnicodeString )
           {
               wchar_t& WideChar = OutUnicodeString[UniIndex]; 

               WideChar  = (UTF8String[UTF8Index + 0] & 0x1F) << 12;
               WideChar |= (UTF8String[UTF8Index + 1] & 0x3F) << 6;
               WideChar |= (UTF8String[UTF8Index + 2] & 0x3F);
           }
           UTF8Index += cUTF8CharRequire;
       } 
       else if((UTF8Char & 0xF8) == 0xF0)  ///< 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
       {
           const UINT cUTF8CharRequire = 4;

           // UTF8字码不足
           if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
               break;

           if ( OutUnicodeString )
           {
               wchar_t& WideChar = OutUnicodeString[UniIndex]; 

               WideChar  = (UTF8String[UTF8Index + 0] & 0x0F) << 18;
               WideChar  = (UTF8String[UTF8Index + 1] & 0x3F) << 12;
               WideChar |= (UTF8String[UTF8Index + 2] & 0x3F) << 6;
               WideChar |= (UTF8String[UTF8Index + 3] & 0x3F);
           }
           UTF8Index += cUTF8CharRequire;
       } 
       else ///< 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
       {
           const UINT cUTF8CharRequire = 5;

           // UTF8字码不足
           if ( UTF8Index + cUTF8CharRequire > UTF8StringLength )
               break;

           if ( OutUnicodeString )
           {
               wchar_t& WideChar = OutUnicodeString[UniIndex]; 

               WideChar  = (UTF8String[UTF8Index + 0] & 0x07) << 24;
               WideChar  = (UTF8String[UTF8Index + 1] & 0x3F) << 18;
               WideChar  = (UTF8String[UTF8Index + 2] & 0x3F) << 12;
               WideChar |= (UTF8String[UTF8Index + 3] & 0x3F) << 6;
               WideChar |= (UTF8String[UTF8Index + 4] & 0x3F);
           }
           UTF8Index += cUTF8CharRequire;
       }
       UniIndex++;
   }
   return UniIndex;
}




void CSearchServer::Parse_Date(char *buf)
{
	UINT buf_size = 0;
	UINT pos_len = 0;
	UINT i = 0;
	UINT idex = 0;
	UINT starpos = 0;
	UINT endpos = 0;
	TCHAR buffer[MAX_PATH] = {0};
	char temp_buf[MAX_PATH] = {0};
	int  chPtr[MAX_PATH] = {0};

	USES_CONVERSION;//

	buf_size = strlen((char *)buf);//长度
	for (i = 0; i < buf_size; i++)
	{
		if (buf[i] == 0x40)
		{
			endpos = i;
			memset(temp_buf, 0, MAX_PATH);
			strncpy(temp_buf, &buf[starpos], pos_len);
			starpos = endpos + 1;
			if (idex == 0) //获取通道名
			{
				//
				UTF8StrToUnicode(temp_buf, strlen(temp_buf), buffer, MAX_PATH);
				_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_channel_name, buffer);
				
			}
			else if(idex == 1) //ip
			{
				_tcscpy(m_ip_address_buf[m_ip_address_idex], A2T(temp_buf));
				_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_buffer,  m_ip_address_buf[m_ip_address_idex]);
			}
			else if (idex == 2)//掩码
			{
				_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_ch_buffer,  A2T(temp_buf));
			}
			else if (idex == 3)//网关
			{
				_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_wg_buffer,  A2T(temp_buf));
			}
			else if (idex == 4)//DNS1
			{
				_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_dns1_buffer,  A2T(temp_buf));
			}
			else if (idex == 5)//DNS2
			{
				_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_dns2_buffer,  A2T(temp_buf));
			}
			else
			{
				//AfxMessageBox(_T("接收到的数据是错误的")); 
				break;
			}
			idex++;
			pos_len = 0;
		}
		else
		{
			pos_len++;
			if (idex == 6)//DHCP
			{
				memset(temp_buf, 0, MAX_PATH);
				strncpy(temp_buf, &buf[starpos], pos_len);
				memcpy(&g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_dhcp_flag,  temp_buf, 1);
			}
		}
	}
	m_ip_address_idex++;
}



int CSearchServer::search_IP_main()
{
	char *data = NULL;
	struct timeval timeout;
    struct sockaddr_in sAddr;
	char temp_buf[IP_ADDRE_LEN] = {0};

	socklen_t len = sizeof(struct sockaddr_in);
	int ret = 0;

	USES_CONVERSION;//
	
	ZeroMemory(&sAddr, len);
	
	data = new char[MAX_BUFFER_LEN];

	timeout.tv_sec = TIME_OUT / 1000;
	timeout.tv_usec = (TIME_OUT % 1000) * 1000;

	timeval stTime = {0};
	m_clock.start(stTime);

	m_nLastMsSinceStart = 0;

	UINT64 nStartTime = 0;
	m_clock.GetMsSinceStart(nStartTime);
	
	int nReady = 0;
	while(TRUE)
	{
		if (m_bStop) break;

		if (!m_start_search)
		{
			continue;
		}

		fd_set rset;
		
		FD_ZERO(&rset);
		FD_SET(m_stSockInfo.recv_fd, &rset);
		
		nReady = select(m_stSockInfo.recv_fd + 1, &rset, NULL, NULL, &timeout);
		if(nReady <= 0) 
		{ 
			DWORD ret = WSAGetLastError();

			//select error or timeout.
			if (nReady == 0)
			{
				m_clock.GetMsSinceStart(m_nLastMsSinceStart);

				UINT64 diff = m_nLastMsSinceStart - nStartTime ;
				if (diff > 2000) 
				{
					m_find_IP_end_flag = TRUE;
					return -1;
				}
			}

			continue;
		}
		else
		{
			//if (m_NetType == NET_TYPE_UDP) {
			memset(data, 0, MAX_BUFFER_LEN);
			ret = recvfrom(m_stSockInfo.recv_fd, (char *)data, MAX_BUFFER_LEN, 0, (struct sockaddr *)&sAddr, &len);
			if (ret <= 0)
			{
				++m_nRecvErrorCnt;
				if (m_nRecvErrorCnt >= MAX_RECV_ERROR_CNT) 
				{//连续recvfrom返回进行断线处理
					m_nRecvErrorCnt = 0;
					m_bDisConnect = TRUE;
					m_bStop = TRUE;
					m_start_search = FALSE;
					closesocket(m_stSockInfo.send_fd);
					m_nWorkFunction = WF_UNKNOWN;
					goto End; 
				}

				fprintf(stderr, "recv from net error! error = %d\n", WSAGetLastError());
				continue;
			}
			
			if ((m_stSockInfo.send_mcast_ip) && (sAddr.sin_addr.S_un.S_addr == m_stSockInfo.send_mcast_ip))
			{ 
				//from host
				continue;
			}

			//判断i是否正确

			//获取ip地址
#if 1
			Parse_Date(data);
#else
			memset(temp_buf, 0, IP_ADDRE_LEN);
			strcpy(temp_buf, inet_ntoa(sAddr.sin_addr));
			_tcscpy(m_ip_address_buf[m_ip_address_idex], A2T(temp_buf));
			m_ip_address_idex++;
#endif

			m_clock.GetMsSinceStart(nStartTime);
		}
	}

End:
	delete[] data;
	return 0;
}

DWORD WINAPI search_IP_thread(LPVOID lpParameter)
{
	CSearchServer search;

	search.search_IP_main();

	search.close_search_thread();

	if (m_ip_address_idex == 0)
	{
		m_not_find_anyIP = TRUE;
	}

	return 1;
}


void CSearchServer::close_search_thread() 
{
	if(m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}

bool CSearchServer::search_Start()
{
	if ( m_bRun )
	{
		return true;
	}

	if (m_handle != INVALID_HANDLE_VALUE)
	{
		close_search_thread();
		m_handle = INVALID_HANDLE_VALUE;
	}

	m_handle = CreateThread(NULL, 0, search_IP_thread, 0, 0, NULL);
	if (m_handle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	m_bRun = (INVALID_HANDLE_VALUE != m_handle);
	
	return m_bRun;
}


int CSearchServer::InitUdpSock()
{
	int optval = 1;

	if((m_stSockInfo.send_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		AfxMessageBox(L"create send socket fail", 0, 0);
		return -1;
	}
	
	optval = 1;
	if (setsockopt(m_stSockInfo.send_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(int)) < 0) 
	{
		AfxMessageBox(L"send setsockopt fail!", 0, 0);
		closesocket(m_stSockInfo.send_fd);
		return -1;
	}
	
	optval = 1;
    if (m_stSockInfo.send_mcast_ip != 0) 
	{
		if (setsockopt(m_stSockInfo.send_fd, SOL_SOCKET, SO_BROADCAST, (const char *)&optval, sizeof(int)) < 0) {
            AfxMessageBox(L"send_sock udp setsockopt fail", 0, 0);
			closesocket(m_stSockInfo.send_fd);
			return -1;
		}
		
		m_stSockInfo.send_addr.sin_addr.s_addr  = htonl(INADDR_BROADCAST);
	}
	
	m_stSockInfo.recv_fd = m_stSockInfo.send_fd;
	
	m_stSockInfo.send_addr.sin_family = AF_INET;
	m_stSockInfo.send_addr.sin_port = htons(m_stSockInfo.nSendPort);
	
	return 0;
}

int CSearchServer::InitSock()
{
	m_nRecvErrorCnt = 0;
	m_bDisConnect = FALSE;
	if (m_NetType == NET_TYPE_UDP) return InitUdpSock();
	else return -1;
}

unsigned int CSearchServer::GetBroadcastIp()
{
	char szHostName[50] = {0};
	
	if (gethostname(szHostName, sizeof(szHostName)) == 0) {
		hostent * hostInfo = gethostbyname(szHostName);
		if (NULL == hostInfo){
			AfxMessageBox(L"can't get host ip address!", 0, 0);
			return 0;
		}
		
		struct in_addr * hAddr = (struct in_addr *)*(hostInfo->h_addr_list);
		return hAddr->S_un.S_addr;
	}
	
	return 0;
}


int CSearchServer::OpenSearch()
{
	if (m_nWorkFunction != WF_UNKNOWN) {
		//AfxMessageBox(L"already work in Search or Normal mode!", 0, 0);
		return 0;
	}
	
	m_bStop = FALSE;
	m_nWorkFunction = WF_SEARCH;
	m_NetType = NET_TYPE_UDP; //search function only can work in udp mode
	
	unsigned int nBroadcastIp = GetBroadcastIp();
	if (nBroadcastIp == 0) {
		AfxMessageBox(L"can't get broadcast ip address!", 0, 0);
		m_nWorkFunction = WF_UNKNOWN;
		return -1;
	}
	
	m_stSockInfo.send_mcast_ip = nBroadcastIp;
	m_stSockInfo.nSendPort = BROADCAST_PORT;
	m_stSockInfo.nListenPort = BROADCAST_PORT;
	
	if (InitSock() < 0) {
		m_nWorkFunction = WF_UNKNOWN;
		return -1;
	}
	
	return search_Start();
}

int CSearchServer::Broadcast_Search()
{
	if (1)//!m_bIsOpen) 
	{
		m_start_search = FALSE;
		m_nWorkFunction = WF_UNKNOWN;
		m_bRun = FALSE;
		OpenSearch();
		//m_NetCtrl.RegisterServerRespond(this);
		m_bIsOpen = TRUE;
	}
	
	TIME_INFO stSystemTime = {0};
	return Search(stSystemTime);
}

