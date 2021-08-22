/**
 * @file spi_flash.c
 * @brief spiflash driver source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_module.h"

#define SPI_FLASH_COM_RDID        0x9f
#define SPI_FLASH_COM_READ        0x03
#define SPI_FLASH_COM_PROGRAM     0x02
#define SPI_FLASH_COM_WR_EN       0x06

#define SPI_FLASH_COM_ERASE       0xd8 //块擦除  256组 64K  256个
#define SPI_FLASH_Block_ERASE_32k 0x52 //块擦除  512组 32K  512个
#define SPI_FLASH_CHIP_ERASE	  0xC7 //全片擦除 16M 

#define SPI_FLASH_COM_RD_STUS1    0x05
#define SPI_FLASH_COM_RD_STUS2    0x35
#define SPI_FLASH_COM_RD_STUS3    0x15


#define SPI_FLASH_COM_WRSR        0x01
#define SPI_FLASH_COM_WRSR2       0x31 //gd spiflash support
#define SPI_FLASH_COM_WRSR3       0x11

#define SPI_FLASH_COM_ERASE_SEC   0x20 //擦除4KB sector //扇区擦除 16组  4K 4096个

//spi flash status3 ADP bit
#define SPI_FLASH_COM_STUS3_ADP   (1<<17)

//aaai command
#define SPI_FLASH_COM_AAI         0xad
#define SPI_FLASH_COM_WRDI        0x04

//2-wire mode
#define SPI_FLASH_COM_D_READ      0x3B

//4-wire mode
#define SPI_FLASH_COM_Q_READ      0x6B
#define SPI_FLASH_COM_Q_WRITE     0x32

// 4 byte address mode
#define SPI_FLASH_4_B_ADDR_EN     0xb7  
#define SPI_FLASH_4_B_ADDR_EX	  0xe9


//quad enable
#define SPI_FLASH_QUAD_ENABLE    (1 << 9)

//setor size
#define SPI_SECTOR_SIZE           256

//max read size
#define MAX_RD_SIZE              (32 * 1024)

//extern flag bit0 and bit1
#define SFLASH_EX_FLAG_ONE_STATUS    0x00
#define SFLASH_EX_FLAG_TWO_STATUS    0x01
#define SFLASH_EX_FLAG_THREE_STATUS  0x02
#define SFLASH_EX_FLAG_FOUR_STATUS   0x03   //reserved

//max page size
#define SPI_FLASH_PAGE_SIZE_MAX		0xffffffff

//1M size
#define ONE_M_SIZE					(1024*1024) 



static bool spi_flash_enter_4byte_addr_mode(void);
static bool spi_flash_exit_4byte_addr_mode(void);
bool spi_flash_read_status(unsigned long *status);
bool spi_flash_write_status(unsigned long status);
static void write_enable(void);
static bool check_status_done(void);
static bool write_page(unsigned long page, unsigned char *buf);
static bool flash_read_standard(unsigned long page, unsigned char *buf, unsigned long page_cnt);
static bool flash_read_quad(unsigned long page, unsigned char *buf, unsigned long page_cnt);
static bool write_page_aai(unsigned long page, unsigned char *buf);
static bool flash_write_standard(unsigned long page, unsigned char *buf, unsigned long page_cnt);
static bool flash_write_quad(unsigned long page, unsigned char *buf, unsigned long page_cnt);
static bool write_page_quad(unsigned long page, unsigned char *buf);
static bool flash_read_dual(unsigned long page, unsigned char *buf, unsigned long page_cnt);
static bool flash_QE_set(bool enable);



typedef struct
{
    bool bInit;

    T_eSPI_ID spi_id;

    T_eSPI_BUS bus_wight;

    T_SFLASH_PARAM param;
}
T_SPI_FLASH;

static T_SPI_FLASH m_sflash = {0};

/**
 * @brief spi flash init
 * 
 * @author liao_zhijun
 * @param spi_id choose spi0 or spi1
 * @param bus_width choose bus width: 1 line/2 line/4 line
 * @date 2010-04-29
 * @return bool
 */
bool spi_flash_init(T_eSPI_ID spi_id, T_eSPI_BUS bus_wight)
{
    //set clock to 25M, use mode0, coz some spi flash may have data error in mode3
    if(!spi_init(spi_id, SPI_MODE0, SPI_MASTER, 10*1000*1000))
    {
        return false;
    }

    m_sflash.spi_id = spi_id;
    m_sflash.bus_wight = bus_wight;
    m_sflash.bInit = true;

    return true;
}

