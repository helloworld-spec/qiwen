
/* ***** ajhdjhajkdja  ***   */




//****************************************************************************
//
//! \addtogroup getting_started_ap
//! @{
//
//****************************************************************************

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
// Simplelink includes
#include "simplelink.h"

// driverlib includes 
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "wdt.h"


// free_rtos/ti-rtos includes 
#include "osi.h"

// common interface includes
#include "common.h"
#include "network_if.h"
#include "wdt_if.h"


//middleware includes
#include "cc_types.h"
#include "rtc_hal.h"
#include "gpio_hal.h"
#include "uart_drv.h"
#include "cc_timer.h"
#include "cc_pm_ops.h"
#include "cc_pm.h"


#ifndef NOTERM
#include "uart_if.h"
#endif
#include "spi_drv.h"
#include "pinmux.h"

//danale lib include
#include "debug.h"
#include  "dana_sock.h"
#include "dana_mem.h"
#include "dana_task.h"
#include "dana_debug.h"

#include "ak_pub_def.h"
#include "ak_system_init.h"
#include "ak_uart.h"
#include "ak_spi.h"

#include "ak_ocv.h"


#define NET_DATA_USE_UDP        0

#define APP_NAME                "WLAN Camview"
#define APPLICATION_VERSION     "1.1.1"
#define OSI_STACK_SIZE          2048
#define OSI_STACK_SIZE_TEST     1024

//#define USER_INPUT_ENABLE 

#define IP_ADDR            0xc0a80166 /* 192.168.1.102 */
#define PORT_NUM           5005
#define BUF_SIZE           1000
#define UDP_PACKET_COUNT   1000
#define CMD_BUF_SIZE       (1024+24)
#define MAX_SSID_LEN	   32
#define MAX_KEY_LEN		   64


#define MAX_TRANSFER_SIZE  1024 //(30*1024) // max is 65536/2!!!

#define FRAME_HEADER_MAGIC    "FRAM" //0x4652414D  //"FRAM"
#define FRAME_END_MAGIC       "FEND" //0x46454E44  //"FEND"

#define CMD_HEADER_MAGIC      "VCMD" //0x56434D44  //"VCMD"
#define CMD_END_MAGIC         "CEND" //0x43454E44  //"CEND"

#define CMD_VIDEO_ARG       0x01
#define CMD_VIDEO_START     0x02
#define CMD_VIDEO_STOP      0x03


// audio recv
#if 1
#define AUDIO_RECV_BUF_SIZE         (1048)
#define AUDIO_RECV_BUF_NUM          16

#define AUDIO_RECV_BUF_EMPTY        0
#define AUDIO_RECV_BUF_READY        1
#define AUDIO_RECV_BUF_USE          2

typedef struct audio_recv_buf{
    unsigned char data[AUDIO_RECV_BUF_SIZE];
    unsigned int time;
    unsigned int len;
    int cnt;
    unsigned char state;
}audio_recv_buf_t;

typedef struct audio_recv_ctrl {
    bool wait_recv_ok;
        
    // cyc buf ctrl
    unsigned int num;
    unsigned int index;
    audio_recv_buf_t buf[AUDIO_RECV_BUF_NUM];
}audio_recv_ctrl_t;
audio_recv_ctrl_t audioRecv;
#else 
char netRecvBuf[CMD_BUF_SIZE];
static int      recv_cnt = 0;
#endif




extern volatile unsigned char ak_start_flg;

cc_hndl adcStartDelayTimerHndl = NULL;      // timer delay befor adc start


#define USER_FILE_NAME  "profile.cfg"



#define FOREVER            1

#define MIN(a,b) ((a > b)? b : a)

// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    SOCKET_CREATE_ERROR = -0x7D0,
    BIND_ERROR = SOCKET_CREATE_ERROR - 1,
    CONNECT_ERROR = BIND_ERROR -1,
    SEND_ERROR = CONNECT_ERROR - 1,
    RECV_ERROR = SEND_ERROR -1,
    SOCKET_CLOSE = RECV_ERROR -1,  
    DEVICE_NOT_IN_STATION_MODE = SOCKET_CLOSE - 1,
    LAN_CONNECTION_FAILED = DEVICE_NOT_IN_STATION_MODE -1,        
    CLIENT_CONNECTION_FAILED = LAN_CONNECTION_FAILED - 1,
    
    FILE_ALREADY_EXIST = CLIENT_CONNECTION_FAILED -1,
    FILE_CLOSE_ERROR = FILE_ALREADY_EXIST - 1,
    FILE_NOT_MATCHED = FILE_CLOSE_ERROR - 1,
    FILE_OPEN_READ_FAILED = FILE_NOT_MATCHED - 1,
    FILE_OPEN_WRITE_FAILED = FILE_OPEN_READ_FAILED -1,
    FILE_READ_FAILED = FILE_OPEN_WRITE_FAILED - 1,
    FILE_WRITE_FAILED = FILE_READ_FAILED - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


//
// Values for below macros shall be modified for setting the 'Ping' properties
//
#define PING_INTERVAL       1000    /* In msecs */
#define PING_TIMEOUT        3000    /* In msecs */
#define PING_PKT_SIZE       20      /* In bytes */
#define NO_OF_ATTEMPTS      3
#define PING_FLAG           0

//
// Values for below macros shall be modified as per access-point(AP) properties
// SimpleLink device will connect to following AP when application is executed
//
#define SPAWN_TASK_PRIORITY     9

#define APP_UDP_PORT            5001    
#define LPDS_DUR_SEC            30
#define LPDS_DUR_NSEC           0
#define FOREVER                 1

#define SL_STOP_TIMEOUT         30
#define CONNECTION_RETRIES      5

#define WD_PERIOD_MS                10000
#define MAP_SysCtlClockGet          80000000
#define MILLISECONDS_TO_TICKS(ms)   ((MAP_SysCtlClockGet/1000) * (ms))

#define CLEAR_CFG  4


enum ap_events{
    EVENT_CONNECTION = 0x1,
    EVENT_DISCONNECTION = 0x2,
    EVENT_IP_ACQUIRED = 0x4,
    WDOG_EXPIRED = 0x8,
    CONNECTION_FAILED = 0x10
};


#ifdef DEBUG_GPIO
cc_hndl tGPIODbgHndl;
#endif

OsiMsgQ_t g_tWkupSignalQueue;
OsiMsgQ_t g_tCommSignalQueue;

cc_hndl g_tUartHndl;
unsigned char g_ucFeedWatchdog = 0;
unsigned char g_ucWdogCount = 0;
OsiMsgQ_t g_tConnection = 0;
char g_cErrBuff[100];

int g_iSocket = 0;

static unsigned char g_socket_state = 0;

int g_iSocket_udp = 0;



unsigned int g_iSleepAllow = 0;

static int frame_crc = 0;

extern void lp3p0_setup_power_policy(int power_policy);
extern int platform_init();

void idle_test( );
void wakeup_loop( );



// Loop forever, user can change it as per application's requirement
#define LOOP_FOREVER() \
            {\
                while(1); \
            }

// check the error code and handle it
#define ASSERT_ON_ERROR(error_code)\
            {\
                 if(error_code < 0) \
                   {\
                        sprintf(g_cErrBuff,"Error [%d] at line [%d] in "\
                                "function [%s]", error_code,__LINE__,\
                                __FUNCTION__);\
                        UART_PRINT(g_cErrBuff);\
                        UART_PRINT("\n\r");\
                        return error_code;\
                 }\
            }



//#define dbg(...) UART_PRINT(__VA_ARGS__)



//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
extern unsigned char  	g_ulStatus ;
extern unsigned long  	g_ulStaIp ;
extern unsigned long  	g_ulIpAddr;
unsigned long  			g_ulPingPacketsRecv = 0;
unsigned long  			g_uiGatewayIP = 0;

char 					g_dstIpStr[32] = {0};
char 					g_dstPortStr[32] = {0} ;
char					g_packetCountStr[32] = {0} ;


unsigned long  			g_ulDestinationIp = IP_ADDR;        // Client IP address
unsigned short   		g_uiPortNum = PORT_NUM;

volatile unsigned long  g_ulPacketCount = UDP_PACKET_COUNT;
unsigned char  			g_ucSimplelinkstarted = 0;
char 					g_cBsdBuf[BUF_SIZE];

signed char 			g_ssid_name[MAX_SSID_LEN +1];
signed char 			g_security_key[MAX_KEY_LEN +1];


OsiSyncObj_t g_NetconnectSyncObj = NULL;
OsiSyncObj_t g_NetSendSyncObj = NULL;

OsiSyncObj_t g_NetrecvSyncObj = NULL;
OsiSyncObj_t g_socketerrorSyncObj = NULL;








OsiSyncObj_t  spi_read_sem = NULL;

volatile unsigned char spi_trans = 0;

volatile unsigned long ad_average = 0;


struct sys_cfg
{
	unsigned char ssid[MAX_SSID_LEN];
	unsigned char key[MAX_KEY_LEN];
	unsigned int  server_ip;
	unsigned int  port_num;
}g_sys_cfg;


#if defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif


//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static int PingTest(unsigned long ulIpAddr);
static long ConfigureSimpleLinkToDefaultState();
static void InitializeAppVariables();
static int GetSsidName(char *pcSsidName, unsigned int uiMaxLen);

unsigned int IpAddressParser(char *ucCMD);
void WlanStaMode(void * pvParameters );
void WlanAPMode(void * pvParameters );


#ifdef USE_FREERTOS
//*****************************************************************************
// FreeRTOS User Hook Functions enabled in FreeRTOSConfig.h
//*****************************************************************************
//*****************************************************************************
//
//! \brief Application defined idle task hook
//! 
//! \param  none
//! 
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook( void)
{
    //Handle Idle Hook for Profiling, Power Management etc
   
    if(g_iSleepAllow)
    {
		cc_idle_task_pm();
    }
}




#endif //USE_FREERTOS

//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
	long lRetVal = -1;
    if(pSock == NULL)
    {
        return;
    }

    //
    // This application doesn't work w/ socket - Events are not expected
    //
    switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->socketAsyncEvent.SockTxFailData.status)
            {
                case SL_ECLOSE: 
                    UART_PRINT("[SOCK ERROR] - close socket (%d) operation "
                                "failed to transmit all queued packets\r\n", 
                                    pSock->socketAsyncEvent.SockTxFailData.sd);
                    break;
                default: 
                    UART_PRINT("[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                                "(%d) \r\n",
                                pSock->socketAsyncEvent.SockTxFailData.sd, pSock->socketAsyncEvent.SockTxFailData.status);
#if 1			
								if(g_socket_state)
								{
									lRetVal = osi_SyncObjSignal(&g_socketerrorSyncObj);
									if(lRetVal < 0)
									{
										UART_PRINT("realse SyncObjSignal g_socketerrorSyncObj error!\r\n");
										return ;
									}
									//g_socket_state = 0;
								}
#endif

				  break;
            }

			break;

        case SL_SOCKET_ASYNC_EVENT:

        	 switch(pSock->socketAsyncEvent.SockAsyncData.type)
        	 {
        	 case SSL_ACCEPT:/*accept failed due to ssl issue ( tcp pass)*/
        		 UART_PRINT("[SOCK ERROR] - close socket (%d) operation"
        				 	 "accept failed due to ssl issue\n\r",
        				 	 pSock->socketAsyncEvent.SockAsyncData.sd);
                 break;
        	 case RX_FRAGMENTATION_TOO_BIG:
        		 UART_PRINT("[SOCK ERROR] -close scoket (%d) operation"
							 "connection less mode, rx packet fragmentation\n\r"
        				 	 "> 16K, packet is being released",
							 pSock->socketAsyncEvent.SockAsyncData.sd);
                 break;
        	 case OTHER_SIDE_CLOSE_SSL_DATA_NOT_ENCRYPTED:
        		 UART_PRINT("[SOCK ERROR] -close socket (%d) operation"
        				 	 "remote side down from secure to unsecure\n\r",
        		 			pSock->socketAsyncEvent.SockAsyncData.sd);
                 break;
        	 default:
        		 UART_PRINT("unknown sock async event: %d\n\r",
        				 	 pSock->socketAsyncEvent.SockAsyncData.type);
        	 }
        	break;
        default:
        	UART_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
          break;
    }
}

