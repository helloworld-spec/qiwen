/**@file arch_sd.c
 * @brief Implement arch level operations of how to control sd&sdio.
 *
 * This file implement sd&sdio controller driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "l2.h"
#include "sysctl.h"
#include "sdio.h"
#include "hal_sd.h"
#include "hal_sdio.h"
#include "hal_common_sdio.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "freq.h"
#include "drv_module.h"

static unsigned long s_SdRegClkCtrl    = SD_CLK_CTRL_REG;          // (clock) control reg
static unsigned long s_SdRegArgument   = SD_ARGUMENT_REG;          // command arg reg
static unsigned long s_SdRegCmd        = SD_CMD_REG;               // command control reg
static unsigned long s_SdRegRespCmd    = SD_RESP_CMD_REG;          // command response reg
static unsigned long s_SdRegResp0      = SD_RESP_REG0;             // response reg 0
static unsigned long s_SdRegResp1      = SD_RESP_REG1;             // response reg 1
static unsigned long s_SdRegResp2      = SD_RESP_REG2;             // response reg 2
static unsigned long s_SdRegResp3      = SD_RESP_REG3;             // response reg 3
static unsigned long s_SdRegDataTim    = SD_DATA_TIM_REG;          // data timer reg
static unsigned long s_SdRegDataLen    = SD_DATA_LEN_REG;          // data length reg
static unsigned long s_SdRegDataCtrl   = SD_DATA_CTRL_REG;         // data control reg
static unsigned long s_SdRegDataCount  = SD_DATA_COUT_REG;         // data remain counter reg
static unsigned long s_SdRegStatus     = SD_INT_STAT_REG;          // command/data status reg
static unsigned long s_SdRegIntEnable  = SD_INT_ENABLE;            // interrupt enable reg
static unsigned long s_SdRegDmaMode    = SD_DMA_MODE_REG;          // data buffer configure reg(0x2002x03C), CPU/DMA mode select
static unsigned long s_SdRegCpuMode    = SD_CPU_MODE_REG;          // data reg(0x2002x040)

static unsigned long s_sdio_clok       = 0;

static T_eCARD_INTERFACE            s_SdInterface = INTERFACE_SDMMC4;
static DEVICE_SELECT                s_L2Select = ADDR_MMC_SD;
static E_GPIO_PIN_SHARE_CONFIG      s_PinSelect = ePIN_AS_MMCSD;

static void sdio_set_data_reg(unsigned long len,unsigned long blk_size,unsigned char bus_mode,unsigned char dir);
static bool sdio_rx_data_cpu(unsigned char buf[], unsigned long len);
static bool sdio_tx_data_cpu(unsigned char buf[], unsigned long len);
static void set_sdio_if(void);
static bool sdio_abort_trans(unsigned char func_nbr);

static void set_sdio_if(void)
{
    s_SdRegClkCtrl      = SDIO_CLK_CTRL_REG;
    s_SdRegArgument     = SDIO_ARGUMENT_REG;
    s_SdRegCmd          = SDIO_CMD_REG;
    s_SdRegRespCmd      = SDIO_RESP_CMD_REG;
    s_SdRegResp0        = SDIO_RESP_REG0;
    s_SdRegResp1        = SDIO_RESP_REG1;
    s_SdRegResp2        = SDIO_RESP_REG2;
    s_SdRegResp3        = SDIO_RESP_REG3;
    s_SdRegDataTim      = SDIO_DATA_TIM_REG;
    s_SdRegDataLen      = SDIO_DATA_LEN_REG;
    s_SdRegDataCtrl     = SDIO_DATA_CTRL_REG;
    s_SdRegDataCount    = SDIO_DATA_COUT_REG;
    s_SdRegStatus       = SDIO_INT_STAT_REG;
    s_SdRegIntEnable    = SDIO_INT_ENABLE;
    s_SdRegDmaMode      = SDIO_DMA_MODE_REG;
    s_SdRegCpuMode      = SDIO_CPU_MODE_REG;
    
    s_L2Select  = ADDR_SDIO;
    s_PinSelect = ePIN_AS_SDIO;    
}




/**
 * @brief Set the sd interface.
 *
 * Select the sd interface(INTERFACE_SDMMC4,INTERFACE_SDMMC8 or INTERFACE_SDIO)and select the relevant registers,L2 ,pin.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cif[in] The selected interface,INTERFACE_SDMMC4,INTERFACE_SDMMC8 or INTERFACE_SDIO.
 * @return void
 */
