// NetCtrl.h: interface for the CNetCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETCTRL_H__280D9032_62DE_4686_9C3A_8790E62B6026__INCLUDED_)
#define AFX_NETCTRL_H__280D9032_62DE_4686_9C3A_8790E62B6026__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define TRANS_STACKSIZE		(1024*300)

#define ISP_ATTR_ID_SIZE		2
#define ISP_CMD_TYPE_SIZE		2

#define ISP_PACKET_HEAD_SIZE	(ISP_ATTR_ID_SIZE+ISP_CMD_TYPE_SIZE)
#define ISP_PARM_LEN_SIZE		4

typedef enum {
	//前n个为ISP模块结构体id
	ISP_BB = 0,   			//黑平衡
	ISP_LSC,				//镜头校正
	ISP_RAW_LUT,			//raw gamma
	ISP_NR,					//NR
	ISP_3DNR,				//3DNR

	ISP_GB,					//绿平衡
	ISP_DEMO,				//DEMOSAIC
	ISP_GAMMA,				//GAMMA
	ISP_CCM,				//颜色校正
	ISP_FCS,				//FCS

	ISP_WDR,				//WDR
	//ISP_EDGE,				//EDGE
	ISP_SHARP,				//SHARP
	ISP_SATURATION,			//饱和度
	ISP_CONTRAST,			//对比度

	ISP_RGB2YUV,			//rgb to yuv
	ISP_YUVEFFECT,			//YUV效果
	ISP_DPC,				//坏点校正
	ISP_ZONE_WEIGHT,		//权重系数
	ISP_AF,					//AF

	ISP_WB,					//WB
	ISP_EXP,				//Expsoure
	ISP_MISC,				//杂项
	ISP_Y_GAMMA,			//y gamma
	ISP_HUE,				//hue

	//统计结果
	ISP_3DSTAT,				//3D降噪统计
	ISP_AESTAT,				//AE统计
	ISP_AFSTAT,				//AF统计
	ISP_AWBSTAT,			//AWB统计

	ISP_SENSOR,				//sensor参数

	//下面是一些特殊值、扩展值
	ISP_PARM_CODE,			//按参数编码
	ISP_REGISTER,			//寄存器参数

	ISP_RAW_IMG,			//一帧raw图像数据
	ISP_YUV_IMG,			//一帧yuv图像数据
	ISP_ENCODE_IMG,			//一帧encode图像数据

	ISP_CFG_DATA,			//day cfg data
	
	ISP_HEARTBEAT,			//heartbeat

    ISP_ATTR_TYPE_NUM
} T_ATTR_TYPE;

typedef enum {
    CMD_GET = 0,
	CMD_REPLY,	// get的回复
	CMD_SET,
	CMD_RET,	//set的结果回复
	CMD_GET_TXT,
	CMD_REPLY_TXT,

    CMD_TYPE_NUM
} T_CMD_TYPE;

typedef void (*RECVCB)(short addr_id, short cmd_type, char* pData, int len, void *pParam);

class CNetCtrl  
{
public:	
	bool m_bConnect;
	HANDLE m_recvtask;


	HANDLE m_hBurnThread_rev_data;
	BOOL test_pass_flag;
	BOOL heat_close_flag;
	HANDLE m_thread_heat;
	BOOL g_test_flag;

	
	CNetCtrl();
	virtual ~CNetCtrl();
	bool TcpClientConnect(unsigned long remote_ip, UINT remote_port);
	bool TcpClientClose();
	bool Socket_close();
	bool IsConnected();
	bool SendCommand(short addr_id, short cmd_type, char* pData, int len);
	int SetRecvCallBack(RECVCB pCB, void *pParam);
	int Socket_Send( char* strData, int iLen );//数据发送函数； 
	BOOL creat_socket(char *lIPAddress, UINT remote_port);


	BOOL Heat_Socket_Create(void);  
	BOOL Heat_Socket_Close(void); //关闭Socket对象； 
	int Heat_Socket_Connect(char *lIPAddress, unsigned int iPort); //定义连接函数； 
	int Heat_Socket_Bind(char* strIP, unsigned int iPort); //绑定函数； 
	int Heat_Socket_Send( char* strData, int iLen ); //数据发送函数； 
	int Heat_Socket_Receive(SOCKET hSocket_heat,  char* strData, int iLen ); //数据接收函数； 

	BOOL create_thread_heat(char *ipaddr, UINT net_uPort);
	void close_thread_heat(); 

	SOCKET m_hSocket_heat;
	SOCKADDR_IN m_sockaddr;
	SOCKADDR_IN m_rsockaddr;

	SOCKET m_tcp_client;
	RECVCB m_pRecvCB;
	void *m_pRecvCBP;
};

#endif // !defined(AFX_NETCTRL_H__280D9032_62DE_4686_9C3A_8790E62B6026__INCLUDED_)
