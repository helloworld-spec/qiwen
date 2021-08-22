/**
* @file l2.c
* @brief l2 buffer driver file
* Copyright (C) 2007 Anyka (Guang zhou) Software Technology Co., LTD
* @author liao_zhijun
* @date 2010-07-11
*/
#include "l2.h"
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "interrupt.h"
#include "sysctl.h"
#include "drv_timer.h"

#define IDLE_STATE              1
#define USED_STATE              0

//define Fraction DMA Address Information Register 's bit map
#define AHB_FLAG_EN             29
#define LDMA_FLAG_EN            28

//define DMA Request Register 's bit map
#define DMA_EN                  0

#define MAX_DMA_WAIT_TIME       (get_cpu_freq()/16)

#define L2_COMMON_BUFFER_NUM    8
#define L2_UART_BUFFER_NUM      8

typedef struct
{
    unsigned char   buf_id;
    bool usable;
    unsigned short  used_time;
}L2_INFO;

typedef struct
{
    DEVICE_SELECT device;
    unsigned char buf_id;
}DEVICE_INFO;

typedef struct
{
    bool bStartDMA;
    bool bIntr;
    unsigned char   tran_dir;
    void *pDMAAddr;
    unsigned long  nDMADataLen;
    bool bNeedFrac;
    bool bStartFrac;
    void *pFracAddr;
    unsigned long  nFracOffset;
    unsigned long  nFracDataLen;
    T_fL2_CALLBACK callback_func;
}L2_DMA_INFO;

/*
    the following structure is for l2 cpu interrupt mode
*/
typedef struct {
    unsigned long   addr;
    unsigned long   nbr;
    signed long timer;
    unsigned char    offset;     //write offset
    unsigned char    buf_id;
    unsigned char    dir;
    unsigned char    stat;       //transfering state
}T_L2_TRANS;

static volatile T_L2_TRANS s_l2_trans;

static bool l2_interrupt_handler();
static void l2_select_combuf(DEVICE_SELECT dev_sel, unsigned char buf_id);
static void l2_combuf_ctrl(unsigned char buf_id, bool bEnable);
static void l2_dma(unsigned long ram_addr, unsigned char buf_id, unsigned long tran_byte, unsigned char tran_dir, bool bIntr);
static void l2_frac_dma(unsigned long ram_addr, unsigned char buf_id, unsigned char buf_offset, unsigned long tran_byte, unsigned char tran_dir, bool bIntr);
static bool l2_wait_dma_finish(unsigned char buf_id);
static void l2_cpu(unsigned long ram_addr, unsigned char buf_id, unsigned long buf_offset, unsigned long tran_byte, unsigned char tran_dir);
static void l2_clear_dma(unsigned char buf_id);
static bool l2_get_dma_param(unsigned long tran_byte, unsigned long *cnt_low, unsigned long *cnt_high);

volatile static L2_DMA_INFO s_L2_DMA[L2_COMMON_BUFFER_NUM+L2_UART_BUFFER_NUM];
volatile static bool s_bFracStart = false;

static L2_INFO L2_INFO_TABLE[] = 
{
    {0, IDLE_STATE, 0},
    {1, IDLE_STATE, 0},
    {2, IDLE_STATE, 0},
    {3, IDLE_STATE, 0},
    {4, IDLE_STATE, 0},
    {5, IDLE_STATE, 0},
    {6, IDLE_STATE, 0},    
    {7, IDLE_STATE, 0}
};

static DEVICE_INFO DEVICE_INFO_TABLE[] = 
{
    {ADDR_USB_EP1,          BUF_NULL},
    {ADDR_USB_EP2,          BUF_NULL},
    {ADDR_USB_EP3,          BUF_NULL},    
    {ADDR_RESERVE,          BUF_NULL},    
    {ADDR_MMC_SD,             BUF_NULL},
    {ADDR_SDIO,             BUF_NULL},    
    {ADDR_RESERVE,          BUF_NULL},    
    {ADDR_SPI0_RX,          BUF_NULL},
    {ADDR_SPI0_TX,          BUF_NULL},        
    {ADDR_DAC,              BUF_NULL},
    {ADDR_SPI1_RX,          BUF_NULL},    
    {ADDR_SPI1_TX,          BUF_NULL},    
    {ADDR_RESERVE,          BUF_NULL},    
    {ADDR_PCM,              BUF_NULL},    
    {ADDR_ADC2,             BUF_NULL},
    {ADDR_USB_EP4,          BUF_NULL},
	
    {ADDR_RESERVE,          BUF_NULL}
};


/**
* @brief initial l2 buffer
* @author liao_zhijun
* @date 2010-06-09
* @return void 
*/
void l2_init(void)
{
		//reset L2 controller
		sysctl_reset(RESET_L2);
		
    //open l2 clock
    sysctl_clock(CLOCK_L2_ENABLE);

    //enable l2 dma
    REG32(L2_DMA_REQ) = (1 << DMA_EN);

    //use auto cpu-controlling of buffer status
    REG32(L2_FRAC_ADDR) = (1<<LDMA_FLAG_EN)|(1<<AHB_FLAG_EN);

    //disable all common buffer
    REG32(L2_COMBUF_CFG) = 0x00000000;

    //enable uart buffer and clear uart buffer status
    REG32(L2_UARTBUF_CFG) = 0x90c30000;

    //enable l2 irq interrupt, but disable the interrupt of all the buffers
    int_register_irq(INT_VECTOR_L2, l2_interrupt_handler);
    REG32(L2_INT_ENA) = 0x0;

    memset(s_L2_DMA, 0, sizeof(s_L2_DMA));
    s_bFracStart = false;

    s_l2_trans.offset = 0;
    s_l2_trans.timer = ERROR_TIMER;
    s_l2_trans.stat = 0;
    s_l2_trans.buf_id = BUF_NULL;
}  