void sdio_set_interface(T_eCARD_INTERFACE cif)
{
    if(INTERFACE_NOT_SD != cif)
    {
        if(INTERFACE_SDIO == cif)
        {
            set_sdio_if(); 
        }
        else 
        {
            //set_sdmmc_if();
            akprintf(C3, M_DRVSYS, "sdio_set_interface ERR, type=%d\n",cif);
            
        }
        
        s_SdInterface = cif;
        gpio_pin_group_cfg(s_PinSelect);            
    }
}

/**
 * @brief Set share pin  for the selected interface
 *
 * Config the share pin for selected interface and set the attributes of the share pin,sush as pull up,IO control. 
 * @author Huang Xin
 * @date 2010-07-14
 * @param cif[in] The selected interface,INTERFACE_SDMMC4,INTERFACE_SDMMC8 or INTERFACE_SDIO.
 * @return void
 */
 #if 0
void set_pin(T_eCARD_INTERFACE cif)
{

    gpio_pin_group_cfg(s_PinSelect);
}
#endif

/**
 * @brief Set sd card clock.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @param clk[in] The main clock of sd card.
 * @param asic[in] current asic freq
 * @param pwr_save[in] Set this parameter true to enable power save
 * @return void
 */
void sdio_set_clock(unsigned long clk, unsigned long asic, bool pwr_save)
{
    unsigned char clk_div_l,clk_div_h;
    unsigned long asic_freq,reg_value,tmp;

    if(0 == clk)
        return;
        
    asic_freq = asic;
    if (asic_freq < clk*2)
    {
        clk_div_l = clk_div_h = 0;
    }
    else
    {
        // clk = asic / ((clk_div_h+1) + (clk_div_l+1))
        //NOTE:clk_div_h and clk_div_l present high and low level cycle time
        tmp = asic_freq / clk;
        if (asic_freq % clk)
            tmp += 1;
        tmp -= 2;
        clk_div_h = tmp/2;
        clk_div_l = tmp - clk_div_h;
    }
    reg_value = (clk_div_l<<CLK_DIV_L_OFFSET)|(clk_div_h<<CLK_DIV_H_OFFSET) | SD_CLK_ENABLE | FALLING_TRIGGER | SD_INTERFACE_ENABLE;
    if (pwr_save)
        reg_value |= PWR_SAVE_ENABLE;

    REG32(s_SdRegClkCtrl) = reg_value;

    //save clock
    tmp = asic_freq/(clk_div_h+clk_div_l+2);
    if(ePIN_AS_SDIO == s_PinSelect)
    {
        s_sdio_clok = clk;
        akprintf(C3, M_DRVSYS, "asic clk = %d, sdio clk = %d\n",asic_freq, tmp);
    }

}

void sdio_cfg_buf(unsigned char buf_mode)
{
    unsigned long reg_value=0;
    if (buf_mode == L2_DMA_MODE)
    {
       reg_value = BUF_EN|DMA_EN|(128<<BUF_SIZE_OFFSET);
    }
	else if(buf_mode == L2_CPU_MODE)   //ZOUTX  FIX THIS BUG  INNER BUFFER 和L2 buffer 根本不是一回事
	{
	   reg_value = BUF_EN|(128<<BUF_SIZE_OFFSET);
	}
    else if(buf_mode == INNER_BUF_MODE)
    {
       reg_value = 0;
    }
	else
	{
		akprintf(C3, M_DRVSYS, "sd config error!\n");
		while(1);
	}
	

    REG32(s_SdRegDmaMode) = reg_value;
}


/**
 * @brief send sd command.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return bool
 * @retval  true: CMD sent successfully
 * @retval  false: CMD sent failed

 */
