/**@file hal_sdio.c
 * @brief Implement sdio operations of how to control sdio.
 *
 * This file implement sdio driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "l2.h"
#include "hal_print.h"
#include "sysctl.h"
#include "sd.h"
#include "sdio.h"
#include "hal_sdio.h"
#include "hal_common_sd.h"
#include "hal_common_sdio.h"

#include "drv_api.h"
#include "drv_module.h"

static volatile T_pSD_DEVICE g_pCurSdDevice; 


//The interface shall be INTERFACE_SDIO
#define SDIO_DRV_PROTECT(interface) \
        do{ \
            DrvModule_Protect(DRV_MODULE_SDIO);\
            if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)\
			sdio_set_interface(interface);\
			else\
			set_interface(interface);\
        }while(0)

//The interface shall be INTERFACE_NOT_SD
#define SDIO_DRV_UNPROTECT(interface) \
        do{ \
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)\
			sdio_set_interface(interface);\
			else\
			set_interface(interface);\
            DrvModule_UnProtect(DRV_MODULE_SDIO);\
        }while(0)



static bool sdio_set_bus_width(unsigned char bus_mode);
static unsigned char   sdio_get_ocr(unsigned long *ocr );
static unsigned char   sdio_nego_volt(unsigned long volt);

 /**
* @brief initial sdio or combo card
* @author Huang Xin
* @date 2010-06-17
* @param bus_mode[in] bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
* @return bool
* @retval true  set initial successful, card type is sdio or combo
* @retval false set initial fail,card type is not sdio or combo
*/
bool sdio_initial(T_eCARD_INTERFACE cif,unsigned char bus_mode, unsigned long block_len)
{
    T_pSD_DEVICE pSdCard = NULL;
	bool ret = false;

    if (INTERFACE_SDMMC4 != cif && INTERFACE_SDMMC8 != cif && INTERFACE_SDIO !=cif)
    {
        akprintf(C1, M_DRVSYS, "sdio_initial():interface is invalid\n");
        return false;
    }
    if (USE_ONE_BUS != bus_mode && USE_FOUR_BUS != bus_mode && USE_EIGHT_BUS != bus_mode)
    {
        akprintf(C1, M_DRVSYS, "sdio_initial():bus_mode is invalid\n");
        return false;
    }
	    
    //SDIO_DRV_PROTECT(INTERFACE_SDIO);
    DrvModule_Protect(DRV_MODULE_SDIO);
    pSdCard = (T_pSD_DEVICE)drv_malloc(sizeof(T_SD_DEVICE));
    if (NULL == pSdCard)
    {
        akprintf(C1, M_DRVSYS, "sd_initial():drv_malloc fail\n");
        DrvModule_UnProtect(DRV_MODULE_SDIO);
        return false;
    }
	
    g_pCurSdDevice = pSdCard;
	
    memset(g_pCurSdDevice,0,sizeof(T_SD_DEVICE));
    g_pCurSdDevice->enmInterface = cif;

    g_pCurSdDevice->ulVolt = SD_DEFAULT_VOLTAGE;

    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	sdio_set_interface(INTERFACE_SDIO);
	else				
   	set_interface(INTERFACE_SDMMC4);

    sd_open_controller(INTERFACE_SDIO);

    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
    {
		g_pSdioDevice = g_pCurSdDevice;
		sdio_cfg_buf(INNER_BUF_MODE);	//zoutx add  
	    g_pCurSdDevice->enmCardType = sdio_if_init_card(true,false);
    }
    else
    {
		g_pSdDevice = g_pCurSdDevice;
		sd_cfg_buf(INNER_BUF_MODE);	//zoutx add  
	    g_pCurSdDevice->enmCardType = init_card(true,false);
    }
	
    mini_delay(25); 
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
		 sdio_set_clock(SD_TRANSFER_MODE_CLK, get_asic_freq(), SD_POWER_SAVE_ENABLE); 	
	}
	else
	{		
    	//g_pCurSdDevice->enmCardType = init_card(true,false);  
	    set_clock(SD_TRANSFER_MODE_CLK, get_asic_freq(), SD_POWER_SAVE_ENABLE);  
	}
    
    if ((CARD_UNUSABLE== g_pCurSdDevice->enmCardType) || (CARD_SD == g_pCurSdDevice->enmCardType)||(CARD_MMC == g_pCurSdDevice->enmCardType))
    {
    	goto EXIT;
    }
    else 
    {
		
		if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			ret = sdio_if_select_card(g_pCurSdDevice->ulRCA);
		else
			ret = select_card(g_pCurSdDevice->ulRCA);
		
	   if (!ret)
        {
            akprintf(C3, M_DRVSYS, "select card fail !\n");
			goto EXIT;
        }

        if(!sdio_set_bus_width(bus_mode))
        {
             akprintf(C3, M_DRVSYS, "sdio_set_bus_width fail !\n");
			 goto EXIT;
        }

        if(!sdio_set_block_len(0,block_len))
        {
             akprintf(C3, M_DRVSYS, "sdio_set_block_len0 %d fail !\n", block_len);
			goto EXIT;
        }
        
		if(!sdio_set_block_len(1,block_len))
        {
             akprintf(C3, M_DRVSYS, "sdio_set_block_len1 %d fail !\n", block_len);
			 goto EXIT;
        }
    }
	
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	    sdio_cfg_buf(INNER_BUF_MODE);
	else
	    sd_cfg_buf(INNER_BUF_MODE);
	

    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return true;

