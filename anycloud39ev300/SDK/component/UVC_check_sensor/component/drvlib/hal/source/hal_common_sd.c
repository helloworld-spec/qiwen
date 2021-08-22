/**@file hal_common_sd.c
 * @brief Implement sd&sdio commonl operations of how to control sd&sdio.
 *
 * This file implement sd&sdio common bus driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "sd.h"
#include "hal_sdio.h"
#include "hal_common_sd.h"
#include "sysctl.h"
#include "drv_api.h"

volatile T_pSD_DEVICE g_pSdDevice = NULL;

static bool init_start(bool bInitMem);
static T_eCARD_TYPE init_finish();
static T_eCARD_TYPE get_card_type();
static unsigned long get_rca(void);
static unsigned long set_rca(unsigned short rca);


/**
 * @brief Init sd host controller.
 *
 * Select sd/sdio interface, set share pin,enable sd/sdio module
 * @author Huang Xin
 * @param cif[in] card interface selected
 * @date 2010-07-14
 * @return void
 */
void sd_open_controller(T_eCARD_INTERFACE cif)
{
    if(INTERFACE_NOT_SD == cif)
    {
        return;
    }
    else if(INTERFACE_SDIO == cif)
    {
        akprintf(C3, M_DRVSYS, "..Use SDIO interface\r\n");     
        sysctl_clock(CLOCK_SDIO_ENABLE);
        //reset sdio controller
        sysctl_reset(RESET_SDIO);
	    sdio_set_clock(SD_IDENTIFICATION_MODE_CLK, get_asic_freq(), SD_POWER_SAVE_ENABLE);

    }
    else
    {
        akprintf(C3, M_DRVSYS, "..Use SD/MMC interface\r\n");   
        sysctl_clock(CLOCK_MMCSD_ENABLE);
        //reset sd controller
        sysctl_reset(RESET_SDMMC);
	    set_clock(SD_IDENTIFICATION_MODE_CLK, get_asic_freq(), SD_POWER_SAVE_ENABLE);

    }
    //set share pin
    //set_pin(cif);	// int set interfac
    //set negoiate working clock
}

/**
 * @brief Close sd host controller.
 *
 * Select non sd/sdio interface, restore share pin, close sd/sdio module
 * @author Huang Xin
 * @param cif[in] card interface selected
 * @date 2010-07-14
 * @return void
 */
void sd_close_controller(T_eCARD_INTERFACE cif)
{
    if(INTERFACE_NOT_SD == cif)
    {
        return;
    }
    else if(INTERFACE_SDIO == cif)
    {
        akprintf(C3, M_DRVSYS, "..close SDIO interface\r\n");   
        sysctl_clock(~CLOCK_SDIO_ENABLE);
    }
    else
    {
        akprintf(C3, M_DRVSYS, "..close SD/MMC interface\r\n"); 
        sysctl_clock(~CLOCK_MMCSD_ENABLE);
    }
}

/**
 * @brief get sd host controller states
 * @author LHS
 * @date 2011-10-26
 * @param cif[in] card interface selected
 * @return bool: return TURE mean controller is opend.
 */
bool sd_get_controller_state(T_eCARD_INTERFACE cif)
{
    bool ret = false;

    if(INTERFACE_NOT_SD == cif)
    {
        return ret;
    }
    else if(INTERFACE_SDIO == cif)
    {
        ret = sysctl_get_clock_state(CLOCK_SDIO_ENABLE);
    }
    else
    {
        ret = sysctl_get_clock_state(CLOCK_MMCSD_ENABLE);
    }

    return ret;
}


