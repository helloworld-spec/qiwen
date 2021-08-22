/** 
 * @file hal_mcex.c
 * @brief source file for sd card mcex function
 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiXiao
 * @date 2006-04-05
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "sd.h"
#include "hal_sd.h"
#include "hal_common_sd.h"
#include "l2.h"
#include "sysctl.h"
#include "hal_mcex.h"
#include "drv_module.h"


//The interface shall be INTERFACE_SDMMC4
#define MCEX_DRV_PROTECT(interface) \
        do{ \
            DrvModule_Protect(DRV_MODULE_SDMMC);\
            set_interface(interface);\
        }while(0)

//The interface shall be INTERFACE_NOT_SD
#define MCEX_DRV_UNPROTECT(interface) \
        do{ \
            set_interface(interface);\
            DrvModule_UnProtect(DRV_MODULE_SDMMC);\
        }while(0)

//global variabl for mcex
static T_pCARD_HANDLE s_pMcexCard = NULL;


/**
 * @brief mcex init
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return bool
 * @retval true init success
 * @retval false fail to init
 */
bool mcex_init()
{
    bool ret = true;

    //sd initial 
    s_pMcexCard = (T_pCARD_HANDLE)sd_initial(INTERFACE_SDMMC4, USE_ONE_BUS);
    if(NULL == s_pMcexCard )
    {
        akprintf(C1, M_DRVSYS, "sd initial error\n");
        return false;
    }
    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);
    //check if mcex support
    if(!mcex_check())
    {
        akprintf(C1, M_DRVSYS, "mcex check fail!\n");
        ret = false;
        goto EXIT;
    }

    //mode switch
    if(!mcex_open())
    {
        akprintf(C1, M_DRVSYS, "mcex open fail!\n");
        ret = false;
        goto EXIT;
    }

EXIT:
    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return ret;
}

/**
 * @brief mcex close
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return void
 */
void mcex_close()
{
}

/**
 * @brief get psi
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param type [in]: psi type
 * @param data [out]: data buffer
 * @return bool
 * @retval true get psi success
 * @retval false fail to get psi
 */
bool mcex_get_psi(unsigned long type, unsigned char *data)
{
    unsigned long status;
    bool ret=false;
    volatile unsigned long reg_value;
    unsigned char buf_id = 0;

    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);
    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    //step2: send card command
    if(send_cmd(SD_CMD(MCEX_SEND_PSI), SD_SHORT_RESPONSE, type) == false)
    {
        akprintf(C3, M_DRVSYS, "MCEX_SEND_PSI command failed!\n");
        goto exit;
    }
    //step3: transfer data
    ret = sd_trans_data_dma((unsigned long)data,SD_DEFAULT_BLOCK_LEN,SD_DATA_CTL_TO_HOST);
  
    exit:

    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD);

    return ret;
}

/**
 * @brief check if the present card support mcex function or not
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return bool
 * @retval true the present card support mcex
 * @retval false the present card doesn't support mcex
 */
bool mcex_check()
{
    unsigned char status[512];
    unsigned long i;

    if(!sd_mode_switch(0, 1, 1, (unsigned long *)status))
    {
        return false;
    }
    if (!(status[11] & 0x02)) {
        akprintf(C3, M_DRVSYS, "G&D Functiongroup 2 - Function 1 not supported\n"); // BIT [431:416]
        return false;
    }
    if ((status[16] & 0x0f0) != (1<<4)) {
        akprintf(C3, M_DRVSYS, "G&D Functiongroup 2 - Function 1 Switch failed\n"); // BIT [383:380]
        return false;
    }

    return true;

}

/**
 * @brief open mcex function for the present card
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return bool
 * @retval true open success
 * @retval false fail to open
 */
bool mcex_open()
{
    unsigned char status[64];
    unsigned long i;

    if(!sd_mode_switch(1, 1, 1, (unsigned long *)status))
    {
        return false;
    }
    if (!(status[11] & 0x02)) {
        akprintf(C3, M_DRVSYS, "G&D Functiongroup 2 - Function 1 not supported\n"); // BIT [431:416]
        return false;
    }
    if ((status[16] & 0x0f0) != (1<<4)) {
        akprintf(C3, M_DRVSYS, "G&D Functiongroup 2 - Function 1 Switch failed\n"); // BIT [383:380]
        return false;
    }

    return true;
}

/**
 * @brief reset mcex function for the present card
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return bool
 * @retval true reset success
 * @retval false fail to reset
 */
