/**
 * @file uart.c
 * @brief UART driver, define UARTs APIs.
 * This file provides UART APIs: UART initialization, write data to UART, read data from
 * UART, register callback function to handle data from UART, and interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010-07-28
 * @version 1.0
 */
#include <string.h>
#include "anyka_cpu.h"
#include "arch_uart.h"
#include "interrupt.h"
#include "sysctl.h"
#include "drv_api.h"
#include "l2.h"
#include "drv_gpio.h"
#include "drv_module.h"
#include "freq.h"

//#define FIQ_UART_TEST

//********************************************************************

#define UART_RX_FIFO_SIZE               (128)    //1//128 bytes


#define UART_TX_END_STA                 (1<<19)

#define UART_RXFIFO_FULL_STA            (1<<1)
#define REG_04_MSK_BIT                  (0x3fe00000)

#define SET_FLAG_OFFSET0                (0x3c)
#define SET_FLAG_OFFSET1                (0x7c)

#define TX_STATUS           1
#define RX_STATUS           0

#define UART_MESSAGE        3

//REG_UART_CONFIG_1
#define TX_ADR_CLR                      (1UL << 31)
#define RX_ADR_CLR                      (1 << 30)
#define RX_STA_CLR                      (1 << 29)
#define TX_STA_CLR                      (1 << 28)
#define ENDIAN_SEL_BIG                  (1 << 27)
#define PARITY_EN                       (1 << 26)
#define PARITY_SEL_EVEN                 (1 << 25)
#define RX_TIMEOUT_EN                   (1 << 23)
#define BAUD_RATE_ADJ_EN                (1 << 22)
#define UART_INTERFACE_EN               (1 << 21)
#define URD_SEL_INVERSELY               (1 << 17)
#define UTD_SEL_INVERSELY               (1 << 16)
#define BAUD_RATE_DIV                   (0)         //[15:0]

//REG_UART_CONFIG_2
#define TX_TH_INT_STA                   (1 << 31)
#define RX_TH_INT_STA                   (1 << 30)
#define TX_TH_INT_EN                    (1 << 29)
#define RX_TH_INT_EN                    (1 << 28)
#define TX_END_INT_EN                   (1 << 27)
#define RX_BUF_FULL_INT_EN              (1 << 26)
#define TX_BUF_EMP_INT_EN               (1 << 24)
#define RX_ERROR_INT_EN                 (1 << 23)
#define TIMEOUT_INT_EN                  (1 << 22)
#define RX_READY_INT_EN                 (1 << 21)
#define TX_END                          (1 << 19)
#define RX_TIMEROUT_STA                 (1 << 18)
#define RX_READY                        (1 << 17)   //?????
#define TX_BYT_CNT_VLD                  (1 << 16)
#define TX_BYT_CNT                      (4)         //[15:4]
#define RX_ERROR                        (1 << 3)
#define RX_TIMEROUT                     (1 << 2)
#define RX_BUF_FULL                     (1 << 1)
#define TX_FIFO_EMPTY                   (1 << 0)
/*
 * @brief: Get Uart's base register address
 */
#define  uart_id2register(uart_id) \
    (UART0_BASE_ADDR+(unsigned long)(uart_id)*0x8000)


static unsigned char  L2_UART_RX[MAX_UART_NUM]={0,0,};  


typedef void (*T_fUART_HISR_HANDLER)(void);

typedef struct {
    unsigned long   baudrate;                   /* baudrate setting */
    T_fUART_CALLBACK callback_func;     /* data receive callback */
    unsigned char    *pReceivePool;              /* receive data pool, assigend by user */
    unsigned long   nReceivePoolLength;         /* the length of data pool */
    unsigned long   nReceivePoolHead;           /* the head pointer of data pool */
    unsigned long   nReceivePoolTail;           /* the tail pointer of data pool */
    
    unsigned long   rxfifo_offset;              /* keep the pointer of L2 rx buffer, 4bytes in unit */
    unsigned long   rxbuf_offset;               /* rx buffer, for timeout data */

    bool  bInterrupt;                 /* indicate interrupt mode */
    bool  bOpen;                      /* indicate uart has be opened */

    bool  bInit;
}T_UART;


static void uart_clock_ctl( T_UART_ID uart_id, bool enable);
static void uart_pin_ctl( T_UART_ID uart_id);
static bool uart0_interrupt_handler(void);
static bool uart1_interrupt_handler(void);
static void uart_handler(T_UART_ID uart_id);
static void uart_storedata(T_UART_ID uart_id, unsigned char *data, unsigned long datalen);
static unsigned long  uart_read_fifo(T_UART_ID uart_id, unsigned char *chr, unsigned long count);
static void uart_callback(unsigned long *param, unsigned long len);
static void ClearStatus(T_UART_ID uart_id, unsigned char txrx_status);
static void ClearTimeout(T_UART_ID uart_id);
static bool uart_write_cpu(T_UART_ID uart_id, unsigned char *chr, unsigned long byte_nbr);

static volatile T_UART m_uart[ MAX_UART_NUM ] = {0};
/**
 * NOTE: 对于AK37,虚拟uiUART0对应物理串口2，uiUART1对应物理串口1
 */