/**
 * @brief set/clear quad enable
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param enable [in] true: enable QE, false: disable QE
 * @return bool
 */
static bool flash_QE_set(bool enable)
{
    unsigned long status;

    //read status 
    if (!spi_flash_read_status(&status))
    {
        akprintf(C1, M_DRVSYS, "spi QE read status fail\r\n");
        return false;
    }

    if (enable)
    {
        //set quad enable
        status = status | SPI_FLASH_QUAD_ENABLE;
    }
    else
    {
        //clear quad enable
        status = status & (~SPI_FLASH_QUAD_ENABLE);
    }

    //write status
    if (!spi_flash_write_status(status & 0xffffff))
    {
        akprintf(C1, M_DRVSYS, "spi QE: write status fail \r\n");
        return false;
    }

    return true;
}

/**
 * @brief set param of serial flash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param sflash_param [in]  serial flash param 
 * @return void
 */
void spi_flash_set_param(T_SFLASH_PARAM *sflash_param)
{
    unsigned long status;

    if(NULL == sflash_param)
        return;

    //save param
    memcpy(&m_sflash.param, sflash_param, sizeof(T_SFLASH_PARAM));

    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    //setup clock
    if(sflash_param->clock > 0)
    {
        spi_init(m_sflash.spi_id, SPI_MODE3, SPI_MASTER, sflash_param->clock);
    }

    //unmask protect
    if(sflash_param->flag & SFLASH_FLAG_UNDER_PROTECT)
    {
        spi_flash_read_status(&status);

        status &= ~sflash_param->protect_mask;

        spi_flash_write_status(status & 0xffffff);
    }

    //enable quad
    if((sflash_param->flag & SFLASH_FLAG_QUAD_WRITE) \
        || (sflash_param->flag & SFLASH_FLAG_QUAD_READ))
    {
        if (SPI_BUS4 == m_sflash.bus_wight)
        {
            if (!flash_QE_set(true))
            {
                akprintf(C1, M_DRVSYS, "warnning: enable QE fail, will set to one-wire mode\n");
                m_sflash.bus_wight = SPI_BUS1;
            }
            else
            {
                akprintf(C1, M_DRVSYS, "Set QE OK\n");
            }
        }
    }
  
    //enter spi flash 4 byte addressing 
    
    if (m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR)
    {
		if ( spi_flash_enter_4byte_addr_mode())
		{
			akprintf(C1, M_DRVSYS, "enter spi flash 4byte addressing success\n");
		}
	    else
		{
			akprintf(C1, M_DRVSYS, "enter spi flash 4byte addressing fail\n");
	    }
	}
	
    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);

}
/**
 * @brief get spiflash id
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return unsigned long
 * @retval unsigned long spiflash id
 */
unsigned long spi_flash_getid(void)
{
    unsigned char buf1[1];
    unsigned long flash_id = 0;
	

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return 0;
    }

    buf1[0] = SPI_FLASH_COM_RDID;

    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    //get id
    spi_master_write(m_sflash.spi_id, buf1, 1, false);
    spi_master_read(m_sflash.spi_id, (unsigned char *)(&flash_id), 3, true);

    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);
    
    akprintf(C1, M_DRVSYS, "the manufacture id is: %x\n", flash_id);

    return flash_id;
}

/**
 * @brief read data from one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to store read data 
 * @param page_cnt [in]  the page count to read  
 * @return bool
 */
bool spi_flash_read(unsigned long page, unsigned char *buf, unsigned long page_cnt)
{
    bool ret = false;

    spi_set_protect(m_sflash.spi_id, m_sflash.bus_wight);
    
    if ((SPI_BUS4 == m_sflash.bus_wight) && (m_sflash.param.flag & SFLASH_FLAG_QUAD_READ))
    {
        //line 4
        ret = flash_read_quad(page, buf, page_cnt); 
		
    }
    else if ((SPI_BUS2 == m_sflash.bus_wight) && ((m_sflash.param.flag & SFLASH_FLAG_DUAL_READ)))
    {
        //line 2
        ret =  flash_read_dual(page, buf, page_cnt);
		
    }
    else
    {
        //line 1
        ret = flash_read_standard(page, buf, page_cnt);
		
    }

    spi_set_unprotect(m_sflash.spi_id, m_sflash.bus_wight);

    return ret;
}


