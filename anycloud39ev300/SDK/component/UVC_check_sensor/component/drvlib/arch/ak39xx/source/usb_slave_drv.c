/**
 * @filename usb_slave_drv.c
 * @brief: frameworks of usb driver.
 *
 * This file describe udriver of usb in slave mode.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-26
 * @version 1.0
 * @ref
 */
#ifdef OS_ANYKA

#include    "anyka_cpu.h"
#include    "usb_slave_drv.h"
#include    "sysctl.h"
#include    "interrupt.h"
#include    "drv_api.h"
#include    "usb_common.h"
#include    "hal_usb_s_state.h"
#include    "hal_usb_s_std.h"
#include    "hal_usb_std.h"
#include    "l2.h"
#include    "usb.h"

#define MAX_EP_NUM       4
#define DMA_PER_BUF_SIZE (128*1024)

typedef enum
{
    EP_RX_FINISH = 0,
    EP_RX_RECEIVING,
    EP_TX_FINISH,
    EP_TX_SENDING
}USB_EP_STATE;


typedef struct
{
    unsigned long EP_TX_Count;
    USB_EP_STATE EP_TX_State;
    T_fUSB_TX_FINISH_CALLBACK TX_Finish; 
    unsigned char  *EP_TX_Buffer;
    unsigned char  L2_Buf_ID;
}USB_EP_TX;

typedef struct
{
    unsigned long EP_RX_Count;
    USB_EP_STATE EP_RX_State;
    T_fUSB_NOTIFY_RX_CALLBACK RX_Notify;
    T_fUSB_RX_FINISH_CALLBACK RX_Finish;
    unsigned char  *EP_RX_Buffer;
    unsigned char  L2_Buf_ID;
    bool bDmaStart;
}USB_EP_RX;

typedef union
{
    USB_EP_RX rx;
    USB_EP_TX tx;
}USB_EP;

typedef struct
{
    unsigned long tx_count;
    T_CONTROL_TRANS ctrl_trans;
    T_fUSB_CONTROL_CALLBACK std_req_callback;
    T_fUSB_CONTROL_CALLBACK class_req_callback;
    T_fUSB_CONTROL_CALLBACK vendor_req_callback;
}
T_USB_SLAVE_CTRL;

typedef struct
{
    unsigned long ulInitMode;                           ///<expected mode when init
    unsigned long mode;
    unsigned long state;
    unsigned long usb_max_pack_size;
    T_fUSB_RESET_CALLBACK reset_callback;
    T_fUSB_SUSPEND_CALLBACK suspend_callback;
    T_fUSB_RESUME_CALLBACK resume_callback;
    T_fUSB_CONFIGOK_CALLBACK configok_callback;
    USB_EP ep[6];
    bool bInit;
    bool usb_need_zero_packet;
}USB_SLAVE;


static void usb_slave_dma_send_mode0(unsigned char EP_index, unsigned long addr, unsigned long count);

static void usb_slave_reset(void);
static void usb_slave_suspend();
static void usb_slave_reset_ep(unsigned long EP_index, unsigned short wMaxPacketSize, unsigned char ep_type, unsigned char dma_surport);

static unsigned long usb_slave_get_intr_type(unsigned char *usb_int, unsigned short *usb_ep_int_tx, unsigned short * usb_ep_int_rx);
static bool usb_slave_intr_handler(void);
static void usb_slave_ep0_rx_handler();
static void usb_slave_ep0_tx_handler();
static void usb_slave_common_intr_handler(unsigned char usb_int);

static void usb_slave_tx_handler(EP_INDEX EP_index);
static void usb_slave_rx_handler(EP_INDEX EP_index);

static void usb_slave_write_ep_reg(unsigned char EP_index, unsigned long reg, unsigned short value);
static void usb_slave_read_ep_reg(unsigned char EP_index, unsigned long reg, unsigned short *value);
static void usb_slave_read_int_reg(unsigned char *value0, unsigned short *value1, unsigned short *value2);

static unsigned long usb_slave_receive_data(EP_INDEX EP_index);
static unsigned long usb_slave_send_data(EP_INDEX EP_index);
static unsigned long usb_slave_dma_start(EP_INDEX EP_index);

static unsigned long usb_slave_ctrl_in(unsigned char *data, unsigned long len);
static unsigned long usb_slave_ctrl_out(unsigned char *data);
static bool usb_slave_ctrl_callback(unsigned char req_type);

volatile USB_SLAVE usb_slave;
volatile T_USB_SLAVE_CTRL usb_ctrl;

static volatile unsigned short m_ep_status[MAX_EP_NUM]={0};

/**
 * @brief   initialize usb slave global variables, and set buffer for control tranfer
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param buffer [IN] buffer to be set for control transfer
 * @param buf_len [IN] buffer length
 * @return  bool
 * @retval true init successfully
 * @retval false init fail
 */
bool usb_slave_init(unsigned char *buffer, unsigned long buf_len)
{
    //check param
    if(NULL == buffer || buf_len == 0)
    {
        return false;
    }

    //init global variables
    memset((void *)&usb_slave, 0, sizeof(usb_slave));
    usb_slave.bInit = true;

    usb_slave.usb_max_pack_size = 64;

    //alloc buffer
    usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID = l2_alloc(ADDR_USB_EP2);
	
	#ifdef BURNTOOL
    usb_slave.ep[EP3_INDEX].rx.L2_Buf_ID = l2_alloc(ADDR_USB_EP3);
    if(BUF_NULL == usb_slave.ep[EP3_INDEX].rx.L2_Buf_ID || BUF_NULL == usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID)
    {
        akprintf(C2, M_DRVSYS, "malloc L2 buffer id error\n");
        return false;
    }
	akprintf(C2, M_DRVSYS, "rx buf id %x\n", usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID);
	akprintf(C2, M_DRVSYS, "tx buf id %x\n", usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID);

	#else
    usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID = l2_alloc(ADDR_USB_EP1);
	

    if(BUF_NULL == usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID || BUF_NULL == usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID)
    {
        akprintf(C2, M_DRVSYS, "malloc L2 buffer id error\n");
        return false;
    }


    usb_slave.ep[EP3_INDEX].rx.L2_Buf_ID = usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID;
	#endif
    usb_ctrl.ctrl_trans.buffer = buffer;
    usb_ctrl.ctrl_trans.buf_len = buf_len;    

    return true;
    
}
void usb_slave_free(void)
{
    memset((void *)&usb_slave, 0, sizeof(usb_slave));
    //free l2 buffer
    #ifdef BURNTOOL
	l2_free(ADDR_USB_EP3);
	#else
    l2_free(ADDR_USB_EP1);
	#endif
	l2_free(ADDR_USB_EP2);
  
    usb_ctrl.ctrl_trans.buffer = NULL;
    usb_ctrl.ctrl_trans.buf_len= 0;
}