/**
* @brief allocate a common l2 buffer for giving device
* 
* first we try to find a common buffer which is not used by other device yet, 
* then clear it's status and attach it to the giving device
* 
* @author liao_zhijun
* @date 2010-06-09
* @param dev_slct [in]: device that need a l2 buffer 
* @return unsigned char buffer id
* @retval 0~7 alloc success and the buffer id is returned
* @retval 0xFF no buffer found, alloc fail
*/
unsigned char l2_alloc(DEVICE_SELECT dev_slct)
{
    unsigned long buffer_nbr = sizeof(L2_INFO_TABLE)/sizeof(L2_INFO);    
    unsigned short used_least_count=0xffff;
    unsigned long i;    
    unsigned char first_usable_id = BUF_NULL;
    unsigned char slct_id=BUF_NULL;

    //check param
    if(ADDR_RESERVE == dev_slct)
    {
        akprintf(C1, M_DRVSYS, "try to alloc to reserved dev!!\r\n");
        return BUF_NULL;
    }

    //check if device already owns a common buffer
    if(DEVICE_INFO_TABLE[(unsigned char)dev_slct].buf_id != BUF_NULL)
    {
        return DEVICE_INFO_TABLE[(unsigned char)dev_slct].buf_id;
    }

    irq_mask();
    
    //try to find a common buffer that is not used yet
    //among all the free buffers we will seek the most seldom used one
    //NOTE: reserved buffer 0 for running program in inner ram
    for (i = 1; i < buffer_nbr; i++)
    {
        if (L2_INFO_TABLE[i].usable == IDLE_STATE)
        {
            if (first_usable_id == BUF_NULL)
            {
                first_usable_id = L2_INFO_TABLE[i].buf_id;
                used_least_count = L2_INFO_TABLE[i].used_time;
                slct_id = first_usable_id;
            }
            if (L2_INFO_TABLE[i].used_time < used_least_count)
            {
                used_least_count = L2_INFO_TABLE[i].used_time;
                slct_id = L2_INFO_TABLE[i].buf_id;
            }
        }
    }    

    //can't find any buffer which hasn't been used
    if (first_usable_id == BUF_NULL)
    {
        irq_unmask();
        akprintf(C1, M_DRVSYS, "no more l2 buffer to alloc!!\r\n");
        return BUF_NULL;    
    }    

    //we got one 
    L2_INFO_TABLE[slct_id].usable = USED_STATE;
    L2_INFO_TABLE[slct_id].used_time++;
    if (L2_INFO_TABLE[slct_id].used_time == 0)
    {
        for (i = 0; i < buffer_nbr; i++)
            L2_INFO_TABLE[i].used_time = 0; //clear all buffer's used_time
    }

    //enable buffer
    l2_combuf_ctrl(slct_id, true);

    //change device info
    DEVICE_INFO_TABLE[(unsigned char)dev_slct].buf_id = slct_id;

    //select buffer for the device
    l2_select_combuf(dev_slct, slct_id);

    irq_unmask();

    //clear buffer status
    l2_clr_status(slct_id);

    return slct_id;
}

/**
* @brief free the l2 common buffer used by giving device
* 
* @author liao_zhijun
* @date 2010-06-09
* @param dev_slct [in]: device name, the buffer attached with it will be freed 
* @return void
*/
void l2_free(DEVICE_SELECT dev_slct)
{
    unsigned long device_nbr = sizeof(DEVICE_INFO_TABLE)/sizeof(DEVICE_INFO);
    unsigned long i;    
    unsigned char free_id;

    free_id = DEVICE_INFO_TABLE[(unsigned char)dev_slct].buf_id;
    if(free_id == BUF_NULL)
    {
        return;
    }

    if (free_id == s_l2_trans.buf_id)
    {
        s_l2_trans.buf_id = BUF_NULL;
        s_l2_trans.stat = 0;    //stop transfer
    }
    
    l2_clear_dma(free_id);
    
    irq_mask();
    //clr cnt  is forbidden until L2 dma finish or l2_clear_dma() finish
    REG32(L2_DMA_CNT + free_id*4) = 0;
    //disable buffer
    l2_combuf_ctrl(free_id, false);
    //disable dma interrupt of this buffer
    REG32(L2_INT_ENA) &= ~(1 << (9 + free_id));
    
    //change the global variable value
    s_L2_DMA[free_id].bIntr= false;
    s_L2_DMA[free_id].callback_func = NULL;
    if (s_L2_DMA[free_id].bStartDMA || s_L2_DMA[free_id].bStartFrac)
    {
        s_L2_DMA[free_id].bStartDMA = false;
        s_L2_DMA[free_id].bStartFrac = false;
    }

    DEVICE_INFO_TABLE[(unsigned char)dev_slct].buf_id = BUF_NULL;
    L2_INFO_TABLE[free_id].usable = IDLE_STATE;

    irq_unmask();
    
}
/**
* @brief stop l2_combuf_dma tranferring 
* 
* if dma can not finish for some reason,stop l2_combuf_dma manually
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return bool 
* @retval void
*/
void l2_combuf_stop_dma(unsigned char buf_id)
{
     //l2_clear_dma(buf_id);
     REG32(L2_DMA_CNT + buf_id*4) = 0;
     s_L2_DMA[buf_id].bStartDMA = false;
}

static void l2_reset()
{
    unsigned long reg1, reg2, reg3;

    irq_mask();

    //save config
    reg1 = REG32(L2_COMBUF_CFG);
    reg2 = REG32(L2_ASSIGN_REG1);
    reg3 = REG32(L2_ASSIGN_REG2);

    //reset l2
    REG32(RESET_CTRL_REG) |= (1<<RESET_L2);
    REG32(RESET_CTRL_REG) &= ~(1<<RESET_L2);

    //enable l2 dma
    REG32(L2_DMA_REQ) = (1 << DMA_EN);

    //use auto cpu-controlling of buffer status
    REG32(L2_FRAC_ADDR) = (1U << LDMA_FLAG_EN)|(1U << AHB_FLAG_EN);

    //enable uart buffer
    REG32(L2_UARTBUF_CFG) |= (1<<28) | (1<<30);

    REG32(L2_COMBUF_CFG) = reg1;
    REG32(L2_ASSIGN_REG1) = reg2;
    REG32(L2_ASSIGN_REG2) = reg3;

    irq_unmask();

}


