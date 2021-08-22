/*
 * Copyright (c) 2015 iComm Semiconductor Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

// Include defines from config.mak to feed eclipse defines from ccflags-y
#ifdef ECLIPSE
#include <ssv_mod_conf.h>
#endif // ECLIPSE

#include <linux/version.h>

#if ((defined SSV_SUPPORT_HAL) && (defined SSV_SUPPORT_SSV6006))
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include "../ssv6006c/ssv6006_cfg.h"
#include "../ssv6006c/ssv6006_mac.h"
#include "ssv6006B_reg.h"
#include "ssv6006B_aux.h"
#include <smac/dev.h>

#include <hal.h>
#include "../ssv6006c/ssv6006_priv.h"
#include "../ssv6006c/ssv6006_priv_normal.h"
#include <smac/ssv_skb.h>
#include <ssvdevice/ssv_cmd.h>
#include <hci/hctrl.h>
#include <hwif/usb/usb.h>
#include <linux_80211.h>

static u32 ssv6006_alloc_pbuf(struct ssv_softc *sc, int size, int type);
static void ssv6006_write_key_to_hw(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, 
        void *key, int wsid, int key_idx, enum SSV6XXX_WSID_SEC key_type);
static int ssv6006_set_macaddr(struct ssv_hw *sh, int vif_idx);
static int ssv6006_set_bssid(struct ssv_hw *sh, u8 *bssid, int vif_idx);
static void ssv6006_write_hw_group_keyidx(struct ssv_hw *sh, struct ssv_vif_priv_data *vif_priv, int key_idx);
static int ssv6006_reset_cpu(struct ssv_hw *sh);

static const ssv_cabrio_reg ssv6006_mac_ini_table[]=
{   
    //-----------------------------------------------------------------------------------------------------------------------------------------
    /* Set wmm parameter to EDCA Q4
     (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
    //{ADR_TXQ4_MTX_Q_AIFSN, 	           0xffff2101},   
    //---
    //HCI module config
    //keep it in host
    //HCI-enable 4 bits
    //RX to Host(bit2)
    //AUTO_SEQNO enable(bit3)
    //Fill tx queue info in rx pacet(Bit 25)	
    //tx rx packet debug counter enable(bit28) 
    //TX on_demand interrupt control between allocate size and transmit data mismatch
    {ADR_CONTROL,                      0x12000006},
    //---
    //RX module config
    //(bit 0)                       Enable hardware timestamp for TSF 
    //(bit8~bit15) value(28)  Time stamp write location
    {ADR_RX_TIME_STAMP_CFG,            ((28 << MRX_STP_OFST_SFT) | 0x01)},
    //-----------------------------------------------------------------------------------------------------------------------------------------
    //MAC config
    {ADR_GLBLE_SET,                    (0 << OP_MODE_SFT)  |                          /* STA mode by default */
                                       (0 << SNIFFER_MODE_SFT) |                      /* disable sniffer mode */
                                       (1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
                                       (SSV6006_TX_PKT_RSVD_SETTING << TX_PKT_RSVD_SFT) |
                                       ((u32)(RXPB_OFFSET) << PB_OFFSET_SFT) },      /* set rx packet buffer offset */

    /**
     * Disable tx/rx ether trap table.
     */
    {ADR_TX_ETHER_TYPE_0,              0x00000000},
    {ADR_TX_ETHER_TYPE_1,              0x00000000},
    {ADR_RX_ETHER_TYPE_0,              0x00000000},
    {ADR_RX_ETHER_TYPE_1,              0x00000000}, 
    /**                                               
     * Set reason trap to discard frames.                
     */                                              
    {ADR_REASON_TRAP0,                 0x7FBC7F87},
    {ADR_REASON_TRAP1,                 0x0000003F},

    // trap HW_ID not match to CPU for smartlink
    {ADR_TRAP_HW_ID,                   M_ENG_CPU},   /* Trap to CPU */


    /**                                         
     * Reset all wsid table entry to invalid.
     */                                      
    {ADR_WSID0,                        0x00000000},  
    {ADR_WSID1,                        0x00000000},
    {ADR_WSID2,                        0x00000000},  
    {ADR_WSID3,                        0x00000000},
    {ADR_WSID4,                        0x00000000},  
    {ADR_WSID5,                        0x00000000},
    {ADR_WSID6,                        0x00000000},  
    {ADR_WSID7,                        0x00000000},            
    //-----------------------------------------------------------------------------------------------------------------------------------------
    //SDIO interrupt
    /*
     * Set tx interrupt threshold for EACA0 ~ EACA3 queue & low threshold
     */
    {ADR_MASK_TYPHOST_INT_MAP,         0xffff7fff}, //bit 15 for int 15 map group
    {ADR_MASK_TYPHOST_INT_MAP_15,      0xff0fffff}, //bit 20,21,22,23
    //BTCX
#ifdef CONFIG_SSV_SUPPORT_BTCX
    /* Set BTCX Parameter*/
    {ADR_BTCX0,                        COEXIST_EN_MSK |
                                       (WIRE_MODE_SZ<<WIRE_MODE_SFT) |
                                       WIFI_TX_SW_POL_MSK | 
                                       BT_SW_POL_MSK},    
    {ADR_BTCX1,                        SSV6006_BT_PRI_SMP_TIME | 
                                       (SSV6006_BT_STA_SMP_TIME << BT_STA_SMP_TIME_SFT) | 
                                       (SSV6006_WLAN_REMAIN_TIME << WLAN_REMAIN_TIME_SFT)},
    {ADR_SWITCH_CTL,                   BT_2WIRE_EN_MSK},

    /*WIFI_TX_SW_O*/
    {ADR_PAD7,                         1},
    /*WIFI_RX_SW_O*/
    {ADR_PAD8,                         0}, 
    /*BT_SW_O*/
    {ADR_PAD9,                         1},
    /*WLAN_ACT (GPIO_1)*/
    {ADR_PAD25,                        1}, 
    /*BT_ACT (GPIO_2)*/
    {ADR_PAD27,                        8},
    /*BT_PRI (GPIO_3)*/
    {ADR_PAD28,                        8},
#endif
    /* update rate for ack/cts response */
    /* set B mode ack/cts rate 1M */
    {ADR_MTX_RESPFRM_RATE_TABLE_01, 0x0000},
    {ADR_MTX_RESPFRM_RATE_TABLE_02, 0x0000},
    {ADR_MTX_RESPFRM_RATE_TABLE_03, 0x0000},
    {ADR_MTX_RESPFRM_RATE_TABLE_11, 0x0000},
    {ADR_MTX_RESPFRM_RATE_TABLE_12, 0x0000},
    {ADR_MTX_RESPFRM_RATE_TABLE_13, 0x0000},
    /* set G mode ack/cts rate */
    //G_12M response 6M control rate
    {ADR_MTX_RESPFRM_RATE_TABLE_92_B2, 0x9090},
    //G_24M response 12M control rate 
    {ADR_MTX_RESPFRM_RATE_TABLE_94_B4, 0x9292},
    /* set N mode ack/cts rate */
    //MCS1(LGI) response 6M control rate
    {ADR_MTX_RESPFRM_RATE_TABLE_C1_E1, 0x9090},
    //MCS3(LGI) response 12M control rate
    {ADR_MTX_RESPFRM_RATE_TABLE_C3_E3, 0x9292},
    //MCS1(SGI) response 6M control rate
    {ADR_MTX_RESPFRM_RATE_TABLE_D1_F1, 0x9090},
    //MCS3(SGI) response 12M control rate
    {ADR_MTX_RESPFRM_RATE_TABLE_D3_F3, 0x9292},

};

static void ssv6006_sec_lut_setting(struct ssv_hw *sh)
{
    u8 lut_sel = 1;
    sh->sc->ccmp_h_sel = 1;

    if (sh->cfg.hw_caps & SSV6200_HW_CAP_AMPDU_TX) {
        printk("Support AMPDU TX mode, ccmp header source must from SW\n");
        sh->sc->ccmp_h_sel = 1;
    }
        
    printk("CCMP header source from %s, Security LUT version V%d\n", (sh->sc->ccmp_h_sel == 1) ?  "SW" : "LUT", lut_sel+1);
    //CCMP header source select. 
    //bit22 set 0: CCMP header source from LUT (Frame to Security Engine: 80211_H + Payload)
    //          1: CCMP header source from SW  (Frame to Security Engine: 80211_H + CCMP_H + Payload + MIC)
    SMAC_REG_SET_BITS(sh, ADR_GLBLE_SET, (sh->sc->ccmp_h_sel << 22), 0x400000);
    
    //LUT version selset.
    //bit23 set 0: V1 (Cabrio design)
    //          1: V2 (Turismo design)
    SMAC_REG_SET_BITS(sh, ADR_GLBLE_SET, (lut_sel << 23), 0x800000);
}

static void ssv6006_set_page_id(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    u32 dev_type = 0, tx_page_threshold = 0;
    
    SMAC_REG_SET_BITS(sh, ADR_TRX_ID_THRESHOLD,
            ((sh->tx_info.tx_id_threshold << TX_ID_THOLD_SFT)|
             (sh->rx_info.rx_id_threshold << RX_ID_THOLD_SFT)),
            (TX_ID_THOLD_MSK | RX_ID_THOLD_MSK));


   dev_type = HCI_DEVICE_TYPE(sh->hci.hci_ctrl);
   
   if ((dev_type == SSV_HWIF_INTERFACE_USB)
       && (sc->sh->cfg.usb_hw_resource & USB_HW_RESOURCE_CHK_TXPAGE)){
       tx_page_threshold = sh->tx_info.tx_page_threshold + SSV6006_USB_FIFO;
   } else {
       tx_page_threshold = sh->tx_info.tx_page_threshold;
   }
   SMAC_REG_SET_BITS(sh, ADR_ID_LEN_THREADSHOLD1,
       ((tx_page_threshold << ID_TX_LEN_THOLD_SFT)|
        (sh->rx_info.rx_page_threshold << ID_RX_LEN_THOLD_SFT)),
        (ID_TX_LEN_THOLD_MSK | ID_RX_LEN_THOLD_MSK));

}

