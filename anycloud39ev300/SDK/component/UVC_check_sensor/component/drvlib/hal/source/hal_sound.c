/**
 * @FILENAME: hal_sound.c
 * @BRIEF the source code of analog controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-27
 * @VERSION 1.0
 */

#include "l2.h"
#include "drv_api.h"
#include "hal_sound.h"

extern bool adc_open(void);
extern bool adc_close(void);
extern bool adc_setinfo(SOUND_INFO *info);
extern bool recvpcm_judge_aecopen(bool aec_open);
extern bool recvpcm_get_aecbuf(unsigned char *aecdata,unsigned long *ts, unsigned long *len);
extern bool recvpcm_malloc (unsigned long OneBufSize, unsigned long BufNum);
extern bool recvpcm_realloc (unsigned long OneBufSize, unsigned long BufNum, T_fSOUND callback);
extern bool recvpcm_setcallback (T_fSOUND setcallback);
extern bool recvpcm_free ( void );
extern void recvpcm_cleanbuf ( void );
extern unsigned long  recvpcm_getnum_fullbuf( void );
extern bool recvpcm_endbuf ( unsigned long len );
extern bool recvpcm_getbuf ( void **pbuf, unsigned long *len );

extern bool dac_open(void);
extern bool dac_close(void);
extern bool dac_setinfo(SOUND_INFO *info);
extern bool dac_setinfo_select(bool option);
extern bool sendpcm_malloc (unsigned long OneBufSize, unsigned long BufNum);
extern bool sendpcm_realloc (unsigned long OneBufSize, unsigned long BufNum, T_fSOUND callback);
extern bool sendpcm_setcallback (T_fSOUND setcallback);
extern bool sendpcm_free ( void );
extern void sendpcm_cleanbuf ( void );
extern unsigned long  sendpcm_getnum_fullbuf( void );
extern bool sendpcm_endbuf ( unsigned long len );
extern bool sendpcm_pause_send(void);
extern bool sendpcm_resume_send(void);
extern bool sendpcm_judge_aecopen(bool aec_open);
extern bool sendpcm_getbuf ( void **pbuf, unsigned long *len );

extern bool i2s_send_open(void);
extern bool i2s_send_close(void);
extern bool i2s_recv_open(void);
extern bool i2s_recv_close(void);
extern bool i2s_send_setinfo(SOUND_INFO *info);
extern bool i2s_recv_setinfo(SOUND_INFO *info);

static T_SOUND_DRV adc_function_handler = 
{
    adc_open,
    adc_close,
    adc_setinfo,
    NULL,
    recvpcm_malloc,
    recvpcm_realloc,
    recvpcm_setcallback,
    recvpcm_free,
    recvpcm_cleanbuf,
    recvpcm_getbuf,
    recvpcm_endbuf,
    NULL,
	NULL,
	recvpcm_judge_aecopen,
	recvpcm_get_aecbuf,
    recvpcm_getnum_fullbuf
};

static T_SOUND_DRV dac_function_handler = 
{
    dac_open,
    dac_close,
    dac_setinfo,
    dac_setinfo_select,
    sendpcm_malloc,
    sendpcm_realloc,
    sendpcm_setcallback,
    sendpcm_free,
    sendpcm_cleanbuf,
    sendpcm_getbuf,
    sendpcm_endbuf,
    sendpcm_pause_send,
	sendpcm_resume_send,
	sendpcm_judge_aecopen,
	NULL,
    sendpcm_getnum_fullbuf
};

static T_SOUND_DRV i2s_send_function_handler = 
{
    i2s_send_open,
    i2s_send_close,
    i2s_send_setinfo,    
    NULL,
    sendpcm_malloc,
    sendpcm_realloc,
    sendpcm_setcallback,
    sendpcm_free,
    sendpcm_cleanbuf,
    sendpcm_getbuf,
    sendpcm_endbuf,
    NULL,
	NULL,
	NULL,
	NULL,
    sendpcm_getnum_fullbuf

};