static void uart_clock_ctl( T_UART_ID uart_id, bool enable)
{
    switch(uart_id)
    {
        case uiUART0:
            if (enable)
                sysctl_clock(CLOCK_UART0_ENABLE);
            else
                sysctl_clock(~CLOCK_UART0_ENABLE);
            break;
            
        case uiUART1:
            if (enable)
                sysctl_clock(CLOCK_UART1_ENABLE);
            else
                sysctl_clock(~CLOCK_UART1_ENABLE);
            break;

        default:
            akprintf(C3, M_DRVSYS, "uart_clock_ctl():unknown uart id %d!!\n", uart_id);
            break;
    }
}

static void uart_pin_ctl( T_UART_ID uart_id)
{
    unsigned long pin;

    /* set share pin to UARTn, and disable pull-up */
    switch(uart_id)
    {
        case uiUART0:
            gpio_pin_group_cfg(ePIN_AS_UART0);
            break;
            
        case uiUART1:
            gpio_pin_group_cfg(ePIN_AS_UART1);
            break;
            
        default:
            akprintf(C3, M_DRVSYS, "uart_pin_ctl():unknown uart id %d!!\n", uart_id);
            break;
    }
}


/**
* @BRIEF Clear transmit or receive status of uart controller
* @AUTHOR Pumbaa
* @DATE 2007-08-04
* @PARAM UART_Select uart_id : uart id
* @PARAM unsigned char txrx_status : TX_STATUS: clear transmit status; RX_STATUS: clear receive status
* @RETURN void: 
* @NOTE: ...
*/
static void ClearStatus(T_UART_ID uart_id, unsigned char txrx_status)
{
    unsigned long base_addr;
    unsigned long reg_value;

    base_addr = uart_id2register(uart_id);
    reg_value = inl(base_addr+0x00);
    if (txrx_status)
        reg_value |= (1<<28);
    else
        reg_value |= (1<<29);
    outl(reg_value, (base_addr+0x00));  
}

/**
* @BRIEF clear time out status
* @AUTHOR Pumbaa
* @DATE 2007-08-04
* @PARAM UART_Select uart_id : uart id
* @RETURN void: 
* @NOTE: ...
*/
static void ClearTimeout(T_UART_ID uart_id)
{
    unsigned long reg_value;
    unsigned long base_addr;

    base_addr = uart_id2register(uart_id);
    reg_value = inl(base_addr+UART_CFG_REG2);

    //此处不能进行屏蔽位动作，否则串口不稳定，原因待查
    //reg_value &= 0x3fe00000;  

    reg_value |= (1<<2);
    reg_value &= ~(1<<3);
    outl(reg_value, (base_addr+UART_CFG_REG2));
    
    REG32(base_addr+UART_RX_THREINT) |= (1U << 31);  //set rx_start
}


/**
* @BRIEF set share pin and ppu or ppd of uart pin
* @AUTHOR Pumbaa
* @DATE 2008-03-18
* @PARAM UART_Select uart_id : uart id
* @RETURN void: 
* @NOTE: ...
*/
	
	//add by hezuanxiong
	//share pin register
#define	SHAREPIN_CTRL_REG1	0x08000074
#define	SHAREPIN_CTRL_REG2	0x08000078
	
	//PPU orr PPD register
#define	PPU_PPD_REG1			0x08000090	// cdh:0x0800009c
#define	PPU_PPD_REG2			0x080000a0
#define	PPU_PPD_REG3			0x080000a4
#define	PPU_PPD_REG4			0x080000a8
#define	IO_CTRL_REG1			0x080000d4
#define	IO_CTRL_REG2			0x080000d8
	
	//clock control register
#define	CLK_CTRL_REG1			0x08000004
#define	CLK_N_CFG_REG			0x080000dc
#define	CLK_CTRL_REG2			0x08000008
	
	//clock ctrl and soft reset ctrl rsgister
#define	CLK_RESET_REG			0x0800000c

/**
 * @brief Initialize UART
 * Initialize UART base on UART ID, baudrate and system clock. If user want to change
 * baudrate or system clock is changed, user should call this function to initialize
 * UART again.
 * Function uart_init() must be called before call any other UART functions
 * @author Liao_Zhijun
 * @date 2010-07-23
 * @param T_UART_ID uart_id: UART ID
 * @param unsigned long baud_rate: Baud rate, use UART_BAUD_9600, UART_BAUD_19200 ...
 * @param unsigned long sys_clk: system clock
 * @return bool: Init UART OK or not
 * @retval true: Successfully initialized UART.
 * @retval false: Initializing UART failed.
 */