static int ssv6006_init_mac(struct ssv_hw *sh)
{
    struct ssv_softc *sc=sh->sc;
    int ret = 0, i = 0;
    u32 regval;
    u8 null_address[6]={0};


    /* do MAC software reset first*/
    SMAC_REG_WRITE(sh, ADR_BRG_SW_RST, 1 << MAC_SW_RST_SFT);
    do
    {
      
        SMAC_REG_READ(sh, ADR_BRG_SW_RST, & regval);
        i ++;
        if (i >10000){
            printk("MAC reset fail !!!!\n");
            WARN_ON(1);
            ret = 1;
            goto exit;
        }
    } while (regval != 0);
   
// load mac init_table
    ret = HAL_WRITE_MAC_INI(sh);   
    
    if (ret)
       goto exit;

/* 
 * HCI RX Aggregation
 * FLOW CONTROL
 * 1. STOP HCI RX
 * 2. CONFIRM RX STATUS
 * 3. ENABLE HCI RX AGGREGATION
 * 4. SET AGGREGATION LENGTH[LENGTH=AGGREGATION+MPDU]
 * 5. START HCI RX
 */
	if (sh->cfg.hw_caps & SSV6200_HW_CAP_HCI_RX_AGGR) {
		SMAC_REG_SET_BITS(sh, ADR_HCI_TRX_MODE, (0<<HCI_RX_EN_SFT), HCI_RX_EN_MSK);
		do {
			SMAC_REG_READ(sh, ADR_RX_PACKET_LENGTH_STATUS, &regval);
			regval &= HCI_RX_LEN_I_MSK;
			i++;
			if (i > 10000) {
				printk("CANNOT ENABLE HCI RX AGGREGATION!!!\n");
				WARN_ON(1);
				ret = 1;
				goto exit;
			}
		} while (regval != 0);
    
        SMAC_REG_SET_BITS(sh, ADR_HCI_TRX_MODE, (1<<HCI_RX_FORM_1_SFT), HCI_RX_FORM_1_MSK);
		SMAC_REG_SET_BITS(sh, ADR_HCI_FORCE_PRE_BULK_IN, HCI_RX_AGGR_SIZE, HCI_BULK_IN_HOST_SIZE_MSK);
		SMAC_REG_SET_BITS(sh, ADR_HCI_TRX_MODE, (1<<HCI_RX_EN_SFT), HCI_RX_EN_MSK);

	    sh->rx_mode = (sh->cfg.hw_rx_agg_method_3) ? RX_HW_AGG_MODE_METH3 : RX_HW_AGG_MODE;
	} 
   
//Enable TSF to be a hw jiffies(add one by us)
    SMAC_REG_SET_BITS(sh, ADR_MTX_BCN_EN_MISC, 
            1 << MTX_TSF_TIMER_EN_SFT, MTX_TSF_TIMER_EN_MSK);
            
//RX module config
    SMAC_REG_WRITE(sh, ADR_HCI_TX_RX_INFO_SIZE,
        ((u32)(TXPB_OFFSET) << TX_PBOFFSET_SFT) |    /* packet buffer offset for tx */
        ((u32)(sh->tx_desc_len)  << TX_INFO_SIZE_SFT) |   /* tx_info_size send (bytes) (need word boundry, times of 4bytes) */
        ((u32)(sh->rx_desc_len)  << RX_INFO_SIZE_SFT) |   /* rx_info_size send (bytes) (need word boundry, times of 4bytes) */
        ((u32)(sh->rx_pinfo_pad) << RX_LAST_PHY_SIZE_SFT )    /* rx_last_phy_size send (bytes) */
    );

//-----------------------------------------------------------------------------------------------------------------------------------------
//MMU[decide packet buffer no.]
    /* Setting MMU to 256 pages */
    // removed, turismo page size is fixed. remove this registers.
    //SMAC_REG_SET_BITS(sh, ADR_MMU_CTRL, (255<<MMU_SHARE_MCU_SFT), MMU_SHARE_MCU_MSK); // not exist for new mac

//-----------------------------------------------------------------------------------------------------------------------------------------
//Dual interface

    // Freddie ToDo: 
    //   1. Check against HW capability. Only SSV6200 ECO version support RX MAC address filter mask.
    //   2. Enable filter only for the second MA address.
    // @C600011C
    // bit0: default 1. 0 to enable don't care bit1 of RX RA, i.e. bit 1 of first byte.
    // bit1: default 1. 0 to enable don't care bit40 of RX RA, i.e. bit0 of last byte.
    // bit2: default 1. 0 to enable don't care bit41 of RX RA. i.e. bit1 of last byte.
    // bit3: default 1. 0 to accept ToDS in non-AP mode and non-ToDS in AP mode.
    /* Setting RX mask to allow muti-MAC*/
    SMAC_REG_READ(sh,ADR_MRX_WATCH_DOG, &regval);
    regval &= 0xfffffff0;
    SMAC_REG_WRITE(sh,ADR_MRX_WATCH_DOG, regval);


//-----------------------------------------------------------------------------------------------------------------------------------------
//Packet buffer tx/rx id and page size
    /**
     * Tx/RX threshold setting for packet buffer resource. 
     */
       
    ssv6006_set_page_id(sh);
//-----------------------------------------------------------------------------------------------------------------------------------------
//Mail box debug
    #ifdef CONFIG_SSV_CABRIO_MB_DEBUG
    //SRAM address 0
    //ToDo, debug_buffer and DEBUG_SIZE is not defined now
    //should remove these code ???
    SMAC_REG_READ(sh, ADR_MB_DBG_CFG3, &regval);
    regval |= (debug_buffer<<0);
    SMAC_REG_WRITE(sh, ADR_MB_DBG_CFG3, regval);

    SMAC_REG_READ(sh, ADR_MB_DBG_CFG2, &regval);
    regval |= (DEBUG_SIZE<<16);
    SMAC_REG_WRITE(sh, ADR_MB_DBG_CFG2, regval);

    //enable mailbox debug

    SMAC_REG_SET_BITS(sh, ADR_MB_DBG_CFG1, (1<<MB_DBG_EN_SFT), MB_DBG_EN_MSK);
    SMAC_REG_SET_BITS(sh, ADR_MBOX_HALT_CFG, (1<<MB_ERR_AUTO_HALT_EN_SFT), 
            MB_ERR_AUTO_HALT_EN_MSK);
    #endif

//-----------------------------------------------------------------------------------------------------------------------------------------
    //Setting Tx resource low
	SET_TX_PAGE_LIMIT(sh->tx_info.tx_lowthreshold_page_trigger);
	SET_TX_COUNT_LIMIT(sh->tx_info.tx_lowthreshold_id_trigger);
	SET_TX_LIMIT_INT_EN(1);

//PHY and security table
    ret = HAL_INI_HW_SEC_PHY_TABLE(sc);
    if (ret)
        goto exit;
    
    /**
     * Set ssv6200 mac address and set default BSSID. In hardware reset,
     * we the BSSID to 00:00:00:00:00:00.
     */
    // Freddie ToDo: Set MAC addresss when primary interface is being added.    
    ssv6006_set_macaddr(sh, 0);
    for (i=0; i<SSV6006_NUM_HW_BSSID; i++)
        ssv6006_set_bssid(sh, null_address, i);

    /**
     * Set Tx/Rx processing flows.
     */
    #ifdef CONFIG_SSV_HW_ENCRYPT_SW_DECRYPT
    HAL_SET_RX_FLOW(sh, RX_DATA_FLOW, RX_HCI);
    #else
    HAL_SET_RX_FLOW(sh, RX_DATA_FLOW, RX_CIPHER_HCI);
    #endif

    #if defined(CONFIG_P2P_NOA) || defined(CONFIG_RX_MGMT_CHECK)
    HAL_SET_RX_FLOW(sh, RX_MGMT_FLOW, RX_CPU_HCI);
    #else
    HAL_SET_RX_FLOW(sh, RX_MGMT_FLOW, RX_HCI);
    #endif

    HAL_SET_RX_FLOW(sh, RX_CTRL_FLOW, RX_HCI);

    HAL_SET_REPLAY_IGNORE(sh, 1); //?? move to init_mac

    /**
     * Set ssv6200 mac decision table for hardware. The table 
     * selection is according to the type of wireless interface: 
     * AP & STA mode.
     */
    HAL_UPDATE_DECISION_TABLE(sc);
   
    SMAC_REG_SET_BITS(sc->sh, ADR_GLBLE_SET, SSV6XXX_OPMODE_STA, OP_MODE_MSK);
    //Enale security LUT V2
    ssv6006_sec_lut_setting(sh);

	/* Set Rate Report HWID */
	SMAC_REG_SET_BITS(sc->sh, ADR_MTX_RATERPT, M_ENG_HWHCI, MTX_RATERPT_HWID_MSK);
	/* Set rate report length */
    SET_PEERPS_REJECT_ENABLE(1); 

    /* set ba window size according to max rx aggr size*/
    SMAC_REG_WRITE(sh, ADR_AMPDU_SCOREBOAD_SIZE, sh->cfg.max_rx_aggr_size);
exit:
    return ret;

}

static void ssv6006_reset_sysplf(struct ssv_hw *sh)
{
    SMAC_SYSPLF_RESET(sh, ADR_BRG_SW_RST, (1 << PLF_SW_RST_SFT));
}


static int ssv6006_init_hw_sec_phy_table(struct ssv_softc *sc)
{

    struct ssv_hw *sh = sc->sh;
    int i, ret = 0;
    //Set key base on VIF. 
    u32 *hw_buf_ptr = sh->hw_buf_ptr;
    u32 *hw_sec_key = sh->hw_sec_key;

//-----------------------------------------------------------------------------------------------------------------------------------------
//PHY and security table
    /**
     * Allocate a hardware packet buffer space. This buffer is for security
     * key caching and phy info space.
     * allocate one packet buffer.
     */
           
    *hw_buf_ptr = ssv6006_alloc_pbuf(sc, SSV6006_HW_SEC_TABLE_SIZE 
                                    , NOTYPE_BUF);
    
    if((*hw_buf_ptr >> 28) != 8)
    {
        //asic pbuf address start from 0x8xxxxxxxx
        printk("opps allocate pbuf error\n");
        WARN_ON(1);	
        ret = 1;
        goto exit;
    }

    //printk("%s(): ssv6200 reserved space=0x%08x, size=%d\n", 
    //    __FUNCTION__, sh->hw_buf_ptr, (u32)(sizeof(phy_info_tbl)+
    //    sizeof(struct ssv6xxx_hw_sec)));

    // phy_info table is not necessary for ssv6006
  

    /**
        * Init ssv6200 hardware security table: clean the table.
        * And set PKT_ID for hardware security.
        */
    *hw_sec_key = *hw_buf_ptr;        

    //==>Section 1. Write Sec table to SRAM
    for(i = 0; i < SSV6006_HW_SEC_TABLE_SIZE; i+=4) {
        SMAC_REG_WRITE(sh, *hw_sec_key + i, 0);
    }
  
    SMAC_REG_SET_BITS(sh, ADR_SCRT_SET, ((*hw_sec_key >> 16) << SCRT_PKT_ID_SFT),
            SCRT_PKT_ID_MSK);
exit:
    return ret;
}


static int ssv6006_write_mac_ini(struct ssv_hw *sh){

    return SSV6XXX_SET_HW_TABLE(sh, ssv6006_mac_ini_table);
}


static int ssv6006_set_rx_flow(struct ssv_hw *sh, enum ssv_rx_flow type, u32 rxflow)
{

    switch (type){
    case RX_DATA_FLOW:
        return SMAC_REG_WRITE(sh, ADR_RX_FLOW_DATA, rxflow);

    case RX_MGMT_FLOW:
        return SMAC_REG_WRITE(sh, ADR_RX_FLOW_MNG, rxflow);
    
    case RX_CTRL_FLOW:
        return SMAC_REG_WRITE(sh, ADR_RX_FLOW_CTRL, rxflow);
    default:
        return 1;
    }
}

static int ssv6006_set_rx_ctrl_flow(struct ssv_hw *sh)
{
    return ssv6006_set_rx_flow(sh, ADR_RX_FLOW_CTRL, RX_HCI);
}

static int ssv6006_set_macaddr(struct ssv_hw *sh, int vif_idx)
{
    int  ret = 0;

    switch (vif_idx) {
        case 0: //interface 0
            ret = SMAC_REG_WRITE(sh, ADR_STA_MAC_0, *((u32 *)&sh->cfg.maddr[0][0]));
            if (!ret)
                ret = SMAC_REG_WRITE(sh, ADR_STA_MAC_1, *((u32 *)&sh->cfg.maddr[0][4]));
            break;
        case 1: //interface 1
            ret = SMAC_REG_WRITE(sh, ADR_STA_MAC1_0, *((u32 *)&sh->cfg.maddr[1][0]));
            if (!ret)
                ret = SMAC_REG_WRITE(sh, ADR_STA_MAC1_1, *((u32 *)&sh->cfg.maddr[1][4]));
            break;
        default:
            printk("Does not support set MAC address to HW for VIF %d\n", vif_idx);
            ret = -1;
            break;
    }
    return ret;
}

static int ssv6006_set_bssid(struct ssv_hw *sh, u8 *bssid, int vif_idx)
{
    int  ret = 0;
    struct ssv_softc *sc = sh->sc;   

    switch (vif_idx) {
        case 0: //interface 0
            memcpy(sc->bssid[vif_idx], bssid, 6);            
            ret = SMAC_REG_WRITE(sh, ADR_BSSID_0, *((u32 *)&sc->bssid[0][0]));
            if (!ret)
                ret = SMAC_REG_WRITE(sh, ADR_BSSID_1, *((u32 *)&sc->bssid[0][4]));
            break;
        case 1: //interface 1
            memcpy(sc->bssid[vif_idx], bssid, 6);
            ret = SMAC_REG_WRITE(sh, ADR_BSSID1_0, *((u32 *)&sc->bssid[1][0]));
            if (!ret)
                ret = SMAC_REG_WRITE(sh, ADR_BSSID1_1, *((u32 *)&sc->bssid[1][4]));
            break;
        default:
            printk("Does not support set BSSID to HW for VIF %d\n", vif_idx);
            ret = -1;
            break;
    }
    return ret;
}

static u64 ssv6006_get_ic_time_tag(struct ssv_hw *sh)
{
    u32 regval;

    SMAC_REG_READ(sh, ADR_CHIP_DATE_YYYYMMDD, &regval);
    sh->chip_tag = ((u64)regval<<32);
    SMAC_REG_READ(sh, ADR_CHIP_DATE_00HHMMSS, &regval);
    sh->chip_tag |= (regval);

    return sh->chip_tag;
}

static void ssv6006_get_chip_id(struct ssv_hw *sh)
{
    char *chip_id = sh->chip_id;
    u32 regval;
   
    //CHIP ID
    SMAC_REG_READ(sh, ADR_CHIP_ID_3, &regval);
    *((u32 *)&chip_id[0]) = __be32_to_cpu(regval);
    SMAC_REG_READ(sh, ADR_CHIP_ID_2, &regval);
    *((u32 *)&chip_id[4]) = __be32_to_cpu(regval);
    SMAC_REG_READ(sh, ADR_CHIP_ID_1, &regval);
    *((u32 *)&chip_id[8]) = __be32_to_cpu(regval);
    SMAC_REG_READ(sh, ADR_CHIP_ID_0, &regval);
    *((u32 *)&chip_id[12]) = __be32_to_cpu(regval);
    chip_id[12+sizeof(u32)] = 0;;
}

static void ssv6006_save_hw_status(struct ssv_softc *sc)
{
    struct ssv_hw *sh = sc->sh;
    int     i = 0;
    int     address = 0;
    u32     word_data = 0;
    u32     sec_key_tbl_base = sh->hw_sec_key[0];
    u32     sec_key_tbl = sec_key_tbl_base;
    
    /* security table */
	for (i = 0; i < sizeof(struct ssv6006_hw_sec); i += 4) {
        address = sec_key_tbl + i;
        SMAC_REG_READ(sh, address, &word_data);
        sh->write_hw_config_cb(sh->write_hw_config_args, address, word_data);
	}
}

static void ssv6006_set_hw_wsid(struct ssv_softc *sc, struct ieee80211_vif *vif,
    struct ieee80211_sta *sta, int wsid)
{
    struct ssv_sta_priv_data *sta_priv_dat=(struct ssv_sta_priv_data *)sta->drv_priv;
    struct ssv_sta_info *sta_info;

