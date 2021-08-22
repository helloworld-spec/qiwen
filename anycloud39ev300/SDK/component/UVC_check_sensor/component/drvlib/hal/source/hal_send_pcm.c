/**
 * @FILENAME: hal_send_pcm.c
 * @BRIEF the source code of analog controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-29
 * @VERSION 1.0
 */
#include "l2.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_sound.h"
#include "anyka_cpu.h"
//#include "hal_timer.h"
#include "drv_timer.h"
#include "drv_module.h"
#include "interrupt.h"

#define BUF_MAX_NUM     100
#define SENDPCM_MESSAGE 1
#define SENDPCM_START_ADC_MESSAGE 2
#define aec_buf_size (1024*2)


typedef struct
{
    signed short *pBufAddr;            ///< the address of buffer
    unsigned long dataLen;              ///< buffer length   
}T_SENDPCM_BUF;

static bool  m_bTaskInit = false;

typedef struct
{
    T_SENDPCM_BUF  Buf[BUF_MAX_NUM];///< data buffer
    unsigned char    rpIndex;            ///< write index
    unsigned char    wpIndex;            ///< read index     
    unsigned long   BufNum;             ///< buffer number
    unsigned long   OneBufSize;         ///< buffer size
    unsigned char    L2Buf_id;
    bool  bL2Start;
	bool bL2Stop;
    T_fSOUND SendFinish;
    signed short   *pNullBuf;       //for send null data while not data to send
}T_SENDPCM_BUF_MSG;

static T_SENDPCM_BUF_MSG m_sendpcm_msg;

static volatile bool recvpcm_state = false ;//adc开始获取数据的标志
static volatile bool sendpcm_aec_enable_flag = false;//上层启动aec的标志
static volatile bool dac_call_flag = false;//防止中断回调里多次调用recvpcm_start这个接口
signed short aec_data[aec_buf_size] ={0};//当启动aec时，dac的l2 bufwei空时，直接把这数据送进去
signed long  time_id;


static void sendpcm_l2_callback(void);
static bool sendpcm_l2_init( void );

bool sendpcm_free ( void );
bool sendpcm_setcallback (T_fSOUND setcallback);


extern void recvpcm_start(void);
extern bool recvpcm_adc_state(void);
extern bool pcm_data_transform(signed short *data,unsigned long dataLen);

static void delay_start_recvpcm(signed long timer_id, unsigned long delay)
{
	recvpcm_start();////调用recvpcm_start接口，启动adc开始接收数据
}
static void sendpcm_adc_handler(unsigned long *param, unsigned long len)
{
   // time_id = vtimer_start(20, false, delay_start_recvpcm);//启动定时器，并且定时20ms
}