bool uart_init(T_UART_ID uart_id, unsigned long baud_rate, unsigned long sys_clk)
{

    unsigned long br_value;
    unsigned long reg_value;
    unsigned long base_addr;  
    T_UART_ID uart_idcfg;
    unsigned long uart_clkbit;

    uart_idcfg = uart_id+ePIN_AS_UART0;
    uart_clkbit = (CLOCK_UART0_ENABLE << uart_id);
    
	base_addr = uart_id2register(uart_id);
	br_value = ((sys_clk<<2)+baud_rate)/(baud_rate<<1);

	gpio_pin_group_cfg(uart_idcfg);
	sysctl_clock(uart_clkbit);
	
	if(br_value%2)
		reg_value = ((br_value/2-1)&0xffff)|(1<<21)|(1<<22)|(1<<23)|(1<<28)|(1<<29);
	else
		reg_value = ((br_value/2-1)&0xffff)|(1<<21)|(1<<23)|(1<<28)|(1<<29);
	if (uiUART1 == uart_id)
	{
		reg_value |= (3<<18);  // 硬件流控，使用232芯片时，bit18 and bit19 置0 ; 直接使用TTL电平，不经过232  ,bit18 and bit19 置1
	}
	
	//if do not use hardware flow control
	outl(reg_value, base_addr+0x00);
	
	reg_value = 0;

	reg_value = ( 1 << 23 | 1 << 22 | 1 << 21);// 1 << 28 
	outl(reg_value, base_addr+0x04);	//mask all interrupt

	REG32(base_addr + 0x18) &= ~(0x1<<11);    //?     

    //init global variable
    if (m_uart[ uart_id ].bOpen == false)
    {
        m_uart[uart_id].callback_func = NULL;        
    }
    m_uart[ uart_id ].baudrate = baud_rate;
    m_uart[ uart_id ].bOpen  = true;
    m_uart[ uart_id ].bInterrupt  = false;

    m_uart[uart_id].pReceivePool = NULL;
    m_uart[uart_id].nReceivePoolLength = 0;
    m_uart[uart_id].nReceivePoolHead = 0;
    m_uart[uart_id].nReceivePoolTail = 0;
    m_uart[uart_id].rxfifo_offset = 0;

    //set rxbuf_offset equal byt_left
    m_uart[uart_id].rxbuf_offset = (REG32(base_addr+UART_DATA_CFG)>>23)&0x3;  
    
    baud_rate = (0 == baud_rate) ? 115200 : baud_rate;

    m_uart[uart_id].bInit = false;

//just for test
#ifdef FIQ_UART_TEST
    {
        unsigned long baseAddress, value;
        baseAddress = uart_id2register( uart_id );
        
        /* set threshold to 4bytes */
        value = REG32(baseAddress+UART_DATA_CFG);
        value &= ~(0x7F<<25);
        REG32(baseAddress+UART_DATA_CFG) = value;
    
        value = REG32(baseAddress+UART_RX_THREINT);
        value &= ~0x1f;
        value |= 0x3; //      
        REG32(baseAddress+UART_RX_THREINT) = value;
    
        ClearStatus(uart_id, RX_STATUS);
        l2_clr_status((unsigned char)uart_id*2+7);
    
        /* clear count */
        value = REG32(baseAddress+UART_RX_THREINT);
        value |= (1<<5);
        REG32(baseAddress+UART_RX_THREINT) = value;
       
        /* enable timeout, r_err, rx buffer full and rx_th interrupt */
        REG32(baseAddress+UART_CFG_REG2) |= (1<<28) | (1<<22); //????????

        int_register_fiq(INT_VECTOR_UART1, uart0_interrupt_handler);
    }
#else
    //Create task
    if(!DrvModule_Create_Task(DRV_MODULE_UART0 + uart_id))
    {
        return false;
    }
#endif

    return true;
}

/**
 * @brief change UART setting according to system clock change
 *
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] sys_clk system clock
 * @return void
 */
void uart_on_change( unsigned long sys_clk )
{
    T_UART_ID i;
    unsigned long baseAddress, br_value, reg_value;

    for( i=uiUART0; i<MAX_UART_NUM; i++ )
    {
        if(m_uart[i].bOpen)
        {
            baseAddress = uart_id2register(i);      

            br_value = sys_clk / m_uart[i].baudrate-1;

            reg_value = REG32(baseAddress+UART_CFG_REG1);
            reg_value &= ~(0xffff);
            reg_value &= ~(1<<22);

            reg_value |= (br_value & 0xffff);
            if(sys_clk % m_uart[i].baudrate)
                reg_value |= (1<<22);

            REG32(baseAddress+UART_CFG_REG1) = reg_value;
        }
    }
}
static bool uart_write_cpu(T_UART_ID uart_id, unsigned char *chr, unsigned long byte_nbr)
{
	unsigned long baseAddress;
    unsigned long status;
    unsigned long reg_value;
    unsigned char  buf_id = 8+(uart_id<<1);
    unsigned long buf_addr, i,cnt=0;
    unsigned long value1,value2;
    
    if(m_uart[ uart_id ].bOpen == false)
        return false;

    if (byte_nbr >= 64)
    {
        akprintf(C3, M_DRVSYS, "uart_write_buf error byte_nbr >= 64");
        return false;
    }

    //get base address
    baseAddress = uart_id2register(uart_id);

    //clear tx status
    ClearStatus(uart_id, TX_STATUS); //clear tx status

    //load data to l2
    l2_clr_status(buf_id);
    l2_uartbuf_cpu((unsigned long)chr, uart_id, byte_nbr, MEM2BUF);

    //start to trans
    reg_value = REG32(baseAddress+UART_CFG_REG2);
    reg_value &= 0x3fe00000;
    reg_value |= (byte_nbr<<4)|(1<<16); 
    REG32(baseAddress + UART_CFG_REG2) = reg_value;

    //wait for tx end
    //uart_wait(uart_id);

    return true;
}