    sta_info = &sc->sta_info[wsid];
    /**
     * Allocate a free hardware WSID for the added STA. If no more
     * hardware entry present, set hw_wsid=-1 for
     * struct ssv_sta_info.
     */    
    if (sta_priv_dat->sta_idx < SSV6006_NUM_HW_STA) 
    {
        u32 reg_wsid[] = {ADR_WSID0, ADR_WSID1, ADR_WSID2, ADR_WSID3, 
                          ADR_WSID4, ADR_WSID5, ADR_WSID6, ADR_WSID7};
        u32 reg_peer_mac0[] = {ADR_PEER_MAC0_0, ADR_PEER_MAC1_0, ADR_PEER_MAC2_0, ADR_PEER_MAC3_0, 
                               ADR_PEER_MAC4_0, ADR_PEER_MAC5_0, ADR_PEER_MAC6_0, ADR_PEER_MAC7_0};        
        u32 reg_peer_mac1[] = {ADR_PEER_MAC0_1, ADR_PEER_MAC1_1, ADR_PEER_MAC2_1, ADR_PEER_MAC3_1, 
                               ADR_PEER_MAC4_1, ADR_PEER_MAC5_1, ADR_PEER_MAC6_1, ADR_PEER_MAC7_1};

        /* Add STA into hardware for hardware offload */
        SMAC_REG_WRITE(sc->sh, reg_peer_mac0[wsid], *((u32 *)&sta->addr[0]));
        SMAC_REG_WRITE(sc->sh, reg_peer_mac1[wsid], *((u32 *)&sta->addr[4]));

        /* Valid this wsid entity */
        SMAC_REG_WRITE(sc->sh, reg_wsid[wsid], 1);
        
        sta_info->hw_wsid = sta_priv_dat->sta_idx;

    } 
}

static void ssv6006_del_hw_wsid(struct ssv_softc *sc, int hw_wsid)
{
    if ((hw_wsid != -1) && (hw_wsid < SSV6006_NUM_HW_STA)) {
        u32 reg_wsid[] = {ADR_WSID0, ADR_WSID1, ADR_WSID2, ADR_WSID3, 
                          ADR_WSID4, ADR_WSID5, ADR_WSID6, ADR_WSID7};
        
        SMAC_REG_WRITE(sc->sh, reg_wsid[hw_wsid], 0x00);
    }
}

static void ssv6006_set_aes_tkip_hw_crypto_group_key (struct ssv_softc *sc,
                                               struct ssv_vif_info *vif_info,
                                               struct ssv_sta_info *sta_info,
                                               void *param)
{
    int                    wsid = sta_info->hw_wsid;
    int                    key_idx = *(u8 *)param;

    if (wsid == (-1))
        return;   

    BUG_ON(key_idx == 0);
    
    printk("Set CCMP/TKIP group key index %d to WSID %d.\n", key_idx, wsid);

    if (vif_info->vif_priv != NULL)
        dev_info(sc->dev, "Write group key index %d to VIF %d \n",
                 key_idx, vif_info->vif_priv->vif_idx);
    else
        dev_err(sc->dev, "NULL VIF.\n");

    ssv6006_write_hw_group_keyidx(sc->sh, vif_info->vif_priv, key_idx);
    HAL_ENABLE_FW_WSID(sc, sta_info->sta, sta_info, SSV6XXX_WSID_SEC_GROUP);
}

static void ssv6006_write_pairwise_keyidx_to_hw(struct ssv_hw *sh, int key_idx, int wsid)
{
    int     address = 0;
    u32     Word_data = 0;
    u32     sec_key_tbl_base = sh->hw_sec_key[0];
    u32     sec_key_tbl = sec_key_tbl_base;

    address = sec_key_tbl + (SSV_NUM_VIF * sizeof(struct ssv6006_bss))  //start address of WSID table in PKT Buffer 
    + wsid * sizeof(struct ssv6006_hw_sta_key);                         //size of WSID entry
    // only write 1 byte pairwise key index in the header of WSID entry
    SMAC_REG_READ(sh, address, &Word_data);
    Word_data = ((Word_data & 0xffffff00) | (u32)key_idx);
    SMAC_REG_WRITE(sh, address, Word_data);
}

static void ssv6006_write_hw_group_keyidx(struct ssv_hw *sh, struct ssv_vif_priv_data *vif_priv, int key_idx)
{
    int     address = 0;
    u32     Word_data = 0;
    u32     sec_key_tbl_base = sh->hw_sec_key[0];
    u32     sec_key_tbl = sec_key_tbl_base;

    address = sec_key_tbl + vif_priv->vif_idx * sizeof(struct ssv6006_bss);
    // only update 1 byte group key index in the header of BSSID entry
    SMAC_REG_READ(sh, address, &Word_data);
    Word_data = ((Word_data & 0xffffff00) | key_idx);
    SMAC_REG_WRITE(sh, address, Word_data);
}

static int ssv6006_write_pairwise_key_to_hw (struct ssv_softc *sc,
    int index, u8 algorithm, const u8 *key, int key_len,
    struct ieee80211_key_conf *keyconf,
    struct ssv_vif_priv_data *vif_priv,
    struct ssv_sta_priv_data *sta_priv)
{
    int                    wsid = (-1);

    if (sta_priv == NULL)
    {
        dev_err(sc->dev, "Set pair-wise key with NULL STA.\n");
        return -EOPNOTSUPP;
    }

    wsid = sta_priv->sta_info->hw_wsid;

    if ((wsid < 0) || (wsid >= SSV_NUM_STA))
    {
        dev_err(sc->dev, "Set pair-wise key to invalid WSID %d.\n", wsid);
        return -EOPNOTSUPP;
    }

    dev_info(sc->dev, "Set STA %d's pair-wise key of %d bytes.\n", wsid, key_len);
    
    ssv6006_write_key_to_hw(sc, vif_priv, keyconf->key, wsid, index, SSV6XXX_WSID_SEC_PAIRWISE);
        
    return 0;
}

static int ssv6006_write_group_key_to_hw (struct ssv_softc *sc,
    int index, u8 algorithm, const u8 *key, int key_len,
    struct ieee80211_key_conf *keyconf,
    struct ssv_vif_priv_data *vif_priv,
    struct ssv_sta_priv_data *sta_priv)
{
    u32     sec_key_tbl_base = sc->sh->hw_sec_key[0];
    int                    address = 0;
    int                   *pointer = NULL;
    int                    i;
    int                    wsid = sta_priv ? sta_priv->sta_info->hw_wsid : (-1);
    int                    ret = 0;

    if (vif_priv == NULL)
    {
        dev_err(sc->dev, "Setting group key to NULL VIF\n");
        return -EOPNOTSUPP;
    }

    dev_info(sc->dev, "Setting VIF %d group key %d of length %d to WSID %d.\n",
             vif_priv->vif_idx, index, key_len, wsid);

    /*save group key index */
    vif_priv->group_key_idx = index;

    if (sta_priv)
        sta_priv->group_key_idx = index;

    address = sec_key_tbl_base
              + (vif_priv->vif_idx * sizeof(struct ssv6006_bss))  //offset of BSSID entry
              + SSV6006_GROUP_KEY_OFFSET                          //offset of groupkey
              + index * sizeof(struct ssv6006_hw_key);          //offset of groupkey[index]
    
    pointer = (int *)key;

    /* write group key*/
    for (i = 0; i < (sizeof(struct ssv6006_hw_key)/4); i++)
         SMAC_REG_WRITE(sc->sh, address+(i*4), *(pointer++));
    
    /* write group key index to all sta entity*/
    WARN_ON(sc->vif_info[vif_priv->vif_idx].vif_priv == NULL);
    ssv6xxx_foreach_vif_sta(sc, &sc->vif_info[vif_priv->vif_idx],
                            ssv6006_set_aes_tkip_hw_crypto_group_key, &index);
    ret = 0;

    return ret;
}

static void ssv6006_write_key_to_hw(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, 
    void *key, int wsid, int key_idx, enum SSV6XXX_WSID_SEC key_type)
{   
    int     address = 0;
    int     *pointer = NULL;
    u32     sec_key_tbl_base = sc->sh->hw_sec_key[0];
    u32     sec_key_tbl = sec_key_tbl_base;
    int     i;
       
    switch (key_type)
    {
        case SSV6XXX_WSID_SEC_PAIRWISE:
            /* write pairwise key index to right position*/
            if(key_idx >= 0)
                ssv6006_write_pairwise_keyidx_to_hw(sc->sh, key_idx, wsid);
            
            /* write pairwise key to right position*/
            address = sec_key_tbl + (SSV_NUM_VIF * sizeof(struct ssv6006_bss))  //start address of WSID table in PKT Buffer 
              + (wsid * sizeof(struct ssv6006_hw_sta_key))                      //size of WSID entry
              + SSV6006_PAIRWISE_KEY_OFFSET;                                    //offset of pairwisekey in ssv6006_hw_sta_key

            pointer = (int *)key;

            for (i = 0; i < (sizeof(struct ssv6006_hw_key)/4); i++)
                SMAC_REG_WRITE(sc->sh, address+(i*4), *(pointer++));                                   
    
            break;
            
        case SSV6XXX_WSID_SEC_GROUP:
            if(key_idx < 0) {
                printk("invalid group key index %d.\n",key_idx);
                return;
            }
            
            /* write group key index to right position*/
            ssv6006_write_hw_group_keyidx(sc->sh, vif_priv, key_idx);
    
            /* write group key to right position*/
            address = sec_key_tbl                     //start address of BSSID table in PKT Buffer 
              + (vif_priv->vif_idx * sizeof(struct ssv6006_bss))  //size of BSSID entry
              + SSV6006_GROUP_KEY_OFFSET              //offset of group key in ssv6006_bss
              + key_idx * SSV6006_HW_KEY_SIZE;        //offset of key in ssv6006_hw_key

            pointer = (int *)key;

            for (i = 0; i < (sizeof(struct ssv6006_hw_key)/4); i++)
                SMAC_REG_WRITE(sc->sh, address+(i*4), *(pointer++));
    
            break;
            
        default:
            printk(KERN_ERR "invalid key type %d.",key_type);
            break;
    } 
}

static void ssv6006_set_pairwise_cipher_type(struct ssv_hw *sh, u8 cipher, u8 wsid)
{
    int     address = 0;
    u32     temp = 0;
    u32     sec_key_tbl_base = sh->hw_sec_key[0];
    u32     sec_key_tbl = sec_key_tbl_base;

    //WSIDX entry in HW sec table
    address = sec_key_tbl + (SSV_NUM_VIF * sizeof(struct ssv6006_bss)) 
	                      + (wsid * sizeof(struct ssv6006_hw_sta_key));

    //Only update pairwise cipher type
    SMAC_REG_READ(sh, address, &temp);
    temp = (temp & 0xffff00ff);
    temp |= (cipher << 8);
    
    /* write to pairwise cipher type to right position*/
    SMAC_REG_WRITE(sh, address, temp);

    printk(KERN_ERR "Set parewise key type %d\n", cipher);
}

static void ssv6006_set_group_cipher_type(struct ssv_hw *sh, struct ssv_vif_priv_data *vif_priv, u8 cipher)
{
    int     address = 0;
    u32     temp = 0;
    u32     sec_key_tbl_base = sh->hw_sec_key[0];
    u32     sec_key_tbl = sec_key_tbl_base;
               
    //SSIDX entry in HW sec table
    address = sec_key_tbl + (vif_priv->vif_idx * sizeof(struct ssv6006_bss));

    //Only update group cipher type
    SMAC_REG_READ(sh, address, &temp);
    temp = (temp & 0xffff00ff);
    temp |= (cipher << 8);
    
    /* write to group cipher type to right position*/
    SMAC_REG_WRITE(sh, address, temp);

    printk(KERN_ERR "Set group key type %d\n", cipher);
}

#ifdef CONFIG_PM
static void ssv6006_save_clear_trap_reason(struct ssv_softc *sc)
{
	u32 trap0, trap1;
	SMAC_REG_READ(sc->sh, ADR_REASON_TRAP0, &trap0);
	SMAC_REG_READ(sc->sh, ADR_REASON_TRAP1, &trap1);
	SMAC_REG_WRITE(sc->sh, ADR_REASON_TRAP0, 0x00000000);
	SMAC_REG_WRITE(sc->sh, ADR_REASON_TRAP1, 0x00000000);
	printk("trap0 %08x, trap1 %08x\n", trap0, trap1);

	sc->trap_data.reason_trap0 = trap0;
	sc->trap_data.reason_trap1 = trap1;
}

static void ssv6006_restore_trap_reason(struct ssv_softc *sc)
{
	SMAC_REG_WRITE(sc->sh, ADR_REASON_TRAP0, sc->trap_data.reason_trap0);
	SMAC_REG_WRITE(sc->sh, ADR_REASON_TRAP1, sc->trap_data.reason_trap1);
}

static void ssv6006_pmu_awake(struct ssv_softc *sc)
{
//TODO: check Turismo how to do it
#if 0
	u32 dev_type = 0;
    dev_type = HCI_DEVICE_TYPE(sh->hci.hci_ctrl);
    if (dev_type == SSV_HWIF_INTERFACE_SDIO) {
        SMAC_REG_SET_BITS(sc->sh, ADR_FN1_INT_CTRL_RESET, (1<<24), 0x01000000);
	    MDELAY(5);
	    SMAC_REG_SET_BITS(sc->sh, ADR_FN1_INT_CTRL_RESET, (0<<24), 0x01000000);
    }
    else if (dev_type == SSV_HWIF_INTERFACE_USB) {
        //USB TODO
        printk("ssv6006_pmu_awake: USB TODO\n");
    
    }
#endif    
}
#endif