bool sdio_send_cmd( unsigned char cmd_index, unsigned char resp ,unsigned long arg)
{
    unsigned long cmd_value = 0;
    unsigned long status;

    if (cmd_index == SD_CMD(1) || cmd_index == SD_CMD(41) || cmd_index == SD_CMD(5))      //R3 is no crc
    {
        cmd_value = CPSM_ENABLE | ( resp << WAIT_REP_OFFSET) | ( cmd_index << CMD_INDEX_OFFSET) | RSP_CRC_NO_CHK;
    }
    else
    {
        cmd_value = CPSM_ENABLE | ( resp << WAIT_REP_OFFSET ) | ( cmd_index << CMD_INDEX_OFFSET) ;
    }

    REG32(s_SdRegArgument) = arg;
    REG32(s_SdRegCmd) = cmd_value;

    if (SD_NO_RESPONSE == resp)
    {
        while(1)
        {
            status = REG32(s_SdRegStatus);
            if (status & CMD_SENT)
            {
                return true;
            }
        }
    }
    else if ((SD_SHORT_RESPONSE == resp) ||(SD_LONG_RESPONSE == resp))
    {
        while(1)
        {
            status = REG32(s_SdRegStatus);
            if ((status & CMD_TIME_OUT)||(status & CMD_CRC_FAIL))
            {
                akprintf(C1, M_DRVSYS, "send cmd %d error, status = %x\n", cmd_index, status);
                return false;       
            }
            else if (status & CMD_RESP_END)
            {
                return true;
            }
        }
    }
    else
    {
        akprintf(C3, M_DRVSYS, "error requeset!\n");
        return false;
    }                   
}

/**
 * @brief Get sd card  short response.
 *
 * Only get register response0 .
 * @author Huang Xin
 * @date 2010-07-14
 * @return The value of register response0
 */
unsigned long sdio_get_short_resp()
{
    unsigned long resp_value=0;
    
    resp_value = REG32(s_SdRegResp0);   
    return resp_value;      
}


/**
 * @brief Get sd card  long response.
 *
 *  Get register response0,1,2,3.
 * @author Huang Xin
 * @date 2010-07-14
 * @param resp[in] The buffer address to save long response
 * @return void
 */
void sdio_get_long_resp(unsigned long resp[])
{
    unsigned long resp_value=0;
    
    resp[3] = REG32(s_SdRegResp0);
    resp[2] = REG32(s_SdRegResp1);
    resp[1] = REG32(s_SdRegResp2);
    resp[0] = REG32(s_SdRegResp3);         
}


/**
 * @brief set sd controller data register.
 *
 * Set timeout value,transfer size,transfer direction,bus mode,data block len
 * @author Huang Xin
 * @date 2010-07-14
 * @param len[in] Transfer size
 * @param blk_size[in] Block length
 * @param dir[in] transfer direction
 * @return void
 */
static void sdio_set_data_reg(unsigned long len,unsigned long blk_size,unsigned char bus_mode ,unsigned char dir)
{
    unsigned long reg_value;

    REG32(s_SdRegDataTim) = SD_DAT_MAX_TIMER_V;
    REG32(s_SdRegDataLen) = len;

    reg_value = SD_DATA_CTL_ENABLE | ( dir << SD_DATA_CTL_DIRECTION_OFFSET ) \
                | (bus_mode << SD_DATA_CTL_BUS_MODE_OFFSET) \
                | (blk_size << SD_DATA_CTL_BLOCK_LEN_OFFSET );
   
    REG32(s_SdRegDataCtrl) = reg_value;
 
}


/**
 * @brief SD read or write data use l2 dma
 *
 * start l2 dma
 * @author Huang Xin
 * @date 2010-07-14
 * @param ram_addr[in] the ram address used by dma
 * @param size[in] transfer bytes
 * @param dir[in] transfer direction
 * @return bool
 * @retval  true: transfer successfully
 * @retval  false: transfer failed
 */