EXIT:
	
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	sdio_if_release(g_pCurSdDevice);
	else
	sd_release(g_pCurSdDevice);	SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD); 
	SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
	return false;

}

/**
 * @brief enable specifical fuction in sdio card
 * @author Huang Xin
 * @date 2010-06-17
 * @param func[in] function to enable
 * @return bool
 * @retval true enable successfully
 * @retval false enable failed
 */
bool sdio_enable_func(unsigned char func)
{
    unsigned char temp = 0;

	
    SDIO_DRV_PROTECT(INTERFACE_SDIO);
    if(!sdio_write_byte(0, CCCR_IO_ENABLE, (1<<func)))
    {
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD); 
        return false;
    }
    while(!(temp & (1<<func)))
    {
        if(!sdio_read_byte(0, CCCR_IO_READY, &temp))
        {   
            SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
            return false;
        }
    }
        
    akprintf(C3, M_DRVSYS, "set function enable ok!\n");
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return true; 
}
 

/**
* @brief set block length to sdio card
* @author Huang Xin
* @date 2010-06-17
* @param func[in] function to set block length
* @param block_len[in]  block length to set
* @return bool
* @retval true enable successfully
* @retval false enable failed
*/
bool sdio_set_block_len(unsigned char func, unsigned long block_len)
{    
    if(func > 7 || block_len > SDIO_MAX_BLOCK_LEN)
    {
        return false;
    }   
   
    //SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    //block len is stored in  LSB first
    if(!sdio_write_byte(0,(func*0x100+ CCCR_FN0_BLOCK_SIZE+1), (block_len/256)))
    {
        //SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD); 
        akprintf(C3, M_DRVSYS, "set block length fail1!\n");
        return false;
    }
        
    if(!sdio_write_byte(0, (func*0x100+ CCCR_FN0_BLOCK_SIZE), (block_len%256)))
    {
        //SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD); 
        akprintf(C3, M_DRVSYS, "set block length fail2!\n");
        return false;
    }
    g_pCurSdDevice->ulFunBlockLen[func] = block_len;

    akprintf(C3, M_DRVSYS, "set block length ok!\n");
    //SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return true;
}

/**
* @brief  set sdio interrupt callback function
* @author Huang Xin
* @date 2010-06-17
* @param func[in] callback function
* @return bool
* @retval true set successfully
* @retval false set failed
*/
bool sdio_set_int_callback(T_SDIO_INT_HANDLER cb)
{
    if (NULL == cb)
    {
        return false;
    }
 

    SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    
    //TBD

    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return true;
}

 
 /**
  * @brief read one byte  from sdio card 
  * @author Huang Xin
  * @date 2010-06-17
  * @param func[in] function to read
  * @param addr[in] register address to read
  * @param rdata[in] data buffer for read data
  * @return bool
  * @retval true read successfully
  * @retval false read failed
  */