static void uart_wait(T_UART_ID uart_id)
{
    unsigned long status;
	unsigned long baseAddress;
	
    //get base address
    baseAddress = uart_id2register(uart_id);

    //wait for tx end
    while (1)
    {
        status = inl(baseAddress + UART_CFG_REG2);
        
        if (status & UART_TX_END_STA)
            break;
    }
}

/**
 * @brief start uart send and return, donot wait for complete
 * @author
 * @date
 * @param uart_id: which uart to send
 * @param data: data buffer
 * @param data_len: data length
 * @return bool
 */
bool uart_send(T_UART_ID uart_id, const unsigned char *data, unsigned long data_len)
{
    unsigned long status;
	unsigned long baseAddress;
	
    //get base address
    baseAddress = uart_id2register(uart_id);
    status = inl(baseAddress + UART_CFG_REG2);

    if(!(status & UART_TX_END_STA))
        return false;

    return uart_write_cpu(uart_id, (unsigned char *)data, data_len);
}

unsigned long uart_write_dma(T_UART_ID uart_id, const unsigned char *chr, unsigned long byte_nbr)
{
    unsigned long reg_value;
    unsigned long status;
    unsigned long base_addr, set_flag_addr;
    unsigned long tran_2k_nbr;
    unsigned long remain_nbr;
    unsigned long tran_64_nbr;
    unsigned long frac_nbr;
    unsigned long i;
    unsigned char buf_id;
    unsigned char buf_offset;
    unsigned long ram_addr = (unsigned long)chr;
    
    base_addr = uart_id2register(uart_id);
    buf_id = 8+uart_id*2;
    tran_2k_nbr = byte_nbr>>11;
    remain_nbr = byte_nbr-(tran_2k_nbr<<11);
    tran_64_nbr = (byte_nbr-(tran_2k_nbr<<11))>>6;
    frac_nbr = byte_nbr%64;

    if (tran_64_nbr%2 == 0)
    {
        set_flag_addr = l2_get_addr(buf_id) + SET_FLAG_OFFSET0;
        buf_offset = 0;
    }
    else
    {
        set_flag_addr = l2_get_addr(buf_id) + SET_FLAG_OFFSET1;
        buf_offset = 1;
    }

    //clear status
    l2_clr_status(buf_id);
    ClearStatus(uart_id, TX_STATUS);

    for (i=0; i<tran_2k_nbr; i++)
    {
        reg_value = inl(base_addr+0x04);
        reg_value &= REG_04_MSK_BIT;
        reg_value |= (2048<<4)|(1<<16);
        outl(reg_value, base_addr+0x04);

        l2_uartbuf_dma(ram_addr+(i<<11), uart_id, 2048, MEM2BUF);
        l2_uartbuf_wait_dma_finish(uart_id, MEM2BUF);
    }

    if (tran_64_nbr || frac_nbr)
    {
        reg_value = inl(base_addr+0x04);
        reg_value &= REG_04_MSK_BIT;
        reg_value |= (remain_nbr<<4)|(1<<16);
        outl(reg_value, base_addr+0x04);    
    }

    l2_uartbuf_dma(ram_addr+(tran_2k_nbr<<11), uart_id, remain_nbr, MEM2BUF);
    l2_uartbuf_wait_dma_finish(uart_id, MEM2BUF);

    //manually change status if the remain bytes <=60
    if(frac_nbr <= 60)
    {
        WriteBuf(0, set_flag_addr);
    }

    //wait for tx end
    while (1)
    {
        status = inl(base_addr+0x04);
        if (status & UART_TX_END_STA)
            break;
    }

    return byte_nbr;
}

/**
 * @brief Write one character to UART base on UART ID
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-23
 * @param T_UART_ID uart_id: UART ID
 * @param unsigned char chr: The character which will be written to UART
 * @return bool: Write character OK or not
 * @retval true: Successfully written character to UART.
 * @retval false: Writing character to UART failed.
 */
bool uart_write_chr(T_UART_ID uart_id, unsigned char chr)
{
    if(uart_write_cpu(uart_id, &chr, 1))
    {
        uart_wait(uart_id);
        return true;
    }
    else
        return false;
}

/**
 * @brief Write string to UART base on UART ID
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-17
 * @param T_UART_ID uart_id: UART ID
 * @param unsigned char *str: The string which will be written to UART
 * @return unsigned long: Length of the data which have been written to UART
 * @retval
 */
unsigned long uart_write_str(T_UART_ID uart_id, unsigned char *str)
{
    unsigned long   written_num = 0;

    if(m_uart[ uart_id ].bOpen == false)
        return 0;

    while (*str != '\0')
    {
        uart_write_chr(uart_id, *str);
        written_num++;
        str++;
    }

    return written_num;
}

/**
 * @brief Write string data to UART
 * Write data to UART base on UART ID and data length
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-16
 * @param T_UART_ID uart_id: UART ID
 * @param const unsigned char * data: Constant data to be written to UART, this data needn't be end with '\0'
 * @param unsigned long data_len: Data length
 * @return unsigned long: Length of the data which have been written to UART
 * @retval
 */