/**
 * @brief Init sd card.
 *
 * Init card ,get the card type
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
T_eCARD_TYPE init_card(bool bInitIo,bool bInitMem)
{
    unsigned char init_io_status = 0;
    unsigned char init_mem_status =0;
    
    if (!init_start(bInitMem))
    {
         return g_pSdDevice->enmCardType;
    }
    init_io_status = init_io(bInitIo);
    if (COMMON_SD_INIT_IO_FAIL == init_io_status)
    {    
        if (!g_pSdDevice->bMemPresent)
        {
            g_pSdDevice->enmCardType = init_finish();
        }
        else
        {
            init_mem_status = init_mem(bInitMem);
            if ((COMMON_SD_SKIP_INIT_MEM == init_mem_status) || 
                (COMMON_SD_INIT_MEM_FAIL == init_mem_status))
            {
                g_pSdDevice->enmCardType = init_finish();
            }
            if (COMMON_SD_INIT_FAIL == init_mem_status)
            {
                g_pSdDevice->enmCardType = CARD_UNUSABLE;
            }
            
            if (COMMON_SD_INIT_MEM_SUCCESS == init_mem_status)
            {
                g_pSdDevice->enmCardType = get_card_type();
            }
        }
    }
    else if (COMMON_SD_INIT_FAIL == init_io_status)
    {
        g_pSdDevice->enmCardType = CARD_UNUSABLE;
    }
    else if (COMMON_SD_INIT_IO_SUCCESS == init_io_status)
    {
        if (!g_pSdDevice->bMemPresent)
        {
            g_pSdDevice->enmCardType = init_finish();
        }
        else
        {
             init_mem_status = init_mem(bInitMem);
             if ((COMMON_SD_SKIP_INIT_MEM == init_mem_status) || 
                (COMMON_SD_INIT_MEM_FAIL == init_mem_status))
             {
                g_pSdDevice->enmCardType = init_finish();
             }
             if (COMMON_SD_INIT_FAIL == init_mem_status)
             {
                g_pSdDevice->enmCardType = CARD_UNUSABLE;
             }
            
             if (COMMON_SD_INIT_MEM_SUCCESS == init_mem_status)
             {
                g_pSdDevice->enmCardType = get_card_type();
             }
        }
    }
    else if (COMMON_SD_SKIP_INIT_IO == init_io_status)
    {
        init_mem_status = init_mem(bInitMem);
        if ((COMMON_SD_SKIP_INIT_MEM == init_mem_status) || 
            (COMMON_SD_INIT_MEM_FAIL == init_mem_status))
        {
            g_pSdDevice->enmCardType = init_finish();
        }
        if (COMMON_SD_INIT_FAIL == init_mem_status)
        {
            g_pSdDevice->enmCardType = CARD_UNUSABLE;
        }
        if (COMMON_SD_INIT_MEM_SUCCESS == init_mem_status)
        {
            g_pSdDevice->enmCardType = get_card_type();
        }
    }
    return g_pSdDevice->enmCardType;
}
 


/**
 * @brief get sd relative address.
 *
 * Send CMD3 to get the sd relative address.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return unsigned long
 * @retval  The RCA
 * @retval  ERROR_INVALID_RCA
 */
static unsigned long get_rca(void)
{
    unsigned long rca=0;
    if (send_cmd( SD_CMD(3), SD_SHORT_RESPONSE, SD_NO_ARGUMENT ))
    {
        rca = get_short_resp();
        rca = rca >> 16;                
        return rca;
    }
    else
        return ERROR_INVALID_RCA;
}

static unsigned long set_rca(unsigned short rca)
{ 
    if (send_cmd( SD_CMD(3), SD_SHORT_RESPONSE, rca<<16 ))
    {              
        return rca;
    }
    else
        return ERROR_INVALID_RCA;
}


/**
 * @brief Slect or reject a mmc or sd card.
 *
 * Send CMD7 to select a sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param rca[in] The selected card relative address 
 * @return bool
 * @retval true Select successful
 * @retval false Select failed
 */
bool select_card(unsigned long rca)
{
    //deselect card
    if (rca == 0)
    {
        if (send_cmd( SD_CMD(7), SD_NO_RESPONSE, (rca << 16) ))
            return true;         
    }
    //select card
    else
    {
        if (send_cmd( SD_CMD(7), SD_SHORT_RESPONSE, (rca << 16) ))
            return true;           
    }
    return false;
}