bool sdio_read_byte(unsigned char func, unsigned long addr,  unsigned char *rdata)
{
    unsigned long arg_value;
	bool ret ;

    //SDIO_DRV_PROTECT(INTERFACE_SDIO); 

    SDIO_SET_CMD52_ARG(arg_value, CMD52_READ, func, 0, addr, 0x0);
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		ret = sdio_send_cmd(SD_CMD(52), SD_SHORT_RESPONSE, arg_value);
	else
		ret = send_cmd(SD_CMD(52), SD_SHORT_RESPONSE, arg_value);
	
    if(ret == false)
    {
        akprintf(C1, M_DRVSYS, "direct read failed.\n");

        return false;
    }
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
    *rdata = (sdio_get_short_resp()& 0xff);
	else
	*rdata = (get_short_resp()& 0xff);	
	
    //SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);

    return true;
}

 
 /**
  * @brief write one byte to sdio card 
  * @author Huang Xin
  * @date 2010-06-17
  * @param func[in] function to write
  * @param addr[in] register address to write
  * @param rdata[in] the write byte
  * @return bool
  * @retval true write successfully
  * @retval false write failed
  */
bool sdio_write_byte(unsigned char func, unsigned long addr, unsigned char wdata)
{
    unsigned long arg_value;
	bool ret;

    //SDIO_DRV_PROTECT(INTERFACE_SDIO); 

    SDIO_SET_CMD52_ARG(arg_value, CMD52_WRITE, func, CMD52_NORMAL_WRITE, addr, wdata);
	
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		ret = sdio_send_cmd(SD_CMD(52), SD_SHORT_RESPONSE, arg_value);
	else
		ret = send_cmd(SD_CMD(52), SD_SHORT_RESPONSE, arg_value);
	
    if (ret == false)
    {
        akprintf(C1, M_DRVSYS, "direct write failed.\n");
        //SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);   

        return false;
    }
    //SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return true;
}

 
 /**
  * @brief read multiple byte or block from sdio card 
  * @author Huang Xin
  * @date 2010-06-17
  * @param func[in] function to read
  * @param src[in] register address to read
  * @param count[in] data size(number of byte) to read
  * @param opcode[in] fixed address or increasing address
  * @param rdata[in] data buffer for read data
  * @return bool
  * @retval true read successfully
  * @retval false read failed
  */

