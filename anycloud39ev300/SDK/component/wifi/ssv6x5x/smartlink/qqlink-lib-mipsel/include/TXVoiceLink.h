#ifndef __TX_DECODE_ENGINE__H__
#define __TX_DECODE_ENGINE__H__ 
#include "TXSDKCommonDef.h"
/***********************************************************************/
/*
*   声波配网简介：
*   功能和效用等同于smartlink，使用方式类似，主要区别是以声波的为媒介，通过手Q将路由器的ssid和密码传送给设备。
*
*	设备要求：
*		设备具有麦克风，最低采样率大于或等于8000Hz，单声道，16位。我们使用的频段是1400~3900Hz
*	
*	手Q要求：
*		版本 >= 5.8 初始化方式含有voicelink
*
*	集成说明：
*		 
         --------------------------------------- 
        |          初始化麦克风模块               |
         ---------------------------------------
		
         ---------------------------------------
        |          初始化声波配网模块             |
         ---------------------------------------
		
         ---------------------------------------
        |       循环读取录音数据交由声波模块分析	    |
        |       调用tx_fill_audio 函数           |
        |       识别成功后回调 VL_FUNC_NOTIFY     |
        |       可以结束循环                      |
         ---------------------------------------

         ---------------------------------------
        |           反初始化声波配网模块           |
         ---------------------------------------
		
         ---------------------------------------
        |           反初始化麦克风模块             |
         ---------------------------------------

特别说明：回调函数，是在解析到ssid，密码后调用，回调结束后回进入解析流程。因此千万不可在回调函数中调用
声波配网模块的反初始化函数 tx_uninit_decoder。

声波采集有两种方式
1. 通过类似readPCMdata函数去读取
2. 设置一个回调类似 onPCMdata，在回调中获取PCM数据

对于方式1. 可以定义一个全局的g_finish变量。开始置为false（0），回调中置为true（1）。处理过程可以参照集成说明的流程
伪码描述：


start_voicelink
{
  tx_init_decoder
  initaudio
  g_finish = 0
  while（!g_finish)
  {
    readPCMdata
    tx_fill_audio
  }
  uninitaudio
  tx_uninit_decoder  
}


对于方式2. 也是定义一个全局的g_finish变量。
伪码描述：
start_voicelink
{
  tx_init_decoder
  g_finish = 0
  initaudio(onPCMdata)
}

onPCMdata
{
  if (g_finish)
  {
     uninitaudio
     tx_uninit_decoder
     return
  }
  tx_fill_audio
}

回调处理，伪码描述
on_voicelink_notify
{
  connect_ap
  g_finish = 1
}
*/   
/***********************************************************************/


CXX_EXTERN_BEGIN

enum RESULT 
{
	ERROR_NULL 					= 0,
	ERROR_INITED				= 1,
	ERROR_NO_MEMORY 			= 2,
	ERROR_CREATE_LOCK_FAIL 		= 3,
	ERROR_CREATE_THREAD_FAIL	= 4,  
};

#define MAX_SSID_LEN      128
#define MAX_PSWD_LEN      128
#define MAX_IP_LEN        16

typedef struct
{
	char                sz_ssid[MAX_SSID_LEN];
	char                sz_password[MAX_PSWD_LEN];
    char                sz_ip[MAX_IP_LEN];
    unsigned short      sh_port;
} tx_voicelink_param;


typedef void (*VL_FUNC_NOTIFY)(tx_voicelink_param* pparam);

/**
*  初始化声波配网模块
*   samplerate: 麦克风的采样率 >= 8000Hz，采样率越高计算量越大，建议不要使用下采样或重采样,可能影响结果。
*
*/
SDK_API int tx_init_decoder(VL_FUNC_NOTIFY func, int samplerate);

/**
*  反初始化声波配网模块
*
*/
SDK_API void tx_uninit_decoder();

/*
*  数据为PCM格式，会被拷贝到内部 buffer 队列中，用于识别。
*  nlen: 是aduio数组的长度，即采样点数。 
*  (通常录音间隔20ms，50ms，100ms。注：内部有一个容纳4096个采样点的缓冲buffer，nlen的值不能超过4096)
*/
SDK_API void tx_fill_audio(signed short* audio, int nlen);



CXX_EXTERN_END

#endif