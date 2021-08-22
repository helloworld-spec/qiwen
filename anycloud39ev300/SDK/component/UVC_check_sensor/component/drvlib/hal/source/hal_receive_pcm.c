/**
 * @FILENAME: hal_receive_pcm.c
 * @BRIEF the adc code of analog controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-27
 * @VERSION 1.0
 */
#include "l2.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_sound.h"
#include "anyka_cpu.h"
#include "drv_module.h"
#include "interrupt.h"

#define BUF_MAX_NUM     100
#define RECVPCM_MESSAGE 1
#define AEC_ONEBUF_SIZE   (1024 * 2)
#define AEC_BUF_NUM   (32)


typedef struct
{
    signed short *pBufAddr;            ///< the address of buffer
    unsigned long dataLen;              ///< buffer length   
    unsigned long   ts;			       ///timestamp(ms)
}T_RECVPCM_BUF;

typedef struct
{
    T_RECVPCM_BUF  Buf[BUF_MAX_NUM];///< data buffer
    unsigned char    rpIndex;            ///< write index
    unsigned char    wpIndex;            ///< read index     
    unsigned long   BufNum;             ///< buffer number
    unsigned long   OneBufSize;         ///< buffer size
    unsigned char    L2Buf_id;           ///< l2 buffer id
    bool  bL2Start;           ///< flag of L2 start
    bool  bBufAllFull;        ///< flag of all buffer are full
    T_fSOUND RecvFinish;        ///< receive finish ,and call back
    signed short   *pNullBuf;          ///< for recv null data while not data to recv
}T_RECVPCM_BUF_MSG;

typedef struct
{	
	unsigned char   buf[AEC_ONEBUF_SIZE];//AEC_ONEBUF_SIZE
	volatile unsigned long   ts;       ///timestamp(ms)
	volatile unsigned long   len;      ///data_len
	volatile bool  ts_flag;            ///打上时间戳的标志
	volatile bool  new_pcm_flag;       ///新数据进来的标志
}T_AECPCM_BUF;

typedef struct
{
	T_AECPCM_BUF  aecbuf[AEC_BUF_NUM];//AEC_BUF_NUM
	volatile unsigned char aec_rpIndex;   ///< write index
    volatile unsigned char aec_wpIndex;   ///< read index  
}T_AEC_BUF_MSG;


static bool m_bTaskInit = false;
static T_RECVPCM_BUF_MSG m_recvpcm_msg;
static volatile bool sendpcm_state = false;//dac开始发送数据的标志
static volatile bool recvpcm_aec_enable_flag = false;//上层启动aec的标志
static volatile bool adc_call_flag = false;//防止中断回调里多次调用sendpcm_start这个接口
static T_AEC_BUF_MSG m_aecpcm_msg;



static void recvpcm_l2_callback(void);
static bool recvpcm_l2_init( void );

bool recvpcm_free ( void );
bool recvpcm_setcallback (T_fSOUND setcallback);


extern void sendpcm_start(void);
extern bool sendpcm_dac_state(void);

bool recvpcm_get_aecbuf(unsigned char *aecdata,unsigned long *ts, unsigned long *len)
{
	int i;	
	unsigned long rindex;
	rindex = m_aecpcm_msg.aec_rpIndex;
	/*
	获取已经进去aec_buf而且打上时间戳的对于buf的数据，
	找到则返回，没找到则返回失败。

	*/
	for(i = 0; i < AEC_BUF_NUM; i++)
	{
		if(m_aecpcm_msg.aecbuf[rindex].ts_flag)
		{
			memcpy(aecdata,m_aecpcm_msg.aecbuf[rindex].buf,m_aecpcm_msg.aecbuf[rindex].len);
			*ts	= m_aecpcm_msg.aecbuf[rindex].ts;
			//akprintf(C3, M_DRVSYS, "1.a: %d\n", *ts);
			*len = m_aecpcm_msg.aecbuf[rindex].len;
			//akprintf(C3, M_DRVSYS, "1.a: %d\n", *len);
			m_aecpcm_msg.aecbuf[rindex].ts_flag = false;//数据已经被取了的buf把对应的标志清
			m_aecpcm_msg.aecbuf[rindex].new_pcm_flag = false;
			m_aecpcm_msg.aec_rpIndex = (m_aecpcm_msg.aec_rpIndex + 1) % AEC_BUF_NUM;
			return true;
		}
		else
		{
			rindex = (rindex + 1) % AEC_BUF_NUM;		
		}
	}
	return false;	
}