bool sdio_read_multi(unsigned char func, unsigned long src, unsigned long count, unsigned char opcode, unsigned char rdata[])
{
    unsigned long status=0;
    unsigned long arg=0;
    unsigned long bytes=0;
    unsigned long blocks=0;
    unsigned char  mode=0;
    unsigned long size=0;
    unsigned long cnt=0;
    unsigned long i = 0;
	bool ret;

    //param validation
    if(func > 7 )
    {
        akprintf(C1, M_DRVSYS, "error param for cmd53 read: func %d", func);
        return false;
    }
    //check sdio controller inner status
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		ret = sdio_trans_busy();
	else
		ret = sd_trans_busy();
    if (ret)
    {
        akprintf(C3, M_DRVSYS, " The sd card is writing and reading.\n");
        return  false;
    } 

    SDIO_DRV_PROTECT(INTERFACE_SDIO); 

    //config cmd 53 param
    blocks = count/g_pCurSdDevice->ulFunBlockLen[func];
    if (blocks)
    {
        mode = CMD53_BLOCK_BASIS;
        size = blocks * g_pCurSdDevice->ulFunBlockLen[func];
        //block transferred,blocks value of 0x0 indicates that the count set to infinite
        //in this case ,the I/O blocks shall be transferred until the operation is aborted by writing to
        //the I/O function select bits(ASx) in the CCCR
        blocks = (blocks >= 512) ? 0 : blocks; 
        SDIO_SET_CMD53_ARG(arg, CMD53_READ, func, mode, opcode, src, blocks);
		if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			ret = sdio_read_multi_block_L2(arg, rdata,size);
		else
			ret = sd_read_multi_block_L2(arg, rdata,size);

        if(!ret)
        {
           SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);

            return false;
        }
        rdata += size;
        if(CMD53_INCR_ADDRESS==opcode)
        {
            src += size;
        }
        
    }
    size = count % g_pCurSdDevice->ulFunBlockLen[func];
    if (size)
    {
        mode = CMD53_BYTE_BASIS;
        cnt = size/512;
        if (cnt)
        {
            //bytes transfered, bytes value of 0x0 shall cause 512 bytes to be read or writen
            bytes = 0;
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			{
	            for (i = 0 ; i < cnt ; i++)
	            {
	                SDIO_SET_CMD53_ARG(arg, CMD53_READ, func, mode, opcode, src, bytes);
	                if (!sdio_read_multi_byte(arg, rdata + i*512, 512))
	                {
	                   SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);

	                    return false;
	                }
	                size -= 512;
	                if(CMD53_INCR_ADDRESS==opcode)
	                {
	                    src += 512;
	                }
	            }
			}
			else
			{
	            for (i = 0 ; i < cnt ; i++)
	            {
	                SDIO_SET_CMD53_ARG(arg, CMD53_READ, func, mode, opcode, src, bytes);
	                if (!sd_read_multi_byte(arg, rdata + i*512, 512))
	                {
	                   SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);

	                    return false;
	                }
	                size -= 512;
	                if(CMD53_INCR_ADDRESS==opcode)
	                {
	                    src += 512;
	                }
	            }

			}
			
        }
        bytes = size;
        if(bytes)
        {
            SDIO_SET_CMD53_ARG(arg, CMD53_READ, func, mode, opcode, src, bytes);
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
				ret = sdio_read_multi_byte(arg, rdata + i*512,size);
			else
				ret = sd_read_multi_byte(arg, rdata + i*512,size);
            if ( !ret)
            {
                SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
				
                return false;
            }
        }         
    }
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);

    return true;
}


 
 /**
  * @brief write multiple byte or block from sdio card 
  * @author Huang Xin
  * @date 2010-06-17
  * @param func[in] function to read
  * @param src[in] register address to read
  * @param count[in] data size(number of byte) to read
  * @param opcode[in] fixed address or increasing address
  * @param wdata[in] the wirte data
  * @return bool
  * @retval true write successfully
  * @retval false write failed
  */
bool sdio_write_multi(unsigned char func, unsigned long dest, unsigned long count, unsigned char opcode, unsigned char wdata[])
{
    unsigned long status=0;
    unsigned long arg=0;
    unsigned long bytes=0;
    unsigned long blocks=0;
    unsigned char  mode=0;
    unsigned long size=0;
    unsigned long cnt=0;
    unsigned long i = 0;
	bool ret;

    //param validation
    if(func > 7 )
    {
        akprintf(C1, M_DRVSYS, "error param for cmd53 read: func %d", func);
        return false;
    }

    //check sdio controller inner status
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		ret = sdio_trans_busy();
	else
		ret = sd_trans_busy();
    if (ret)
    {
        akprintf(C1, M_DRVSYS, " The sd card is writing and reading.\n");
        return  false;
    }
    SDIO_DRV_PROTECT(INTERFACE_SDIO); 

    //config cmd 53 param
    blocks = count/g_pCurSdDevice->ulFunBlockLen[func];
    if (blocks)
    {
        mode = CMD53_BLOCK_BASIS;
        size = blocks * g_pCurSdDevice->ulFunBlockLen[func];
        //block transferred,blocks value of 0x0 indicates that the count set to infinite
        //in this case ,the I/O blocks shall be transferred until the operation is aborted by writing to
        //the I/O function select bits(ASx) in the CCCR
        blocks = (blocks >= 512) ? 0 : blocks; 
        SDIO_SET_CMD53_ARG(arg, CMD53_WRITE, func, mode, opcode, dest, blocks);
        if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			ret = sdio_write_multi_block_L2(arg, wdata,size);
		else
			ret = sd_write_multi_block_L2(arg, wdata,size);
        if(!ret)
        {
            SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);		
            return false;
        }
        wdata += size;
        if(CMD53_INCR_ADDRESS==opcode)
        {
            dest += size;
        }
        
    }
    size = count % g_pCurSdDevice->ulFunBlockLen[func];
    if (size)
    {
        mode = CMD53_BYTE_BASIS;    
        cnt = size/512;
        if (cnt)
        {
            //bytes transfered, bytes value of 0x0 shall cause 512 bytes to be read or writen
            bytes = 0; 
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			{
	            for (i = 0; i < cnt; i++)
	            {
	                SDIO_SET_CMD53_ARG(arg, CMD53_WRITE, func, mode, opcode, dest, bytes);
	                if (!sdio_write_multi_byte(arg, wdata + i*512, 512))
	                {
	                    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
						
	                    return false;
	                }
	                size -= 512;
	                
	                if(CMD53_INCR_ADDRESS==opcode)
	                {
	                    dest += 512;
	                }
	            }
			}
			else
			{
	            for (i = 0; i < cnt; i++)
	            {
	                SDIO_SET_CMD53_ARG(arg, CMD53_WRITE, func, mode, opcode, dest, bytes);
	                if (!sd_write_multi_byte(arg, wdata + i*512, 512))
	                {
	                    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
						
	                    return false;
	                }
	                size -= 512;
	                
	                if(CMD53_INCR_ADDRESS==opcode)
	                {
	                    dest += 512;
	                }
	            }
			}

        }
        bytes = size;
        if(bytes)
        {
            SDIO_SET_CMD53_ARG(arg, CMD53_WRITE, func, mode, opcode, dest, bytes);
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
				ret  = sdio_write_multi_byte(arg, wdata + i*512,size);
			else
				ret  = sd_write_multi_byte(arg, wdata + i*512,size);
            if ( !ret)
            {
                SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);		
				
                return false;
            }
        }        
    }
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);

    return true;
}