/**
 * @brief   enable usb interrupt, used in usb download
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  bool
 * @retval true init successfully
 * @retval false init fail
 */
bool usb_boot_init()
{
    //enble irq for usb
    int_register_irq(INT_VECTOR_USB, usb_slave_intr_handler);

    return true;
}


/**
 * @brief   set control transfer call back function
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param type [IN] request type, must be one of (REQUEST_STANDARD, REQUEST_CLASS, REQUEST_VENDOR)
 * @param callback [In] callback function
 * @return  bool
 * @retval true callback function set successfully
 * @retval false fail to set callback function
 */
bool usb_slave_set_ctrl_callback(unsigned char type, T_fUSB_CONTROL_CALLBACK callback)
{
    //standard request
    if(REQUEST_STANDARD == type)
        usb_ctrl.std_req_callback = callback;

    //class request
    if(REQUEST_CLASS == type)
        usb_ctrl.class_req_callback = callback;

    //vendor request
    if(REQUEST_VENDOR == type)
        usb_ctrl.vendor_req_callback = callback;

    return true;
}

/**
 * @brief   set usb event(reset, suspend, resume, configok) callback.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param reset_callback [IN] callback function for reset interrupt
 * @param suspend_callback [IN] callback function for suspend interrupt
 * @param resume_callback [IN] callback function for resume interrupt
 * @param configok_callback [IN] callback function for config ok event
 * @return  bool
 * @retval true callback function set successfully
 * @retval false fail to set callback function
 */
bool usb_slave_set_callback(T_fUSB_RESET_CALLBACK reset_callback, T_fUSB_SUSPEND_CALLBACK suspend_callback, T_fUSB_RESUME_CALLBACK resume_callback, T_fUSB_CONFIGOK_CALLBACK configok_callback)
{
    if(!usb_slave.bInit)
        return false;

    usb_slave.reset_callback = reset_callback;
    usb_slave.suspend_callback = suspend_callback;
    usb_slave.resume_callback = resume_callback;
    usb_slave.configok_callback = configok_callback;

    return true;
}

/**
 * @brief   Register a callback function to notify tx send data finish.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in] EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0??
 * @param  callback_func [in]  T_fUSB_TX_FINISH_CALLBACK can be null
 * @return  bool
 * @retval true callback function set successfully
 * @retval false fail to set callback function
 */
bool usb_slave_set_tx_callback(EP_INDEX EP_index, T_fUSB_TX_FINISH_CALLBACK callback_func)
{
    unsigned long tx_index = EP_index;
    
    if(!usb_slave.bInit)
        return false;

    usb_slave.ep[tx_index].tx.TX_Finish = callback_func;
    
    return true;
}

/**
 * @brief   Register a callback function to notify rx receive data finish and rx have data.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in] EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0
 * @param  notify_rx [in] rx notify callbakc function, can be null
 * @param  rx_finish [in] rx finish callbakc function, can be null
 * @return  bool
 * @retval true callback function set successfully
 * @retval false fail to set callback function
 */
bool usb_slave_set_rx_callback(EP_INDEX EP_index, T_fUSB_NOTIFY_RX_CALLBACK notify_rx, T_fUSB_RX_FINISH_CALLBACK rx_finish)
{
    unsigned long rx_index = EP_index;
    
    if(!usb_slave.bInit)
        return false;

    usb_slave.ep[rx_index].rx.RX_Notify = notify_rx;
    usb_slave.ep[rx_index].rx.RX_Finish = rx_finish;

    return true;
}

//********************************************************************
/**
 * @brief   enable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param mode  [in] unsigned long usb mode
 * @return  void
 */
void usb_slave_device_enable(unsigned long mode)
{
    unsigned long regvalue;

    //check init
    if(!usb_slave.bInit)
    {
        akprintf(C3, M_DRVSYS, "usb slave isn't init\n");
        return;
    }
	#ifndef BURNTOOL
    //open usb clock 
	sysctl_clock(CLOCK_USB_ENABLE);
	//reset usb controller

	sysctl_reset(RESET_USB_OTG);
	// usb id config
	REG32(USB_I2S_CTRL_REG) |= (0x3<<12); 
   	REG32(USB_I2S_CTRL_REG) &= (~(0x3f<<6)); 	//enable device

	
   	REG32(USB_I2S_CTRL_REG) |= (0x17<<6); 		//Enable the usb transceiver and suspend enable

	REG32(USB_I2S_CTRL_REG) &= (~0x7);
	REG32(USB_I2S_CTRL_REG) |= (0x4); 			//Enable the usb transceiver and suspend enable

    REG8(USB_REG_POWER) = USB_POWER_ENSUSPEND | USB_POWER_HSENABLE;
   	#endif

    //set speed mode
    if(USB_MODE_20 == (USB_MODE_20 & mode))
    {
        REG8(USB_REG_POWER) = 0x20;
        usb_slave.ulInitMode = USB_MODE_20;
        usb_slave.usb_max_pack_size = 512;
        usb_slave.mode = USB_MODE_20;   
        akprintf(C2, M_DRVSYS, "high speed!\n");
    }
    else
    {
        REG8(USB_REG_POWER) = 0x0;
        usb_slave.ulInitMode = USB_MODE_11;
        usb_slave.usb_max_pack_size = 64;
        usb_slave.mode = USB_MODE_11;   
        akprintf(C2, M_DRVSYS, "full speed!\n");
    }

    //set status
    usb_slave_set_state(USB_CONFIG);

    //enable usb irq
    int_register_irq(INT_VECTOR_USB, usb_slave_intr_handler);

    //enable usb irq
    int_register_irq(INT_VECTOR_USB_DMA, usb_slave_intr_handler);

}

