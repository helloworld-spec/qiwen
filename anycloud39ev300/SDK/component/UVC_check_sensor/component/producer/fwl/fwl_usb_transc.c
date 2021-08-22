/**
 * @filename usb_slave_disk.c
 * @brief: AK3223M how to use usb disk.
 *
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "drv_api.h"
#include "fwl_usb_transc.h"
#include "transc.h"

static volatile unsigned char s_AnykaCmd = 0;
//static volatile unsigned long s_AnykaCmdParams[2];
static unsigned char s_AnykaCmdParams[9] = {0};


extern signed long pp_printf(unsigned char * s, ...);


//********************************************************************
static T_ANYKA_TRANSC *m_pTrans = NULL;
static unsigned long m_cntTrans = 0;

static volatile unsigned long m_cntBuffs = 0;


void Fwl_Usb_Set_Trans(T_ANYKA_TRANSC *pTrans, unsigned long count)
{
    if(NULL == pTrans)
    {
        count = 0;
    }

    m_pTrans = pTrans;
    m_cntTrans = count;

    {
        unsigned long i;
        for(i = 0; i < count; i++)
        {
            pp_printf("[%d]", pTrans[i].cmd);
        }
    }
}


bool ausb_enable(unsigned long mode)
{
    usb_slave_device_enable(USB_MODE_20);

    return true;
}
//********************************************************************
void ausb_disable(void)
{
    usb_slave_set_state(USB_NOTUSE);
    
    usb_slave_device_disable();
}

T_ANYKA_TRANSC* ausb_get_transc(unsigned char cmd)
{
    unsigned long i;
    
    //check for command
    for(i = 0; i < m_cntTrans; i++)
    {
       // pp_printf("<%d, %d>", m_pTrans[i].cmd, pCmd->cmd);
        if(m_pTrans[i].cmd == cmd)
        {
            return (m_pTrans + i);
        }
    }

    return NULL;
}

bool ausb_recv(unsigned long buf, unsigned long count)
{
    T_ANYKA_TRANSC *pTrans = NULL;

    pTrans = ausb_get_transc(s_AnykaCmd);
    if(NULL == pTrans)
    {
        return false;
    }
	
	if(NULL == pTrans->fRcv)
    {
        return true;
    }

    if(!pTrans->fRcv(buf, count))
    {
        return false;
    }

    return true;
}

bool ausb_send(unsigned long buf, unsigned long count)
{
    T_ANYKA_TRANSC *pTrans = NULL;

    pTrans = ausb_get_transc(s_AnykaCmd);
    if(NULL == pTrans)
    {
        pp_printf("error get transc: %d\r\n", s_AnykaCmd);
        return false;
    }
	
	if(NULL == pTrans->fSnd)
    {
        return true;
    }
        
    if(!pTrans->fSnd(buf, count))
    {
        return false;
    }

    return true;
}

bool HandleCmd(unsigned char* scsi_data, unsigned long data_len)
{
    T_CMD_RESULT Rslt = {0};
    T_ANYKA_TRANSC *pTrans = NULL;
    unsigned long i;
   if(NULL == scsi_data)
   {
        if(TRANS_SWITCH_USB == s_AnykaCmd || TRANS_RESET == s_AnykaCmd)
        {
            pp_printf("#");
            pTrans = ausb_get_transc(s_AnykaCmd);
            if(NULL == pTrans)
            {
                return false;
            }

            Rslt.data_count = data_len;
            if(!pTrans->fCmd(s_AnykaCmdParams, sizeof(s_AnykaCmdParams), &Rslt))
            {
                //exit loop if need
                return false;
            }
            return true;
        }
        return false;
   }
   s_AnykaCmd =  *(scsi_data+1);
   
    //pp_printf("s_AnykaCmd\n");
   if(s_AnykaCmd > 128)
   {
        s_AnykaCmd -= 128;
   }
   else
   {
        s_AnykaCmd = 0;
        return true;
   }

   #if 0
   s_AnykaCmdParams[0] = (( *(scsi_data + 5) ) 
                        | ( *(scsi_data + 6) << 8 ) 
                        | ( *(scsi_data + 7) << 16 ) 
                        | ( *(scsi_data + 8) << 24 ));

   s_AnykaCmdParams[1] = (( *(scsi_data + 9) ) 
                        | ( *(scsi_data + 10) << 8 ) 
                        | ( *(scsi_data + 11) << 16 ) 
                        | ( *(scsi_data + 12) << 24 ));

   #else
   //memset(s_AnykaCmdParams, 0, 9);
   //memcpy(s_AnykaCmdParams, &scsi_data[5], 8);
   #if 1
   s_AnykaCmdParams[0] = scsi_data[5];
   s_AnykaCmdParams[1] = scsi_data[6];
   s_AnykaCmdParams[2] = scsi_data[7];
   s_AnykaCmdParams[3] = scsi_data[8];

   s_AnykaCmdParams[4] = scsi_data[9];
   s_AnykaCmdParams[5] = scsi_data[10];
   s_AnykaCmdParams[6] = scsi_data[11];
   s_AnykaCmdParams[7] = scsi_data[12];
   #endif
   //pp_printf("s_AnykaCmdParams[0]:%d \r\n", s_AnykaCmdParams[0]);
   //pp_printf("s_AnykaCmdParams[1]:%d \r\n", s_AnykaCmdParams[1]);
   //pp_printf("s_AnykaCmdParams[2]:%d \r\n", s_AnykaCmdParams[2]);
   //pp_printf("s_AnykaCmdParams[3]:%d \r\n", s_AnykaCmdParams[3]);
   //pp_printf("s_AnykaCmdParams[4]:%d \r\n", s_AnykaCmdParams[4]);
   //pp_printf("s_AnykaCmdParams[5]:%d \r\n", s_AnykaCmdParams[5]);
   //pp_printf("s_AnykaCmdParams[6]:%d \r\n", s_AnykaCmdParams[6]);
   //pp_printf("s_AnykaCmdParams[7]:%d \r\n", s_AnykaCmdParams[7]);
   
   
   #endif
    pTrans = ausb_get_transc(s_AnykaCmd);
    if(NULL == pTrans)
    {
        return false;
    }

    Rslt.data_count = data_len;

    if (TRANS_SWITCH_USB != s_AnykaCmd && TRANS_RESET != s_AnykaCmd)
    {
        //parse command

        if(!pTrans->fCmd(s_AnykaCmdParams, sizeof(s_AnykaCmdParams), &Rslt))
        {
            //exit loop if need
            return false;
        }
    }

    return true;
    
}

void Fwl_Usb_Main()
{
    unsigned long mode = 0;
    
    mode = usb_slave_get_mode();
    pp_printf("usb_slave_get_mode:%d \r\n", mode);
    
    usbdisk_mboot_init(mode);
    usbdisk_mboot_set_cb(HandleCmd,ausb_send,ausb_recv);

    usbdisk_init(mode);
    if(!usbdisk_start())
    {
        pp_printf("usbdisk start fail\r\n");
        while(1);
    }
    while(1)
    {
        usbdisk_proc();
    }
}


