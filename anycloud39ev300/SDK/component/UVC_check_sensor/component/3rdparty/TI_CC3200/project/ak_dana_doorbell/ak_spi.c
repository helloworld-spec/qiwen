
/**
 * @file  
 * @brief 
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2016-12-3
 * @version 1.0
 */
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "uart.h"
#include "interrupt.h"

#include "hw_mcspi.h"

#include "simplelink.h"
#include "wlan.h"

#include "cc_types.h"
#include "gpio_hal.h"
#include "pinmux.h"
 
#include "ak_pub_def.h"
#include "ak_spi.h"

//#include "gpio.h"
//#include "pin.h"
//#include "gpio_if.h"

#include "ak_dana_common.h"


#define SPI_BIT_10M       20000000
#define SPI_IF_BIT_RATE  SPI_BIT_10M


//#define SPI_DATA_DEBUG

static char video_buf[VIDEO_BUF_SIZE];
extern OsiSyncObj_t  spi_read_sem;
extern volatile unsigned char spi_trans;
//OsiMsgQ_t spi2videomsg;
extern OsiSyncObj_t g_NetSendSyncObj;

static unsigned char send_error_flag = 0;

//char *video_buf= NULL; 
//char *video_buf= data_buf; 
volatile unsigned long  gpio_spi_int = 0;


#ifdef SPI_DATA_DEBUG
static volatile unsigned int gpio_int_test = 0;
#endif

typedef struct _spi_trans_head{
    char frame_head[4];
	unsigned int reserve;
	unsigned int pack_id;
	unsigned int data_len;
} spi_trans_head_t;

#define SPI_HEAD_SIZE	(sizeof(struct _spi_trans_head))


void start_spi_task(void);
#if SPI_WRITE_DATA

#define SPI_WRITE_FLG     (1<<1) 

#define REPORT_BUFFER_SIZE 128
#define MASTER_MSG       "==test==This is CC3200 SPI Master Application\n\r"

static OsiSyncObj_t  spi_write_sem = NULL;

static unsigned char g_ucRxBuff[SPI_BUFF_SIZE];
static unsigned char g_ucTxBuff[SPI_BUFF_SIZE];
static unsigned char *gp_ucTxBuff = &g_ucTxBuff[0];
static T_fspireadcb spi_master_read_cb;

/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
void ak_spi_master_write(unsigned char* buf)
{
	#if SPI_DATA_TEST
	memcpy(g_ucTxBuff,MASTER_MSG,sizeof(MASTER_MSG));
	gp_ucTxBuff = g_ucTxBuff;

	#else
	gp_ucTxBuff = buf;
	#endif
	
	spi_trans |= SPI_WRITE_FLG;
	osi_SyncObjSignalFromISR(&spi_read_sem); // 
	#if 1
	osi_SyncObjWait(&spi_write_sem,OSI_WAIT_FOREVER); // ×èÈû
	#else
	
	while(spi_trans & SPI_WRITE_FLG)
		;
	#endif
}



/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
void ak_spi_master_read_set_cb(T_fspireadcb setcallcb)
{

	spi_master_read_cb = setcallcb;
}
#endif