/**
 * @brief  L2 callback,it will be call while one buffer send finished
 * @author LianGenhui
 * @date   2010-07-29
*/
static void sendpcm_l2_callback(void)
{
    unsigned long read_index,get_index;
	//signed long  tmr;
    if(BUF_NULL == m_sendpcm_msg.L2Buf_id)
        return;

    DrvModule_Send_Message(DRV_MODULE_DA, SENDPCM_MESSAGE, NULL);

    //send data to pcm 
    if(m_sendpcm_msg.rpIndex == m_sendpcm_msg.wpIndex )
    {   
    	/*
		这里是当aec启动的情况下，buf为空时，才会执行

    	     */
		if(sendpcm_aec_enable_flag && recvpcm_state && (false == m_sendpcm_msg.bL2Stop))
		{
	        //send 0  data buffer
			pcm_data_transform(aec_data, m_sendpcm_msg.OneBufSize);//把0数据送进去
		
	        l2_combuf_dma((unsigned long)aec_data, m_sendpcm_msg.L2Buf_id, m_sendpcm_msg.OneBufSize, MEM2BUF, true);
			
		}
		else
		{
			//akprintf(C3, M_DRVSYS, "########d0: %d\n", get_tick_count());
        	m_sendpcm_msg.bL2Start = false;
		}
    }
    else
    {
    	//判断是否需要执行aec的相关操作
    	if(recvpcm_state && sendpcm_aec_enable_flag && dac_call_flag)
		{
			//akprintf(C3, M_DRVSYS, "1.#adc\n");
			/*
			通过抛消息的方式来启动定时器，
			定时器的作用是当执行aec，又是dac先启动
			的情况下，为了时序对的上，需要开启定
			时器，20ms后在启动adc，让adc的启动差不多
			对上dac的下一次中断，而不会导致adc采集的数据超前
			*/
			time_id = timer_start(uiTIMER0,30, false, delay_start_recvpcm);
			//DrvModule_Send_Message(DRV_MODULE_DA, SENDPCM_START_ADC_MESSAGE, NULL);
			dac_call_flag = false;////置反，为了防止多次进入
		}
    	if(false == m_sendpcm_msg.bL2Stop)
    	{
	        //send next data buffer
	        read_index = m_sendpcm_msg.rpIndex;	
			//akprintf(C3, M_DRVSYS, "#d1: %d\n", get_tick_count());
			/*判断是否需要执行aec的相关操作*/
			if(sendpcm_aec_enable_flag && recvpcm_state)
			{			
				//调用接口把数据送进去aec_buf
				pcm_data_transform(m_sendpcm_msg.Buf[read_index].pBufAddr, m_sendpcm_msg.Buf[read_index].dataLen);			
			}
	        l2_combuf_dma((unsigned long)m_sendpcm_msg.Buf[read_index].pBufAddr, m_sendpcm_msg.L2Buf_id, m_sendpcm_msg.Buf[read_index].dataLen, MEM2BUF, true);
			
			m_sendpcm_msg.rpIndex = (m_sendpcm_msg.rpIndex + 1) % m_sendpcm_msg.BufNum;
		}
    }

}

static void sendpcm_handler(unsigned long *param, unsigned long len)
{
    if(m_sendpcm_msg.SendFinish != NULL)
        m_sendpcm_msg.SendFinish ();//callback,send one buffer data finished 
}



/**
 * @brief  initial l2,alloc L2 buffer,set l2 callback
 * @author LianGenhui
 * @date   2010-07-29
 */
static bool sendpcm_l2_init( void )
{
    signed long status;

    //alloc l2 buffer
    m_sendpcm_msg.L2Buf_id = l2_alloc(ADDR_DAC);

    if(BUF_NULL == m_sendpcm_msg.L2Buf_id)
    {
        akprintf(C2, M_DRVSYS, "alloc L2 buffer failed!, buf=%d\n");
        return false;
    }    
    
    //set L2 callback function
    l2_set_dma_callback (m_sendpcm_msg.L2Buf_id, sendpcm_l2_callback);

    return true;
}

static void parameter_init(void)
{
    m_sendpcm_msg.L2Buf_id = BUF_NULL;
    m_sendpcm_msg.SendFinish = NULL;
    m_sendpcm_msg.bL2Start = false;
	m_sendpcm_msg.bL2Stop = false;
    m_sendpcm_msg.pNullBuf = NULL;
}

/**
 * @brief  malloc sendpcm buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @param[in] OneBufSize one buffer size
 * @param[in] BufNum buffer number
 * @return bool
 */
