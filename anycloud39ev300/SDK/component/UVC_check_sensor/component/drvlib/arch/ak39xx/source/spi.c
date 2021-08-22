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
#include "sysctl.h"
#include "drv_api.h"
#include "l2.h"
#include "drv_module.h"
#include "spi.h"
#include "drv_gpio.h"
#include "freq.h"


//temporarily move the spi protection to hal layer, which is spi flash driver
//coz it's difficulty to handle the cs release/unrelease situation on this layer
#define SPI_DRV_PROTECT(id) 

#define SPI_DRV_UNPROTECT(id) 

static T_SPI s_tSpi[SPI_NUM] = {0};

static void spi_on_change(unsigned long asic_clk);


void spi_set_protect(unsigned long spi_id, unsigned char width)
{
    DrvModule_Protect(DRV_MODULE_SPI);
    gpio_pin_group_cfg(ePIN_AS_SPI0+spi_id);
}

void spi_set_unprotect(unsigned long spi_id, unsigned char width)
{
    DrvModule_UnProtect(DRV_MODULE_SPI);
}

static void set_tx(unsigned char spi_id)
{
    unsigned long reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(1<<0);
    reg_value |= (1<<1);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
}

static void set_rx(unsigned char spi_id)
{
    unsigned long reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value |= (1<<0);
    reg_value &= ~(1<<1);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
}

static void set_rx_tx(unsigned char spi_id)
{
    unsigned long reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(3<<0);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value;
}

static void force_cs(unsigned char spi_id)
{
    unsigned long reg_value;

    gpio_pin_group_cfg(ePIN_AS_SPI0+spi_id);

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value |= SPI_CTRL_FORCE_CS;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
    s_tSpi[spi_id].ucCS = 1;    //cs enable
}

static void unforce_cs(unsigned char spi_id)
{
    unsigned long reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~SPI_CTRL_FORCE_CS;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
    s_tSpi[spi_id].ucCS = 0;    //cs disable
}

static unsigned char get_spi_clk_div(unsigned long asic_clk, unsigned long spi_clk)
{
    unsigned char clk_div;

    clk_div = (asic_clk/2 - 1) / spi_clk;

    akprintf(C3, M_DRVSYS, "spi clk: %d\n", asic_clk/(2*(1+clk_div)));

    return clk_div;
}

static void spi_on_change(unsigned long asic_clk)
{
    unsigned long reg;
    unsigned char div;
    unsigned long i;
    
    DrvModule_Protect(DRV_MODULE_SPI);
    
    for (i = 0; i < SPI_NUM; i++)
    {
        if (s_tSpi[i].bOpen)
        {
            //akprintf(C3, M_DRVSYS, "spi on change: %d\n", s_tSpi[i].clock);

            //calculate clock
            div = get_spi_clk_div(asic_clk, s_tSpi[i].clock);
            if((SPI_SLAVE == s_tSpi[i].ucRole) && (div < 3)) div = 3;
            
            REG32(s_tSpi[i].ulBaseAddr + ASPEN_SPI_CTRL) = (1<<1) | (s_tSpi[i].ucMode<<2) | (s_tSpi[i].ucRole<<4) | SPI_CTRL_ENA_WORK | (div<<8);
        }
    }
    
    DrvModule_UnProtect(DRV_MODULE_SPI);
}

/**
 * @brief spi_data_mode - select spi data mode, set GPIO pin #SPI_D2 #SPI_D3 
 * @author LuHeshan
 * @date 2012-12-24
 * @param spi_id: spi id
 * @param data_mode: 1-2-4wire
 * @return bool
 * @version 
 */