/**
 * @brief Init card start
 *
 * Called at the beginning of the init card 
 * @author Huang Xin
 * @date 2010-07-14
 * @return bool
 * @retval true Start successful
 * @retval false  Start failed
 */
static bool init_start(bool bInitMem)
{
    unsigned char i=0;
    unsigned long response=0;
    unsigned long arg_val=0;
    //only init mem,or init all
    if (bInitMem)
    {
        if (!send_cmd( SD_CMD(0), SD_NO_RESPONSE, SD_NO_ARGUMENT ))
        {
            akprintf(C1, M_DRVSYS, "Set the SD CARD idle  failed!\n");  
            return false;
        }
        //NOTE:this delay is necessary ,otherwise re-init will failed while sd is power on for some cards
        mini_delay(3);
        for (i = 0; i<3; i++)
        {
            if (send_cmd( SD_CMD(8), SD_SHORT_RESPONSE, 0x1aa ))
            {                   
                response = get_short_resp();
                if (response != 0x1aa)
                { 
                    g_pSdDevice->enmCardType = CARD_UNUSABLE;
                    return false;
                }
                break;
            }
        }
    }
    //only init io
    else
    {
        SDIO_SET_CMD52_ARG(arg_val,CMD52_WRITE,0,0,CCCR_IO_ABORT,(1<<3));
        if(send_cmd(SD_CMD(52), SD_SHORT_RESPONSE, arg_val) == false)
        {
			akprintf(C3, M_DRVSYS, "no response.\n"); 
			//akprintf(C1, M_DRVSYS, "direct write failed.\n");   
            //return false;
        }
    }
    return true;
}

/**
 * @brief Init card finish
 *
 * Called at the end of the init card 
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
static T_eCARD_TYPE init_finish()
{
    T_eCARD_TYPE card_type;
    if(g_pSdDevice->bInitIoSuccess)
    {
        card_type = get_card_type();
    }
    else
    {
        card_type = CARD_UNUSABLE;
    }
    return card_type;
}

/**
 * @brief Get card type
 *
 * Called at the end of the init card 
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
static T_eCARD_TYPE get_card_type()
{
    unsigned char type=CARD_UNUSABLE;
    if ((!g_pSdDevice->bInitIoSuccess)&&(g_pSdDevice->bInitMemSuccess))
    {
        akprintf(C3, M_DRVSYS, "card type :sd card\r\n");
        g_pSdDevice->ulRCA = get_rca();
        type = CARD_SD;
    }
    if ((g_pSdDevice->bInitIoSuccess)&&(!g_pSdDevice->bInitMemSuccess))
    {
        akprintf(C3, M_DRVSYS, "card type :sdio card\r\n");
        g_pSdDevice->ulRCA = get_rca();
        type = CARD_SDIO;
    }
    if ((g_pSdDevice->bInitIoSuccess)&&(g_pSdDevice->bInitMemSuccess))
    {
        akprintf(C3, M_DRVSYS, "card type :combo card\r\n");
        g_pSdDevice->ulRCA = get_rca();
        type = CARD_COMBO;
    }
    if ((!g_pSdDevice->bInitIoSuccess)&&(!g_pSdDevice->bInitMemSuccess))
    {
        akprintf(C3, M_DRVSYS, "card type :mmc card\r\n");
        //card addr is fixed 0x2,but the addr must be different  for supporting multi card 
        g_pSdDevice->ulRCA = set_rca(0x2);
        type = CARD_MMC;
    }
    if(ERROR_INVALID_RCA != g_pSdDevice->ulRCA)
    {
        return type;
    }
    else
    {
        return CARD_UNUSABLE;
    }
      
}

/**
 * @brief Release card
 *
 * close sd controller and free card device struct, called when init card fail
 * @author Huang Xin
 * @date 2010-07-14
 * @return void
 */
void sd_release()
{
    sd_close_controller(g_pSdDevice->enmInterface);
    drv_free(g_pSdDevice);  
}

