/**
 * @file spi_flash.c
 * @brief spiflash driver source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#include "anyka_types.h"
#include "hal_spiflash.h"

#define SPI_FLASH_COM_RDID        0x9f
#define SPI_FLASH_COM_READ        0x03
#define SPI_FLASH_COM_PROGRAM     0x02
#define SPI_FLASH_COM_WR_EN       0x06

#define SPI_FLASH_COM_ERASE       0xd8 //¿é²Á³ý  256×é 64K  256¸ö
#define SPI_FLASH_Block_ERASE_32k 0x52 //¿é²Á³ý  512×é 32K  512¸ö
#define SPI_FLASH_CHIP_ERASE	  0xC7 //È«Æ¬²Á³ý 16M 

#define SPI_FLASH_COM_RD_STUS1    0x05
#define SPI_FLASH_COM_RD_STUS2    0x35
#define SPI_FLASH_COM_RD_STUS3    0x15


#define SPI_FLASH_COM_WRSR        0x01
#define SPI_FLASH_COM_WRSR2       0x31 //gd spiflash support
#define SPI_FLASH_COM_WRSR3       0x11

#define SPI_FLASH_COM_ERASE_SEC   0x20 //²Á³ý4KB sector //ÉÈÇø²Á³ý 16×é  4K 4096¸ö

//spi flash status3 ADP bit
#define SPI_FLASH_COM_STUS3_ADP   (1<<17)

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



static T_BOOL spi_flash_enter_4byte_addr_mode(T_VOID);
static T_BOOL spi_flash_exit_4byte_addr_mode(T_VOID);
T_BOOL spi_flash_read_status(T_U32 *status);
T_BOOL spi_flash_write_status(T_U32 status);
static T_VOID write_enable(T_VOID);
static T_BOOL check_status_done(T_VOID);
static T_BOOL flash_read_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_read_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_QE_set(T_BOOL enable);



typedef struct
{
    T_BOOL bInit;

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
 * @date 2010-04-29
 * @return T_BOOL
 */
T_BOOL spi_flash_init(T_eSPI_ID spi_id, T_eSPI_BUS bus_wight)
{

    m_sflash.spi_id = spi_id;
    m_sflash.bus_wight = bus_wight;
    m_sflash.bInit = AK_TRUE;

    return AK_TRUE;
}

/**
 * @brief set/clear quad enable
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param enable [in] AK_TRUE: enable QE, AK_FALSE: disable QE
 * @return T_BOOL
 */
static T_BOOL flash_QE_set(T_BOOL enable)
{
    T_U32 status;

    //read status 
    if (!spi_flash_read_status(&status))
    {
        printf("spi QE read status fail\r\n");
        return AK_FALSE;
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
        printf("spi QE: write status fail \r\n");
        return AK_FALSE;
    }

    return AK_TRUE;
}

/**
 * @brief set param of serial flash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param sflash_param [in]  serial flash param 
 * @return T_VOID
 */
T_VOID spi_flash_set_param(T_SFLASH_PARAM *sflash_param)
{
    T_U32 status;

    if(AK_NULL == sflash_param)
        return;

    //save param
    memcpy(&m_sflash.param, sflash_param, sizeof(T_SFLASH_PARAM));

    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    //setup clock
    if(sflash_param->clock > 0)
    {
        spi_init(m_sflash.spi_id, SPI_MODE0, SPI_MASTER, sflash_param->clock);
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
            if (!flash_QE_set(AK_TRUE))
            {
                printf("warnning: enable QE fail, will set to one-wire mode\r\n");
                m_sflash.bus_wight = SPI_BUS1;
            }
            else
            {
                printf( "Set QE OK\r\n");
            }
        }
    }
  
    //enter spi flash 4 byte addressing 
    
    if (m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR)
    {
		if ( spi_flash_enter_4byte_addr_mode())
		{
			printf( "enter spi flash 4byte addressing success\r\n");
		}
	    else
		{
			printf( "enter spi flash 4byte addressing fail\r\n");
	    }
	}
	
    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);

}