bool spi_data_mode(T_eSPI_ID spi_id, T_eSPI_BUS data_mode)
{
    unsigned long reg_value;

    if ((!s_tSpi[spi_id].bOpen) || (SPI_BUS_NUM <= data_mode))
    {
        akprintf(C1, M_DRVSYS, "spi_data_mode(): spi no open or param fail!\n");
        return false;
    }

    s_tSpi[spi_id].ucBusWidth = data_mode;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(0x3 << 16);
    reg_value |= (data_mode << 16);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value;

    return true;
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
 * @return bool
 * @retval true: Successfully initialized SPI.
 * @retval false: Initializing SPI failed.
 */
bool spi_init(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, unsigned long clk)
{
    unsigned long div;
    
    if (spi_id > SPI_ID1 || mode >= SPI_MODE_NUM|| role >= SPI_ROLE_NUM)
    {
        akprintf(C1, M_DRVSYS, "SPI initialized failed!\n");
        return false;
    }
    
    s_tSpi[spi_id].ucRole= role;
    s_tSpi[spi_id].ucMode = mode;  
    s_tSpi[spi_id].clock = clk;  
    s_tSpi[spi_id].bOpen = true;
    
    s_tSpi[spi_id].ulBaseAddr = (SPI0_BASE_ADDR + spi_id*0x8000);
    s_tSpi[spi_id].ucL2Rx = (ADDR_SPI0_RX + spi_id*3);
    s_tSpi[spi_id].ucL2Tx = (ADDR_SPI0_TX + spi_id*3);
    s_tSpi[spi_id].ucBusWidth = SPI_BUS1;

	if(SPI_ID0 == spi_id)
	{
		sysctl_clock(CLOCK_SPI0_ENABLE);
	}
	else
	{
		sysctl_clock(CLOCK_SPI1_ENABLE);
	}

    div = get_spi_clk_div(get_asic_freq(), clk);  
    if((SPI_SLAVE == role) && (div < 3)) div = 3;

    s_tSpi[spi_id].ucCS = 0;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = (1<<1) | (mode<<2) | (role<<4) | SPI_CTRL_ENA_WORK | (div<<8);

    akprintf(C3, M_DRVSYS, "SPI initialized ok!\n");
    return true;
}

/**
 * @brief close SPI
 *
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @return void
 */
void spi_close(T_eSPI_ID spi_id)
{
    unsigned long i = 0;

    if (spi_id > SPI_ID1)
    {
        akprintf(C3, M_DRVSYS, "spi_id %d is invalid!\n",spi_id);
        return;
    }
    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_id %d is not open!\n",spi_id);
        return;
    }

    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) &= ~(1<<6);
    s_tSpi[spi_id].bOpen = false;

    //check whether all spi is closed
	if(SPI_ID0 == spi_id)
	{
		sysctl_clock(~CLOCK_SPI0_ENABLE);
	}
	else
	{
		sysctl_clock(~CLOCK_SPI1_ENABLE);
	}
        
}

/**
 * @brief reset SPI
 *
 * @author HuangXin
 * @date 2010-11-17
 * @return void
 */
void spi_reset()
{
    unsigned long i = 0;
    unsigned char div;
    
    for (i = 0; i < SPI_NUM; i++)
    {
        if (s_tSpi[i].bOpen)
        {
            div = get_spi_clk_div(get_asic_freq(), s_tSpi[i].clock);
            if((SPI_SLAVE == s_tSpi[i].ucRole) && (div < 3)) div = 3;

            REG32(s_tSpi[i].ulBaseAddr + ASPEN_SPI_CTRL) = (1<<1) | (s_tSpi[i].ucMode << 2) | (s_tSpi[i].ucRole << 4) | SPI_CTRL_ENA_WORK | (div << 8);           
        }
    }
}