static void l2_clear_dma(unsigned char buf_id)
{
    unsigned char state;
    
    // if there is l2 dma request still not finish
    // need to select this buffer controllered by cpu and set the buffer num = 0 
    if(REG32(L2_DMA_REQ)&(1<<(24+buf_id)))
    {
        state = l2_get_status(buf_id);
        akprintf(C3, M_DRVSYS, "unfinished DMA in buf%d, %d ", buf_id, state);

        //if transfer direction is  from l2 to ram, and the buffer status > 0
        //we wait for 100 us and check the status again, if status stay the same, 
        //that means l2 dma already die, need to reset l2 controller 
        if((BUF2MEM == s_L2_DMA[buf_id].tran_dir) && (state > 0))
        {
            us_delay(100);

            if(l2_get_status(buf_id) == state)
            {
                l2_reset();
                akprintf(C3, M_DRVSYS, "reset l2!\r\n");

                return;
             }
        }

        //if there are still l2 dma not finish, mostly becoz the data transfer between 
        //l2 and device controller already broken, we mannully set status to make sure dma finish
        while(REG32(L2_DMA_REQ)&(1<<(24+buf_id)))
        {            
            if(BUF2MEM == s_L2_DMA[buf_id].tran_dir)
            {
                l2_set_status(buf_id,8);
            }
            else
            {                
                l2_clr_status(buf_id);
            }
        }

        akprintf(C3, M_DRVSYS, "cleared!!\r\n");
    }
}

/**
* @brief enable or disable common buffer
* 
* @author liao_zhijun
* @data 2010-06-09
* @param buf_id [in]: buffer id 
* @param bEnable [in]: true: Enable, false: disable 
* @return void
*/
static void l2_combuf_ctrl(unsigned char buf_id, bool bEnable)
{
    if(bEnable)
    {
        //enable buffer and dma
        REG32(L2_COMBUF_CFG) |= (1 << buf_id) | (1 << (buf_id+16));
    }
    else
    {
        //disable buffer and dma
        REG32(L2_COMBUF_CFG) &= ~((1 << buf_id) | (1 << (buf_id+16)));
    }
}

/**
* @BRIEF set a buffer as device buffer
* @author liao_zhijun
* @data 2010-06-09
* @param DEVICE_SELECT dev_slct: the device which will not use buffer
* @param unsigned char buf_id: the buffer id
* @return void:  
*/
static void l2_select_combuf(DEVICE_SELECT dev_sel, unsigned char buf_id)
{
    unsigned long reg_id;
    unsigned long base_bit;
    unsigned long reg_value;

    if ((unsigned char)dev_sel>9)
    {
        reg_id = L2_ASSIGN_REG2;
        base_bit = ((unsigned char)dev_sel-10)*3;
    }
    else
    {
        reg_id = L2_ASSIGN_REG1;
        base_bit = (unsigned char)dev_sel*3;
    }

    reg_value = inl(reg_id);
    reg_value &= ~(0x7<<base_bit);
    reg_value |= ((buf_id&0x7)<<base_bit);
    outl(reg_value, reg_id);  
}

/**
* @brief set a callback function
* @author liao_zhijun
* @date 2010-06-09
* @param callback_func [in]: dam finish call this function
* @param buf_id [in]: the buffer id
* @return bool
*/
bool l2_set_dma_callback(unsigned char buf_id, T_fL2_CALLBACK callback_func)
{
    //check param
    if(buf_id >= L2_COMMON_BUFFER_NUM)
    {
        akprintf(C3, M_DRVSYS, "L2_SetDMACallback : buf_id error %d\n", buf_id);
        return false;
    }
    
    if(s_L2_DMA[buf_id].bStartDMA || s_L2_DMA[buf_id].bStartFrac)
    {
        akprintf(C3, M_DRVSYS, "L2_SetDMACallback : DAM not finish\n");
        return false;
    }

    int_register_irq(INT_VECTOR_L2, l2_interrupt_handler);

    s_L2_DMA[buf_id].callback_func = callback_func;

    return true;
}


/**
* @BRIEF L2 interrupt handler
* @AUTHOR Pumbaa
* @DATE 2007-07-19
* @PARAM 
* @RETURN 
* @NOTE: 
*/
static bool l2_interrupt_handler()
{
    unsigned long reg_value;
    unsigned long i = 0;

    reg_value = REG32(L2_DMA_REQ);
    for(i = 0; i < L2_COMMON_BUFFER_NUM; i++)
    {
        unsigned long tmpvalue = (reg_value >> (24 + i));
                
        if(s_L2_DMA[i].bStartDMA && ((tmpvalue & 0x01) == 0))  //该buf的DMA模式开始，并已完成
        {
            if(!s_bFracStart && s_L2_DMA[i].bNeedFrac)
            {
                akprintf(C3, M_DRVSYS, "[FRAC]\n");
                s_L2_DMA[i].bStartFrac = true;

                s_L2_DMA[i].bStartDMA = false;
                
                //start fraction DAM
                l2_frac_dma((unsigned long)s_L2_DMA[i].pFracAddr, 
                            i, 
                            s_L2_DMA[i].nFracOffset, 
                            s_L2_DMA[i].nFracDataLen, 
                            s_L2_DMA[i].tran_dir,
                            true);
                
                s_bFracStart = true;

				//DMA finish
                REG32(L2_INT_ENA) &= ~(1 << (9 + i));
                
            }
            else
            {
                //DMA finish
                REG32(L2_INT_ENA) &= ~(1 << (9 + i));

                s_L2_DMA[i].bStartDMA = false;
                
                if(s_L2_DMA[i].callback_func != NULL)
                    s_L2_DMA[i].callback_func();
            }
            
        }
       	else if(s_L2_DMA[i].bStartFrac &&(((reg_value >> 9) & 0x1) == 0))
        {
           // if(s_bFracStart && (((reg_value >> 9) & 0x1) == 0))
            {
                s_bFracStart = false;
                
                //fraction DMA finish
               	REG32(L2_INT_ENA) &= ~(1<<0);

                if(s_L2_DMA[i].tran_dir == MEM2BUF)    //from SDRAM to L2
                {
                    if (s_L2_DMA[i].nFracDataLen <= 60)
                    {
                        REG32(l2_get_addr(i) + 0x1fc) = 0;
                    }                    
                }
                else
                {
                    if (s_L2_DMA[i].nFracDataLen <= 512-4)
                        l2_clr_status(i);
                    
                }
                s_L2_DMA[i].bStartFrac = false;

                if(s_L2_DMA[i].callback_func != NULL)
                    s_L2_DMA[i].callback_func();
                
            }
        }
		
    }
    return true;
    
}

