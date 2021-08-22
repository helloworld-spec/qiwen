#include "StdAfx.h"
#include "ServerSearch.h"
#include <time.h>
#include "Config_test.h"


BOOL m_start_search = FALSE;
int	m_nWorkFunction;
sock_info m_stSockInfo;
BOOL m_bStop = FALSE;
int m_nRecvErrorCnt;
BOOL  m_bDisConnect;
HANDLE m_handle = INVALID_HANDLE_VALUE;
extern UINT m_ip_address_idex;
extern BOOL m_not_find_anyIP;
extern BOOL m_find_IP_end_flag;
extern BOOL m_find_anyIP;

extern BOOL g_sousuo_flag;

extern CConfig_test g_test_config;

extern BOOL g_first_find_flag;

extern UINT current_ip_idex;
extern TCHAR m_connect_ip[MAX_PATH+1];


enum WorkFunction_en{
	WF_SEARCH = 0,
	WF_NORMAL,
	WF_UNKNOWN
};

CServerSearch::CServerSearch(void)
: m_bIsOpen(FALSE)
{
}

CServerSearch::~CServerSearch(void)
{
	DeleteAllServer();
	m_NetCtrl.CloseSearch();
}

int CServerSearch::Search()
{
	if (!m_bIsOpen) {
		m_NetCtrl.OpenSearch();
		m_NetCtrl.RegisterServerRespond(this);
		m_bIsOpen = TRUE;
	}

	time_t t = time(0);
	struct tm * ptNow = NULL;
	
	ptNow = localtime(&t);
	TIME_INFO stSystemTime = {0};

	stSystemTime.nYear = (ptNow->tm_year + 1900) - 1970;
	stSystemTime.nMon = ptNow->tm_mon + 1;
	stSystemTime.nDay = ptNow->tm_mday;
	stSystemTime.nHour = ptNow->tm_hour;
	stSystemTime.nMin = ptNow->tm_min;
	stSystemTime.nSecond = ptNow->tm_sec;

	return m_NetCtrl.Search(stSystemTime);
}

int CServerSearch::ServerRespond(SERVER_INFOEX & stSInfo)
{
	return MakeServer(stSInfo);
}

int CServerSearch::GetServerCount()
{
	CAutoLock lock(&m_cs);
	return m_mapServer.size();
}

int CServerSearch::GetServer(int iServerNum, IServer ** ppIServer)
{
	CAutoLock lock(&m_cs);
	if (((unsigned int)iServerNum >= m_mapServer.size()) || (iServerNum < 0)) return -1;
	
	map<string, Server*>::iterator iterService = m_mapServer.begin();

	for (int i = 0; i != iServerNum; ++i) ++iterService;

	*ppIServer = iterService->second;

	return 0;
}

int CServerSearch::MakeServer(SERVER_INFOEX & stSInfo)
{
	Server * ser = new Server();
	
	strncpy(ser->m_strServerID, stSInfo.stServerInfo.strDeviceID, MAX_ID_LEN);
	strncpy(ser->m_strServerIP, stSInfo.astrServerIpAddr, MAX_IP_LEN);
	
	if (strlen(stSInfo.stServerInfo.strMainAddr)) {
		ser->m_nStreamCnt += 1;
		strncpy(ser->m_strServerStreamName[0], stSInfo.stServerInfo.strMainAddr, MAX_STREAM_NAME);
		ser->m_nFps[0] = stSInfo.stServerInfo.nMainFps;

		switch(stSInfo.stServerInfo.enMainStream) {
		case VIDEO_MODE_QVGA:
			ser->m_enStreamMode[0] = STREAM_MODE_VIDEO_QVGA;
			break;
		case VIDEO_MODE_VGA:
			ser->m_enStreamMode[0] = STREAM_MODE_VIDEO_VGA;
			break;
		case VIDEO_MODE_D1:
			ser->m_enStreamMode[0] = STREAM_MODE_VIDEO_D1;
			break;
		case VIDEO_MODE_720P:
			ser->m_enStreamMode[0] = STREAM_MODE_VIDEO_720P;
			break;
		default:
			fprintf(stderr, "main stream %d mode unsupport\n", stSInfo.stServerInfo.enMainStream);
			ser->m_enStreamMode[0] = STREAM_MODE_MAX;
			break;
		}
	}

	if (strlen(stSInfo.stServerInfo.strSubAddr)) {
		ser->m_nStreamCnt += 1;
		strncpy(ser->m_strServerStreamName[1], stSInfo.stServerInfo.strSubAddr, MAX_STREAM_NAME);
		
		ser->m_nFps[1] = stSInfo.stServerInfo.nSubFps;
		switch(stSInfo.stServerInfo.enSubStream) {
		case VIDEO_MODE_QVGA:
			ser->m_enStreamMode[1] = STREAM_MODE_VIDEO_QVGA;
			break;
		case VIDEO_MODE_VGA:
			ser->m_enStreamMode[1] = STREAM_MODE_VIDEO_VGA;
			break;
		case VIDEO_MODE_D1:
			ser->m_enStreamMode[1] = STREAM_MODE_VIDEO_D1;
			break;
		case VIDEO_MODE_720P:
			ser->m_enStreamMode[1] = STREAM_MODE_VIDEO_720P;
			break;
		default:
			fprintf(stderr, "sub stream %d mode unsupport\n", stSInfo.stServerInfo.enSubStream);
			ser->m_enStreamMode[1] = STREAM_MODE_MAX;
			break;
		}
	}

	ser->m_nStreamPort = stSInfo.stServerInfo.nRtspPort;
	ser->m_nCommandPort = stSInfo.stServerInfo.nCommandPort;
	ser->m_stImageSet = stSInfo.stServerInfo.stImageSet;
	
	CAutoLock lock(&m_cs);
	pair<map<string, Server*>::iterator, bool> insertRet = m_mapServer.insert(pair<string, Server*>(ser->m_strServerIP, ser));
	if (!(insertRet.second)) {
		fprintf(stderr, "the server %s already been searched\n", ser->m_strServerIP);
		delete ser;
		return 1;
	}
	
	//ser->m_NetCtrl.OpenNetCtrl(ser->m_nCommandPort, 0, ser->m_strServerIP, NET_TYPE_TCP);

	return 0;
}

