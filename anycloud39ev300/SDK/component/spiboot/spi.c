/*
 * serial flash driver
 */
/**
* @file serial_flash_drv.c
* @brief serial flash driver C file.
*
* Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
* @author 
* @date 2005-11-15
* @version 1.0
* @ref AK37XX technical manual.
*/


#include "system.h" 
#include "anyka_cpu.h"
#include "spi.h"
#include "L2.h"
#define DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
#define    SPI_DEBUG     printf
#else
#define    SPI_DEBUG     
#endif
#define    BUF2MEM                0
#define    MEM2BUF                1
#define    SPI_MASTER_TRANFINISH           (1<<8)
#define    BUF_NULL     0xff

#define    PARAM_ID           0
#define    PARAM_TOTAL_SIZE   1
#define    PARAM_PAGE_SIZE    2
#define    PARAM_PROGAM_SIZE  3
#define    PARAM_ERASE_SIZE   4
#define    PARAM_CLOCK        5
#define    PARAM_OTHER        6 
#define		MAX_RD_SIZE     	(32 * 1024)
//#define   SPI_GPIO_37_TO_40  

static  T_BOOL SPI_INIT_FLG=AK_FALSE;

extern T_SPI s_tSpi[SPI_NUM];

//extern  T_U32 spiflash_param;

extern T_U32 	get_asic_freq(T_VOID);

static T_VOID spi_sharepin_cfg(T_eSPI_ID ID);


T_BOOL spi_master_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS)
{
    T_BOOL ret = AK_TRUE;
    T_U32 i = 0;
    
    //alloc L2 buffer
    s_tSpi[spi_id].ucTxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Tx);
    if (0 == s_tSpi[spi_id].ucTxBufferID)
    {
        return AK_FALSE;
    }
    //prepare spi
    force_cs(spi_id);
    set_tx(spi_id);
	    
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;

    if((T_U32)buf & 0x3)
    {
        l2_combuf_cpu((T_U32)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF);
    }
    else
    {
        //start l2 dma transmit
        l2_combuf_dma((T_U32)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF, AK_FALSE);
        if (AK_FALSE == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucTxBufferID))
        {
            ret =  AK_FALSE;
            goto EXIT;
        }        
    }

    //wait data cnt decreased to 0
    while (1)
    {      
        if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR))
            break;
    }

    //wait finish status
    while (1)
    {   
        if ((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH )
            break;
    }
        
EXIT:

    //disable l2 dma
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
    l2_clr_status(s_tSpi[spi_id].ucTxBufferID);
	
    //pull up CS
    if (bReleaseCS)
    { 
        for (i = 0; i < 3000; i++);
        unforce_cs(spi_id);
    }
    //free L2 buffer
    l2_free(s_tSpi[spi_id].ucL2Tx); 
    return ret;
  
}

T_BOOL spi_master_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS)
{
    T_BOOL ret = AK_TRUE;
    T_U32 i = 0;
    if (!s_tSpi[spi_id].bOpen)
    {
        return AK_FALSE;
    }
    //alloc L2 buffer
    s_tSpi[spi_id].ucRxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Rx);
    if (BUF_NULL == s_tSpi[spi_id].ucRxBufferID)
    {
        return AK_FALSE;
    }
    //prepare spi read
    force_cs(spi_id);
    set_rx(spi_id);
	
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = ((1<<0)|(1<<16));
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;

    if(count >= 64)
    {
        //start L2 dma
        l2_combuf_dma((T_U32)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, AK_FALSE);
		//wait spi trans finish
        while (1)
        {            
            if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
                break;
        }
        //wait L2 dma finish, if need frac dma,start frac dma
        if (AK_FALSE == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID))
        {
			ret =  AK_FALSE;
        }
    }
    else
    {
        //wait spi trans finish
        while (1)
        {            
            if(0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
                break;
        }
        //start frac dma
        l2_combuf_dma((T_U32)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, AK_FALSE);
        //wait frac dma finish
        l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID);
    }
    //disable l2 dma
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;    
    l2_clr_status(s_tSpi[spi_id].ucRxBufferID);
    //pull up CS
    if (bReleaseCS)
    {
        for(i = 0; i < 3000; i++);
        unforce_cs(spi_id);
    }  
    //free L2 buffer
    l2_free(s_tSpi[spi_id].ucL2Rx);   
    return ret;

}

