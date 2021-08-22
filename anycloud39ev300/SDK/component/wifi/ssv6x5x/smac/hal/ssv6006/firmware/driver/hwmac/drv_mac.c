#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <ssv_lib.h>
#include <hal.h>
#include <bsp_hal.h>
#include <regs.h>
#include <intc/intc.h>
#include <msgevt.h>
#include <mbox/drv_mbox.h>
#include <p2p/drv_p2p.h>
#include <hdr80211.h>
#include <pbuf.h>
#include "drv_mac.h"
#include <log.h>

static u32 drop_all_frame = false;
extern void RxDataHandler(void* RxInfo);
extern void mlme_ps_exit(void);


#define IEEE80211_FTYPE_MGMT        0x0000
#define IEEE80211_FTYPE_CTL     0x0004
#define IEEE80211_FTYPE_DATA        0x0008

/* management */
#define IEEE80211_STYPE_ASSOC_REQ   0x0000
#define IEEE80211_STYPE_ASSOC_RESP  0x0010
#define IEEE80211_STYPE_REASSOC_REQ 0x0020
#define IEEE80211_STYPE_REASSOC_RESP    0x0030
#define IEEE80211_STYPE_PROBE_REQ   0x0040
#define IEEE80211_STYPE_PROBE_RESP  0x0050
#define IEEE80211_STYPE_BEACON      0x0080
#define IEEE80211_STYPE_ATIM        0x0090
#define IEEE80211_STYPE_DISASSOC    0x00A0
#define IEEE80211_STYPE_AUTH        0x00B0
#define IEEE80211_STYPE_DEAUTH      0x00C0

#define IEEE80211_FCTL_FTYPE        0x000c
#define IEEE80211_FCTL_STYPE        0x00f0



//----------------------------------------------------------------------------------------------------------------
static inline bool ssv6200_is_mgmt_frame_pkt(u32 rx_data)
{
    //struct ssv6200_rx_desc_from_soc *rx_desc_from_soc;
    u8 MAC_header;
    
    //rx_desc_from_soc = (struct ssv6200_rx_desc_from_soc *)rx_data;
    MAC_header = *(u8 *)(rx_data + GET_PB_OFFSET);

    return (/*((((rx_desc_from_soc->fCmd)&0xf) == M_ENG_MACRX))&&*/((MAC_header & 0xc) == IEEE80211_FTYPE_MGMT))? 1: 0;
}


static inline bool ssv6200_is_beacon_frame_pkt(u32 rx_data)
{
    //struct ssv6200_rx_desc_from_soc *rx_desc_from_soc;
    u8 MAC_header;
    
    //rx_desc_from_soc = (struct ssv6200_rx_desc_from_soc *)rx_data;
    MAC_header = *(u8 *)(rx_data + GET_PB_OFFSET);

    return (/*((((rx_desc_from_soc->fCmd)&0xf) == M_ENG_MACRX))&&*/((MAC_header & (IEEE80211_FCTL_FTYPE|IEEE80211_FCTL_STYPE))
        == (IEEE80211_FTYPE_MGMT|IEEE80211_STYPE_BEACON)))? 1: 0;
}



//----------------------------------------------------------------------------------------------------------------
// CPU mailbox interrupt handler.
extern void txThroughputHandler(u32 sdio_tx_ptk_len);
void irq_mbox_handler (int m_data)
{
    u8  count;
    (void)m_data;
    ASSERT(gOsFromISR > 0);

    // Get count of the CPU's mailbox FIFO and process every packet in it.
    //count = GET_FFO0_CNT;
    // while (count--)
    while ((count = GET_FFO0_CNT))
    {
        u32 rx_data = (u32)GET_CH0_INT_ADDR;
        //Write 1 to POP
        SET_CPU_QUE_POP(1);

        if (drop_all_frame)
            PBUF_MFree((void *)rx_data);        
        else if (((PKT_TxInfo *)rx_data)->c_type == HOST_CMD) {
                msg_evt_post_data1(MBOX_CMD_ENGINE, MEVT_PKT_BUF, rx_data,0);
        }
        else
            RxDataHandler((void*)rx_data);
    }
}

static void irq_sdio_handler (int irq_id)
{
    //msg_evt_post_data1(MBOX_SOFT_MAC, MEVT_PS_WAKEUP, 0,1);
    mlme_ps_exit();
}

