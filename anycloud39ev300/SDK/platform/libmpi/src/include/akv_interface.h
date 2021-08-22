#ifndef _AKV_ENC_CODEC_H_
#define _AKV_ENC_CODEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "anyka_types_internal.h"
#include <stdio.h>

#define VIDEO_LIB_VERSION			"AKV Encoder Lib V2.2.21"

typedef struct
{
	T_U8       *pData;  // virtual address of the buffer
	T_U32      pa;      // physical address of the buffer
	T_U32      size;    // size of the buffer
	T_pVOID    hHandle; // handle for user
}T_AKV_DMA_BUFFER;

typedef struct
{
    T_U16 width; // should be 32
    T_U16 height; // should be 24 or 16
    T_U16 (*data)[24][32]; // at most 32*24
}T_AK_MDINFO;

typedef struct
{
	T_AKV_DMA_BUFFER *pdmabuf;
	T_AK_MDINFO       mdinfo; //info of movedetect
	T_U64             srcTimeStamp; //for source timestamp
}T_AKV_SRC_FRAME;

typedef struct
{
	T_AKV_SRC_FRAME *pSrcFrame;
	T_U16 SrcWidth;	// width of the source picture, multiple of 32
	T_U16 SrcHeight; //height of the source picture, multiple of 8
	T_U16 HorOffsetSrc;	//the starting position for cropping in horizontal direction, relative to the upper left corner of the source picture, multiple of 32
	T_U16 VerOffsetSrc;	//the starting position for cropping in vertical direction, relative to the upper left corner of the source picture, multiple of 2
}T_AKV_SRC_FRAME_CROP;

typedef enum Slice_Type
{
  AK_SLICE_B = 0,  /*!< B Slice */
  AK_SLICE_P = 1,  /*!< P Slice */
  AK_SLICE_I = 2,  /*!< I Slice */
  AK_SLICE_PI = 3  /*!< PI Slice */
}AK_SLICE_TYPE;

typedef enum AK_e_RateCtrlMode
{
	AK_RC_CBR           = 0x01,
	AK_RC_VBR           = 0x02,
}AK_ERateCtrlMode;

typedef enum AK_e_Profile
{
	AK_PROFILE_AVC               = 0x01000000,
	AK_PROFILE_AVC_BASELINE      = AK_PROFILE_AVC |  66,
	AK_PROFILE_AVC_MAIN          = AK_PROFILE_AVC |  77,
	AK_PROFILE_AVC_HIGH          = AK_PROFILE_AVC | 100,

	AK_PROFILE_HEVC             = 0x02000000,
	AK_PROFILE_HEVC_MAIN        = AK_PROFILE_HEVC | 1,
	AK_PROFILE_HEVC_MAIN_STILL  = AK_PROFILE_HEVC | 3,

	AK_PROFILE_JPEG             = 0x04000000,
} AK_EProfile;
typedef enum AK_e_AVC_LEVEL
{
	AK_AVC_LEVEL_1   = 10,
	AK_AVC_LEVEL_1_1 = 11,
	AK_AVC_LEVEL_1_2 = 12,
	AK_AVC_LEVEL_1_3 = 13,
	AK_AVC_LEVEL_2   = 20,
	AK_AVC_LEVEL_2_1 = 21,
	AK_AVC_LEVEL_2_2 = 22,
	AK_AVC_LEVEL_3   = 30,
	AK_AVC_LEVEL_3_1 = 31,
	AK_AVC_LEVEL_3_2 = 32,
	AK_AVC_LEVEL_4   = 40,
	AK_AVC_LEVEL_4_1 = 41,
	AK_AVC_LEVEL_4_2 = 42,
	AK_AVC_LEVEL_5   = 50,
	AK_AVC_LEVEL_5_1 = 51,
	AK_AVC_LEVEL_5_2 = 52,
	AK_AVC_LEVEL_6   = 60,
	AK_AVC_LEVEL_6_1 = 61,
	AK_AVC_LEVEL_6_2 = 62		
}AK_EAVC_LEVEL;
typedef enum AK_e_HEVC_LEVEL
{
	AK_HEVC_LEVEL_1   = 10,
	AK_HEVC_LEVEL_2   = 20,
	AK_HEVC_LEVEL_2_1 = 21,
	AK_HEVC_LEVEL_3   = 30,
	AK_HEVC_LEVEL_3_1 = 31,
	AK_HEVC_LEVEL_4   = 40,
	AK_HEVC_LEVEL_4_1 = 41,
	AK_HEVC_LEVEL_5   = 50,
	AK_HEVC_LEVEL_5_1 = 51,
	AK_HEVC_LEVEL_5_2 = 52,
	AK_HEVC_LEVEL_6   = 60,
	AK_HEVC_LEVEL_6_1 = 61,
	AK_HEVC_LEVEL_6_2 = 62		
}AK_EHEVC_LEVEL;