static T_SOUND_DRV i2s_recv_function_handler = 
{
    i2s_recv_open,
    i2s_recv_close,
    i2s_recv_setinfo,    
    NULL,
    recvpcm_malloc,
    recvpcm_realloc,
    recvpcm_setcallback,
    recvpcm_free,
    recvpcm_cleanbuf,
    recvpcm_getbuf,
    recvpcm_endbuf,
    NULL,
	NULL,
	NULL,
	NULL,
    recvpcm_getnum_fullbuf
};


static T_SOUND_DRV *array_sound_function[] = {
    &adc_function_handler,
    &dac_function_handler,
    &i2s_send_function_handler,
    &i2s_recv_function_handler,
    NULL
};

/**
 * @brief   create a sound driver, it will malloc sound buffer and init L2\n
 *          the callback function will be called when a read or write buffer complete, it can be NULL and do nothing.
 * @author  LianGenhui
 * @date    2010-07-27
 * @param[in] driver sound driver, refer to SOUND_DRIVER
 * @param[in] OneBufSize buffer size
 * @param[in] DABufNum buffer number
 * @param[in] callback callback function or NULL 
 * @return  T_SOUND_DRV
 * @retval  NULL created failed
 */
T_SOUND_DRV *sound_create (SOUND_DRIVER driver,unsigned long OneBufSize, unsigned long DABufNum, T_fSOUND callback)
{
    if((driver >= SOUND_DRIVER_MAX) || (NULL == array_sound_function[driver]))
    {
        return NULL;
    }

    if((NULL == array_sound_function[driver]->sound_malloc_func) ||
        (NULL == array_sound_function[driver]->sound_setcallback_func))
    {
        return NULL;
    }

    if(false == array_sound_function[driver]->sound_malloc_func(OneBufSize, DABufNum))
    {
        return NULL;
    }
    
    array_sound_function[driver]->sound_setcallback_func(callback);

    return array_sound_function[driver];//adc:0,dac:1,2:IIS send,3:IIS receive
}




/**
 * @brief  realloc buffer for giving sound driver
 * @author    liao_zhijun
 * @date    2010-11-02
 * @param[in] handler handler of the sound device
 * @param[in]  OneBufSize buffer size
 * @param[in]  DABufNum buffer number
 * @return  bool
 * @retval  true  realloc successful
 * @retval  false realloc failed
 */
bool sound_realloc (T_SOUND_DRV *handler,unsigned long OneBufSize, unsigned long DABufNum, T_fSOUND callback)
{
    if((NULL == handler) || (NULL == handler->sound_realloc_func))
        return false;

    if(handler->sound_realloc_func != NULL)
        return(handler->sound_realloc_func(OneBufSize, DABufNum, callback));
    else
        return false;    
}


/**
 * @brief  delete sound driver and Free sound buffer
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device
 */
void sound_delete (T_SOUND_DRV *handler)
{
    if((NULL == handler) || (NULL == handler->sound_free_func))
        return;
    
    handler->sound_free_func();
}

/**
 * @brief  open a sound device and it can be used
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @return bool
 * @retval true  open successful
 * @retval false open failed
 */
bool sound_open (T_SOUND_DRV *handler)
{
    if(NULL == handler)
    {
        return false;
    }

    if(handler->sound_open_func != NULL)
        return(handler->sound_open_func());
    else
        return false;
}

/**
 * @brief  Close a sound device
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @return bool
 * @retval true close successful
 * @retval false close failed
 */
bool sound_close (T_SOUND_DRV *handler)
{
    if(NULL == handler)
    {
        return false;
    }

    if(handler->sound_close_func != NULL)
        return(handler->sound_close_func());
    else
        return false;
}