bool sendpcm_malloc (unsigned long OneBufSize, unsigned long BufNum)
{
    unsigned long i;
    unsigned long j;

    if(((OneBufSize % 512) != 0) || (BufNum < 4))
    {
        akprintf(C1, M_DRVSYS, "OneBufSize %% 512 != 0 or BufNum < 4\r\n");
        return false;
    }

    if(m_bTaskInit)
    {
        akprintf(C1, M_DRVSYS, "can't malloc again,please free!\n");
        return false;
    }

    DrvModule_Protect(DRV_MODULE_DA); 
    
    parameter_init();
    
    //create send pcm task
    if(!DrvModule_Create_Task(DRV_MODULE_DA))
    {
       DrvModule_UnProtect(DRV_MODULE_DA); 
       return false;
    }

    //set massage and handler
    DrvModule_Map_Message(DRV_MODULE_DA, SENDPCM_MESSAGE, sendpcm_handler);
    //DrvModule_Map_Message(DRV_MODULE_DA, SENDPCM_START_ADC_MESSAGE, sendpcm_adc_handler);

    //malloc all buffer and set to 0
    for (i=0; i<BufNum; i++)
    {
        m_sendpcm_msg.Buf[i].pBufAddr = (signed short *)drv_malloc(OneBufSize);

        if (NULL ==  m_sendpcm_msg.Buf[i].pBufAddr)//malloc fail
        {
            for (j=0; j<i; j++)//free the mallocated memories
            {
                drv_free(m_sendpcm_msg.Buf[j].pBufAddr);
                m_sendpcm_msg.Buf[j].pBufAddr = NULL;
            }

            akprintf(C1, M_DRVSYS, "malloc fail!\n");
            DrvModule_UnProtect(DRV_MODULE_DA); 
            return false;
        }
        memset(m_sendpcm_msg.Buf[i].pBufAddr, 0, OneBufSize);
    } 

    //malloc null buffer,for send 0 data to pcm while not data to send
    m_sendpcm_msg.pNullBuf = (signed short *)drv_malloc(OneBufSize);
    if(NULL == m_sendpcm_msg.pNullBuf)
    {
        akprintf(C1, M_DRVSYS, "malloc fail!\n");
        m_sendpcm_msg.pNullBuf = NULL;
        for(i=0; i<BufNum; i++)
        {
            drv_free(m_sendpcm_msg.Buf[i].pBufAddr);
            m_sendpcm_msg.Buf[i].pBufAddr = NULL;
        }
        DrvModule_UnProtect(DRV_MODULE_DA); 
        return false;
     }
    memset(m_sendpcm_msg.pNullBuf, 0, OneBufSize);//set to 0 for null buffer

    m_sendpcm_msg.rpIndex = 0; //set default for read point index
    m_sendpcm_msg.wpIndex = 0;
    m_sendpcm_msg.BufNum = BufNum;
    m_sendpcm_msg.OneBufSize = OneBufSize;
    m_sendpcm_msg.bL2Start = false;
	m_sendpcm_msg.bL2Stop = false;

    //L2 to PCM initial ,include DAC and IIS send
    sendpcm_l2_init();
    
    m_bTaskInit = true;

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return true;
}

/**
 * @brief  realloc sendpcm buffer
 * @author liao_zhijun
 * @date   2010-11-02
 * @param[in] OneBufSize one buffer size
 * @param[in] BufNum buffer number
 * @return bool
 */
bool sendpcm_realloc (unsigned long OneBufSize, unsigned long BufNum, T_fSOUND callback)
{
    bool ret;

    sendpcm_free();
    ret = sendpcm_malloc(OneBufSize, BufNum);

    sendpcm_setcallback(callback);

    return ret;
}

/**
 * @brief  free sendpcm buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @return bool
 */
bool sendpcm_free ( void )
{
    unsigned long i;
    DrvModule_Protect(DRV_MODULE_DA); 
    
    m_sendpcm_msg.L2Buf_id = BUF_NULL;
    l2_free(ADDR_DAC);
    
    DrvModule_Terminate_Task(DRV_MODULE_DA);//release DA task
    
    for(i=0; i<m_sendpcm_msg.BufNum; i++)
    {
        drv_free(m_sendpcm_msg.Buf[i].pBufAddr);
        m_sendpcm_msg.Buf[i].pBufAddr = NULL;
    }
    m_sendpcm_msg.BufNum = 0;
    m_sendpcm_msg.OneBufSize = 0;

    //free null buf
    drv_free(m_sendpcm_msg.pNullBuf);
    m_sendpcm_msg.pNullBuf = NULL;
    m_bTaskInit = false;
    m_sendpcm_msg.bL2Start = false;
	sendpcm_aec_enable_flag = false;
	recvpcm_state = false;
	dac_call_flag = false;
	timer_stop(time_id);
    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return true;
}

/**
 * @brief  set callback function
 * @author LianGenhui
 * @date   2010-07-29
 * @param[in] setcallback callback function,it will be call when one buffer send finished
 * @return bool
 */