int CServerSearch::DeleteAllServer()
{
	CAutoLock lock(&m_cs);
	if (m_mapServer.empty()) return 0;

	map<string, Server*>::iterator iterService = m_mapServer.begin();
	for (; iterService != m_mapServer.end(); ++iterService) {
		if (iterService->second)
			delete iterService->second;
	}

	m_mapServer.clear();
	return 0;
}

CServerSearch::Server::Server()
:m_nStreamCnt(0), m_nStreamPort(0), m_nCommandPort(0), m_bIsPrivacyAreaComplete(FALSE)
{
	ZeroMemory(m_strServerID, sizeof(m_strServerID));
	ZeroMemory(m_strServerIP, sizeof(m_strServerIP));
	ZeroMemory(m_strServerStreamName, sizeof(m_strServerStreamName));
	ZeroMemory(&m_stImageSet, sizeof(IMAGE_SET));
	memset(m_enStreamMode, 0xff, sizeof(m_enStreamMode));
	
	for (int i = 0; i < MAX_STREAM_CNT; ++i) m_nFps[i] = 30;

	m_NetCtrl.RegisterCommandRespond(this);
	m_NetCtrl.RegisterServerRespond((IServerRespond *)this);
	m_playURL = "";

	m_nSensitivity = 0;
	m_nMDAreas = 0;

	m_bIsServerRespond = TRUE;

	m_pTalkKickOutCB = NULL;
	m_pTalkKickOutCBP = NULL;

	m_pFileListAddCB = NULL;
	m_pFileListAddCBP = NULL;
}


CServerSearch::Server::~Server()
{
	m_FilesList.clear();
	m_NetCtrl.CloseNetCtrl();
}

int CServerSearch::Server::GetServerID(char * pstrServerID, unsigned int * pnIDlen)
{
	if (*pnIDlen < strlen(m_strServerID) || pstrServerID == NULL) return -1;
	
	CAutoLock lock(&m_cs);

	*pnIDlen = strlen(m_strServerID);
	strncpy(pstrServerID, m_strServerID, *pnIDlen);

	return 0;
}

int CServerSearch::Server::GetServerIp(char * pstrServerIP, unsigned int * pnIPLen)
{
	if (*pnIPLen < strlen(m_strServerIP) || pstrServerIP == NULL) return -1;
	
	*pnIPLen = strlen(m_strServerIP);
	strncpy(pstrServerIP, m_strServerIP, *pnIPLen);

	return 0;
}

int CServerSearch::Server::GetServerIp(unsigned int & nIP)
{
	nIP = inet_addr(m_strServerIP);
	return 0;
}

int CServerSearch::Server::GetServerStreamPort(unsigned int & nPort)
{
	CAutoLock lock(&m_cs);
	nPort = m_nStreamPort;
	return 0;
}

int CServerSearch::Server::GetServerCommandPort(unsigned int & nPort)
{
	nPort = m_nCommandPort;
	return 0;
}

int CServerSearch::Server::GetServerStreamCnt(unsigned int & nCnt)
{
	nCnt = m_nStreamCnt;
	return 0;
}