bool pcm_data_transform(signed short *data,unsigned long dataLen)
{
	unsigned long i = 0,j = 0;
	
	signed short *temp = (short*)m_aecpcm_msg.aecbuf[m_aecpcm_msg.aec_wpIndex].buf;
	while(j != dataLen)
	{
		temp[i] = data[j];
		i += 1;
		j += 2;
	}
	m_aecpcm_msg.aecbuf[m_aecpcm_msg.aec_wpIndex].len = i;
	m_aecpcm_msg.aecbuf[m_aecpcm_msg.aec_wpIndex].new_pcm_flag = true;//新数据进buf，置标志位
	/*
		这里把标志位置反，原因是有可能当之前的数据一直还没有被取，
		但是新的数据又来了，旧的数据会被覆盖，为了重新打上时间戳
		，所以需要把ts_flag 置反。
	*/
	m_aecpcm_msg.aecbuf[m_aecpcm_msg.aec_wpIndex].ts_flag = false;
	
	//akprintf(C3, M_DRVSYS, "2.b: %d,w :%d\n", m_aecpcm_msg.aecbuf[m_aecpcm_msg.aec_wpIndex].ts,m_aecpcm_msg.aec_wpIndex);
	m_aecpcm_msg.aec_wpIndex = (m_aecpcm_msg.aec_wpIndex + 1) % AEC_BUF_NUM;

	return true;
}

static int get_aecbuf_index(void)
{
	unsigned long get_index ,index;
	int i;
	index = m_aecpcm_msg.aec_wpIndex;
	/*
	这里是从当前W下标的前一个开始找，看哪一个aec的buf新近了数据，
	但是还没有打上新的时间戳的
	*/
	get_index = (index + (AEC_BUF_NUM - 1)) % AEC_BUF_NUM;//从当前W下标的前一个开始找
	for(i = 0;i < AEC_BUF_NUM; i++)
	{
		if((m_aecpcm_msg.aecbuf[get_index].new_pcm_flag) && (!m_aecpcm_msg.aecbuf[get_index].ts_flag))
		{
			return get_index;
		}
		else
		{
			get_index = (get_index + 1) % AEC_BUF_NUM;
		}

	}
	return -1;
}
/**
 * @brief  L2 callback,it will be call while one buffer recv finished
 * @author LianGenhui
 * @date   2010-07-29
*/
static void recvpcm_l2_callback(void)
{
    unsigned long write_index;
	int aec_index;

    if(BUF_NULL == m_recvpcm_msg.L2Buf_id)
        return;

    DrvModule_Send_Message(DRV_MODULE_AD, RECVPCM_MESSAGE, NULL);

    if(((m_recvpcm_msg.wpIndex + 1) % m_recvpcm_msg.BufNum) ==  m_recvpcm_msg.rpIndex )
    {
        l2_combuf_dma((unsigned long)m_recvpcm_msg.pNullBuf, m_recvpcm_msg.L2Buf_id, m_recvpcm_msg.OneBufSize, BUF2MEM, true);

        //set all buffer full flag
        m_recvpcm_msg.bBufAllFull = true;

    }
    else
    { 	//判断是否需要执行aec的相关操作
    	if((sendpcm_state) && (recvpcm_aec_enable_flag) && (adc_call_flag))
		{	
			/*
			若adc先于dac启动时，在调用接口启动dac的同时，
			需要先把adc采集到的数据也清空。
			*/
			m_recvpcm_msg.rpIndex = m_recvpcm_msg.wpIndex;
			sendpcm_start();//调用sendpcm_start接口，启动dac开始发送数据
			adc_call_flag = false;//置反，为了防止多次进入
		}
 
        m_recvpcm_msg.wpIndex = (m_recvpcm_msg.wpIndex + 1) % m_recvpcm_msg.BufNum;

        write_index = m_recvpcm_msg.wpIndex;
		m_recvpcm_msg.Buf[write_index].ts = get_tick_count();//打时间戳
		
		if((sendpcm_state) && (recvpcm_aec_enable_flag))//只有当aec open 和dac start时才调用
		{
			/*		
			调用接口查找哪一个buf进了新的数据，
			但是没有打上时间戳，就把对应的时间戳
			给该buf打上，同时把该buf对应的ts_flag 置真。
			*/
			aec_index = get_aecbuf_index();
			if(aec_index >= 0)
			{
				m_aecpcm_msg.aecbuf[aec_index].ts = m_recvpcm_msg.Buf[write_index].ts;			
				m_aecpcm_msg.aecbuf[aec_index].ts_flag = true;//打上时间戳的标志
				//akprintf(C3, M_DRVSYS, "2.a: %d, %d\n", aec_index, m_aecpcm_msg.aecbuf[aec_index].ts);
			}
		}	
        l2_combuf_dma((unsigned long)m_recvpcm_msg.Buf[write_index].pBufAddr, m_recvpcm_msg.L2Buf_id, m_recvpcm_msg.OneBufSize, BUF2MEM, true);
   }
}

