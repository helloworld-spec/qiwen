#include <config.h>
#include <ssv_lib.h>
#include <ssv_regs.h>
#include <stdio.h>
   
#ifdef _NO_OS_
#define LOG_PRINTF printf
#else
#include <soc_global.h>
#include <log.h>
#endif

#include <msgevt.h>
#include <mbox/drv_mbox.h>
#include <pbuf.h>
#include <intc/intc.h>
#include "drv_phy.h"

void drv_phy_off(void) {

    SET_RG_PHY_MD_EN(0);  //Liam renamed.

}

void drv_phy_b_only(void) {

	(*(volatile u32 *) ADR_WIFI_PHY_COMMON_ENABLE_REG) = 0x07e;// B only
}

void drv_phy_bgn(void) {

	(*(volatile u32 *) ADR_WIFI_PHY_COMMON_ENABLE_REG) = 0x17e;// B/G/N only

}

void drv_phy_on(void) {

    SET_RG_PHY_MD_EN(1); //Liam renamed.

}

static void drv_phy_beacon_miss_isr(void) 
{
    LOG_DEBUG("***************************\n");
    LOG_DEBUG("******* Beacon Loss *******\n");
    LOG_DEBUG("***************************\n");

    msg_evt_post_data1(MBOX_SOFT_MAC, MEVT_BEACON_LOSS, 0,0);
	SET_RG_RX_MONITOR_ON(0);	
}

void drv_phy_init(void)
{
    intc_group31_enable(IRQ_31_WIFI_PHY, drv_phy_beacon_miss_isr);
}