bool spi_master_write_read(T_eSPI_ID spi_id, unsigned char *w_buf, unsigned char *r_buf,unsigned long count, bool bReleaseCS)
{
    bool ret = true;
    unsigned long i = 0, j;
    unsigned long offset = 0;
    volatile unsigned long last_len = 0;

    volatile unsigned long temp = 0;
    volatile unsigned long temp1 = 0;
	
	unsigned int *tmp_buf = (unsigned int *)r_buf;
    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_master_write(): SPI not initialized!\n");
        return false;
    }

    SPI_DRV_PROTECT(spi_id);
    if ((count < 512)||((unsigned long)r_buf & 0x3)||((unsigned long)w_buf & 0x3))
    {

        force_cs(spi_id);
        //set_tx(spi_id);
		set_rx_tx(spi_id);
        
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
		REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;
		REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count; 
		
        for(i = 0; i < count; i++)
        {
            temp |= (w_buf[i] << (offset * 8));

            offset++;
            if((offset == 4) || (i == (count-1)))
            {
                while(!(REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_TXBUF_HALFEMPTY));

                REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_INBUF) = temp;


				if(i == (count-1))
                {
	        		while((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) != SPI_MASTER_TRANFINISH );

                }
                else
                {
					while(!(REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_RXBUF_HALFEMPTY));
                }

				
				temp1 = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_INBUF);

				if(i == (count-1))
				{
					if(count%4)
					{
						offset = count - count%4;
						last_len = count%4;
						
					}				
					else
					{
						offset = count - 4;
						last_len = 4;
						
					}
					
					for(j = 0; j < last_len; j++)
					{
						r_buf[offset+j] = (unsigned char *)((temp1 >> j*8)&0xff);
					}
				}
				else
				{
					*tmp_buf++ = temp1;
				}
								
                temp = 0;
                offset = 0;
            }
			
        }
	}
    else
    {
    
        //alloc L2 buffer
        s_tSpi[spi_id].ucTxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Tx);
        if (BUF_NULL == s_tSpi[spi_id].ucTxBufferID)
        {
            akprintf(C1, M_DRVSYS, "spi_master_write(): allocate L2 buffer failed,  tx=%d\n", s_tSpi[spi_id].ucTxBufferID);
            SPI_DRV_UNPROTECT(spi_id);       
            return false;
        }
		s_tSpi[spi_id].ucRxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Rx);
        if (BUF_NULL == s_tSpi[spi_id].ucRxBufferID)
        {
			l2_free(s_tSpi[spi_id].ucL2Tx);   
            akprintf(C3, M_DRVSYS, "spi_master_read(): allocate L2 buffer failed,  rx=%d\n", s_tSpi[spi_id].ucRxBufferID);
            SPI_DRV_UNPROTECT(spi_id);       
            return false;
        }
        force_cs(spi_id);
        set_rx_tx(spi_id);

		                       
		REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = ((1<<0)|(1<<16));
        //REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;
		REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
		REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;

		//start L2 dma
        l2_combuf_dma((unsigned long)r_buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, false);
		//mini_delay(1);	
		//start l2 dma transmit
		l2_combuf_dma((unsigned long)w_buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF, false);	
		
        if (false == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucTxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_master_write(): l2 dma timeout\n");
            ret =  false;
            goto EXIT;
        } 

        //wait data cnt decreased to 0
        while (1)
        {      
            if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR))
                break;
        }
        
        //wait finish status
        while((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) != SPI_MASTER_TRANFINISH );


        //wait L2 dma finish, if need frac dma,start frac dma
        if (false == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_master_read(): l2 dma timeout\n");
            ret =  false;
        }
    EXIT:
    
        //disable l2 dma
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
        l2_clr_status(s_tSpi[spi_id].ucTxBufferID); 
        //free L2 buffer
        l2_free(s_tSpi[spi_id].ucL2Tx); 

		
		//disable l2 dma
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;    
        l2_clr_status(s_tSpi[spi_id].ucRxBufferID);
        //free L2 buffer
        l2_free(s_tSpi[spi_id].ucL2Rx);   
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
 * @brief spi master write
 *
 * this func must be called in spi  master mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @param bReleaseCS[in] whether pll up cs
 * @return bool
 * @retval true:  spi write successfully.
 * @retval false: spi write failed.
 */
bool spi_master_write(T_eSPI_ID spi_id, unsigned char *buf, unsigned long count, bool bReleaseCS)
{
    bool ret = true;
    unsigned long i = 0;
    unsigned long offset = 0;
    unsigned long temp = 0;

    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_master_write(): SPI not initialized!\n");
        return false;
    }

    SPI_DRV_PROTECT(spi_id);

    if ((count < 512)||((unsigned long)buf & 0x3))
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
            akprintf(C1, M_DRVSYS, "spi_master_write(): allocate L2 buffer failed,  tx=%d\n", s_tSpi[spi_id].ucTxBufferID);
            SPI_DRV_UNPROTECT(spi_id);       
            return false;
        }
        
        force_cs(spi_id);
        set_tx(spi_id);

        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;

        //start l2 dma transmit
        l2_combuf_dma((unsigned long)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF, false);
        
        if (false == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucTxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_master_write(): l2 dma timeout\n");
            ret =  false;
            goto EXIT;
        } 

        //wait data cnt decreased to 0
        while (1)
        {      
            if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR))
                break;
        }
        
        //wait finish status
        while((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) != SPI_MASTER_TRANFINISH );

    EXIT:
    
        //disable l2 dma
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
 * @return bool
 * @retval true:  spi read successfully.
 * @retval false: spi read failed.
 */
