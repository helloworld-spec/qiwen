

#ifndef _AEC_INTERFACE_H_
#define _AEC_INTERFACE_H_

#include "anyka_types.h"
#include "anyka_types_internal.h"
#define MIC_ONEBUF_SIZE   (1024 * 2)


#define  ECHO_LIB_VERSION_STRING (T_U8 *)"Echo Version V1.1.10"


#define  AKAGC_GAIN_Q  10
#define  AKAGC_F32G(x)    ((T_U32)((x)*(1L<<AKAGC_GAIN_Q)))

// chip binding
typedef enum 
{
    ECHO_CHIP_UNKNOW = 0,
    ECHO_CHIP_AK10XXC = 1,
    ECHO_CHIP_AK37XXC = 2,
    ECHO_CHIP_AK39XX = 3,
    ECHO_CHIP_AK10XXT = 4,
    ECHO_CHIP_AK39XXE = 5,
    ECHO_CHIP_AK37XXC_RTOS = 6,
    ECHO_CHIP_AK39XXEV2 = 7,
    ECHO_CHIP_AK10XXD = 8,
    ECHO_CHIP_AK39XXEV2_RTOS = 9
}ECHO_CHIP_ID;

typedef T_pVOID (*AEC_CALLBACK_FUN_MALLOC)(T_U32 size); 
typedef T_VOID  (*AEC_CALLBACK_FUN_FREE)(T_pVOID mem);
typedef T_S32   (*AEC_CALLBACK_FUN_PRINTF)(T_pCSTR format, ...);
typedef T_VOID  (*AEC_CALLBACK_FUN_RTC_DELAY) (T_U32 ulTicks);
typedef T_VOID  (*AEC_CALLBACK_FUN_NOTIFY) (T_U32 event);

typedef struct
{
    AEC_CALLBACK_FUN_MALLOC			Malloc;
    AEC_CALLBACK_FUN_FREE			Free;
    AEC_CALLBACK_FUN_PRINTF			printf;
    AEC_CALLBACK_FUN_RTC_DELAY		delay;
    AEC_CALLBACK_FUN_NOTIFY         notify;
}T_AEC_CB_FUNS;

typedef enum
{
    AEC_TYPE_UNKNOWN ,
    AEC_TYPE_1
}T_AEC_TYPE;

typedef struct {
    T_U32   m_aecEna;
    T_U32   m_PreprocessEna;
    T_U32   m_framelen;  //NN
    T_U32   m_tail;      //TAIL 
    T_U32   m_agcEna;
    T_U32   m_agcLevel;  // 1~32767, 0: use default
    T_U32   m_maxGain;   // agc's max_gain
}T_AEC_PARAMS;

typedef struct
{
    T_pCSTR strVersion; // must be ECHO_LIB_VERSION_STRING
    ECHO_CHIP_ID chip;
    T_U32	m_Type;				//扩展用，标明AEC算法类别，是T_AEC_TYPE的枚举之一
    T_U32	m_SampleRate;		//sample rate, sample per second
    T_U16	m_Channels;			//channel number
    T_U16	m_BitsPerSample;	//bits per sample  
    
    union {
        T_AEC_PARAMS m_aec;
    }m_Private;
}T_AEC_IN_INFO;

typedef struct
{
    T_VOID *buf_near;  //near, mic
    T_U32  len_near;
    T_VOID *buf_far;  //far, speaker
    T_U32  len_far;
    T_VOID *buf_out; // output of aec
    T_U32  len_out;
}T_AEC_BUF;

typedef struct
{
    T_AEC_CB_FUNS	cb_fun;
    T_AEC_IN_INFO	m_info;
}T_AEC_INPUT;

typedef enum
{
    AEC_REQ_UNKNOWN ,
    AEC_REQ_GET_SAMPLING_RATE, //get sample rate, data: int
    AEC_REQ_GET_FILTER_SIZE, // get filter length, data: int
    AEC_REQ_GET_FILTER, // get filter weight, data: 
    AEC_REQ_GET_PARAMS, 
    AEC_REQ_SET_PARAMS, 
    AEC_REQ_SET_AGC_MINGAIN2
}T_AEC_REQ_TYPE;

typedef struct
{
    T_U32 m_req_type;  // 要做什么事情，例如是否设置采样率
    T_U8  *addr;       // 要设置或获取的参数地址
    T_U32 addrLen;     // addr指向的地址空间的字节数
}T_AEC_REQUEST;

T_VOID *AECLib_Open(T_AEC_INPUT *p_aec_input);
T_S32 AECLib_Close(T_VOID *p_aec);
T_S32 AECLib_Control(T_VOID *p_aec, T_AEC_BUF *p_aec_buf);
T_S32 AECLib_Request(T_VOID *p_aec, T_AEC_REQUEST *p_aec_req);

#endif