static void recvpcm_handler(unsigned long *param, unsigned long len)
{
    //callback,receive one buffer data finished 
    if(m_recvpcm_msg.RecvFinish != NULL)
        m_recvpcm_msg.RecvFinish ();
}

/**
 * @brief  initial l2,alloc L2 buffer,set l2 callback
 * @author LianGenhui
 * @date   2010-07-29
 */
static bool recvpcm_l2_init( void )
{
	signed long status;

    //alloc l2 buffer
    m_recvpcm_msg.L2Buf_id = l2_alloc(ADDR_ADC2);

    if(BUF_NULL == m_recvpcm_msg.L2Buf_id)
    {
        akprintf(C2, M_DRVSYS, "alloc L2 buffer failed!, buf=%d\n");
        return false;
    }    
    
    //set L2 callback function
    l2_set_dma_callback (m_recvpcm_msg.L2Buf_id, recvpcm_l2_callback);

    return true;
}

static void parameter_init(void)
{
	unsigned long i;
    m_recvpcm_msg.L2Buf_id = BUF_NULL;
    m_recvpcm_msg.RecvFinish = NULL;
    m_recvpcm_msg.bL2Start = false;
    m_recvpcm_msg.pNullBuf = NULL; 
    m_recvpcm_msg.bBufAllFull = false;
	
	m_aecpcm_msg.aec_rpIndex = 0;
	m_aecpcm_msg.aec_wpIndex = 0;
	for(i = 0;i <AEC_BUF_NUM;i++)
	{		
		memset(m_aecpcm_msg.aecbuf[i].buf, 0, AEC_ONEBUF_SIZE);
		m_aecpcm_msg.aecbuf[i].ts = 0;
		m_aecpcm_msg.aecbuf[i].len = 0;		
		m_aecpcm_msg.aecbuf[i].ts_flag = false;
		m_aecpcm_msg.aecbuf[i].new_pcm_flag = false;
	}
}

/**
 * @brief  malloc recvpcm buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @param[in] OneBufSize one buffer size
 * @param[in] BufNum buffer number
 * @return bool
 */