bool mcex_reset()
{
    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);

    if(!send_cmd( SD_CMD(MCEX_CONTROL_TRM), SD_SHORT_RESPONSE, 1 ) )
    {
        akprintf(C1, M_DRVSYS, "MCEX_CONTROL_TRM failed!\n");
        MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD);
        return false;
    }

    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return true;
}

/**
 * @brief get timeout for mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param read_timeout [in]: timeout for read operation
 * @param write_timeout [in]: timeout for write operation
 * @return bool
 * @retval true reset success
 * @retval false fail to reset
 */
bool mcex_get_timeout(unsigned long *read_timeout, unsigned long *write_timeout)
{
    unsigned char data[512];

    *read_timeout = 0;
    *write_timeout = 0;

    memset(data, 0, 512);
    if(!mcex_get_psi(MCEX_PSI_PR, data))
    {
        return false;
    }

    *read_timeout = (unsigned long)(data[0]) * 250;
    *write_timeout = (unsigned long)(data[1]) * 250;

    return true;
}

static bool mcex_get_status(unsigned char *status, unsigned char *error)
{
    unsigned char data[512];

    *status = 0;
    *error = 0;

    if(!mcex_get_psi(MCEX_PSI_SR, data))
    {
        return false;
    }

    *status = data[0];
    *error = data[1];
    
    return true;
}

static bool mcex_check_status(MCEX_STATUS stop)
{
    unsigned char status, error;
    unsigned long cnt = 0, errcnt= 0 ;

    do
    {
        if(mcex_get_status(&status, &error))
        {
            if(error != eMCEX_error_none)
            {
                akprintf(C3, M_DRVSYS, "progress error!, status=%x, error=%x\n", status, error);
                return false;
            }
            
            if(status == stop)
            {
                return true;
            }
        }
        else
        {
            akprintf(C3, M_DRVSYS, "get mcex status error!\n");
            return false;
        }
    }
    while(cnt++ < 1000);

    akprintf(C3, M_DRVSYS, "get mcex status time out, status=0x%x, error = 0x%x!\n", status, error);
    return false;
}

/**
 * @brief write data through mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param mode [in]: 
 * @param data [in]: data to be written
 * @param blk_size [in]: block size
 * @return bool
 * @retval true write success
 * @retval false fail to write
 */
bool mcex_write(unsigned long mode, unsigned char *data, unsigned long blk_size)
{
    unsigned long arg;
    unsigned long size;
    unsigned long reg_value;
    unsigned long i;
    unsigned long status, ret = false;
    unsigned char buf_id = 0;

    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);

    mode = !!mode;
    arg = mode << 31;
    arg |= blk_size & 0x0000FFFF;
    size = blk_size*SD_DEFAULT_BLOCK_LEN;
    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    //step2: send card command
    if (!send_cmd( SD_CMD(MCEX_WRITE_SEC_CMD), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "The MCEX_WRITE_SEC_CMD command failed!\n");
        goto exit; 
    }
    //step3: transfer data
    ret = sd_trans_data_dma((unsigned long)data,size,SD_DATA_CTL_TO_CARD);
    //step4: wait card status to idle
    if(!mcex_check_status(eMCEX_status_cmd_complete))
    {
        akprintf(C3, M_DRVSYS, "mcex write error\n");
        ret = false;
    }  
    exit:
    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD);

    return ret;

}

/**
 * @brief read data through mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param data [out]: data to be read
 * @param blk_size [in]: block size
 * @return bool
 * @retval true read success
 * @retval false fail to read
 */
bool mcex_read(unsigned char *data, unsigned long blk_size)
{
    unsigned long arg;
    unsigned long size;
    unsigned long reg_value;
    unsigned long i;
    unsigned long status, ret = false;
    unsigned char buf_id;

    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);

    size = blk_size * SD_DEFAULT_BLOCK_LEN;

    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    arg = blk_size & 0x0000FFFF;
    //step2: send card command
    if (!send_cmd( SD_CMD(MCEX_READ_SEC_CMD), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "The MCEX_READ_SEC_CMD command failed!\n");
        goto exit;
    }

    //step3: transfer data
    ret = sd_trans_data_dma((unsigned long)data,size,SD_DATA_CTL_TO_HOST);
    //step4: wait card status to idle
    if(!mcex_check_status(eMCEX_status_idle))
    {
        akprintf(C3, M_DRVSYS, "mcex read error\n");
        ret = false;
    }
    exit:
    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return ret;

}