static void RebootMCU()
{

  //
  // Configure hibernate RTC wakeup
  //
  PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);

  //
  // Delay loop
  //
  MAP_UtilsDelay(8000000);

  //
  // Set wake up time
  //
  PRCMHibernateIntervalSet(330);

  //
  // Request hibernate
  //
  PRCMHibernateEnter();

  //
  // Control should never reach here
  //
  while(1)
  {

  }
}

void key_press_cb(void)
{
#if 0
	// unsigned char queue_msg = CLEAR_CFG;
	// UART_PRINT("key int !\r\n");
	// UART_PRINT("clear_config irq\n\r");
   	 //osi_MsgQWrite(&g_tCommSignalQueue, &queue_msg, OSI_NO_WAIT);
#else
	unsigned char axp_vol;
   	 axp_vol = axp_ocv_restcap();
	Report("axp_vol: %d\r\n", axp_vol);
#endif	
}


void clear_config(void)
{
	UART_PRINT("clear_config....\n\r");
	memset((char*)&g_sys_cfg, 0, sizeof(struct sys_cfg));
	WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
	RebootMCU();
	
}

void msg_process_task(void * pvParameters )
{   
    
    int iTestResult = 0;
    unsigned char ucDHCP;
    long lRetVal = -1;
	unsigned char ucQueueMsg = 0;
  
	
	
	//
    // Queue management related configurations
    //
    lRetVal = osi_MsgQCreate(&g_tCommSignalQueue, NULL,
                             sizeof( unsigned char ), 10);
    if (lRetVal < 0)
    {
        UART_PRINT("unable to create the msg queue\n\r");
        LOOP_FOREVER();
    }
	  
    while(FOREVER)
    {
        //
        // waits for the message from the various interrupt handlers(GPIO,
        // Timer) and the UDPServerTask.
        //
        osi_MsgQRead(&g_tCommSignalQueue, &ucQueueMsg, OSI_WAIT_FOREVER);
        switch(ucQueueMsg){
       		case CLEAR_CFG:
				UART_PRINT("clear config irq\n\r");
				clear_config();
	        default:
	            UART_PRINT("invalid msg\n\r");
	            break;
        }
    }

}


//*****************************************************************************
//
//!  This funtion includes the following steps:
//!  -open a user file for writing
//!  -write wlan ssid password and server ip port
//!  -close the user file
//!
//!  /param[out] ulToken : file token
//!  /param[out] lFileHandle : file handle
//!
//!  /return  0:Success, -ve: failure
//
//*****************************************************************************
int WriteProfile(unsigned char* profile_data, unsigned int len)
{
    long lRetVal = -1;
    unsigned long ulToken;
	long lFileHandle;
	long write_len = -1;
    //
    //  create a user file
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                FS_MODE_OPEN_CREATE(65536, \
                          _FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        //
        // File may already be created
        //
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);
    }
    else
    {
        write_len = sl_FsWrite(lFileHandle, 0, profile_data, len);
        if (write_len < 0)
        {
        	UART_PRINT("write profile failed\n\r");
            lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
            ASSERT_ON_ERROR(FILE_WRITE_FAILED);
        }
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	    if (SL_RET_CODE_OK != lRetVal)
	    {
	        ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
	    }
    }
	UART_PRINT("write profile success\n\r");
    return write_len;
}

//*****************************************************************************
//
//!  This funtion includes the following steps:
//!  -open a user file for writing
//!  -write wlan ssid password and server ip port
//!  -close the user file
//!
//!  /param[out] ulToken : file token
//!  /param[out] lFileHandle : file handle
//!
//!  /return  0:Success, -ve: failure
//
//*****************************************************************************
int ReadProfile(unsigned char* profile_data, unsigned int len)
{
    long lRetVal = -1;
    unsigned long ulToken;
	long lFileHandle;
	long read_len;

    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                        FS_MODE_OPEN_READ,
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(FILE_OPEN_READ_FAILED);
    }

    //
    // read the data and compare with the stored buffer
    //
    
    read_len = sl_FsRead(lFileHandle, 0, profile_data, len);
    if ((read_len < 0))
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(FILE_READ_FAILED);
    }
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    if (SL_RET_CODE_OK != lRetVal)
    {
        ASSERT_ON_ERROR(FILE_CLOSE_ERROR);
    }
	return read_len;
    
}

//****************************************************************************
//
//! \brief get user input system config with wifi and server
//!
//! This function get user input wifi ssid and key and video server
//!    
//!
//! \return    0 on success, -1 on Error.
//
//****************************************************************************
int SysConfig()
{
    int 			iRetVal;
	
	UART_PRINT("Please enter the SSID name to connect: ");
	GetSsidName(g_ssid_name, MAX_SSID_LEN);
	UART_PRINT("Enter the key: ");
	GetSsidName(g_security_key, MAX_KEY_LEN);

  	do
  	{
		UART_PRINT("Enter the dest ip addr: ");
		GetSsidName(g_dstIpStr,18);
  	}while(IpAddressParser(g_dstIpStr) == 0);
	do
	{
		UART_PRINT("Enter the dest port: ");
		GetSsidName(g_dstPortStr,6);
		g_uiPortNum = strtoul(g_dstPortStr, 0, 10);
	}while(g_uiPortNum == 0);

	memset((char*)&g_sys_cfg, 0, sizeof(struct sys_cfg));
	strcpy(g_sys_cfg.ssid, g_ssid_name);
	strcpy(g_sys_cfg.key, g_security_key);
	g_sys_cfg.server_ip = g_ulDestinationIp;
	g_sys_cfg.port_num  = g_uiPortNum;

	iRetVal = WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
	
	return iRetVal;
   
}

int ShowConfig()
{
    int 			iRetVal;

	memset((char*)&g_sys_cfg, 0, sizeof(struct sys_cfg));
	iRetVal = ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
	if(iRetVal != sizeof(struct sys_cfg))
	{
		UART_PRINT("Read profile error\n\r ");
	}
	UART_PRINT("SSID: %s \n\r", g_sys_cfg.ssid);
    UART_PRINT("KEY:  %s \n\r", g_sys_cfg.key);
    UART_PRINT("Server IP: %d.%d.%d.%d \n\r", SL_IPV4_BYTE(g_sys_cfg.server_ip,3),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,2),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,1),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,0));
    UART_PRINT("PORT: %d \n\r", g_sys_cfg.port_num);
    return iRetVal;  
}


//****************************************************************************
//
//! \brief creat a TCP  socket for sending video stream
//!
//! This function opens a UDP socket and tries to connect to a Server IP_ADDR
//!    waiting on port PORT_NUM.
//!    Then the function will send 1000 UDP packets to the server.
//!
//! \param[in]  port number on which the server will be listening on
//!
//! \return    0 on success, -1 on Error.
//
//****************************************************************************
int CreateTcpSocket(int *retSockID)
{
    SlSockAddrIn_t  sAddr;
    int             iAddrSize;
	int             iSockID;
    int             iStatus;
    
	UART_PRINT("Video stream will send to dest IP addr:%d.%d.%d.%d, dest port %d\n\r", 
		        			SL_IPV4_BYTE(g_sys_cfg.server_ip,3),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,2),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,1),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,0),
                            g_sys_cfg.port_num);
#ifdef USER_INPUT_ENABLE
	UART_PRINT("Enter the dest ip addr: ");
	GetCmd(g_dstIpStr,18);
	UART_PRINT("Enter the dest port: ");
	GetCmd(g_dstPortStr,6);
	if(strlen(g_dstIpStr) != 0)
	{
		IpAddressParser(g_dstIpStr);
	}
	
	if(strlen(g_dstPortStr) != 0)
	{
		g_uiPortNum = strtoul(g_dstPortStr, 0, 10);
	}
#endif	
	g_ulDestinationIp = g_sys_cfg.server_ip;
    g_uiPortNum = g_sys_cfg.port_num;

	if(0 == g_ulDestinationIp)
	{
		//use self addr + 1
		//g_ulDestinationIp = g_ulIpAddr + 1;
		g_ulDestinationIp = IP_ADDR;
	}
	if(0 == g_uiPortNum)
	{
		g_uiPortNum = PORT_NUM;
	}
	
	//filling the TCP server socket address
   sAddr.sin_family = SL_AF_INET;
   sAddr.sin_port = sl_Htons((unsigned short)g_uiPortNum);
   sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)g_ulDestinationIp);

   iAddrSize = sizeof(SlSockAddrIn_t);
   if(0 != *retSockID)
   {
       sl_Close(*retSockID);	
   }
   // creating a TCP socket
   iSockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
   if( iSockID < 0 )
   {
	   ASSERT_ON_ERROR(SOCKET_CREATE_ERROR);
   }
   
   // connecting to TCP server
   do

   {
	   iStatus = sl_Connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize);
	   if( iStatus < 0 )
	   {
		   // error
		   UART_PRINT("connect to  %d.%d.%d.%d port %d error\n\r",
		   						SL_IPV4_BYTE(g_ulDestinationIp,3),
	                            SL_IPV4_BYTE(g_ulDestinationIp,2),
	                            SL_IPV4_BYTE(g_ulDestinationIp,1),
	                            SL_IPV4_BYTE(g_ulDestinationIp,0),
	                            g_uiPortNum);
		   /*wait 1 sec*/	
		   MAP_UtilsDelay(8000000);
       }
	}while(iStatus < 0);	
	
	UART_PRINT("socket ID: %d\n\r", iSockID);
	*retSockID = iSockID;
	
    return SUCCESS;
}


//****************************************************************************
//
//! \brief creat a udp  socket for sending video stream
//!
//! This function opens a UDP socket and tries to connect to a Server IP_ADDR
//!    waiting on port PORT_NUM.
//!    Then the function will send 1000 UDP packets to the server.
//!
//! \param[in]  port number on which the server will be listening on
//!
//! \return    0 on success, -1 on Error.
//
//****************************************************************************
int CreateUdpSocket(int *retSockID)
{
        int             iSockID;
    
	UART_PRINT("Video stream will send to dest IP addr:%d.%d.%d.%d, dest port %d\n\r", 
		        			SL_IPV4_BYTE(g_sys_cfg.server_ip,3),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,2),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,1),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,0),
                            g_sys_cfg.port_num);

#ifdef USER_INPUT_ENABLE
	UART_PRINT("Enter the dest ip addr: ");
	GetCmd(g_dstIpStr,18);
	UART_PRINT("Enter the dest port: ");
	GetCmd(g_dstPortStr,6);