bool spi_master_read(T_eSPI_ID spi_id, unsigned char *buf, unsigned long count, bool bReleaseCS)
{
    bool ret = true;
    unsigned long i = 0;
    unsigned long offset = 0;
    unsigned long temp = 0;

    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_master_read(): SPI not initialized!\n");
        return false;
    }

    SPI_DRV_PROTECT(spi_id);

    if ((count < 512)||((unsigned long)buf & 0x3))
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
            buf[i] = (unsigned char)((temp >> (offset * 8)) & 0xff);

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
            akprintf(C3, M_DRVSYS, "spi_master_read(): allocate L2 buffer failed,  rx=%d\n", s_tSpi[spi_id].ucRxBufferID);
            SPI_DRV_UNPROTECT(spi_id);       
            return false;
        }
        //prepare spi read
        force_cs(spi_id);
        set_rx(spi_id);

        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = ((1<<0)|(1<<16));
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;   
        
        //start L2 dma
        l2_combuf_dma((unsigned long)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, false);

        //wait spi trans finish
        while (1)
        {            
            if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
                break;
        }

        //wait L2 dma finish, if need frac dma,start frac dma
        if (false == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_master_read(): l2 dma timeout\n");
            ret =  false;
        }
        //disable l2 dma
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

/**
 * @brief spi slave read
 *
 * this func must be called in spi  slave mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the read data buffer  
 * @param count[in] the read data count
 * @return bool
 * @retval true:  spi read successfully.
 * @retval false: spi read failed.
 */
bool spi_slave_read(T_eSPI_ID spi_id, unsigned char *buf, unsigned long count)
{
    bool ret = true;

    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_slave_read(): SPI not initialized!\n");
        return false;
    }

    SPI_DRV_PROTECT(spi_id);
    
    //alloc L2 buffer
    s_tSpi[spi_id].ucRxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Rx);
    if (BUF_NULL == s_tSpi[spi_id].ucRxBufferID)
    {
        akprintf(C3, M_DRVSYS, "spi_master_read(): allocate L2 buffer failed,  rx=%d\n", s_tSpi[spi_id].ucRxBufferID);
        SPI_DRV_UNPROTECT(spi_id);       
        return false;
    }
    //prepare spi read
    set_rx(spi_id);
    //initial slaver time out max time cycle
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RTIM) = 0xffff;
    //config rx buf in L2
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = ((1<<0)|(1<<16));
    //set the spi send data_cnt
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;   
 
    if(count >= 64)
    {

        l2_combuf_dma((unsigned long)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, false);    
        //wait spi trans finish
        while (1)
        { 
            if(0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH )
                break;
        }
        //if 4bytes misaligned,wait timeout is necessary
        if (0 != (count & 3) )
        {
            while (1)
            { 
                if((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_SLAVER_TIMEOUT) == SPI_SLAVER_TIMEOUT)
                    break;
            }
        }
            
        //wait L2 dma finish, if need frac dma,start frac dma
        if (false == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_slave_read(): l2 dma timeout\n");
            ret =  false;
        }
    }
    else
    {
        //wait spi trans finish
        while (1)
        { 
            if(0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH )
                break;
        }
        //if 4bytes misaligned,wait timeout is necessary
        if (0 != (count & 3) )
        {
            while (1)
            { 
                if((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_SLAVER_TIMEOUT) == SPI_SLAVER_TIMEOUT)
                    break;
            }
        }
        //start frac dma
        l2_combuf_dma((unsigned long)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, false);
        l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID);
    }
    //disable l2 dma
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;  
    l2_clr_status(s_tSpi[spi_id].ucRxBufferID);
    //free L2 buffer
    l2_free(s_tSpi[spi_id].ucL2Rx);   
    SPI_DRV_UNPROTECT(spi_id);       
    return ret;

}