void _notify_host_reset()
{
    u32 notification_data;
    HDR_HostEvent *host_evt;
    u32 evt_size = sizeof(HDR_HostEvent);

    LOG_DEBUG("**************************\n");
    LOG_DEBUG("*** Software MAC reset ***\n");
    LOG_DEBUG("**************************\n");

    // Allocate BA drop notification packet
    do{
        notification_data = (u32)PBUF_MAlloc_Raw(evt_size, 0, RX_BUF);
    }while(notification_data == 0);

    host_evt = (HDR_HostEvent *)notification_data;
    host_evt->c_type = HOST_EVENT;
    host_evt->h_event = SOC_EVT_RESET_HOST;
    host_evt->len = evt_size;

    // Send out to host
    TX_FRAME(notification_data);
}

extern void dump_mailbox_dbg(int num);
u32 damage_count = 0;
static void irq_mac_int_debug_handler(int irq)
{
    LOG_DEBUG("****************************\n");
    LOG_DEBUG("*   Interrupt Exception    \n");
    LOG_DEBUG("* Erro number(%d) count(%d)\n",(u16)irq,damage_count);
    LOG_DEBUG("****************************\n");
   
    LOG_DEBUG("DOUBLE_RLS_INT_EN: %x, ID_DOUBLE_RLS_INT: %x, DOUBLE_RLS_ID: %x\n", 
        GET_DOUBLE_RLS_INT_EN, GET_ID_DOUBLE_RLS_INT, GET_DOUBLE_RLS_ID);
    LOG_DEBUG("ALC_ERR_STS: %x, RLS_ERR_STS: %x, ALC_ERR_CLR: %x, RLS_ERR_CLR: %x\n",
        GET_ALC_ERR_STS, GET_RLS_ERR_STS, GET_ALC_ERR_CLR, GET_RLS_ERR_CLR);
    LOG_DEBUG("AL_STATE: %x, RL_STATE: %x, ALC_ERR_ID: %x, RLS_ERR_ID: %x\n", 
        GET_AL_STATE, GET_RL_STATE, GET_ALC_ERR_ID, GET_RLS_ERR_ID);
    LOG_DEBUG("DMN_NOHIT_STS: %x, DMN_NOHIT_CLR: %x, DMN_WR: %x\n",
        GET_DMN_NOHIT_STS, GET_DMN_NOHIT_CLR, GET_DMN_WR);
    
    LOG_DEBUG("DMN_PORT: %x, DMN_NHIT_ID: %x, DMN_NHIT_ADDR: 0x%08x\n",
        GET_DMN_PORT, GET_DMN_NHIT_ID, (0x80000000|(GET_DMN_NHIT_ID<<16)|(GET_DMN_NHIT_ADDR<<8)));
     
    LOG_DEBUG("ALL_ID[%d]TX[%d]RX[%d]AVA[%d]\n", 
        GET_ALL_ID_ALC_LEN, GET_TX_ID_ALC_LEN, GET_RX_ID_ALC_LEN, GET_AVA_TAG);
    
    if (irq == IRQ_15_DMN_NOHIT)
    {
        u32 adr_dmn_st = REG32(ADR_DMN_STATUS);
        u32 mrx_err = GET_MRX_ERR;
        u32 id_in_use = GET_ID_IN_USE;
        u32 port = (adr_dmn_st & 0x00F0)>>4;
        LOG_DEBUG("mrx_err: %x, id: %x\n",mrx_err, id_in_use);
        // Freddie/Turismo: Turismo should not have demand issue
        #if 1
        if(port == 0x0d) // if port is MRX, reset MRX engine, AMPDU engine, and phy
        {
            damage_count++;
            SET_MB_EXCEPT_CLR(1);
            SET_DMN_NOHIT_CLR(1);

            if(damage_count > 300)
                _notify_host_reset();
        }
        else //if port is not MRX, dump mailbox and memory
        #endif
        {
            //Disable interrupt mask in addition to timer.
            SET_MASK_TYPMCU_INT_MAP(0xf807ffff);
            SET_MASK_TYPMCU_INT_MAP_02(0xffffffff);
            SET_MASK_TYPMCU_INT_MAP_15(0xffffffff);
            SET_MASK_TYPMCU_INT_MAP_31(0xffffffff);

#if (MAILBOX_DBG ==1)
            dump_mailbox_dbg(0);
#endif            
            hex_dump( ((void*)(0x80000000 | ((adr_dmn_st & 0x7f00) << 8))), 256);
            hex_parser((u32)(0x80000000 | ((adr_dmn_st & 0x7f00) << 8)));
            ASSERT(FALSE);
        }
    }
    else
    {
        //Disable interrupt mask in addition to timer.
        SET_MASK_TYPMCU_INT_MAP(0xf807ffff);
        SET_MASK_TYPMCU_INT_MAP_02(0xffffffff);
        SET_MASK_TYPMCU_INT_MAP_15(0xffffffff);
        SET_MASK_TYPMCU_INT_MAP_31(0xffffffff);

        ASSERT(FALSE);
    }

}