int CServerSearch::Server::GetServerStreamMode(unsigned int nStreamNum, STREAMMODE & enStreamMode)
{
	if (nStreamNum > MAX_STREAM_CNT) return -1;
	
	CAutoLock lock(&m_cs);
	enStreamMode = m_enStreamMode[nStreamNum];
	return 0;
}

int CServerSearch::Server::GetServerStreamName(unsigned int nStreamNum, char * pstrStreamName, unsigned int * pnNameLen)
{
	if (nStreamNum > MAX_STREAM_CNT) return -1;
	if (*pnNameLen < strlen(m_strServerStreamName[nStreamNum])) return -1;
	
	*pnNameLen = strlen(m_strServerStreamName[nStreamNum]);
	strncpy(pstrStreamName, m_strServerStreamName[nStreamNum], *pnNameLen);
	return 0;
}

int CServerSearch::Server::GetServerImageSet(IMAGE_SET & stServerImageSet)
{
	CAutoLock lock(&m_cs);
	stServerImageSet = m_stImageSet;
	return 0;
}

int CServerSearch::Server::GetServerFileDir(string & strDir)
{
	strDir = m_fileDirectory;
	return 0;
}

int CServerSearch::Server::GetStreamFps(unsigned int nStreamNum, int & nFps)
{
	if (nStreamNum > MAX_STREAM_CNT) return -1;
	CAutoLock lock(&m_cs);

	nFps = m_nFps[nStreamNum];
	return 0;
}

int CServerSearch::Server::GetServerRespondComplete(BOOL & bIsRespond)
{
	bIsRespond = m_bIsServerRespond;
	return 0;
}

int CServerSearch::Server::Connect()
{
	return m_NetCtrl.OpenNetCtrl(m_nCommandPort, 0, m_strServerIP, NET_TYPE_TCP);
}

int CServerSearch::Server::DisConnect()
{
	return m_NetCtrl.CloseNetCtrl();
}

int CServerSearch::Server::IsDisConnect()
{
	return m_NetCtrl.IsDisConnect();
}

int	CServerSearch::Server::SendImageSet(IMAGE_SET & stImageSet)
{
	int ret = m_NetCtrl.SendImageSet(stImageSet);
	if (ret == 0){
		CAutoLock lock(&m_cs);
		m_stImageSet = stImageSet;
	}
	return ret;
}

int	CServerSearch::Server::SendGetPrivacyArea(unsigned int nPrivacyAreaNum)
{
	m_bIsPrivacyAreaComplete = FALSE;
	int ret = m_NetCtrl.SendGetPrivacyArea(nPrivacyAreaNum);
	return ret;
}

int	CServerSearch::Server::SendSetPrivacyArea(PRIVACY_AREA * stAreas, int nAreasCnt)
{
	int ret = m_NetCtrl.SendSetPrivacyArea(stAreas, nAreasCnt);
	
	if (ret != 0) return ret;
	
	PRIVACY_AREA stDeleteAreaContent;
	ZeroMemory(&stDeleteAreaContent, sizeof(PRIVACY_AREA));

	CAutoLock lock(&m_cs4PAreas);

	for (int i = 0; i < nAreasCnt; ++i) {
		vector<PRIVACY_AREA>::iterator iter = m_vecPrivacyArea.begin();
		for (; iter != m_vecPrivacyArea.end(); ++iter) {
			if (iter->nNumber == stAreas[i].nNumber) {
				stDeleteAreaContent.nNumber = stAreas[i].nNumber;
				if (memcmp(stAreas + i, &stDeleteAreaContent, sizeof(PRIVACY_AREA)) == 0)
					iter = m_vecPrivacyArea.erase(iter);
				else
					*iter = stAreas[i];
			}

			break;
		}

		if (iter == m_vecPrivacyArea.end()) {
			stDeleteAreaContent.nNumber = stAreas[i].nNumber;
			if (memcmp(stAreas + i, &stDeleteAreaContent, sizeof(PRIVACY_AREA)) != 0)
				m_vecPrivacyArea.push_back(stAreas[i]);
		}
	}

	return 0;
}

int CServerSearch::Server::SendTakePic()
{
	int ret = m_NetCtrl.SendTakePic();
	return ret;
}

int CServerSearch::Server::SendRecode()
{
	int ret = m_NetCtrl.SendRecode();
	return ret;
}

int CServerSearch::Server::SendStopRecode()
{
	return m_NetCtrl.SendStopRecode();
}

int CServerSearch::Server::SendZoomInOut(uint8_t nZoom)
{
	int ret = m_NetCtrl.SendZoomInOut(nZoom);
	return ret;
}