/**
 * @brief write data to one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to be write
  * @param page_cnt [in]  the page count to write  
* @return bool
 */
bool spi_flash_write(unsigned long page, unsigned char *buf, unsigned long page_cnt)
{
    bool ret = false;

    spi_set_protect(m_sflash.spi_id, m_sflash.bus_wight);
        
    if ((SPI_BUS4 == m_sflash.bus_wight) && (m_sflash.param.flag & SFLASH_FLAG_QUAD_WRITE))
    {
        //line 4
        ret = flash_write_quad(page, buf, page_cnt);
    }
    else
    {
        //line 1
        ret = flash_write_standard(page, buf, page_cnt);
		
    }

    spi_set_unprotect(m_sflash.spi_id, m_sflash.bus_wight);

    return ret;
}


#define SPIFLASH_ERASE_UNIT (4 * 1024)
/**
 * @brief erase one sector(4K) of spiflash 
 * 
 * @author  kejianping
 * @date 2014-09-25
 * @param addr [in]  sector  number
 * @return bool
 */
bool spi_flash_erase_sector(unsigned long sector)

{
    unsigned char buf1[5];
    unsigned long addr = sector * SPIFLASH_ERASE_UNIT;
    bool ret = false;

	if( (addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (sector > (m_sflash.param.total_size/SPIFLASH_ERASE_UNIT)))
  	{
		akprintf(C1, M_DRVSYS, "erase sector page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return 0;
    }
    spi_set_protect(m_sflash.spi_id, SPI_BUS1);
    write_enable();

  	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
  	{
  		buf1[0] = SPI_FLASH_COM_ERASE_SEC;//每次擦写只是擦除4K的大小
    	buf1[1] = (addr >> 16) & 0xFF;
    	buf1[2] = (addr >> 8)  & 0xFF;
    	buf1[3] = (addr >> 0)  & 0xFF;
    	//send erase cmd
    	if(!spi_master_write(m_sflash.spi_id, buf1, 4, true))
    	{
			goto ERASE_EXIT;
    	}
  	}
  	else
  	{
    	buf1[0] = SPI_FLASH_COM_ERASE_SEC;//每次擦写只是擦除4K的大小
    	buf1[1] = (addr >> 24) & 0xFF;
		buf1[2] = (addr >> 16) & 0xFF;
    	buf1[3] = (addr >> 8)  & 0xFF;
    	buf1[4] = (addr >> 0)  & 0xFF;
    	//send erase cmd
    	if(!spi_master_write(m_sflash.spi_id, buf1, 5, true))
    	{
			goto ERASE_EXIT;
    	}
    }
    
    if(!check_status_done())
    {
		goto ERASE_EXIT;
    }
    
    ret = true;

ERASE_EXIT:    
    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);
    return ret;
}



/**
 * @brief erase one block(64k) of spiflash 
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  block number
 * @return bool
 */
 
bool spi_flash_erase(unsigned long block)
{
    unsigned char buf1[5];
	#define ERASE_BLOCK_SIZE  (64*1024)
    unsigned long addr = block * ERASE_BLOCK_SIZE;
    bool ret = false;

    if( (addr > (m_sflash.param.total_size - m_sflash.param.page_size))||(block > (m_sflash.param.total_size/m_sflash.param.erase_size)))
  	{
		akprintf(C1, M_DRVSYS, "erase flash page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n", 0, true);
        return false;
    }
    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    //enable write
    write_enable();
    if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
    {
    	buf1[0] = SPI_FLASH_COM_ERASE;
    	buf1[1] = (addr >> 16) & 0xFF;
    	buf1[2] = (addr >> 8) & 0xFF;
    	buf1[3] = (addr >> 0) & 0xFF;

    	//erase
    	if(!spi_master_write(m_sflash.spi_id, buf1, 4, true))
    	{
        	goto ERASE_EXIT;
    	}
    }
	else
	{
    	buf1[0] = SPI_FLASH_COM_ERASE;
    	buf1[1] = (addr >> 24) & 0xFF;
    	buf1[2] = (addr >> 16) & 0xFF;
    	buf1[3] = (addr >> 8) & 0xFF;
    	buf1[4] = (addr >> 0) & 0xFF;
    	if(!spi_master_write(m_sflash.spi_id, buf1, 5, true))
    	{
        	goto ERASE_EXIT;
   		}
	}
	
    if(!check_status_done())
    {
        goto ERASE_EXIT;
    }

    ret = true;

ERASE_EXIT:    
    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);
    return ret;
}