/**
 * @brief read data from one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to store read data 
 * @param page_cnt [in]  the page count to read  
 * @return T_BOOL
 */
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_BOOL ret = AK_FALSE;

    spi_set_protect(m_sflash.spi_id, m_sflash.bus_wight);
    if ((SPI_BUS4 == m_sflash.bus_wight) && (m_sflash.param.flag & SFLASH_FLAG_QUAD_READ))
    {
        //line 4
        ret = flash_read_quad(page, buf, page_cnt); 
		
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
 * @brief read spi flash status
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param status [out] status buffer
 * @ 0-7 bit to status1 8-15 bit to status2 16-24 bit to status3
 * @return T_BOOL
 */
T_BOOL spi_flash_read_status(T_U32 *status)
{
    T_U8 buf1[1];
    T_U8 status1 = 0, status2 = 0 , status3 = 0;


     //read status1
   	 buf1[0] = SPI_FLASH_COM_RD_STUS1;
     spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);
     if (!spi_master_read(m_sflash.spi_id, &status1, 1, AK_TRUE))
     	return AK_FALSE;
	 
     //more one status reg
	 if( m_sflash.param.flag & SFLASH_FLAG_COM_STATUS2 )
	 {
		 switch (m_sflash.param.ex_flag & SFLASH_EX_FLAG_STATUS_MASK)
		 {
		 case SFLASH_EX_FLAG_ONE_STATUS:
		 case SFLASH_EX_FLAG_TWO_STATUS:
		 	
		 	//read status 2
	        buf1[0] = SPI_FLASH_COM_RD_STUS2;
	        spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);
	        if(!spi_master_read(m_sflash.spi_id, &status2, 1, AK_TRUE))
	            return AK_FALSE;
		    break;
			
		 case SFLASH_EX_FLAG_THREE_STATUS:
		 	//read status 2
	        buf1[0] = SPI_FLASH_COM_RD_STUS2;
	        spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);
	        if(!spi_master_read(m_sflash.spi_id, &status2, 1, AK_TRUE))
	            return AK_FALSE;
			//read status3
		 	buf1[0] = SPI_FLASH_COM_RD_STUS3;
	     	spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);
	     	if (!spi_master_read(m_sflash.spi_id, &status3, 1, AK_TRUE))
	        	return AK_FALSE;
		    break;
			
		 case SFLASH_EX_FLAG_FOUR_STATUS:
		 	printf( "there are not four status reg to read!\r\n");
	        return AK_FALSE;
			
		    break;

		 }
		 
	 }
	 
    *status = (status1 | (status2 << 8)| (status3 << 16)) & 0x00ffffff;
     return AK_TRUE;
}

/**
*@
*@NOTE: 0-7 bit to status1; 8-15 bit to status2 ;16-24 bit to status3
*@
*/
T_BOOL spi_flash_write_status(T_U32 status)
{
	
    T_U8 buf[4],buf2[2],buf3[2];
    T_U32 write_cnt;


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
				 if (!spi_master_write(m_sflash.spi_id, buf, 3, AK_TRUE))	//common write status reg
					return AK_FALSE;
			 	 if (!check_status_done())  
			 		return AK_FALSE;
				 
					break;
	
			case SFLASH_EX_FLAG_THREE_STATUS:  //reserved
			  	printf( "there are not three status reg common to write!\r\n");
			  	return AK_FALSE;
			  	
			 	 break;
			  
			case SFLASH_EX_FLAG_FOUR_STATUS :  //reserved
			  	printf( "there are not four status reg common to write!\r\n");
              	return AK_FALSE;
			  
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
				if (!spi_master_write(m_sflash.spi_id, buf, 2, AK_TRUE)) // Write C-version flash status Reg. 1
				     return  AK_FALSE;
				if (!check_status_done())  
					return AK_FALSE;
					
				    //write status2 reg         
				 write_enable();
			     buf2[0] = SPI_FLASH_COM_WRSR2;
				 buf2[1] = buf[2];
				 if (!spi_master_write(m_sflash.spi_id, buf2, 2, AK_TRUE))//Write C-version flash status Reg.2
					return AK_FALSE;
				 if (!check_status_done())  
					return AK_FALSE;
				 break;
			//three status
			case SFLASH_EX_FLAG_THREE_STATUS:
					//write status1 reg
				write_enable();
				if (!spi_master_write(m_sflash.spi_id, buf, 2, AK_TRUE)) // Write C-version flash status Reg. 1
				    return  AK_FALSE;
				if (!check_status_done())  
					return AK_FALSE;
					
				    //write status2 reg         
				 write_enable();
				 buf2[0] = SPI_FLASH_COM_WRSR2;
				 buf2[1] = buf[2];
				 if (!spi_master_write(m_sflash.spi_id, buf2, 2, AK_TRUE))//Write C-version flash status Reg.2
					return AK_FALSE;
				 if (!check_status_done())  
					return AK_FALSE;

					//write status3 reg
		          buf3[0] = SPI_FLASH_COM_WRSR3;
			 	  buf3[1] = buf[3];
			 	  write_enable();
			 	 if (!spi_master_write(m_sflash.spi_id, buf3, 2, AK_TRUE))
			   		return AK_FALSE;
			 
		         if (!check_status_done()) 
			        return AK_FALSE;
				 break;
			
			case SFLASH_EX_FLAG_FOUR_STATUS :  //reserved
				printf( "there are not four status reg!\r\n");
	        	return AK_FALSE;
				
				break;
			default:
				printf( "this are not divide to write bits!\r\n");
				return AK_FALSE;
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
	 	if(!spi_master_write(m_sflash.spi_id, buf, write_cnt, AK_TRUE))
	 		return AK_FALSE;
	 	if (!check_status_done()) 
			return AK_FALSE;
	}
	

    return AK_TRUE;
}