/**
* @brief select or deselect a sdio device
*
* the card is selected by its own relative address and gets deselected by any other address; address 0 deselects all
* @author Huang Xin
* @date 2010-06-17
* @param addr[in] the rca of  the card which will be selected
* @return bool
* @retval true  select or deselect successfully
* @retval false  select or deselect failed
*/
bool sdio_select_card(unsigned long addr)
{
	bool ret;
	SDIO_DRV_PROTECT(INTERFACE_SDIO); 

	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		ret = sdio_if_select_card(addr);
	else
		ret = select_card(addr);
    
    if(!ret)
    {
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
        return false;
    }
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return true;
}


/**
 * @brief Init the io partion 
 *
 * Called when init card
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCOMMON_SD_STATUS
 */
unsigned char init_io(bool bInitIo)
{
    unsigned long status,resp,ocr;

    if(!bInitIo)
    {
        akprintf(C1, M_DRVSYS, "skip init sdio\n");
        return COMMON_SD_SKIP_INIT_IO;
    }
    status = sdio_get_ocr(&ocr);
   
    if (SDIO_GET_OCR_FAIL == status)
    {
        akprintf(C1, M_DRVSYS,"no cmd5 resp,skip init sdio\n");
        return COMMON_SD_SKIP_INIT_IO;
    }
 
    if ((SDIO_GET_OCR_INVALID == status)||(SDIO_NO_FUN == status))
    {   
        return COMMON_SD_INIT_IO_FAIL;//mp=1:a_process,   mp=0:b_process,
    }
    if (SDIO_GET_OCR_VALID == status)
    {
        status = sdio_nego_volt(ocr & g_pCurSdDevice->ulVolt);
        if (SDIO_NEGO_FAIL == status)
        {
            return COMMON_SD_INIT_FAIL;
        }
     
        if (SDIO_NEGO_TIMEOUT == status)
        {
            return COMMON_SD_INIT_IO_FAIL;//mp=1:a_process,   mp=0:b_process,
        }
        if(SDIO_NEGO_SUCCESS == status)
        {
            g_pCurSdDevice->bInitIoSuccess = 1;
            return COMMON_SD_INIT_IO_SUCCESS;
        }
    }
    
    akprintf(C1, M_DRVSYS,"init sdio status error!!! \n");
    return COMMON_SD_INIT_IO_ERROR;
}


