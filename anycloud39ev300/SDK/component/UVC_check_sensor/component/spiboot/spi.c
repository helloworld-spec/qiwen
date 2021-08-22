/**
 * @file spi.c
 * @brief SPI driver, define SPI APIs.
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Huang Xin
 * @date 2010-11-17
 * @version 1.0
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "l2.h"
#include "spi.h"
#include "arch_spi.h"
#include "freq.h"


//temporarily move the spi protection to hal layer, which is spi flash driver
//coz it's difficulty to handle the cs release/unrelease situation on this layer
#define SPI_DRV_PROTECT(id) 

#define SPI_DRV_UNPROTECT(id) 
static T_SPI s_tSpi[SPI_NUM] = {0};

static T_VOID spi_on_change(T_U32 asic_clk);

T_VOID spi_set_protect(T_U32 spi_id, T_U8 width)
{
    T_U32 reg_value;
    
    reg_value = REG32(GPIO_SHAREPIN_CONTROL1);
    reg_value &= ~(1<<25);
    reg_value |= (1<<25);
    REG32(GPIO_SHAREPIN_CONTROL1) = reg_value;
	
    reg_value = REG32(GPIO_SHAREPIN_CONTROL4);
    reg_value &= ~((3<<0)|(0x3f<<8));
    reg_value |= ((3<<0)|(0x2a<<8));
    REG32(GPIO_SHAREPIN_CONTROL4) = reg_value;

}

T_VOID spi_set_unprotect(T_U32 spi_id, T_U8 width)
{

}

static void set_tx(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(1<<0);
    reg_value |= (1<<1);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
}

static void set_rx(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value |= (1<<0);
    reg_value &= ~(1<<1);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
}

static void set_rx_tx(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(3<<0);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value;
}

static void force_cs(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value |= SPI_CTRL_FORCE_CS;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
    s_tSpi[spi_id].ucCS = 1;    //cs enable
}

static void unforce_cs(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~SPI_CTRL_FORCE_CS;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
    s_tSpi[spi_id].ucCS = 0;    //cs disable
}

static T_U8 get_spi_clk_div(T_U32 asic_clk, T_U32 spi_clk)
{
    T_U8 clk_div;

    clk_div = (asic_clk/2 - 1) / spi_clk;

    return clk_div;
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
        return AK_FALSE;
    }

    s_tSpi[spi_id].ucBusWidth = data_mode;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(0x3 << 16);
    reg_value |= (data_mode << 16);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value;

    return AK_TRUE;
}

/**
 * @brief Initialize SPI
 *
 * this func must be called before call any other SPI functions
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param mode[in] spi mode selected 
 * @param role[in] master or slave
 * @param clk_div[in] SPI working frequency = ASICCLK/(2*(clk_div+1)
 * @return T_BOOL
 * @retval AK_TRUE: Successfully initialized SPI.
 * @retval AK_FALSE: Initializing SPI failed.
 */
T_BOOL spi_init(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, T_U32 clk)
{
    T_U32 div;
    
    if (spi_id > SPI_ID1 || mode >= SPI_MODE_NUM|| role >= SPI_ROLE_NUM)
    {
        return AK_FALSE;
    }
    
    s_tSpi[spi_id].ucRole= role;
    s_tSpi[spi_id].ucMode = mode;  
    s_tSpi[spi_id].clock = clk;  
    s_tSpi[spi_id].bOpen = AK_TRUE;
    
    s_tSpi[spi_id].ulBaseAddr = (SPI0_BASE_ADDR + spi_id*0x8000);
    s_tSpi[spi_id].ucL2Rx = (ADDR_SPI0_RX + spi_id*3);
    s_tSpi[spi_id].ucL2Tx = (ADDR_SPI0_TX + spi_id*3);

    s_tSpi[spi_id].ucBusWidth = SPI_BUS1;

	if(SPI_ID0 == spi_id)
	{
	    REG32(CLOCK_CTRL_REG) &= ~(1<<5);
	}
	else
	{
	    REG32(CLOCK_CTRL_REG) &= ~(1<<6);
	}
	
    div = get_spi_clk_div(get_asic_freq(), clk);  
    if((SPI_SLAVE == role) && (div < 3)) div = 3;

    s_tSpi[spi_id].ucCS = 0;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = (1<<1) | (mode<<2) | (role<<4) | SPI_CTRL_ENA_WORK | (div<<8);

    return AK_TRUE;
}


/**
 * @brief spi master write
 *
 * this func must be called in spi  master mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @param bReleaseCS[in] whether pll up cs
 * @return T_BOOL
 * @retval AK_TRUE:  spi write successfully.
 * @retval AK_FALSE: spi write failed.
 */