/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
#define SPI_DATA_SIZE       (SPI_BUFF_SIZE-4)
static void ak_spi_master_read(void * pvParameters )
{	
	long retval;
    unsigned char spi_recv[SPI_BUFF_SIZE];
    unsigned int check_sum = 0;
    unsigned int check_sum_get = 0;
    
    
	static volatile unsigned int fram_len =0;
	static volatile unsigned int start_flag = 1;
	unsigned char spi_msg = 0;
	struct _spi_trans_head    spi_head;
	#ifdef SPI_DATA_DEBUG
	static volatile unsigned long spi_data_test = 0;
	static volatile unsigned long spi_data_flg  = 0;
	#endif
    int i;
    int ret;
    while(1)
    {
		retval = osi_SyncObjWait(&g_NetSendSyncObj, OSI_WAIT_FOREVER);/*wmj+ wait net ok*/
		if(retval <0)
		{
			 Report("wait g_NetSendSyncObj error\r\n");
			 return;
		}
		send_error_flag = 0;
		while(1)
		{
			retval = osi_SyncObjWait(&spi_read_sem,OSI_WAIT_FOREVER); // ×èÈû
			if(retval <0 )
			{
				Report("wait spi_read_sem error\r\n");
				return ;
			}
			//dana_mutex_lock(danavideotest_lock);
			//osi_MsgQRead(&video2spimsg, &spi_msg, OSI_WAIT_FOREVER);
			#ifdef SPI_DATA_DEBUG
			spi_data_test++;
			//Report("#=%d,%d,%d\r\n",spi_data_test,spi_trans,gpio_int_test);
			#endif
			switch (spi_trans)
			{
				case SPI_GPIO_INT_FLG:	
					spi_trans = 0; 
					///Report("#");

#if 0  // if 1, with spi pack check 
	                MAP_SPITransfer(GSPI_BASE,0,(unsigned char *)(&spi_recv[0]),SPI_BUFF_SIZE,
	                                SPI_CS_ENABLE|SPI_CS_DISABLE);
	                check_sum = 0;
	                for(i = 0; i < SPI_DATA_SIZE; i++) {
	                    check_sum += spi_recv[i];
	                }
	                check_sum_get =  *(unsigned int *)&spi_recv[SPI_DATA_SIZE];
	                if(check_sum != check_sum_get){
	                     Report("spi check error cal=%d get=%d\r\n",check_sum,check_sum_get);
	                     for(i = SPI_DATA_SIZE; i < SPI_BUFF_SIZE; i++) {
	                        Report("%0x ", spi_recv[i]);
	                     }
	                     Report("\r\n");
	                } else {
	                    memcpy((unsigned char *)(video_buf+fram_len),spi_recv,SPI_DATA_SIZE);
	                }


	                if (1 == start_flag)   // spi head(first package)
					{
					    memset(&spi_head,0,SPI_HEAD_SIZE);
	                    for(i = 0; i < SPI_DATA_SIZE-4; i++) {
	    					if((video_buf[i] != 'S' )|| (video_buf[i+1] != 'P')||
	                           (video_buf[i+2] != 'I')|| (video_buf[i+3] != 'S'))
	    					{
	    						continue;
	    					} else {
	    					    memcpy(&spi_head, video_buf, SPI_HEAD_SIZE);
	                            fram_len = 0;
	                            start_flag = 0; 
	                            break;
	                        } 
	                    }

	 
	    #if 1
						//Report("recv stream header:  type=%d,ts=%d,size: %d ",
						//				stream_head.iFrame,
						//				stream_head.timestame,
						//				stream_head.data_len);
						if(spi_head.reserve!= 2) {
	                        Report("recv id=%d len=%d \r\n",
	                            spi_head.pack_id,spi_head.data_len);

	                        for(i = 0; i < 16; i++)
	                            Report("%0x ", video_buf[i]);
	                        Report("\r\n");
	                    }
	                    if(spi_head.data_len > 100*1024+24) {
	                        Report("======fram len lager 100k====\r\n");  
	                    }
	    #endif
					}
					else
					{
						if(spi_head.data_len  <= fram_len + SPI_DATA_SIZE)//one frame < SPI_BUFF_SIZE
						{
							#ifdef SPI_DATA_DEBUG
							//Report("data len=%d,\n ",stream_head.data_len,stream_head.data_len);
							//Report("frame len=%d, spi_data_flag = %d\n ",fram_len,fram_len,spi_data_flg);
							#endif
							//Report("1e=%d\r\n ",stream_head.data_len);
							fram_len = 0;
							start_flag = 1;
							SendVideoStream(video_buf,spi_head.data_len);
						}
	                    else
	                    {
	                        fram_len += SPI_DATA_SIZE;
	                        //Report("fram_len=%d \r\n ",fram_len);
	                        if(fram_len + SPI_DATA_SIZE > VIDEO_BUF_SIZE)
	                        {
	                            SendVideoStream(video_buf,fram_len);
	                            spi_head.data_len -= fram_len;
	                            fram_len = 0;
	                            //Report("data_len=%d \r\n ",spi_head.data_len);
	                            //start_flag = 1;
	                        }
	                    }
	                }
	                
#else
					MAP_SPITransfer(GSPI_BASE,0,(unsigned char *)(video_buf+fram_len),SPI_BUFF_SIZE,
		            SPI_CS_ENABLE|SPI_CS_DISABLE);
	      
#if 1

	    #ifdef SPI_DATA_DEBUG
					spi_data_flg++;	
	    #endif
					if (1 == start_flag)   // spi head(first package)
					{
					    memset(&spi_head,0,SPI_HEAD_SIZE);
	                    for(i = 0; i < SPI_BUFF_SIZE-4; i++) {
	    					if((video_buf[i] != 'S' )|| (video_buf[i+1] != 'P')||
	                           (video_buf[i+2] != 'I')|| (video_buf[i+3] != 'S'))
	    					{
	    						continue;
	    					} else {
	    					    memcpy(&spi_head, video_buf, SPI_HEAD_SIZE);
	                            fram_len = 0;
	                            start_flag = 0; 
	                            break;
	                        } 
	                    }

	 
	    #if 1
						//Report("recv stream header:  type=%d,ts=%d,size: %d ",
						//				stream_head.iFrame,
						//				stream_head.timestame,
						//				stream_head.data_len);
						//if(spi_head.reserve!= 2) {
	                        Report("t=%d i=%d l=%d \r\n", spi_head.reserve, spi_head.pack_id, spi_head.data_len);
	                            

	                        for(i = 0; i < 16; i++)
	                            Report("%0x ", video_buf[i]);
	                        Report("\r\n");
	                 //   }
	                    if(spi_head.data_len > 100*1024+24) {
	                        Report("======fram len lager 100k====\r\n");  
	                    }
	    #endif
					}
					else
					{
	    #if 1 // second logic to recv data
						if(spi_head.data_len  <= fram_len + SPI_BUFF_SIZE)//one frame < SPI_BUFF_SIZE
						{
							#ifdef SPI_DATA_DEBUG
							//Report("data len=%d,\n ",stream_head.data_len,stream_head.data_len);
							//Report("frame len=%d, spi_data_flag = %d\n ",fram_len,fram_len,spi_data_flg);
							#endif
							//Report("1e=%d\r\n ",stream_head.data_len);
							fram_len = 0;
							start_flag = 1;
							retval = SendVideoStream(video_buf,spi_head.data_len);
							if(retval < 0)
							{
								send_error_flag = 1;
							}
						}
	                    else
	                    {
	                        fram_len += SPI_BUFF_SIZE;
	                        //Report("fram_len=%d \r\n ",fram_len);
	                        if(fram_len + SPI_BUFF_SIZE > VIDEO_BUF_SIZE)
	                        {
	                            SendVideoStream(video_buf,fram_len);
								if(retval < 0)
								{
									send_error_flag = 1;
								}
	                            spi_head.data_len -= fram_len;
	                            fram_len = 0;
	                            //Report("data_len=%d \r\n ",spi_head.data_len);
	                            //start_flag = 1;
	                        }
	                    }
	    #else
	                    if(spi_head.data_len  <= SPI_BUFF_SIZE)//one frame < SPI_BUFF_SIZE
	                     {
	                        #ifdef SPI_DATA_DEBUG
	                         //Report("data len=%d,\n ",stream_head.data_len,stream_head.data_len);
	                         //Report("frame len=%d, spi_data_flag = %d\n ",fram_len,fram_len,spi_data_flg);
	                        #endif
	                         //Report("1e=%d\r\n ",stream_head.data_len);
	                         fram_len = 0;
	                         start_flag = 1;
	                         //osi_MsgQWrite(&spi2videomsg, &spi_msg, OSI_NO_WAIT);  
	                         SendVideoStream(video_buf,spi_head.data_len);
	                     }

	                    else {
	                        if(spi_head.data_len  >= VIDEO_BUF_SIZE)
	                       {
	                           if(fram_len + SPI_BUFF_SIZE >= VIDEO_BUF_SIZE) {
	                               SendVideoStream(video_buf,fram_len);
	                               fram_len = 0;
	                               spi_head.data_len -= fram_len;
	                               if(spi_head.data_len == 0) {
	                                   start_flag = 1;
	                               }
	                           } else {
	                               
	                               fram_len += SPI_BUFF_SIZE;
	                           }
	                       }
	                       else                    
	                       {
	                           if(fram_len + SPI_BUFF_SIZE >= spi_head.data_len)    
	                           {
	                                #ifdef SPI_DATA_DEBUG
	                               //Report("big data len=%d,\n ",stream_head.data_len,stream_head.data_len);
	                               //Report("big frame len=%d, spi_data_flag = %d\n ",fram_len,fram_len,spi_data_flg);
	                                #endif
	                               fram_len = 0;
	                               start_flag = 1;
	                               //osi_MsgQWrite(&spi2videomsg, &spi_msg, OSI_NO_WAIT);  
	                               SendVideoStream(video_buf,spi_head.data_len);
	                           }
	                           else
	                           {
	                               fram_len += SPI_BUFF_SIZE;
	                           }
	                       }

	                    }
						
	    #endif
	                    
						
					}
	  #endif
#endif

					break;
					
				#if SPI_WRITE_DATA
				case SPI_WRITE_FLG:
					MAP_SPITransfer(GSPI_BASE,gp_ucTxBuff,g_ucRxBuff,SPI_BUFF_SIZE,
			            			SPI_CS_ENABLE|SPI_CS_DISABLE);	
					
					if((spi_trans & SPI_WRITE_FLG)==SPI_WRITE_FLG)
					{
						osi_SyncObjSignalFromISR(&spi_write_sem); // 

					}
						
					break;
				case SPI_WRITE_FLG|SPI_GPIO_INT_FLG:
					MAP_SPITransfer(GSPI_BASE,gp_ucTxBuff,g_ucRxBuff,SPI_BUFF_SIZE,
			            			SPI_CS_ENABLE|SPI_CS_DISABLE);	
					
					if((spi_trans & SPI_WRITE_FLG)==SPI_WRITE_FLG)
					{
						osi_SyncObjSignalFromISR(&spi_write_sem); // 

					}				
				#endif	
				default:				
					break;					
			}
			//dana_mutex_unlock(danavideotest_lock);

			if(send_error_flag)
			{
				break;
			}
			#if SPI_WRITE_DATA
			spi_master_read_cb(g_ucRxBuff,SPI_BUFF_SIZE); //cb ,move data
			#endif	
		}
    }

}