/**
 * @brief Set sdio card bus width.
 *
 * Usually set the bus width  1 bit or 4 bit  .
 * @author Huang Xin
 * @date 2010-07-14
 * @param bus_mode[in] The bus mode.
 * @return bool
 * @retval  true: set successfully
 * @retval  false: set failed
 */
static bool sdio_set_bus_width(unsigned char bus_mode)
{
    unsigned char temp;

    if(!sdio_read_byte(0, CCCR_BUS_INTERFACE_CONTOROL, &temp))
        return false;

    if(USE_ONE_BUS == bus_mode)
    {
        temp &= ~(0x3<<0);
        temp |= (0x0<<0);
    }
    else if(USE_FOUR_BUS == bus_mode)
    {
        temp &= ~(0x3<<0);
        temp |= (0x2<<0);
    }
    else
    {
        return false;
    }

    if(!sdio_write_byte(0, CCCR_BUS_INTERFACE_CONTOROL, temp))
        return false;

    if(!sdio_read_byte(0, CCCR_BUS_INTERFACE_CONTOROL, &temp))
        return false;


	 
	 g_pCurSdDevice->enmBusMode = bus_mode;

	 if(USE_ONE_BUS == bus_mode)
	 {
		 if( (temp & 0x3) == 0x0 )
		 {
		 	return true;
		 }
	 }
	 else if(USE_FOUR_BUS == bus_mode)
	 {
		 if( (temp & 0x3) == 0x02 )
		 {
		 	return true;
		 }
	 }
	 
    return false; 
}

/**
 * @brief Get the sdio card  ocr register
 *
 * Called when init sdio card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param ocr[out] The buffer to save card ocr.
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
static unsigned char sdio_get_ocr(unsigned long *ocr )
{
    unsigned long response = 0;
    unsigned char fun_num = 0;
    bool mem_present = 0;
	bool ret;

	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		ret = sdio_send_cmd(SD_CMD(5), SD_SHORT_RESPONSE, SD_NO_ARGUMENT);
	else
		ret = send_cmd(SD_CMD(5), SD_SHORT_RESPONSE, SD_NO_ARGUMENT);
	
    if (ret)     
    {   
		if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		response = sdio_get_short_resp();
		else
		response = get_short_resp();
		
        if(response & SDIO_MP_MASK)
        {
            g_pCurSdDevice->bMemPresent = 1;
        }
        else
        {
            g_pCurSdDevice->bMemPresent = 0;
        }

        if(0 == (response & SDIO_FUN_NUM_MASK))
        {
            return SDIO_NO_FUN;
        }
        else
        {
            g_pCurSdDevice->ulFunNum = (response & SDIO_FUN_NUM_MASK)>>SDIO_FUN_NUM_OFFSET ;
        }
        if(0 == (response & g_pCurSdDevice->ulVolt))
            return SDIO_GET_OCR_INVALID;
        
        *ocr = response & SDIO_OCR_MASK;
        return SDIO_GET_OCR_VALID;
    }
    else    
        return SDIO_GET_OCR_FAIL;
}

/**
 * @brief Negotiation of the sdio card  voltage
 *
 * Called when init sdio card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param volt[in] The voltage to try.
 * @return T_eSDIO_STATUS
 */
static unsigned char sdio_nego_volt(unsigned long volt)
{
    unsigned long response = 0;
    unsigned long i=0;
	bool ret;

    do
    {
		if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			ret = sdio_send_cmd(SD_CMD(5), SD_SHORT_RESPONSE, volt);
		else
			ret = send_cmd(SD_CMD(5), SD_SHORT_RESPONSE, volt);
		
		if (ret)       
        {      
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			response = sdio_get_short_resp();    
			else
			response = get_short_resp(); 	
        }
        else
        {
            return SDIO_NEGO_FAIL;
        }
    }while((!(response & SD_STATUS_POWERUP))&& (i++ < 10000));
	
    if(i >= 10000)
    {
        akprintf(C1, M_DRVSYS, "sdio nego time out!\n");    
        return SDIO_NEGO_TIMEOUT;
    }

    akprintf(C3, M_DRVSYS, "sdio nego success, ocr value is 0x%x.\n",response&SDIO_OCR_MASK);
    return SDIO_NEGO_SUCCESS;
}