bool sendpcm_setcallback (T_fSOUND setcallback)
{
    if(NULL == setcallback)
    return false;

    m_sendpcm_msg.SendFinish = setcallback;
    return true;
}
 
/**
 * @brief  clean  buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @return void
 */
void sendpcm_cleanbuf ( void )
{
    unsigned long i;
    unsigned long delay_cnt = 0;

    DrvModule_Protect(DRV_MODULE_DA); 

    while (1)
    {
        if (false == m_sendpcm_msg.bL2Start)
            break;
        else 
        {
            mini_delay(1);
            delay_cnt++;
            if (delay_cnt > 300)
            {
                akprintf(C3, M_DRVSYS, "clean buf timeout\n");
                break;
            }
        }
    }       
    
    for(i=0; i<m_sendpcm_msg.BufNum; i++)
    {
       memset(m_sendpcm_msg.Buf[i].pBufAddr, 0, m_sendpcm_msg.OneBufSize);
    }

    m_sendpcm_msg.rpIndex = 0; //set define for read point index
    m_sendpcm_msg.wpIndex = 0;
    
    DrvModule_UnProtect(DRV_MODULE_DA); 
}

/**
* @brief  judge  aecopen
* @author 
* @date   2017-07-27
* @param[in] aec_open, use to judge whether aec open or not,if open,sendpcm_aec_enable_flag = true,
*                   then it will execute aec operation.
* @return bool
*/
bool sendpcm_judge_aecopen(bool aec_open)
{
	sendpcm_aec_enable_flag = aec_open;
	return true;
}

 
/**
 * @brief  get buffer address and buffer len, which can be used to fill or retrieve dac data
 * @author LianGenhui
 * @date   2010-07-29
 * @param[out] pbuf return buffer address or NULL
 * @param[out] len return buffer len or 0
 * @return bool
 * @retval true  get buffer successful
 * @retval false get buffer failed
 * @note   if sendpcm_create failed or no buffer to return, it will return failed
 */
bool sendpcm_getbuf ( void **pbuf, unsigned long *len )
{
    unsigned long bufHead;
    
    DrvModule_Protect(DRV_MODULE_DA); 
	/*
	这里是调用接口判断recvpcm  start是否已经
	准备好，由于不确定recvpcm  start具体什么
	时候准备好，所以需要不停的调用
	*/
	recvpcm_state = recvpcm_adc_state();
	/*
	dac_call_flag 这个标志是为了在中断回调里面调用接口
	启动recvpcm_start时不会多次调用
	*/
	if(!recvpcm_state)
	{
		dac_call_flag = !recvpcm_state;
	}
    if(((m_sendpcm_msg.wpIndex +1) % m_sendpcm_msg.BufNum) ==  m_sendpcm_msg.rpIndex )
    {
        *pbuf = NULL;
        *len = 0;
        DrvModule_UnProtect(DRV_MODULE_DA); 
		//akprintf(C3, M_DRVSYS, "w:%d, r:%d, jk:%d\n", m_sendpcm_msg.wpIndex, m_sendpcm_msg.rpIndex, jk);
        return false;
    }

    bufHead = m_sendpcm_msg.wpIndex;

    *pbuf = m_sendpcm_msg.Buf[bufHead].pBufAddr;
    *len = m_sendpcm_msg.OneBufSize;
	
    DrvModule_UnProtect(DRV_MODULE_DA); 
    return true;
}
/**
* @brief  get sendpcm dac state
* @author 
* @date   2017-07-27
* @param[] this funs as extern funs ,main use to be call by adc to judge whether senddata start or not,     
*                only in order to adc pcmdata and dac pcmdata Synchronization for aec.
* @return bool
*/
bool sendpcm_dac_state(void)
{
	if(m_sendpcm_msg.bL2Start)
		return true;
	else
		return false;
}