unsigned long l2_get_addr(unsigned char buf_id)
{
    unsigned long buf_addr = 0;

	if(buf_id < L2_COMMON_BUFFER_NUM)
	{
		buf_addr = L2_BUF_MEM_BASE_ADDR + buf_id*512;
	}
	else if(buf_id <= 11)
	{
		buf_addr = L2_BUF_MEM_BASE_ADDR + 0x1000 + (buf_id-L2_COMMON_BUFFER_NUM)*128;
	}
	else
	{
        akprintf(C3, M_DRVSYS, "invalid buf id %d\n", buf_id);
        return 0;
	}

    return buf_addr;
}


/**
* @brief transfer data between memory and l2 buffer with cpu
* 
* if their transfer direction is from memory to l2, and trans_bytes is not multiple of 64_bytes,
* l2_cpu will add 1 to buffer flag automatically, user doesn't need to set buffer flag again
* 
* @author liao_zhijun
* @data 2010-06-09
* @param unsigned long ram_addr: the memory address
* @param unsigned char buf_id: the buffer id
* @param unsigned char buf_offset: the buffer offset
* @param unsigned long tran_byte: the size of data to be transfered
* @param unsigned char tran_dir: transfer data from memory to buffer or from buffer to memory
* @return void
*/
static void l2_cpu(unsigned long ram_addr, unsigned char buf_id, unsigned long buf_offset, unsigned long tran_byte, unsigned char tran_dir)
{
    unsigned long tran_nbr, frac_nbr;
    unsigned long buf_cnt, buf_remain;
    unsigned long temp_ram = 0, temp_buf = 0;
    unsigned long i,j;
    unsigned long buf_addr;

    buf_addr = l2_get_addr(buf_id);
    if(0 == buf_addr)
        return;

    buf_addr += buf_offset;
    tran_nbr = tran_byte >> 2;
    frac_nbr = tran_byte & 0x3;
    
    buf_cnt = (buf_offset+tran_byte) / 64;
    buf_remain = (buf_offset+tran_byte) % 64;

    if (tran_dir) 
    {
        //memory to buffer
        if (ram_addr & 0x3)
        {
            for (i=0; i<tran_nbr; i++)
            {
                temp_ram = 0;
                for (j=0; j<4; j++)
                    temp_ram |= ((ReadRamb(ram_addr+i*4+j))<<(j*8));
                WriteBuf(temp_ram, (buf_addr+i*4));
            }
            if (frac_nbr)
            {
                temp_ram = 0;
                for (j=0; j<frac_nbr; j++)
                    temp_ram |= ((ReadRamb(ram_addr+tran_nbr*4+j))<<(j*8));    
                WriteBuf(temp_ram, (buf_addr+tran_nbr*4));
            }
        }
        else
        {
            for (i=0; i<tran_nbr; i++)
            {
                WriteBuf(ReadRaml(ram_addr+i*4), (buf_addr+i*4));
            }
            if (frac_nbr)
            {
                WriteBuf(ReadRaml(ram_addr+tran_nbr*4), (buf_addr+tran_nbr*4));
            }
        }

        //set buffer status
        if((buf_remain > 0) && (buf_remain <= 60))
        {
            WriteBuf(0, (buf_addr-buf_offset+buf_cnt*64+60));
        }
    }
    else
    {
        //buffer to memory
        if (ram_addr%4)
        {
            for (i=0; i<tran_nbr; i++)
            {
                temp_buf = ReadBuf(buf_addr+i*4);            
                for (j=0; j<4; j++)
                {
                    WriteRamb((unsigned char)((temp_buf>>j*8)&0xff), (ram_addr+i*4+j));
                }
            }
            if (frac_nbr)
            {
                temp_buf = ReadBuf(buf_addr+tran_nbr*4);    
                for (j=0; j<frac_nbr; j++)
                {
                    WriteRamb((unsigned char)((temp_buf>>j*8)&0xff), (ram_addr+tran_nbr*4+j));                
                }
            }
        }
        else
        {
            for (i=0; i<tran_nbr; i++)
            {    
                WriteRaml(ReadBuf(buf_addr+i*4), (ram_addr+i*4));
            }
            if (frac_nbr)
            {
                temp_buf = ReadBuf(buf_addr+tran_nbr*4);    
                temp_ram = ReadRaml(ram_addr+tran_nbr*4);
                temp_buf &= ((1<<(frac_nbr*8+1))-1);
                temp_ram &= ~((1<<(frac_nbr*8+1))-1);
                temp_ram |= temp_buf;
                WriteRaml(temp_ram, (ram_addr+tran_nbr*4));
            }
        }

        //clr buffer status
        if((buf_remain > 0) && (buf_remain <= 60))
        {
            temp_buf = ReadBuf(buf_addr-buf_offset+buf_cnt*64+60);
        }
    }
}

static bool l2_get_dma_param(unsigned long tran_byte, unsigned long *cnt_low, unsigned long *cnt_high)
{
    unsigned long factor;
    unsigned long data_len = tran_byte >> 6;

    //just use cnt_cfg if tran_byte < 8K
    if(data_len <= 128)
    {
        *cnt_low = data_len;
        *cnt_high = 0;

        return true;
    }
    else if((data_len&0x7) != 0)
    {
        return false;
    }

    factor = 16*8;

    //loop to get the best param
    while(factor > 0)
    {
        if((data_len % factor) == 0)
        {
            *cnt_low = factor;
            *cnt_high = data_len / factor -1;

            if(*cnt_high < 0xff)
                return true;
            else
                return false;
        }

        factor -= 8;
    }

    return false;
}