//********************************************************************
/**
 * @brief   disable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_slave_device_disable(void)
{
    //disable irq
    INTR_DISABLE(IRQ_MASK_USB_BIT);

    //close USB clock
    sysctl_clock(~CLOCK_USB_ENABLE);

    //disable transceiver
    //REG32(USB_CONTROL_REG) &= (~0x1f); 
    
    //clear power reg
    REG8(USB_REG_POWER) = 0;

    //reset usb controller
    sysctl_reset(RESET_USB_OTG);
}


unsigned long usb_slave_get_mode(void)
{
    if( (REG8(USB_REG_POWER) & USB_POWER_HSMODE) == USB_POWER_HSMODE )
    {
        return USB_MODE_20;
    }
    else
    {
        return USB_MODE_11;
    }
}

//********************************************************************
static void usb_slave_reset(void)
{
    unsigned long temp;

    //set address to default addr
    usb_slave_set_address(0);

    //check which speed now on after negotiation
    if( (REG8(USB_REG_POWER) & USB_POWER_HSMODE) == USB_POWER_HSMODE )
    {
        REG8(USB_REG_POWER) = 0x20;
        
        usb_slave.usb_max_pack_size = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
        usb_slave.mode = USB_MODE_20;   
        
        akprintf(C3, M_DRVSYS, "reset to high speed!\n");
    }
    else
    {
        REG8(USB_REG_POWER) = 0x0;
        
        usb_slave.usb_max_pack_size = EP_BULK_FULLSPEED_MAX_PAK_SIZE;
        usb_slave.mode = USB_MODE_11;
        
        akprintf(C3, M_DRVSYS, "reset to full speed!\n");
    }


    //open all common interrupt except sof
    REG8(USB_REG_INTRUSBE) = 0xFF & (~USB_INTR_SOF);      //disable the sof interrupt

    //config all endpoint
    usb_slave_reset_ep(USB_EP1_INDEX, EP_BULK_HIGHSPEED_MAX_PAK_SIZE, USB_EP_OUT_TYPE, USB_DMA_UNSUPPORT);
    usb_slave_reset_ep(USB_EP2_INDEX, EP_BULK_HIGHSPEED_MAX_PAK_SIZE, USB_EP_IN_TYPE, USB_DMA_UNSUPPORT);
    usb_slave_reset_ep(USB_EP3_INDEX, EP_BULK_HIGHSPEED_MAX_PAK_SIZE, USB_EP_OUT_TYPE, USB_DMA_UNSUPPORT);
    usb_slave_reset_ep(USB_EP4_INDEX, EP_BULK_FULLSPEED_MAX_PAK_SIZE, USB_EP_IN_TYPE, USB_DMA_UNSUPPORT);

    //enable the TX endpoint
    REG8(USB_REG_INTRTX1E) = (USB_EP0_ENABLE | USB_EP2_TX_ENABLE | USB_EP4_TX_ENABLE);

    //enable the RX endpoint
    REG8(USB_REG_INTRRX1E) = (USB_EP1_RX_ENABLE | USB_EP3_RX_ENABLE);

    //clear the interrupt
    temp = REG8(USB_REG_INTRUSB);
    temp = REG8(USB_REG_INTRTX1);
    temp = REG8(USB_REG_INTRTX2);
    temp = REG8(USB_REG_INTRRX1);
    temp = REG8(USB_REG_INTRRX2);

    //select this EP0
    REG8( USB_REG_INDEX) = USB_EP0_INDEX;

    usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
}
//********************************************************************
static void usb_slave_suspend(void)
{
    //device is at full speed when in suspend ,so reinit high speed
    if (USB_MODE_20 == usb_slave.ulInitMode)
    {
        REG8(USB_REG_POWER) = 0x20;
    }
}

//********************************************************************
static void usb_slave_reset_ep(unsigned long EP_index, unsigned short wMaxPacketSize, unsigned char ep_type, unsigned char dma_surport)
{
    unsigned long fifo_size;
    unsigned char tmp;

    //select the ep
    REG8(USB_REG_INDEX) = EP_index;             /* select this EP */

    //select ep type and max packet size
    if( ep_type == USB_EP_IN_TYPE )
    {
        REG8(USB_REG_TXCSR2) = USB_TXCSR2_MODE;
        REG16(USB_REG_TXMAXP1) = wMaxPacketSize;
    }
    else if ( ep_type == USB_EP_OUT_TYPE )
    {
        REG8(USB_REG_TXCSR2) = 0;
        REG16(USB_REG_RXMAXP1) = wMaxPacketSize;
        REG8(USB_REG_RXCSR1) &=  (~USB_RXCSR1_RXPKTRDY);
    }

}

static unsigned long usb_slave_receive_data(EP_INDEX EP_index)
{
    unsigned short ret, i;
    unsigned long rx_index = EP_index;

    //read the count of receive data
    REG8(USB_REG_INDEX) = EP_index;
    ret = REG16(USB_REG_RXCOUNT1);
    for(i = 0; i < ret; i++)
    {
        usb_slave.ep[rx_index].rx.EP_RX_Buffer[i] = REG8(USB_FIFO_EP0 + (EP_index << 2));
    }
        
    //change global variable status
    if(usb_slave.ep[rx_index].rx.EP_RX_Count > ret && usb_slave.ep[rx_index].rx.EP_RX_Count > usb_slave.usb_max_pack_size)
        usb_slave.ep[rx_index].rx.EP_RX_Count -= ret;
    else
    {
        usb_slave.ep[rx_index].rx.EP_RX_Count = 0;
        usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_FINISH;
    }

    usb_slave.ep[rx_index].rx.EP_RX_Buffer += ret;

    //clear RXPKTRDY
    REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

    return ret;
}

static unsigned long usb_slave_dma_start(EP_INDEX EP_index)
{
    unsigned long ret;
    unsigned long tx_index = EP_index;
    
    //set autoset/DMAReqEnable/DMAReqMode
    REG8(USB_REG_INDEX) = EP_index;
    REG8(USB_REG_TXCSR2) |= (USB_TXCSR2_DMAMODE|USB_TXCSR2_DMAENAB|USB_TXCSR2_AUTOSET); 
    
    ret = usb_slave.ep[tx_index].tx.EP_TX_Count;
    ret -= (ret % usb_slave.usb_max_pack_size);
    if (ret > DMA_PER_BUF_SIZE)
        ret = DMA_PER_BUF_SIZE;        
    
    //send data to l2
    l2_clr_status(usb_slave.ep[tx_index].tx.L2_Buf_ID);
    l2_combuf_dma((unsigned long)usb_slave.ep[tx_index].tx.EP_TX_Buffer, usb_slave.ep[tx_index].tx.L2_Buf_ID, ret, MEM2BUF, false);
    
    REG32(USB_DMA_ADDR_1) = 0x71000000;
    REG32(USB_DMA_COUNT_1) = ret;
    
    //change global variable value    
    usb_slave.ep[tx_index].tx.EP_TX_Count -= ret;
    usb_slave.ep[tx_index].tx.EP_TX_Buffer += ret;
    
    REG32(USB_DMA_CNTL_1) = (USB_ENABLE_DMA | USB_DIRECTION_TX| USB_DMA_MODE1 | USB_DMA_INT_ENABLE| (EP_index<<4) | USB_DMA_BUS_MODE3);
    return ret;

}