#endif	
	if(strlen(g_dstIpStr) != 0)
	{
		IpAddressParser(g_dstIpStr);
	}
	else
	{
		//use self addr + 1
		g_ulDestinationIp = g_ulIpAddr + 1;
	}
	if(strlen(g_dstPortStr) != 0)
	{
		g_uiPortNum = strtoul(g_dstPortStr, 0, 10);
	}
    
    g_ulDestinationIp = g_sys_cfg.server_ip;
    g_uiPortNum = g_sys_cfg.port_num;

    if(0 == g_ulDestinationIp)
    {
        //use self addr + 1
        //g_ulDestinationIp = g_ulIpAddr + 1;
        g_ulDestinationIp = IP_ADDR;
    }
    if(0 == g_uiPortNum)
    {
        g_uiPortNum = PORT_NUM;
    }
    

    // creating a UDP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    if( iSockID < 0 )
    {
        // error
        ASSERT_ON_ERROR(SOCKET_CREATE_ERROR);
    }
	UART_PRINT("socket ID: %d\n\r", iSockID);

	*retSockID = iSockID;
    return SUCCESS;
}


//****************************************************************************
//
//! \brief Opening a UDP client side socket and sending data
//!
//! This function opens a UDP socket and tries to connect to a Server IP_ADDR
//!    waiting on port PORT_NUM.
//!    Then the function will send 1000 UDP packets to the server.
//!
//! \param[in]  port number on which the server will be listening on
//!
//! \return    0 on success, -1 on Error.
//
//****************************************************************************
int BsdUdpClient()
{
    int             iCounter;
    short           sTestBufLen;
    SlSockAddrIn_t  sAddr;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    unsigned long   lLoopCount = 0;
	char            sleepStr[8], sizeStr[8];
	int 			iSleep, iSize;
	
	UART_PRINT("default dest IP addr:%d.%d.%d.%d, dest port %d, packet count %d\n\r", 
		        			SL_IPV4_BYTE(g_ulIpAddr,3),
                            SL_IPV4_BYTE(g_ulIpAddr,2),
                            SL_IPV4_BYTE(g_ulIpAddr,1),
                            SL_IPV4_BYTE(g_ulIpAddr,0) + 1,
                            g_uiPortNum,
                            g_ulPacketCount);
#ifdef USER_INPUT_ENABLE
	UART_PRINT("Enter the dest ip addr: ");
	GetCmd(g_dstIpStr,18);
	UART_PRINT("Enter the dest port: ");
	GetCmd(g_dstPortStr,6);
	UART_PRINT("Enter pkg size: ");
	GetCmd(sizeStr, 8);
	UART_PRINT("Enter packet count to send: ");
	GetCmd(g_packetCountStr,10);
	UART_PRINT("Enter sleep ms: ");
	GetCmd(sleepStr, 8);
#endif
	/*IP addr*/
	if(strlen(g_dstIpStr) != 0)
	{
		IpAddressParser(g_dstIpStr);
	}
	else
	{
		//use self addr + 1
		g_ulDestinationIp = g_ulIpAddr + 1;
	}
	/*port number*/
	if(strlen(g_dstPortStr) != 0)
	{
		g_uiPortNum = strtoul(g_dstPortStr, 0, 10);
	}
	/*packet count*/
	if(strlen(g_packetCountStr) != 0)
	{
		g_ulPacketCount = strtoul(g_packetCountStr, 0, 10);
	}
	/*pkg size*/
	if(strlen(sizeStr) != 0)
	{
		iSize = strtoul(sizeStr, 0, 10);
	}
	else
	{
		iSize = 1460;
	}
	/*interval*/
	if(strlen(sleepStr) != 0)
	{
		iSleep = strtoul(sleepStr, 0, 10);
	}
	else
	{
		iSleep = 10;
	}
	
	
    // filling the buffer
    for (iCounter = 0 ; iCounter < BUF_SIZE ; iCounter++)
    {
        g_cBsdBuf[iCounter] = 0x61 + iCounter % 26; //from a to z;
    }
	

	//filling the UDP server socket address
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons((unsigned short)g_uiPortNum);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)g_ulDestinationIp);

    iAddrSize = sizeof(SlSockAddrIn_t);

    // creating a UDP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    if( iSockID < 0 )
    {
        // error
        ASSERT_ON_ERROR(SOCKET_CREATE_ERROR);
    }
	UART_PRINT("udp socket %d \n\r", iSockID);
Resend:

	sTestBufLen = MIN(iSize, BUF_SIZE);
    // for a UDP connection connect is not required
    // sending 1000 packets to the UDP server
    UART_PRINT("begin Send UDP packets to %d.%d.%d.%d : %d, pkg size %d, count: %d, interval:%d\n\r", 
                            SL_IPV4_BYTE(g_ulDestinationIp,3),
                            SL_IPV4_BYTE(g_ulDestinationIp,2),
                            SL_IPV4_BYTE(g_ulDestinationIp,1),
                            SL_IPV4_BYTE(g_ulDestinationIp,0),
                            g_uiPortNum, sTestBufLen, g_ulPacketCount, iSleep);

	
	lLoopCount = 0;
    while (lLoopCount < g_ulPacketCount)
    {
        
		// sending packet
        iStatus = sl_SendTo(iSockID, g_cBsdBuf, sTestBufLen, 0,
                                (SlSockAddr_t *)&sAddr, iAddrSize);
        if( iStatus <= 0 )
        {
            // error
            sl_Close(iSockID);
            ASSERT_ON_ERROR(SEND_ERROR);
        }
        lLoopCount++;
		osi_Sleep(iSleep);
    }

    UART_PRINT("Sent %u packets successfully\n\r",lLoopCount);
	UART_PRINT("Enter packet count resend:");
	GetCmd(g_packetCountStr,10);
	if(strlen(g_packetCountStr) != 0)
	{
		g_ulPacketCount = strtoul(g_packetCountStr, 0, 10);
	}
	if(g_ulPacketCount > 0)
	{
		UART_PRINT("Enter pkg size (Byte):");
		GetCmd(sizeStr,10);
		if(strlen(sizeStr) != 0)
		{
			iSize = strtoul(sizeStr, 0, 10);
		}
		else
		{
			iSize = 1460;
		}
		UART_PRINT("Enter send interval (ms):");
		GetCmd(sleepStr,10);
		if(strlen(sleepStr) != 0)
		{
			iSleep = strtoul(sleepStr, 0, 10);
		}
		else
		{
			iSleep = 10;
		}
	
		goto Resend;
	}
	

    //closing the socket after sending 1000 packets
    sl_Close(iSockID);

    return SUCCESS;
}


//****************************************************************************
//
//! \brief cmd loop for control 
//!
//! This function opens a UDP socket in Listen mode and waits for an incoming
//! UDP connection.
//!    If a socket connection is established then the function will try to
//!    read 1000 UDP packets from the connected client.
//!
//! \param[in]          port number on which the server will be listening on
//!
//! \return             0 on success, Negative value on Error.
//
//****************************************************************************
int CmdLoop(void)
{
	
    char cmdStr[CMD_BUF_SIZE];
    CMD_INFO cmd_uart ;

    
	while(FOREVER)
	{
		Message(">");
		GetCmd(cmdStr, sizeof(cmdStr));
		if(strlen(cmdStr) == 0)
		{
			continue;
		}
		
		/*parse cmd */
		
		if(strstr(cmdStr, AK_CMD_POWER_ON))
		{
			power_ak(AK_POWER_ON);
		}
		else if(strstr(cmdStr, AK_CMD_POWER_OFF))
		{
			power_ak(AK_POWER_OFF);
		}
		else if(strstr(cmdStr, AK_CMD_START_VIDEO))
		{
			cmd_uart.cmd_id = CMD_VIDEO_PREVIEW;
			send_cmd(&cmd_uart);	
		}
		else if(strstr(cmdStr, AK_CMD_STOP_VIDEO))
		{
			cmd_uart.cmd_id = CMD_VIDEO_PREVIEW_END;
			send_cmd(&cmd_uart);
		}
		else if(strstr(cmdStr, AK_CMD_WIFI_TX_TEST))
		{
			BsdUdpClient();  //no memery to put all this in
		}
		else if(strstr(cmdStr, AK_CMD_CONFIG))
		{
			SysConfig();
		}
		else if(strstr(cmdStr, AK_CMD_CONFIG_SHOW))
		{
			ShowConfig();
		}
		else if(strstr(cmdStr, AK_CMD_REBOOT))
		{
			RebootMCU();
		}
		else
		{
			UART_PRINT("unknow cmd \n\r");
		}
	
	}

    return SUCCESS;
}

