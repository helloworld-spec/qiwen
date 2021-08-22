// TranceCom.h: interface for the CTranceCom class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRANCECOM_H__2FE824FF_4BEE_4F59_A130_69AC9B144F4F__INCLUDED_)
#define AFX_TRANCECOM_H__2FE824FF_4BEE_4F59_A130_69AC9B144F4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



typedef enum
{
    SPP_COMMAND_CHECK= 0X10,    //命令回应
	SPP_CHANGE_MODE,            //切换系统工作模式
	SPP_GET_PLAY_NAME,          //得到当前播放的文件名
	SPP_GET_MUSIC_LIST,         //得到当前播放的列表
	SPP_SET_VOL,                    //设置音量
	SPP_GET_VOL,                    //得到当前音量
	SPP_SET_PLAY_STATUS, //0x0a           //设置当前播放状态
	SPP_GET_PLAY_STATUS,        //得到当前播放状态
	SPP_SET_EQ_MODE,                //设置当前的EQ模式
	SPP_GET_EQ_MODE,            //得到当前EQ模式
	SPP_SET_PLAY_MODE,      //设置当前播放模式
	SPP_GET_PLAY_MODE,  //0x0f    //得到当前的播放模式
	SPP_GET_VOLTAGE,     //0x10       //得到设备电压
	SPP_SET_PLAY_MUSICID,   //设置当前播放的歌曲
	SPP_SET_LAST_NEXT,           //设置上下曲
	SPP_GET_STDB_MODE,           //得到系统工作模式
	SPP_SEND_LIST_OVER,           //通知列表发完
	SPP_SEND_TF_STATUS,           //通知T卡有插拔动作
	SPP_GET_FM_LIST,				//得到FM的频道列表
	SPP_SET_FM_PLAY_ID, 		//设置FM的播放频道
	SPP_SET_SYSTEM_TIME,			//设置系统时钟
	SPP_GET_ALARM_TIME, 		//得到系统闹钟
	SPP_SET_ALARM_TIME, 		//设置系统闹钟	
	SPP_SET_PLAY_BACK_FORWARD_START,	//设置音乐播放快进/快退开始
	SPP_SET_PLAY_BACK_FORWARD_STOP,	//设置音乐播放快进/快退结束
		
}SPP_MSG_ID;

#define PACK_START_FLAG          0X06
#define PACK_END_FLAG            0X07
#define COMMAND_CHECK_FLAG       0X10
#define PACK_START               0
#define PACK_LEN                 1
#define COMMAND_CHECK            3
#define COMMAND_SRC              4
#define COMMAND_OKORNOT          5




#define PACKET_DATA_OK		0X55
#define PACKET_DATA_ERR		0XAA

#define PACKET_ALL		    0x80 //0XFF
#define PACKET_NOTALL	    0X00

#define PACKET_TRAN_FLAG	0X08
#define PACKET_TRAN_FLAG1	(PACK_START_FLAG + PACKET_TRAN_FLAG)
#define PACKET_TRAN_FLAG2	(PACK_END_FLAG + PACKET_TRAN_FLAG)
#define PACKET_TRAN_FLAG3	(PACKET_TRAN_FLAG + PACKET_TRAN_FLAG)


#define DATA_FAIL		0
#define DATA_SUCCEC		1
#define DATA_OK		    2
#define DATA_ERR		3

typedef struct
{
    short	list_id;
    short    frequency; 		                                      
}T_FM_FREQUENCY_LIST_INFO;

typedef struct
{
    T_FM_FREQUENCY_LIST_INFO *m_fm_frequency_info;
    short m_fm_frequency_info_count;		                                      
}T_FM_FREQUENCY;

typedef struct
{
    UINT	list_id;
    TCHAR   file_name[MAX_PATH]; 	                                      
}T_PLAY_LIST_INFO;


typedef struct
{
    T_PLAY_LIST_INFO *m_play_list_info;
    UINT m_play_list_info_count;		                                      
}T_PLAY_LIST;

typedef struct 
{
	WORD	year;			/* 4 byte: 1-9999 */
	BYTE	month;			/* 1-12 */
	BYTE	day;			/* 1-31 */
	BYTE	hour;			/* 0-23 */
	BYTE	minute; 		/* 0-59 */
	BYTE	second; 		/* 0-59 */
	BYTE	week;			/* bit0: whether single alarm; bit1:monday ~ bit7:sunday*/
} T_SYSTIME, *T_pSYSTIME;   /* system time structure */ 

class CTranceCom  
{
public:
	CTranceCom();
	virtual ~CTranceCom();

	BOOL trancecom_Open(TCHAR *com, UINT userBaudRate);
	void trancecom_Close(void);
	UINT trancecom_Read(char *buf, UINT buf_len);
	UINT trancecom_Write(char *buf, UINT buf_len);
	BOOL trancecom_out(char cmd, char *buf, UINT buf_len);
    BOOL trancecom_in(char cmd_ok, char scr_cmd, char *buf, UINT buf_len);
	//void DoRxTx(void *args);
	//int ReadCommBlock(TCHAR *buf,int maxLen);


};

#endif // !defined(AFX_TRANCECOM_H__2FE824FF_4BEE_4F59_A130_69AC9B144F4F__INCLUDED_)
