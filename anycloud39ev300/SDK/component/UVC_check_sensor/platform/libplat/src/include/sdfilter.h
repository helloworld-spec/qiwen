/**
 * @file	sdfilter.h
 * @brief	Anyka Sound Device Module interfaces header file.
 *
 * This file declare Anyka Sound Device Module interfaces.\n
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author	Deng Zhou
 * @date	2008-04-10
 * @version V0.0.1
 * @ref
 */

#ifndef __SOUND_FILTER_H__
#define __SOUND_FILTER_H__

#include "medialib_global.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup Audio Filter library
 * @ingroup ENG
 */
/*@{*/

/* @{@name Define audio version*/
/**	Use this to define version string */	
#define AUDIO_FILTER_VERSION		(T_U8 *)"AudioFilter Version V1.9.00_svn5145"
#ifdef AUDIOLIB_LINK_AUTOMATIC
#   define AUDIO_FILTER_VERSION_STRING (AUDIO_FILTER_VERSION " AUTOLINK")
#else
#   define AUDIO_FILTER_VERSION_STRING AUDIO_FILTER_VERSION
#endif
/** @} */

#ifdef _WIN32
#define _SD_FILTER_EQ_SUPPORT    
#define _SD_FILTER_WSOLA_SUPPORT   
#define _SD_FILTER_3DSOUND_SUPPORT
#define _SD_FILTER_RESAMPLE_SUPPORT
#define _SD_FILTER_DENOISE_SUPPORT
#define _SD_FILTER_AGC_SUPPORT
#define _SD_FILTER_VOICECHANGE_SUPPORT
#define _SD_FILTER_REECHO_SUPPORT
#define _SD_FILTER_PCMMIXER_SUPPORT
#define _SD_FILTER_3DENHANCE_SUPPORT
#define _SD_FILTER_MVBASS_SUPPORT
#define _SD_FILTER_ASLC_SUPPORT
// #define _SD_FILTER_TONEDETECTION_SUPPORT
#define _SD_FILTER_VOLUMECONTROL_SUPPORT
#endif


    
#define _SD_VOLCTL_VOLDB_Q  10
#define _SD_EQ_MAX_BANDS    10
#define _SD_REECHO_DECAY_Q  10

typedef enum
{
	_SD_FILTER_UNKNOWN ,
	_SD_FILTER_EQ ,
	_SD_FILTER_WSOLA ,
	_SD_FILTER_RESAMPLE,
	_SD_FILTER_3DSOUND,
	_SD_FILTER_DENOICE,
	_SD_FILTER_AGC,
	_SD_FILTER_VOICECHANGE,
    _SD_FILTER_PCMMIXER,
    _SD_FILTER_3DENHANCE,
    _SD_FILTER_MVBASS,
    _SD_FILTER_ASLC,
    _SD_FILTER_TONE_DETECTION,
    _SD_FILTER_VOLUME_CONTROL,
    _SD_FILTER_REECHO,
}T_AUDIO_FILTER_TYPE;

typedef enum
{
	_SD_EQ_MODE_NORMAL,
	_SD_EQ_MODE_CLASSIC,
	_SD_EQ_MODE_JAZZ,
    _SD_EQ_MODE_POP,
    _SD_EQ_MODE_ROCK,
    _SD_EQ_MODE_EXBASS,
    _SD_EQ_MODE_SOFT,
    _SD_EQ_USER_DEFINE,
} T_EQ_MODE;

//to define the filter type
typedef enum
{
    FILTER_TYPE_NO ,
    FILTER_TYPE_HPF ,
    FILTER_TYPE_LPF ,
    FILTER_TYPE_HSF ,
    FILTER_TYPE_LSF ,
    FILTER_TYPE_PF1    //PeaKing filter
}T_EQ_FILTER_TYPE;