/**
* @brief start data tranferring between memory and l2 buffer with dma mode
* 
* if l2 interrupt is disabled here and tran_byte is not the multi-64-bytes, 
* a following call of l2_wait_dma_finish() is required
* 
* @author liao_zhijun
* @data 2010-06-09
* @param unsigned long ram_addr: the memory address
* @param unsigned char buf_id: the buffer id
* @param unsigned long tran_byte: the size of data to be transfered
* @param unsigned char tran_dir: transfer data from memory to buffer or from buffer to memory
* @param bool bIntr: open interrupt for this buffer or not
* @return void
*/
static void l2_dma(unsigned long ram_addr, unsigned char buf_id, unsigned long tran_byte, unsigned char tran_dir, bool bIntr)
{
    unsigned long tran_nbr;
    unsigned long reg_value;
    unsigned long reg_id;
    unsigned long cnt_low = 0, cnt_high = 0;
	//akprintf(C3, M_DRVSYS, "jj buf_id:%d, byte:%d, dir:%d, int:%d\n", buf_id, tran_byte, tran_dir, bIntr);
    //get dma cnt cfg
    if((0 == tran_byte) || (!l2_get_dma_param(tran_byte, &cnt_low, &cnt_high)))
    {
        akprintf(C1, M_DRVSYS, "data length error: %d, %d\n", buf_id, tran_byte);
        return;
    }

    //check if the previous dma finish or not 
    if(s_L2_DMA[buf_id].bStartDMA || s_L2_DMA[buf_id].bStartFrac)
    {
        akprintf(C3, M_DRVSYS, "l2_dma : dma not finish yet\n");
        return;
    }

    //update global variable value
    s_L2_DMA[buf_id].nDMADataLen  = tran_byte >> 6;
    s_L2_DMA[buf_id].nFracDataLen = tran_byte % 64;
    s_L2_DMA[buf_id].pDMAAddr = (void *)ram_addr;
    s_L2_DMA[buf_id].tran_dir = tran_dir;
    s_L2_DMA[buf_id].bIntr = bIntr;

    s_L2_DMA[buf_id].bNeedFrac = false;
    if(s_L2_DMA[buf_id].nFracDataLen != 0)
    {
        s_L2_DMA[buf_id].bNeedFrac = true;
        s_L2_DMA[buf_id].pFracAddr = (void *)((unsigned char *)s_L2_DMA[buf_id].pDMAAddr + (s_L2_DMA[buf_id].nDMADataLen << 6));
        s_L2_DMA[buf_id].nFracOffset = s_L2_DMA[buf_id].nDMADataLen;
    }

    //invalidate data cache
    MMU_Clean_Invalidate_Dcache();

    if(s_L2_DMA[buf_id].nDMADataLen == 0)
    {
        s_L2_DMA[buf_id].bStartDMA = false;
        s_L2_DMA[buf_id].bStartFrac = true;
        
        //start fraction DAM
        l2_frac_dma((unsigned long)s_L2_DMA[buf_id].pFracAddr, 
                            buf_id, 
                            s_L2_DMA[buf_id].nFracOffset, 
                            s_L2_DMA[buf_id].nFracDataLen, 
                            s_L2_DMA[buf_id].tran_dir,
                            bIntr);

        return;
    }
    
    s_L2_DMA[buf_id].bStartDMA = true;

    irq_mask();

    //set address of extern RAM
    reg_value = ((unsigned long)s_L2_DMA[buf_id].pDMAAddr);    
    reg_id = L2_DMA_ADDR+ (buf_id << 2);    
    outl(reg_value, reg_id);


    //set DMA Operation Times
    reg_value = (cnt_high << 16) | (cnt_low & 0xff);    
    reg_id = L2_DMA_CNT + (buf_id<<2);    
    outl(reg_value, reg_id);

    //set DMA Dir for common buffer
    if(buf_id < L2_COMMON_BUFFER_NUM)
    {
        reg_value = inl(L2_COMBUF_CFG);
        if (s_L2_DMA[buf_id].tran_dir)
            reg_value |= ( 1 << (8+buf_id));
        else
            reg_value &= ~(1 << (8+buf_id));
        outl(reg_value, L2_COMBUF_CFG);
    }
    
    //start bufx DMA
    reg_value = inl(L2_DMA_REQ);
    reg_value &= ~((1 << 9) |(0xffffUL << 16));        //clear other buf req
    if(buf_id < 8)
        reg_value |= (1 << (24 + buf_id));
    else
        reg_value |= (1 << (8 + buf_id));
        
    outl(reg_value, L2_DMA_REQ);

    //enable interrupt
    if(s_L2_DMA[buf_id].bIntr)
    {
        //REG32(L2_INT_ENA) |= (1 << (1 + buf_id));
        if(buf_id < L2_COMMON_BUFFER_NUM)
            REG32(L2_INT_ENA) |= (1 << (9 + buf_id));
        else
            REG32(L2_INT_ENA) |= (1 << (1 + buf_id - 8));
    }
	//akprintf(C3, M_DRVSYS, "buf_id:%d, byte:%d, dir:%d, int:%d\n", buf_id, tran_byte, tran_dir, bIntr);

    irq_unmask();

}

/**
* @brief transfer fraction data between memory and l2 common buffer with dma mode
* @author liao_zhijun
* @data 2010-06-09
* @param unsigned long ram_addr: the memory address
* @param unsigned char buf_id: the buffer id
* @param unsigned char buf_offset: the offset between buffer start address and transfer start address
* @param unsigned long tran_byte: the size of data which will be transfered
* @param unsigned char tran_dir: transfer data from memory to buffer or from buffer to memory
* @param bool bIntr: open l2 interrupt for this buffer or not
* @return void: 
* @note: transfer size can be 1~64 byte(s), buffer offset can be 0~7, 1 mean 64 bytes data
*/
static void l2_frac_dma(unsigned long ram_addr, unsigned char buf_id, unsigned char buf_offset, unsigned long tran_byte, unsigned char tran_dir, bool bIntr)
{
    unsigned long high_addr;
    unsigned long buf_addr;
    unsigned long reg_value;
    irq_mask();
	//printf("jk %s %d\r\n", __func__, __LINE__);

    //set fraction address
    high_addr = ram_addr&0xf0000000;
    high_addr = high_addr << 2;

    reg_value = inl(L2_FRAC_ADDR);
    reg_value &= ~0xfffffff;
    reg_value |= ((ram_addr&0xfffffff) | high_addr);
    outl(reg_value, L2_FRAC_ADDR);

    //set Fraction DAM addr
    if(buf_id < L2_COMMON_BUFFER_NUM)
        buf_addr = ( (buf_id&0x7)<<3) | (buf_offset&0x7);
    else
        buf_addr = (0x40+((buf_id-L2_COMMON_BUFFER_NUM)<<1)) | (buf_offset&0x1);
    
    reg_value = inl(L2_DMA_REQ);
    reg_value &= ~( (0x7f<<1) | (0x3f<<10) );
    reg_value &= ~( (1<<9) | (0xffffUL<<16) );        //clear other buf req        

    if (tran_dir)
    {
        if(tran_byte & 0x1) //data to be transfer in frac dma should be even in mem to buf 
            reg_value |= (1<<9)|(1<<8)|(buf_addr<<1)|(tran_byte << 10);    
        else
            reg_value |= (1<<9)|(1<<8)|(buf_addr<<1)|((tran_byte-1) << 10);    
    }
    else
    {
        reg_value &= ~(1<<8);
        reg_value |= (1<<9)|(buf_addr<<1)|((tran_byte-1)<< 10);    
    }
    outl(reg_value, L2_DMA_REQ);
	//printf("jk %s %d,regv=%x\r\n", __func__, __LINE__, inl(L2_DMA_REQ));
    //enable interrupt
    if(bIntr)
        REG32(L2_INT_ENA) |= 1;
	
    irq_unmask();
	//printf("jk %s %d\r\n", __func__, __LINE__);
}