int CServerSearch::Server::SendGetMotionDetectedAreas()
{
	m_nSensitivity = 0;
	int ret = m_NetCtrl.SendGetMotionDetectedAreas();
	return ret;
}

int CServerSearch::Server::SendSetMotionDetectedAreas(MOTION_DETECT & stMotionDetected)
{
	int ret = m_NetCtrl.SendSetMotionDetectedAreas(stMotionDetected);
	if (ret != 0) return ret;

	CAutoLock lock(&m_cs4MDAreas);
	m_nSensitivity = stMotionDetected.nSensitivity;
	m_nMDAreas = stMotionDetected.nMDAreas;

	return 0;
}

int CServerSearch::Server::SendCameraMovement(CAMERAMOVEMENT movement)
{
	int ret = m_NetCtrl.SendCameraMovement(movement);
	return ret;
}

int CServerSearch::Server::SendTalk(AUDIOPARAMETER & stAudioParam)
{
	int ret = m_NetCtrl.SendTalk(stAudioParam);
	return ret;
}

int CServerSearch::Server::SendISPCommand(BYTE * pCommandInfo, int nInfoLen)
{
	return m_NetCtrl.SendISPCommand(pCommandInfo, nInfoLen);
}

int CServerSearch::Server::SendGetFiles()
{	
	{
		CAutoLock lock(&m_cs4FilesListUpdate);
		m_FilesList.clear();
	}

	return m_NetCtrl.SendGetFiles();
}

int CServerSearch::Server::SendVolumeCtrl(VOLUME volumeCtrl)
{
	return m_NetCtrl.SendVolumeCtrl(volumeCtrl);
}

int CServerSearch::Server::SendGetIspInfo(int nInfoType)
{
	return m_NetCtrl.SendGetIspInfo(nInfoType);
}

int CServerSearch::Server::SendGetServerInfo()
{
	m_bIsServerRespond = FALSE;

	time_t t = time(0);
	struct tm * ptNow = NULL;
	
	ptNow = localtime(&t);
	TIME_INFO stSystemTime = {0};

	stSystemTime.nYear = (ptNow->tm_year + 1900) - 1970;
	stSystemTime.nMon = ptNow->tm_mon + 1;
	stSystemTime.nDay = ptNow->tm_mday;
	stSystemTime.nHour = ptNow->tm_hour;
	stSystemTime.nMin = ptNow->tm_min;
	stSystemTime.nSecond = ptNow->tm_sec;

	return m_NetCtrl.Search(stSystemTime);
}

int CServerSearch::Server::GetRespondComplete(int nMainType, int nSubType, BOOL & bIsComplete)
{
	switch(nMainType) {
	case COMM_TYPE_GET_INFO:
		switch(nSubType) {
		case GET_COMM_PRIVACY_AREA:
			bIsComplete = m_bIsPrivacyAreaComplete;
			break;
		default:
			bIsComplete = FALSE;
		}
	default:
		bIsComplete = FALSE;
	}

	return 0;
}

int CServerSearch::Server::GetPrivacyAreaCount(unsigned int & nCount)
{
	CAutoLock lock(&m_cs4PAreas);
	nCount = m_vecPrivacyArea.size();
	return 0;
}

int CServerSearch::Server::GetPrivacyArea(unsigned int nIndex, PRIVACY_AREA & stPrivacyArea)
{
	CAutoLock lock(&m_cs4PAreas);
	
	if ((nIndex >= m_vecPrivacyArea.size()) || (nIndex < 0)) return -1; 

	stPrivacyArea = m_vecPrivacyArea[nIndex];
	return 0;
}

int CServerSearch::Server::PrivacyAreaRespond(PRIVACY_AREA & stPrivacyArea)
{
	CAutoLock lock(&m_cs4PAreas);
	
	if (stPrivacyArea.nNumber == 0) {
		m_bIsPrivacyAreaComplete = TRUE;
		return 0;
	}
	
	PRIVACY_AREA stEmptyArea;
	ZeroMemory(&stEmptyArea, sizeof(PRIVACY_AREA));

	for (unsigned int i = 0; i < m_vecPrivacyArea.size(); ++i) {
		if (m_vecPrivacyArea[i].nNumber == stPrivacyArea.nNumber) {
			m_vecPrivacyArea[i] = stPrivacyArea;
			return 0;
		}
	}

	stEmptyArea.nNumber = stPrivacyArea.nNumber;
	
	if (memcmp(&stEmptyArea, &stPrivacyArea, sizeof(PRIVACY_AREA)) != 0)
		m_vecPrivacyArea.push_back(stPrivacyArea);

	return 0;
}