static unsigned long usb_slave_send_data(EP_INDEX EP_index)
{
    unsigned long count, i;
    unsigned long tx_index = EP_index;

    count = usb_slave.ep[tx_index].tx.EP_TX_Count;
    if (0)//(usb_slave.mode == USB_MODE_20) && (usb_slave.ep[tx_index].tx.EP_TX_Count >= usb_slave.usb_max_pack_size) )
    {
        count = usb_slave_dma_start(tx_index);
    }
    else 
    {
        if (count > usb_slave.usb_max_pack_size)
            count = usb_slave.usb_max_pack_size;
        
        for(i = 0; i < count; i++)
        {
            REG8(USB_FIFO_EP0 + (EP_index << 2)) = usb_slave.ep[tx_index].tx.EP_TX_Buffer[i];
        }   
        //change global variable value    
        usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_SENDING;
        usb_slave.ep[tx_index].tx.EP_TX_Count = usb_slave.ep[tx_index].tx.EP_TX_Count - count;
        usb_slave.ep[tx_index].tx.EP_TX_Buffer += count;    
        //set TXPKTRDY, start sending    
        REG8(USB_REG_INDEX) = EP_index;
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_TXPKTRDY;
        
    }
    
    return count;

}


//********************************************************************
/**
 * @brief   read usb data with end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in]  usb end point.
 * @param  pBuf [out] usb data buffer.
 * @param  count [in] count to be read
 * @return unsigned long data out count
 */
unsigned long usb_slave_data_out(EP_INDEX EP_index, void *pBuf, unsigned long count)
{
    unsigned long rx_index = EP_index;
    unsigned long ret = 0, res, i;

    if(EP0_INDEX == EP_index)
    {
        akprintf(C1, M_DRVSYS, "usb_slave_data_out: error ep number: %d\n", EP_index);
        return 0;
    }

    if(usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_RECEIVING)
    {
        akprintf(C1, M_DRVSYS, "usb_slave_data_out: still receiving\n");
        return 0;
    }
    
    usb_slave.ep[rx_index].rx.EP_RX_Buffer = pBuf;
    usb_slave.ep[rx_index].rx.EP_RX_Count = count;
    usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_RECEIVING;
    
    //read the count of receive data
    REG8(USB_REG_INDEX) = EP_index;
    ret = REG16(USB_REG_RXCOUNT1);
    //read data from fifo
    for(i = 0; i < ret; i++)
        usb_slave.ep[rx_index].rx.EP_RX_Buffer[i] = REG8(USB_FIFO_EP0 + (EP_index << 2));

    if(usb_slave.ep[rx_index].rx.EP_RX_Count <= ret ||
        usb_slave.ep[rx_index].rx.EP_RX_Count <= usb_slave.usb_max_pack_size)
    {
        REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

        usb_slave.ep[rx_index].rx.EP_RX_Count = 0;
        usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_FINISH;

        if(usb_slave.ep[rx_index].rx.RX_Finish != NULL)
            usb_slave.ep[rx_index].rx.RX_Finish(); 

        return ret;
    }

    usb_slave.ep[rx_index].rx.EP_RX_Count -= ret;
    usb_slave.ep[rx_index].rx.EP_RX_Buffer += ret;
    
    #ifdef BURNTOOL
	if (0)
	#else
    if((usb_slave.mode == USB_MODE_20) && (usb_slave.ep[rx_index].rx.EP_RX_Count > usb_slave.usb_max_pack_size))
	#endif
	{

		res = usb_slave.ep[rx_index].rx.EP_RX_Count % usb_slave.usb_max_pack_size;
        if (0 != res)
        {
            ret = usb_slave.ep[rx_index].rx.EP_RX_Count - res + usb_slave.usb_max_pack_size;
        }
        else
        {
            ret = usb_slave.ep[rx_index].rx.EP_RX_Count;
        }

        REG8(USB_REG_RXCSR2) |= (USB_RXCSR2_AUTOCLEAR | USB_RXCSR2_DMAENAB | USB_RXCSR2_DMAMODE);

        l2_combuf_dma((unsigned long)usb_slave.ep[rx_index].rx.EP_RX_Buffer, usb_slave.ep[rx_index].rx.L2_Buf_ID, ret, BUF2MEM, false);

        REG32(USB_DMA_ADDR_2) = 0x70000000;
        REG32(USB_DMA_COUNT_2) = ret;

        usb_slave.ep[rx_index].rx.EP_RX_Count = 0;
        usb_slave.ep[rx_index].rx.bDmaStart = true;

        REG32(USB_DMA_CNTL_2) = (USB_ENABLE_DMA|USB_DIRECTION_RX|USB_DMA_MODE1|USB_DMA_INT_ENABLE|(EP_index<<4)|USB_DMA_BUS_MODE3);
    }
    
    REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

    return ret;
}

/**
 * @brief   write usb data with end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in]  usb end point.
 * @param  data [in] usb data buffer.
 * @param  count [in] count to be send.
 * @return  unsigned long data in count
 */
unsigned long usb_slave_data_in(EP_INDEX EP_index, unsigned char *data, unsigned long count)
{
    unsigned long tx_index = EP_index;
    unsigned long ret = 0;

    //check EP_index
    if(EP0_INDEX == EP_index)
    {
        akprintf(C1, M_DRVSYS, "usb_slave_data_in: error ep number: %d\n", EP_index);
        return 0;        
    }

    //check status
    if(usb_slave.ep[tx_index].tx.EP_TX_State == EP_TX_SENDING)
    {
        akprintf(C1, M_DRVSYS, "usb_slave_data_in: still sending\n");
        return 0;
    }

    usb_slave.ep[tx_index].tx.EP_TX_Buffer = data;
    usb_slave.ep[tx_index].tx.EP_TX_Count = count;
    usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_SENDING;

    #ifdef BURNTOOL
	if (0)
	#else
    if((usb_slave.mode == USB_MODE_20) && (usb_slave.ep[tx_index].tx.EP_TX_Count > usb_slave.usb_max_pack_size))
	#endif
	{
        ret = usb_slave_dma_start(tx_index);
    }
    else
    {
		ret = usb_slave_send_data(EP_index);
    }
    
    return ret;
}


/**
 * @brief   set usb slave stage.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  state [in] unsigned char.
 * @return  void
 */
void usb_slave_set_state(unsigned char stage)
{
    usb_slave.state = stage;
    if(usb_slave.state == USB_OK)
    {
        if(usb_slave.configok_callback != NULL)
            usb_slave.configok_callback();
    }
}

/**
 * @brief   get usb slave stage.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  unsigned char
 */
unsigned char usb_slave_getstate(void)
{
    return usb_slave.state;
}

