/**@file  hal_sound.h
 * @brief sound operation interface
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author  Liangenhui 
 * @date  2010-06-30
 * @version 1.0
 */

#ifndef         __HAL_SOUND_H
#define         __HAL_SOUND_H

/** @defgroup SOUND sound group
 *  @ingroup Drv_Lib
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief sound callback function
 */
typedef void (*T_fSOUND)( void );

/**
 * @brief sound driver module
 */
typedef enum
{
    SOUND_ADC = 0,        ///< ADC driver
    SOUND_DAC,            ///< DAC driver
    SOUND_IIS_SEND,       ///< IIS send driver
    SOUND_IIS_RECEIVE,     ///< IIS receive driver
    SOUND_DRIVER_MAX
}SOUND_DRIVER;

/**
 * @brief sound info
 */
typedef struct
{
    unsigned long    nSampleRate;    ///< SampleRate
    unsigned short    nChannel;       ///< dimensional sound(2), single channel(1)
    unsigned short    BitsPerSample;  ///< 8bit or 16 bit
}SOUND_INFO;

/**
 * @brief handler of the sound device
 */
typedef struct
{
   bool (*sound_open_func)( void ); 
    bool (*sound_close_func)( void );   
    bool (*sound_setinfo_func)( SOUND_INFO *info);	
	bool (*sound_setinfo_select_funs)(bool option);	
    bool (*sound_malloc_func) (unsigned long OneBufSize, unsigned long DABufNum);
    bool (*sound_realloc_func) (unsigned long OneBufSize, unsigned long DABufNum, T_fSOUND callback);
    bool (*sound_setcallback_func) (T_fSOUND setcallback);
    bool (*sound_free_func)( void );
    void (*sound_cleanbuf_func)( void );
    bool (*sound_getbuf_func)( void **pbuf, unsigned long *len);
    bool (*sound_endbuf_func)( unsigned long len );
	bool (*sound_pause_func)(void );
	bool (*sound_resume_func)(void );	
	bool (*sound_judge_aecopen_funs)(bool state);
	bool (*sound_get_aecbuf_funs)(unsigned char *aecdata,unsigned long *ts, unsigned long *len);
    unsigned long  (*sound_getnum_fullbuf_func)( void );
}T_SOUND_DRV;

/**
 * @brief   create a sound driver, it will malloc sound buffer and init L2\n
 *          the callback function will be called when a read or write buffer complete, it can be NULL and do nothing.
 * @author    LianGenhui
 * @date    2010-06-30
 * @param[in]  driver sound driver, refer to SOUND_DRIVER
 * @param[in]  OneBufSize buffer size
 * @param[in]  DABufNum buffer number
 * @param[in]  callback callback function or NULL 
 * @return  T_SOUND_DRV
 * @retval  NULL created failed
 */
T_SOUND_DRV *sound_create (SOUND_DRIVER driver,unsigned long OneBufSize, unsigned long DABufNum, T_fSOUND callback);


/**
 * @brief  realloc buffer for giving sound driver
 * @author    liao_zhijun
 * @date    2010-11-02
 * @param[in] handler handler of the sound device
 * @param[in]  OneBufSize buffer size
 * @param[in]  DABufNum buffer number
 * @param[in]  callback callback function or NULL 
 * @return  bool
 * @retval  true  realloc successful
 * @retval  false realloc failed
 */
bool sound_realloc (T_SOUND_DRV *handler,unsigned long OneBufSize, unsigned long DABufNum, T_fSOUND callback);

/**
 * @brief  delete sound driver and Free sound buffer
 * @author LianGenhui
 * @date   2010-06-30
 * @param[in] handler handler of the sound device
 */
void sound_delete (T_SOUND_DRV *handler);

/**
 * @brief  open a sound device and it can be used
 * @author LianGenhui
 * @date   2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  bool
 * @retval  true  open successful
 * @retval  false open failed
 */
bool sound_open (T_SOUND_DRV *handler);

/**
 * @brief   Close a sound device
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  bool
 * @retval  true close successful
 * @retval  false close failed
 */
bool sound_close (T_SOUND_DRV *handler);

/**
 * @brief   Set sound sample rate, channel, bits per sample of the sound device
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @param[in] info     refer to SOUND_INFO
 * @return  bool
 * @retval  true set successful
 * @retval  false set failed
 */
bool sound_setinfo (T_SOUND_DRV *handler, SOUND_INFO *info);

/**
 * @brief   clean sound buffer
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  void
 */
void sound_cleanbuf (T_SOUND_DRV *handler);

/**
 * @brief   get buffer address and buffer len, which can be used to fill or retrieve sound data
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @param[out] pbuf return buffer address or NULL
 * @param[out] len return buffer len or 0
 * @return  bool
 * @retval  true  get buffer successful
 * @retval  false get buffer failed
 * @note    if sound_create failed or no buffer to return, it will return failed
 */
bool sound_getbuf (T_SOUND_DRV *handler, void **pbuf, unsigned long *len);

/**
 * @brief set one buffer end\n
 * after call sound_getbuf and finish the operation of sound data,call this function
 * @author    LianGenhui
 * @date     2010-06-30 
 * @param[in] handler handler of the sound device  
 * @param[in] len     buffer len(use for write)
 * @return  bool
 * @retval  true  successful
 * @retval  false longer than one buffer's len
 */
bool sound_endbuf(T_SOUND_DRV *handler, unsigned long len);


bool sound_judge_aec (T_SOUND_DRV *handler,bool option);
bool sound_setinfo_select(T_SOUND_DRV *handler, bool option);
bool sound_get_aecbuf(T_SOUND_DRV *handler, unsigned char *aecdata,unsigned long *ts, unsigned long *len);


bool sound_pause(T_SOUND_DRV *handler);



bool sound_resume(T_SOUND_DRV *handler);


/**
 * @brief get the number of buffers which have been filled or retrieved sound data 
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  unsigned long
 * @retval  value the value will from 0 to the number when create a sound set 
  */
unsigned long sound_getnum_fullbuf(T_SOUND_DRV *handler);

#ifdef __cplusplus
}
#endif

/*@}*/

#endif //end  #ifndef         __HAL_SOUND_H