int CServerSearch::Server::MotionDetectedAreaRespond(MOTION_DETECT & stMotionDetected)
{
	CAutoLock lock(&m_cs4MDAreas);
	m_nSensitivity = stMotionDetected.nSensitivity;
	m_nMDAreas = stMotionDetected.nMDAreas;

	return 0;
}

int CServerSearch::Server::TalkKickOutRespond(unsigned long ulIpAddr, unsigned short usPort)
{
	CAutoLock lock(&m_cs4TalkKickOut);

	if (m_pTalkKickOutCB)
		m_pTalkKickOutCB((IServer*)this, ulIpAddr, usPort, m_pTalkKickOutCBP);
	
	return 0;
}

#define NAME_SEPARATOR			';'
#define STRING_SEPARATOR		0
#define DIRECTORY_SEPARATOR		"[]"

int CServerSearch::Server::FilesList(char * strFilesName, unsigned int nLen)
{
	string strName;
	char * pEnd = strFilesName + nLen, * pWhere = NULL, * pNameStart = strFilesName;
	BOOL bHasSeparator = TRUE;
	BOOL bIsDir = FALSE;

	if (strFilesName[nLen - 1] != STRING_SEPARATOR)
		bHasSeparator = FALSE;
	
	while ((pNameStart < pEnd) && (*pNameStart != STRING_SEPARATOR)) {
		bIsDir = FALSE;
		pWhere = strchr(pNameStart, NAME_SEPARATOR);

		if (pWhere == NULL) {
			if (!bHasSeparator) {
				char * pstrName = new char[pEnd - pNameStart + 1];
				int i = 0;

				for (; i < pEnd - pNameStart; ++i) {
					pstrName[i] = pNameStart[i];
				}
				pstrName[i] = STRING_SEPARATOR;
				strName = pstrName;
			}else {
				strName = pNameStart;
			}

			if (strName.at(0) == DIRECTORY_SEPARATOR[0] && 
				strName.at(strName.length()-1) == DIRECTORY_SEPARATOR[1]) {
					strName.erase(0, 1);
					strName.erase(strName.length()-1, 1);
					bIsDir = TRUE;
			}
			
			if (bIsDir) {
				m_fileDirectory = strName;
				break;
			}

			CAutoLock lock(&m_cs4FilesListUpdate);
			if (m_pFileListAddCB)
				m_pFileListAddCB(&strName, (IServer*)this, m_pFileListAddCBP);

			m_FilesList.push_back(strName);
			break;
		}

		*pWhere = STRING_SEPARATOR;
		strName = pNameStart;
		if (strName.at(0) == DIRECTORY_SEPARATOR[0] && 
			strName.at(strName.length()-1) == DIRECTORY_SEPARATOR[1]) {
				strName.erase(0, 1);
				strName.erase(strName.length()-1, 1);
				bIsDir = TRUE;
		}
	
		*pWhere = NAME_SEPARATOR;

		pNameStart = ++pWhere;

		if (bIsDir) {
			m_fileDirectory = strName;
			continue;
		}

		CAutoLock lock(&m_cs4FilesListUpdate);
		if (m_pFileListAddCB)
			m_pFileListAddCB(&strName, (IServer*)this, m_pFileListAddCBP);

		m_FilesList.push_back(strName);
	}

	return 0;
}

int CServerSearch::Server::IspInfoParamRespond(BYTE * pInfoParam, unsigned int nlen)
{
	CAutoLock lock(&m_cs4Awb);
	if (m_pInfoCB)
		m_pInfoCB((IServer*)this, pInfoParam, nlen, m_pInfoCBP);
	return 0;
}

int CServerSearch::Server::SetCurrentPlayURL(const char * pstrURL)
{
	m_playURL = pstrURL;
	return 0;
}

int CServerSearch::Server::GetCurrentPlayURL(char * pstrURL, unsigned int & nStrLen)
{
	if (nStrLen < m_playURL.length() + 1) return -1;

	if (m_playURL.empty()) {
		nStrLen = 0;
		return 0;
	}
	
	nStrLen = m_playURL.length() + 1;
	memcpy(pstrURL, m_playURL.c_str(), nStrLen);
	return 0;
}

int CServerSearch::Server::GetMotionDetectedSensitivity(unsigned int & nSensitivity)
{
	nSensitivity = m_nSensitivity;
	return 0;
}

int CServerSearch::Server::GetMotionDetectedAreas(uint64_t & nMDAreas)
{
	CAutoLock lock(&m_cs4MDAreas);
	nMDAreas = m_nMDAreas;

	return 0;
}