//*****************************************************************************
//
//! This function opens a TCP socket in Listen mode and waits for an incoming 
//! TCP connection.. If a socket connection is established then the function 
//! will try to read 1000 TCP packets from the connected client.
//!
//! \param port number on which the server will be listening on
//!
//! \return  0 on success, -ve on Error.
//!
//*****************************************************************************
void NetCmdTcpServer(void * pvParameters)
{
    SlSockAddrIn_t  sRemoteAddr;
    SlSockAddrIn_t  sLocalAddr;
    int             indexCount,iAddrSize,iSockID,iStatus,newSockID;
    long            LoopCount = 0;
    long            nonBlocking = 1;
    SlTimeval_t     timeVal;

	int             iCounter;
	CMD_INFO cmd_uart ;
	char cmd_buf[CMD_BUF_SIZE] = {0};
	char *optPtr;
	int port;
	int iRetVal;

	UART_PRINT("Net cmd TCP server IP addr:%d.%d.%d.%d, listen PORT %d \n\r", 
		        			SL_IPV4_BYTE(g_ulIpAddr,3),
                            SL_IPV4_BYTE(g_ulIpAddr,2),
                            SL_IPV4_BYTE(g_ulIpAddr,1),
                            SL_IPV4_BYTE(g_ulIpAddr,0),
                            g_uiPortNum
                            );
    
    sLocalAddr.sin_family = SL_AF_INET;
    sLocalAddr.sin_port = sl_Htons((unsigned short)g_uiPortNum);
    sLocalAddr.sin_addr.s_addr = 0;
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
	
    if( iSockID < 0 )
    {
        // error
        UART_PRINT("create socket error\n\r");
	    return;
    }

    iAddrSize = sizeof(SlSockAddrIn_t);
    iStatus = sl_Bind(iSockID, (SlSockAddr_t *)&sLocalAddr, iAddrSize);

    if( iStatus < 0 )
    {
        // error
        sl_Close(iSockID);
        UART_PRINT("sl_Bind socket error\n\r");
	    return;
    }

    iStatus = sl_Listen(iSockID, 0);
    if( iStatus < 0 )
    {
         // error
        sl_Close(iSockID);
        UART_PRINT("sl_Listen error\n\r");
	    return;
    }

    iStatus = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_NONBLOCKING, \
                            &nonBlocking,
                            sizeof(nonBlocking));
    
    newSockID = SL_EAGAIN;
    while( newSockID < 0 )
    {
        newSockID = sl_Accept(iSockID, ( struct SlSockAddr_t *)&sRemoteAddr, 
                                        (SlSocklen_t*)&iAddrSize);
        if( newSockID == SL_EAGAIN )
        {
                MAP_UtilsDelay(80000);
        }
        else if( newSockID < 0 )
        {
            sl_Close(iSockID);
			UART_PRINT("sl_Accept error\n\r");
            return;
        }
    }
	#if 0
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;
	
    if( newSockID >= 0 )
    {
        iStatus = sl_SetSockOpt(newSockID, SL_SOL_SOCKET, SL_SO_RCVTIMEO,
                                &timeVal, sizeof(timeVal));
        UART_PRINT("sl_SetSockOpt SL_SO_RCVTIMEO error\n\r");
	    sl_Close(newSockID);
        sl_Close(iSockID);
        return ;
    }
    #endif
	// filling the buffer
    for (iCounter = 0 ; iCounter < CMD_BUF_SIZE ; iCounter++)
    {
        cmd_buf[iCounter] = 0;
    }
	
    while (FOREVER)
    {
        
		iStatus = sl_Recv(newSockID, cmd_buf, CMD_BUF_SIZE, 0);

	    if( iStatus <= 0 )
	    {
	        // error
	        UART_PRINT("sl_Recv error\n\r");
	        sl_Close(newSockID);
            sl_Close(iSockID);
            return ;
	    }
	    
		/*parse cmd from network*/
		//only support 16 charactor cmd
		
		cmd_buf[iStatus] = 0;
		
		UART_PRINT("cmd recv: %s\n\r", cmd_buf);
		
		if(strstr(cmd_buf, AK_CMD_POWER_ON))
		{
			power_ak(AK_POWER_ON);
		}
		else if(strstr(cmd_buf, AK_CMD_POWER_OFF))
		{
			power_ak(AK_POWER_OFF);
		}
		else if(strstr(cmd_buf, AK_CMD_START_VIDEO))
		{
			cmd_uart.cmd_id = CMD_VIDEO_PREVIEW;
			send_cmd(&cmd_uart);//	
		}
		else if(strstr(cmd_buf, AK_CMD_STOP_VIDEO))
		{
			cmd_uart.cmd_id = CMD_VIDEO_PREVIEW_END;
			send_cmd(&cmd_uart);	
		}
		else if(strstr(cmd_buf, AK_CMD_REBOOT))
		{
			RebootMCU();
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_SET_SSID))
		{
			if(strlen(optPtr) > strlen(AK_CMD_SET_SSID) && strlen(optPtr) <= strlen(AK_CMD_SET_SSID) + MAX_SSID_LEN)
			{
				ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				strcpy(g_sys_cfg.ssid, strstr(cmd_buf, AK_CMD_SET_SSID) + strlen(AK_CMD_SET_SSID));
				iRetVal = WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				if(iRetVal > 0)
				{
					char *result = "Set ssid OK\n";
					/*send back set result*/
					iStatus = sl_Send(newSockID, result, strlen(result), 0);
					
				}
			}
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_SET_KEY))
		{
			if(strlen(optPtr) > strlen(AK_CMD_SET_KEY) && strlen(optPtr) <= strlen(AK_CMD_SET_KEY) + MAX_KEY_LEN)
			{
				ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				strcpy(g_sys_cfg.key, strstr(cmd_buf, AK_CMD_SET_KEY) + strlen(AK_CMD_SET_KEY));
				iRetVal = WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				if(iRetVal > 0)
				{
					char *result = "Set key OK\n";
					/*send back set result*/
					iStatus = sl_Send(newSockID, result, strlen(result), 0);
					
				}
			}
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_SET_SERVER))
		{
			strcpy(g_dstIpStr, strstr(cmd_buf, AK_CMD_SET_SERVER) + strlen(AK_CMD_SET_SERVER));
			if(0 != IpAddressParser(g_dstIpStr))
			{
				ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				g_sys_cfg.server_ip = g_ulDestinationIp;
				iRetVal = WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				if(iRetVal > 0)
				{
					char *result = "Set server OK\n";
					/*send back set result*/
					iStatus = sl_Send(newSockID, result, strlen(result), 0);
					
				}
			}
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_SET_PORT))
		{
			strcpy(g_dstPortStr, strstr(cmd_buf, AK_CMD_SET_PORT) + strlen(AK_CMD_SET_PORT));
			port = strtoul(g_dstPortStr, 0, 10);
			if(port > 0 && port < 65536)
			{
				ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
			    g_sys_cfg.port_num = port;
				iRetVal = WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				if(iRetVal > 0)
				{
					char *result = "Set port OK\n";
					/*send back set result*/
					iStatus = sl_Send(newSockID, result, strlen(result), 0);
					
				}
			}
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_CONFIG_SHOW))
		{
			char result[CMD_BUF_SIZE] = {0};
			memset((char*)&g_sys_cfg, 0, sizeof(struct sys_cfg));
			iRetVal = ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
			if(iRetVal != sizeof(struct sys_cfg))
			{
				UART_PRINT("Read profile error\n\r ");
			}
			sprintf(result, "SSID: %s \n", g_sys_cfg.ssid);
    		sprintf(result + strlen(result), "KEY:  %s \n", g_sys_cfg.key);
    		sprintf(result + strlen(result), "Server IP: %d.%d.%d.%d \n", SL_IPV4_BYTE(g_sys_cfg.server_ip,3),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,2),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,1),
                            SL_IPV4_BYTE(g_sys_cfg.server_ip,0));
    		sprintf(result + strlen(result), "PORT: %d \n", g_sys_cfg.port_num);
			
			iStatus = sl_Send(newSockID, result, strlen(result), 0);
			
		}
		else if(strstr(cmd_buf, AK_CMD_CLEAR_CFG))
		{
			clear_config();
		}
		else
		{
			UART_PRINT("no desired cmd\n\r");
		}
			
    }

    sl_Close(iSockID);
   
}


//****************************************************************************
//
//! \brief Opening a UDP server side socket and receiving data
//!
//! This function opens a UDP socket in Listen mode and waits for an incoming
//! UDP connection.
//!    If a socket connection is established then the function will try to
//!    read 1000 UDP packets from the connected client.
//!
//! \param[in]          port number on which the server will be listening on
//!
//! \return             0 on success, Negative value on Error.
//
//****************************************************************************
void NetCmdUdpServer(void * pvParameters)
{
    SlSockAddrIn_t  sAddr;
    SlSockAddrIn_t  sLocalAddr;
    int             iCounter;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    short           sTestBufLen;

	CMD_INFO cmd_uart ;
	char cmd_buf[CMD_BUF_SIZE] = {0};
	char *optPtr;
	int port;

    

	UART_PRINT("Net cmd UDP server IP addr:%d.%d.%d.%d, listen PORT %d \n\r", 
		        			SL_IPV4_BYTE(g_ulIpAddr,3),
                            SL_IPV4_BYTE(g_ulIpAddr,2),
                            SL_IPV4_BYTE(g_ulIpAddr,1),
                            SL_IPV4_BYTE(g_ulIpAddr,0),
                            g_uiPortNum
                            );
    sTestBufLen  = CMD_BUF_SIZE;
    //filling the UDP server socket address
    sLocalAddr.sin_family = SL_AF_INET;
    sLocalAddr.sin_port = sl_Htons((unsigned short)g_uiPortNum);
    sLocalAddr.sin_addr.s_addr = 0;

    iAddrSize = sizeof(SlSockAddrIn_t);

    // creating a UDP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    if( iSockID < 0 )
    {
        // error
        UART_PRINT("create socket error\n\r");
	    return;
    }

    // binding the UDP socket to the UDP server address
    iStatus = sl_Bind(iSockID, (SlSockAddr_t *)&sLocalAddr, iAddrSize);
    if( iStatus < 0 )
    {
        // error
        sl_Close(iSockID);
        UART_PRINT("bind socket error\n\r");
	    return;
    }
	// filling the buffer
    for (iCounter = 0 ; iCounter < CMD_BUF_SIZE ; iCounter++)
    {
        cmd_buf[iCounter] = 0;
    }
    // no listen or accept is required as UDP is connectionless protocol
    /// waits for 1000 packets from a UDP client
    //while (lLoopCount < g_ulPacketCount)
    while (FOREVER)
    {
        iStatus = sl_RecvFrom(iSockID, cmd_buf, sTestBufLen, 0,
                     ( SlSockAddr_t *)&sAddr, (SlSocklen_t*)&iAddrSize );

	    if( iStatus < 0 )
	    {
	        // error
	        sl_Close(iSockID);
	        UART_PRINT("receiv from socket error\n\r");
	        return;
	    }
	    
		/*parse cmd from network*/
		//only support 16 charactor cmd
		
		cmd_buf[iStatus] = 0;
		
		UART_PRINT("cmd recv: %s\n\r", cmd_buf);
		
		if(strstr(cmd_buf, AK_CMD_POWER_ON))
		{
			power_ak(AK_POWER_ON);
		}
		else if(strstr(cmd_buf, AK_CMD_POWER_OFF))
		{
			power_ak(AK_POWER_OFF);
		}
		else if(strstr(cmd_buf, AK_CMD_START_VIDEO))
		{
			cmd_uart.cmd_id = CMD_VIDEO_PREVIEW;
			send_cmd(&cmd_uart);//	
		}
		else if(strstr(cmd_buf, AK_CMD_STOP_VIDEO))
		{
			cmd_uart.cmd_id = CMD_VIDEO_PREVIEW_END;
			send_cmd(&cmd_uart);	
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_SET_SSID))
		{
			if(strlen(optPtr) > strlen(AK_CMD_SET_SSID) && strlen(optPtr) <= strlen(AK_CMD_SET_SSID) + MAX_SSID_LEN)
			{
				ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				strcpy(g_sys_cfg.ssid, strstr(cmd_buf, AK_CMD_SET_SSID) + strlen(AK_CMD_SET_SSID));
				WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
			}
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_SET_KEY))
		{
			if(strlen(optPtr) > strlen(AK_CMD_SET_KEY) && strlen(optPtr) <= strlen(AK_CMD_SET_KEY) + MAX_KEY_LEN)
			{
				ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				strcpy(g_sys_cfg.key, strstr(cmd_buf, AK_CMD_SET_KEY) + strlen(AK_CMD_SET_KEY));
				WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
			}
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_SET_SERVER))
		{
			strcpy(g_dstIpStr, strstr(cmd_buf, AK_CMD_SET_SERVER) + strlen(AK_CMD_SET_SERVER));
			if(0 != IpAddressParser(g_dstIpStr))
			{
				ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
				g_sys_cfg.server_ip = g_ulDestinationIp;
				WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
			}
		}
		else if(optPtr = strstr(cmd_buf, AK_CMD_SET_PORT))
		{
			strcpy(g_dstPortStr, strstr(cmd_buf, AK_CMD_SET_PORT) + strlen(AK_CMD_SET_PORT));
			port = strtoul(g_dstPortStr, 0, 10);
			if(port > 0 && port < 65536)
			{
				ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
			    g_sys_cfg.port_num = port;
				WriteProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
			}
		}
		else if(strstr(cmd_buf, AK_CMD_CLEAR_CFG))
		{
			clear_config();
		}
		else
		{
			UART_PRINT("no desired cmd\n\r");
		}
			
    }

    sl_Close(iSockID);

}