/**
* @brief wait for dma started by l2_combuf_dma tranferring finish
* 
* if the tran_byte set in l2_combuf_dma is not multi-64-bytes, we'll start a fraction dma here
* and wait it finish, if the direction is memory to l2, buffer flag is changed manually
*
* @author liao_zhijun
* @data 2010-06-09
* @param unsigned char buf_id: the buffer id
* @return bool 
* @retval true: previous dma finished succesfully
* @retval false: previous dma fail
*/
static bool l2_wait_dma_finish(unsigned char buf_id)
{
    unsigned long timeout;
    unsigned long dma_bit;
    unsigned long max_wait_time = MAX_DMA_WAIT_TIME;
    
    timeout = 0;
    if(s_L2_DMA[buf_id].bStartDMA)
    {
        dma_bit = (buf_id < 8) ? (24 + buf_id) : ((8 + buf_id));

        //wait dma finish
        do
        {
            if(!(REG32(L2_DMA_REQ) & (1<<dma_bit)))
                break;
        }
        while(timeout++ < max_wait_time); 

        s_L2_DMA[buf_id].bStartDMA = false;


        //check timeout
        if(timeout >= max_wait_time)
        {
            akprintf(C3, M_DRVSYS, "l2 wait dma timeout: buf %d, flag %d!\n", buf_id, l2_get_status(buf_id));

            l2_clear_dma(buf_id);
            REG32(L2_DMA_CNT + buf_id*4) = 0;
            
            return false;
        }
        
        //start frac dma if needed
        if(s_L2_DMA[buf_id].bNeedFrac)
        {
            s_L2_DMA[buf_id].bStartFrac = true;
            
            //start fraction DAM
            l2_frac_dma((unsigned long)s_L2_DMA[buf_id].pFracAddr, 
                                buf_id, 
                                s_L2_DMA[buf_id].nFracOffset, 
                                s_L2_DMA[buf_id].nFracDataLen, 
                                s_L2_DMA[buf_id].tran_dir,
                                false);

        }
        else
        {
            return true;
        }

    }

    timeout = 0;
    if(s_L2_DMA[buf_id].bStartFrac)
    {
        do
        {
            if(!(REG32(L2_DMA_REQ) & FRAC_DMA_START_REQ))
                break;
        }
        while(timeout++ < max_wait_time);

        s_L2_DMA[buf_id].bStartFrac = false;

        //check timeout
        if(timeout >= max_wait_time)
        {
            akprintf(C3, M_DRVSYS, "l2 wait frac dma timeout: buf %d, flag %d!\n", buf_id, l2_get_status(buf_id));
            return false;
        }

        //set flag
        if((s_L2_DMA[buf_id].tran_dir) && (s_L2_DMA[buf_id].nFracDataLen <= 60))
        {
            unsigned long buf_addr;

            buf_addr = l2_get_addr(buf_id);
            WriteBuf(0x0, buf_addr+((s_L2_DMA[buf_id].nFracOffset%8)<<6)+60);       
        }
    }

    return true;
}

/**
* @brief transfer data between memory and l2 common buffer with cpu
* 
* if their transfer direction is from memory to l2, and trans_bytes is not multiple of 64_bytes,
* l2_combuf_cpu will add 1 to buffer flag automatically, user doesn't need to set buffer flag again
* 
* @author liao_zhijun
* @date 2010-06-09
* @param ram_addr [in/out]: the memory address
* @param buf_id [in]: the buffer id
* @param tran_byte [in]: the size of data to be transfered
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @return void
*/
bool l2_combuf_cpu(unsigned long ram_addr, unsigned char buf_id, unsigned long tran_byte, unsigned char tran_dir)
{
    unsigned char *buf = (unsigned char*)ram_addr;
    unsigned long i, loop, remain;
    unsigned long to = 0, maxto = MAX_DMA_WAIT_TIME/4;

    loop = tran_byte / 64;
    remain = tran_byte - loop * 64;
    
    if (tran_dir == MEM2BUF)
    {
        for(i = 0; i < loop; i++)
        {
            //wait for buffer is not full
            to = 0;
            while((l2_get_status(buf_id) == 8) && (to++ < maxto));
            if(to >= maxto) goto L2CPU_TIMEOUT;

            l2_cpu(ram_addr+i*64, buf_id, (i%8)*64, 64, tran_dir);
        }

        if(remain > 0)
        {
            to = 0;
            while((l2_get_status(buf_id) > 0) && (to++ < maxto));
            if(to >= maxto) goto L2CPU_TIMEOUT;
            
            l2_cpu(ram_addr+loop*64, buf_id, (loop%8)*64, remain, tran_dir);
        }
    }
    else
    {
        for(i = 0; i < loop; i++)
        {
            //wait for buffer is not empty
            to = 0;
            while((l2_get_status(buf_id) == 0) && (to++ < maxto));
            if(to >= maxto) goto L2CPU_TIMEOUT;
            
            l2_cpu(ram_addr+i*64, buf_id, (i%8)*64, 64, tran_dir);
        }

        if(remain > 0)
        {
            l2_cpu(ram_addr+loop*64, buf_id, (loop%8)*64, remain, tran_dir);
        }
    }

    return true;

L2CPU_TIMEOUT:
    akprintf(C1, M_DRVSYS, "l2 cpu timeout\n");
    return false;
}