/**
 * @brief erase one sector or block of spiflash
 * 
 * @author  kejianping
 * @date 2014-08-29
 * @param addr [in]  erase addr 
 * @param length [in]  erase length 
 * @return bool
 */
bool spi_flash_erase_addr(unsigned long addr, unsigned long length)
{
    unsigned char buf1[5];
    bool ret = false;
	
    if( addr > (m_sflash.param.total_size - m_sflash.param.page_size))
  	{
		akprintf(C1, M_DRVSYS, "erase flash address size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return 0;
    }
    
    spi_set_protect(m_sflash.spi_id, SPI_BUS1);
    write_enable();
	
	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
	{
   		buf1[0] = (length > SPIFLASH_ERASE_UNIT)?(SPI_FLASH_COM_ERASE):(SPI_FLASH_COM_ERASE_SEC);
    	buf1[1] = (addr >> 16) & 0xFF;
    	buf1[2] = (addr >> 8) & 0xFF;
    	buf1[3] = (addr >> 0) & 0xFF;

    	//erase
    	if(!spi_master_write(m_sflash.spi_id, buf1, 4, true))
    	{
			goto ERASE_EXIT;
		}
	}	
	else
	{
    	buf1[0] = (length > SPIFLASH_ERASE_UNIT)?(SPI_FLASH_COM_ERASE):(SPI_FLASH_COM_ERASE_SEC);
    	buf1[1] = (addr >> 24) & 0xFF;
		buf1[2] = (addr >> 16) & 0xFF;
    	buf1[3] = (addr >> 8) & 0xFF;
    	buf1[4] = (addr >> 0) & 0xFF;

    	//erase
    	if(!spi_master_write(m_sflash.spi_id, buf1, 5, true))
    	{
			goto ERASE_EXIT;
		}
	
	}
    
    if(!check_status_done())
    {
		goto ERASE_EXIT;
	}
	
    ret = true;

ERASE_EXIT:    
    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);
    return ret;
}



/**
 * @brief read spi flash status
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param status [out] status buffer
 * @ 0-7 bit to status1 8-15 bit to status2 16-24 bit to status3
 * @return bool
 */
bool spi_flash_read_status(unsigned long *status)
{
    unsigned char buf1[1];
    unsigned char status1 = 0, status2 = 0 , status3 = 0;


     //read status1
   	 buf1[0] = SPI_FLASH_COM_RD_STUS1;
     spi_master_write(m_sflash.spi_id, buf1, 1, false);
     if (!spi_master_read(m_sflash.spi_id, &status1, 1, true))
     	return false;
	 
     //more one status reg
	 if( m_sflash.param.flag & SFLASH_FLAG_COM_STATUS2 )
	 {
		 switch (m_sflash.param.ex_flag & SFLASH_EX_FLAG_STATUS_MASK)
		 {
		 case SFLASH_EX_FLAG_ONE_STATUS:
		 case SFLASH_EX_FLAG_TWO_STATUS:
		 	
		 	//read status 2
	        buf1[0] = SPI_FLASH_COM_RD_STUS2;
	        spi_master_write(m_sflash.spi_id, buf1, 1, false);
	        if(!spi_master_read(m_sflash.spi_id, &status2, 1, true))
	            return false;
		    break;
			
		 case SFLASH_EX_FLAG_THREE_STATUS:
		 	//read status 2
	        buf1[0] = SPI_FLASH_COM_RD_STUS2;
	        spi_master_write(m_sflash.spi_id, buf1, 1, false);
	        if(!spi_master_read(m_sflash.spi_id, &status2, 1, true))
	            return false;
			//read status3
		 	buf1[0] = SPI_FLASH_COM_RD_STUS3;
	     	spi_master_write(m_sflash.spi_id, buf1, 1, false);
	     	if (!spi_master_read(m_sflash.spi_id, &status3, 1, true))
	        	return false;
		    break;
			
		 case SFLASH_EX_FLAG_FOUR_STATUS:
		 	akprintf(C1, M_DRVSYS, "there are not four status reg to read!\r\n");
	        return false;
			
		    break;

		 }
		 
	 }
	 
    *status = (status1 | (status2 << 8)| (status3 << 16)) & 0x00ffffff;
     return true;
}