/**
 * @brief spi slave write
 *
 * this func must be called in spi  slave mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @return bool
 * @retval true:  spi write successfully.
 * @retval false: spi write failed.
 */
bool spi_slave_write(T_eSPI_ID spi_id, unsigned char *buf, unsigned long count)
{
    unsigned long state;
    bool ret = true;
    
    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_slave_write(): SPI not initialized!\n");
        return false;
    }

    SPI_DRV_PROTECT(spi_id);
    //select the spi L2 buffer
    s_tSpi[spi_id].ucTxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Tx);
    if(BUF_NULL == s_tSpi[spi_id].ucTxBufferID)
    {
        akprintf(C3, M_DRVSYS, "alloc L2 buffer failed!\n");  
        SPI_DRV_UNPROTECT(spi_id);
        return false;
    }
    //prepare spi
    set_tx(spi_id);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;
    
    if((unsigned long)buf & 0x3)
    {
        l2_combuf_cpu((unsigned long)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF);
    }
    else
    {
        //start l2 dma transmit
        l2_combuf_dma((unsigned long)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF, false);
        if (false == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucTxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_slave_write(): l2 dma timeout\n");
            ret =  false;
            goto EXIT;
        }        
    }
    //wait spi trans finish
    while (1)
    {      
        if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
            break;
    }
        
EXIT:
    //disable l2 dma
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
    l2_clr_status(s_tSpi[spi_id].ucTxBufferID);
    //free L2 buffer
    l2_free(s_tSpi[spi_id].ucL2Tx);   
    SPI_DRV_UNPROTECT(spi_id);       
    return ret;
   
}


/**
 * @brief Initialize SPI
 *
 * this func must be called before call any other SPI functions
 * @author 
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param mode[in] spi mode selected 
 * @param role[in] master or slave
 * @param clk_div[in] SPI working frequency = ASICCLK/(2*(clk_div+1)
 * @return bool
 * @retval true: Successfully initialized SPI.
 * @retval false: Initializing SPI failed.
 */
bool spi_dma_set_callback(T_eSPI_ID spi_id, T_fL2_CALLBACK rx_setcb,T_fL2_CALLBACK tx_setcb)
{
    
    if (spi_id > SPI_ID1 )
    {
        akprintf(C1, M_DRVSYS, "spi_dma_set_callback failed!\n");
        return false;
    }
    
	if(NULL != tx_setcb)
	{
	    s_tSpi[spi_id].ucTxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Tx);
	    if(BUF_NULL == s_tSpi[spi_id].ucTxBufferID)
	    {
	        akprintf(C3, M_DRVSYS, "alloc SPI Tx L2 buffer failed!\n");  
	        return false;
	    }
	}
	
	if(NULL != rx_setcb)
	{
	    s_tSpi[spi_id].ucRxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Rx);
	    if(BUF_NULL == s_tSpi[spi_id].ucRxBufferID)
	    {
			l2_free(s_tSpi[spi_id].ucTxBufferID);//	 
			s_tSpi[spi_id].ucTxBufferID = BUF_NULL;
			akprintf(C3, M_DRVSYS, "alloc SPI Rx L2 buffer failed!\n");  
	        return false;
	    }
	}

    //set L2 callback function
    l2_set_dma_callback (s_tSpi[spi_id].ucTxBufferID, tx_setcb);
    //set L2 callback function
    l2_set_dma_callback (s_tSpi[spi_id].ucRxBufferID, rx_setcb);

    akprintf(C3, M_DRVSYS, "SPI DMA set call back ok!\n");
    return true;
}



/**
 * @brief spi slave write
 *
 * this func must be called in spi  slave mode
 * @author 
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @return bool
 * @retval true:  spi write successfully.
 * @retval false: spi write failed.
 */