unsigned long uart_write(T_UART_ID uart_id, const unsigned char *data, unsigned long data_len)
{
    unsigned long   written_num = 0, write_cnt=60;

    if(m_uart[ uart_id ].bOpen == false)
        return 0;

    //  目前改为除uart 0外，其他串口都使用dma发送
    //if (uiUART0 != uart_id)
    //NOTE: 2011.06.09 by xc
    // 为解决音频播放papa音问题，需要将所有的L2访问改为cpu方式
    if (0)
    {
        while (data_len>0)
        {
            written_num = uart_write_dma(uart_id, data, data_len);
            data_len -= written_num;
        }
    }
    else
    {
        while (data_len>0)
        {
            if (data_len < write_cnt)
                write_cnt = data_len;
                
            uart_write_cpu(uart_id, (unsigned char*)data, write_cnt);
            uart_wait(uart_id);
            
            data_len -= write_cnt;
            data += write_cnt;
            written_num += write_cnt;
        }
    }
    
    return written_num;
}

unsigned long uart_read(T_UART_ID uart_id, unsigned char *data, unsigned long datalen)
{
    unsigned long i = 0;

    if(m_uart[ uart_id ].bOpen == false)
        return 0;

    if (m_uart[uart_id].bInterrupt == true)
    {
        if (m_uart[uart_id].nReceivePoolLength != 0) //has datapool
        {
            for(i = 0; i < datalen; i++)
            {
                if(m_uart[uart_id].nReceivePoolTail != m_uart[uart_id].nReceivePoolHead)
                {
                    data[i] = m_uart[uart_id].pReceivePool[m_uart[uart_id].nReceivePoolHead++];
                    m_uart[uart_id].nReceivePoolHead &= (m_uart[uart_id].nReceivePoolLength - 1);
                }
                else
                    break;
            }
        }
    }
    else
    {
#if 1
            i = uart_read_fifo(uart_id, data, datalen); 
#else
        unsigned long len = datalen&3;
        unsigned long len4;
        len4 = datalen - len;
        if (len4) //read 4byte align
        {
            i += uart_read_fifo(uart_id, data, len4);
            data += i;
        }
        if (len) //read less than 4byte
        {
            while(i<datalen)
            {
                uart_read_chr(uart_id, data);
                i++;
                data++;
            }
        }
#endif
        
    }
    
    return i;
}

static void idle_hook(void)
{	
    //sleep(10);	
    #ifndef BURNTOOL
    AK_Sleep(2);
    #endif
}

/**
 * @brief Read a character from UART
 * This function will not return until get a character from UART
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-17
 * @param T_UART_ID uart_id: UART ID
 * @param unsigned char *chr: character for return
 * @return bool: Got character or not
 * @retval
 */
bool uart_read_chr(T_UART_ID uart_id, unsigned char *chr)
{
    unsigned long reg_value;
    unsigned long baseAddress, bufferAddress;   
    unsigned long remain;
    unsigned long buf_addr, buf_end_addr;

    if(uart_id > MAX_UART_NUM)
        return false;
    if(m_uart[ uart_id ].bInterrupt  == false)
    {
        baseAddress = uart_id2register( uart_id );
        buf_addr = l2_get_addr(L2_UART_RX[uart_id]);

        REG32(baseAddress + UART_CFG_REG1) |= RX_TIMEOUT_EN;    

        while(!(REG32(baseAddress + UART_CFG_REG2)&RX_TIMEROUT))
        {
			idle_hook();
	 	}

        REG32(baseAddress+UART_RX_THREINT) |= (1U << 31);  //sjj: bit 31 means to start to receive data
        REG32(baseAddress+UART_CFG_REG2) |= RX_TIMEROUT;

        remain = (REG32(baseAddress+UART_DATA_CFG)>>13) & 0x7f;


        if(0 == remain)
        {
            remain = UART_RX_FIFO_SIZE;//RX L2 buffer max is 128 BYTE
        }
        
        buf_end_addr =  remain + buf_addr - 0x1;


        *chr = *(volatile unsigned char*)buf_end_addr;

        REG32(baseAddress+UART_CFG_REG1) &= ~RX_TIMEOUT_EN;
                   
        return true;
    }
    else if(m_uart[ uart_id ].bInterrupt  == true)
    {
        baseAddress = uart_id2register( uart_id );
        buf_addr = l2_get_addr(L2_UART_RX[uart_id]);
        
        REG32(baseAddress + UART_CFG_REG1) |= RX_TIMEOUT_EN;    

        while(!(REG32(baseAddress + UART_CFG_REG2)&RX_TIMEROUT))
        {
        	idle_hook();
        }

        REG32(baseAddress+UART_RX_THREINT) |= (1U << 31);  //sjj: bit 31 means to start to receive data
        REG32(baseAddress+UART_CFG_REG2) |= RX_TIMEROUT;

        remain = (REG32(baseAddress+UART_DATA_CFG)>>13) & 0x7f;

        if(0 == remain)
        {
	        remain = 128;//RX L2 buffer max is 128 BYTE
        }
        buf_end_addr =  remain + buf_addr - 0x1;


        *chr = *(volatile unsigned char*)buf_end_addr;

        REG32(baseAddress+UART_CFG_REG1) &= ~RX_TIMEOUT_EN;
                   
        return true;
    }

	return false;
    
}