T_BOOL spi_master_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS)
{
    T_BOOL ret = AK_TRUE;
    T_U32 i = 0;
    T_U32 offset = 0;
    T_U32 temp = 0;

    if (!s_tSpi[spi_id].bOpen)
    {
        printf("spi_master_write(): SPI not initialized!\n");
        return AK_FALSE;
    }

    SPI_DRV_PROTECT(spi_id);

    if ((T_U32)buf & 0x3 || count < 64)
    {

        force_cs(spi_id);
        set_tx(spi_id);
        
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;
        for(i = 0; i < count; i++)
        {
            temp |= (buf[i] << (offset * 8));

            offset++;
            if((offset == 4) || (i == (count-1)))
            {
                while(!(REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_TXBUF_HALFEMPTY));

                REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_INBUF) = temp;

                temp = 0;
                offset = 0;
            }
        }
        //wait finish status
        while((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) != SPI_MASTER_TRANFINISH );
    }
    else
    {
        //alloc L2 buffer
        s_tSpi[spi_id].ucTxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Tx);
        if (BUF_NULL == s_tSpi[spi_id].ucTxBufferID)
        {
            printf("spi_master_write(): allocate L2 buffer failed,  tx=%d\n", s_tSpi[spi_id].ucTxBufferID);
            SPI_DRV_UNPROTECT(spi_id);       
            return AK_FALSE;
        }
        
        force_cs(spi_id);
        set_tx(spi_id);

        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;

        l2_combuf_cpu((T_U32)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF);

        //wait data cnt decreased to 0
        while (1)
        {      
            if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR))
                break;
        }
        
        //wait finish status
        while((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) != SPI_MASTER_TRANFINISH );

    EXIT:
        //disable l2 
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
        l2_clr_status(s_tSpi[spi_id].ucTxBufferID); 
        //free L2 buffer
        l2_free(s_tSpi[spi_id].ucL2Tx);   
    }       

    //pull up CS
    if (bReleaseCS)
    { 
        for (i = 0; i < 30; i++);//improve spi speed 
        unforce_cs(spi_id);
    }

    SPI_DRV_UNPROTECT(spi_id);       
    return ret;
}



/**
 * @brief spi master read
 *
 * this func must be called in spi  master mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the read data buffer  
 * @param count[in] the read data count
 * @param bReleaseCS[in] whether pll up cs
 * @return T_BOOL
 * @retval AK_TRUE:  spi read successfully.
 * @retval AK_FALSE: spi read failed.
 */
T_BOOL spi_master_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS)
{
    T_BOOL ret = AK_TRUE;
    T_U32 i = 0;
    T_U32 offset = 0;
    T_U32 temp = 0;

    if (!s_tSpi[spi_id].bOpen)
    {
        return AK_FALSE;
    }

    SPI_DRV_PROTECT(spi_id);
    if((T_U32)buf & 0x3 || count < 64 )    // spi 
    {   
        //prepare spi read
        force_cs(spi_id);
        set_rx(spi_id);

        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;   
        for(i = 0; i < count; i++)
        {
            if(offset == 0)
            {
                if((i+4) > count)
                {
                    while(!(REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH));
                }
                else
                {
                    while(!(REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_RXBUF_HALFEMPTY));
                }
                
                temp = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_INBUF);
            }
            buf[i] = (T_U8)((temp >> (offset * 8)) & 0xff);

            if(++offset == 4)
            {
                offset = 0;
            }
        }
    }
    else            
    {
        //alloc L2 buffer
        s_tSpi[spi_id].ucRxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Rx);
        
        if (BUF_NULL == s_tSpi[spi_id].ucRxBufferID)
        {
            printf("L2 alloc Fail\n");
            return AK_FALSE;
        }
        
        force_cs(spi_id);
        set_rx(spi_id);

        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = ((1<<0)|(1<<16));
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;   

        //start L2 CPU
        l2_combuf_cpu((T_U32)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM);
        
        
        //wait spi trans finish
        while (1)
        {            
            if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
                break;
        }

        //disable l2
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;
        l2_clr_status(s_tSpi[spi_id].ucRxBufferID);
        //free L2 buffer
        l2_free(s_tSpi[spi_id].ucL2Rx);    
    }
    
    //pull up CS
    if (bReleaseCS)
    {
        for(i = 0; i < 30; i++);
        unforce_cs(spi_id);
    }
    
    SPI_DRV_UNPROTECT(spi_id);       
    return ret;
}