bool sound_get_aecbuf(T_SOUND_DRV *handler, unsigned char *aecdata,unsigned long *ts, unsigned long *len)
{
	if(NULL == handler)
    {
        return false;
    }
	if(handler->sound_get_aecbuf_funs != NULL)
		return(handler->sound_get_aecbuf_funs(aecdata,ts, len));
	else
		return false;

}

bool sound_judge_aec (T_SOUND_DRV *handler,bool option)
{
	if(NULL == handler)
    {
        return false;
    }
	if(handler->sound_judge_aecopen_funs != NULL)
		return(handler->sound_judge_aecopen_funs(option));
	else
		return false;
}

bool sound_setinfo_select(T_SOUND_DRV *handler, bool option)
{
	if(NULL == handler)
    {
        return false;
    }
	if(handler->sound_setinfo_select_funs != NULL)
		return(handler->sound_setinfo_select_funs(option));
	else
		return false;
}

/**
 * @brief  Set sound sample rate, channel, bits per sample of the sound device
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @param[in] info     refer to SOUND_INFO
 * @return bool
 * @retval true set successful
 * @retval false set failed
 */
bool sound_setinfo (T_SOUND_DRV *handler, SOUND_INFO *info)
{
    if(NULL == handler)
    {
        return false;
    }
    
    if(handler->sound_setinfo_func != NULL)
        return(handler->sound_setinfo_func(info));
    else
        return false;
}

/**
 * @brief  clean sound buffer
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @return void
 */
void sound_cleanbuf (T_SOUND_DRV *handler)
{
    if(NULL == handler)
    {
        return;
    }

    if(handler->sound_cleanbuf_func != NULL)
        handler->sound_cleanbuf_func();
}

/**
 * @brief  get buffer address and buffer len, which can be used to fill or retrieve sound data
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @param[out] pbuf return buffer address or NULL
 * @param[out] len return buffer len or 0
 * @return bool
 * @retval true  get buffer successful
 * @retval false get buffer failed
 * @note   if sound_create failed or no buffer to return, it will return failed
 */
bool sound_getbuf (T_SOUND_DRV *handler, void **pbuf, unsigned long *len)
{
    if(NULL == handler)
    {
        return false;
    }

    if(handler->sound_getbuf_func != NULL)
        return(handler->sound_getbuf_func((void *)((signed short *)pbuf), len));
    else
        return false;
}

/**
 * @brief set one buffer end\n
 * after call sound_getbuf and finish the operation of sound data,call this function
 * @author  LianGenhui
 * @date    2010-07-27 
 * @param[in] handler handler of the sound device  
 * @param[in] len     buffer len(use for write)
 * @return bool
 * @retval true  successful
 * @retval false longer than one buffer's len
 */
bool sound_endbuf (T_SOUND_DRV *handler, unsigned long len)
{
    if(NULL == handler)
    {
        return false;
    }
    
    if(handler->sound_endbuf_func != NULL)
        return(handler->sound_endbuf_func(len));
    else
        return false;
}

bool sound_pause(T_SOUND_DRV *handler)
{
	if(NULL == handler)
    {
        return false;
    }
	if(handler->sound_pause_func != NULL)
        return(handler->sound_pause_func());
    else
        return false;
}

bool sound_resume(T_SOUND_DRV *handler)
{
	if(NULL == handler)
    {
        return false;
    }
	if(handler->sound_resume_func != NULL)
        return(handler->sound_resume_func());
    else
        return false;
}
/**
 * @brief get the number of buffers which have been filled or retrieved sound data 
 * @author LianGenhui
 * @date   2010-07-27
 * @param[in] handler handler of the sound device  
 * @return unsigned long
 * @retval value the value will from 0 to the number when create a sound set 
  */
unsigned long sound_getnum_fullbuf (T_SOUND_DRV *handler)
{
    unsigned long num = 0;
    
    if(NULL == handler)
    {
        return false;
    }

    if(handler->sound_getnum_fullbuf_func != NULL)
        num = handler->sound_getnum_fullbuf_func();
 
    return num;
}