/**
* @brief  sendpcm_start
* @author 
* @date   2017-07-27
* @param[] this funs is first to send pcmdata would be call  or aec enable and adc start earlier than
*                dac start ,then it will be call in adc  interrupt callback
* @return 
*/
void sendpcm_start(void)
{
    unsigned long idx;
	idx = m_sendpcm_msg.rpIndex;
			
	m_sendpcm_msg.rpIndex = (m_sendpcm_msg.rpIndex + 1) % m_sendpcm_msg.BufNum;
	
	//akprintf(C3, M_DRVSYS, "#d2: %d\n", get_tick_count());
	l2_combuf_dma((unsigned long)m_sendpcm_msg.Buf[idx].pBufAddr, m_sendpcm_msg.L2Buf_id, 
    m_sendpcm_msg.Buf[idx].dataLen, MEM2BUF, true);

}
/**
 * @brief set one buffer end\n
 * after call sendpcm_getbuf and finish the operation of dac data,call this function
 * @author  LianGenhui
 * @date    2010-07-29 
 * @param[in] len   buffer len(use for write)
 * @return bool
 * @retval true  successful
 * @retval false longer than one buffer's len
 */
bool sendpcm_endbuf ( unsigned long len )
{
    unsigned long idx;
    if(len > m_sendpcm_msg.OneBufSize)
        return false;

    DrvModule_Protect(DRV_MODULE_DA); 

    //save this buffer len
    m_sendpcm_msg.Buf[m_sendpcm_msg.wpIndex].dataLen = len;

    //point to next buffer
    m_sendpcm_msg.wpIndex = (m_sendpcm_msg.wpIndex + 1) % m_sendpcm_msg.BufNum;

    if(!m_sendpcm_msg.bL2Start)
    { 
    	m_sendpcm_msg.bL2Start = true;	
		if((false == m_sendpcm_msg.bL2Stop))	
		{
			/*
			这里的判断是如果sendpcm_aec_enable_flag = TRUE，
			说明aec 已经open，但是如果adc并没有开始
			采集数据，就是false == recvpcm_state，然后代码
			可以正常的继续往下跑
			*/
			if((false == sendpcm_aec_enable_flag) || (false == recvpcm_state))
			{
				sendpcm_start();
			}		
		}
    }

    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return true;
}

/**
 * @brief get the number of buffers which have been filled or retrieved dac data 
 * @author LianGenhui
 * @date   2010-07-29
 * @return unsigned long
 * @retval value the value will from 0 to the number when create a dac set 
  */
unsigned long sendpcm_getnum_fullbuf( void )
{
    unsigned long num_fullbuf;
    DrvModule_Protect(DRV_MODULE_DA); 
    
    if( m_sendpcm_msg.wpIndex < m_sendpcm_msg.rpIndex )
    {
        num_fullbuf =  m_sendpcm_msg.wpIndex + m_sendpcm_msg.BufNum -  m_sendpcm_msg.rpIndex;
    }
    else
    {
         num_fullbuf =  m_sendpcm_msg.wpIndex -  m_sendpcm_msg.rpIndex;
    }
    
    DrvModule_UnProtect(DRV_MODULE_DA); 
    
    return num_fullbuf;
}
/**

*/
bool sendpcm_pause_send(void)
{
	m_sendpcm_msg.bL2Stop = true;
	return true;
	//l2_combuf_stop_dma(m_sendpcm_msg.L2Buf_id);
}
bool sendpcm_resume_send(void)
{
	unsigned long read_index;
	
	DrvModule_Protect(DRV_MODULE_DA); 
	
	m_sendpcm_msg.bL2Stop = false;

	if(m_sendpcm_msg.bL2Start == true)
	{
		read_index = m_sendpcm_msg.rpIndex;
	    l2_combuf_dma((unsigned long)m_sendpcm_msg.Buf[read_index].pBufAddr, m_sendpcm_msg.L2Buf_id, m_sendpcm_msg.Buf[read_index].dataLen, MEM2BUF, true);

		m_sendpcm_msg.rpIndex = (m_sendpcm_msg.rpIndex + 1) % m_sendpcm_msg.BufNum;
	}
	DrvModule_UnProtect(DRV_MODULE_DA); 
	return true;
}