static void uart_l2_cpy(unsigned char *data, unsigned long base_addr, unsigned long offset, unsigned long len)
{
    unsigned long offset_adj = offset & 0xfffffffc;
    unsigned long tmp, i;
    unsigned long cnt = 0;

    i = offset - offset_adj;
    tmp = REG32(base_addr + offset_adj);
    
    while(cnt < len)
    {
        data[cnt++] = (tmp >> (i*8)) & 0xff;

        i = (i+1)&0x3;
        if(i == 0)
        {
            offset_adj += 4;
            tmp = REG32(base_addr + offset_adj);
        }
    }
}

/**
 * @brief Read a character from UART asynchronously.
 * If no data, this function will also return directly.
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-17
 * @param T_UART_ID uart_id: UART ID
 * @param unsigned char *chr: character for return
 * @return bool: Got character or not
 * @retval
 */
static unsigned long uart_read_fifo(T_UART_ID uart_id, unsigned char *chr, unsigned long count)
{
    unsigned long baseAddress;  
    unsigned long nbr_to_read=0,read_cnt=0;
    unsigned long bufferAddress, rxfifo_offset=0;
    baseAddress = uart_id2register( uart_id );


    bufferAddress = l2_get_addr(L2_UART_RX[uart_id]);

    while(REG32(baseAddress+UART_CFG_REG2) & RX_READY)
    {        
        rxfifo_offset = m_uart[uart_id].rxfifo_offset;

        if (0 == rxfifo_offset)
        {
            uart_l2_cpy(chr, bufferAddress, 0, UART_RX_FIFO_SIZE);
        }
        else
        {
            read_cnt = UART_RX_FIFO_SIZE - rxfifo_offset;
            uart_l2_cpy(chr, bufferAddress, rxfifo_offset, read_cnt);
            chr += read_cnt;
            uart_l2_cpy(chr, bufferAddress, 0, rxfifo_offset);
        }
        
        return   UART_RX_FIFO_SIZE; 
        
    }
   
    nbr_to_read = (REG32(baseAddress+UART_DATA_CFG)>>13)&0x7f;


    /* check fifo empty or not */
    if (nbr_to_read != m_uart[uart_id].rxfifo_offset)
    {

        rxfifo_offset = m_uart[uart_id].rxfifo_offset;
        /* copy data */
        if (nbr_to_read > rxfifo_offset)
        {
            
            read_cnt = nbr_to_read - rxfifo_offset;
            if (read_cnt > count)
                read_cnt = count;
            uart_l2_cpy(chr, bufferAddress, rxfifo_offset, read_cnt);
        }
        else
        {
            read_cnt = (UART_RX_FIFO_SIZE - rxfifo_offset);
            
            if (count <= read_cnt)
            {
                uart_l2_cpy(chr, bufferAddress, rxfifo_offset, count); 
                read_cnt = count;
            }
            else
            {
                uart_l2_cpy(chr, bufferAddress, rxfifo_offset, read_cnt); 
                if (count-read_cnt < nbr_to_read)
                    nbr_to_read = count - read_cnt;
                uart_l2_cpy(chr+read_cnt, bufferAddress, 0, nbr_to_read);
                read_cnt += nbr_to_read;
            }

        }
        
        m_uart[uart_id].rxfifo_offset = (m_uart[uart_id].rxfifo_offset + read_cnt)%UART_RX_FIFO_SIZE;
        
    }

    return read_cnt;
}

void uart_set_datapool(T_UART_ID uart_id, unsigned char *pool, unsigned long poollength)
{
    unsigned long i = 0;
    
    if(pool == NULL || poollength == 0)
    {
        akprintf(C3, M_DRVSYS, "pool can't be null\n");
        return;
    }

    if(m_uart[ uart_id ].bOpen == false)
        return;
    
    m_uart[uart_id].pReceivePool = pool;
    m_uart[uart_id].nReceivePoolLength = poollength;

    while(1)
    {
        if ((m_uart[uart_id].nReceivePoolLength & (0x80000000 >> i )) != 0)
        {
           m_uart[uart_id].nReceivePoolLength = m_uart[uart_id].nReceivePoolLength & (0x80000000 >> i );
           akprintf(C3, M_DRVSYS, "set uart%d pool size %x\n", uart_id, m_uart[uart_id].nReceivePoolLength);
           break;
        }
        
        i++;
    }
}

static void uart_callback(unsigned long *param, unsigned long len)
{
    unsigned long uart_id;

    if(NULL == param)
        return;
        
    uart_id = param[0];

    if(uart_id < MAX_UART_NUM && m_uart[uart_id].callback_func)
    {
        m_uart[uart_id].callback_func();
    }
}

/**
 * @brief Register a callback function to process UART received data.
 * This function words only in the UART interrupt mode.
 * Caution: The macro definition "__ENABLE_UARTxx_INT__" must be defined. 
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-17
 * @param T_UART_ID uart_id: UART ID
 * @param T_fUART_CALLBACK callback_func: Callback function
 * @return void
 * @retval
 */