static void ssv6006_set_wep_hw_crypto_setting (struct ssv_softc *sc,
    struct ssv_vif_info *vif_info, struct ssv_sta_info *sta_info,
    void *param)
{
    int                       wsid = sta_info->hw_wsid;
    struct ssv_sta_priv_data *sta_priv = (struct ssv_sta_priv_data *)sta_info->sta->drv_priv;
    struct ssv_vif_priv_data *vif_priv = (struct ssv_vif_priv_data *)vif_info->vif->drv_priv;

    if (wsid == (-1))
        return;

    sta_priv->has_hw_encrypt = vif_priv->has_hw_encrypt;
    sta_priv->has_hw_decrypt = vif_priv->has_hw_decrypt;
    sta_priv->need_sw_encrypt = vif_priv->need_sw_encrypt;
    sta_priv->need_sw_decrypt = vif_priv->need_sw_decrypt;
}

/**
 * WEP set hw key flow:
 *------------------------------------------------------------------
 * (1) Set Key | store all key in HW group_key[] of BSSIDX     | 
 *------------------------------------------------------------------
 * (2) TX      | write the latest WEP Cipher type & Key index |
 *------------------------------------------------------------------
 */
static void ssv6006_store_wep_key(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv,
    struct ssv_sta_priv_data *sta_priv, enum SSV_CIPHER_E cipher, struct ieee80211_key_conf *key)
{
    if ((vif_priv->has_hw_decrypt == true) && (vif_priv->has_hw_encrypt == true)) {
        //store all wep key in group_key[] of BSSIDX
        printk("Store WEP key index %d to HW group_key[%d] of VIF %d\n", key->keyidx, key->keyidx,vif_priv->vif_idx);
        ssv6006_write_key_to_hw(sc, vif_priv, key->key, 0, key->keyidx, SSV6XXX_WSID_SEC_GROUP);
        ssv6xxx_foreach_vif_sta(sc, &sc->vif_info[vif_priv->vif_idx], ssv6006_set_wep_hw_crypto_setting, key);
    }
    else
        printk("Not support HW security\n");
}

static void ssv6006_set_replay_ignore(struct ssv_hw *sh,u8 ignore)
{
    u32 temp;
    SMAC_REG_READ(sh,ADR_SCRT_SET,&temp);
    temp = temp & SCRT_RPLY_IGNORE_I_MSK;
    temp |= (ignore << SCRT_RPLY_IGNORE_SFT);
    SMAC_REG_WRITE(sh,ADR_SCRT_SET, temp);
}

static void ssv6006_update_decision_table_6(struct ssv_hw *sh, u32 value)
{
    SMAC_REG_WRITE(sh, ADR_MRX_FLT_TB6, value);
}

static int ssv6006_update_decision_table(struct ssv_softc *sc)
{
    int i;
#ifndef USE_CONCURRENT_DECI_TBL
    for(i=0; i<MAC_DECITBL1_SIZE; i++) {
        SMAC_REG_WRITE(sc->sh, ADR_MRX_FLT_TB0+i*4, 
        sc->mac_deci_tbl[i]);
        SMAC_REG_CONFIRM(sc->sh, ADR_MRX_FLT_TB0+i*4,
        sc->mac_deci_tbl[i]);  
    }
    for(i=0; i<MAC_DECITBL2_SIZE; i++) {
        SMAC_REG_WRITE(sc->sh, ADR_MRX_FLT_EN0+i*4,
        sc->mac_deci_tbl[i+MAC_DECITBL1_SIZE]); 
        SMAC_REG_CONFIRM(sc->sh, ADR_MRX_FLT_EN0+i*4,
        sc->mac_deci_tbl[i+MAC_DECITBL1_SIZE]);   
    }
#else
extern u16 concurrent_deci_tbl[];
    for(i=0; i<MAC_DECITBL1_SIZE; i++) {
        SMAC_REG_WRITE(sc->sh, ADR_MRX_FLT_TB0+i*4, 
        concurrent_deci_tbl[i]);
        SMAC_REG_CONFIRM(sc->sh, ADR_MRX_FLT_TB0+i*4,
        concurrent_deci_tbl[i]);  
    }
    for(i=0; i<MAC_DECITBL2_SIZE; i++) {
        SMAC_REG_WRITE(sc->sh, ADR_MRX_FLT_EN0+i*4,
        concurrent_deci_tbl[i+MAC_DECITBL1_SIZE]); 
        SMAC_REG_CONFIRM(sc->sh, ADR_MRX_FLT_EN0+i*4,
        concurrent_deci_tbl[i+MAC_DECITBL1_SIZE]);   
    }
    SMAC_REG_WRITE(sc->sh, ADR_MRX_FLT_EN9,
        concurrent_deci_tbl[8]); 
    SMAC_REG_CONFIRM(sc->sh, ADR_MRX_FLT_EN10,
        concurrent_deci_tbl[9]);
    SMAC_REG_CONFIRM(sc->sh, ADR_DUAL_IDX_EXTEND, 1); 
#endif
    return 0;
}

static void ssv6006_get_fw_version(struct ssv_hw *sh, u32 *regval)
{
    SMAC_REG_READ(sh, ADR_TX_SEG, regval);
}

static void ssv6006_set_op_mode(struct ssv_hw *sh, u32 op_mode, int vif_idx)
{
    switch (vif_idx) {
        case 0:
            SMAC_REG_SET_BITS(sh, ADR_GLBLE_SET, op_mode, OP_MODE_MSK);
            break;
        case 1:
            SMAC_REG_SET_BITS(sh, ADR_OP_MODE1, op_mode, OP_MODE1_MSK);
            break;
        default:
            printk("Does not support set OP mode to HW for VIF %d\n", vif_idx);
            break;
    }
}

static void ssv6006_set_halt_mngq_util_dtim(struct ssv_hw *sh, bool val)
{
#if 0 // invalid after release_20161123063141
    if (val) {
        SMAC_REG_SET_BITS(sh, ADR_MTX_BCN_EN_MISC, 
            MTX_HALT_MNG_UNTIL_DTIM_MSK, MTX_HALT_MNG_UNTIL_DTIM_MSK);
    } else {
        SMAC_REG_SET_BITS(sh, ADR_MTX_BCN_EN_MISC,
            0, MTX_HALT_MNG_UNTIL_DTIM_MSK);
    }
#endif
}

static void ssv6006_set_dur_burst_sifs_g(struct ssv_hw *sh, u32 val)
{

    u32 regval;
    
    SMAC_REG_READ(sh, ADR_MTX_DUR_SIFS_G, &regval); 
    
    regval &= MTX_DUR_BURST_SIFS_G_I_MSK;
    regval |= val << MTX_DUR_BURST_SIFS_G_SFT ;
    SMAC_REG_WRITE(sh, ADR_MTX_DUR_SIFS_G, regval);

}

static void ssv6006_set_dur_slot(struct ssv_hw *sh, u32 val)
{

    u32 regval;

    SMAC_REG_READ(sh, ADR_MTX_DUR_SIFS_G, &regval); 
    
    regval &= MTX_DUR_SLOT_G_I_MSK;
    regval |= val << MTX_DUR_SLOT_G_SFT;
    SMAC_REG_WRITE(sh, ADR_MTX_DUR_SIFS_G, regval);
    
    SMAC_REG_READ(sh, ADR_MTX_DUR_IFS, &regval); 
    
    regval &= MTX_DUR_SLOT_I_MSK;
    regval |= val << MTX_DUR_SLOT_SFT;
    SMAC_REG_WRITE(sh, ADR_MTX_DUR_IFS, regval);

}
      
static void ssv6006_set_sifs(struct ssv_hw *sh, int band)
{
    if (band == INDEX_80211_BAND_2GHZ){
        SET_MTX_DUR_RSP_SIFS_G(10);
    	SET_MTX_DUR_RSP_SIFS_G(0);
    	SET_TX2TX_SIFS(13);
    } else {
    	SET_MTX_DUR_RSP_SIFS_G(16);
    	SET_MTX_DUR_RSP_SIFS_G(6);
    	SET_TX2TX_SIFS(19);
    }
} 
      
static void ssv6006_set_qos_enable(struct ssv_hw *sh, bool val)
{
   //set QoS status
    SMAC_REG_SET_BITS(sh, ADR_GLBLE_SET, 
            (val<<QOS_EN_SFT), QOS_EN_MSK);
    
}

static void ssv6006_set_wmm_param(struct ssv_softc *sc, 
    const struct ieee80211_tx_queue_params *params, u16 queue)
{
    u32 cw;
    u8 hw_txqid = sc->tx.hw_txqid[queue];
    struct ssv_hw *sh = sc->sh;
 
    cw = params->aifs&0xf;
    cw|= ((ilog2(params->cw_min+1))&0xf)<<TXQ1_MTX_Q_ECWMIN_SFT;//8;
    cw|= ((ilog2(params->cw_max+1))&0xf)<<TXQ1_MTX_Q_ECWMAX_SFT;//12;
    cw|= ((params->txop)&0xff)<<TXQ1_MTX_Q_TXOP_LIMIT_SFT;//16;

    SMAC_REG_WRITE(sc->sh, ADR_TXQ0_MTX_Q_AIFSN+0x100*hw_txqid, cw);
    
    // for USB. AIFS = 10 use generic tx id setting and start tx resource check
    if (params->aifs == 10) {
            u32 dev_type = 0;
    
        dev_type = HCI_DEVICE_TYPE(sc->sh->hci.hci_ctrl);
        if (dev_type == SSV_HWIF_INTERFACE_USB) {
            sc->sh->cfg.usb_hw_resource = ( USB_HW_RESOURCE_CHK_TXID 
                                          | USB_HW_RESOURCE_CHK_TXPAGE);
            HAL_INIT_TX_CFG(sh);
            HAL_INIT_RX_CFG(sh);
            ssv6006_set_page_id(sh);
        }
    }
}

static void ssv6006_update_page_id(struct ssv_hw *sh)
{
    HAL_INIT_TX_CFG(sh);
    HAL_INIT_RX_CFG(sh);
    ssv6006_set_page_id(sh);   
}
// allocate pbuf    
static u32 ssv6006_alloc_pbuf(struct ssv_softc *sc, int size, int type)
{
    u32 regval, pad;
    int cnt = MAX_RETRY_COUNT;
    int page_cnt = (size + ((1 << HW_MMU_PAGE_SHIFT) - 1)) >> HW_MMU_PAGE_SHIFT;

    regval = 0;

    mutex_lock(&sc->mem_mutex);
    
    //brust could be dividen by 4
    pad = size%4;
    size += pad;

    do{
        //printk("[A] ssv6xxx_pbuf_alloc\n");

        SMAC_REG_WRITE(sc->sh, ADR_WR_ALC, (size | (type << 16)));
        SMAC_REG_READ(sc->sh, ADR_WR_ALC, &regval);
        
        if (regval == 0) {
            cnt--;
            msleep(1);
        }
        else
            break;
                
    } while (cnt);

    // If TX buffer is allocated, AMPDU maximum size m
    if (type == TX_BUF) {
        sc->sh->tx_page_available -= page_cnt;
        sc->sh->page_count[PACKET_ADDR_2_ID(regval)] = page_cnt;
    }

    mutex_unlock(&sc->mem_mutex);

    if (regval == 0)
        dev_err(sc->dev, "Failed to allocate packet buffer of %d bytes in %d type.",
                size, type);
    else {
        dev_info(sc->dev, "Allocated %d type packet buffer of size %d (%d) at address %x.\n",
                 type, size, page_cnt, regval);
    }
    
    return regval;
}

static inline bool ssv6006_mcu_input_full(struct ssv_softc *sc)
{
    u32 regval=0;
    
    SMAC_REG_READ(sc->sh, ADR_MCU_STATUS, &regval);
    return (CH0_FULL_MSK & regval);
}

// free pbuf
static bool ssv6006_free_pbuf(struct ssv_softc *sc, u32 pbuf_addr)
{
    u32  regval=0;
    u16  failCount=0;
    u8  *p_tx_page_cnt = &sc->sh->page_count[PACKET_ADDR_2_ID(pbuf_addr)];
    
    while (ssv6006_mcu_input_full(sc))
    {
        if (failCount++ < 1000) continue;
            printk("=============>ERROR!!MAILBOX Block[%d]\n", failCount);
            return false;
    } //Wait until input queue of cho is not full.

    mutex_lock(&sc->mem_mutex);

    // {HWID[3:0], PKTID[6:0]}
    regval = ((M_ENG_TRASH_CAN << HW_ID_OFFSET) |(pbuf_addr >> ADDRESS_OFFSET));
    
    printk("[A] ssv6xxx_pbuf_free addr[%08x][%x]\n", pbuf_addr, regval);
    SMAC_REG_WRITE(sc->sh, ADR_CH0_TRIG_1, regval);

    if (*p_tx_page_cnt)
    {
        sc->sh->tx_page_available += *p_tx_page_cnt;
        *p_tx_page_cnt = 0;
    }

    mutex_unlock(&sc->mem_mutex);
    
    return true;
}

static void ssv6006_ampdu_auto_crc_en(struct ssv_hw *sh)
{
    // Enable HW_AUTO_CRC_32 ======================================
    SMAC_REG_SET_BITS(sh, ADR_MTX_MISC_EN, (0x1 << MTX_AMPDU_CRC8_AUTO_SFT),
                    MTX_AMPDU_CRC8_AUTO_MSK);
}