bool recvpcm_malloc (unsigned long OneBufSize, unsigned long BufNum)
{
    unsigned long i;
    unsigned long j;

    if(((OneBufSize % 512) != 0) || (BufNum < 4))
    {
        akprintf(C2, M_DRVSYS, "OneBufSize %% 512 != 0 or BufNum < 4\r\n");
        return false;
    }

    if(m_bTaskInit)
    {
        akprintf(C2, M_DRVSYS, "can't malloc again,please free!\n");
        return false;
    }

    DrvModule_Protect(DRV_MODULE_AD); 
    
    parameter_init();
    
    //create receive pcm task
    if(!DrvModule_Create_Task(DRV_MODULE_AD))
    {
       DrvModule_UnProtect(DRV_MODULE_AD); 
       return false;
    }

    //set massage and handler
    DrvModule_Map_Message(DRV_MODULE_AD, RECVPCM_MESSAGE, recvpcm_handler);

    //malloc all buffer and set to 0
    for (i=0; i<BufNum; i++)
    {
        m_recvpcm_msg.Buf[i].pBufAddr = (signed short *)drv_malloc(OneBufSize);

        if (NULL ==  m_recvpcm_msg.Buf[i].pBufAddr)//malloc fail
        {
            for (j=0; j<i; j++)//free the mallocated memories
            {
                drv_free(m_recvpcm_msg.Buf[j].pBufAddr);
                m_recvpcm_msg.Buf[j].pBufAddr = NULL;
            }

            akprintf(C2, M_DRVSYS, "malloc fail!\n");
            DrvModule_UnProtect(DRV_MODULE_AD); 
            return false;
        }
        memset(m_recvpcm_msg.Buf[i].pBufAddr, 0, OneBufSize);
		m_recvpcm_msg.Buf[i].ts = 0;
    } 

    m_recvpcm_msg.pNullBuf = (signed short *)drv_malloc(OneBufSize);
    if(NULL == m_recvpcm_msg.pNullBuf)
    {
        akprintf(C1, M_DRVSYS, "malloc fail!\n");
        
        m_recvpcm_msg.pNullBuf = NULL;
        for(i=0; i<BufNum; i++)
        {
            drv_free(m_recvpcm_msg.Buf[i].pBufAddr);
            m_recvpcm_msg.Buf[i].pBufAddr = NULL;
        }
        
        DrvModule_UnProtect(DRV_MODULE_AD); 
        return false;
    }
    memset(m_recvpcm_msg.pNullBuf, 0, OneBufSize);//set to 0 for null buffer

    m_recvpcm_msg.rpIndex = 0; //set define for read point index
    m_recvpcm_msg.wpIndex = 0;
    m_recvpcm_msg.BufNum = BufNum;
    m_recvpcm_msg.OneBufSize = OneBufSize;
    m_recvpcm_msg.bL2Start = false;

    //PCM to L2 initial ,include adc and IIS receive
    recvpcm_l2_init();
    
    m_bTaskInit = true;

    DrvModule_UnProtect(DRV_MODULE_AD); 
    
    return true;
}

/**
 * @brief  realloc recvpcm buffer
 * @author liao_zhijun
 * @date   2010-11-02
 * @param[in] OneBufSize one buffer size
 * @param[in] BufNum buffer number
 * @return bool
 */
bool recvpcm_realloc (unsigned long OneBufSize, unsigned long BufNum, T_fSOUND callback)
{
    bool ret;
    
    recvpcm_free();
    ret = recvpcm_malloc(OneBufSize, BufNum);

    recvpcm_setcallback (callback);

    return ret;    
}

/**
 * @brief  free recvpcm buffer
 * @author LianGenhui
 * @date   2010-07-29
 * @return bool
 */