typedef T_VOID  (*AKV_CB_FUN_PRINTF)(T_pCSTR format, ...);
typedef T_pVOID (*AKV_CB_FUN_MALLOC)(T_U32 size); 
typedef T_VOID  (*AKV_CB_FUN_FREE)(T_pVOID mem);

typedef T_U32   (*AKV_CB_FUN_IPCTRL_READREGISTER) (T_U32 uReg);
typedef T_VOID  (*AKV_CB_FUN_IPCTRL_WRITEREGISTER) (T_U32 uReg, T_U32 uVal);

typedef T_AKV_DMA_BUFFER* (*AKV_CB_FUN_DMA_ALLOC)(T_U32 zSize);
typedef T_BOOL  (*AKV_CB_FUN_DMA_FREE)(T_AKV_DMA_BUFFER* hBuf);
typedef T_VOID  (*AKV_CB_FUN_FLUSH_DCACHE)(void);

typedef T_VOID  (*AKV_CB_FUN_RELEASE_FRAME_BUF)(T_AKV_DMA_BUFFER* hBuf);
typedef T_VOID  (*AKV_CB_FUN_MODULE_CLOCK)(T_BOOL bEnable);
typedef T_VOID  (*AKV_CB_FUN_MODULE_RESET)(void);

typedef T_pVOID (*AKV_CB_FUN_MUTEX_CREATE)(T_BOOL bInitialState);
typedef T_VOID  (*AKV_CB_FUN_MUTEX_DELETE)(T_pVOID pMutex);
typedef T_BOOL  (*AKV_CB_FUN_MUTEX_GET)(T_pVOID pMutex);
typedef T_BOOL  (*AKV_CB_FUN_MUTEX_RELEASE)(T_pVOID pMutex);

typedef T_pVOID (*AKV_CB_FUN_SEMAPHORE_CREATE)(T_S32 iInitialCount);
typedef T_VOID  (*AKV_CB_FUN_SEMAPHORE_DELETE)(T_pVOID Semaphore);
typedef T_BOOL  (*AKV_CB_FUN_SEMAPHORE_GET)(T_pVOID Semaphore, T_U32 Wait);
typedef T_BOOL  (*AKV_CB_FUN_SEMAPHORE_RELEASE)(T_pVOID Semaphore);

typedef T_VOID  (*AKV_CB_FUN_IRQ_WAIT)(int *irqState);

typedef T_S32   (*AKV_CB_FUN_ATOMIC_INCREMENT) (T_S32* iVal);
typedef T_S32   (*AKV_CB_FUN_ATOMIC_DECREMENT) (T_S32* iVal);