int CServerSearch::Server::SetTalkKickOutCallBack(TALKKICKOUTCB pTalkKickOutCB, void * pParam)
{
	CAutoLock lock(&m_cs4TalkKickOut);
	m_pTalkKickOutCB = pTalkKickOutCB;
	m_pTalkKickOutCBP = pParam;
	return 0;
}

int CServerSearch::Server::SetFileListAddCallBack(FILELISTADDCB pFileListAddCB, void * pParam)
{
	CAutoLock lock(&m_cs4FilesListUpdate);
	m_pFileListAddCB = pFileListAddCB;
	m_pFileListAddCBP = pParam;
	return 0;
}

int CServerSearch::Server::SetIspInfoParamCallBack(ISPINFOPARAMCB pIspInfoCB, void * pParam)
{
	CAutoLock lock(&m_cs4Awb);
	m_pInfoCB = pIspInfoCB;
	m_pInfoCBP = pParam;
	return 0;
}

int CServerSearch::Server::ServerRespond(SERVER_INFOEX & stSInfo)
{
	CAutoLock lock(&m_cs);

	strncpy(m_strServerID, stSInfo.stServerInfo.strDeviceID, MAX_ID_LEN);
	
	if (strlen(stSInfo.stServerInfo.strMainAddr)) {
		m_nStreamCnt += 1;
		strncpy(m_strServerStreamName[0], stSInfo.stServerInfo.strMainAddr, MAX_STREAM_NAME);
		m_nFps[0] = stSInfo.stServerInfo.nMainFps;

		switch(stSInfo.stServerInfo.enMainStream) {
		case VIDEO_MODE_QVGA:
			m_enStreamMode[0] = STREAM_MODE_VIDEO_QVGA;
			break;
		case VIDEO_MODE_VGA:
			m_enStreamMode[0] = STREAM_MODE_VIDEO_VGA;
			break;
		case VIDEO_MODE_D1:
			m_enStreamMode[0] = STREAM_MODE_VIDEO_D1;
			break;
		case VIDEO_MODE_720P:
			m_enStreamMode[0] = STREAM_MODE_VIDEO_720P;
			break;
		default:
			fprintf(stderr, "main stream %d mode unsupport\n", stSInfo.stServerInfo.enMainStream);
			m_enStreamMode[0] = STREAM_MODE_MAX;
			break;
		}
	}

	if (strlen(stSInfo.stServerInfo.strSubAddr)) {
		m_nStreamCnt += 1;
		strncpy(m_strServerStreamName[1], stSInfo.stServerInfo.strSubAddr, MAX_STREAM_NAME);
		
		m_nFps[1] = stSInfo.stServerInfo.nSubFps;
		switch(stSInfo.stServerInfo.enSubStream) {
		case VIDEO_MODE_QVGA:
			m_enStreamMode[1] = STREAM_MODE_VIDEO_QVGA;
			break;
		case VIDEO_MODE_VGA:
			m_enStreamMode[1] = STREAM_MODE_VIDEO_VGA;
			break;
		case VIDEO_MODE_D1:
			m_enStreamMode[1] = STREAM_MODE_VIDEO_D1;
			break;
		case VIDEO_MODE_720P:
			m_enStreamMode[1] = STREAM_MODE_VIDEO_720P;
			break;
		default:
			fprintf(stderr, "sub stream %d mode unsupport\n", stSInfo.stServerInfo.enSubStream);
			m_enStreamMode[1] = STREAM_MODE_MAX;
			break;
		}
	}

	m_nStreamPort = stSInfo.stServerInfo.nRtspPort;
	m_nCommandPort = stSInfo.stServerInfo.nCommandPort;
	m_stImageSet = stSInfo.stServerInfo.stImageSet;

	m_bIsServerRespond = TRUE;

	return 0;
}

int CServerSearch::Server::ServerReturnInfo(RETINFO & stRetInfo)
{
	CAutoLock lock(&m_cs4SRet);
	if (m_pServerRetCB)
		m_pServerRetCB((IServer*)this, &stRetInfo, m_pServerRetCBP);

	return 0;
}

int CServerSearch::Server::SetServerRetCallBack(SERVERRETCB pServerRetCB, void * pParam)
{
	CAutoLock lock(&m_cs4SRet);
	m_pServerRetCB = pServerRetCB;
	m_pServerRetCBP = pParam;

	return 0;
}


int CServerSearch::InitUdpSock()
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

int CServerSearch::InitSock()
{
	m_nRecvErrorCnt = 0;
	m_bDisConnect = FALSE;
	if (m_NetType == NET_TYPE_UDP) return InitUdpSock();
	else return -1;
}

unsigned int CServerSearch::GetBroadcastIp()
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


int CServerSearch::OpenSearch()
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