bool sdio_trans_data_dma(unsigned long ram_addr,unsigned long size,unsigned char dir)
{
    unsigned long status,data_left,data_to_trans;
    unsigned char buf_id = BUF_NULL;
    bool ret;
    
    //alloc L2 buffer
    buf_id = l2_alloc(s_L2Select);
    if(BUF_NULL == buf_id)
    {
        akprintf(C3, M_DRVSYS, "alloc L2 buffer failed!, buf=%d\n", buf_id);
        return false;
    }       

    sdio_cfg_buf(L2_DMA_MODE);
    //set data reg
    sdio_set_data_reg(size, g_pSdioDevice->ulDataBlockLen, g_pSdioDevice->enmBusMode, dir);

    //set l2 trans dir
    if (SD_DATA_CTL_TO_HOST == dir)
        dir = BUF2MEM;
    else
        dir = MEM2BUF;
    
    data_left = size;
    //start l2 dma to transfer data     
    do
    {
        //there is a wired problem when we use SD_DMA_SIZE_32K in the following operation:
        //l2 data trans finish, but sd controller stay in RX_ACTIVE status
        //so we change the length to SD_DMA_SIZE_64K. the reason remains to be explored
        //by lzj
        data_to_trans = (data_left >= SD_DMA_SIZE_64K) ? SD_DMA_SIZE_64K : data_left;
        //NOTE: 2011.06.09 by xc
        // 为解决音频播放papa音问题，需要将所有的L2访问改为cpu方式
        #if 0   
        l2_combuf_dma(ram_addr, buf_id, data_to_trans, dir,false);
        if(!l2_combuf_wait_dma_finish(buf_id))
        {
            goto EXIT;
        }
        #else
        if(!l2_combuf_cpu(ram_addr, buf_id, data_to_trans, dir))
        {
            goto EXIT;
        }
        #endif
        data_left -= data_to_trans;
        ram_addr += data_to_trans;
    }while(data_left>0);

EXIT:    
    while(1)
    {
        status = REG32(s_SdRegStatus);
        if (status & DATA_TIME_OUT)
        {
            akprintf(C3, M_DRVSYS, "timeout, status is %x\n", status);
            ret = false;
            break;
        }
        else if (status & DATA_CRC_FAIL)
        {
            akprintf(C3, M_DRVSYS, "crc error, status is %x\n", status);
            ret = false;
            break;
        }
        else if (BUF2MEM == dir && !(status & RX_ACTIVE))
        {
            ret = true;
            break;
        }
        else if (MEM2BUF == dir && !(status & TX_ACTIVE))
        {
            ret = true;
            break;
        }
    }
    l2_free(s_L2Select);
    return ret;
}

/**
 * @brief SD read or write data use cpu mode
 *
 * @author Huang Xin
 * @date 2010-07-14
 * @param ram_addr[in] the ram address used by dma
 * @param size[in] transfer bytes
 * @param dir[in] transfer direction
 * @return bool
 * @retval  true: transfer successfully
 * @retval  false: transfer failed
 */
bool sdio_trans_data_cpu(unsigned long ram_addr,unsigned long size,unsigned char dir)
{  
    bool ret = true;
    
	//set data reg
	if(size < g_pSdioDevice->ulDataBlockLen)  //zoutx add 判断blocklen是否大于size
	{
		sdio_set_data_reg(size, size, g_pSdioDevice->enmBusMode, dir); //zoutx: 如果大于则blocklen调成和size一样大		 
	}
	else
	{
    	sdio_set_data_reg(size, g_pSdioDevice->ulDataBlockLen, g_pSdioDevice->enmBusMode, dir);
	}
	
    //receive data
    if (SD_DATA_CTL_TO_HOST == dir)
    {    
        if(!sdio_rx_data_cpu((unsigned char*)ram_addr, size))
        {
            ret = false;
        }
    }
    //send data
    else
    {
        if(!sdio_tx_data_cpu((unsigned char*)ram_addr, size))
        {
            ret = false;
        }
    }
    return ret;

}