typedef enum
{
	_SD_WSOLA_0_5 ,
	_SD_WSOLA_0_6 ,
	_SD_WSOLA_0_7 ,
	_SD_WSOLA_0_8 ,
	_SD_WSOLA_0_9 ,
	_SD_WSOLA_1_0 ,
	_SD_WSOLA_1_1 ,
	_SD_WSOLA_1_2 ,
	_SD_WSOLA_1_3 ,
	_SD_WSOLA_1_4 ,
	_SD_WSOLA_1_5 ,
	_SD_WSOLA_1_6 ,
	_SD_WSOLA_1_7 ,
	_SD_WSOLA_1_8 ,
	_SD_WSOLA_1_9 ,
	_SD_WSOLA_2_0 
}T_WSOLA_TEMPO;

typedef enum
{
	_SD_WSOLA_ARITHMATIC_0 , // 0:WSOLA, fast but tone bab
	_SD_WSOLA_ARITHMATIC_1   // 1:PJWSOLA, slow but tone well
}T_WSOLA_ARITHMATIC;


typedef enum
{
    RESAMPLE_ARITHMETIC_0 = 0,
    RESAMPLE_ARITHMETIC_1
}RESAMPLE_ARITHMETIC;

typedef enum
{
    _SD_OUTSR_UNKNOW = 0,
	_SD_OUTSR_48KHZ = 1,
	_SD_OUTSR_44KHZ,
	_SD_OUTSR_32KHZ,
	_SD_OUTSR_24KHZ,
	_SD_OUTSR_22KHZ,
	_SD_OUTSR_16KHZ,
	_SD_OUTSR_12KHZ,
	_SD_OUTSR_11KHZ,
	_SD_OUTSR_8KHZ
}T_RES_OUTSR;

typedef enum
{
    PITCH_NORMAL = 0,
    PITCH_CHILD_VOICE ,
    PITCH_MACHINE_VOICE,
    PITCH_ECHO_EFFECT,
    PITCH_ROBOT_VOICE,
    PITCH_RESERVE
}T_PITCH_MODES;

typedef enum
{
    VOLCTL_VOL_MUTIPLE = 0,
    VOLCTL_VOL_DB = 2,
}VOLCTL_VOL_MODE;

typedef struct
{
    int num;
    struct 
    {
        int x;
        int y;
    }stone[10];
    int lookAheadTime;  //ms
    int gainAttackTime;  //ms
    int gainReleaseTime;  //ms
}T_FILTER_MILESTONE;

typedef struct
{
	MEDIALIB_CALLBACK_FUN_MALLOC			Malloc;
	MEDIALIB_CALLBACK_FUN_FREE				Free;
	MEDIALIB_CALLBACK_FUN_PRINTF			printf;
	MEDIALIB_CALLBACK_FUN_RTC_DELAY			delay;
    MEDIALIB_CALLBACK_FUN_INVALID_DCACHE    invDcache;
}T_AUDIO_FILTER_CB_FUNS;