static unsigned long usb_slave_ctrl_in(unsigned char *data, unsigned long len)
{
    unsigned long i;

    REG8(USB_REG_INDEX) = EP0_INDEX;

    if(0 == len)
    {
        REG32(USB_EP0_TX_COUNT) = len;
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY | USB_CSR0_P_DATAEND);

        return 0;
    }

    if(len > EP0_MAX_PAK_SIZE)
        len = EP0_MAX_PAK_SIZE;

    //write fifo
    for( i = 0; i < len; i++ )
    {
        REG8( USB_FIFO_EP0 ) = data[i];
    }


    //set TXPKTRDY, if last packet set data end
    if(len < EP0_MAX_PAK_SIZE)
    {
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY | USB_CSR0_P_DATAEND);
    }
    else
    {
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY);
    }
            
    return len;
}

static unsigned long usb_slave_ctrl_out(unsigned char *data)
{
    unsigned long ret, i;
    unsigned char tmp;

    unsigned long temp_buf[512/4];
    unsigned long temp_count = 0;

    //get rx data count
    REG8(USB_REG_INDEX) = EP0_INDEX;
    ret = REG16(USB_REG_RXCOUNT1);
    
    temp_count = ret;    
    if((temp_count & 0x3) != 0)
        temp_count = (ret + 4) & ~0x3;

    //read data from usb fifo
    for(i = 0; i < ret; i++)
    {
        data[i] = REG8(USB_FIFO_EP0);
    }

    return ret;
}

static bool usb_slave_ctrl_callback(unsigned char req_type)
{
    bool ret;
    T_CONTROL_TRANS *pTrans = (T_CONTROL_TRANS *)&usb_ctrl.ctrl_trans;
    
    if((REQUEST_STANDARD == req_type) && usb_ctrl.std_req_callback)
    {
        ret = usb_ctrl.std_req_callback(pTrans);
    }
    else if((REQUEST_CLASS == req_type) && usb_ctrl.class_req_callback)
    {
        ret = usb_ctrl.class_req_callback(pTrans);
    }
    else if((REQUEST_VENDOR == req_type) && usb_ctrl.vendor_req_callback)
    {
        ret = usb_ctrl.vendor_req_callback(pTrans);
    }

    if(!ret)
    {
        usb_slave_ep_stall(EP0_INDEX);
    }

    return ret;
}


//********************************************************************
/**
 * @brief   write usb address.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  address [in]  usb device address.
 * @return  void
 */
void usb_slave_set_address(unsigned char address)
{
    REG8(USB_REG_FADDR) = address;
}

//********************************************************************
/**
 * @brief   set ep status.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @param bStall  [in]  stall or not.
 * @return  void
 */
void usb_slave_set_ep_status(unsigned char EP_Index, bool bStall)
{
    if(EP_Index >= MAX_EP_NUM)
        return;

    if(bStall)
        m_ep_status[EP_Index] = 1;
    else
        m_ep_status[EP_Index] = 0;
}

/**
 * @brief   get ep status.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  void
 */
unsigned short usb_slave_get_ep_status(unsigned char EP_Index)
{
    if(EP_Index >= MAX_EP_NUM)
        return 0;

    return m_ep_status[EP_Index];
}

//********************************************************************
/**
 * @brief  stall ep
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  void
 */
void usb_slave_ep_stall(unsigned char EP_index)
{
    REG8(USB_REG_INDEX) = EP_index;

    if(USB_EP0_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) |= USB_CSR0_P_SENDSTALL;
    }
    else if(EP2_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_P_SENDSTALL;
        usb_slave_set_ep_status(EP_index, true);
    }
    else if(EP1_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_P_SENDSTALL;
        usb_slave_set_ep_status(EP_index, true);
    }
    else if(EP3_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_P_SENDSTALL;
        usb_slave_set_ep_status(EP_index, true);
    }
}

/**
 * @brief  clear stall
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  void
 */
void usb_slave_ep_clr_stall(unsigned char EP_index)
{
    REG8(USB_REG_INDEX) = EP_index;

    if(EP0_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) &= (~USB_CSR0_P_SENTSTALL);
    }
    else if(EP2_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) &=(~(USB_TXCSR1_P_SENDSTALL|USB_TXCSR1_P_SENTSTALL));
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_CLRDATATOG;
        usb_slave_set_ep_status(EP_index, false);
    }
    else if(EP1_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) &= (~(USB_RXCSR1_P_SENDSTALL | USB_RXCSR1_P_SENTSTALL));
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
        usb_slave_set_ep_status(EP_index, false);
    }
    else if(EP3_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) &= (~(USB_RXCSR1_P_SENDSTALL | USB_RXCSR1_P_SENTSTALL));
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
        usb_slave_set_ep_status(EP_index, false);
    }
}

void usb_slave_clr_toggle()
{
    REG8(USB_REG_INDEX) = USB_EP2_INDEX;
    REG8(USB_REG_TXCSR1) |= USB_TXCSR1_CLRDATATOG;

    REG8(USB_REG_INDEX) = USB_EP1_INDEX;
    REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;

    REG8(USB_REG_INDEX) = USB_EP3_INDEX;
    REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
}


//********************************************************************
static void usb_slave_write_ep_reg(unsigned char EP_index, unsigned long reg, unsigned short value)
{
    REG8(USB_REG_INDEX) = EP_index;
    REG16(reg) = value;
}
//********************************************************************
static void usb_slave_read_ep_reg(unsigned char EP_index, unsigned long reg, unsigned short *value)
{
    REG8(USB_REG_INDEX) = EP_index;
    *value = REG16(reg);
}
//********************************************************************
static void usb_slave_read_int_reg(unsigned char *value0, unsigned short *value1, unsigned short *value2)
{
    *value0 = REG8(USB_REG_INTRUSB);
    *value1 = REG16(USB_REG_INTRTX1);
    *value2 = REG16(USB_REG_INTRRX1);
}
//********************************************************************

/**
 * @brief   read data count of usb end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index [in] usb end point.
 * @param cnt  [out] cnt data count.
 * @return  void
 */
void  usb_slave_read_ep_cnt(unsigned char EP_index, unsigned long *cnt)
{
    REG8(USB_REG_INDEX) = EP_index;
    *cnt = REG16(USB_REG_RXCOUNT1);
}

/**
 * @brief   set usb controller to enter test mode
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  testmode [in] unsigned char test mode, it can be one of the following value: 
 *
 *        Test_J                 0x1
 *
 *        Test_K                 0x2
 *
 *        Test_SE0_NAK       0x3
 *
 *        Test_Packet          0x4
 *
 *        Test_Force_Enable  0x5
 *
 * @return  void
 */