/**
* @brief start data tranferring between memory and l2 common buffer with dma mode
* 
* we just start dma here and return immediately , and the l2 interrupt is disabled here
* if tran_byte is not the multi-64-bytes, a following call of l2_combuf_wait_dma_finish() is required
* 
* @author liao_zhijun
* @date 2010-06-09
* @param ram_addr [in/out]: the memory address
* @param buf_id [in]: the buffer id
* @param tran_byte [in]: the size of data to be transfered
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @param bIntr [in]: open interrupt for this buffer or not
* @return void
*/
void l2_combuf_dma(unsigned long ram_addr, unsigned char buf_id, unsigned long tran_byte, unsigned char tran_dir, bool bIntr)
{
    unsigned long tran_nbr;
    unsigned long reg_value;
    unsigned long reg_id;

    //check param
    if(buf_id >= L2_COMMON_BUFFER_NUM)
    {
        akprintf(C3, M_DRVSYS, "l2_combuf_dma: id error: %d\n", buf_id);
        return;
    }

    l2_dma(ram_addr, buf_id, tran_byte, tran_dir, bIntr);
}

static void l2_timer_cb(signed long timer_id, unsigned long delay)
{
    unsigned long i, bufnum;

    if ((s_l2_trans.stat) == 1 && (s_l2_trans.buf_id != BUF_NULL))
    {
        bufnum = 8 - l2_get_status(s_l2_trans.buf_id);

        for (i=0; i<bufnum && s_l2_trans.nbr!=0; i++)
        {
            l2_cpu(s_l2_trans.addr, s_l2_trans.buf_id, s_l2_trans.offset*64, 64, s_l2_trans.dir);
            s_l2_trans.addr += 64;
            s_l2_trans.nbr -= 64;
            s_l2_trans.offset = (s_l2_trans.offset+1)&0x7;            
        }        
        
        if (s_l2_trans.nbr == 0)    //transfer finish
        {
            s_l2_trans.stat = 0;
            if(s_L2_DMA[s_l2_trans.buf_id].callback_func != NULL)
                s_L2_DMA[s_l2_trans.buf_id].callback_func();
        }
    }    
}

//NOTE: 2011.06.09 by xc
// 该接口是为解决音频播放papa音问题，需要将所有的L2访问改为cpu方式
void l2_combuf_timer(unsigned long ram_addr, unsigned char buf_id, unsigned long tran_byte, unsigned char tran_dir, bool bIntr)
{
    unsigned long tran_nbr;
    unsigned long reg_value;
    unsigned long reg_id;

    //check param
    if(buf_id >= L2_COMMON_BUFFER_NUM)
    {
        akprintf(C3, M_DRVSYS, "l2_combuf_dma: id error: %d\n", buf_id);
        return;
    }

    if (s_l2_trans.timer == ERROR_TIMER)
    {
        s_l2_trans.timer = timer_start(uiTIMER3, 1, true, l2_timer_cb);
        if (s_l2_trans.timer == ERROR_TIMER)
        {
            akprintf(C3, M_DRVSYS, "l2 timer error\n");
            return;
        }        
    }
    
    if (s_l2_trans.stat == 1)
    {
        akprintf(C3, M_DRVSYS, "l2 transfer not finished: %d\n", buf_id);
        return;
    }
    
    s_l2_trans.addr = ram_addr;
    s_l2_trans.buf_id = buf_id;
    s_l2_trans.nbr = tran_byte;
    s_l2_trans.dir = tran_dir;
    s_l2_trans.stat = 1;    //start transfer
    
}

/**
* @brief wait for dma started by l2_combuf_dma tranferring finish
* 
* if the tran_byte set in l2_combuf_dma is not multi-64-bytes, we'll start a fraction dma here
* and wait it finish, if the direction is memory to l2, buffer flag is changed manually
*
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return bool 
* @retval true: previous dma finished succesfully
* @retval false: previous dma fail
*/
bool l2_combuf_wait_dma_finish(unsigned char buf_id)
{
    bool ret;

    ret = l2_wait_dma_finish(buf_id);

    return ret;
}

/**
* @brief transfer data between memory and l2 uart buffer with cpu
* 
* if ther trans direction is from memory to l2, and trans_bytes is not multi-64-bytes,
* we'll set buffer flag manually
* 
* @author liao_zhijun
* @date 2010-06-09
* @param ram_addr [in/out]: the memory address
* @param uart_id [in]: the uart id
* @param tran_byte [in]: the size of data to be transfered
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @return void
*/
void l2_uartbuf_cpu(unsigned long ram_addr, unsigned char uart_id, unsigned long tran_byte, unsigned char tran_dir)
{
    unsigned long buf_id;
    
    if(uart_id >= 2)
    {
        akprintf(C3, M_DRVSYS, "l2_uartbuf_dma: error uart_id %d\n", uart_id);
        return;
    }
    
    if (MEM2BUF == tran_dir)
        buf_id = L2_COMMON_BUFFER_NUM + uart_id*2;
    else
        buf_id = L2_COMMON_BUFFER_NUM + uart_id*2 + 1;

    l2_cpu(ram_addr, buf_id, 0, tran_byte, tran_dir);

}

/**
* @brief start data tranferring between memory and l2 uart buffer with dma mode
* 
* we just start dma here and return immediately , and the l2 interrupt is disabled here
* if tran_byte is not the multi-64-bytes, a following call of l2_uartbuf_wait_dma_finish() is required
* 
* @author liao_zhijun
* @date 2010-06-09
* @param ram_addr [in/out]: the memory address
* @param uart_id [in]: the uart id
* @param tran_byte [in]: the size of data to be transfered
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @return void
*/
void l2_uartbuf_dma(unsigned long ram_addr, unsigned char uart_id, unsigned long tran_byte, unsigned char tran_dir)
{
    unsigned long buf_id;

    if(uart_id >= 2)
    {
        akprintf(C3, M_DRVSYS, "l2_uartbuf_dma: error uart_id %d\n", uart_id);
        return;
    }
    
    if (tran_dir)
        buf_id = L2_COMMON_BUFFER_NUM + uart_id*2;
    else
        buf_id = L2_COMMON_BUFFER_NUM + uart_id*2 + 1;

    l2_dma(ram_addr, buf_id, tran_byte, tran_dir, false);
}