int ak_tcp_recv(int  fd, char *recv_buf, int len, long timeout_usec)
{
    long ret;
    if (0 == timeout_usec) {
        return sl_Recv(fd, recv_buf, len, SL_SO_NONBLOCKING);
    }

    long timeout_left = timeout_usec;
    struct timeval delay;
    while(len > 0) {
        mem_set(&delay, 0, sizeof(struct timeval));
        delay.tv_sec = timeout_left / 1000000;
        delay.tv_usec = timeout_left % 1000000;
        fd_set fds;
        SL_FD_ZERO(&fds);
		SL_FD_SET(fd, &fds);
		if(g_socket_state)
		{
	        	ret = sl_Select(fd + 1, &fds, NULL, NULL, &delay);
	        if(ret < 0) {
				Report("select error!\r\n");
	            return -1;
	        } else if(ret == 0){
				return 0;
	        }
			else
			{
	            if(SL_FD_ISSET(fd, &fds))
				{
	                ret = sl_Recv(fd, recv_buf, len, SL_SO_NONBLOCKING);
					Report("recv %d\r\n",ret);
	           //     if(SL_EAGAIN == ret) {
				//		osi_Sleep(100);
				//		continue;	
	            //    } 
					if(ret <= 0)
					{
						return -1;
	                }
					else
					{
						return ret;	
	                }
	                
	            } 
				else 
				{
					return -1;
	            }
	        }
		}
		else
		{
			return -1;
		}
    }
    return 0;
}

//****************************************************************************
//
//! \brief Opening a UDP server side socket and receiving data
//!
//! This function opens a UDP socket in Listen mode and waits for an incoming
//! UDP connection.
//!    If a socket connection is established then the function will try to
//!    read 1000 UDP packets from the connected client.
//!
//! \param[in]          port number on which the server will be listening on
//!
//! \return             0 on success, Negative value on Error.
//
//****************************************************************************
void TcpRecvCmdTask(void * pvParameters )//(int iSockID)//
{
	long retval;
	unsigned int 	heart = 0;
    int             iCounter;
    int             iStatus;
    u8				cmd_id, cmd_arg_len;
	int 			i;
	int 			iSockID;
	char			cmd_buf[CMD_BUF_SIZE] = {0};
	unsigned char ucQueueMsg = 3;
	CMD_INFO cmd_uart ;



#if NET_DATA_USE_UDP
        iRetVal = CreateUdpSocket(&g_iSocket_udp);
        if(SUCCESS != iRetVal)
        {
            UART_PRINT("Create Udp Socket error\n\r");
        } else {
            //BsdUdpClient();
        }      
#endif


    memset(&audioRecv,0,sizeof(audio_recv_ctrl_t));
    audioRecv.num = 0;
    audioRecv.index = 0;
    // no listen or accept is required as UDP is connectionless protocol
    /// waits for 1000 packets from a UDP client
    //while (lLoopCount < g_ulPacketCount)
    while(1)
    {
    	retval = osi_SyncObjWait(&g_NetrecvSyncObj, OSI_WAIT_FOREVER);
		if(retval <0 )
		{
			Report("wait g_NetrecvSyncObj error\r\n");
			return ;
		}
    	while (FOREVER)
	    {
	    	
#if 0	    	
	    		iStatus = sl_Recv(g_iSocket, cmd_buf, CMD_BUF_SIZE, 0);
#else			
				iStatus = ak_tcp_recv(g_iSocket, cmd_buf, CMD_BUF_SIZE, 30*1000000);
#endif
			
			
		    if( iStatus < 0 )
		    {
		        // error
		        //sl_Close(g_iSocket);
				UART_PRINT("recv cmd error, %d\n\r", iStatus);
				//g_allowSleep = 0;
				//if(g_socket_state)
				{
					retval = osi_SyncObjSignal(&g_socketerrorSyncObj);
					if(retval < 0)
					{
						UART_PRINT("realse SyncObjSignal g_socketerrorSyncObj error!\r\n");
						return ;
					}
					g_socket_state = 0;
					break;
				}
				//ASSERT_ON_ERROR(RECV_ERROR);
		    }
			else if(iStatus == 0)
			{
				continue;
			}
			else
			{
				Report("recv cmd len %d\r\n",iStatus);
	        
	        	//audio_recv_buf_t *aRecv = &audioRecv.buf[audioRecv.index];
				for(i = 0; i < iStatus; i++)
				{
		            if(cmd_buf[i] == 'V' && cmd_buf[i + 1] == 'C' && cmd_buf[i + 2] == 'M' && cmd_buf[i + 3] == 'D')
		            {
		                i += 4;
		    			/*check crc and cmd end*/
		    			
		    			
		    			/*parse cmd id*/
		    			cmd_id = cmd_buf[i];
		    			i += 1;  /*1 bytes cmd id */
		    			cmd_arg_len = cmd_buf[i];
		    			i += 1;  /*1 bytes cmd arg len*/
		    			UART_PRINT("recv cmd id %d\n\r", cmd_id);
		    			switch(cmd_id)
		    			{
		    				case CMD_VIDEO_ARG:
		    					cmd_uart.cmd_id = CMD_VIDEO_PARAM;
		    					memcpy(&cmd_uart.param, &cmd_buf[i], 4);
		    					send_cmd(&cmd_uart);
		                        //UART_PRINT("video para send\n\r");
		    					break;
		    				case CMD_VIDEO_START:
		                        if(g_iSleepAllow == 1) {
		                            ucQueueMsg = 3;
		                            osi_MsgQWrite(&g_tWkupSignalQueue, &ucQueueMsg, OSI_WAIT_FOREVER);
		                        }
		    					cmd_uart.cmd_id = CMD_VIDEO_PREVIEW;
		    					//send_cmd(&cmd_uart);
		    					break;
		    				case CMD_VIDEO_STOP:
		    					cmd_uart.cmd_id = CMD_VIDEO_PREVIEW_END;
		                        if(g_iSleepAllow == 0) {
		                            ucQueueMsg = 4;
		                            osi_MsgQWrite(&g_tWkupSignalQueue, &ucQueueMsg, OSI_WAIT_FOREVER);
		                        }
		    					//send_cmd(&cmd_uart);
		    					break;
		    				default:
		    					UART_PRINT("unkown cmd\n\r");
		    					break;	
		    			}
		    			//UART_PRINT("i = %d\n\r", i);
		            } 
		            else 
		            {
#if 0
		                if(ak_start_flg) {
		                    if(aRecv->cnt >= CMD_BUF_SIZE ) {
		                        aRecv->cnt = 0;
		                        if(audioRecv.num < AUDIO_RECV_BUF_NUM) {
		                            audioRecv.num++;
		                            audioRecv.index++;
		                            audioRecv.index %= AUDIO_RECV_BUF_NUM; 
		                            aRecv = &audioRecv.buf[audioRecv.index];
		                            aRecv->cnt = 0; // new aRecv->cnt clear
		                            aRecv->data[aRecv->cnt++] = cmd_buf[i];
		                        } else {
		                            UART_PRINT("audio recv buffer flowOver\r\n");
		                        }
		                        
		                        //UART_PRINT("ao send %d\r\n", recv_cnt);
		                        //ak_uart_write(netRecvBuf,recv_cnt);
		                        //recv_cnt= 0;
		                    } else {
		                        aRecv->data[aRecv->cnt++] = cmd_buf[i];
		                    }
		                }
#endif
						continue;
					}
					
		            
				}
			}		
	    }
	}
}
void heart_Task(void * pvParameters )
{
    static int audio_cnt = 0;
	while(1)
	{
		//Report("$\r\n");
		#if 0
        if(audioRecv.num > 0) {
            audio_recv_buf_t *aRecv = &audioRecv.buf[(audioRecv.index+\
                AUDIO_RECV_BUF_NUM-audioRecv.num)%AUDIO_RECV_BUF_NUM];
            
            UART_PRINT("ao send id=%d  num=%d ", audio_cnt,audioRecv.num);
            memcpy(&aRecv->data[1040], &audio_cnt, 4);
            //int i;
            //for(i = 0; i < 8; i++)
            //    Report("%0x ", aRecv->data[i]);
            //for(i = 0; i < 4; i++)
            //    Report("%0x ", aRecv->data[i+1044]);
            UART_PRINT("\r\n");
            
            ak_uart_write(&aRecv->data[0],CMD_BUF_SIZE);
            audioRecv.num--;
            aRecv->cnt = 0;

            audio_cnt++;
            
        }
        #endif

        Report("heart \r\n");
		osi_Sleep(5000);
	}
}


int SendVideoStream(char *video_buf,unsigned int data_len)
{
	
    char         *pBuf = NULL;
	char         *optPtr = NULL;
	//unsigned int data_len=0;
	unsigned int iFrame=0,timestamp=0;
	int             iStatus;
	SlSockAddrIn_t  sAddr;
    int             iAddrSize;
	volatile int 			transfer_progress,transfer_size;
	
	
	//filling the UDP server socket address
	sAddr.sin_family = SL_AF_INET;
	sAddr.sin_port = sl_Htons((unsigned short)g_uiPortNum);
	sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)g_ulDestinationIp);

	iAddrSize = sizeof(SlSockAddrIn_t);

    pBuf		= video_buf;
#if 0
	frame_crc++;
	//get data_len iFrame timestamp
	data_len   = *(unsigned int*)video_buf;
	iFrame     = *(unsigned int*)(video_buf+4);
	timestamp  = *(unsigned int*)(video_buf+8);

	/*convert header for PC camview tool*/
	/*4 "FRAM" + 4 frame len + frame + 4 CRC + 4"FEND"*/
	
	pBuf		= video_buf + 4;
	
	optPtr		= video_buf + 4;
	memcpy(optPtr, FRAME_HEADER_MAGIC, 4);

	optPtr      += 4;
	memcpy(optPtr, &data_len, 4);

	optPtr      += 4; 
	optPtr      += data_len ;
	memcpy(optPtr, &frame_crc, 4); /*crc not check yet*/

	optPtr		+= 4;
	memcpy(optPtr, FRAME_END_MAGIC, 4);

	data_len 	+= 16; /* frame len + 4 frame header and 4 frame end*/
#endif
	
	/*send data*/
	if(g_iSocket < 0 || (g_iSocket_udp < 0))
	{
		UART_PRINT("socekt has not created\n\r");
		return FAILURE;
	}
	else
	{	
		//UART_PRINT("video frame len %d\n\r", data_len);
		//UDP
		//iStatus = sl_SendTo(g_iSocket, pBuf, data_len, 0, (SlSockAddr_t *)&sAddr, iAddrSize);
		
		for ( transfer_progress = 0; transfer_progress < data_len; transfer_progress += transfer_size, pBuf += transfer_size)
    	{
			transfer_size = MIN( MAX_TRANSFER_SIZE, ( data_len - transfer_progress ) );

         #if NET_DATA_USE_UDP
            //UART_PRINT("udp %d\n\r", transfer_size);
            iStatus = sl_SendTo(g_iSocket_udp, pBuf, transfer_size, 0, (SlSockAddr_t *)&sAddr, iAddrSize);
            //UART_PRINT("udp end\n\r");
         #else  //TCP
#if 1
           UART_PRINT("tcp %d\r\n", transfer_size);
           //int i;
           //for(i = 0; i < 16; i++)
           //  Report("%0x ", pBuf[i]);
           //UART_PRINT("\r\n");
#endif
            if(g_socket_state)
            {
            	iStatus = sl_Send(g_iSocket, pBuf, transfer_size, 0);
            	UART_PRINT("ret %d\r\n", iStatus);
            }
			else
			{
				return -1;
			}
            //UART_PRINT("tcp end\n\r");
#endif 
			if( iStatus <= 0 )
	        {
	            // error
	            UART_PRINT("sent to %0x error\n\r", g_ulDestinationIp);
	            ASSERT_ON_ERROR(SEND_ERROR);
	        }
            
		}
		//UART_PRINT("sent %d bytes to %0x successfully\n\r", iStatus, g_ulDestinationIp);
		return SUCCESS;
	}

}