static void spi_gpio_init(void)
{

	
	unsigned int g_uiKeyPort;
	unsigned char g_ucKeyPin;	
	
	Report("spi_gpio_init\r\n ");
	
	 cc_hndl tGPIOHndl;
    tGPIOHndl = cc_gpio_open(GPIO_SPI_SIG, GPIO_DIR_INPUT);
    cc_gpio_enable_notification(tGPIOHndl, GPIO_SPI_SIG, INT_RISING_EDGE, GPIO_TYPE_NORMAL);
#if 0	
	MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK); //gpio02
	PinConfigSet(PIN_NB, PIN_STRENGTH_2MA, PIN_TYPE_STD_PD);
	PinModeSet(PIN_NB, PIN_MODE_0);
	MAP_GPIODirModeSet(GPIO_BASE, GPIO_MAST, GPIO_DIR_MODE_IN);
	GPIO_IF_GetPortNPin(GPIO_NB, &g_uiKeyPort, &g_ucKeyPin);
	GPIO_IF_ConfigureNIntEnable(g_uiKeyPort, g_ucKeyPin, GPIO_HIGH_LEVEL, spi_gpio_Interrupt_handle);//GPIO_BOTH_EDGES  GPIO_RISING_EDGE
#endif
}


/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
void ak_spi_init(void)
{	
  

	Report("\r\nak_spi_init\r\n ");
	
#if 0
	ret = osi_MsgQCreate(&spi2videomsg, NULL, sizeof( unsigned char), 1);
    if (ret < 0)
    {
        Report("crate s2vmsg\n\r");      
    }
#endif//xrq-no use msg

#if 0 //wmj- use staic mem
	video_buf = (char *)mem_Malloc(VIDEO_BUF_SIZE);
	
	if(video_buf == NULL)
	{
	   Report("th_stream Malloc falid\r\n");
	   while(1)
	   {
			;
	   }
    }

	memset(video_buf,0,VIDEO_BUF_SIZE);
#endif
    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    //
    // Reset the peripheral
    //
    MAP_PRCMPeripheralReset(PRCM_GSPI);


    //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVELOW |
                     SPI_WL_8));//SPI_CS_ACTIVELOW    SPI_CS_ACTIVEHIGH  SPI_WL_8

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

	spi_gpio_init();

	//start_spi_task();
	
}
void start_spi_task(void)
{
	int ret;
	ret = osi_SyncObjCreate(&spi_read_sem);
	if(ret != OSI_OK)
	{
		Report("osi_SyncObjCreate  err!\r\n ");
	}
		
	#if SPI_WRITE_DATA
	ret = osi_SyncObjCreate(&spi_write_sem);
	if(ret != OSI_OK)
	{
		Report("\r\osi_SyncObjCreate  err!\r\n ");
	}
	#endif
	ret = osi_TaskCreate(ak_spi_master_read,(signed char *)"ak_spi_master_read",2048, (void *)NULL, 2, (void *)NULL);
	if(ret < 0)
	{
		UART_PRINT("Failed to creat heart task !\r\n");
		return ;
	}
}