bool spi_dma_int_write(T_eSPI_ID spi_id, unsigned char *buf,unsigned long count)
{

#if 1
#define GPIO_SPI_CC3200				23		//39E请求CC3200 SPI数据传输引脚
	
{
    unsigned long state;
    bool ret = true;

	
    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_slave_write(): SPI not initialized!\n");
        return false;
    }

    SPI_DRV_PROTECT(spi_id);
    //select the spi L2 buffer
    s_tSpi[spi_id].ucTxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Tx);
    if(BUF_NULL == s_tSpi[spi_id].ucTxBufferID)
    {
        akprintf(C3, M_DRVSYS, "alloc L2 buffer failed!\n");  
        SPI_DRV_UNPROTECT(spi_id);
        return false;
    }
    //prepare spi
    set_tx(spi_id);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;
    
    if((unsigned long)buf & 0x3)
    {
		akprintf(C1, M_DRVSYS, "spi_slave_write(): l2_combuf_cpu\n");
		gpio_set_pin_level(GPIO_SPI_CC3200, 1);	//GPIO 高电平使能，请求数据传输

		l2_combuf_cpu((unsigned long)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF);
    }
    else
    {
        //start l2 dma transmit
        l2_combuf_dma((unsigned long)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF, false);
		gpio_set_pin_level(GPIO_SPI_CC3200, 1);	//GPIO 高电平使能，请求数据传输

        if (false == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucTxBufferID))
        {
			gpio_set_pin_level(GPIO_SPI_CC3200,0);	//GPIO 高电平使能，请求数据传输
			akprintf(C1, M_DRVSYS, "spi_slave_write(): l2 dma timeout\n");
            ret =  false;
            goto EXIT;
        }        
    }


	
    //wait spi trans finish
    while (1)
    {      
        if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
            break;
    }
        
EXIT:
    //disable l2 dma
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
    l2_clr_status(s_tSpi[spi_id].ucTxBufferID);
    //free L2 buffer
    l2_free(s_tSpi[spi_id].ucL2Tx);   
    SPI_DRV_UNPROTECT(spi_id);
	
	gpio_set_pin_level(GPIO_SPI_CC3200,0);	//GPIO 高电平使能，请求数据传输

    return ret;
   
}

#else

    //spi_time_count = get_tick_count_us();
    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_slave_write(): SPI not initialized!\n");
        return false;
    }
		
    //prepare spi
    set_tx(spi_id);
    //set_rx_tx(spi_id);
    
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;
    
    if((unsigned long)buf & 0x3)
    {
		l2_combuf_cpu((unsigned long)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF);
    }
    else
    {
		//start l2 dma transmit
        l2_combuf_dma((unsigned long)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF, false);

		gpio_set_pin_level(SPI_REQUEST_GPIO, 1);	//GPIO 高电平使能，请求数据传输

	    if (false == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucTxBufferID))
	    {
	        akprintf(C1, M_DRVSYS, "spi_slave_write(): l2 dma timeout\n");
	    } 

	}
      
    //wait spi trans finish
    while (1)
    {      
        if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
            break;
    }

	gpio_set_pin_level(SPI_REQUEST_GPIO, 0);	//GPIO 高电平使能，请求数据传输

	
    return true;

#endif
   
}


/**
 * @brief spi slave write
 *
 * this func must be called in spi  slave mode
 * @author 
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @return bool
 * @retval true:  spi write successfully.
 * @retval false: spi write failed.
 */
bool spi_dma_int_read(T_eSPI_ID spi_id, unsigned char *buf,unsigned long count)
{
	//TBD
}



/**
 * @brief close SPI
 *
 * @author 
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @return void
 */
void spi_dma_int_close(T_eSPI_ID spi_id)
{

	if(BUF_NULL != s_tSpi[spi_id].ucTxBufferID)
	{
	    //disable l2 dma
	    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
	    l2_clr_status(s_tSpi[spi_id].ucTxBufferID);
	    //free L2 buffer
	    l2_free(s_tSpi[spi_id].ucL2Tx);   

	}	
	
	if(BUF_NULL != s_tSpi[spi_id].ucRxBufferID)
	{
	    //disable l2 dma
	    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;  
	    l2_clr_status(s_tSpi[spi_id].ucRxBufferID);
	    //free L2 buffer
	    l2_free(s_tSpi[spi_id].ucL2Rx);   
	}

}


static int spi_reg(void)
{
    freq_register(E_ASIC_CALLBACK, spi_on_change);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(spi_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