void start_ADC_task(void *vParam)
{
	unsigned char i = 0;
	unsigned long ad_value, max_ad_value, min_ad_value;


    if(adcStartDelayTimerHndl != NULL) {
        cc_timer_delete(adcStartDelayTimerHndl);
        adcStartDelayTimerHndl = NULL;
    }
	
	ak_adc_init(3);
	max_ad_value = 0;
	min_ad_value = 2000;
	/*get ADC value*/
	while(i < 9)
	{
		
		ad_value = ak_getValue_adc(3);
		//Report("ad: %d \r\n ", ad_value);
		ak_adc_realse(3);
		if(-1 != ad_value)
		{
			if(i>1)
			{
				ad_average += ad_value;
				if(ad_value > max_ad_value)
					max_ad_value = ad_value;
				if(ad_value < min_ad_value)
					min_ad_value = ad_value;
			}
			i++;
		}
	}
	ad_average = ad_average -max_ad_value -min_ad_value;
	ad_average = ad_average/5;
	//Report("max:%d,min:%d,ad_average : %d \r\n ", max_ad_value, min_ad_value, ad_average);

	//gpio_init(GPIO_IRCUT, 0);
	//gpio_set_pin_dir(GPIO_IRCUT, 1);
	if(ad_average < 1000)
	{
		/*control IRCUT open */
		//gpio_set_pin_level(GPIO_IRCUT, 1);
	}
	else
	{
		/*control IRCUT close */
		//gpio_set_pin_level(GPIO_IRCUT, 0);
	}
}

static unsigned char star_ak39_flag = 0;
void StartAk39e(void)
{
	cc_hndl wake_gpio_hndl;

	
    struct cc_timer_cfg sRealTimeTimer;
    struct u64_time sIntervalTimer;

    
	/*power on ak39e, quicken system start,
	  but uart1 spi1 init should lesser ak39estart time
	*/
     if(-1 == power_ak(1))
	 	return ;
	

	//wake_gpio_hndl = cc_gpio_open(GPIO_POWER_AK, GPIO_DIR_OUTPUT);
	//cc_gpio_write(wake_gpio_hndl, GPIO_POWER_AK, 1);
	//power_ak(AK_POWER_ON);

	/*creat adc mask timer*/
	sRealTimeTimer.source = HW_REALTIME_CLK;
    sRealTimeTimer.timeout_cb = start_ADC_task;
    sRealTimeTimer.cb_param = NULL;

    if(adcStartDelayTimerHndl == NULL) {
        adcStartDelayTimerHndl = cc_timer_create(&sRealTimeTimer);
        if(NULL == adcStartDelayTimerHndl) {
            UART_PRINT("adcStartDelayTimerHndl Null\n\r");
            return ;
        }
        sIntervalTimer.secs = 0;
        sIntervalTimer.nsec = 100000000;
        cc_timer_start(adcStartDelayTimerHndl, &sIntervalTimer, OPT_TIMER_ONE_SHOT);
    }


	/*init UART1 and GSPI*/
	ak_uart1_init();

	//Report("ak_spi_init\n");
	ak_spi_init();
	

	//set gpio 4 as config reset button, press 3 second will reset config 
	//key_open(GPIO_04, 1, 3, key_press_cb);
	if(0 != star_ak39_flag) 
	//osi_TaskCreate(msg_process_task, \
    //                        (const signed char*)"MSG queue", \
    //                        OSI_STACK_SIZE, NULL, 2, NULL );
	
	{
		UART_PRINT("start 39e flag: %d\n\r", star_ak39_flag);
		return ;
	}
	UART_PRINT("start ak39e \n\r");
	star_ak39_flag++;
	
	star_uart1_task();
	start_spi_task();
	

	//set gpio 4 as config reset button, press 3 second will reset config 
	//key_open(GPIO_04, 1, 3, key_press_cb);

	//osi_TaskCreate(msg_process_task, \
    //                        (const signed char*)"MSG queue", \
    //                        OSI_STACK_SIZE, NULL, 2, NULL );
	
    /*power on ak39e*/
    //power_ak(1);
	//g_ak_statu = 1;
    
	UART_PRINT("start ak39e , exit\n\r");
	//BsdUdpClient();
	//CreateUdpSocket(&g_iSocket);
	//CreateTcpSocket(&g_iSocket);
	//TcpRecvCmdTask(g_iSocket);
	//NetCmdUdpServer(PORT_NUM);
	//CmdLoop();
	
}


//*****************************************************************************
//
//! \brief callback function for gpio interrupt handler
//!
//! \param gpio_num is the gpio number which has triggered the interrupt
//!
//! \return 0
//
//*****************************************************************************
void (*key_callback)(unsigned char io_num) = NULL;
int gpio_intr_hndlr(int gpio_num)
{
    unsigned char queue_msg = 2;

   if(GPIO_PIR == gpio_num)
    {
        osi_MsgQWrite(&g_tWkupSignalQueue, &queue_msg, OSI_NO_WAIT);
   }
   else if(GPIO_SPI_SIG == gpio_num)
   {
		spi_trans |= SPI_GPIO_INT_FLG;
		//Report("j");
		osi_SyncObjSignalFromISR(&spi_read_sem);
   }
   else if(GPIO_KEY == gpio_num)
   {
   		//Report("gp30,%x\r\n",key_callback);
		//key_callback(gpio_num);
		gpioKey_function_callback(gpio_num);
		//Report("int\r\n");
   }
   
    return 0;
}

//*****************************************************************************
//
//! \brief callback function for timer interrupt handler
//!
//! \param vParam is a general void pointer (not used here)
//!
//! \return None
//
//*****************************************************************************
void TimerCallback(void *vParam)
{
	
    unsigned char ucQueueMsg = 1;
    //osi_MsgQWrite(&g_tWkupSignalQueue, &ucQueueMsg, OSI_NO_WAIT);
    UART_PRINT("********wake up from timer***\r\n");
	//wakeup_flag = 1;
    
    return;
}

//*****************************************************************************
//
//! \brief set GPIO as a wake up source from low power modes.
//!
//! \param none
//!
//! \return handle for the gpio used
//
//*****************************************************************************
cc_hndl SetGPIOAsWkUp()
{
    cc_hndl tGPIOHndl;
    //
    // setting up GPIO as a wk up source and configuring other related
    // parameters
    //
    tGPIOHndl = cc_gpio_open(GPIO_PIR, GPIO_DIR_INPUT);
    cc_gpio_enable_notification(tGPIOHndl, GPIO_PIR, INT_RISING_EDGE,
                                (GPIO_TYPE_NORMAL | GPIO_TYPE_WAKE_SOURCE));
    return(tGPIOHndl);
}

cc_hndl closeGPIOWkUp()
{
	cc_hndl tGPIOHndl;
    //
    // setting up GPIO as a wk up source and configuring other related
    // parameters
    //
    tGPIOHndl = cc_gpio_open(GPIO_PIR, GPIO_DIR_INPUT);
    cc_gpio_disable_notification(tGPIOHndl, GPIO_PIR);
    return(tGPIOHndl);
}
//*****************************************************************************
//
//! \brief set Timer as a wake up source from low power modes.
//!
//! \param none
//!
//! \return handle for the Timer setup as a wakeup source
//
//*****************************************************************************
cc_hndl SetTimerAsWkUp()
{
    cc_hndl tTimerHndl;
    struct cc_timer_cfg sRealTimeTimer;
    struct u64_time sInitTime, sIntervalTimer;
    //
    // setting up Timer as a wk up source and other timer configurations
    //
    sInitTime.secs = 0;
    sInitTime.nsec = 0;
    cc_rtc_set(&sInitTime);

    sRealTimeTimer.source = HW_REALTIME_CLK;
    sRealTimeTimer.timeout_cb = TimerCallback;
    sRealTimeTimer.cb_param = NULL;

    tTimerHndl = cc_timer_create(&sRealTimeTimer);

    sIntervalTimer.secs = LPDS_DUR_SEC;
    sIntervalTimer.nsec = LPDS_DUR_NSEC;
    cc_timer_start(tTimerHndl, &sIntervalTimer, OPT_TIMER_PERIODIC);
    return(tTimerHndl);
}



//*****************************************************************************
//
//! \brief This function handles ping report events
//!
//! \param[in]     pPingReport - Ping report statistics
//!
//! \return None
//
//****************************************************************************
void SimpleLinkPingReport(SlPingReport_t *pPingReport)
{
    SET_STATUS_BIT(g_ulStatus, STATUS_BIT_PING_DONE);
    g_ulPingPacketsRecv = pPingReport->PacketsReceived;
}


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End
//*****************************************************************************


//****************************************************************************
//
//!    \brief This function initializes the application variables
//!
//!    \param[in]  None
//!
//!    \return     None
//
//****************************************************************************
static void InitializeAppVariables()
{
    g_ulStatus = 0;
    g_ulStaIp = 0;
    g_ulPingPacketsRecv = 0;
    g_uiGatewayIP = 0;
}

//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to STATION
//!           - Configures connection policy to Auto and AutoSmartConfig
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
static long ConfigureSimpleLinkToDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode 
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event 
            // before doing anything 
            while(!IS_IP_ACQUIRED(g_ulStatus))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
#endif
            }
        }

        // Switch to STA role and restart 
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again 
        if (ROLE_STA != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in STA-mode 
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }
    
    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);
    
    UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig 
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, 
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);

    

    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore 
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(g_ulStatus))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask(); 
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 2;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, 
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeAppVariables();
    
    return lRetVal; // Success
}



//****************************************************************************
//
//! Confgiures the mode in which the device will work
//!
//! \param iMode is the current mode of the device
//!
//! This function
//!    1. prompt user for desired configuration and accordingly configure the
//!          networking mode(STA or AP).
//!       2. also give the user the option to configure the ssid name in case of
//!       AP mode.
//!
//! \return sl_start return value(int).
//
//****************************************************************************
static int ConfigureMode(int iMode)
{
    char    pcSsidName[33] = "ak_cc3200";
    long   lRetVal = -1;
	
#ifdef USER_INPUT_ENABLE
    UART_PRINT("Enter the AP SSID name: ");
    GetSsidName(pcSsidName,33);
#endif
	
    lRetVal = sl_WlanSetMode(ROLE_AP);
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID, strlen(pcSsidName),
                            (unsigned char*)pcSsidName);
    ASSERT_ON_ERROR(lRetVal);

    UART_PRINT("Device is configured in AP mode\n\r");

    /* Restart Network processor */
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);

    // reset status bits
    CLR_STATUS_BIT_ALL(g_ulStatus);

    return sl_Start(NULL,NULL,NULL);
}