typedef struct
{
	T_U32	m_Type;				//media type
	T_U32	m_SampleRate;		//sample rate, sample per second
	T_U16	m_Channels;			//channel number
	T_U16	m_BitsPerSample;	//bits per sample 

	union {
		struct {
			T_EQ_MODE eqmode;
            /* 
            设置总增益值(db)，注意：preGain 赋值形式为 (T_S16)(x.xxx*(1<<10))
            */
            T_S16 preGain;      //-12 <= x.xxx <= 12
			
			// For User Presets
			T_U32 bands;      //1~10
			T_U32 bandfreqs[_SD_EQ_MAX_BANDS];
            /* 
            设置每个频带的增益值，注意：bandgains 赋值形式为 (T_S16)(x.xxx*(1<<10))
            */
			T_S16 bandgains[_SD_EQ_MAX_BANDS];  //-12 <= x.xxx <= 12
            /* 
            设置每个频带的Q值，注意：
            1. bandQ赋值形式为 (T_U16)(x.xxx*(1<<10))
            2. bandQ如果设置为0，则采用库内部的默认值为 (T_U16)(1.22*(1<<10))
            3. x.xxx < 采样率/(2*该频带的中心频点), 并且x.xxx值必须小于64.000
            */
            T_U16 bandQ[_SD_EQ_MAX_BANDS];     // q < sr/(2*f)
            T_U16 bandTypes[_SD_EQ_MAX_BANDS];		
            
            /*** for ffeq dc_remove ***/
            T_U8     dcRmEna;
            T_U32    dcfb;

            /*** for EQ aslc ***/
            T_U8   aslcEna;
            T_U16  aslcLevelMax;
		} m_eq;
		struct {
			T_WSOLA_TEMPO tempo;
            T_WSOLA_ARITHMATIC arithmeticChoice;
		} m_wsola;
		struct{
			T_U8 is3DSurround;
		}m_3dsound;
		struct {
			//目标采样率 1:48k 2:44k 3:32k 4:24K 5:22K 6:16K 7:12K 8:11K 9:8K
			T_RES_OUTSR  outSrindex;

			//设置最大输入长度(bytes)，open时需要用作动态分配的依据。
			//后面具体调用重采样时，输入长度不能超过这个值
			T_U32 maxinputlen; 

            // 由于outSrindex这个限制只能是enum中的几个，当希望的目标采样率是enum之外的值的时候，用这个参数。
            // 这个参数不是采样率的索引了，直接是目标采样率的值。例如8000， 16000 ...
            // 如果想让这个参数生效，必须设置outSrindex=0
            T_U32 outSrFree; 
            
            T_U32 reSampleArithmetic;
            T_U32 outChannel;
		}m_resample;
		struct{
			T_U16 AGClevel;  // make sure AGClevel < 32767
            /* used in AGC_1 */
            T_U32  max_noise;
            T_U32  min_noise;
            /* used in AGC_2 */
            T_U8  noiseReduceDis;  // 是否屏蔽自带的降噪功能
            T_U8  agcDis;  // 是否屏蔽自带的AGC功能
            /*
            agcPostEna：在agcDis==0的情况下，设置是否真正的AGC2库里面做AGC：
            0：表示真正在库里面做agc，即filter_control出来的数据是已经做好agc的；
            1: 表示库里面只要计算agc的gain值，不需要真正做agc处理；真正的agc由外面的调用者后续处理
            */
            T_U8  agcPostEna;  
            T_U16 maxGain;  // 最大放大倍数
            T_U16 minGain;  // 最小放大倍数
            T_U32 dc_freq;  // hz
            T_U32 nr_range; // 1~300,越低降噪效果越明显
		}m_agc;
		struct{
			T_U32 ASLC_ena;  // 0:disable aslc;  1:enable aslc
			T_U32 NR_Level;  //  0 ~ 4 越大,降噪越狠
		}m_NR;
		struct{
			T_PITCH_MODES pitchMode;  // 
		}m_pitch;
        struct{
            /*是否使能混响效果，1为使能，0为关闭*/
            T_S32 reechoEna;  
            /*
            衰减因子，格式为 (T_S32)(0.xx * (1<<_SD_REECHO_DECAY_Q))
            例如要设置参数为0.32， 则给这个变量赋值 (T_S32)(0.32 * (1<<_SD_REECHO_DECAY_Q))
            */
            T_S32 degree;      //0-无混响效果
            /*
            设置房间大小，建议设置0-300 
            */
            T_U16 roomsize;    //0-采用默认值(71)。
            /*
            设置最长混响时间(ms)，即多长时间后混响消失。
            注意：这个设置的越长，需要的缓冲就越大，所以不建议设置太大。
                  一般建议设置1000以内，如果内存足够，也可以设置大些。
                  如果内存不够，则减小这个值。
            */
            T_U16 reechoTime;  //0-采用默认值(840)
            /*
            是否需要把原始主声音同时输出。 
            0: 不输出原始主声音，输出的都是反射之后的声音；
            1：主声音和反射之后的声音一起输出。
            */
            T_U8  needMainBody; //0 or 1
		}m_reecho;
        struct{
            /* 
            设置总增益值(db)，
            注意：preGain 赋值形式为 (T_S16)(x.xxx*(1<<10))， 
            限制 -12 <= x.xxx <= 12
            */
            T_S16 preGain;  
            T_S16 cutOffFreq;
            /* 
            设置3D深度，
            注意: depth赋值形式为 (T_S16)(x.xxx*(1<<10)), 
            限制 -1 < x.xxx < 1
            */
            T_S16 depth;   
            /*** for 3D Enhance's aslc, resvered***/
            T_U8   aslcEna;
            T_U16  aslcLevelMax;
		}m_3DEnhance;
        struct{
            /* 
            设置总增益值(db)，
            注意：preGain 赋值形式为 (T_S16)(x.xxx*(1<<10))， 
            限制 -12 <= x.xxx <= 12
            */
            T_S16 preGain;
            T_S16 cutOffFreq;  
            /* 
            设置增强幅度，
            注意: bassGain 赋值形式为 (T_S16)(x.xxx*(1<<10)), 
            限制 0 < x.xxx < 12
            */
            T_S16 bassGain;
            /*** for MVBass's aslc ***/
            T_U8   aslcEna;
            T_U16  aslcLevelMax;
		}m_mvBass;
        struct{
            T_BOOL aslcEna;
            T_U16  aslcLimitLevel;  //限幅阈值
            T_U16  aslcStartLevel;  //要限幅的起始能量
            /* 
            jointChannels:
               0: 两个声道独立增益计算，及独立增益处理输出；
               1: 两个声道合叠加取均值合并，然后计算一个增益，左右声道输出相同的数据；
               2: 两个声道的数据交错混合寻找最大值及增益计算，然后用一个增益值分别对左右声道做增益处理输出。
            */            
            T_U16  jointChannels;
		}m_aslc;
        struct{
            /* 
            set volume mode::
            VOLCTL_VOL_MUTIPLE: 音量值是 volume 的值，即外部传入音量倍数值
            VOLCTL_VOL_DB:      音量值是 voldb 的值， 即外部出入db值
            */
            T_U16 setVolMode;   //0:vol multiple;  2:vol db

            /* 
            设置音量倍数值, (T_U16)(x.xx*(1<<10)), x.xx=[0.00~7.99]表示倍数
            建议设置的音量值不要超过1.00*(1<<10)，因为超过可能会导致数据溢出，声音产生失真
            */
            T_U16 volume; 

            /* 
            设置音量DB, 赋值形式为(T_S32)(x.xx*(1<<10)), x.xx=[-60.00~8.00]
            建议设置的音量值不要超过0db，因为超过可能会导致数据溢出，声音产生失真
            若 x.xxx<=-79db, 则输出无声； 若x.xxx>8.0, 可能会导致输出噪音。
            */
            T_S32 voldb;

            /* 为了防止音量变换过程产生pipa音，对音量进行平滑处理，这里设置平滑的过渡时间 */
            T_U16 volSmoothTime;  //ms
		}m_volumeControl;
        struct{
            T_U32 baseFreq;
        }m_toneDetection;
	}m_Private;
}T_AUDIO_FILTER_IN_INFO;