/**
*@
*@NOTE: 0-7 bit to status1; 8-15 bit to status2 ;16-24 bit to status3
*@
*/
bool spi_flash_write_status(unsigned long status)
{
	
    unsigned char buf[4],buf2[2],buf3[2];
    unsigned long write_cnt;


    buf[0] = SPI_FLASH_COM_WRSR;
    buf[1] = status & 0xff;
    buf[2] = (status >> 8) & 0xff;
	buf[3] = (status >> 16) & 0xff;

    //more than one status reg
	if(m_sflash.param.flag & SFLASH_FLAG_COM_STATUS2 )
    {
    	
    	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_STATUS_COM_W_MASK))
      	{
	  		switch(m_sflash.param.ex_flag & SFLASH_EX_FLAG_STATUS_MASK)
	  		{
	  		case SFLASH_EX_FLAG_ONE_STATUS:
			case SFLASH_EX_FLAG_TWO_STATUS:
			 //lower vision flash write status
				 write_enable();
				 if (!spi_master_write(m_sflash.spi_id, buf, 3, true))	//common write status reg
					return false;
			 	 if (!check_status_done())  
			 		return false;
				 
					break;
	
			case SFLASH_EX_FLAG_THREE_STATUS:  //reserved
			  	akprintf(C3, M_DRVSYS, "there are not three status reg common to write!\r\n");
			  	return false;
			  	
			 	 break;
			  
			case SFLASH_EX_FLAG_FOUR_STATUS :  //reserved
			  	akprintf(C3, M_DRVSYS, "there are not four status reg common to write!\r\n");
              	return false;
			  
			  	break;
		  
		    }
			
			
        }
		else
		{
		
			switch(m_sflash.param.ex_flag & SFLASH_EX_FLAG_STATUS_MASK)
			{
			//two status
			case SFLASH_EX_FLAG_TWO_STATUS:
					//write status1 reg
				write_enable();
				if (!spi_master_write(m_sflash.spi_id, buf, 2, true)) // Write C-version flash status Reg. 1
				     return  false;
				if (!check_status_done())  
					return false;
					
				    //write status2 reg         
				 write_enable();
			     buf2[0] = SPI_FLASH_COM_WRSR2;
				 buf2[1] = buf[2];
				 if (!spi_master_write(m_sflash.spi_id, buf2, 2, true))//Write C-version flash status Reg.2
					return false;
				 if (!check_status_done())  
					return false;
				 break;
			//three status
			case SFLASH_EX_FLAG_THREE_STATUS:
					//write status1 reg
				write_enable();
				if (!spi_master_write(m_sflash.spi_id, buf, 2, true)) // Write C-version flash status Reg. 1
				    return  false;
				if (!check_status_done())  
					return false;
					
				    //write status2 reg         
				 write_enable();
				 buf2[0] = SPI_FLASH_COM_WRSR2;
				 buf2[1] = buf[2];
				 if (!spi_master_write(m_sflash.spi_id, buf2, 2, true))//Write C-version flash status Reg.2
					return false;
				 if (!check_status_done())  
					return false;

					//write status3 reg
		          buf3[0] = SPI_FLASH_COM_WRSR3;
			 	  buf3[1] = buf[3];
			 	  write_enable();
			 	 if (!spi_master_write(m_sflash.spi_id, buf3, 2, true))
			   		return false;
			 
		         if (!check_status_done()) 
			        return false;
				 break;
			
			case SFLASH_EX_FLAG_FOUR_STATUS :  //reserved
				akprintf(C3, M_DRVSYS, "there are not four status reg!\r\n");
	        	return false;
				
				break;
			default:
				akprintf(C3, M_DRVSYS, "this are not divide to write bits!\r\n");
				return false;
				break;
	    	}
			
		
		 }

		 
    }
	else
	{   
	    //only one status
	 	//write status1 reg
     	write_cnt = 2;
		write_enable();
	 	if(!spi_master_write(m_sflash.spi_id, buf, write_cnt, true))
	 		return false;
	 	if (!check_status_done()) 
			return false;
	}
	

    return true;
}