//****************************************************************************
//
//!    \brief Parse the input IP address from the user
//!
//!    \param[in]                     ucCMD (char pointer to input string)
//!
//!    \return                        ipaddr : if correct IP, -1 : incorrect IP
//
//****************************************************************************
unsigned int IpAddressParser(char *ucCMD)
{
    int i=0;
    unsigned int uiUserInputData;
    unsigned long ulUserIpAddress = 0;
    char *ucInpString;
    ucInpString = strtok(ucCMD, ".");
    uiUserInputData = (int)strtoul(ucInpString,0,10);
    while(i<4)
    {
        //
       // Check Whether IP is valid
       //
       if((ucInpString != NULL) && (uiUserInputData < 256))
       {
           ulUserIpAddress |= uiUserInputData;
           if(i < 3)
               ulUserIpAddress = ulUserIpAddress << 8;
           ucInpString=strtok(NULL,".");
           uiUserInputData = (int)strtoul(ucInpString,0,10);
           i++;
       }
       else
       {
       	   UART_PRINT("IP addr parse error\n\r");	
           return 0;
       }
    }
    g_ulDestinationIp = ulUserIpAddress;
    return ulUserIpAddress;
}

//****************************************************************************
//
//! Get Ssid name form the user over UART
//!
//! \param pcSsidName is a pointer to the array which will contain the ssid name
//!
//! This function
//!    1. gets the ssid name string over uart
//!
//! \return iRetVal is the length of the ssid(user input).
//
//****************************************************************************
static int GetSsidName(char *pcSsidName, unsigned int uiMaxLen)
{
  char ucRecvdAPDetails = 0;
  int  iRetVal = 0;
  char acCmdStore[128];
  do
  {
      ucRecvdAPDetails = 1;

      //
      // Get the AP name to connect over the UART
      //
      iRetVal = GetCmd(acCmdStore, sizeof(acCmdStore));
      if(iRetVal >= 0)
      {
          // remove start/end spaces if any
          iRetVal = TrimSpace(acCmdStore);

          //
          // Parse the AP name
          //
          if(pcSsidName != NULL)
          {
              strncpy(pcSsidName, acCmdStore, iRetVal);
              ucRecvdAPDetails = 1;
              pcSsidName[iRetVal] = '\0';
          }
      }
  }while(ucRecvdAPDetails == 0);

  return(iRetVal);
}


//****************************************************************************
//
//!    \brief device will try to ping the machine that has just connected to the
//!           device.
//!
//!    \param  ulIpAddr is the ip address of the station which has connected to
//!            device
//!
//!    \return 0 if ping is successful, -1 for error
//
//****************************************************************************
static int PingTest(unsigned long ulIpAddr)
{  
    signed long           lRetVal = -1;
    SlPingStartCommand_t PingParams;
    SlPingReport_t PingReport;
    PingParams.PingIntervalTime = PING_INTERVAL;
    PingParams.PingSize = PING_PKT_SIZE;
    PingParams.PingRequestTimeout = PING_TIMEOUT;
    PingParams.TotalNumberOfAttempts = NO_OF_ATTEMPTS;
    PingParams.Flags = PING_FLAG;
    PingParams.Ip = ulIpAddr; /* Cleint's ip address */
    
    UART_PRINT("Running Ping Test...\n\r");
    /* Check for LAN connection */
    lRetVal = sl_NetAppPingStart((SlPingStartCommand_t*)&PingParams, SL_AF_INET,
                            (SlPingReport_t*)&PingReport, NULL);
    ASSERT_ON_ERROR(lRetVal);

    g_ulPingPacketsRecv = PingReport.PacketsReceived;

    if (g_ulPingPacketsRecv > 0 && g_ulPingPacketsRecv <= NO_OF_ATTEMPTS)
    {
      // LAN connection is successful
      UART_PRINT("Ping Test successful\n\r");
    }
    else
    {
        // Problem with LAN connection
        ASSERT_ON_ERROR(LAN_CONNECTION_FAILED);
    }

    return SUCCESS;
}

static long WlanConnect()
{
    SlSecParams_t secParams = {0};
    long lRetVal = 0;
		        			
#ifdef USER_INPUT_ENABLE
	UART_PRINT("default SSID:%s, key: %s\r\n", SSID_NAME, SECURITY_KEY);
	UART_PRINT("Enter the SSID name to connect: ");
	GetSsidName(g_ssid_name,33);
	UART_PRINT("Enter the key: ");
	GetSsidName(g_security_key,65);
#endif

    strcpy(g_ssid_name, g_sys_cfg.ssid);
    strcpy(g_security_key, g_sys_cfg.key);
	
    if (0 == strlen(g_ssid_name))
	{	
		strcpy(g_ssid_name, SSID_NAME);
		strcpy(g_security_key, SECURITY_KEY);
    }
        
    secParams.Key = (signed char *)g_security_key;
    secParams.KeyLen = strlen(g_security_key);
	if(secParams.KeyLen > 0)
	{
    	secParams.Type = SL_SEC_TYPE_WPA;
	}
	else
	{
		secParams.Type = SL_SEC_TYPE_OPEN;
	}
	
	UART_PRINT("try to connect %s  key: %s\n\r", g_ssid_name, secParams.Key);
#if 1
	lRetVal = sl_WlanConnect((signed char *)g_ssid_name, strlen((const char *)g_ssid_name), 0, &secParams, 0);
    

    ASSERT_ON_ERROR(lRetVal);
#else
	 /* Clear all stored profiles and reset the policies */
    lRetVal = sl_WlanProfileDel(0xFF);    
    ASSERT_ON_ERROR(lRetVal);
    
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(0,0,0,0,0), 0, 0);    
    ASSERT_ON_ERROR(lRetVal);
    
    
    //Add Profile
    /* user needs to change SSID_NAME = "<Secured AP>"
             SECURITY_TYPE = SL_SEC_TYPE_WPA
             SECURITY_KEY = "<password>"
      and set the priority as per requirement 
      to connect with a secured AP */
    SlSecParams_t secParams;
    secParams.Key = SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;
    lRetVal = sl_WlanProfileAdd(SSID_NAME,strlen(SSID_NAME),0,&secParams,0,1,0);
    ASSERT_ON_ERROR(lRetVal);

    //set AUTO policy
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                      SL_CONNECTION_POLICY(1,0,0,0,0),
                      &policyVal, 1 /*PolicyValLen*/);    
    ASSERT_ON_ERROR(lRetVal);
#endif
    // Wait for WLAN Event
    while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
    {
        // wait till connects to an AP or some event
        osi_Sleep(10);
        
    }
    Report("Connect to %s success\r\n",g_ssid_name);
    return SUCCESS;

}


void WlanStaMode(void *pvParameters)
{   
    
    int iTestResult = 0;
    unsigned char ucDHCP;
    long lRetVal = -1;
	unsigned char ucQueueMsg = 0;
  
	
    InitializeAppVariables();
    //
    lRetVal = ConfigureSimpleLinkToDefaultState();
    if(lRetVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
            UART_PRINT("Failed to configure the device in its default state \n\r");

        LOOP_FOREVER();
    }

    UART_PRINT("Device is configured in default state \n\r");

	
    //
    // Asumption is that the device is configured in station mode already
    // and it is in its default state
    //
    lRetVal = sl_Start(0, 0, 0);

    if (lRetVal < 0)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Device started as STATION \n\r");


    //
    // reset all network policies
    //
    unsigned char policyVal;
    lRetVal = sl_WlanPolicySet(  SL_POLICY_CONNECTION,
                  SL_CONNECTION_POLICY(0,0,0,0,0),
                  &policyVal,
                  1 /*PolicyValLen*/);
    if (lRetVal < 0)
    {
        UART_PRINT("Failed to set policy \n\r");
        LOOP_FOREVER();
    }

	//
    // Set the power management policy of NWP
    //
	unsigned short PolicyBuff[4] = {0,0,200,0};
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM, SL_LONG_SLEEP_INTERVAL_POLICY, (unsigned char*)PolicyBuff, sizeof(PolicyBuff));
    if (lRetVal < 0)
    {
        UART_PRINT("unable to configure network power policy\n\r");
        LOOP_FOREVER();
    }
#if 0	
    // Connecting to WLAN AP - Set with static parameters defined at the top
    // After this call we will be connected and have IP address	
    lRetVal = WlanConnect(); 
	if(lRetVal < 0)
    {
        sl_Stop(SL_STOP_TIMEOUT);
        UART_PRINT("Connection to AP failed\n\r");
    }
    else
    {
        UART_PRINT("Connected to AP\n\r");
    }
#endif	

}

//****************************************************************************
//
//!    \brief start simplelink, wait for the sta to connect to the device and 
//!        run the ping test for that sta
//!
//!    \param  pvparameters is the pointer to the list of parameters that can be
//!         passed to the task while creating it
//!
//!    \return None
//
//****************************************************************************
void WlanAPMode(void *pvParameters)
{   
    int iTestResult = 0;
    unsigned char ucDHCP;
    long lRetVal = -1;

    InitializeAppVariables();

    //
    // Following function configure the device to default state by cleaning
    // the persistent settings stored in NVMEM (viz. connection profiles &
    // policies, power policy etc)
    //
    // Applications may choose to skip this step if the developer is sure
    // that the device is in its default state at start of applicaton
    //
    // Note that all profiles and persistent settings that were done on the
    // device will be lost
    //
    lRetVal = ConfigureSimpleLinkToDefaultState();
    if(lRetVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
            UART_PRINT("Failed to configure the device in its default state \n\r");

        LOOP_FOREVER();
    }

    UART_PRINT("Device is configured in default state \n\r");

    //
    // Asumption is that the device is configured in station mode already
    // and it is in its default state
    //
    lRetVal = sl_Start(NULL,NULL,NULL);

    if (lRetVal < 0)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Device started as AP \n\r");
    
    //
    // Configure the networking mode and ssid name(for AP mode)
    //
    if(lRetVal != ROLE_AP)
    {
        if(ConfigureMode(lRetVal) != ROLE_AP)
        {
            UART_PRINT("Unable to set AP mode, exiting Application...\n\r");
            sl_Stop(SL_STOP_TIMEOUT);
            LOOP_FOREVER();
        }
    }

    while(!IS_IP_ACQUIRED(g_ulStatus))
    {
      //looping till ip is acquired
    }

    unsigned char len = sizeof(SlNetCfgIpV4Args_t);
    SlNetCfgIpV4Args_t ipV4 = {0};

    // get network configuration
    lRetVal = sl_NetCfgGet(SL_IPV4_AP_P2P_GO_GET_INFO,&ucDHCP,&len,
                            (unsigned char *)&ipV4);
    if (lRetVal < 0)
    {
        UART_PRINT("Failed to get network configuration \n\r");
        LOOP_FOREVER();
    }
#if 0    
    UART_PRINT("Connect a client to Device\n\r");
    while(!IS_IP_LEASED(g_ulStatus))
    {
      //wating for the client to connect
    }
    UART_PRINT("Client is connected to Device\n\r");

    iTestResult = PingTest(g_ulStaIp);
    if(iTestResult < 0)
    {
        UART_PRINT("Ping to client failed \n\r");
    }

    UNUSED(ucDHCP);
    UNUSED(iTestResult);

    // revert to STA mode
    lRetVal = sl_WlanSetMode(ROLE_STA);
    if(lRetVal < 0)
    {
      ERR_PRINT(lRetVal);
      LOOP_FOREVER();
    }

    // Switch off Network processor
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    UART_PRINT("WLAN AP example executed successfully.\n\r");
    while(1);
#endif
}