void uart_set_callback(T_UART_ID uart_id, T_fUART_CALLBACK callback_func)
{
    unsigned long baseAddress;
    unsigned long uart_status,value;
    unsigned char cur_chr;
    
    L2_UART_RX[uart_id] = (2*uart_id +1)+8;

#ifdef FIQ_UART_TEST
    return;
#endif

    if(uart_id > MAX_UART_NUM)
        return ;

    if(m_uart[ uart_id ].bOpen == false)
        return;
    baseAddress = uart_id2register( uart_id );

    /* disable uart interrupt callback */
    if (callback_func == NULL)
    {
        m_uart[uart_id].bInterrupt = false;
        m_uart[uart_id].rxfifo_offset = 0;
        HAL_WRITE_UINT32(baseAddress+UART_CFG_REG2, 0);//mask all interrupt
        ClearStatus(uart_id, RX_STATUS);      
        l2_clr_status(L2_UART_RX[uart_id]);       
        return;
    }

    m_uart[uart_id].callback_func = callback_func;
    m_uart[uart_id].bInterrupt  = true;

    /* set threshold to 64bytes or 256B(uart2) */
    value = REG32(baseAddress+UART_DATA_CFG);
    value &= ~(0x7FU << 25);
    value |= 0x1 << 25; 
    REG32(baseAddress+UART_DATA_CFG) = value;    

    value = REG32(baseAddress+UART_RX_THREINT);
    value &= ~0x1f;
    value |= 0x1f;       
    REG32(baseAddress+UART_RX_THREINT) = value;

    ClearStatus(uart_id, RX_STATUS);
    
    l2_clr_status(L2_UART_RX[uart_id]);

    /* clear count */
    value = REG32(baseAddress+UART_RX_THREINT);
    value &= ~(1<<5);
    REG32(baseAddress+UART_RX_THREINT) = value;
   
    /* enable timeout, r_err, rx buffer full and rx_th interrupt */
    REG32(baseAddress+UART_CFG_REG2) |= (1<<28) | (1<<22) | (1<<23); //????????
        
    if (!m_uart[uart_id].bInit)
    {
        DrvModule_Map_Message(DRV_MODULE_UART0 + uart_id, UART_MESSAGE+uart_id, uart_callback);

        m_uart[uart_id].bInit = true;
    }

    if (uart_id == uiUART0)
        int_register_irq(INT_VECTOR_UART1, uart0_interrupt_handler);
    else if (uart_id == uiUART1)
        int_register_irq(INT_VECTOR_UART2, uart1_interrupt_handler);

    return;
}

static void uart_storedata(T_UART_ID uart_id, unsigned char *data, unsigned long datalen)
{
    if (m_uart[uart_id].pReceivePool != NULL)
    {
        if(m_uart[uart_id].nReceivePoolTail + datalen > m_uart[uart_id].nReceivePoolLength)
        {
            unsigned long cpylen = m_uart[uart_id].nReceivePoolLength - m_uart[uart_id].nReceivePoolTail;
          
            memcpy(&m_uart[uart_id].pReceivePool[m_uart[uart_id].nReceivePoolTail], data, cpylen);
            memcpy(&m_uart[uart_id].pReceivePool[0], data + cpylen, datalen - cpylen);
            m_uart[uart_id].nReceivePoolTail = datalen - cpylen;
        }
        else
        {
            memcpy(&m_uart[uart_id].pReceivePool[m_uart[uart_id].nReceivePoolTail], data, datalen);
            m_uart[uart_id].nReceivePoolTail += datalen;
        }

        m_uart[uart_id].nReceivePoolTail &= ( m_uart[uart_id].nReceivePoolLength - 1 );
    }
}
static unsigned long uart_rx_get_data(T_UART_ID uart_id, unsigned long status)
{
    unsigned long rxcount=0,i;
    unsigned long data[128];
  
    //read L2 data
    rxcount = uart_read_fifo(uart_id, (unsigned char *)data, UART_RX_FIFO_SIZE);    
//    printf("R:%d\n",rxcount);
    if (rxcount != 0)
    {
        uart_storedata(uart_id, (unsigned char *)data, rxcount);
        
        data[0] = uart_id;
        DrvModule_Send_Message(DRV_MODULE_UART0 + uart_id, UART_MESSAGE+uart_id, data);
    }
}
/**
 * @brief UART interrupt handler
 * If chip detect that UART0 received data, this function will be called.
 * This function will get UART data from UART Receive Data Hold Register, and call
 * UART callback function to process the data if it is available.
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-16
 * @param void
 * @return void
 * @retval
 */