bool recvpcm_free ( void )
{
	unsigned long i,j;
    DrvModule_Protect(DRV_MODULE_AD); 

    m_recvpcm_msg.L2Buf_id = BUF_NULL;
    l2_free(ADDR_ADC2);

    DrvModule_Terminate_Task(DRV_MODULE_AD);//release ad task

    drv_free(m_recvpcm_msg.pNullBuf);
    m_recvpcm_msg.pNullBuf = NULL;

    for(i=0; i<m_recvpcm_msg.BufNum; i++)
    {
        drv_free(m_recvpcm_msg.Buf[i].pBufAddr);
        m_recvpcm_msg.Buf[i].pBufAddr = NULL;
    }
    m_recvpcm_msg.BufNum = 0;
    m_recvpcm_msg.OneBufSize = 0;

    //free null buf
    m_bTaskInit = false;
    m_recvpcm_msg.bL2Start = false;
	recvpcm_aec_enable_flag = false;
	sendpcm_state = false;
	adc_call_flag = false;
	
	m_aecpcm_msg.aec_rpIndex = 0;
	m_aecpcm_msg.aec_wpIndex = 0;
	for(j = 0;j <AEC_BUF_NUM;j++)
	{		
		memset(m_aecpcm_msg.aecbuf[j].buf, 0, AEC_ONEBUF_SIZE);
		m_aecpcm_msg.aecbuf[j].ts = 0;
		m_aecpcm_msg.aecbuf[j].len = 0;
		m_aecpcm_msg.aecbuf[j].ts_flag = false;	
		m_aecpcm_msg.aecbuf[j].new_pcm_flag = false;
	}
    DrvModule_UnProtect(DRV_MODULE_AD); 

    return true;
}


/**
* @brief  judge  aecopen
* @author 
* @date   2017-07-27
* @param[in] aec_open, use to judge whether aec open or not,if open,recvpcm_aec_enable_flag = true,
*                   then it will execute aec operation.
* @return bool
*/
bool recvpcm_judge_aecopen(bool aec_open)
{
	recvpcm_aec_enable_flag = aec_open;
	return true;
}

/**
 * @brief  set callback function
 * @author LianGenhui
 * @date   2010-07-29
 * @param[in] setcallback callback function,it will be call when one buffer receive finished
 * @return bool
 */
bool recvpcm_setcallback (T_fSOUND setcallback)
{
    if(NULL == setcallback)
    return false;

    m_recvpcm_msg.RecvFinish = setcallback;
    return true;
}

/**
 * @brief  clean adc buffer
 * @author LianGenhui
 * @date   2010-07-27
 * @return void
 */
void recvpcm_cleanbuf ( void )
{
    unsigned long i;
    DrvModule_Protect(DRV_MODULE_AD); 

    for(i=0; i<m_recvpcm_msg.BufNum; i++)
    {
       memset(m_recvpcm_msg.Buf[i].pBufAddr, 0, m_recvpcm_msg.OneBufSize);
    }

    m_recvpcm_msg.rpIndex = 0; //set define for read point index
    m_recvpcm_msg.wpIndex = 0;
    
    DrvModule_UnProtect(DRV_MODULE_AD); 
}

/**
* @brief  get recvpcm adc state
* @author 
* @date   2017-07-27
* @param[] this funs as extern funs ,main use to be call by dac to judge whether get data start or not,     
*                only in order to adc pcmdata and dac pcmdata Synchronization for aec.
* @return bool
*/
bool recvpcm_adc_state(void)
{
	if(m_recvpcm_msg.bL2Start)
		return true;
	else
		return false;
}

/**
* @brief  recvpcm_start
* @author 
* @date   2017-07-27
* @param[] this funs is first to get pcmdata would be call  or aec enable and dac start earlier than
*                adc start ,then it will be call in dac  interrupt callback
* @return 
*/
void recvpcm_start(void)
{
	//akprintf(C3, M_DRVSYS, "#a1: %d\n", get_tick_count());
	//这里是打第一个buf的时间戳
	m_recvpcm_msg.Buf[m_recvpcm_msg.wpIndex].ts = get_tick_count();//打时间戳
    l2_combuf_dma((unsigned long)m_recvpcm_msg.Buf[m_recvpcm_msg.rpIndex].pBufAddr, m_recvpcm_msg.L2Buf_id, m_recvpcm_msg.OneBufSize, BUF2MEM, true);
    //m_recvpcm_msg.bL2Start = true;	
}

/**
 * @brief  get buffer address and buffer len, which can be used to fill or retrieve adc data
 * @author LianGenhui
 * @date   2010-07-27
 * @param[out] pbuf return buffer address or NULL
 * @param[out] len return buffer len or 0
 * @return bool
 * @retval true  get buffer successful
 * @retval false get buffer failed
 * @note   if adbuf_create failed or no buffer to return, it will return failed
 */
