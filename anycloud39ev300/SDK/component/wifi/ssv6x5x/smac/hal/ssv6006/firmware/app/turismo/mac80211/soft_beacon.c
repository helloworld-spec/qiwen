#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <msgevt.h>
#include <pbuf.h>
#include <cmd_def.h>
#include <mbox/drv_mbox.h>
#include <timer/drv_timer.h>
#include <turismo_regs.h>

#include <rtos.h>
#include "soft_beacon.h"

void* last_bcn_ptr = NULL;
OsTimer   gSoftBeaconTimer;

void SoftBeacon_set_bcn_index(u8 bcn_idx)
{
    if ((bcn_idx == 1) || (bcn_idx == 2))
        SET_STAT_PKT_ID(bcn_idx);
    else
        printf("!!!!Invalid value %d\n", bcn_idx);
}

void SoftBeaconHandler(void *data)
{
    PKT_TxInfo *bcn = NULL, *target_bcn = NULL;
    u32 regval = GET_MTX_BCN_CFG_VLD;
    u8 fw_bcn_st = GET_STAT_PKT_ID;
    u32 bcn_pkt_len = 0;

    if(regval == 0x1) {
        bcn = (PKT_TxInfo *)((0x80000000 | (GET_MTX_BCN_PKT_ID0 << 16)));
    } else if(regval == 0x2) {
        bcn = (PKT_TxInfo *)((0x80000000 | (GET_MTX_BCN_PKT_ID1 << 16)));
    } else {
        printf("=============>ERROR!!bcn is unavailable, %d\n", regval);
        return;
    }
    
    bcn_pkt_len = bcn->len+bcn->hdr_offset;
    target_bcn = (PKT_TxInfo *)PBUF_MAlloc_Raw(bcn_pkt_len, 0 , RX_BUF);
    if (target_bcn == NULL)
    {
        printf("No buff for sending beacon\n");
        return ;
    }

    memcpy(target_bcn,bcn, bcn_pkt_len);


    if ((fw_bcn_st & regval) == 0)
        SoftBeacon_set_bcn_index(regval);

    target_bcn->fCmd = (M_ENG_TX_MNG|M_ENG_TRASH_CAN<<4);
    target_bcn->fCmdIdx = 0;
    target_bcn->reason = 0;
    //bcn_back = 0;
    ENG_MBOX_SEND((u32)M_ENG_TX_MNG,(u32)target_bcn);
    last_bcn_ptr = (void *)target_bcn;

}

void SoftBeaconResponse(void)
{
	PKT_RxInfo *NewPkt=NULL;	
    HDR_HostEvent *host_evt;
    
    NewPkt = (PKT_RxInfo *)PBUF_MAlloc(sizeof(HDR_HostEvent), RX_BUF);
	if(NewPkt == NULL)
        return;
	
    host_evt = (HDR_HostEvent *)NewPkt;
	host_evt->c_type = HOST_EVENT;
	host_evt->h_event = SOC_EVT_SW_BEACON_RESP;
	host_evt->len = sizeof(HDR_HostEvent);
	host_evt->evt_seq_no = 0;
			
	TX_FRAME((u32)NewPkt);
}

int StartSoftBeaconTimer(void)
{
    int ret = 0;
    u32 interval = 0;

    if(gSoftBeaconTimer == 0) {
        printf("Create SoftBeaconTimer ...\n");
        interval = GET_MTX_BCN_PERIOD;
        ret = OS_TimerCreate(&gSoftBeaconTimer, interval, TRUE, NULL, SoftBeaconHandler);
        OS_TimerSet(gSoftBeaconTimer, interval, TRUE, NULL);
        printf("Start SoftBeaconTimer ...\n");
        OS_TimerStart(gSoftBeaconTimer);
    } else {
        printf("Re-start SoftBeaconTimer ...\n");
        OS_TimerStart(gSoftBeaconTimer);
    }
    
    if(gSoftBeaconTimer != 0)
        SoftBeaconResponse();
    
    return ret;
}

int StopSoftBeaconTimer(void)
{
    printf("Stop SoftBeaconTimer ...\n");
    if(gSoftBeaconTimer != 0)
        OS_TimerStop(gSoftBeaconTimer);
    
    return 0;
}

int soft_beacon_init(void)
{
    memset((void *)&gSoftBeaconTimer, 0, sizeof(OsTimer));
    return 0;
}