/**
* @brief sdio read multiply bytes
* @author Huang Xin
* @date 2010-07-14
* @param arg[in] the address of block to be selected to read 
* @param data[in] the pointer of array which will be read from card 
* @param size[in] the size of data which will read from card
* @return bool
* @retval  true: read  successfully
* @retval  false: read failed
*/
bool sdio_read_multi_byte(unsigned long arg,unsigned char data[],unsigned long size)
{
    unsigned char func=0;
    func = (arg>>CMD53_FUN_NUM)&0x7;
	 //config INNER BUFFER
	 sdio_cfg_buf(INNER_BUF_MODE);
    //config data register
	 if( size < g_pSdioDevice->ulFunBlockLen[func])  //zoutx: 如果大于则blocklen调成和size一样大		 
	 {
	 	sdio_set_data_reg(size, size,g_pSdioDevice->enmBusMode, SD_DATA_CTL_TO_HOST);
	 }
	 else
	 {
	 	sdio_set_data_reg(size, g_pSdioDevice->ulFunBlockLen[func],g_pSdioDevice->enmBusMode, SD_DATA_CTL_TO_HOST);
	 }
    //send cmd 53
    if (!sdio_send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "read sdio failed.\n");
        return  false; 
    }
    //receive data
    if(!sdio_rx_data_cpu(data, size))
    {
        return false;
    }
    return true;
}

/**
* @brief sdio write multiply bytes
* @author Huang Xin
* @date 2010-07-14
* @param arg[in] the argument of cmd53 
* @param data[in] the pointer of array which will be wrote to card 
* @param size[in] the size of data which will wrote to card
* @return bool
* @retval  true: write  successfully
* @retval  false: write failed
*/
bool sdio_write_multi_byte(unsigned long arg,unsigned char data[],unsigned long size)
{
    unsigned char func=0;
    func = (arg>>CMD53_FUN_NUM)&0x7;

	 //config INNER BUFFER
	 sdio_cfg_buf(INNER_BUF_MODE);
	 

	 //config data register	 
 	 if( size < g_pSdioDevice->ulFunBlockLen[func])  //zoutx add 判断blocklen是否大于size
 	 {
 	 	sdio_set_data_reg(size, size, g_pSdioDevice->enmBusMode, SD_DATA_CTL_TO_CARD);  //zoutx: 如果大于则blocklen调成和size一样大		 
 	 }
	 else
	 {
		 sdio_set_data_reg(size, g_pSdioDevice->ulFunBlockLen[func],g_pSdioDevice->enmBusMode, SD_DATA_CTL_TO_CARD);		 
	 }		 
	 
    //send cmd 53
    if (!sdio_send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "read sdio failed.\n");
        return  false; 
    }
    //send data
    if(!sdio_tx_data_cpu(data, size))
    {
        return false;
    }
    return true;
}

/**
* @brief sdio read multiply blocks
* @author Huang Xin
* @date 2010-07-14
* @param arg[in] the argument of cmd53 
* @param data[in] the pointer of array which will be wrote to card 
* @param size[in] the size of data which will read from card
* @return bool
* @retval  true: read  successfully
* @retval  false: read failed
*/
bool sdio_read_multi_block(unsigned long arg,unsigned char data[],unsigned long size)
{
    unsigned char func=0;
    func = (arg>>CMD53_FUN_NUM)&0x7;
	 //config INNER BUFFER
	 sdio_cfg_buf(INNER_BUF_MODE);
    //config data register
    sdio_set_data_reg(size, g_pSdioDevice->ulFunBlockLen[func], g_pSdioDevice->enmBusMode,SD_DATA_CTL_TO_HOST);
    //send cmd 53
    if (!sdio_send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "read sdio failed.\n");
        return  false; 
    }
    //receive data
    if(!sdio_rx_data_cpu(data, size))
    {
        return false;
    }
    if(0 == (arg&CMD53_COUNT_MASK))
    {
        sdio_abort_trans(func);
    }
    return true;
}


/**
  * @brief 使用L2模式代替原来的CPU FIFO模式
  * @author Zoutx
  * @date 2016-02-21
  * @param arg[in] the argument of cmd53 
  * @param data[in] the pointer of array which will be read from card 
  * @param size[in] the size of data which will read from card
  * @return T_BOOL
  * @retval  AK_TRUE: read	successfully
  * @retval  AK_FALSE: read failed
  */