typedef struct
{
	AKV_CB_FUN_PRINTF						ak_FunPrintf;
	AKV_CB_FUN_MALLOC						ak_FunMalloc;
	AKV_CB_FUN_FREE							ak_FunFree;

	AKV_CB_FUN_IPCTRL_READREGISTER			ak_FunReadRegister;
	AKV_CB_FUN_IPCTRL_WRITEREGISTER			ak_FunWriteRegister;

	AKV_CB_FUN_DMA_ALLOC					ak_FunDmaAlloc; // 32 Byte aligned
	AKV_CB_FUN_DMA_FREE						ak_FunDmaFree;
	AKV_CB_FUN_FLUSH_DCACHE	                ak_FunFlushDcache;

    AKV_CB_FUN_RELEASE_FRAME_BUF            ak_FunReleaseFrameBuf;
	AKV_CB_FUN_MODULE_CLOCK					ak_FunModuleClock;
	AKV_CB_FUN_MODULE_RESET					ak_FunModuleReset;

	AKV_CB_FUN_MUTEX_CREATE					ak_FunMutexCreate; 
	AKV_CB_FUN_MUTEX_DELETE					ak_FunMutexDelete;
	AKV_CB_FUN_MUTEX_GET					ak_FunMutexGet;
	AKV_CB_FUN_MUTEX_RELEASE				ak_FunMutexRelease;
    
	AKV_CB_FUN_SEMAPHORE_CREATE				ak_FunSemaphoreCreate;
	AKV_CB_FUN_SEMAPHORE_DELETE				ak_FunSemaphoreDelete;
	AKV_CB_FUN_SEMAPHORE_GET				ak_FunSemaphoreGet;
	AKV_CB_FUN_SEMAPHORE_RELEASE			ak_FunSemaphoreRelease;
    
	AKV_CB_FUN_IRQ_WAIT						ak_FunIrqWait;

	AKV_CB_FUN_ATOMIC_INCREMENT				ak_FunAtomicIncrement;
	AKV_CB_FUN_ATOMIC_DECREMENT				ak_FunAtomicDecrement;
}T_AKVLIB_CB;

typedef struct
{
	AK_EProfile eProfile;
	AK_ERateCtrlMode eRCMode;
	T_U16 FrameWidth;
	T_U16 FrameHeight;
	T_U16 TargetBitRate; // kbps
	T_U16 MaxBitRate;    // kbps
	T_U16 EncFrameRate;
	T_U16 InitialSliceQP;
	//T_U16 MaxIQP;
	//T_U16 MinIQP;
	T_U16 MinQP;
	T_U16 MaxQP;
	T_U16 GopLength;
	T_U16 GopNumB;
	T_U16 NumSlices;
	T_U16 SmartMode; //0:disable smart, 1:LTR, 2:change GOP, 3:skip frame reference
	T_U16 SmartGopLength;
	T_U16 SmartQuality;
	T_U16 QuantTableLevel;//0:defaut, 1: hightest, 2: high, 3: low 
	T_U16 SmartStaticValue;
	//T_U16 MaxFrameSize;//Kbyte
	//T_U16 MinFrameSize;//Kbyte
	//T_U16 EncLevel; //specifies the Level to which the bitstream conforms. from 10 to 50
}T_AKVLIB_PAR;

typedef struct
{
	char *sCfgFileName;      // path of cfg file
	T_AKVLIB_PAR AkvParams;     // user parameters which will overwrite those in cfg file
}T_AKV_ENC_OPEN_INPUT;

typedef struct
{
	T_U8       *pData;  // virtual address of the buffer
	T_U32      BufSize; // size of the buffer
	T_U32      OutSize; // size of output data
	AK_SLICE_TYPE slicetype;
	T_U64      streamTimeStamp; //for stream timestamp
}T_AKV_STREAM_BUFFER;

//初始化视频编码库并分配全局资源。
T_BOOL  AKV_Encoder_Init(const T_AKVLIB_CB *init_cb_fun);

//分配编码器需要的内存，并初始化
T_pVOID AKV_Encoder_Open(const T_AKV_ENC_OPEN_INPUT *open_input);

//释放Open时申请的资源
T_VOID  AKV_Encoder_Close(T_pVOID hVEnc);

//销毁视频编码库并释放全局资源
T_VOID  AKV_Encoder_Destory(void);

//编码处理
// pFrame: [in] 输入帧
// pStream: [in,out] 输出缓冲区, 暂时需要给出一个足够大的缓冲
T_VOID  AKV_Encoder_Process(T_pVOID hVEnc, T_AKV_SRC_FRAME *pFrame, T_AKV_STREAM_BUFFER *pStream);

//编码处理
// pCropFrame: [in] 输入帧，具有crop信息
// pStream: [in,out] 输出缓冲区, 暂时需要给出一个足够大的缓冲
T_VOID  AKV_Encoder_Process_Crop(T_pVOID hVEnc, T_AKV_SRC_FRAME_CROP *pCropFrame, T_AKV_STREAM_BUFFER *pStream);

//获取编码器参数
T_VOID  AKV_Encoder_Get_Parameters(T_pVOID hVEnc, T_AKVLIB_PAR *pAkvParams);

//在线设置编码器参数
T_VOID  AKV_Encoder_Set_Parameters(T_pVOID hVEnc, const T_AKVLIB_PAR *pAkvParams);