bool recvpcm_getbuf ( void **pbuf, unsigned long *len)
{
    unsigned long bufHead;
    static struct {
		unsigned long len;
		unsigned long ts;
    	}data_buf;
    DrvModule_Protect(DRV_MODULE_AD); 
	/*
	这里是调用接口判断sendpcm  start是否已经
	准备好，由于不确定sendpcm  start具体什么
	时候准备好，所以需要不停的调用
	*/
	sendpcm_state = sendpcm_dac_state();
	//akprintf(C3, M_DRVSYS, "1.adc\n");

	/*
	adc_call_flag 这个标志是为了在中断回调里面调用接口
	启动sendpcm_start时不会多次调用
	*/
	if(!sendpcm_state)
	{
		adc_call_flag = !sendpcm_state;
	}
    //if not start,receive first buffer
    if(!m_recvpcm_msg.bL2Start)
    {  	 	
		m_recvpcm_msg.bL2Start = true;	
    	/*
    		判断aec是否open，或者aec已经open，但是sendpcm
    		start还没启动的时候，可以正常的往下执
    		*/
		if((false == recvpcm_aec_enable_flag) || (false == sendpcm_state))
		{
			recvpcm_start();			
		}
        *pbuf = NULL;
        *len = 0;
        DrvModule_UnProtect(DRV_MODULE_AD); 
        return false;
    }

    //not buffer to read
    if(m_recvpcm_msg.rpIndex == m_recvpcm_msg.wpIndex )
    {
        *pbuf = NULL;
        *len = 0;
        DrvModule_UnProtect(DRV_MODULE_AD); 
        return false;
    }
	
    //get read buffer index and buffer size
    bufHead = m_recvpcm_msg.rpIndex;
    *pbuf = m_recvpcm_msg.Buf[bufHead].pBufAddr;
   
	data_buf.len  = m_recvpcm_msg.OneBufSize;
	data_buf.ts   = m_recvpcm_msg.Buf[bufHead].ts;
	//akprintf(C3, M_DRVSYS, "2.a: %d\n", data_buf.ts);
	*len =(unsigned long) &data_buf;

    DrvModule_UnProtect(DRV_MODULE_AD); 
    return true;
}

/**
 * @brief set one buffer end\n
 * after call adbuf_getbuf and finish the operation of adc data,call this function
 * @author  LianGenhui
 * @date    2010-07-27 
 * @param[in] len     buffer len(use for write)
 * @return bool
 * @retval true  successful
 * @retval false longer than one buffer's len
 */
bool recvpcm_endbuf ( unsigned long len )
{
    DrvModule_Protect(DRV_MODULE_AD); 
    
    //point to next buffer
    m_recvpcm_msg.rpIndex = (m_recvpcm_msg.rpIndex + 1) % m_recvpcm_msg.BufNum;

    //receive AD again, it is closed when all buffer fill data  
    if(true == m_recvpcm_msg.bBufAllFull)
    {
        m_recvpcm_msg.bBufAllFull = false;
    }

    DrvModule_UnProtect(DRV_MODULE_AD); 
    return true;
}

/**
 * @brief get the number of buffers which have been filled or retrieved adc data 
 * @author LianGenhui
 * @date   2010-07-27
 * @return unsigned long
 * @retval value the value will from 0 to the number when create a adc set 
  */
unsigned long recvpcm_getnum_fullbuf( void )
{
    unsigned long num_fullbuf;
    DrvModule_Protect(DRV_MODULE_AD); 
    
    if( m_recvpcm_msg.wpIndex < m_recvpcm_msg.rpIndex )
    {
        num_fullbuf =  m_recvpcm_msg.wpIndex + m_recvpcm_msg.BufNum -  m_recvpcm_msg.rpIndex;
    }
    else
    {
         num_fullbuf =  m_recvpcm_msg.wpIndex -  m_recvpcm_msg.rpIndex;
    }
    
    DrvModule_UnProtect(DRV_MODULE_AD); 
    
    return num_fullbuf;
}