void wlan_connect_task(void * pvParameters)
{
	long lRetVal = -1;
	while(1)
	{
		
		// Connecting to WLAN AP - Set with static parameters defined at the top
	    // After this call we will be connected and have IP address	
	    lRetVal = WlanConnect(); 
		if(lRetVal < 0)
	    {
	        sl_Stop(SL_STOP_TIMEOUT);
	        UART_PRINT("Connection to AP failed\n\r");
	    }
	    else
	    {
	        UART_PRINT("Connected to AP\n\r");
	    }

		UART_PRINT("wait net error!\r\n");
		
		while(IS_CONNECTED(g_ulStatus)&&IS_IP_ACQUIRED(g_ulStatus))
		{
			osi_Sleep(10);
		}
		g_iSleepAllow = 0;
		g_socket_state = 0;
		UART_PRINT("\r\n\r\nnet error!\r\n");

		

	}
}

void socket_connect_task(void * pvParameters)
{
	long lRetVal = -1;
	
	while(1)
	{
		
		lRetVal = CreateTcpSocket(&g_iSocket);
		if(SUCCESS != lRetVal)
		{
			UART_PRINT("create tcp socket error!\r\n");
			return ;
		}
		g_socket_state = 1;
		
		lRetVal = osi_SyncObjSignal(&g_NetrecvSyncObj);
		if(lRetVal < 0)
		{
			UART_PRINT("realse SyncObjSignal g_NetrecvSyncObj error!\r\n");
			return ;
		}
		lRetVal = osi_SyncObjSignal(&g_NetSendSyncObj);
		if(lRetVal < 0)
		{
			UART_PRINT("realse SyncObjSignal g_NetSendSyncObj error!\r\n");
			return ;
		}
		lRetVal = osi_SyncObjSignal(&g_NetconnectSyncObj);
		if(lRetVal < 0)
		{
			UART_PRINT("realse SyncObjSignal g_NetconnectSyncObj error!\r\n");
			return ;
		}
		Report("wait socket error\r\n");
		
		g_iSleepAllow = 1;
		lRetVal = osi_SyncObjWait(&g_socketerrorSyncObj, OSI_WAIT_FOREVER);
		if(lRetVal <0 )
		{
			Report("wait g_NeterrorSyncObj error\r\n");
			return ;
		}
		//g_iSleepAllow = 0;
		Report("socket error\r\n");
		power_ak(0);
		//g_socket_state = 0;
		//osi_Sleep(1000);
		
		lRetVal = sl_Close(g_iSocket);
		if(lRetVal <0 )
		{
			Report("clsoe socket error %d\r\n", lRetVal);
			osi_Sleep(100);
		}
		
		Report("socket sck close success\r\n");
					
	}
}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t       CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs) || defined(gcc)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}


void main_task(void * pvParameters )
{   
    
    int iTestResult = 0;
    unsigned char ucDHCP;
    long lRetVal = -1;
	unsigned char ucQueueMsg = 0;
	//  ak_sys_init();
#if 0
    InitializeAppVariables();
    //
    lRetVal = ConfigureSimpleLinkToDefaultState();
    if(lRetVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
            UART_PRINT("Failed to configure the device in its default state \n\r");

        LOOP_FOREVER();
    }

    UART_PRINT("Device is configured in default state \n\r");


	//wmj+ power off ak first
	//power_off_ak(0);
    //
    // Asumption is that the device is configured in station mode already
    // and it is in its default state
    //
    lRetVal = sl_Start(0, 0, 0);

    if (lRetVal < 0)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Device started as STATION \n\r");


    //
    // reset all network policies
    //
    unsigned char policyVal;
    lRetVal = sl_WlanPolicySet(  SL_POLICY_CONNECTION,
                  SL_CONNECTION_POLICY(0,0,0,0,0),
                  &policyVal,
                  1 /*PolicyValLen*/);
    if (lRetVal < 0)
    {
        UART_PRINT("Failed to set policy \n\r");
        LOOP_FOREVER();
    }

	//
    // Set the power management policy of NWP
    //
	unsigned short PolicyBuff[4] = {0,0,800,0};
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM, SL_LONG_SLEEP_INTERVAL_POLICY, (unsigned char*)PolicyBuff, sizeof(PolicyBuff));
    if (lRetVal < 0)
    {
        UART_PRINT("unable to configure network power policy\n\r");
        LOOP_FOREVER();
    }
     // Connecting to WLAN AP - Set with static parameters defined at the top
     // After this call we will be connected and have IP address
     
     lRetVal = WlanConnect(); 
	if(lRetVal < 0)
    {
        sl_Stop(SL_STOP_TIMEOUT);
        UART_PRINT("Connection to AP failed\n\r");
    }
    else
    {
        UART_PRINT("Connected to AP\n\r");
    }
#endif
	
	lRetVal = osi_SyncObjCreate(&g_NetSendSyncObj);
	if(lRetVal < 0)
	{
		UART_PRINT("syncobj create faile! \n\r");
	}
	lRetVal = osi_SyncObjCreate(&g_NetconnectSyncObj);
	if(lRetVal < 0)
	{
		UART_PRINT("syncobj create faile! \n\r");
	}
	lRetVal = osi_SyncObjCreate(&g_NetrecvSyncObj);
	if(lRetVal < 0)
	{
		UART_PRINT("syncobj create faile! \n\r");
	}
	lRetVal = osi_SyncObjCreate(&g_socketerrorSyncObj);
	if(lRetVal < 0)
	{
		UART_PRINT("syncobj create faile! \n\r");
	}
	

	//set gpio 4 as config reset button, press 3 second will reset config 
	key_open(GPIO_KEY, 1, 100, key_press_cb);

	//osi_TaskCreate(msg_process_task, \
      //                     (const signed char*)"MSG queue", \
      //                     OSI_STACK_SIZE, NULL, 5, NULL );
	
	//
    // Initializing the CC3200 networking layers
    //
    lRetVal = sl_Start(0, 0, 0);
    if(lRetVal < 0)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }
    /*read profile*/
	ReadProfile((unsigned char * )&g_sys_cfg, sizeof(struct sys_cfg));
	
	lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	if(lRetVal < 0)
	{
		UART_PRINT("Failed to sl_Stop\n\r");
	}

	
	if(strlen(g_sys_cfg.ssid) == 0 || g_sys_cfg.server_ip == 0 || g_sys_cfg.port_num == 0)
	{
	    WlanAPMode(NULL);
		lRetVal = osi_TaskCreate(NetCmdTcpServer, \
                            (const signed char*)"NetCmdUdpServer", \
                            OSI_STACK_SIZE*2, NULL, 8, NULL );
		if(lRetVal < 0)
   		{
        	UART_PRINT("Failed to creat cmd udp task \n\r");
        	LOOP_FOREVER();
    	}
		CmdLoop();
	}
	else
	{
		WlanStaMode(NULL);
	}
	//
	lRetVal = osi_TaskCreate(heart_Task,(const signed char*)"heart throb task", OSI_STACK_SIZE, NULL, 9, NULL );
	if(lRetVal < 0)
	{
		UART_PRINT("Failed to creat heart task !\r\n");
		return ;
	}
	
	lRetVal = osi_TaskCreate(wlan_connect_task,(const signed char*)"wlan socket connect task", OSI_STACK_SIZE, NULL, 1, NULL );
    if(lRetVal < 0)
	{
		UART_PRINT("Failed to creat wlan connect task !\r\n");
		return ;
	}
	lRetVal = osi_TaskCreate(socket_connect_task,(const signed char*)"wlan socket connect task", OSI_STACK_SIZE, NULL, 1, NULL );
    if(lRetVal < 0)
	{
		UART_PRINT("Failed to creat socket connect task !\r\n");
		return ;
	}
	lRetVal = osi_TaskCreate(TcpRecvCmdTask,(const signed char*)"tcp recvcmd task", OSI_STACK_SIZE, NULL, 2, NULL );
	if(lRetVal < 0)
	{
		UART_PRINT("Failed to creat tcp recv cmd task !\r\n");
		return ;
	}
	
	lRetVal = osi_SyncObjWait(&g_NetconnectSyncObj, OSI_WAIT_FOREVER);
	if(lRetVal <0 )
	{
		Report("wait g_NetconnectSyncObj error\r\n");
		return ;
	}

	// Queue management related configurations
    //
    lRetVal = osi_MsgQCreate(&g_tWkupSignalQueue, NULL,
                             sizeof( unsigned char ), 10);
    if (lRetVal < 0)
    {
        UART_PRINT("unable to create the msg queue\n\r");
        LOOP_FOREVER();
    }
	//
    // setting some GPIO as one of the wakeup source
    //
   
    SetGPIOAsWkUp();

	//
	// setting Timer as one of the wakeup source
	//
    //SetTimerAsWkUp();
	    
    //
    // setting Apps power policy
    //
    lp3p0_setup_power_policy(POWER_POLICY_STANDBY);
      
    while(FOREVER)
    {
        //
        // waits for the message from the various interrupt handlers(GPIO,
        // Timer) and the UDPServerTask.
        //
        osi_MsgQRead(&g_tWkupSignalQueue, &ucQueueMsg, OSI_WAIT_FOREVER);
        switch(ucQueueMsg){
        case 1:
            UART_PRINT("timer\n\r");
            break;
        case 2:
            UART_PRINT("GPIO\n\r");
			wakeup_type = EVENT_PIR_WAKEUP;
			g_iSleepAllow = 0;
			closeGPIOWkUp();
			StartAk39e();
            break;
        case 3:         // start
            UART_PRINT("host irq\n\r");
			wakeup_type = EVENT_VIDEO_PREVIEW_WAKEUP;
			g_iSleepAllow = 0;
			closeGPIOWkUp();
			StartAk39e();
            break;
		case 4:         // stop
			if(0 == power_ak(0))
			{
				osi_Sleep(5000);
			}
			SetGPIOAsWkUp();
			g_iSleepAllow = 1;
			//lp3p0_setup_power_policy(POWER_POLICY_STANDBY);
			break;
		case 5:
			UART_PRINT("clear config irq\n\r");
			clear_config();
			break;
        default:
            UART_PRINT("invalid msg\n\r");
            break;
        }
    }

}



//*****************************************************************************
//                            MAIN FUNCTION
//*****************************************************************************

void main()
{
    long lRetVal = -1;
  
    //
    // Board Initialization
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();
    
#ifndef NOTERM
    //
    // Configuring UART
    //
    InitTerm();
#endif

	/*PM initalization*/

	platform_init(); //wmj

    //
    // Display banner
    //
    DisplayBanner(APP_NAME);
    //Report(APP_NAME);


	#ifdef DEBUG_GPIO
    //
    // setting up GPIO for indicating the entry into LPDS
    //
    tGPIODbgHndl = cc_gpio_open(GPIO_10, GPIO_DIR_OUTPUT);
    cc_gpio_write(tGPIODbgHndl, GPIO_10, 1);
	#endif
	
	
    //
    // Start the SimpleLink Host
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

	
	
#if 0
    // Start the WlanAPMode task
    lRetVal = osi_TaskCreate( WlanAPMode, \
                            (const signed char*)"wireless LAN in AP mode", \
                            OSI_STACK_SIZE*5, NULL, 1, NULL );
#endif
	
#if 1
	lRetVal = osi_TaskCreate(main_task, \
                            (const signed char*)"main process task", \
                            OSI_STACK_SIZE*5, NULL, 1, NULL );
	
#endif	
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
	
    //
    // Start the task scheduler
    //
    osi_start();
}