static T_BOOL flash_read_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf1[5];
    T_U32 i, addr = page * m_sflash.param.page_size;
    T_BOOL bReleaseCS=AK_FALSE;
    T_U32 count, readlen;
	
  	if( (addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (page > (m_sflash.param.total_size/m_sflash.param.page_size)))
  	{
		printf( "read standard page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return AK_FALSE;	
	}
	
    if((page > SPI_FLASH_PAGE_SIZE_MAX) || (AK_NULL == buf))
    {
        printf( "spi_flash_read: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        printf( "spi flash not init\r\n");
        return AK_FALSE;
    }
	
	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
	{

    	buf1[0] = SPI_FLASH_COM_READ;
    	buf1[1] = (addr >> 16) & 0xFF;
    	buf1[2] = (addr >> 8) & 0xFF;
    	buf1[3] = (addr >> 0) & 0xFF;
    	spi_master_write(m_sflash.spi_id, buf1, 4, AK_FALSE);
	}
	else
	{
	
		buf1[0] = SPI_FLASH_COM_READ;
    	buf1[1] = (addr >> 24) & 0xFF;
  		buf1[2] = (addr >> 16) & 0xFF;
  		buf1[3] = (addr >> 8) & 0xFF;
  		buf1[4] = (addr >> 0) & 0xFF;
		spi_master_write(m_sflash.spi_id, buf1, 5, AK_FALSE);
	}
	
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
            return AK_FALSE;
        buf += readlen;        
    }
    
    return AK_TRUE;
}


static T_BOOL flash_read_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf_cmd[6];
    T_U16 status;
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 count;
    T_U32 readlen;
    T_BOOL bReleaseCS=AK_FALSE;
    T_BOOL ret = AK_TRUE;

	if( (addr > (m_sflash.param.total_size - m_sflash.param.page_size)) || (page > (m_sflash.param.total_size/m_sflash.param.page_size)))
  	{
		printf( "read quad page size beyond the maximal address size:%dM\r\n", (m_sflash.param.total_size/ONE_M_SIZE));
		return AK_FALSE;	
	}
	
    if((page > SPI_FLASH_PAGE_SIZE_MAX) || (AK_NULL == buf))
    {
        printf( "spi_flash_read: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        printf( "spi flash not init\r\n");
        return AK_FALSE;
    }

    // cmd
	if (!(m_sflash.param.ex_flag & SFLASH_EX_FLAG_4B_ADDR))
	{
    	buf_cmd[0] = SPI_FLASH_COM_Q_READ;
    	buf_cmd[1] = (T_U8)((addr >> 16) & 0xFF);
    	buf_cmd[2] = (T_U8)((addr >> 8) & 0xFF);
    	buf_cmd[3] = (T_U8)(addr & 0xFF);
    	buf_cmd[4] = 0;
		
		if (!spi_master_write(m_sflash.spi_id, buf_cmd, 5, AK_FALSE))
	   	{
		   	printf( "spi flash_read_quad: write cmd fail\r\n");
		   	goto EXIT;
	   	}
	}
	else
	{
		buf_cmd[0] = SPI_FLASH_COM_Q_READ;
    	buf_cmd[1] = (T_U8)((addr >> 24) & 0xFF);
  		buf_cmd[2] = (T_U8)((addr >> 16) & 0xFF);
  		buf_cmd[3] = (T_U8)((addr >> 8) & 0xFF);
  		buf_cmd[4] = (T_U8)(addr & 0xFF);
  		buf_cmd[5] = 0;
	
    	if (!spi_master_write(m_sflash.spi_id, buf_cmd, 6, AK_FALSE))
    	{
        	printf( "spi flash_read_quad: write cmd fail\r\n");
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
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
        {
            ret = AK_FALSE;
            goto EXIT;
        }
        buf += readlen;        
    }

EXIT:   
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

    return ret;
}


static T_VOID write_enable(T_VOID)
{
    T_U8 buf1[1];
    
    //write enable
    buf1[0] = SPI_FLASH_COM_WR_EN;
    spi_master_write(m_sflash.spi_id, buf1, 1, AK_TRUE);
}

static T_BOOL check_status_done(T_VOID)
{
    T_U32 timeout = 0;
    T_U32 status = 0;

    while(1)
    {
        spi_flash_read_status(&status);

        if((status & (1 << 0 )) == 0)
            break;

        if(timeout++ > 100000)
        {
            printf( "spiflash check_status_done timeout\r\n");
            return AK_FALSE;
        }
    }

    return AK_TRUE;
}

static T_BOOL spi_flash_enter_4byte_addr_mode(T_VOID)
{

	T_U8 buf1[1];
    //write enable
    buf1[0] = SPI_FLASH_4_B_ADDR_EN;
    return(spi_master_write(m_sflash.spi_id, buf1, 1, AK_TRUE));

}

static T_BOOL spi_flash_exit_4byte_addr_mode(T_VOID)
{

	T_U8 buf1[1];
    //write enable
    buf1[0] = SPI_FLASH_4_B_ADDR_EX;
    return(spi_master_write(m_sflash.spi_id, buf1, 1, AK_TRUE));

}