typedef struct
{
	T_AUDIO_FILTER_CB_FUNS	cb_fun;
	T_AUDIO_FILTER_IN_INFO	m_info;
    T_AUDIO_CHIP_ID         chip;

    const T_VOID            *ploginInfo;
}T_AUDIO_FILTER_INPUT;

typedef struct
{
	T_VOID *buf_in;
	T_U32 len_in;
	T_VOID *buf_out;
	T_U32 len_out;
    T_VOID *buf_in2;  //for mix pcm samples
	T_U32 len_in2;
}T_AUDIO_FILTER_BUF_STRC;

typedef struct
{
    T_AUDIO_FILTER_CB_FUNS cb;
    T_U32	m_Type;
}T_AUDIO_FILTER_LOG_INPUT;

//////////////////////////////////////////////////////////////////////////

/**
 * @brief	获取音效处理库版本信息.
 * @author	Deng Zhou
 * @date	2009-04-21
 * @param	[in] T_VOID
 * @return	T_S8 *
 * @retval	返回音效处理库版本号
 */
T_S8 *_SD_GetAudioFilterVersionInfo(void);

/**
 * @brief	获取音效库版本信息, 包括支持哪些功能.
 * @author  Tang Xuechai
 * @date	2014-05-05
 * @param	[in] T_AUDIO_FILTER_CB_FUNS
 * @return	T_S8 *
 * @retval	返回库版本号
 */
