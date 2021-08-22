// SearchServer.h: interface for the CSearchServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEARCHSERVER_H__3AB527B5_3195_4808_9B25_EE58DBC32809__INCLUDED_)
#define AFX_SEARCHSERVER_H__3AB527B5_3195_4808_9B25_EE58DBC32809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AutoLock.h"
#include "Clock.h"

#define  IP_ADDRE_LEN   32

#define BROADCAST_PORT		8192
#define THREAD_STACK_SIZE	0 // π”√ƒ¨»œ1M
#define CREATE_SUSPENDED                  0x00000004
#define MAX_BUFFER_LEN			1024 * 10
#define TIME_OUT				500
#define MAX_RECV_ERROR_CNT		10

#define CON_SYSTEM_TAG(x, y)		memcpy(y, x, 4);
#define MAKE_SYSTEM_TAG(x)			(UINT)((x[3] << 24) | (x[2] << 16) | (x[1] << 8) | x[0])

#define SYSTEM_TAG					"give me your ip address!" //"AK39"

enum WorkFunction_en{
	WF_SEARCH = 0,
	WF_NORMAL,
	WF_UNKNOWN
};


typedef enum NetType_en
{
	NET_TYPE_TCP,
	NET_TYPE_UDP,
	NET_TYPE_MAX
}NET_TYPE;

typedef enum CommandType_en
{
	COMM_TYPE_OPEN_SERVICE = 0,
	COMM_TYPE_SET_PARAMETER,
	COMM_TYPE_GET_INFO,
	COMM_TYPE_SERVER_PROCESS_RETURN,
	COMM_TYPE_HEART_BEAT,
	COMM_TYPE_CNT
}COMMAND_TYPE;

typedef enum GetParamCommandType_en
{
	GET_COMM_RECODE_FILE = 0,
	GET_COMM_SERVER_INFO,
	GET_COMM_PRIVACY_AREA,
	GET_COMM_MOTION_DETECT_AREAS,
	GET_COMM_IMAGE_SET,
	GET_COMM_FILES_LIST,
	GET_COMM_ISP_INFO,
	GET_COMM_MAX
}GET_COMM_TYPE;

typedef struct sock_info
{
	int recv_fd;
	int send_fd;
	struct sockaddr_in recv_addr;
	struct sockaddr_in send_addr;
	UINT	send_mcast_ip;
	UINT	nListenPort;
	UINT	nSendPort;
}sock_info;

typedef struct SystemHeader_st
{
	UINT	nSystemTag;
	UINT	nCommandCnt;
	UINT	nLen;
}SYSTEM_HEADER;

typedef struct CommandHeader_st
{
	char	CommandType;
	char    subCommandType;
	short	nLen;
}COMMAND_HEADER;

typedef struct TimeInfo_t
{
	char		nYear;	//since 1970
	char		nMon;
	char		nDay;
	char		nHour;
	char		nMin;
	char		nSecond;
}TIME_INFO;


typedef int socklen_t;

class CSearchServer  
{
public:
	CSearchServer();
	virtual ~CSearchServer();

public:
	NET_TYPE m_NetType;

	UINT m_ThreadID;
	//string m_ThreadName;
	volatile bool m_bRun;
	CClock m_clock;
	UINT64 m_nLastMsSinceStart;
	BOOL m_bIsOpen;

	//char m_ServerIpAddr[256][20];
	//UINT m_ServerIpAddr_idex;

	int OpenSearch();
	UINT GetBroadcastIp();
	int InitSock();
	int InitUdpSock();
	bool search_Start();
	int search_IP_main();
	void ParseCommand(char *data, UINT len, struct sockaddr_in & sAddr);
	int SubCommandGetInfoParse(char *data, char subCommandType, UINT len, struct sockaddr_in & sAddr);
	int SendHeartBeat();
	int Search(TIME_INFO & SystemTime);
	int Broadcast_Search();
	void close_search_thread();
	void Parse_Date(char *buf); 
};


#endif // !defined(AFX_SEARCHSERVER_H__3AB527B5_3195_4808_9B25_EE58DBC32809__INCLUDED_)