static void ssv6006_set_rx_ba(struct ssv_hw *sh, bool on, u8 *ta,
        u16 tid, u16 ssn, u8 buf_size)
{
    if (on) {
        //turn on ba session
        SMAC_REG_WRITE(sh, ADR_BA_CTRL, 
            (1 << BA_AGRE_EN_SFT)| (0x01 << BA_CTRL_SFT));
    } else {
        //turn off ba session
        if (sh->sc->rx_ba_session_count == 0)
            SMAC_REG_WRITE(sh, ADR_BA_CTRL, 0x0);
    }  
}

static u8 ssv6006_read_efuse(struct ssv_hw *sh, u8 *pbuf)
{
    // ToDo Liam: comment out efuse routine. let all read out val be 0 
#if 0
    //extern struct ssv6xxx_cfg ssv_cfg;
    u32 val, i , j ;

    SMAC_REG_WRITE(sh, ADR_EFUSE_SPI_RD0_EN, 0x1);
    SMAC_REG_READ(sh, ADR_EFUSE_SPI_RDATA_0, &val);
    sh->cfg.chip_identity = val;

	SMAC_REG_WRITE(sh, ADR_EFUSE_SPI_RD1_EN, 0x1);
    SMAC_REG_READ(sh, ADR_EFUSE_SPI_RDATA_1, &val);
 
    for (i = 0; i < (EFUSE_MAX_SECTION_MAP); i++)
    {
        SMAC_REG_WRITE(sh, ADR_EFUSE_SPI_RD1_EN+i*4, 0x1);
        SMAC_REG_READ(sh, ADR_EFUSE_SPI_RDATA_1+i*4, &val);
        for ( j = 0; j < 4; j++) 
            *pbuf++ = ((val >> j*8) & 0xff);
    }


#else

    u32 i, j;
    
    sh->cfg.chip_identity = 0;
    
    for (i = 0; i < (EFUSE_MAX_SECTION_MAP); i++)
    {
        for ( j = 0; j < 4; j++) 
            *pbuf++ = 0;
    }    
#endif
    return 1;

}


#define CLK_SRC_SYNTH_40M  4
#define CLK_80M_PHY        8
static int ssv6006_chg_clk_src(struct ssv_hw *sh)
{
	int ret = 0;

    // phy rf clock source selection; default to digi; after dpll is enabled; 
    // user may switch to 4'b1000 for 80m_phy 0: digi 1: xosc 2: rtc 3: 80m_phy
    // ToDo Liam, it will be automatic in the future;
    
    //ret = SMAC_REG_WRITE(sh, ADR_PHYRF_CLK_SEL, CLK_80M_PHY);
    //Switch clock to PLL output of RF
    //MAC and MCU clock selection : 
    //   bit[2-0]  01 : RTC clock, 10 : XTAL, 100 : 40MHz clock, 1000: 80MHZ clock
    if (ret == 0) 
       ret = SMAC_REG_WRITE(sh, ADR_CLOCK_SELECTION, CLK_SRC_SYNTH_40M);
    msleep(1);
    return ret;
}

static enum ssv6xxx_beacon_type ssv6006_beacon_get_valid_cfg(struct ssv_hw *sh)
{
	u32 regval =0;
	
	SMAC_REG_READ(sh, ADR_MTX_BCN_MISC, &regval);
	
	regval &= MTX_BCN_CFG_VLD_MSK;
	regval = regval >> MTX_BCN_CFG_VLD_SFT;
	//get MTX_BCN_CFG_VLD
	
	if(regval==0x2 || regval == 0x0)//bcn 0 is availabke to use.
		return SSV6xxx_BEACON_0;
	else if(regval==0x1)//bcn 1 is availabke to use.
		return SSV6xxx_BEACON_1;
	else
		printk("=============>ERROR!!drv_bcn_reg_available\n");//ASSERT(FALSE);// 11 error happened need check with ASIC.
		
	return SSV6xxx_BEACON_0;     
}

static void ssv6006_set_beacon_reg_lock(struct ssv_hw *sh, bool val)
{
	struct ssv_softc *sc = sh->sc;
	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
	    "ssv6xxx_beacon_reg_lock   val[0x:%08x]\n ", val);
	SMAC_REG_SET_BITS(sh, ADR_MTX_BCN_MISC, 
	    val<<MTX_BCN_PKTID_CH_LOCK_SFT, MTX_BCN_PKTID_CH_LOCK_MSK);
}
                                                                                       
static void ssv6006_set_beacon_id_dtim(struct ssv_softc *sc,
        enum ssv6xxx_beacon_type avl_bcn_type, int dtim_offset)
{
#define BEACON_HDR_LEN 24
    struct ssv_hw *sh = sc->sh;

    dtim_offset -= BEACON_HDR_LEN;
    if (avl_bcn_type == SSV6xxx_BEACON_1){
        
        SET_MTX_BCN_PKT_ID1(PBUF_MapPkttoID(sc->beacon_info[avl_bcn_type].pubf_addr));
        SET_MTX_DTIM_OFST1(dtim_offset);
    } else {
        
        SET_MTX_BCN_PKT_ID0(PBUF_MapPkttoID(sc->beacon_info[avl_bcn_type].pubf_addr));
        SET_MTX_DTIM_OFST0(dtim_offset);
    }
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
        " update beacon %d, pktid %d, dtim offset %d \n", avl_bcn_type,
        sc->beacon_info[avl_bcn_type].pubf_addr, dtim_offset);
	
}

static void ssv6006_fill_beacon(struct ssv_softc *sc, u32 regaddr, struct sk_buff *skb)
{
    u8 *beacon = skb->data;
    u32 i, j, val;
    
    int size;
    
    size = (skb->len)/4;
    
    for(i = 0; i < size; i++){
        val = 0;
        for ( j = 0; j < 4; j ++) { 
            val += (*beacon++) << j*8;  // convert u8 to u32
        }
		dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON, "[%08x] ", val );	
		SMAC_REG_WRITE(sc->sh, regaddr+i*4, val);
	}   
		dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON, "\n");
    
}    

static bool ssv6006_beacon_enable(struct ssv_softc *sc, bool bEnable)
{
    u32 regval = 0;
    bool ret = 0;

	//If there is no beacon set to register, beacon could not be turn on.
	if (bEnable && !sc->beacon_usage){
		printk("[A] Reject to set beacon!!!. ssv6xxx_beacon_enable bEnable[%d] sc->beacon_usage[%d]\n",
		    bEnable ,sc->beacon_usage);
        sc->enable_beacon = BEACON_WAITING_ENABLED;
        return ret;
	}

	if((bEnable && (BEACON_ENABLED & sc->enable_beacon))||
        (!bEnable && !sc->enable_beacon)){
		printk("[A] ssv6xxx_beacon_enable bEnable[%d] and sc->enable_beacon[%d] are the same. no need to execute.\n",
		    bEnable ,sc->enable_beacon);
		//return -1;
        if(bEnable){
            printk("        Ignore enable beacon cmd!!!!\n");
            return ret;
        }
	}
	
	SMAC_REG_READ(sc->sh, ADR_MTX_BCN_EN_MISC, &regval);
	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
	    "[A] ssv6xxx_beacon_enable read misc reg val [%08x]\n", regval);

	regval &= MTX_BCN_TIMER_EN_I_MSK;
	
	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
	    "[A] ssv6xxx_beacon_enable read misc reg val [%08x]\n", regval);

	regval |= (bEnable << MTX_BCN_TIMER_EN_SFT);

	ret = SMAC_REG_WRITE(sc->sh, ADR_MTX_BCN_EN_MISC, regval);

	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
	    "[A] ssv6xxx_beacon_enable read misc reg val [%08x]\n", regval);

    sc->enable_beacon = (bEnable==true)?BEACON_ENABLED:0;
    
    return ret;
}

static void ssv6006_set_beacon_info(struct ssv_hw *sh, u8 beacon_interval, u8 dtim_cnt)
{
    struct ssv_softc *sc = sh->sc;  

	//if default is 0 set to our default
	if(beacon_interval==0)
		beacon_interval = 100;

	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
	    "[A] BSS_CHANGED_BEACON_INT beacon_int[%d] dtim_cnt[%d]\n", beacon_interval, (dtim_cnt));
    SET_MTX_BCN_PERIOD(beacon_interval);
    SET_MTX_DTIM_NUM(dtim_cnt);   //=DTIM_Period-1
    SET_MTX_BCN_AUTO_SEQ_NO(1);     //seqnum auto-fill
    SET_MTX_TIME_STAMP_AUTO_FILL(1);//timestamp auto-fill

}

static bool ssv6006_get_bcn_ongoing(struct ssv_hw *sh)
{
    u32 regval;

	SMAC_REG_READ(sh, ADR_MTX_BCN_MISC, &regval);
	
	return ((MTX_AUTO_BCN_ONGOING_MSK & regval) >> MTX_AUTO_BCN_ONGOING_SFT);
}

static void ssv6006_beacon_loss_enable(struct ssv_hw *sh)
{
	SET_RG_RX_MONITOR_ON(1);
}

static void ssv6006_beacon_loss_disable(struct ssv_hw *sh)
{
	SET_RG_RX_MONITOR_ON(0);
}

static void ssv6006_beacon_loss_config(struct ssv_hw *sh, u16 beacon_int, const u8 *bssid)
{
    struct ssv_softc *sc = sh->sc;  
	u32   mac_31_0;
	u16   mac_47_32;

	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON, 
		"%s(): beacon_int %x, bssid %02x:%02x:%02x:%02x:%02x:%02x\n", 
		__FUNCTION__, beacon_int, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

	memcpy(&mac_31_0, bssid, 4);
	memcpy(&mac_47_32, bssid+4, 2);

	ssv6006_beacon_loss_disable(sh);
	SET_RG_RX_BEACON_INTERVAL(beacon_int);
	SET_RG_RX_BEACON_LOSS_CNT_LMT(10);

	SET_RG_RX_PKT_FC(0x0080);
	SET_RG_RX_PKT_ADDR1_ON(0);
	SET_RG_RX_PKT_ADDR2_ON(0);
	SET_RG_RX_PKT_ADDR3_ON(1);
	SET_RG_RX_PKT_ADDR3_31_0((u32)mac_31_0);
	SET_RG_RX_PKT_ADDR3_47_32((u16)mac_47_32);

	ssv6006_beacon_loss_enable(sh);
	return;
}

static void ssv6006_update_txq_mask(struct ssv_hw *sh, u32 txq_mask)
{
     SMAC_REG_SET_BITS(sh, ADR_MTX_MISC_EN,
        (txq_mask << MTX_HALT_Q_MB_SFT), MTX_HALT_Q_MB_MSK);
}

static void ssv6006_readrg_hci_inq_info(struct ssv_hw *sh, int *hci_used_id, int *tx_use_page)
{
    int ret = 0;
    u32 regval = 0;

    ret = SMAC_REG_READ(sh, ADR_RD_IN_FFCNT1, &regval);
    if (ret == 0) 
        *hci_used_id = ((regval & FF1_CNT_MSK) >> FF1_CNT_SFT);
    *tx_use_page = GET_TX_PAGE_USE_7_0;    
}

#define MAX_HW_TXQ_INFO_LEN     2
static bool ssv6006_readrg_txq_info(struct ssv_hw *sh, u32 *txq_info, int *hci_used_id)
{
    int ret = 0;
    u32 addr[MAX_HW_TXQ_INFO_LEN], value[MAX_HW_TXQ_INFO_LEN];

    addr[0] = ADR_TX_ID_ALL_INFO;
    addr[1] = ADR_RD_IN_FFCNT1; //get hci input used txid
    
    ret = SMAC_BURST_REG_READ(sh, addr, value, MAX_HW_TXQ_INFO_LEN);
    if (ret == 0) {
        *txq_info = value[0];
        *hci_used_id = ((value[1] & FF1_CNT_MSK) >> FF1_CNT_SFT);
    }

    return ret;
}

static bool ssv6006_readrg_txq_info2(struct ssv_hw *sh, u32 *txq_info2, int *hci_used_id)
{
    int ret = 0;
    u32 addr[MAX_HW_TXQ_INFO_LEN], value[MAX_HW_TXQ_INFO_LEN];

    addr[0] = ADR_TX_ID_ALL_INFO2;
    addr[1] = ADR_RD_IN_FFCNT1; //get hci input used txid
    
    ret = SMAC_BURST_REG_READ(sh, addr, value, MAX_HW_TXQ_INFO_LEN);
    if (ret == 0) {
        *txq_info2 = value[0];
        *hci_used_id = ((value[1] & FF1_CNT_MSK) >> FF1_CNT_SFT);
    }

    return ret;
}

static bool ssv6006_dump_wsid(struct ssv_hw *sh)
{
    // ToDo Liam
    // These register definitions should be redefined to fit turismo usage.
    const u32 reg_wsid[] = {ADR_WSID0, ADR_WSID1, ADR_WSID2, ADR_WSID3, 
                          ADR_WSID4, ADR_WSID5, ADR_WSID6, ADR_WSID7};    
    const u8 *op_mode_str[]={"STA", "AP", "AD-HOC", "WDS"};
    const u8 *ht_mode_str[]={"Non-HT", "HT-MF", "HT-GF", "RSVD"}; 
    u32 regval;
    int s;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;    

    // ToDO Liam
    // s < 2 should changed to SSV6006_NUM_HW_STA when above register define extended.
    for (s = 0; s < SSV6006_NUM_HW_STA; s++) {
       
        SMAC_REG_READ(sh, reg_wsid[s], &regval);
        snprintf_res(cmd_data, "==>WSID[%d]\n\tvalid[%d] qos[%d] op_mode[%s] ht_mode[%s]\n",
            s, regval&0x1, (regval>>1)&0x1, op_mode_str[((regval>>2)&3)], ht_mode_str[((regval>>4)&3)]);
                                               
        SMAC_REG_READ(sh, reg_wsid[s]+4, &regval);
        snprintf_res(cmd_data, "\tMAC[%02x:%02x:%02x:%02x:",
               (regval&0xff), ((regval>>8)&0xff), ((regval>>16)&0xff), ((regval>>24)&0xff));

        SMAC_REG_READ(sh, reg_wsid[s]+8, &regval);
        snprintf_res(cmd_data, "%02x:%02x]\n",
               (regval&0xff), ((regval>>8)&0xff));                                     
    }

    return 0;
}