/**
* @brief wait for dma started by l2_uartbuf_dma tranferring finish
* 
* if the tran_byte set in l2_uartbuf_dma is not multi-64-bytes, we'll start a fraction dma here
* and wait it finish, if the direction is memory to l2, buffer flag is changed manually
*
* @author liao_zhijun
* @date 2010-06-09
* @param uart_id [in]: the uart id
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @return bool 
* @retval true: previous dma finished succesfully
* @retval false: previous dma fail
*/
void l2_uartbuf_wait_dma_finish(unsigned char uart_id, unsigned char tran_dir)
{
    unsigned long buf_id;
    
    if(uart_id >= 2)
    {
        akprintf(C3, M_DRVSYS, "l2_uartbuf_dma: error uart_id %d\n", uart_id);
        return;
    }
    
    if (tran_dir)
        buf_id = 8 + uart_id*2;
    else
        buf_id = 8 + uart_id*2 + 1;

    l2_wait_dma_finish(buf_id);
}

/**
* @brief return a l2 buffer' current status 
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return 0~7 buffer status
*/
unsigned char l2_get_status(unsigned char buf_id)
{
    unsigned char status;
    
    if(buf_id < L2_COMMON_BUFFER_NUM)
    {
        status =  (unsigned char)((inl(L2_STAT_REG1)>>(buf_id<<2))&0xf);    
    }
    else
    {
        status = (unsigned char)((inl(L2_STAT_REG2)>>((buf_id-8)<<1))&0x3);
    }

    return status;
}


/**
* @brief clear a l2 buffer's status 
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return void
*/
void l2_clr_status(unsigned char buf_id)
{
    unsigned long reg_value;

    irq_mask();

    if (buf_id < L2_COMMON_BUFFER_NUM)
    {
        reg_value = inl(L2_COMBUF_CFG);
        reg_value |= 1<<(buf_id + 24);
        outl(reg_value, L2_COMBUF_CFG);
    }
    else
    {
        reg_value = inl(L2_UARTBUF_CFG);
        reg_value |= (1<<(buf_id + 8));
        outl(reg_value, L2_UARTBUF_CFG);
    }

	irq_unmask();
}

/**
* @brief clear a l2 uart0 rx buffer's status 
* @author yang_mengxia
* @date 2017-04-11
* @param 
* @return void
*/
void l2_clr_uart0_rx_status()
{
    unsigned long reg_value;

    irq_mask();

	reg_value = inl(L2_UARTBUF_CFG);
    reg_value |= (1<<17);
    outl(reg_value, L2_UARTBUF_CFG);

    irq_unmask();
}

/**
* @brief set status for a l2 common buffer 
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: common buffer id
* @param status [in]: status number to be set, 0 <= status <= 8
* @return void
*/
void l2_set_status(unsigned char buf_id, unsigned char status)
{
    unsigned long reg_value; 

    if(buf_id >= 8 || status > 8)
        return;

    irq_mask();
    
    //select the buf and set the buf status
    reg_value = inl(L2_UARTBUF_CFG);
    reg_value &= (~0xff);
    reg_value |= (buf_id | (1<<3) | (status<<4));
    outl(reg_value, L2_UARTBUF_CFG);
        
    //deselect the buf
    reg_value = inl(L2_UARTBUF_CFG);
    reg_value &= (~0xff);
    outl(reg_value, L2_UARTBUF_CFG);    

    irq_unmask();
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__inner_backup_"
#endif
//the following array is used to save inner code
unsigned long inner_backup_data[128]={ /* initialized to prevent optimizing */
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 
};
#ifdef __CC_ARM
#pragma arm section
#endif

extern void jumpto_L2(unsigned long param, unsigned long addr);

/**
* @brief Execute the code in inner ram
* 
* The buffer to be used must be less than 512 bytes
* 
* @author huang_xin
* @date 2009-11-05
* @param table [in]: The table which contain specific codes
* @param len [in]:  size of table 
* @return bool
*/
bool l2_specific_exebuf(T_fL2Exe_CALLBACK code, unsigned long param)
{
    unsigned long i, len;
    unsigned char *location;
    unsigned long *base= inner_backup_data;
    unsigned long buffer_nbr = sizeof(L2_INFO_TABLE)/sizeof(L2_INFO);  

    if ((unsigned long)code < L2_BUF_MEM_BASE_ADDR || (unsigned long)code > (L2_BUF_MEM_BASE_ADDR+512))
    {
        code(param);
        return true;
    }

    len = L2_BUF_MEM_BASE_ADDR + 512 - (unsigned long)code;
    //disable L2 AHB flag
    REG32(L2_FRAC_ADDR) &= ~(1U << AHB_FLAG_EN);

    //disable ARM interrupt
    irq_mask();

    //invalidate cache and disable cache,MMU
    MMU_Clean_Invalidate_Dcache();
    MMU_InvalidateICache();
    MMU_InvalidateTLB();

    MMU_DisableDCache();
    MMU_DisableICache();
    MMU_DisableMMU();

    location = (unsigned char *)((unsigned long)code - 0x48000000);
    location += (unsigned long)base;
    
    for (i=0; i<len; i+=4)
    {
        REG32((unsigned long)code + i) = (*(location+i)) | (*(location+i+1)<<8) | (*(location+i+2)<<16) | (*(location+i+3)<<24);
    }

    for(i=0; i<5000; i++);
    
    //jump to L2 and enter standby
    jumpto_L2(param, (unsigned long)code);

    //enable AHB flag
    REG32(L2_FRAC_ADDR) |= (1U << AHB_FLAG_EN); 

    //enable MMU and cache
    MMU_EnableMMU();
    MMU_EnableDCache();
    MMU_EnableICache();

    //enable ARM interrupt
    irq_unmask();
    return true;
}


