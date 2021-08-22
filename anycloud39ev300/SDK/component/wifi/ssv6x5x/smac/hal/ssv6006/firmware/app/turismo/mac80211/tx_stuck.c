#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <msgevt.h>
#include <pbuf.h>
#include <cmd_def.h>
#include <mbox/drv_mbox.h>
#include <turismo_regs.h>

#include <rtos.h>
#include "tx_stuck.h"

#define TX_STUCK_TIMER_TIME   200
#define MAX_TX_STUCK_CNT      20
OsTimer         gTxStuckTimer;
int             gTxStuckCnt = 0;

void ResetTxStuckCnt(void)
{
    gTxStuckCnt = 0;
}

void TxStuckHandler(void *args)
{
	PKT_RxInfo *NewPkt=NULL;	
    HDR_HostEvent *host_evt;
    
    gTxStuckCnt++;
    if (gTxStuckCnt < MAX_TX_STUCK_CNT)
        return;

    NewPkt = (PKT_RxInfo *)PBUF_MAlloc(sizeof(HDR_HostEvent), NOTYPE_BUF);
	if(NewPkt == NULL)
        return;
	
    host_evt = (HDR_HostEvent *)NewPkt;
	host_evt->c_type = HOST_EVENT;
	host_evt->h_event = SOC_EVT_TX_STUCK_RESP;
	host_evt->len = sizeof(HDR_HostEvent);
	host_evt->evt_seq_no = gTxStuckCnt;
			
	TX_FRAME((u32)NewPkt);

    OS_MsDelay(50); 
    SET_ALL_SW_RST(1);
}

int StartTxStuckTimer(void)
{
    int ret = 0;
    
    if(gTxStuckTimer == 0) {
        printf("Create TxStuckTimer ...\n");
        ret = OS_TimerCreate(&gTxStuckTimer, TX_STUCK_TIMER_TIME, TRUE, NULL, TxStuckHandler);
        OS_TimerSet(gTxStuckTimer, TX_STUCK_TIMER_TIME, TRUE, NULL);
        printf("Start TxStuckTimer ...\n");
        OS_TimerStart(gTxStuckTimer);
    } else {
        printf("Re-start TxStuckTimer ...\n");
        OS_TimerStart(gTxStuckTimer);
    }
    
    return ret;
}

int StopTxStuckTimer(void)
{
    printf("Stop TxStuckTimer ...\n");
    if(gTxStuckTimer != 0)
        OS_TimerStop(gTxStuckTimer);
    
    return 0;
}

int tx_stuck_init(void)
{
    memset((void *)&gTxStuckTimer, 0, sizeof(OsTimer));

    return 0;
}