static bool ssv6006_dump_decision(struct ssv_hw *sh)
{
    u32 addr, regval;
    int s;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data; 
    
    snprintf_res(cmd_data, ">> Decision Table:\n");
    for(s = 0, addr = ADR_MRX_FLT_TB0; s < 16; s++, addr+=4) {
        SMAC_REG_READ(sh, addr, &regval);
        snprintf_res(cmd_data, "   [%d]: ADDR[0x%08x] = 0x%08x\n",
            s, addr, regval);
    }
    snprintf_res(cmd_data, "\n\n>> Decision Mask:\n");
    for (s = 0, addr = ADR_MRX_FLT_EN0; s < 9; s++, addr+=4) {
        SMAC_REG_READ(sh, addr, &regval);
        snprintf_res(cmd_data, "   [%d]: ADDR[0x%08x] = 0x%08x\n",
            s, addr, regval);
    }
    snprintf_res(cmd_data, "\n\n");
    return 0;
}

static u32 ssv6006_get_ffout_cnt(u32 value, int tag)
{
	switch (tag) {
		case M_ENG_CPU: 	
			return ((value & FFO0_CNT_MSK) >> FFO0_CNT_SFT);
		case M_ENG_HWHCI: 	
			return ((value & FFO1_CNT_MSK) >> FFO1_CNT_SFT);
		case M_ENG_ENCRYPT:	
			return ((value & FFO3_CNT_MSK) >> FFO3_CNT_SFT); 	
		case M_ENG_MACRX:  	
			return ((value & FFO4_CNT_MSK) >> FFO4_CNT_SFT);
		case M_ENG_MIC: 	
			return ((value & FFO5_CNT_MSK) >> FFO5_CNT_SFT);
		case M_ENG_TX_EDCA0: 	
			return ((value & FFO6_CNT_MSK) >> FFO6_CNT_SFT);
		case M_ENG_TX_EDCA1:	
			return ((value & FFO7_CNT_MSK) >> FFO7_CNT_SFT);
		case M_ENG_TX_EDCA2:	
			return ((value & FFO8_CNT_MSK) >> FFO8_CNT_SFT);
		case M_ENG_TX_EDCA3:	
			return ((value & FFO9_CNT_MSK) >> FFO9_CNT_SFT);
		case M_ENG_TX_MNG:	
			return ((value & FFO10_CNT_MSK) >> FFO10_CNT_SFT);
		case M_ENG_ENCRYPT_SEC:	
			return ((value & FFO11_CNT_MSK) >> FFO11_CNT_SFT);
		case M_ENG_MIC_SEC:	
			return ((value & FFO12_CNT_MSK) >> FFO12_CNT_SFT);
		case M_ENG_TRASH_CAN:	
			return ((value & FFO15_CNT_MSK) >> FFO15_CNT_SFT);
		default:
			return 0;
	}
}

static u32 ssv6006_get_in_ffcnt(u32 value, int tag)
{
	switch (tag) {
		case M_ENG_CPU: 	
			return ((value & FF0_CNT_MSK) >> FF0_CNT_SFT);
		case M_ENG_HWHCI: 	
			return ((value & FF1_CNT_MSK) >> FF1_CNT_SFT);
		case M_ENG_ENCRYPT:	
			return ((value & FF3_CNT_MSK) >> FF3_CNT_SFT); 	
		case M_ENG_MACRX:  	
			return ((value & FF4_CNT_MSK) >> FF4_CNT_SFT);
		case M_ENG_MIC: 	
			return ((value & FF5_CNT_MSK) >> FF5_CNT_SFT);
		case M_ENG_TX_EDCA0: 	
			return ((value & FF6_CNT_MSK) >> FF6_CNT_SFT);
		case M_ENG_TX_EDCA1:	
			return ((value & FF7_CNT_MSK) >> FF7_CNT_SFT);
		case M_ENG_TX_EDCA2:	
			return ((value & FF8_CNT_MSK) >> FF8_CNT_SFT);
		case M_ENG_TX_EDCA3:	
			return ((value & FF9_CNT_MSK) >> FF9_CNT_SFT);
		case M_ENG_TX_MNG:	
			return ((value & FF10_CNT_MSK) >> FF10_CNT_SFT);
		case M_ENG_ENCRYPT_SEC:	
			return ((value & FF11_CNT_MSK) >> FF11_CNT_SFT);
		case M_ENG_MIC_SEC:	
			return ((value & FF12_CNT_MSK) >> FF12_CNT_SFT);
		case M_ENG_TRASH_CAN:	
			return ((value & FF15_CNT_MSK) >> FF15_CNT_SFT);
		default:
			return 0;
	}
}

static void ssv6006_read_ffout_cnt(struct ssv_hw *sh, 
    u32 *value, u32 *value1, u32 *value2)
{
    SMAC_REG_READ(sh, ADR_RD_FFOUT_CNT1, value);
    SMAC_REG_READ(sh, ADR_RD_FFOUT_CNT2, value1);
    SMAC_REG_READ(sh, ADR_RD_FFOUT_CNT3, value2);    
}

static void ssv6006_read_in_ffcnt(struct ssv_hw *sh, 
    u32 *value, u32 *value1)
{
    SMAC_REG_READ(sh, ADR_RD_IN_FFCNT1, value);
    SMAC_REG_READ(sh, ADR_RD_IN_FFCNT2, value1);   
}

static void ssv6006_read_id_len_threshold(struct ssv_hw *sh, 
    u32 *tx_len, u32 *rx_len)
{
	u32 regval = 0;

    if(SMAC_REG_READ(sh, ADR_ID_LEN_THREADSHOLD2, &regval));
	*tx_len = ((regval & TX_ID_ALC_LEN_MSK) >> TX_ID_ALC_LEN_SFT);
	*rx_len = ((regval & RX_ID_ALC_LEN_MSK) >> RX_ID_ALC_LEN_SFT);
}

static void ssv6006_read_tag_status(struct ssv_hw *sh, 
    u32 *ava_status)
{
	u32 regval = 0;
    
	if(SMAC_REG_READ(sh, ADR_TAG_STATUS, &regval));
	*ava_status = ((regval & AVA_TAG_MSK) >> AVA_TAG_SFT);
}

static void ssv6006_reset_mib(struct ssv_hw *sh)
{

    SMAC_REG_WRITE(sh, ADR_MIB_EN, 0);
    msleep(1);
    SMAC_REG_WRITE(sh, ADR_MIB_EN, 0xffffffff);

    HAL_RESET_MIB_PHY(sh);
}
   
static void ssv6006_list_mib(struct ssv_hw *sh)
{
    u32 addr, value;
    int i;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;  
    
    addr = MIB_REG_BASE;

    for (i = 0; i < 120; i++, addr+=4) {
        SMAC_REG_READ(sh, addr, &value); 
        snprintf_res(cmd_data, "%08x ", value);

        if (((i+1) & 0x07) == 0)
            snprintf_res(cmd_data, "\n");
    }
    snprintf_res(cmd_data, "\n");  
}

static void ssv6006_dump_mib_rx(struct ssv_hw *sh)
{
    u32  value;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;  
    
    snprintf_res(cmd_data, "HWHCI status:\n");
    snprintf_res(cmd_data, "%-12s\t%-12s\t%-12s\t%-12s\n",
        "HCI_RX_PKT_CNT", "HCI_RX_DROP_CNT", "HCI_RX_TRAP_CNT", "HCI_RX_FAIL_CNT");

    snprintf_res(cmd_data, "[%08x]\t", GET_RX_PKT_COUNTER);
    snprintf_res(cmd_data, "[%08x]\t", GET_RX_PKT_DROP_COUNTER);
    snprintf_res(cmd_data, "[%08x]\t", GET_RX_PKT_TRAP_COUNTER);
    snprintf_res(cmd_data, "[%08x]\n\n", GET_HOST_RX_FAIL_COUNTER);
    
    snprintf_res(cmd_data, "MAC RX status:\n");
    snprintf_res(cmd_data, "%-12s\t%-12s\t%-12s\t%-12s\n",
        "MRX_FCS_SUCC", "MRX_FCS_ERR", "MRX_ALC_FAIL", "MRX_MISS");
  
    SMAC_REG_READ(sh, ADR_MRX_FCS_SUCC, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);
    
    SMAC_REG_READ(sh, ADR_MRX_FCS_ERR, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);
    
    SMAC_REG_READ(sh, ADR_MRX_ALC_FAIL, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);

    SMAC_REG_READ(sh, ADR_MRX_MISS, &value); 
    snprintf_res(cmd_data, "[%08x]\n", value);

    snprintf_res(cmd_data, "%-12s\t%-12s\t%-12s\t%-12s\n",
        "MRX_MB_MISS", "MRX_NIDLE_MISS", "LEN_ALC_FAIL", "LEN_CRC_FAIL");

    SMAC_REG_READ(sh, ADR_MRX_MB_MISS, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);
    
    SMAC_REG_READ(sh, ADR_MRX_NIDLE_MISS, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);
    
    SMAC_REG_READ(sh, ADR_DBG_LEN_ALC_FAIL, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);
    
    SMAC_REG_READ(sh, ADR_DBG_LEN_CRC_FAIL, &value); 
    snprintf_res(cmd_data, "[%08x]\n", value);

    snprintf_res(cmd_data, "%-12s\t%-12s\t%-12s\t%-12s\n",
        "DBG_AMPDU_PASS", "DBG_AMPDU_FAIL", "ID_ALC_FAIL1", "ID_ALC_FAIL2");

    SMAC_REG_READ(sh, ADR_DBG_AMPDU_PASS, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);
 
    SMAC_REG_READ(sh, ADR_DBG_AMPDU_FAIL, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);
 
    SMAC_REG_READ(sh, ADR_ID_ALC_FAIL1, &value); 
    snprintf_res(cmd_data, "[%08x]\t", value);
 
    SMAC_REG_READ(sh, ADR_ID_ALC_FAIL2, &value); 
    snprintf_res(cmd_data, "[%08x]\n\n", value);
    
    HAL_DUMP_MIB_RX_PHY(sh);
}

static void ssv6006_dump_mib_tx(struct ssv_hw *sh)
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
     
    snprintf_res(cmd_data, "HWHCI status:\n");
    snprintf_res(cmd_data, "  %-16s  :%08x\t%-16s  :%08x\n", 
        "HCI_TX_ALLOC_CNT", GET_HCI_TX_ALLOC_CNT, "HCI_TX_PKT_CNT", GET_TX_PKT_COUNTER);
    snprintf_res(cmd_data, "  %-16s  :%08x\t%-16s  :%08x\n", 
        "HCI_TX_DROP_CNT", GET_TX_PKT_DROP_COUNTER, "HCI_TX_TRAP_CNT", GET_TX_PKT_TRAP_COUNTER);
    snprintf_res(cmd_data, "  %-16s  :%08x\n\n", "HCI_TX_FAIL_CNT", GET_HOST_TX_FAIL_COUNTER);
    
    snprintf_res(cmd_data, "MAC TX status:\n");
    snprintf_res(cmd_data, "  %-16s  :%08d\t%-16s  :%08d\n", "Tx Group"
        , GET_MTX_GRP,"Tx Fail", GET_MTX_FAIL);
    snprintf_res(cmd_data, "  %-16s  :%08d\t%-16s  :%08d\n", "Tx Retry"
        , GET_MTX_RETRY,"Tx Multi Retry", GET_MTX_MULTI_RETRY);
    snprintf_res(cmd_data, "  %-16s  :%08d\t%-16s  :%08d\n", "Tx RTS success"
        , GET_MTX_RTS_SUCC,"Tx RTS Fail", GET_MTX_RTS_FAIL);
    snprintf_res(cmd_data, "  %-16s  :%08d\t%-16s  :%08d\n", "Tx ACK Fail"
        , GET_MTX_ACK_FAIL,"Tx total count", GET_MTX_FRM);
    snprintf_res(cmd_data, "  %-16s  :%08d\t%-16s  :%08d\n", "Tx ack count"
        , GET_MTX_ACK_TX,"Tx WSID0 success", GET_MTX_WSID0_SUCC);
    snprintf_res(cmd_data, "  %-16s  :%08d\t%-16s  :%08d\n", "Tx WSID0 frame"
        , GET_MTX_WSID0_FRM,"Tx WSID0 retry", GET_MTX_WSID0_RETRY);
    snprintf_res(cmd_data, "  %-16s  :%08d\n", "Tx WSID0 Total "
        , GET_MTX_WSID0_TOTAL);

}