T_S8 *_SD_GetAudioFilterVersions(T_AUDIO_FILTER_CB_FUNS *cb);

/**
 * @brief	打开音效处理设备.
 * @author	Deng Zhou
 * @date	2008-04-10
 * @param	[in] filter_input:
 * 音效处理的输入结构
 * @return	T_VOID *
 * @retval	返回音效库内部结构的指针，空表示失败
 */
T_VOID *_SD_Filter_Open(T_AUDIO_FILTER_INPUT *filter_input);

/**
 * @brief	音效处理.
 * @author	Deng Zhou
 * @date	2008-04-10
 * @param	[in] audio_filter:
 * 音效处理内部解码保存结构
 * @param	[in] audio_filter_buf:
 * 输入输出buffer结构
 * @return	T_S32
 * @retval	返回音效库处理后的音频数据大小，以byte为单位
 */
T_S32 _SD_Filter_Control(T_VOID *audio_filter, T_AUDIO_FILTER_BUF_STRC *audio_filter_buf);

/**
 * @brief	关闭音效处理设备.
 * @author	Deng Zhou
 * @date	2008-04-10
 * @param	[in] audio_decode:
 * 音效处理内部解码保存结构
 * @return	T_S32
 * @retval	AK_TRUE :  关闭成功
 * @retval	AK_FLASE :  关闭异常
 */
T_S32 _SD_Filter_Close(T_VOID *audio_filter);

/**
 * @brief	设置音效参数:播放速度,EQ模式.
 *          如果m_SampleRate,m_BitsPerSample,m_Channels三个有1个为0,则不改变任何音效,返回AK_TRUE
 * @author	Wang Bo
 * @date	2008-10-07
 * @param	[in] audio_filter:
 * 音效处理内部解码保存结构
 * @param	[in] info:
 * 音效信息保存结构
 * @return	T_S32
 * @retval	AK_TRUE :  设置成功
 * @retval	AK_FLASE :  设置异常
 */
T_S32 _SD_Filter_SetParam(T_VOID *audio_filter, T_AUDIO_FILTER_IN_INFO *info);

/**
 * @brief	设置ASLC模块的限幅曲线.
 * @author	Tang Xuechai
 * @date	2015-04-17
 * @param	[in] audio_filter: 音效处理内部解码保存结构
 * @param	[in] fmileStones: ASLC的限幅曲线参数，具体参考音频库接口说明文档
 * @return	T_S32
 * @retval	AK_TRUE :  设置成功
 * @retval	AK_FLASE :  设置异常
 */
T_S32 _SD_Filter_SetAslcMileStones(T_VOID *audio_filter, T_FILTER_MILESTONE *fmileStones);

/**
 * @brief	快速重采样
 * @author	Tang_Xuechai
 * @date	    2013-07-03
 * @param	[in] audio_filter:
 *               音效处理内部解码保存结构
 * @param	[out] dstData 
 *               输出的pcm数据
 * @param	[in] srcData:
 *               输入的pcm数据
 * @param	[in] srcLen 
 *               输入pcm数据的byte数
 * @return	T_S32
 * @retval	>=0 :  重采样后的输出pcm数据的byte数
 * @retval	<0  :  重采样失败
 */