void drv_mac_int_debug()
{
    intc_group15_enable(IRQ_15_ALC_ERR,     irq_mac_int_debug_handler);
    intc_group15_enable(IRQ_15_RLS_ERR,     irq_mac_int_debug_handler);
    intc_group15_enable(IRQ_15_ALC_TIMEPUT, irq_mac_int_debug_handler);
    intc_group15_enable(IRQ_15_TRASH_CAN,   irq_mac_int_debug_handler);
    intc_group15_enable(IRQ_15_DMN_NOHIT,   irq_mac_int_debug_handler);
    intc_group15_enable(IRQ_15_DOUBLE_RLS,  irq_mac_int_debug_handler);
}

void drv_mac_check_debug_int (const char *file, int line)
{
    int int_status = intc_group15_status();
    if (  int_status 
        & (  (1 << IRQ_15_ALC_ERR)
           | (1 << IRQ_15_RLS_ERR)
           | (1 << IRQ_15_ALC_TIMEPUT)
           | (1 << IRQ_15_TRASH_CAN)
           | (1 << IRQ_15_DMN_NOHIT)
           | (1 << IRQ_15_DOUBLE_RLS)))
    {
        printf("%s:%d MAC INT: 0x%08X\n", file, line, int_status);
    }
}

/**
* s32 drv_mac_init() - install MBOX interrupt handler to process packet from MAC hardware or host driver.
*/
s32 drv_mac_init(void)
{
#if(MAILBOX_DBG == 1)
    enable_mailbox_dbg();
#endif//__MAILBOX_DBG__
    // Freddie/Turismo 

    SET_MRX_LEN_FLT(0x1000); //Set lens filter, limit lens to 4096

    /* Register MAC interrupt handler here: */
    hal_register_isr(IRQ_MBOX, irq_mbox_handler, NULL);
    hal_intc_irq_enable(IRQ_MBOX);

    intc_group2_enable(IRQ_2_SDIO_WAKE, irq_sdio_handler);  

    /* Enable hw detect mechanism */
    drv_mac_int_debug();
    
    //request_irq(IRQ_MBOX, irq_mbox_handler, (void *)IRQ_MBOX);
    return 0;
}

void block_all_traffic (void)
{
    // ToDo: how to block traffic in soft MAC driver FW?
    drop_all_frame = true;
#if 0
#include "../phy/drv_phy.h"
    // stop PHY RX
    drv_phy_off();    
    // redirect all TX frame to CPU and drop them all
    drop_all_frame = true;
    drv_mac_set_tx_flow_data(0);
    drv_mac_set_tx_flow_ctrl(0);
    drv_mac_set_tx_flow_mgmt(0);
    drv_mac_set_rx_flow_data(0);
    drv_mac_set_rx_flow_ctrl(0);
    drv_mac_set_rx_flow_mgmt(0);
#endif // 0
} // end of - block_all_traffic -

void restore_all_traffic (void)
{
    // start PHY RX
    // restore all traffic
    drop_all_frame = false;
}
void stop_and_halt (void)
{
    block_all_traffic();
    /*lint -save -e716 */
    while (1) {}
    /*lint -restore */
} // end of - stop_and_halt -

extern void hex_dump (const void *addr, u32 size);

void stop_and_dump_and_halt (const void *addr, u32 size)
{
    block_all_traffic();
    hex_dump(addr, size);    
    /*lint -save -e716 */
    while (1) ;
    /*lint -restore */
} // end of - stop_and_halt -

void stop_and_dump (const u32 *addr, u32 size)
{
    block_all_traffic();
    hex_dump(addr, size);    
} // end of - stop_and_halt -


s32 drv_mac_set_rx_flow_data(const u32 *cflow)
{
//    OS_MutexLock(sg_mac_mutex);
    SET_RX_FLOW_DATA(cflow[0]);
    //gRxFlowDataReason = cflow[1];
    //LOG_DEBUG("set gRxFlowDataReason =%x\n",gRxFlowDataReason);
    
//    OS_MutexUnLock(sg_mac_mutex);
    return 0;
}