static void ssv6006_cmd_mib(struct ssv_softc *sc, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sc->cmd_data;
    int i, primary = 0, secondary = 0;
    /**
        *  mib [reset|rx|tx|edca]
        * (1) mib reset
        * (2) mib rx
        * (3) mib tx
        * (4) mib edca
        */
    if ((argc == 2) && (!strcmp(argv[1], "reset"))) {
        ssv6006_reset_mib(sc->sh);
        snprintf_res(cmd_data, " => MIB reseted\n");

    } else if ((argc == 2) && (!strcmp(argv[1], "list"))) {
        ssv6006_list_mib(sc->sh);      
    } else if ((argc == 2) && (strcmp(argv[1], "rx") == 0)) {
        ssv6006_dump_mib_rx(sc->sh);
    } else if ((argc == 2) && (strcmp(argv[1], "tx") == 0)) {
        ssv6006_dump_mib_tx(sc->sh);
    } else if ((argc == 2) && (strcmp(argv[1], "edca") == 0)) {
        for (i = 0; i < 5; i++) { // show last five edca stat
            SSV_EDCA_STAT(sc->sh, &primary, &secondary);        
	        snprintf_res(cmd_data, "Primary EDCCA   :channel use percentage %8d %\n", primary);
	        snprintf_res(cmd_data, "Secondary EDCCA :channel use percentage %8d %\n\n\n", secondary);
            msleep(100); // edca update 100ms 
        }
    } else {
        snprintf_res(cmd_data, "mib [reset|list|rx|tx|edca]\n\n");
    }   
}

static void ssv6006_cmd_lpbk_setup_env_sec_talbe(struct ssv_hw *sh)
{
    int i, address = 0;
    u32 temp;
    u32 sec_key_tbl_base = sh->hw_sec_key[0];
    u32 sec_key_tbl = sec_key_tbl_base;
               
    u8 sec_tbl[] = {
        //group key 1
        0x01, 0x03, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0x58, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x87, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x87, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03, 0x59, 0xce, 0x11, 0x7c,
        0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f, 0x53, 0x90, 0xe8, 0x34,
        0x88, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03, 0x88, 0xbb, 0xdd, 0x0a,
        0x7f, 0x6c, 0x52, 0x03, 0x5a, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41,
        0xc7, 0x5d, 0xc4, 0x5f, 0x53, 0x90, 0xe8, 0x34, 0x89, 0xbb, 0xdd, 0x0a,
        0x7f, 0x6c, 0x52, 0x03, 0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x5a, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //group key 2
        0x01, 0x03, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0x58, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x87, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x87, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03, 0x59, 0xce, 0x11, 0x7c,
        0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f, 0x53, 0x90, 0xe8, 0x34,
        0x88, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03, 0x88, 0xbb, 0xdd, 0x0a,
        0x7f, 0x6c, 0x52, 0x03, 0x5a, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41,
        0xc7, 0x5d, 0xc4, 0x5f, 0x53, 0x90, 0xe8, 0x34, 0x89, 0xbb, 0xdd, 0x0a,
        0x7f, 0x6c, 0x52, 0x03, 0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x5a, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //sta1 wep64bit
        0x00, 0x01, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x5e, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //sta2 wep128bit
        0x00, 0x02, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x5e, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //sta3 tkip
        0x00, 0x03, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x5e, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //sta4 aes
        0x00, 0x04, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x5e, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
    };

    address = sec_key_tbl;
    for (i=0; i < sizeof(sec_tbl); i+=4) { 
        memcpy(&temp, &sec_tbl[i], sizeof(u32)); 
        SMAC_REG_WRITE(sh, address, temp);
        address += 4;
    }
}

static void ssv6006_cmd_loopback_setup_env(struct ssv_hw *sh)
{
    u32 mac0, mac1;
    
    //phy enable
    SSV_PHY_ENABLE(sh, 1);

    // HCI setup
    SET_RX_2_HOST(1);
    SET_TX_INFO_SIZE(80);
    SET_RX_INFO_SIZE(80);

    // Secutiry setup
    SET_LUT_SEL_V2(1);
    ssv6006_cmd_lpbk_setup_env_sec_talbe(sh);

    // sta mac & bssid & peer
    mac0 = GET_STA_MAC_31_0;
    mac1 = GET_STA_MAC_47_32;
    SET_BSSID_31_0(mac0);
    SET_BSSID_47_32(mac1);
    SET_PEER_MAC0_31_0(mac0);
    SET_PEER_MAC0_47_32(mac1);
    SET_VALID0(1);

    // mrx packet flow
    HAL_SET_RX_FLOW(sh, RX_DATA_FLOW, RX_CIPHER_MIC_HCI);
    HAL_SET_RX_FLOW(sh, RX_MGMT_FLOW, RX_HCI);
    HAL_SET_RX_FLOW(sh, RX_CTRL_FLOW, RX_HCI);
   
    
    // Mac lpbk setup
    if (sh->cfg.lpbk_type == SSV6006_CMD_LPBK_TYPE_MAC) {
        // hci lpbk default
        SET_TX_PKT_SEND_TO_RX(0);
        // mac lpbk 
        SET_RG_MAC_LPBK(1);
        SET_MTX_MTX2PHY_SLOW(1);
        SET_MTX_M2M_SLOW_PRD(3);
        // phy lpbk default
        SET_RG_PMDLBK(0);
    } else if (sh->cfg.lpbk_type == SSV6006_CMD_LPBK_TYPE_PHY) {
        // hci lpbk default
        SET_TX_PKT_SEND_TO_RX(0);
        // mac lpbk defualt 
        SET_RG_MAC_LPBK(0);
        SET_MTX_MTX2PHY_SLOW(0);
        SET_MTX_M2M_SLOW_PRD(0);
        // phy lpbk 
        SET_RG_PMDLBK(1);
    } else if (sh->cfg.lpbk_type == SSV6006_CMD_LPBK_TYPE_HCI) {
        // hci lpbk, it is necessary to disable phy
        // Otherwise, the packet from phy will affects the lpbk. 
        SSV_PHY_ENABLE(sh, 0);
        SET_TX_PKT_SEND_TO_RX(1);
        // mac lpbk defualt 
        SET_RG_MAC_LPBK(0);
        SET_MTX_MTX2PHY_SLOW(0);
        SET_MTX_M2M_SLOW_PRD(0);
        // phy lpbk default
        SET_RG_PMDLBK(0);
    }

    // setup lpbk
    msleep(10);
    sh->sc->lpbk_enable = true;
}

static void ssv6006_cmd_mtx_lpbk_mac80211_hdr(struct ssv_hw *sh, struct sk_buff *skb, u8 seq)
{
    struct ieee80211_hdr_3addr *hdr = NULL;
    int i, tx_desc_size = 0, hdrlen = 24;
    u8 macaddr[ETH_ALEN];
    u8 *payload;

    tx_desc_size = SSV_GET_TX_DESC_SIZE(sh);
    memcpy(macaddr, sh->cfg.maddr[0], ETH_ALEN);
    hdr = (struct ieee80211_hdr_3addr *)(skb->data + tx_desc_size);
    memset(hdr, 0, sizeof(struct ieee80211_hdr_3addr));
    hdr->frame_control = cpu_to_le16(IEEE80211_FTYPE_DATA |
                                    IEEE80211_STYPE_QOS_DATA |
                                    IEEE80211_FCTL_TODS);
    memcpy(hdr->addr1, macaddr, ETH_ALEN);
    memcpy(hdr->addr2, macaddr, ETH_ALEN);
    memcpy(hdr->addr3, macaddr, ETH_ALEN);
    hdr->seq_ctrl = seq;
    
    // payload
    payload = (u8 *)hdr + hdrlen;
    for (i = (tx_desc_size + hdrlen); i < skb->len; i++) {
        *payload = (u8)seq;
        payload++;
    }
}   

static void ssv6006_cmd_loopback_start(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data; 
    int tx_desc_size = SSV_GET_TX_DESC_SIZE(sh);
    struct sk_buff *skb;
    int lpbk_pkt_cnt, lpbk_sec_loop_cnt, lpbk_rate_loop_cnt;
    int i, j, k, seq = 0;
    unsigned int len, sec;
    unsigned char rate = 0;
    unsigned char rate_tbl[] = {
        0x00,0x01,0x02,0x03,                        // B mode long preamble [0~3]
        0x11,0x12,0x13,                             // B mode short preamble  [4~6]
        0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,    // G mode [7~14]
        0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,    // N mode HT20 long GI mixed format [15~22]
        0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,    // N mode HT20 short GI mixed format  [23~30]
        0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,    // N mode HT40 long GI mixed format [31~38]
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,    // N mode HT40 short GI mixed format  [39~46]
    };
#define SIZE_RATE_TBL   (sizeof(rate_tbl) / sizeof((rate_tbl)[0]))

    /* Setup sample count */
    lpbk_pkt_cnt = (sh->cfg.lpbk_pkt_cnt == 0) ? 10 : sh->cfg.lpbk_pkt_cnt;
    lpbk_sec_loop_cnt = (sh->cfg.lpbk_sec == SSV6006_CMD_LPBK_SEC_AUTO) ? 5 : 1;
    if (sh->cfg.lpbk_mode) {
        lpbk_rate_loop_cnt = 1;
        rate = ((sh->cfg.rc_rate_idx_set & SSV6006RC_RATE_MSK)>> SSV6006RC_RATE_SFT) |
               (sh->cfg.rc_long_short >> SSV6006RC_LONG_SHORT_SFT) | 
               (sh->cfg.rc_ht40 >> SSV6006RC_20_40_SFT) | 
               (sh->cfg.rc_phy_mode >> SSV6006RC_PHY_MODE_SFT);
    } else {
        lpbk_rate_loop_cnt = SIZE_RATE_TBL;
    }
    
    sc->lpbk_tx_pkt_cnt = lpbk_pkt_cnt * lpbk_sec_loop_cnt * lpbk_rate_loop_cnt;
    sc->lpbk_rx_pkt_cnt = 0;
    sc->lpbk_err_cnt = 0;

    for (i = 0; i < lpbk_sec_loop_cnt; i++) { 
        for (j = 0; j < lpbk_rate_loop_cnt; j++) { 
            for (k = 0; k < lpbk_pkt_cnt; k++) {

                // Set security mode
                sec = (sh->cfg.lpbk_sec == SSV6006_CMD_LPBK_SEC_AUTO) ? (i+1) : sh->cfg.lpbk_sec;
                
                // Set rate
                if (!sh->cfg.lpbk_mode)
                    rate = rate_tbl[j % SIZE_RATE_TBL];

                // generate random payload len
                get_random_bytes(&len, sizeof(unsigned int));
                len = sizeof(struct ieee80211_hdr_3addr) + (len % 2000);
                skb = ssv_skb_alloc(sc, len + tx_desc_size);
                if (!skb) {
		            printk("%s(): Fail to alloc lpbk buffer.\n", __FUNCTION__);
		            goto out;
                }
	            skb_put(skb, len + tx_desc_size);
               
                // update tx_desc & mac80211 hdr
                SSV_FILL_LPBK_TX_DESC(sc, skb, sec, rate);
                ssv6006_cmd_mtx_lpbk_mac80211_hdr(sh, skb, (u8)(seq % 255));
                HCI_SEND(sh, skb, 0);
                seq++;
            }
        }
    }

    // expect test period
    msleep(sc->lpbk_tx_pkt_cnt + 1000);

out:
    if ((sc->lpbk_tx_pkt_cnt == sc->lpbk_rx_pkt_cnt) && (sc->lpbk_err_cnt == 0)) {
        snprintf_res(cmd_data, "\n mtx lpbk: pass, tx/rx pkt_cnt=%d/%d, err_cnt=%d\n", 
            sc->lpbk_tx_pkt_cnt, sc->lpbk_rx_pkt_cnt, sc->lpbk_err_cnt);
    } else {
        snprintf_res(cmd_data, "\n mtx lpbk: fail, tx/rx pkt_cnt=%d/%d, err_cnt=%d\n", 
            sc->lpbk_tx_pkt_cnt, sc->lpbk_rx_pkt_cnt, sc->lpbk_err_cnt);
    }
}

static void ssv6006_cmd_hwinfo(struct ssv_hw *sh, int argc, char *argv[])
{
    return;
}

static void ssv6006_get_rd_id_adr(u32 *id_base_address)
{
    id_base_address[0] = ADR_RD_ID0;
    id_base_address[1] = ADR_RD_ID1;
    id_base_address[2] = ADR_RD_ID2;
    id_base_address[3] = ADR_RD_ID3;
}

static int ssv6006_burst_read_reg(struct ssv_hw *sh, u32 *addr, u32 *buf, u8 reg_amount)
{
    int ret = (-1);
    ret = SMAC_BURST_REG_READ(sh, addr, buf, reg_amount);
	
    return ret;
}

static int ssv6006_burst_write_reg(struct ssv_hw *sh, u32 *addr, u32 *buf, u8 reg_amount)
{
    int ret = (-1);
    ret = SMAC_BURST_REG_WRITE(sh, addr, buf, reg_amount); 	
	
    return ret;
}

static int ssv6006_auto_gen_nullpkt(struct ssv_hw *sh, int hwq)
{
    return -EOPNOTSUPP;
}

static void ssv6006_load_fw_enable_mcu(struct ssv_hw *sh)
{
    // After FW loaded, set IVB to 0, boot from SRAM, enable N10 clock, and release N10
    SET_N10CFG_DEFAULT_IVB(0);
 
    SET_ROM_REBOOT_FROM_SRAM(1);
    SET_CLK_EN_CPUN10(1);
    SET_RESET_N_CPUN10(1); // N10 might be disabled by default. Enable it. 
}