//Search命令发送
int CServerSearch::IPC_Search(TIME_INFO & SystemTime)
{
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

int CServerSearch::Broadcast_Search()
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
	return IPC_Search(stSystemTime);
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


int CServerSearch::anyka_utf8_to_unicode(char* pInput, char* ppOutput)
{
	int outputSize = 0; 

	memset(ppOutput, 0, strlen(pInput) * 2);
	char *tmp = ppOutput; //临时变量，用于遍历输出字符串

	while (*pInput)
	{
		if (*pInput > 0x00 && *pInput <= 0x7F) //处理单字节UTF8字符（英文字母、数字）
		{
			*tmp = *pInput;
			tmp++;
			*tmp = 0; //小端法表示，在高地址填补0
		}
		else if (((*pInput) & 0xE0) == 0xC0) //处理双字节UTF8字符
		{
			char high = *pInput;
			pInput++;
			char low = *pInput;            if ((low & 0xC0) != 0x80)  //检查是否为合法的UTF8字符表示
			{
				return -1; //如果不是则报错
			}

			*tmp = (high << 6) + (low & 0x3F);
			tmp++;
			*tmp = (high >> 2) & 0x07;
		}
		else if (((*pInput) & 0xF0) == 0xE0) //处理三字节UTF8字符
		{
			char high = *pInput;
			pInput++;
			char middle = *pInput;
			pInput++;
			char low = *pInput;           
			if (((middle & 0xC0) != 0x80) || ((low & 0xC0) != 0x80))
			{
				return -1;
			}            
			*tmp = (middle << 6) + (low & 0x7F);
			tmp++;
			*tmp = (high << 4) + ((middle >> 2) & 0x0F); 
		}
		else //对于其他字节数的UTF8字符不进行处理
		{
			return -1;
		}        
		pInput ++;
		tmp ++;
		outputSize += 2;
	}    
	*tmp = 0;
	tmp++;
	*tmp = 0;    
	return outputSize;
} 


void CServerSearch::Parse_Date(char *buf)
{
	UINT buf_size = 0;
	UINT pos_len = 0;
	UINT i = 0, len = 0;
	UINT idex = 0;
	UINT starpos = 0;
	UINT endpos = 0;
	TCHAR buffer[MAX_PATH] = {0};
	char temp_buf[MAX_PATH] = {0};
	char diver_name[MAX_PATH] = {0};
	//int  chPtr[MAX_PATH] = {0};

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

			len = _tcslen(A2T(temp_buf));
			if (idex == 0) //（获取通道名）获取UID
			{
				//
				//UTF8StrToUnicode(temp_buf, strlen(temp_buf), buffer, MAX_PATH);
				//_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_channel_name, buffer);
				_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_UID,  A2T(temp_buf), len);

			}
			else if(idex == 1) //ip
			{
				//_tcscpy(m_ip_address_buf[m_ip_address_idex], A2T(temp_buf));
				_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_buffer,  A2T(temp_buf), len);
			}
			else if (idex == 2)//掩码
			{
				//_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_ch_buffer,  A2T(temp_buf));
			}
			else if (idex == 3)//网关
			{
				//_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_wg_buffer,  A2T(temp_buf));
			}
			else if (idex == 4)//（DNS1）MAC地址
			{
				//_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_dns1_buffer,  A2T(temp_buf));
				_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_MAC,  A2T(temp_buf), len);
			}
			else if (idex == 5)//（DNS2）版本号
			{
				//_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_dns2_buffer,  A2T(temp_buf));
				_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_version,  A2T(temp_buf), len);
			}
			else if (idex == 6)//（DNS2）设备名
			{
				//_tcscpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_dns2_buffer,  A2T(temp_buf));
				UTF8StrToUnicode(temp_buf, strlen(temp_buf), buffer, MAX_PATH);
				_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_diver_name,  buffer, len);
			}
			else if (idex == 7)//设备名
			{
				//anyka_utf8_to_unicode(temp_buf, diver_name);
				//_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_diver_name,  A2T(temp_buf), len);

			}
			else if (idex == 8)//版本号
			{
				
				//_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_version,  A2T(temp_buf), len);
			}
			else if (idex == 9)//MAC
			{
				//_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_MAC,  A2T(temp_buf), len);

			}
			else if (idex == 10)//UID
			{
				//_tcsncpy(g_test_config.m_current_config[m_ip_address_idex].Current_IP_UID,  A2T(temp_buf), len);

			}
			else
			{
				AfxMessageBox(_T("接收到的数据是错误的")); 
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
				//memset(temp_buf, 0, MAX_PATH);
				//strncpy(temp_buf, &buf[starpos], pos_len);
				//memcpy(&g_test_config.m_current_config[m_ip_address_idex].Current_IP_address_dhcp_flag,  temp_buf, 1);
			}
		}
	}
	m_ip_address_idex++;
}