s32 drv_mac_get_rx_flow_data(u32 *cflow)
{
//    OS_MutexLock(sg_mac_mutex);
    cflow[0] = GET_RX_FLOW_DATA;     
    //cflow[1] = (u32)gRxFlowDataReason;   
//    OS_MutexUnLock(sg_mac_mutex);
    return 0;
}

s32 drv_mac_set_rx_flow_mgmt(const u32 *cflow)
{
//    OS_MutexLock(sg_mac_mutex);
    SET_RX_FLOW_MNG(cflow[0]); 
    //gRxFlowMgmtReason = cflow[1];

//    OS_MutexUnLock(sg_mac_mutex);
    return 0;
}


s32 drv_mac_get_rx_flow_mgmt(u32 *cflow)
{
//    OS_MutexLock(sg_mac_mutex);
    cflow[0] = GET_RX_FLOW_MNG;     
    //cflow[1] = (u32)gRxFlowMgmtReason;
//    OS_MutexUnLock(sg_mac_mutex);
    return 0;
}



s32 drv_mac_set_rx_flow_ctrl(const u32 *cflow)
{
//    OS_MutexLock(sg_mac_mutex);
    SET_RX_FLOW_CTRL(cflow[0]);    
    //gRxFlowCtrlReason = cflow[1];
    
//    OS_MutexUnLock(sg_mac_mutex);
    return 0;
}



s32 drv_mac_get_rx_flow_ctrl(u32 *cflow)
{
//    OS_MutexLock(sg_mac_mutex);
    cflow[0] = GET_RX_FLOW_CTRL;   
    //cflow[1] = (u32)gRxFlowCtrlReason;
//    OS_MutexUnLock(sg_mac_mutex);
    return 0;
}

s32 drv_mac_get_bssid(u8 *mac)
{
    u8 temp[8];

    u32   *mac_31_0 = (u32 *)(temp+0);
    u16   *mac_47_32 = (u16 *)(temp+4);
    *mac_31_0 = (u32)GET_BSSID_31_0;
    *mac_47_32 = (u16)GET_BSSID_47_32;

    memcpy(mac,temp,MAC_ADDR_LEN);
    return 0;
}

s32 drv_mac_get_wsid_peer_mac (s32 index, ETHER_ADDR *mac)
{
    u32 mac32[2];
    
    //*(u32 *)(mac->addr+0) = REG_WSID_ENTRY[index].MAC00_31;
    //*(u32 *)(mac->addr+4) = REG_WSID_ENTRY[index].MAC32_47;
    mac32[0] = REG_WSID_ENTRY[index].MAC00_31;
    mac32[1] = REG_WSID_ENTRY[index].MAC32_47;
    memcpy(mac, mac32, sizeof(ETHER_ADDR));
    return 0;
} // end of - drv_mac_get_wsid_peer_mac -

s32 drv_mac_get_sta_mac(u8 *mac)
{
    u8 temp[8];

    u32   *mac_31_0 = (u32 *)(temp+0);
    u16   *mac_47_32 = (u16 *)(temp+4);

    *mac_31_0 = GET_STA_MAC_31_0;
    *mac_47_32 = (u16)GET_STA_MAC_47_32;

    memcpy(mac,temp,MAC_ADDR_LEN);

    return 0;
}

void drv_set_GPIO(u32 value)
{
    // Freddie/Turismo PAD20 is no longer needed for efuse in Turismo
    #if 0 // PAD20 not defined yet. Liam
    SET_PAD20_SEL_I(8);
    SET_PAD20_OE(1);
    SET_PAD20_OD(value);
    LOG_DEBUG("PAD20_OD=%x\n",GET_PAD20_OD);
    #endif
}

void drv_mac_set_mtrx_tsp(u8 value)
{
    SET_MRX_STP_EN(value);
    SET_MTX_TSF_TIMER_EN(value);
}

u32 drv_mac_get_mtx_tsp(void)
{
    //printf("MTX TSP: %x %x\n", GET_MTX_BCN_TSF_U, GET_MTX_BCN_TSF_L);
    return (GET_MTX_BCN_TSF_L);
}

u32 drv_mac_get_mrx_tsp(const void *ptk_addr)
{
    //printf("MRX TSP: %x \n", REG32(addr+GET_MRX_STP_OFST));
    return REG32(ptk_addr+GET_MRX_STP_OFST);
}