static int ssv6006_load_fw_disable_mcu(struct ssv_hw *sh)
{
    int ret = 0;
    // Before loading FW, reset N10

    SET_RESET_N_CPUN10(0);

    SET_CLK_EN_CPUN10(0);

    SET_MCU_ENABLE(0);
    SET_RG_REBOOT(1);

    return ret;
}

static int ssv6006_load_fw_set_status(struct ssv_hw *sh, u32 status)
{
	return SMAC_REG_WRITE(sh, ADR_TX_SEG, status);
}

static int ssv6006_load_fw_get_status(struct ssv_hw *sh, u32 *status)
{
	return SMAC_REG_READ(sh, ADR_TX_SEG, status);
}

static void ssv6006_load_fw_pre_config_device(struct ssv_hw *sh)
{
	HCI_LOAD_FW_PRE_CONFIG_DEVICE(sh->hci.hci_ctrl);
}

static void ssv6006_load_fw_post_config_device(struct ssv_hw *sh)
{
	HCI_LOAD_FW_POST_CONFIG_DEVICE(sh->hci.hci_ctrl);
}

// Reset CPU (after reset, CPU is stopped)
static int ssv6006_reset_cpu(struct ssv_hw *sh)
{    
    // Keep original interrupt mask
    u32 org_int_mask = GET_MASK_TYPMCU_INT_MAP;

    // Mask all interrupt for CPU
    SET_MASK_TYPMCU_INT_MAP(0xffffffff);
    
    // Reset CPU
    SET_RESET_N_CPUN10(0);
    // Set original interrupt mask back
    SET_MASK_TYPMCU_INT_MAP(org_int_mask);

    return 0;
}

/* Set SRAM mapping mode to:
 * 0: ILM  64KB, DLM 128KB 
 * 1: ILM 160KB, DLM 32KB
 */
static void ssv6006_set_sram_mode(struct ssv_hw *sh, enum SSV_SRAM_MODE mode)
{
    struct ssv_softc *sc = sh->sc;
    
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support set sram mode for this model!! \n");
}

static void ssv6006_enable_usb_acc(struct ssv_softc *sc, u8 epnum)
{
    switch (epnum) {
        case SSV_EP_CMD:
            dev_info(sc->dev, "Enable Command(ep%d) acc\n", SSV_EP_CMD);
            SMAC_REG_SET_BITS(sc->sh, 0x700041AC, (1<<0), 0x1);
            break;
        case SSV_EP_RSP:
            dev_info(sc->dev, "Enable Command Rsp(ep%d) acc\n", SSV_EP_RSP);
            SMAC_REG_SET_BITS(sc->sh, 0x700041AC, (1<<1), 0x2);
            break;
        case SSV_EP_TX:
            dev_info(sc->dev, "Enable TX(ep%d) acc\n", SSV_EP_TX);
            SMAC_REG_SET_BITS(sc->sh, 0x700041AC, (1<<2), 0x4);
            break;
        case SSV_EP_RX:
            dev_info(sc->dev, "Enable RX(ep%d) acc\n", SSV_EP_RX);
            SMAC_REG_SET_BITS(sc->sh, 0x700041AC, (1<<3), 0x8);
            break;
    }
}

static void ssv6006_disable_usb_acc(struct ssv_softc *sc, u8 epnum)
{   
    switch (epnum) {
        case SSV_EP_CMD:
            dev_info(sc->dev, "Disable Command(ep%d) acc\n", SSV_EP_CMD);
            SMAC_REG_SET_BITS(sc->sh, 0x700041AC, (0<<0), 0x1);
            break;
        case SSV_EP_RSP:
            dev_info(sc->dev, "Disable Command Rsp(ep%d) acc\n", SSV_EP_RSP);
            SMAC_REG_SET_BITS(sc->sh, 0x700041AC, (0<<1), 0x2);
            break;
        case SSV_EP_TX:
            dev_info(sc->dev, "Disable TX(ep%d) acc\n", SSV_EP_TX);
            SMAC_REG_SET_BITS(sc->sh, 0x700041AC, (0<<2), 0x4);
            break;
        case SSV_EP_RX:
            dev_info(sc->dev, "Disable RX(ep%d) acc\n", SSV_EP_RX);
            SMAC_REG_SET_BITS(sc->sh, 0x700041AC, (0<<3), 0x8);
            break;
    }
}

// Set Hardware support USB Link Power Management (LPM)
static void ssv6006_set_usb_lpm(struct ssv_softc *sc, u8 enable)
{
    int dev_type = HCI_DEVICE_TYPE(sc->sh->hci.hci_ctrl);
    int i = 0;

    if (dev_type != SSV_HWIF_INTERFACE_USB) {
        printk("Not support set USB LPM for this model!!\n");
        return;
    }
    
    for(i=0 ; i<SSV6200_MAX_VIF ;i++) {
        if (sc->vif_info[i].vif == NULL)
            continue;
        
        if (sc->vif_info[i].vif->type == NL80211_IFTYPE_AP) {
            printk("Force to disable USB LPM function due to exist AP interface\n");
            //TODO: don't use magic number, wait USB register address define to replace it
            SMAC_REG_SET_BITS(sc->sh, 0x70004008, (0 << 11), 0x800);
            return;
        }
    }        

    printk("Set USB LPM support to %d\n", enable);
    //TODO: don't use magic number, wait USB register address define to replace it
    SMAC_REG_SET_BITS(sc->sh, 0x70004008, (enable << 11), 0x800);
}

static int ssv6006_jump_to_rom(struct ssv_softc *sc)
{
    struct ssv_hw *sh = sc->sh;
    int ret = 0;
    
    dev_info(sc->dev, "Jump to ROM\n");

    SET_RESET_N_CPUN10(0); //reset CPU

    SET_ROM_REBOOT_FROM_SRAM(0); //swicth to boot from ILM mode
    SET_N10CFG_DEFAULT_IVB(0x4); //set IVB to ROM code address: 0x40000
    SET_RESET_N_CPUN10(1); //release reset to let CPU run
    
    msleep(50); // Wait ROM code ready

    return ret;
}
 
static void ssv6006_init_gpio_cfg(struct ssv_hw *sh)
{
    /* GPIO 15 will cause the RF interfence. */
    SET_MANUAL_IO(0x8000);
}

void ssv_attach_ssv6006_mac(struct ssv_hal_ops *hal_ops)
{
    hal_ops->init_mac = ssv6006_init_mac;
    hal_ops->reset_sysplf = ssv6006_reset_sysplf;

    hal_ops->init_hw_sec_phy_table = ssv6006_init_hw_sec_phy_table;
    hal_ops->write_mac_ini = ssv6006_write_mac_ini;

    hal_ops->set_rx_flow = ssv6006_set_rx_flow;
    hal_ops->set_rx_ctrl_flow = ssv6006_set_rx_ctrl_flow;
    hal_ops->set_macaddr = ssv6006_set_macaddr;
    hal_ops->set_bssid = ssv6006_set_bssid;
    hal_ops->get_ic_time_tag = ssv6006_get_ic_time_tag;
    hal_ops->get_chip_id = ssv6006_get_chip_id;
    
    hal_ops->save_hw_status = ssv6006_save_hw_status;
    hal_ops->set_hw_wsid = ssv6006_set_hw_wsid;
    hal_ops->del_hw_wsid = ssv6006_del_hw_wsid;


    hal_ops->set_aes_tkip_hw_crypto_group_key = ssv6006_set_aes_tkip_hw_crypto_group_key;
    hal_ops->write_pairwise_keyidx_to_hw = ssv6006_write_pairwise_keyidx_to_hw;
    hal_ops->write_group_keyidx_to_hw = ssv6006_write_hw_group_keyidx;
    hal_ops->write_pairwise_key_to_hw = ssv6006_write_pairwise_key_to_hw;
    hal_ops->write_group_key_to_hw = ssv6006_write_group_key_to_hw;
    hal_ops->write_key_to_hw = ssv6006_write_key_to_hw;
    hal_ops->set_pairwise_cipher_type = ssv6006_set_pairwise_cipher_type;
    hal_ops->set_group_cipher_type = ssv6006_set_group_cipher_type;
    
#ifdef CONFIG_PM
	hal_ops->save_clear_trap_reason = ssv6006_save_clear_trap_reason;
	hal_ops->restore_trap_reason = ssv6006_restore_trap_reason;
	hal_ops->pmu_awake = ssv6006_pmu_awake;
#endif

    hal_ops->store_wep_key = ssv6006_store_wep_key;   
    
    hal_ops->set_replay_ignore = ssv6006_set_replay_ignore;
    hal_ops->update_decision_table_6 = ssv6006_update_decision_table_6;
    hal_ops->update_decision_table = ssv6006_update_decision_table;
    hal_ops->get_fw_version = ssv6006_get_fw_version;
    hal_ops->set_op_mode = ssv6006_set_op_mode;
    hal_ops->set_halt_mngq_util_dtim = ssv6006_set_halt_mngq_util_dtim;
    hal_ops->set_dur_burst_sifs_g = ssv6006_set_dur_burst_sifs_g;
    hal_ops->set_dur_slot = ssv6006_set_dur_slot;
    hal_ops->set_sifs = ssv6006_set_sifs;
    hal_ops->set_qos_enable = ssv6006_set_qos_enable;
    hal_ops->set_wmm_param = ssv6006_set_wmm_param;
    hal_ops->update_page_id = ssv6006_update_page_id;
    
    hal_ops->alloc_pbuf = ssv6006_alloc_pbuf;
    hal_ops->free_pbuf = ssv6006_free_pbuf;
    hal_ops->ampdu_auto_crc_en = ssv6006_ampdu_auto_crc_en;
    hal_ops->set_rx_ba = ssv6006_set_rx_ba;
    hal_ops->read_efuse = ssv6006_read_efuse;
    hal_ops->chg_clk_src = ssv6006_chg_clk_src;
    
    hal_ops->beacon_get_valid_cfg = ssv6006_beacon_get_valid_cfg;
    hal_ops->set_beacon_reg_lock = ssv6006_set_beacon_reg_lock;
    hal_ops->set_beacon_id_dtim = ssv6006_set_beacon_id_dtim;
    hal_ops->fill_beacon = ssv6006_fill_beacon;
    hal_ops->beacon_enable = ssv6006_beacon_enable;
    hal_ops->set_beacon_info = ssv6006_set_beacon_info;
    hal_ops->get_bcn_ongoing = ssv6006_get_bcn_ongoing;
    hal_ops->beacon_loss_enable = ssv6006_beacon_loss_enable;
    hal_ops->beacon_loss_disable = ssv6006_beacon_loss_disable;
    hal_ops->beacon_loss_config = ssv6006_beacon_loss_config;
    
    hal_ops->update_txq_mask = ssv6006_update_txq_mask;
    hal_ops->readrg_hci_inq_info = ssv6006_readrg_hci_inq_info;
    hal_ops->readrg_txq_info = ssv6006_readrg_txq_info;
    hal_ops->readrg_txq_info2 = ssv6006_readrg_txq_info2;
    
    hal_ops->dump_wsid = ssv6006_dump_wsid;
    hal_ops->dump_decision = ssv6006_dump_decision;
	hal_ops->get_ffout_cnt = ssv6006_get_ffout_cnt;
	hal_ops->get_in_ffcnt = ssv6006_get_in_ffcnt;
    hal_ops->read_ffout_cnt = ssv6006_read_ffout_cnt;
    hal_ops->read_in_ffcnt = ssv6006_read_in_ffcnt;
    hal_ops->read_id_len_threshold = ssv6006_read_id_len_threshold;
    hal_ops->read_tag_status = ssv6006_read_tag_status;
    hal_ops->cmd_mib = ssv6006_cmd_mib;
    hal_ops->cmd_loopback_start = ssv6006_cmd_loopback_start;
    hal_ops->cmd_loopback_setup_env = ssv6006_cmd_loopback_setup_env;
	hal_ops->cmd_hwinfo = ssv6006_cmd_hwinfo;
    hal_ops->get_rd_id_adr = ssv6006_get_rd_id_adr;
    hal_ops->burst_read_reg = ssv6006_burst_read_reg;
    hal_ops->burst_write_reg = ssv6006_burst_write_reg;
    hal_ops->auto_gen_nullpkt = ssv6006_auto_gen_nullpkt;

	hal_ops->load_fw_enable_mcu = ssv6006_load_fw_enable_mcu;
	hal_ops->load_fw_disable_mcu = ssv6006_load_fw_disable_mcu;
    hal_ops->load_fw_set_status = ssv6006_load_fw_set_status;
    hal_ops->load_fw_get_status = ssv6006_load_fw_get_status;
    hal_ops->load_fw_pre_config_device = ssv6006_load_fw_pre_config_device;
    hal_ops->load_fw_post_config_device = ssv6006_load_fw_post_config_device;
    hal_ops->reset_cpu = ssv6006_reset_cpu;    
    hal_ops->set_sram_mode = ssv6006_set_sram_mode;    
    hal_ops->enable_usb_acc = ssv6006_enable_usb_acc;
    hal_ops->disable_usb_acc = ssv6006_disable_usb_acc;
    hal_ops->set_usb_lpm = ssv6006_set_usb_lpm;    
    hal_ops->jump_to_rom = ssv6006_jump_to_rom;
    hal_ops->init_gpio_cfg = ssv6006_init_gpio_cfg;
 
}

#endif