//强制编I帧
T_VOID  AKV_Encoder_Set_EncIFrame(T_pVOID hVEnc);

//设置编码器StreamBuffer的大小，单位为字节数， 在AKV_Encoder_Open调用之后和AKV_Encoder_Process调用之前调用
T_VOID  AKV_Encoder_Set_StreamBufferSize(T_pVOID hVEnc, T_U32 usize);

// 获取编码库版本信息
T_S8 *AKV_Encoder_GetVersionInfo(void);

// 设置编码器应用新设置的最小最大QP，新设置的值必须在调用AKV_Encoder_Open时设置的最小最大QP范围之内
T_VOID  AKV_Encoder_Apply_Quality(T_pVOID hVEnc, T_U8 minQP, T_U8 maxQP);

// 取消应用接口AKV_Encoder_Apply_Quality设置到编码器的最小最大QP
T_VOID  AKV_Encoder_Disable_Apply_Quality(T_pVOID hVEnc);

// 获取调整I帧大小的相关参数, MinFrameSize and MaxFrameSize (单位为kbytes) 
T_VOID  AKV_Encoder_Get_IFrameSize_AdjustmentParameters(T_pVOID hVEnc, T_U16 *MinIQP, T_U16 *MaxIQP, T_U32 *MinFrameSize, T_U32 *MaxFrameSize);

// 设置调整I帧大小的相关参数, MinQP<=MinIQP<=MaxIQP<=MaxQP, MinFrameSize<MaxFrameSize and MaxFrameSize * 1024 < StreamBufferSize, 当MinIQP=0 or MaxIQP=0,and  MinFrameSize = 0 or MaxFrameSize=0, 则不使用这些参数调整I帧大小

T_VOID  AKV_Encoder_Set_IFrameSize_AdjustmentParameters(T_pVOID hVEnc, T_U16 MinIQP, T_U16 MaxIQP, T_U32 MinFrameSize, T_U32 MaxFrameSize);
/**
 * @brief Limit the size of frame strictly
 * @param	hVEnc				[in]	encoder handle
 * @param	uMaxSize			[in]	max frame size(unit is kbytes), 0 <= uMaxSize(KB) * 1024 < StreamBufferSize, when uMaxSize==0, will disable frame size limit
 * @return T_VOID
 */
T_VOID  AKV_Encoder_StrictlyLimit_FrameSize(T_pVOID hVEnc, T_U32 uMaxSize);

// todo: 
// 1. 输入输出都需要有时间戳
// 2. 输出stream应该是完整的一帧的码流，并说明编码类型


/* 调用示例

T_pVOID hVEnc;

main()
{
    T_AKVLIB_CB init_cb_fun;
    T_AKV_ENC_OPEN_INPUT open_input;
    T_AKV_DMA_BUFFER *pFrame;
    T_AKV_STREAM_BUFFER *pStream;
    
    // todo: set init_cb_fun

    // 初始化编码器
    AKV_Encoder_Init(&init_cb_fun);

    // 开启一个码流
    open_input.sCfgFileName = "/mnt/test.cfg";
    AKV_Encoder_Get_Parameters(AK_NULL, &open_input.encPar); // 填入合法的无效参数
    open_input.encPar.EncFrameRate = 25; // 填入启动参数
    open_input.encPar.GopLength = 50;
    hVEnc = AKV_Encoder_Open(&open_input);

    // 编码
    while (have_yuv_frame)
    {
        
        // todo: fill yuv frame into pFrame
        // todo: 准备一个stream 缓冲区pStream

        // 编码一帧
        AKV_Encoder_Process(hVEnc, pFrame, pStream);

        // todo: 处理pStream中的码流数据
    }

    // 改变参数
	T_AKVLIB_PAR params;
    AKV_Encoder_Get_Parameters(hVEnc, &params);
    params.EncFrameRate = 10; // 填入新的参数
    params.GopLength = 20;
    AKV_Encoder_Set_Parameters(hVEnc, &params);

    // 继续编码

    // 码流结束
    AKV_Encoder_Close(hVEnc);
    
    // 关闭编码器
    AKV_Encoder_Destory();
}

*/


#ifdef __cplusplus
}
#endif

#endif