static void uart_handler(T_UART_ID uart_id)
{ 
    unsigned long baseAddress, status, reg;  
 
    baseAddress = uart_id2register( uart_id);

    status = REG32(baseAddress+UART_CFG_REG2);

    //printf("[uart int: %x]", status);
    
    //clear rx_err
    if (status & RX_ERROR)
    {
        //printf("E\n");
        reg = REG32(baseAddress+UART_CFG_REG2);
        reg |= RX_ERROR; 
        REG32(baseAddress+UART_CFG_REG2) = reg; 
    }
    
    //rx threshold interrupt
    if (status & RX_TH_INT_STA)
    {
        //printf("S\n");
        //clear rx th status
        reg = REG32(baseAddress+UART_CFG_REG2);
        reg |= RX_TH_INT_STA; 
        REG32(baseAddress+UART_CFG_REG2) = reg; 
    }     

    //rx threshold interrupt
    if (status & RX_TIMEROUT)
    {
        //printf("T\n");
        ClearTimeout(uart_id);
    }     

//just for test
#ifdef FIQ_UART_TEST
{
    extern unsigned long pc_lr;

    l2_clr_status((unsigned char)uart_id*2+7);
    akprintf(C1, M_DRVSYS, "[pc: %x]\n", pc_lr);
}
#else
    uart_rx_get_data(uart_id, status);
#endif

    return;
}

static bool uart0_interrupt_handler(void)
{
    uart_handler(uiUART0);
    return true;
}

static bool uart1_interrupt_handler(void)
{
    uart_handler(uiUART1);
    return true;
}


/**
 * @brief Close UART
 * Function uart_init() must be called before call this function
 * @author Junhua Zhao
 * @date 2005-05-18
 * @param T_UART_ID uart_id: UART ID
 * @return void
 * @retval
 */
void uart_free(T_UART_ID uart_id)
{
    unsigned long baseAddress;
    unsigned long i = 0;

    baseAddress = uart_id2register( uart_id );

    /* disable uart interrupt */
    HAL_WRITE_UINT32(baseAddress+UART_CFG_REG2, 0);//mask all interrupt

    m_uart[ uart_id ].bOpen = false;

    uart_clock_ctl(uart_id, false);

    if(m_uart[uart_id].bInit)
    {
        m_uart[uart_id].bInit = false;
    }
#if 0 //kjp 2017-6-16  
    for(i = 0; i < MAX_UART_NUM; i++)
    {
        if(m_uart[i].bInit)
            break;
    }

    if(i == MAX_UART_NUM)
#endif    
    {
        DrvModule_Terminate_Task(DRV_MODULE_UART0 + uart_id);
    }
}

bool uart_setflowcontrol(T_UART_ID uart_id, bool enable)
{
    unsigned long   baseAddress = 0;

    if(uart_id == uiUART0)
        return false;

    if(m_uart[ uart_id ].bOpen == false)
        return false;

    baseAddress = uart_id2register( uart_id );

    if(true == enable)
    {
        REG32(baseAddress+UART_CFG_REG1) |= (1<<19);
    }
    else
    {
        REG32(baseAddress+UART_CFG_REG1) &= ~(1<<19);
    }

    return true;
}

bool uart_setdataparity(T_UART_ID uart_id, bool enable, bool evenParity)
{
    unsigned long   baseAddress = 0;
    unsigned long   reg_value;

    if(m_uart[ uart_id ].bOpen == false)
        return false;

    baseAddress = uart_id2register( uart_id );

    reg_value = REG32(baseAddress+UART_CFG_REG1);
    if (enable == true)
    {
        reg_value |= (1<<26);
        if (evenParity == true)  //even parity
            reg_value |= (1<<25);
        else
            reg_value &= ~(1<<25);  //odd parity
    }
    else
    {
        //no parity
        reg_value &= ~(1<<26);
    }

    REG32(baseAddress+UART_CFG_REG1) = reg_value;
    return true;
}

bool uart_setbaudrate(T_UART_ID uart_id, unsigned long baud_rate)
{
    unsigned long   baseAddress = 0;
    unsigned long   reg_value, br_value;

    if(m_uart[ uart_id ].bOpen == false)
        return false;

    baseAddress = uart_id2register( uart_id );
    br_value = get_asic_freq()/baud_rate -1;

    reg_value = REG32(baseAddress+UART_CFG_REG1);
    reg_value &= ~(0xffff);
    reg_value &= ~(1<<22);

    if (br_value > 0x10000)
    {
        akprintf(C3, M_DRVSYS, "baudrate out of range %d\n", baud_rate);
    }
    reg_value |= (br_value & 0xffff);
    if(get_asic_freq() % baud_rate)
        reg_value |= (1<<22);
    
    REG32(baseAddress+UART_CFG_REG1) = reg_value;      
    m_uart[uart_id].baudrate = baud_rate;
    return true;
}

void uart_clr_l2_buf(T_UART_ID uart_id)
{
	if( uart_id == uiUART0 )
		l2_clr_uart0_rx_status();
}

/**
 * @brief clear uart datapool
 * @author liao_zhijun
 * @date 2017-4-24
 * @param[in] uart_id UART ID
 * @return void
 */
void uart_clear_datapool(T_UART_ID uart_id)
{
    if(m_uart[uart_id].pReceivePool == NULL)
        return;

    m_uart[uart_id].nReceivePoolHead = m_uart[uart_id].nReceivePoolTail;
}

static int uart_reg(void)
{
    freq_register(E_UART_CALLBACK, uart_on_change);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(uart_reg)
#ifdef __CC_ARM
#pragma arm section
#endif