void usb_slave_enter_testmode(unsigned char testmode)
{
    const unsigned char test_packet_data[64] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
    0xEE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF,
    0xEF, 0xF7, 0xFB, 0xFD, 0xFC, 0x7E, 0xBF, 0xDF,
    0xEF, 0xF7, 0xFB, 0xFD, 0x7E, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };   //MENTOR DESIGNE
    
    switch(testmode)
    {
    case Test_SE0_NAK:
        REG8(USB_REG_TESEMODE) = (1 << 0);
        break;

    case Test_J:
        REG8(USB_REG_TESEMODE) = (1 << 1);
        break;

    case Test_K:
        REG8(USB_REG_TESEMODE) = (1 << 2);
        break;

    case Test_Packet:
        {
            unsigned long i;
            
            // write data to usb fifo
            for( i = 0; i < 53; i++)
            {
                REG8(USB_FIFO_EP0) = test_packet_data[i];  
            }

            REG8(USB_REG_TESEMODE) = (1 << 3);

            REG8(USB_REG_CSR0) = USB_CSR0_TXPKTRDY;

            //delay
            for( i = 0; i < 3; i++);
        }
        break;

    default:
        break;
    }
}

//********************************************************************
void  usb_slave_start_send(EP_INDEX EP_index)
{
    unsigned long tx_index = EP_index;

    usb_slave.ep[tx_index].tx.EP_TX_Count = 0;
    usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_FINISH;
    usb_slave.usb_need_zero_packet = false;
}