/**
 * @brief spi_data_mode - select spi data mode, set GPIO pin #SPI_D2 #SPI_D3 
 * @author LuHeshan
 * @date 2012-12-24
 * @param spi_id: spi id
 * @param data_mode: 1-2-4wire
 * @return T_BOOL
 * @version 
 */
T_BOOL spi_data_mode(T_eSPI_ID spi_id, T_eSPI_BUS data_mode)
{
    T_U32 reg_value;

    if ((!s_tSpi[spi_id].bOpen) || (SPI_BUS_NUM <= data_mode))
    {
        SPI_DEBUG("spi_data_mode(): spi no open or param fail!\n", 0,  AK_TRUE);
        return AK_FALSE;
    }

    s_tSpi[spi_id].ucBusWidth = data_mode;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(0x3 << 16);
    reg_value |= (data_mode << 16);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value;

    return AK_TRUE;
}


#define MHz	1000000UL

/**************************************************************************
  Function:       T_U32 spi_get_clk_div(T_VOID)
  Description:    get spi clk div.  
  Input:          role: master or slaver                 
  Output:         none
  Return:         none
  Author:         
****************************************************************************/
T_U32 spi_get_clk_div(T_U32 typical_hz)
{
  	T_U32 asic_clk, div, hz;

	asic_clk = get_asic_freq();
	div = asic_clk / (typical_hz*2) - 1;
	
	if (div > 255)
		div = 255;
	
	hz = asic_clk /((div+1)*2);
	if((hz - typical_hz) > typical_hz/10)
		div++;

	SPI_DEBUG("asic clk:%d, pre-scaler=%d (wanted %dMhz, got %dMhz)\r\n",
			asic_clk, div, typical_hz/MHz, (asic_clk / (2 * (div + 1)))/MHz);

	return div;

}

static T_VOID spi_sharepin_cfg(T_eSPI_ID ID)
{
	T_U32 regval;

	if(ID == SPI_ID0) {
#ifndef SPI_GPIO_37_TO_40
		REG32(SHARE_PIN_CFG_REG4) |= (0x3 | (0x3f<<8));
		REG32(SHARE_PIN_CFG_REG1) |= (0x0<<25);
#else
		REG32(SHARE_PIN_CFG_REG4) |= (0x3 | (0x3f<<14));
		REG32(SHARE_PIN_CFG_REG1) |= (0x0<<25);
#endif
	}
}

/**************************************************************************
  Function:       T_VOID SpiCtrlInit(T_U32 mode, T_U32 role, T_U32 clk_div);
  Description:    initial the spi ctrl
  Input:          role: master or slaver                 
  Output:         none
  Return:         none
  Author:         Zou Tianxiang
****************************************************************************/
T_VOID spi_ctrl_init(T_eSPI_ID ID,T_U32 mode, T_U32 role, T_U32 hz)
{
  	T_U32 ctrl_value=0, clk_div;
	
	clk_div = spi_get_clk_div(hz);
	ctrl_value = (role | (clk_div << 8) | SPI_CTRL_ENABLE | mode | SPI_CTRL_RX_REJECT);
	REG32(SPI0_BASE_ADDR + ASPEN_SPI_CTRL) = ctrl_value;					//open spi1
	
	spi_sharepin_cfg(ID);
	
	s_tSpi[ID].ucL2Rx = ADDR_SPI1_RX;
    s_tSpi[ID].ucL2Tx = ADDR_SPI1_TX;
	s_tSpi[ID].ulBaseAddr = SPI0_BASE_ADDR;
	s_tSpi[ID].bOpen = AK_TRUE;

	SPI_INIT_FLG = AK_TRUE;
}

T_BOOL spi_IsInit(T_VOID)
{
	return SPI_INIT_FLG;
}