static bool flash_read_standard(unsigned long page, unsigned char *buf, unsigned long page_cnt)
{
    unsigned char buf1[5];
    unsigned long i, addr = page * m_sflash.param.page_size;
    bool bReleaseCS=false;
    unsigned long count, readlen;
	
  	if( (addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (page > (m_sflash.param.total_size/m_sflash.param.page_size)))
  	{
		akprintf(C1, M_DRVSYS, "read standard page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    if((page > SPI_FLASH_PAGE_SIZE_MAX) || (NULL == buf))
    {
        akprintf(C1, M_DRVSYS, "spi_flash_read: param error\r\n");
        return false;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return false;
    }
	
	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
	{

    	buf1[0] = SPI_FLASH_COM_READ;
    	buf1[1] = (addr >> 16) & 0xFF;
    	buf1[2] = (addr >> 8) & 0xFF;
    	buf1[3] = (addr >> 0) & 0xFF;
    	spi_master_write(m_sflash.spi_id, buf1, 4, false);
	}
	else
	{
	
		buf1[0] = SPI_FLASH_COM_READ;
    	buf1[1] = (addr >> 24) & 0xFF;
  		buf1[2] = (addr >> 16) & 0xFF;
  		buf1[3] = (addr >> 8) & 0xFF;
  		buf1[4] = (addr >> 0) & 0xFF;
		spi_master_write(m_sflash.spi_id, buf1, 5, false);
	}
	
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? false: true;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
            return false;
        buf += readlen;        
    }
    
    return true;
}

static bool flash_read_dual(unsigned long page, unsigned char *buf, unsigned long page_cnt)
{
    unsigned char buf_cmd[6];
    unsigned short status;
    unsigned long addr = page * m_sflash.param.page_size;
    unsigned long count;
    unsigned long readlen;
    bool bReleaseCS=false;

   	if( (addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (page > (m_sflash.param.total_size/m_sflash.param.page_size)))
  	{
		akprintf(C1, M_DRVSYS, "read dual page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    if((page > SPI_FLASH_PAGE_SIZE_MAX) || (NULL == buf))
    {
        akprintf(C1, M_DRVSYS, "spi_flash_read: param error\r\n");
        return false;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return false;
    }

	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
	{
   		buf_cmd[0] = SPI_FLASH_COM_D_READ;
    	buf_cmd[1] = (unsigned char)((addr >> 16) & 0xFF);
    	buf_cmd[2] = (unsigned char)((addr >> 8) & 0xFF);
    	buf_cmd[3] = (unsigned char)(addr & 0xFF);
    	buf_cmd[4] = 0;
	
		if (!spi_master_write(m_sflash.spi_id, buf_cmd, 5, false))
	  	{
		  	akprintf(C1, M_DRVSYS, "spi flash_read_quad: write cmd fail\r\n");
		  	return false;
	  	}
	}
	else
	{
    	buf_cmd[0] = SPI_FLASH_COM_D_READ;
    	buf_cmd[1] = (unsigned char)((addr >> 24) & 0xFF);
    	buf_cmd[2] = (unsigned char)((addr >> 16) & 0xFF);
    	buf_cmd[3] = (unsigned char)((addr >> 8) & 0xFF);
    	buf_cmd[4] = (unsigned char)(addr & 0xFF);
    	buf_cmd[5] = 0;
		// cmd
		if (!spi_master_write(m_sflash.spi_id, buf_cmd, 6, false))
	  	{
		  	akprintf(C1, M_DRVSYS, "spi flash_read_quad: write cmd fail\r\n");
		  	return false;
	  	}	
	}
   
    // set spi 2-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS2);

    // read
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? false: true;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
        {
            // set spi 1-wire
            spi_data_mode(m_sflash.spi_id, SPI_BUS1);
            return false;
        }
        buf += readlen;        
    }
    
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

    return true;
}

static bool flash_read_quad(unsigned long page, unsigned char *buf, unsigned long page_cnt)
{
    unsigned char buf_cmd[6];
    unsigned short status;
    unsigned long addr = page * m_sflash.param.page_size;
    unsigned long count;
    unsigned long readlen;
    bool bReleaseCS=false;
    bool ret = true;

	if( (addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (page > (m_sflash.param.total_size/m_sflash.param.page_size)))
  	{
		akprintf(C1, M_DRVSYS, "read quad page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    if((page > SPI_FLASH_PAGE_SIZE_MAX) || (NULL == buf))
    {
        akprintf(C1, M_DRVSYS, "spi_flash_read: param error\r\n");
        return false;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return false;
    }

    // cmd
	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
	{
    	buf_cmd[0] = SPI_FLASH_COM_Q_READ;
    	buf_cmd[1] = (unsigned char)((addr >> 16) & 0xFF);
    	buf_cmd[2] = (unsigned char)((addr >> 8) & 0xFF);
    	buf_cmd[3] = (unsigned char)(addr & 0xFF);
    	buf_cmd[4] = 0;
		
		if (!spi_master_write(m_sflash.spi_id, buf_cmd, 5, false))
	   	{
		   	akprintf(C1, M_DRVSYS, "spi flash_read_quad: write cmd fail\r\n");
		   	goto EXIT;
	   	}
	}
	else
	{
		buf_cmd[0] = SPI_FLASH_COM_Q_READ;
    	buf_cmd[1] = (unsigned char)((addr >> 24) & 0xFF);
  		buf_cmd[2] = (unsigned char)((addr >> 16) & 0xFF);
  		buf_cmd[3] = (unsigned char)((addr >> 8) & 0xFF);
  		buf_cmd[4] = (unsigned char)(addr & 0xFF);
  		buf_cmd[5] = 0;
	
    	if (!spi_master_write(m_sflash.spi_id, buf_cmd, 6, false))
    	{
        	akprintf(C1, M_DRVSYS, "spi flash_read_quad: write cmd fail\r\n");
        	goto EXIT;
    	}
	
	}
	
    // set spi 4-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS4);

    // read
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? false: true;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
        {
            ret = false;
            goto EXIT;
        }
        buf += readlen;        
    }

EXIT:   
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

    return ret;
}

static bool flash_write_standard(unsigned long page, unsigned char *buf, unsigned long page_cnt)
{
    unsigned char buf1[4];
    unsigned long i;
    
    if((page > SPI_FLASH_PAGE_SIZE_MAX) || (NULL == buf))
    {
        akprintf(C1, M_DRVSYS, "spi_flash_write: param error\r\n");
        return false;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return false;
    }

    for(i = 0; i < page_cnt; i++)
    {
        if(m_sflash.param.flag & SFLASH_FLAG_AAAI)
        {
            //auto address increment word program
            if(!write_page_aai(page + i, buf + i*m_sflash.param.page_size))
            {
                akprintf(C1, M_DRVSYS, "write aai fail: %d\n");
                return false;
            }
        }
        else
        {   
            //regular page write
            if(!write_page(page + i, buf + i*m_sflash.param.page_size))
            {
                akprintf(C1, M_DRVSYS, "write page fail: %d\n");
                return false;
            }
        }
    }

    return true;
}

static bool flash_write_quad(unsigned long page, unsigned char *buf, unsigned long page_cnt)
{
    unsigned long i;

	
    if((page > SPI_FLASH_PAGE_SIZE_MAX) || (NULL == buf))
    {
        akprintf(C1, M_DRVSYS, "spi_flash_write: param error\r\n");
        return false;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return false;
    }
    
    // write data
    for(i = 0; i < page_cnt; i++)
    {
        //quad page write
        if(!write_page_quad(page + i, buf + i*m_sflash.param.page_size))
        {
            akprintf(C1, M_DRVSYS, "write page quad fail: %d\n", page+i);
            return false;
        }
    }

    return true;
}

/**
 * @brief write data to one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to be write
 * @return bool
 */
static bool write_page(unsigned long page, unsigned char *buf)
{
    unsigned char buf1[5];
    unsigned long addr = page * m_sflash.param.page_size;
    unsigned long i;

	if(( addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (page > (m_sflash.param.total_size/m_sflash.param.page_size)))
  	{
		akprintf(C1, M_DRVSYS, "write page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    for(i = 0; i < m_sflash.param.page_size / m_sflash.param.program_size; i++)
    {
        write_enable();
	
	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
	{
        buf1[0] = SPI_FLASH_COM_PROGRAM;
        buf1[1] = (addr >> 16) & 0xFF;
        buf1[2] = (addr >> 8) & 0xFF;
        buf1[3] = (addr >> 0) & 0xFF;
		spi_master_write(m_sflash.spi_id, buf1, 4, false);
		}
	else
	{
        buf1[0] = SPI_FLASH_COM_PROGRAM;
        buf1[1] = (addr >> 24) & 0xFF;
        buf1[2] = (addr >> 16) & 0xFF;
        buf1[3] = (addr >> 8) & 0xFF;
        buf1[4] = (addr >> 0) & 0xFF;
        spi_master_write(m_sflash.spi_id, buf1, 5, false);
	}

        if(!spi_master_write(m_sflash.spi_id, buf + i*m_sflash.param.program_size, m_sflash.param.program_size, true))
            return false;
        
        if(!check_status_done())
            return false;

        addr += m_sflash.param.program_size;
    }
    
    return true;
}

static bool write_page_aai(unsigned long page, unsigned char *buf)
{
    unsigned char buf1[6];
    unsigned long addr = page * m_sflash.param.page_size;
    unsigned long i;

	
    if(( addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (page > (m_sflash.param.total_size/m_sflash.param.page_size)))
  	{
		akprintf(C1, M_DRVSYS, "write aai page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    write_enable();


    buf1[0] = SPI_FLASH_COM_AAI;
    buf1[1] = (addr >> 16) & 0xFF;
    buf1[2] = (addr >> 8) & 0xFF;
    buf1[3] = (addr >> 0) & 0xFF;
    buf1[4] = buf[0];
    buf1[5] = buf[1];
    spi_master_write(m_sflash.spi_id, buf1, 6, true);


    if(!check_status_done())
        return false;

    for (i = 2; i < 256; i+=2)
    {
        buf1[1] = buf[i];
        buf1[2] = buf[i+1];

        spi_master_write(m_sflash.spi_id, buf1, 3, true);

        if(!check_status_done())
            return false;
    }

    buf1[0] = SPI_FLASH_COM_WRDI;
    spi_master_write(m_sflash.spi_id, buf1, 1, true);
    return true;
}

static bool write_page_quad(unsigned long page, unsigned char *buf)
{
    unsigned char buf_cmd[5];
    unsigned long addr = page * m_sflash.param.page_size;
    unsigned long i;
    unsigned short status;
	
   if( (addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (page > (m_sflash.param.total_size/m_sflash.param.page_size)))
  	{
		akprintf(C1, M_DRVSYS, "write quad page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return false;	
	}
	
    for(i = 0; i < m_sflash.param.page_size / m_sflash.param.program_size; i++)
    {
    
        write_enable();
		if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
		{

        	buf_cmd[0] = SPI_FLASH_COM_Q_WRITE;
        	buf_cmd[1] = (unsigned char)((addr >> 16) & 0xFF);
        	buf_cmd[2] = (unsigned char)((addr >> 8) & 0xFF);
        	buf_cmd[3] = (unsigned char)(addr & 0xFF);
			spi_master_write(m_sflash.spi_id, buf_cmd, 4, false);
		}
		else
		{	
     		buf_cmd[0] = SPI_FLASH_COM_Q_WRITE;
     		buf_cmd[1] = (unsigned char)((addr >> 24) & 0xFF);
	 		buf_cmd[2] = (unsigned char)((addr >> 16) & 0xFF);
			buf_cmd[3] = (unsigned char)((addr >> 8) & 0xFF);
	 		buf_cmd[4] = (unsigned char)(addr & 0xFF);
        	spi_master_write(m_sflash.spi_id, buf_cmd, 5, false);
		}
		

        // set spi 4-wire
        spi_data_mode(m_sflash.spi_id, SPI_BUS4);

        if(!spi_master_write(m_sflash.spi_id, buf + i*m_sflash.param.program_size, m_sflash.param.program_size, true))
        {
            akprintf(C1, M_DRVSYS, "write page quad write data fail: ");
        }
        
        // set spi 1-wire
        spi_data_mode(m_sflash.spi_id, SPI_BUS1);
        
        if(!check_status_done())
        {
            akprintf(C1, M_DRVSYS, "write page quad check done fail: ");
            return false;
        }

        addr += m_sflash.param.program_size;
    }
    
    return true;
}

static void write_enable(void)
{
    unsigned char buf1[1];
    
    //write enable
    buf1[0] = SPI_FLASH_COM_WR_EN;
    spi_master_write(m_sflash.spi_id, buf1, 1, true);
}

static bool check_status_done(void)
{
    unsigned long timeout = 0;
    unsigned long status = 0;

    while(1)
    {
        spi_flash_read_status(&status);

        if((status & (1 << 0 )) == 0)
            break;

        if(timeout++ > 100000)
        {
            akprintf(C1, M_DRVSYS, "spiflash check_status_done timeout\n");
            return false;
        }
    }

    return true;
}

static bool spi_flash_enter_4byte_addr_mode(void)
{

	unsigned char buf1[1];
    //write enable
    buf1[0] = SPI_FLASH_4_B_ADDR_EN;
    return(spi_master_write(m_sflash.spi_id, buf1, 1, true));

}

static bool spi_flash_exit_4byte_addr_mode(void)
{

	unsigned char buf1[1];
    //write enable
    buf1[0] = SPI_FLASH_4_B_ADDR_EX;
    return(spi_master_write(m_sflash.spi_id, buf1, 1, true));

}