unsigned long usb_slave_get_intr_type(unsigned char *usb_int, unsigned short *usb_ep_int_tx, unsigned short * usb_ep_int_rx)
{
    unsigned short usb_ep_csr;
    unsigned long tmp;
    unsigned long intr_ep = EP_UNKNOWN;
    unsigned long usb_dma_int;
    unsigned long dma_ep_index, dma_intr;

    if(USB_NOTUSE == usb_slave.state)
    {
        return EP_UNKNOWN;
    }
    
    usb_dma_int = REG32(USB_DMA_INTR);
    
    if( (usb_dma_int & DMA_CHANNEL1_INT)  == DMA_CHANNEL1_INT)
    {
        REG8(USB_REG_INDEX) = USB_EP2_INDEX;
        REG8(USB_REG_TXCSR2) = USB_TXCSR_MODE1;
        REG32(USB_DMA_CNTL_1) = 0;

        l2_combuf_wait_dma_finish(usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID);
        return EP2_DMA_INTR;
    }

    if( (usb_dma_int & DMA_CHANNEL2_INT)  == DMA_CHANNEL2_INT)
    {
        if(usb_slave.ep[EP3_INDEX].rx.bDmaStart)
        {
            dma_ep_index = EP3_INDEX;
            dma_intr = EP3_DMA_INTR;
        }
        else
        {
            dma_ep_index = EP1_INDEX;
            dma_intr = EP1_DMA_INTR;
        }
        
        REG8(USB_REG_INDEX) = dma_ep_index;
        REG8(USB_REG_RXCSR2) = 0;
        REG32(USB_DMA_CNTL_2) = 0;
         
        l2_combuf_wait_dma_finish(usb_slave.ep[dma_ep_index].rx.L2_Buf_ID);
        return dma_intr;
    }
    
    usb_slave_read_int_reg(usb_int, usb_ep_int_tx, usb_ep_int_rx);

    //common interrupt
    if( 0 != (*usb_int & USB_INTR_RESET) ||
        0 != (*usb_int & USB_INTR_SUSPEND) ||
        0 != (*usb_int & USB_INTR_RESUME))
        
    {
        return USB_INTR;
    }

    //EP0 INTR
    if(0 != (*usb_ep_int_tx & USB_INTR_EP0))
    {
         intr_ep |= EP0_INTR;
    }

    //EP1 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP4))
    {
        usb_slave_read_ep_reg(USB_EP4_INDEX, USB_REG_TXCSR1, &usb_ep_csr);

        if (0 == (usb_ep_csr & USB_TXCSR1_TXPKTRDY))
        {
            intr_ep |= EP4_INTR;
        }
        //clear underrun        
        if (0 != (usb_ep_csr & USB_TXCSR1_P_UNDERRUN))
        {            
            usb_slave_write_ep_reg(USB_EP4_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_UNDERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_TXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP4_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }
    
    //EP2 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP2))
    {
        usb_slave_read_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, &usb_ep_csr);

        if (0 == (usb_ep_csr & USB_TXCSR1_TXPKTRDY) && 0 == (usb_ep_csr & USB_TXCSR1_P_SENTSTALL))
        {
            intr_ep |= EP2_INTR;
        }

        //clear underrun        
        if (0 != (usb_ep_csr & USB_TXCSR1_P_UNDERRUN))
        {               
            usb_slave_write_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_UNDERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_TXCSR1_P_SENTSTALL))
        {
            
            usb_slave_write_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }
    
    //receive EP1 INT
    if(0 != (*usb_ep_int_rx & USB_INTR_EP1))
    {
        usb_slave_read_ep_reg(USB_EP1_INDEX, USB_REG_RXCSR1, &usb_ep_csr);

        if (0 != (usb_ep_csr & USB_RXCSR1_RXPKTRDY))
        {
            intr_ep |= EP1_INTR;
        }
    
        //clear over run
        if (0 != (usb_ep_csr & USB_RXCSR1_P_OVERRUN))
        {
            usb_slave_write_ep_reg(USB_EP1_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_OVERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_RXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP1_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }
    //receive EP3 INT
    if(0 != (*usb_ep_int_rx & USB_INTR_EP3))
    {
        usb_slave_read_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, &usb_ep_csr);

        if (0 != (usb_ep_csr & USB_RXCSR1_RXPKTRDY))
        {
            intr_ep |= EP3_INTR;
        }
    
        //clear over run
        if (0 != (usb_ep_csr & USB_RXCSR1_P_OVERRUN))
        {
            usb_slave_write_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_OVERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_RXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }

    //EP2 rx INT
    if(0 != (*usb_ep_int_rx & USB_INTR_EP2))
    {
        akprintf(C1, M_DRVSYS, "EP2 RX INT!\n");
        return EP_UNKNOWN;
    }
    
    //EP3 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP3))
    {
        akprintf(C1, M_DRVSYS, "EP3 TX INT!\n");
        return EP_UNKNOWN;
    }
    
    return intr_ep;
}


static void usb_slave_common_intr_handler(unsigned char usb_int)
{
    //RESET
    if(0 != (usb_int & USB_INTR_RESET))
    {
        //prepare to receive the enumeration
        usb_slave_reset();

        if(usb_slave.reset_callback != NULL)
            usb_slave.reset_callback(usb_slave.mode);

        akprintf(C3, M_DRVSYS, "R\n");
        
        return;
    }
    //SUSPEND
    else if(0 != (usb_int & USB_INTR_SUSPEND))
    {
        usb_slave_suspend();
        //enter the suspend mode
        if(USB_OK == usb_slave.state)
        {
            akprintf(C3, M_DRVSYS, "suspend in config,usb done\n");

            if(usb_slave.suspend_callback != NULL)
                usb_slave.suspend_callback();

            usb_slave_set_state(USB_SUSPEND);
        }
        
        return;
    }
    //RESUME
    else if(0 != (usb_int & USB_INTR_RESUME))
    {
        usb_slave_set_state(USB_OK);

        if(usb_slave.resume_callback != NULL)
            usb_slave.resume_callback();
        
        akprintf(C3, M_DRVSYS, "RESUME\n");
        
        return;
    }
}

static void usb_slave_ep0_rx_handler()
{
    unsigned long data_len;
    unsigned char req_type;
    unsigned char data_dir;
    T_UsbDevReq *pDevReq = (T_UsbDevReq *)&usb_ctrl.ctrl_trans.dev_req;
    
    //stage: idle, setup packet comes
    if(CTRL_STAGE_IDLE == usb_ctrl.ctrl_trans.stage)
    {
        //receive data
        data_len = usb_slave_ctrl_out(usb_ctrl.ctrl_trans.buffer);
        if(data_len != SETUP_PKT_SIZE)
        {
            akprintf(C1, M_DRVSYS, "error setup packet size %d\r\n",data_len);
            return;
        }

        memcpy(&usb_ctrl.ctrl_trans.dev_req, usb_ctrl.ctrl_trans.buffer, SETUP_PKT_SIZE);
    
        usb_ctrl.ctrl_trans.stage = CTRL_STAGE_SETUP;
        usb_ctrl.tx_count = 0;
        usb_ctrl.ctrl_trans.data_len = 0;

        //analysis bmRequest Type
        req_type = (pDevReq->bmRequestType >> 5) & 0x3;
        data_dir = pDevReq->bmRequestType >> 7;
        
        if(0 == pDevReq->wLength)
        {
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }
            //no data stage
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY | USB_CSR0_P_DATAEND;

            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
        }
        else
        {
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY;
            if(!data_dir) //data out
            {
                usb_ctrl.ctrl_trans.stage = CTRL_STAGE_DATA_OUT;
            }
            else         //data in
            {
                usb_ctrl.ctrl_trans.stage = CTRL_STAGE_DATA_IN;
                //start send
                data_len = usb_ctrl.ctrl_trans.data_len;
                if(data_len > EP0_MAX_PAK_SIZE)
                {
                    data_len = EP0_MAX_PAK_SIZE;
                }
                else if(data_len < EP0_MAX_PAK_SIZE)
                {
                    usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
                }

                usb_ctrl.tx_count = data_len;
                usb_slave_ctrl_in(usb_ctrl.ctrl_trans.buffer, data_len);
            }
        }

        return;
    }

    if(CTRL_STAGE_DATA_OUT == usb_ctrl.ctrl_trans.stage)
    {
        data_len = usb_slave_ctrl_out(usb_ctrl.ctrl_trans.buffer + usb_ctrl.ctrl_trans.data_len);
        usb_ctrl.ctrl_trans.data_len += data_len;

        if(data_len < EP0_MAX_PAK_SIZE || usb_ctrl.ctrl_trans.data_len > usb_ctrl.ctrl_trans.dev_req.wLength)
        {
            //callback            
            req_type = (pDevReq->bmRequestType >> 5) & 0x3;
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }

            //last packet
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY | USB_CSR0_P_DATAEND;
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
        }
        else
        {
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY;
        }
    }
}

static void usb_slave_ep0_tx_handler()
{
    unsigned char req_type;
    unsigned long data_trans;
    
    req_type = (usb_ctrl.ctrl_trans.dev_req.bmRequestType >> 5) & 0x3;

    if(CTRL_STAGE_DATA_IN == usb_ctrl.ctrl_trans.stage)
    {
        data_trans = usb_ctrl.ctrl_trans.data_len - usb_ctrl.tx_count;
        if(data_trans > EP0_MAX_PAK_SIZE)
        {
            data_trans = EP0_MAX_PAK_SIZE;
        } 
        else if(data_trans < EP0_MAX_PAK_SIZE)
        {
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
        }

        //send data
        usb_slave_ctrl_in(usb_ctrl.ctrl_trans.buffer + usb_ctrl.tx_count, data_trans);
        usb_ctrl.tx_count += data_trans;

        return;
    }

    if(CTRL_STAGE_STATUS == usb_ctrl.ctrl_trans.stage)
    {
        usb_slave_ctrl_callback(req_type);

        usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
    }
}


void usb_slave_ep0_intr_handler(unsigned short usb_ep_int_tx)
{
    unsigned short usb_ep_csr;
    bool bError = false;
    
    //because EP0's all interrupt is in USB_REG_INTRTX1
    if(0 != (usb_ep_int_tx & USB_INTR_EP0))
    {
        usb_slave_read_ep_reg(USB_EP0_INDEX, USB_REG_CSR0, &usb_ep_csr);

        //setup end
        if (0 != (usb_ep_csr & USB_CSR0_P_SETUPEND))
        {
            usb_slave_write_ep_reg(USB_EP0_INDEX, USB_REG_CSR0, USB_CSR0_P_SVDSETUPEND);
            bError = true;

            //back to idle stage after steup end
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
        }
        //stall
        else if(0 != (usb_ep_csr & USB_CSR0_P_SENTSTALL))
        {
            //clear stall
            usb_slave_ep_clr_stall(EP0_INDEX);
            bError = true;

            //back to idle stage after stall
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE; 
        }
        //rec pkt
        if (0 != (usb_ep_csr & USB_CSR0_RXPKTRDY))
        {
            usb_slave_ep0_rx_handler();
        }
        //send pkt or status end
        else if(!bError)
        {
            usb_slave_ep0_tx_handler();
        }
    }
    

}

static void usb_slave_tx_handler(EP_INDEX EP_index)
{
    unsigned long tx_index = EP_index;

    if(usb_slave.ep[tx_index].tx.EP_TX_State != EP_TX_SENDING)
    {
        return;
    }

    if(usb_slave.ep[tx_index].tx.EP_TX_Count == 0)
    {
        
        if(usb_slave.ep[tx_index].tx.EP_TX_State != EP_TX_FINISH)
        {
            usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_FINISH;
         
            if(usb_slave.ep[tx_index].tx.TX_Finish != NULL)
                usb_slave.ep[tx_index].tx.TX_Finish();
        }

        return;
    }

    usb_slave_send_data(EP_index);
}


static void usb_slave_rx_handler(EP_INDEX EP_index)
{
    unsigned long i;
    unsigned long *p32;
    unsigned short ret;
    unsigned char *p8;

    unsigned long rx_index = EP_index;
    
    if(usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_FINISH)
    {
        if(usb_slave.ep[rx_index].rx.RX_Notify != NULL)
            usb_slave.ep[rx_index].rx.RX_Notify();
    }
    else
    {
        usb_slave_receive_data(EP_index);

        if(usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_FINISH)
        {
            if(usb_slave.ep[rx_index].rx.RX_Finish != NULL)
                usb_slave.ep[rx_index].rx.RX_Finish();    
        }
    }
}


static bool usb_slave_intr_handler(void)
{
    unsigned char usb_int;
    unsigned short usb_ep_csr;
    unsigned long tmp, usb_dma_int;
    unsigned short usb_ep_int_tx;
    unsigned short usb_ep_int_rx;
    unsigned long intr_ep = EP_UNKNOWN;
    signed long status;

    intr_ep = usb_slave_get_intr_type(&usb_int, &usb_ep_int_tx, &usb_ep_int_rx);

    if((intr_ep & USB_INTR) == USB_INTR)
    {
        usb_slave_common_intr_handler(usb_int);
    }
    
    if((intr_ep & EP0_INTR) == EP0_INTR)
    {
        usb_slave_ep0_intr_handler(usb_ep_int_tx);
    }

    if((intr_ep & EP4_INTR) == EP4_INTR)
    {        
        usb_slave_tx_handler(EP4_INDEX);
    }
    
    if((intr_ep & EP2_INTR) == EP2_INTR)
    {
        usb_slave_tx_handler(EP2_INDEX);
    }
    if ((intr_ep & EP2_DMA_INTR) == EP2_DMA_INTR)
    {
        usb_slave_tx_handler(EP2_INDEX);
    }
    if((intr_ep & EP1_INTR) == EP1_INTR)
    {
        if (usb_slave.ep[EP1_INDEX].rx.bDmaStart)
        {
            usb_slave.ep[EP1_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x70000000;       
            usb_slave.ep[EP1_INDEX].rx.bDmaStart = false;
            REG8(USB_REG_RXCSR2) = 0;
            REG32(USB_DMA_CNTL_2) = 0;
            REG32(USB_DMA_COUNT_2) = 0;
            l2_combuf_stop_dma(usb_slave.ep[EP1_INDEX].rx.L2_Buf_ID);
        }
        usb_slave_rx_handler(EP1_INDEX);
    }
    
    if((intr_ep & EP3_INTR) == EP3_INTR)
    {
        if (usb_slave.ep[EP3_INDEX].rx.bDmaStart)
        {
            usb_slave.ep[EP3_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x71000000;       
            usb_slave.ep[EP3_INDEX].rx.bDmaStart = false;
            REG8(USB_REG_RXCSR2) = 0;
            REG32(USB_DMA_CNTL_2) = 0;
            REG32(USB_DMA_COUNT_2) = 0;
            l2_combuf_stop_dma(usb_slave.ep[EP3_INDEX].rx.L2_Buf_ID);
        }
        usb_slave_rx_handler(EP3_INDEX);
    }
    if ((intr_ep & EP1_DMA_INTR) == EP1_DMA_INTR)
    {
        usb_slave.ep[EP1_INDEX].rx.bDmaStart = false;
        usb_slave.ep[EP1_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x70000000;
        usb_slave.ep[EP1_INDEX].rx.EP_RX_Count = 0;
        usb_slave.ep[EP1_INDEX].rx.EP_RX_State = EP_RX_FINISH;
        if(usb_slave.ep[EP1_INDEX].rx.RX_Finish != NULL)
                usb_slave.ep[EP1_INDEX].rx.RX_Finish();    
    }
    if ((intr_ep & EP3_DMA_INTR) == EP3_DMA_INTR)
    {
        usb_slave.ep[EP3_INDEX].rx.bDmaStart = false;
        usb_slave.ep[EP3_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x71000000;
        usb_slave.ep[EP3_INDEX].rx.EP_RX_Count = 0;
        usb_slave.ep[EP3_INDEX].rx.EP_RX_State = EP_RX_FINISH;
        if(usb_slave.ep[EP3_INDEX].rx.RX_Finish != NULL)
                usb_slave.ep[EP3_INDEX].rx.RX_Finish();    
    }
    return true;
}

bool usb_detect()
{
    unsigned long i;
    bool stat = false;
    unsigned long tmp;

    //temp disable ... for host not figure out yet 
    //if(usb_host_is_device_connected())
    //{
    //    return false;
    //}

    if(usb_slave.state != USB_NOTUSE)
        return true;

    INTR_DISABLE(IRQ_MASK_USB_BIT);

    // enable clock, USB PLL, USB 2.0
    sysctl_clock(CLOCK_USB_ENABLE);

    REG32(USB_CONTROL_REG) &= (~0x7); 
    REG32(USB_CONTROL_REG) |= (0x6);        //Enable the usb transceiver and suspend enable

    //ak37xx dosen't have vbus and id pin,so these bit must be set 
    REG32(USB_I2S_CTRL_REG) &= ~(1UL<<31);
    REG32(USB_I2S_CTRL_REG) |= (1<<30);
    REG32(USB_I2S_CTRL_REG) &= ~(1<<29);
    REG32(USB_I2S_CTRL_REG) |= (1<<28);
    REG32(USB_I2S_CTRL_REG) |= (1<<27);
    REG32(USB_I2S_CTRL_REG) |= (1<<26);

    REG8(USB_REG_POWER) = 0x20;

    for(i=0; i<2000000; i++)
    {
        HAL_READ_UINT32(INT_STATUS_REG, tmp);
        if((tmp & 0x2000000) == 0x2000000)
        {
            *( volatile unsigned long* )INT_STATUS_REG &= ~0x2000000;
            //HAL_READ_UINT8(USB_REG_INTRUSB, usb_int);         
            if(REG8(USB_REG_INTRUSB)&USB_INTR_SOF)
            {
                stat = true;
                break;
            }
        }
    }

    //close USB clock
    sysctl_clock(~CLOCK_USB_ENABLE);

    REG32(USB_CONTROL_REG) &= (~0x7); 

    REG8(USB_REG_POWER) = 0;

    REG32(RESET_CTRL_REG) |= RESET_CTRL_USB;
    REG32(RESET_CTRL_REG) &= ~RESET_CTRL_USB;
    akprintf(C2, M_DRVSYS, "usb detect %d\n", stat);
    //this delay is necessary when host is mac or linux,otherwise it will be no usb reset before enum when phy is opened again
    mini_delay(500);

    return(stat);
   
}

#endif