bool sdio_read_multi_block_L2(unsigned long arg,unsigned char data[],unsigned long size)
{
	unsigned char func=0;
	  
	func = (arg>>CMD53_FUN_NUM)&0x7;

	//config cpu L2 mode
	sdio_cfg_buf(L2_CPU_MODE);
	  
	if (!sdio_send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
	{
		akprintf(C3, M_DRVSYS, "read sdio CMD53 failed.\n");
		return  false; 
	}

	g_pSdioDevice->ulDataBlockLen = g_pSdioDevice->ulFunBlockLen[func];
	  
	//zoutx : 其实这里并没有用到DMA， 只是用到L2模式而已
	if( sdio_trans_data_dma((unsigned long)data, size, SD_DATA_CTL_TO_HOST) == false)
	{
		akprintf(C3, M_DRVSYS, "sdio_trans_data_dma failed.\n");
		return  false; 
	}
	  	
	if(0 == (arg&CMD53_COUNT_MASK))
	{
		sdio_abort_trans(func);
	}

	return true;
}

/* @brief sdio write multiply blocks
* @author Huang Xin
* @date 2010-07-14
* @param arg[in] the argument of cmd53 
* @param data[in] the pointer of array which will be wrote to card 
* @param size[in] the size of data which will wrote to card
* @return bool
* @retval  true: write  successfully
* @retval  false: write failed
*/
bool sdio_write_multi_block(unsigned long arg,unsigned char data[],unsigned long size)
{
    //send cmd 53
    unsigned char func=0;
    func = (arg>>CMD53_FUN_NUM)&0x7;
	 //config INNER BUFFER
	 sdio_cfg_buf(INNER_BUF_MODE);
    //config data register
    sdio_set_data_reg(size, g_pSdioDevice->ulFunBlockLen[func], g_pSdioDevice->enmBusMode,SD_DATA_CTL_TO_CARD);
    
    if (!sdio_send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "read sdio failed.\n");
        return  false; 
    }
    //receive data
    if(!sdio_tx_data_cpu(data, size))
    {
        return false;
    }
	 if(0 == (arg&CMD53_COUNT_MASK))
	 {
		 sdio_abort_trans(func);
	 }
	 return true;
 }



 /**
   * @brief 使用L2模式代替原来的CPU FIFO模式
   * @author Zoutx
   * @date 2016-02-21
   * @param arg[in] the argument of cmd53 
   * @param data[in] the pointer of array which will be wrote to card 
   * @param size[in] the size of data which will write from card
   * @return T_BOOL
   * @retval  AK_TRUE: read  successfully
   * @retval  AK_FALSE: read failed
   */