T_S32  _SD_Filter_Audio_Scale(T_VOID *audio_filter, T_S16 dstData[], T_S16 srcData[], T_U32 srcLen);


/**
* @brief	把EQ的频域参数转为时域参数.
* @author	Tang Xuechai
* @date	    2015-03-24
* @param	[in] audio_filter:
*           音效处理内部保存结构，即_SD_Filter_Open的返回指针
* @param	[in] info:
*           音效信息保存结构
* @return	T_VOID *
* @retval	返回EQ库内部获取的时域参数的指针，空表示失败
*/
T_VOID *_SD_Filter_GetEqTimePara(T_VOID *audio_filter, T_AUDIO_FILTER_IN_INFO *info);

/**
* @brief	把当前要使用的EQ时域参数传递给EQ库.
* @author	Tang Xuechai
* @date	    2015-03-24
* @param	[in] audio_filter:
*           音效处理内部保存结构，即_SD_Filter_Open的返回指针
* @param	[in] peqTime:
*           时域参数指针
* @return	T_S32
* @retval	AK_TRUE :  设置成功
* @retval	AK_FLASE:  设置异常
*/
T_S32 _SD_Filter_SetEqTimePara(T_VOID *audio_filter, T_VOID *peqTime);

/**
* @brief	释放EQ时域参数占用的空间.
* @author	Tang Xuechai
* @date	    2015-03-24
* @param	[in] audio_filter:
*           音效处理内部保存结构，即_SD_Filter_Open的返回指针
* @param	[in] peqTime:
*           时域参数指针
* @return	T_S32
* @retval	AK_TRUE :  设置成功
* @retval	AK_FLASE:  设置异常
*/
T_S32 _SD_Filter_DestoryEqTimePara(T_VOID *audio_filter, T_VOID *peqTime);


/**
 * @brief	设置音量控制模块的音量值.
 * @author	Tang Xuechai
 * @date	2015-08-11
 * @param	[in] audio_filter: 音效处理内部解码保存结构
 * @param	[in] volume: 目标音量值。
 *    音量倍数值, (T_U16)(x.xx*(1<<10)), x.xx=[0.00~7.99]表示倍数
 *    建议设置的音量值不要超过1.00*(1<<10)，因为超过可能会导致数据溢出，声音产生失真
 * @return	T_S32
 * @retval	AK_TRUE :  设置成功
 * @retval	AK_FLASE :  设置异常
 */
T_S32 _SD_Filter_SetVolume(T_VOID *audio_filter, T_U16 volume);

/**
 * @brief	设置音量控制模块的音量值.
 * @author	Tang Xuechai
 * @date	2015-08-11
 * @param	[in] audio_filter: 音效处理内部解码保存结构
 * @param	[in] volume: 目标音量DB值。
 *    音量DB, 赋值形式为(T_S32)(x.xx*(1<<10)), x.xx=[-100.00~8.00], 在[-60.00~8.00]之间步长1db有效
 *    建议设置的音量值不要超过0db，因为超过可能会导致数据溢出，声音产生失真
 *    若 x.xxx<=-79db, 则输出无声； 若x.xxx>8.0, 可能会导致输出噪音。
 * @return	T_S32
 * @retval	AK_TRUE :  设置成功
 * @retval	AK_FLASE :  设置异常
 */

T_S32 _SD_Filter_SetVolumeDB(T_VOID *audio_filter, T_S32 volume);


const T_VOID *_SD_EQ_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_3DEnhance_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_3DSound_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_ASLC_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_mvBass_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_NR_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_AGC_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_VolCtl_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_WSOLA_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_pitch_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_Mixer_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_Reecho_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_Resample_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);
const T_VOID *_SD_toneDetection_login(T_AUDIO_FILTER_LOG_INPUT *plogInput);

#ifdef __cplusplus
}
#endif

#endif
/* end of sdfilter.h */
/*@}*/