int CServerSearch::search_IP_main()
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

				UINT64 diff = m_nLastMsSinceStart - nStartTime;
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
			Parse_Date(data);


			m_clock.GetMsSinceStart(nStartTime);
		}
	}

End:
	delete[] data;
	return 0;
}



DWORD WINAPI search_IP_thread(LPVOID lpParameter)
{
	T_CURRENT_IP_CONFIG m_temp_config[MAX_PATH] = {0};
	T_CURRENT_IP_CONFIG m_new_config[MAX_PATH] = {0};

	CServerSearch search;

	UINT src_ip_address_idex = 0, i = 0, j = 0, last_idex = 0, new_idex = 0;
	BOOL new_flag = FALSE;


	memset(m_temp_config, 0, sizeof(T_CURRENT_IP_CONFIG)*MAX_PATH);
	memset(m_new_config, 0, sizeof(T_CURRENT_IP_CONFIG)*MAX_PATH);

	search.search_IP_main();

	search.close_search_thread();

	if (m_ip_address_idex == 0)
	{
		m_not_find_anyIP = TRUE;
	}

	else
	{
		if (g_test_config.m_last_config[0].Current_IP_address_buffer[0] == 0 
			&& g_test_config.m_last_config[0].Current_IP_address_buffer[1] == 0
			&& g_test_config.m_last_config[0].Current_IP_address_buffer[2] == 0
			&& g_test_config.m_last_config[0].Current_IP_address_buffer[3] == 0)
		{
			memcpy(g_test_config.m_last_config, g_test_config.m_current_config, sizeof(T_CURRENT_IP_CONFIG)*MAX_PATH);
		}
		else
		{
			for (i = 0; i < MAX_PATH; i++)
			{
				if (g_test_config.m_last_config[i].Current_IP_address_buffer[0] == 0 
					&& g_test_config.m_last_config[i].Current_IP_address_buffer[1] == 0
					&& g_test_config.m_last_config[i].Current_IP_address_buffer[2] == 0
					&& g_test_config.m_last_config[i].Current_IP_address_buffer[3] == 0)
				{
					break;
				}
				src_ip_address_idex++;
			}

			for (i = 0; i < src_ip_address_idex; i++)
			{
				for (j = 0; j < m_ip_address_idex; j++)
				{
					if (memcmp(g_test_config.m_last_config[i].Current_IP_address_buffer, g_test_config.m_current_config[j].Current_IP_address_buffer, IP_ADDRE_LEN) == 0)
					{
						memcpy(&m_temp_config[last_idex], &g_test_config.m_current_config[j], sizeof(T_CURRENT_IP_CONFIG));
						last_idex++;
						break;
					}
				}
			}

			if (last_idex < m_ip_address_idex)
			{
				for (i = 0; i < m_ip_address_idex; i++)
				{
					for (j = 0; j < src_ip_address_idex; j++)
					{
						if (memcmp(g_test_config.m_last_config[j].Current_IP_address_buffer, g_test_config.m_current_config[i].Current_IP_address_buffer, IP_ADDRE_LEN) != 0)
						{
							new_flag = TRUE;
						}
						else
						{
							new_flag = FALSE;
							break;
						}
					}
					if (j == src_ip_address_idex && new_flag)
					{
						memcpy(&m_new_config[new_idex], &g_test_config.m_current_config[i], sizeof(T_CURRENT_IP_CONFIG));
						new_idex++;
					}
				}
				memcpy(&m_temp_config[last_idex], m_new_config, new_idex*sizeof(T_CURRENT_IP_CONFIG));
			}

			memset(g_test_config.m_last_config, 0, sizeof(T_CURRENT_IP_CONFIG)*MAX_PATH);
			memcpy(g_test_config.m_last_config, m_temp_config, m_ip_address_idex*sizeof(T_CURRENT_IP_CONFIG));

			//if (m_ip_address_idex < src_ip_address_idex)
			{
				for (i = 0; i < m_ip_address_idex; i++)
				{
					if (_tcsncmp(m_connect_ip, g_test_config.m_last_config[i].Current_IP_address_buffer, IP_ADDRE_LEN) == 0)
					{
						current_ip_idex = i;
						break;
					}
				}

				if (i == m_ip_address_idex)
				{
					current_ip_idex = 0;
				}

				g_first_find_flag = TRUE;
			}

			
		}
	}

	g_sousuo_flag = TRUE;


	return 1;
}


void CServerSearch::close_search_thread() 
{
	if(m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}

bool CServerSearch::search_Start()
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