bool sdio_write_multi_block_L2(unsigned long arg,unsigned char data[],unsigned long size)
{
	 //send cmd 53
	 unsigned char func=0;
	 func = (arg>>CMD53_FUN_NUM)&0x7;

	 //config cpu L2 mode
	 sdio_cfg_buf(L2_CPU_MODE);

	 if (!sdio_send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
	 {
		 akprintf(C3, M_DRVSYS, "read sdio failed.\n");
		 return  false; 
	 }

 	 g_pSdioDevice->ulDataBlockLen = g_pSdioDevice->ulFunBlockLen[func];

  	 //zoutx : 其实这里并没有用到DMA， 只是用到L2模式而已
	 if( sdio_trans_data_dma((unsigned long)data, size, SD_DATA_CTL_TO_CARD) == false)
	 {
		akprintf(C3, M_DRVSYS, "sdio_trans_data_dma failed.\n");
		return  false; 
	 }
 
	 if(0 == (arg&CMD53_COUNT_MASK))
	 {
		 sdio_abort_trans(func);
	 }
	 return true;
}


/**
* @brief Abort sdio multi blocks transfer
* @author Huang Xin
* @date 2010-07-14
* @param func_nbr[in] the function number of sdio,0~7
* @return bool
* @retval  true: abort  successfully
* @retval  false: abort failed
*/
static bool sdio_abort_trans(unsigned char func_nbr)
{
    unsigned char temp = 0;

    if(!sdio_read_byte(0, CCCR_IO_ABORT, &temp))
    {  
        return false;
    }
    temp &= ~(0x7<<0);
    temp |= (func_nbr<<0);

    if(!sdio_write_byte(0, CCCR_IO_ABORT, temp))
    {  
        return false;
    }
    akprintf(C3, M_DRVSYS, "abort tranfer ok!\n");
    return true;     
}

/**
* @brief sd controller send data ,use cpu mode
* @author Huang Xin
* @date 2010-07-14
* @param buf[in] the pointer of array which will be sent to card 
* @param len[in] the size of data which will be sent to card 
* @return bool
* @retval  true: send  successfully
* @retval  false: send failed
*/
static bool sdio_tx_data_cpu(unsigned char buf[], unsigned long len)
{
    unsigned long status;
    unsigned long offset = 0;
    
    while(1)
    {
        status = REG32(s_SdRegStatus);
        if ((offset < len) && (status & DATA_BUF_EMPTY))
        {
            REG32(s_SdRegCpuMode) = (buf[offset])|(buf[offset+1]<<8)|(buf[offset+2]<<16)|(buf[offset+3]<<24);
            offset += 4;
        }
        if ((status & DATA_TIME_OUT) || (status & DATA_CRC_FAIL))
        {
            akprintf(C3, M_DRVSYS, "crc error or timeout, status is %x\n",status); 
            return false;
        }
        if (!(status & TX_ACTIVE))
        {
            break;
        }
    }

    return true;
}

/**
* @brief sd controller receive data ,use cpu mode
* @author Huang Xin
* @date 2010-07-14
* @param buf[in] the pointer of array to save received data
* @param len[in] the size of data received
* @return bool
* @retval  true: receive  successfully
* @retval  false: receive failed
*/
static bool sdio_rx_data_cpu(unsigned char buf[], unsigned long len)
{
    unsigned long status;
    unsigned long buf_tmp;
    unsigned long i;
    unsigned long offset, size;

    offset = 0;
    size = len;
    while(1)
    {
        status = REG32(s_SdRegStatus);
        if ((status & DATA_BUF_FULL))
        {
            buf_tmp  = REG32(s_SdRegCpuMode );   
            for (i = 0; i < 4; i++)
            {
                buf[offset + i] = (unsigned char)((buf_tmp >> (i * 8)) & 0xff);
            }
            offset += 4;
            size -= 4;
        }
        if ((size > 0) && (size < 4) && (status & DATA_END))
        {
            buf_tmp = REG32(s_SdRegCpuMode );                
            for ( i = 0; i < size; i++)
            {
                buf[offset + i] = (unsigned char)((buf_tmp >> (i * 8)) & 0xff);
            }
            size = 0;           
        }
        if ((status & DATA_TIME_OUT) || (status & DATA_CRC_FAIL))
        {
            akprintf(C3, M_DRVSYS, "crc error or timeout, status is %x\n", status); 
            return false;
        }
        if (!(status & RX_ACTIVE))
        {
            break;
        }
    }

    return true;
}

/**
* @brief Check if sd controller is transferring now
* @author Huang Xin
* @date 2010-07-14
* @return bool
* @retval  true: sd controller is transferring
* @retval  false: sd controller is not transferring
*/
bool sdio_trans_busy()
{
    unsigned long status;
    status = REG32(s_SdRegStatus);
    if ((status & TX_ACTIVE) || (status & RX_ACTIVE))
    {
        akprintf(C3, M_DRVSYS, " The sd card is writing and reading: %x.\n", status);
        return  true;
    }
    else 
    {
        return false;
    }
}

#if 0

/**
 * @brief change sd timing when Freq has changed
 *
 * @author liaozhijun
 * @date 2011-6-23
 * @param[in] Freq frequency
 * @return  void
 */
void sd_changetiming(unsigned long freq)
{

    DrvModule_Protect(DRV_MODULE_SDMMC);
    
    //check sdio interface
    if(REG32(SDIO_CLK_CTRL_REG) & (1<<20))
    {
        set_sdio_if();
        set_clock(s_sdio_clok, freq, true);
    }

    //check sdmmc interface
    if(REG32(SD_CLK_CTRL_REG) & (1<<20))
    {
        set_sdmmc_if();
        set_clock(s_sdmmc_clok, freq, true);
    }
    
    DrvModule_UnProtect(DRV_MODULE_SDMMC);

}

static int sd_reg(void)
{
    freq_register(E_ASIC_CALLBACK, sd_changetiming);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(sd_reg)
#ifdef __CC_ARM
#pragma arm section
#endif
#endif

