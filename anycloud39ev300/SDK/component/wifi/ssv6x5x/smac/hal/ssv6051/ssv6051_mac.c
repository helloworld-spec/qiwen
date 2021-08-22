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
#ifdef SSV_SUPPORT_HAL
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <ssv6200.h>
#include "ssv6051_mac.h"
#include <ssv6200_reg.h>
#include <ssv6200_aux.h>
#include <smac/dev.h>
#include <smac/ssv_rc.h>
#include <hal.h>
#include <ssvdevice/ssv_cmd.h>
#include <hci/hctrl.h>
#include "ssv6051_priv.h"

static u32 ssv6051_alloc_pbuf(struct ssv_softc *sc, int size, int type);
static void ssv6051_write_key_to_hw(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, 
        void *sram_ptr, int wsid, int key_idx, enum SSV6XXX_WSID_SEC key_type);    
static int ssv6051_set_macaddr(struct ssv_hw *sh, int vif_idx);
static int ssv6051_set_bssid(struct ssv_hw *sh, u8 *bssid, int vif_idx);

static const ssv_cabrio_reg ssv6051_mac_ini_table[]=
{
    //-----------------------------------------------------------------------------------------------------------------------------------------
    /* Set wmm parameter to EDCA Q4
     (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
    {ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101},
   
    //---
    //HCI module config
    //keep it in host
    //HCI-enable 4 bits
    //RX to Host(bit2)
    //AUTO_SEQNO enable(bit3)
    //Fill tx queue info in rx pacet(Bit 25)	
    //tx rx packet debug counter enable(bit28) 
    //TX on_demand interrupt control between allocate size and transmit data mismatch
    {ADR_CONTROL, 0x12000006},
   
    //---
    //RX module config
    //(bit 0)                       Enable hardware timestamp for TSF 
    //(bit8~bit15) value(28)  Time stamp write location
    {ADR_RX_TIME_STAMP_CFG, ((28<<MRX_STP_OFST_SFT)|0x01)},
    
    //--
    {ADR_INFO_RATE_OFFSET, 0x00040000},
    //-----------------------------------------------------------------------------------------------------------------------------------------
    //MAC config
    {ADR_GLBLE_SET,
          (0 << OP_MODE_SFT)  |                          /* STA mode by default */
          (0 << SNIFFER_MODE_SFT) |                      /* disable sniffer mode */
          (1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
          (SSV6200_TX_PKT_RSVD_SETTING << TX_PKT_RSVD_SFT) |
          ((u32)(RXPB_OFFSET) << PB_OFFSET_SFT) },      /* set rx packet buffer offset */

    /**
     * Disable tx/rx ether trap table.
     */
    {ADR_TX_ETHER_TYPE_0, 0x00000000},
    {ADR_TX_ETHER_TYPE_1, 0x00000000},
    {ADR_RX_ETHER_TYPE_0, 0x00000000},
    {ADR_RX_ETHER_TYPE_1, 0x00000000}, 
    /**                                               
     * Set reason trap to discard frames.                
     */                                              
    {ADR_REASON_TRAP0, 0x7FBC7F87},
    {ADR_REASON_TRAP1, 0x0000003F},
    #ifndef FW_WSID_WATCH_LIST
    {ADR_TRAP_HW_ID, M_ENG_HWHCI}, /* Trap to HCI */
    #else
    {ADR_TRAP_HW_ID, M_ENG_CPU},   /* Trap to CPU */
    #endif

    /**                                         
     * Reset all wsid table entry to invalid.
     */                                      
    {ADR_WSID0, 0x00000000},  
    {ADR_WSID1, 0x00000000},
    //-----------------------------------------------------------------------------------------------------------------------------------------
    //SDIO interrupt
    /*
     * Set tx interrupt threshold for EACA0 ~ EACA3 queue & low threshold
     */
    // Freddie ToDo: Use resource low instead of id threshold as interrupt source to reduce unecessary
    //Inital soc interrupt Bit13-Bit16(EDCA 0-3) Bit28(Tx resource low)
    {ADR_SDIO_MASK, 0xfffe1fff},
    //BTCX
#ifdef CONFIG_SSV_SUPPORT_BTCX
    /* Set BTCX Parameter*/
    {ADR_BTCX0, COEXIST_EN_MSK|(WIRE_MODE_SZ<<WIRE_MODE_SFT)
        | WIFI_TX_SW_POL_MSK | BT_SW_POL_MSK},    
    {ADR_BTCX1, SSV6200_BT_PRI_SMP_TIME| (SSV6200_BT_STA_SMP_TIME<<BT_STA_SMP_TIME_SFT)
      | (SSV6200_WLAN_REMAIN_TIME<<WLAN_REMAIN_TIME_SFT)},
    {ADR_SWITCH_CTL, BT_2WIRE_EN_MSK},

    /*WIFI_TX_SW_O*/
    {ADR_PAD7, 1},
    /*WIFI_RX_SW_O*/
    {ADR_PAD8, 0}, 
    /*BT_SW_O*/
    {ADR_PAD9, 1},
    /*WLAN_ACT (GPIO_1)*/
    {ADR_PAD25, 1}, 
    /*BT_ACT (GPIO_2)*/
    {ADR_PAD27, 8},
    /*BT_PRI (GPIO_3)*/
    {ADR_PAD28, 8},
#endif
};

int ssv6051_init_mac(struct ssv_hw *sh)
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

    if (sh->cfg.rx_burstread) 
	    sh->rx_mode = RX_BURSTREAD_MODE;
    
/* Setup q4 behavior STA mode-> act as normal queue    
//Enable TSF to be a hw jiffies(add one by us)
  */
    SMAC_REG_SET_BITS(sh, ADR_MTX_BCN_EN_MISC, 
            (0 << MTX_HALT_MNG_UNTIL_DTIM_SFT |(1<<MTX_TSF_TIMER_EN_SFT)),
            (MTX_HALT_MNG_UNTIL_DTIM_MSK | MTX_TSF_TIMER_EN_MSK));
            
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
    SMAC_REG_SET_BITS(sh, ADR_MMU_CTRL, (255<<MMU_SHARE_MCU_SFT), MMU_SHARE_MCU_MSK);

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

    SMAC_REG_SET_BITS(sh, ADR_TRX_ID_THRESHOLD,
            ((sh->tx_info.tx_id_threshold<<TX_ID_THOLD_SFT)|
             (sh->rx_info.rx_id_threshold<<RX_ID_THOLD_SFT)),
            (TX_ID_THOLD_MSK | RX_ID_THOLD_MSK));

    SMAC_REG_SET_BITS(sh, ADR_ID_LEN_THREADSHOLD1,
            ((sh->tx_info.tx_page_threshold<<ID_TX_LEN_THOLD_SFT)|
             (sh->rx_info.rx_page_threshold<<ID_RX_LEN_THOLD_SFT)),
             (ID_TX_LEN_THOLD_MSK | ID_RX_LEN_THOLD_MSK));
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
#ifdef CONFIG_SSV_TX_LOWTHRESHOLD
    //Setting Tx resource low ---ID[0x5] Page[0x15]
    SMAC_REG_WRITE(sh, ADR_TX_LIMIT_INTR, 
		0x80000000 | 
      SSV6200_TX_LOWTHRESHOLD_ID_TRIGGER << 16 |
	  SSV6200_TX_LOWTHRESHOLD_PAGE_TRIGGER);
#else
    //Enable EDCA low threshold 
    SMAC_REG_WRITE(sh, ADR_MB_THRESHOLD6, 0x80000000);
    //Enable EDCA low threshold EDCA-1[8] EDCA-0[4] 
    SMAC_REG_WRITE(sh, ADR_MB_THRESHOLD8, 0x04020000);
    //Enable EDCA low threshold EDCA-3[8] EDCA-2[8] 
    SMAC_REG_WRITE(sh, ADR_MB_THRESHOLD9, 0x00000404);
#endif

//PHY and security table
    ret = HAL_INI_HW_SEC_PHY_TABLE(sc);
    if (ret)
        goto exit;
    
    /**
     * Set ssv6200 mac address and set default BSSID. In hardware reset,
     * we the BSSID to 00:00:00:00:00:00.
     */
    // Freddie ToDo: Set MAC addresss when primary interface is being added.
    ssv6051_set_macaddr(sh, 0);
    ssv6051_set_bssid(sh, null_address, 0);
    
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

    HAL_SET_RX_FLOW(sh, RX_CTRL_FLOW, RX_CPU_HCI);

    HAL_SET_REPLAY_IGNORE(sh, 1); //?? move to init_mac

    /**
     * Set ssv6200 mac decision table for hardware. The table 
     * selection is according to the type of wireless interface: 
     * AP & STA mode.
     */
    HAL_UPDATE_DECISION_TABLE(sc);
   
    SMAC_REG_SET_BITS(sc->sh, ADR_GLBLE_SET, SSV6XXX_OPMODE_STA, OP_MODE_MSK);
exit:
    return ret;
}

void ssv6051_reset_sysplf(struct ssv_hw *sh)
{
    SMAC_REG_SET_BITS(sh, ADR_BRG_SW_RST, (1 << MCU_SW_RST_SFT), MCU_SW_RST_MSK);
}


static struct ssv_hw * ssv6051_alloc_hw (void)
{
    struct ssv_hw *sh;

    sh = kzalloc(sizeof(struct ssv_hw), GFP_KERNEL);
    if (sh == NULL)
        goto out;
    
    memset((void *)sh, 0, sizeof(struct ssv_hw));
   
    sh->page_count = (u8 *)kzalloc(sizeof(u8) * SSV6200_ID_NUMBER, GFP_KERNEL);
    if (sh->page_count == NULL) 
        goto out;
    
    memset(sh->page_count, 0, sizeof(u8) * SSV6200_ID_NUMBER);
    
    return sh;
out:
    if (sh->page_count)
        kfree(sh->page_count);
    if (sh)
        kfree(sh);

    return NULL;
}

int ssv6051_init_hw_sec_phy_table(struct ssv_softc *sc)
{

    struct ssv_hw *sh = sc->sh;
    int i, ret = 0;
#ifdef SSV6200_ECO
    u32 *ptr, temp[0x8];
#else
    u32 *ptr;
#endif
//-----------------------------------------------------------------------------------------------------------------------------------------
//PHY and security table
#ifdef SSV6200_ECO
    SMAC_REG_WRITE(sh, 0xcd010004, 0x1213);
    /**
        * Allocate a hardware packet buffer space. This buffer is for security
        * key caching and phy info space.
        * allocate 8 packet buffer.
        */
    for (i = 0; i < SSV_RC_MAX_STA ;i ++){
        if (i == 0){
            sh->hw_buf_ptr[i] = ssv6051_alloc_pbuf(sc, phy_info_tbl_size+
                                    sizeof(struct ssv6xxx_hw_sec), NOTYPE_BUF);
          if((sh->hw_buf_ptr[i]>>28) != 8){
            //asic pbuf address start from 0x8xxxxxxxx
            printk("opps allocate pbuf error\n");
            WARN_ON(1);	
            ret = 1;
            goto exit;
          }
        } else {
            sh->hw_buf_ptr[i] = ssv6051_alloc_pbuf(sc,
                                    sizeof(struct ssv6xxx_hw_sec), NOTYPE_BUF);
            if((sh->hw_buf_ptr[i]>>28) != 8){
                //asic pbuf address start from 0x8xxxxxxxx
                printk("opps allocate pbuf error\n");
                WARN_ON(1); 
                ret = 1;
                goto exit;
            }
        }
        printk("%s(): ssv6200 reserved space=0x%08x\n", 
            __FUNCTION__, sh->hw_buf_ptr[i]);
    }
    //WSD-124 issue
    //Force to allocate 0x800e0000
    for (i = 0; i < SSV_RC_MAX_STA; i++) {
        temp[i] = 0;
        temp[i] = ssv6051_alloc_pbuf(sc, 256,NOTYPE_BUF);
    }
    for (i = 0; i < SSV_RC_MAX_STA; i++) {
        if(temp[i] == 0x800e0000)
            printk("0x800e0000\n");
        else
            HAL_FREE_PBUF(sc,temp[i]);
    }
#else // SSV6200_ECO
    /**
        * Allocate a hardware packet buffer space. This buffer is for security
        * key caching and phy info space.
        */
    sh->hw_buf_ptr = ssv6051_alloc_pbuf(sc, phy_info_tbl_size+
                                    sizeof(struct ssv6xxx_hw_sec), NOTYPE_BUF);
    if((sh->hw_buf_ptr>>28) != 8){
        //asic pbuf address start from 0x8xxxxxxxx
        printk("opps allocate pbuf error\n");
        WARN_ON(1);	
        ret = 1;
        goto exit;
    }

    printk("%s(): ssv6200 reserved space=0x%08x, size=%d\n", 
        __FUNCTION__, sh->hw_buf_ptr, (u32)(sizeof(phy_info_tbl)+
        sizeof(struct ssv6xxx_hw_sec)));

#endif // SSV6200_ECO
	//BUG_ON(SSV_HW_RESERVE_SIZE < (sizeof(struct ssv6xxx_hw_sec)+sizeof(phy_info_tbl)-PHY_INFO_TBL1_SIZE*4+4));

/**	
Part 1. SRAM
	**********************
	*				          * 
	*	1. Security key table *
	* 				          *
	* *********************
	*				          *
	*    	2. PHY index table     *
	* 				          *
	* *********************
	* 				          *
	*	3. PHY ll-length table * 
	*				          *
	* *********************	
=============================================	
Part 2. Register     
	**********************
	*				          * 
	*	PHY Infor Table         *
	* 				          *
	* *********************
*
*/

#ifdef SSV6200_ECO
    /**
        * Init ssv6200 hardware security table: clean the table.
        * And set PKT_ID for hardware security.
        */
    for(i=0;i<SSV_RC_MAX_STA;i++)
        sh->hw_sec_key[i] = sh->hw_buf_ptr[i];

    for(i=0;i<SSV_RC_MAX_STA;i++){
        int x;
        //==>Section 1. Write Sec table to SRAM
        for(x=0; x<sizeof(struct ssv6xxx_hw_sec); x+=4) {
            SMAC_REG_WRITE(sh, sh->hw_sec_key[i]+x, 0);
        }
    }
  
    SMAC_REG_SET_BITS(sh, ADR_SCRT_SET, ((sh->hw_sec_key[0] >> 16) << SCRT_PKT_ID_SFT),
            SCRT_PKT_ID_MSK);
    /**
        * Set default ssv6200 phy infomation table.
        */
    sh->hw_pinfo = sh->hw_sec_key[0] + sizeof(struct ssv6xxx_hw_sec);
    for(i=0, ptr=phy_info_tbl; i<PHY_INFO_TBL1_SIZE; i++, ptr++) {
        SMAC_REG_WRITE(sh, ADR_INFO0+i*4, *ptr);
        SMAC_REG_CONFIRM(sh, ADR_INFO0+i*4, *ptr);
    }	
#else // SSV6200_ECO
    /**
        * Init ssv6200 hardware security table: clean the table.
        * And set PKT_ID for hardware security.
        */
    sh->hw_sec_key = sh->hw_buf_ptr;

    //==>Section 1. Write Sec table to SRAM
    for(i=0; i<sizeof(struct ssv6xxx_hw_sec); i+=4) {
        SMAC_REG_WRITE(sh, sh->hw_sec_key+i, 0);
    }

    SMAC_REG_SET_BITS(sh, ADR_SCRT_SET, ((sh->hw_sec_key[0] >> 16) << SCRT_PKT_ID_SFT),
            SCRT_PKT_ID_MSK);

    /**
        * Set default ssv6200 phy infomation table.
        */
    sh->hw_pinfo = sh->hw_sec_key + sizeof(struct ssv6xxx_hw_sec);
    for(i=0, ptr=phy_info_tbl; i<PHY_INFO_TBL1_SIZE; i++, ptr++) {
        SMAC_REG_WRITE(sh, ADR_INFO0+i*4, *ptr);
        SMAC_REG_CONFIRM(sh, ADR_INFO0+i*4, *ptr);
    }	
#endif // SSV6200_ECO

    //==>Section 2. Write PHY index table and PHY ll-length table to SRAM
    for(i=0; i<PHY_INFO_TBL2_SIZE; i++, ptr++) {
        SMAC_REG_WRITE(sh, sh->hw_pinfo+i*4, *ptr);
        SMAC_REG_CONFIRM(sh, sh->hw_pinfo+i*4, *ptr);
    }
    for(i=0; i<PHY_INFO_TBL3_SIZE; i++, ptr++) {
        SMAC_REG_WRITE(sh, sh->hw_pinfo+
        (PHY_INFO_TBL2_SIZE<<2)+i*4, *ptr);
        SMAC_REG_CONFIRM(sh, sh->hw_pinfo+
        (PHY_INFO_TBL2_SIZE<<2)+i*4, *ptr);
    }


    //Set SRAM address to register
    SMAC_REG_WRITE(sh, ADR_INFO_IDX_ADDR, sh->hw_pinfo);
    //4byte for one entry
    SMAC_REG_WRITE(sh, ADR_INFO_LEN_ADDR, sh->hw_pinfo+(PHY_INFO_TBL2_SIZE)*4);

    printk("ADR_INFO_IDX_ADDR[%08x] ADR_INFO_LEN_ADDR[%08x]\n", sh->hw_pinfo,
        sh->hw_pinfo+(PHY_INFO_TBL2_SIZE)*4);    

//--------------------------------------------------------------------------------------------------------------
exit:
    return ret;
}


static int ssv6051_write_mac_ini(struct ssv_hw *sh)
{
    return SSV6XXX_SET_HW_TABLE(sh, ssv6051_mac_ini_table);
}

static bool ssv6051_use_hw_encrypt(int cipher, struct ssv_softc *sc, 
        struct ssv_sta_priv_data *sta_priv, struct ssv_vif_priv_data *vif_priv )
{
   
    if ((cipher == SSV_CIPHER_TKIP)
        || ((!(sc->sh->cfg.hw_caps & SSV6200_HW_CAP_AMPDU_TX) || 
            (sta_priv->sta_info->sta->ht_cap.ht_supported == false))
            && (vif_priv->force_sw_encrypt == false))) {
        return true;
    } else {
        return false;	
    }                     
}

static int ssv6051_set_rx_flow(struct ssv_hw *sh, enum ssv_rx_flow type, u32 rxflow)
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

static int ssv6051_set_rx_ctrl_flow(struct ssv_hw *sh)
{
    return ssv6051_set_rx_flow(sh, ADR_RX_FLOW_CTRL, RX_CPU_HCI);
}

//6051 HW Only supports One STA MAC address, force to update first interface's setting
static int ssv6051_set_macaddr(struct ssv_hw *sh, int vif_idx)
{
    int  ret = 0;

    if (vif_idx != 0) {
        printk("Does not support set MAC Address to HW for VIF %d\n", vif_idx);
        return -1;
    }

    ret = SMAC_REG_WRITE(sh, ADR_STA_MAC_0, *((u32 *)&sh->cfg.maddr[0][0]));
    if (!ret)
        ret = SMAC_REG_WRITE(sh, ADR_STA_MAC_1, *((u32 *)&sh->cfg.maddr[0][4]));
    return ret;
}

//6051 HW Only supports One BSSID address, force to update first interface's setting
static int ssv6051_set_bssid(struct ssv_hw *sh, u8 *bssid, int vif_idx)
{
    int  ret = 0;
    struct ssv_softc *sc = sh->sc;

    if (vif_idx != 0) {
        printk("Does not support set BSSID to HW for VIF %d\n", vif_idx);
        return -1;
    }
    
    /* Set BSSID to hardware and enable WSID entry 0 */
    memcpy(sc->bssid[0], bssid, 6);
        
    ret = SMAC_REG_WRITE(sh, ADR_BSSID_0, *((u32 *)&sc->bssid[0][0]));
    if (!ret)
        ret = SMAC_REG_WRITE(sh, ADR_BSSID_1, *((u32 *)&sc->bssid[0][4]));
    
    return ret;
}

static u64 ssv6051_get_ic_time_tag(struct ssv_hw *sh)
{
    u32 regval;
    
    SMAC_REG_READ(sh, ADR_IC_TIME_TAG_1, &regval);
    sh->chip_tag = ((u64) regval<<32);
    SMAC_REG_READ(sh, ADR_IC_TIME_TAG_0, &regval);
    sh->chip_tag |= (regval);

    return sh->chip_tag;
}

static void ssv6051_get_chip_id(struct ssv_hw *sh)
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

static bool ssv6051_if_chk_mac2(struct ssv_hw *sh)
{    
    return true;
}

static void ssv6051_save_hw_status(struct ssv_softc *sc)
{
    struct ssv_hw *sh = sc->sh;
    int     i = 0;
    int     address = 0;
    u32     word_data = 0;
    u32     sec_key_tbl_base = sh->hw_sec_key[0];
    u32     sec_key_tbl = sec_key_tbl_base;

    /* security table */
	for (i = 0; i < sizeof(struct ssv6xxx_hw_sec); i += 4) {
        address = sec_key_tbl + i;
        SMAC_REG_READ(sh, address, &word_data);
        sh->write_hw_config_cb(sh->write_hw_config_args, address, word_data);
	}
}

static int ssv6051_get_wsid(struct ssv_softc *sc, struct ieee80211_vif *vif,
    struct ieee80211_sta *sta)
{    
    struct ssv_vif_priv_data *vif_priv = (struct ssv_vif_priv_data *)vif->drv_priv;
    int     s;
    bool tdls_use_sw_cipher = false, tdls_link= false;
    struct ssv_sta_priv_data *sta_priv_dat=NULL;
    struct ssv_sta_info *sta_info;

 
     /* The security type for TDLS can only be open or CCMP. If original AP-STA
     * link is not open or CCMP mode, then it should use dual cipher. Since Cabrio
     * HW doesn't support dual cipher mode, it uses sw cipher for tdls link
     * This is for use_wpa2_only = 0. For use_wpa2_only =1, it should be ignored.            
     */
    if ( !list_empty(&vif_priv->sta_list) && vif->type == NL80211_IFTYPE_STATION){
        // second link for STA mode should be tdls link
        tdls_link = true;
    }
    
    if ((tdls_link) && (vif_priv->pair_cipher != SSV_CIPHER_NONE) 
        && (vif_priv->pair_cipher != SSV_CIPHER_CCMP)
        && (sc->sh->cfg.use_wpa2_only == false)){
        tdls_use_sw_cipher = true;
    }
    // Freddie ToDo: Ensure sta_info sync to WSID.
    // 1. Find empty slot of sta_info for the STA.
    // 2. It's the WSID in single interface.
    // 3. In dual interface, if the same security mode is used. It's the WSID.
    //    WSID >= 2 should be set to firmware for WSID mapping in order to
    //    utilize hardware security decode.
    // 4. In dual interface, if different security mode is used,
    //    the first 2 WSID (HW) should be used by the primary interface.
    //    The second interface should not use HW WSID, and the WSID should
    //    not set to firmware in order to have software security work.
    /**
    * Add the new added station into the driver data structure.
    * Driver keeps this information for AMPDU use.
    * Note that we use drv_priv of struct ieee80211_sta to
    * bind our driver's sta structure.
    */
    //#ifndef FW_WSID_WATCH_LIST
    #if 1
    if (((vif_priv->vif_idx == 0) && (tdls_use_sw_cipher == false))
        || sc->sh->cfg.use_wpa2_only)
        s = 0;
    else
        s = 2;
    #else
    // Freddie ToDo:
    //   When security mode is the same: WSID 0 for primary interface. WSID 1 for second interface.
    //   Otherwise, WSID 0, 1 for primary. Secondary uses software security.
    #endif

    for (; s < SSV_NUM_STA; s++)
    {
        sta_info = &sc->sta_info[s];
        if ((sta_info->s_flags & STA_FLAG_VALID) == 0)
        {
            sta_info->aid = sta->aid;
            sta_info->sta = sta;
            sta_info->vif = vif;
            sta_info->s_flags = STA_FLAG_VALID;

            sta_priv_dat = 
                (struct ssv_sta_priv_data *)sta->drv_priv;
            sta_priv_dat->sta_idx = s;
            sta_priv_dat->sta_info = sta_info;
            sta_priv_dat->has_hw_encrypt = false;
            sta_priv_dat->has_hw_decrypt = false;
            sta_priv_dat->need_sw_decrypt = false;
            sta_priv_dat->need_sw_encrypt = false;
            sta_priv_dat->use_mac80211_decrypt = false;
            #ifdef USE_LOCAL_CRYPTO
            sta_priv_dat->crypto_data.ops = NULL;
            sta_priv_dat->crypto_data.priv = NULL;
            #ifdef HAS_CRYPTO_LOCK
            rwlock_init(&sta_priv_dat->crypto_data.lock);
            #endif // HAS_CRYPTO_LOCK
            #endif // USE_LOCAL_CRYPTO

            // WEP use single key for pairwise and broadcast frames.
            // In AP mode, key is only set when AP mode is initialized.
            if (   (vif_priv->pair_cipher == SSV_CIPHER_WEP40)
                || (vif_priv->pair_cipher == SSV_CIPHER_WEP104))
            {
                #ifdef USE_LOCAL_CRYPTO
                if (vif_priv->crypto_data.ops != NULL)
                {
                    sta_priv_dat->crypto_data.ops = vif_priv->crypto_data.ops;
                    sta_priv_dat->crypto_data.priv = vif_priv->crypto_data.priv;
                }
                #endif // USE_LOCAL_CRYPTO
                sta_priv_dat->has_hw_encrypt = vif_priv->has_hw_encrypt;
                sta_priv_dat->has_hw_decrypt = vif_priv->has_hw_decrypt;
                sta_priv_dat->need_sw_encrypt = vif_priv->need_sw_encrypt;
                sta_priv_dat->need_sw_decrypt = vif_priv->need_sw_decrypt;                    
            }

            list_add_tail(&sta_priv_dat->list, &vif_priv->sta_list);

          //temp mark: PS
          //sc->ps_aid = sta->aid;
            break;
        }
    }
    return s;
}

static void ssv6051_rc_hw_reset(struct ssv_softc *sc, int rc_idx, int hwidx)
{
    struct ssv_rate_ctrl *ssv_rc=sc->rc;
    struct ssv_sta_rc_info *rc_sta;
    u32 rc_hw_reg[] = { ADR_MTX_MIB_WSID0, ADR_MTX_MIB_WSID1 };
    //printk("add-sta[%d]\n",hwidx);
    BUG_ON(rc_idx >= SSV_RC_MAX_STA);
    
    rc_sta = &ssv_rc->sta_rc_info[rc_idx];
    //printk("ssv6xxx_rc_hw_reset rc_sta [%08x]\n",(u32)rc_sta);
    if (hwidx >=0 && hwidx< SSV6051_NUM_HW_STA) {
        rc_sta->rc_wsid = hwidx;
        printk("rc_wsid[%d] rc_idx[%d]\n",rc_sta[rc_idx].rc_wsid,rc_idx);
        SMAC_REG_WRITE(sc->sh, rc_hw_reg[hwidx], 0x40000000);
    }
    else
    {
        rc_sta->rc_wsid = -1;
        //printk("rc_wsid[%d] rc_idx[%d]\n",rc_sta[rc_idx].rc_wsid,rc_idx);
    }
}

static void ssv6051_set_hw_wsid(struct ssv_softc *sc, struct ieee80211_vif *vif,
    struct ieee80211_sta *sta, int wsid)
{
#ifdef FW_WSID_WATCH_LIST
    struct ssv_vif_priv_data *vif_priv = (struct ssv_vif_priv_data *)vif->drv_priv;
#endif
    struct ssv_sta_priv_data *sta_priv_dat=(struct ssv_sta_priv_data *)sta->drv_priv;
    struct ssv_sta_info *sta_info;
    int i;

    sta_info = &sc->sta_info[wsid];
    /**
     * Allocate a free hardware WSID for the added STA. If no more
     * hardware entry present, set hw_wsid=-1 for
     * struct ssv_sta_info.
     */
    if (sta_priv_dat->sta_idx < SSV6051_NUM_HW_STA) 
    {
        u32 reg_wsid[] = {ADR_WSID0, ADR_WSID1};
        u32 reg_wsid_tid0[] = {ADR_WSID0_TID0_RX_SEQ, ADR_WSID1_TID0_RX_SEQ};
        u32 reg_wsid_tid7[] = {ADR_WSID0_TID7_RX_SEQ, ADR_WSID1_TID7_RX_SEQ};

        #if 0
        /*  Disable to check vaild bit for station in ps mode
         *  When station in sleep mode, we neet to keep 
         *     hw register value to receive beacon. 
         *   Therefore host wakeup by AISC and wnat to add new station. 
         *   It may get valid bit is enable.
         */
        SMAC_REG_READ(sc->sh, reg_wsid[wsid], &reg_val);
        if ((reg_val & 0x01) == 0) 
        {
        #endif // 0

        /* Add STA into hardware for hardware offload */
        SMAC_REG_WRITE(sc->sh, reg_wsid[wsid]+4, *((u32 *)&sta->addr[0]));
        SMAC_REG_WRITE(sc->sh, reg_wsid[wsid]+8, *((u32 *)&sta->addr[4]));

        /* Valid this wsid entity */
        SMAC_REG_WRITE(sc->sh, reg_wsid[wsid], 1);
            
        /* Reset rx requence number */
        for (i = reg_wsid_tid0[wsid]; i <= reg_wsid_tid7[wsid]; i += 4)
             SMAC_REG_WRITE(sc->sh, i, 0);
            
        /**
         * Enable hardware RC counters if the hardware RC is supported.
         * If RC is supported, the ssv6xxx_rate_alloc_sta() is called before
         * ssv6200_sta_add(). We shall make sure, if the added STA is 
         * set to SoC, we also enable RC for the STA.
         */
        ssv6051_rc_hw_reset(sc, sta_priv_dat->rc_idx, wsid);

        sta_info->hw_wsid = sta_priv_dat->sta_idx;
//          }
//          else BUG_ON(1);
    }
    #ifdef FW_WSID_WATCH_LIST
    else if (   (vif_priv->vif_idx == 0)
             || sc->sh->cfg.use_wpa2_only
            )
    {
        sta_info->hw_wsid = sta_priv_dat->sta_idx;
    }
    #endif // FW_WSID_WATCH_LIST  
  
}

static void ssv6051_del_hw_wsid(struct ssv_softc *sc, int hw_wsid)
{
    if ((hw_wsid != -1) && (hw_wsid < SSV6051_NUM_HW_STA)) {
        u32 reg_wsid[] = {ADR_WSID0, ADR_WSID1};

        SMAC_REG_WRITE(sc->sh, reg_wsid[hw_wsid], 0x00);
    }
}

static void ssv6051_add_fw_wsid(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, 
    struct ieee80211_sta *sta, struct ssv_sta_info *sta_info)
{
    
#ifdef FW_WSID_WATCH_LIST
    struct ssv_sta_priv_data *sta_priv_dat=(struct ssv_sta_priv_data *)sta->drv_priv; 

    if (sta_info->hw_wsid >= SSV6051_NUM_HW_STA){
        int fw_sec_caps = SSV6XXX_WSID_SEC_NONE;
    
        if (sta_priv_dat->has_hw_decrypt)
            // Only AP mode w. WEP security.
            fw_sec_caps = SSV6XXX_WSID_SEC_PAIRWISE;
        
        if (vif_priv->need_sw_decrypt)
            fw_sec_caps |= SSV6XXX_WSID_SEC_GROUP;
        
        hw_update_watch_wsid(sc, sta, sta_info, sta_priv_dat->sta_idx,
            fw_sec_caps, SSV6XXX_WSID_OPS_ADD);
    } else if (SSV6200_USE_HW_WSID(sta_priv_dat->sta_idx))  {
        // clear wsid0,1 sec type at add station, set type at add key
        // force both pairwise and group key type to SW at initial
        	  hw_update_watch_wsid(sc, sta, sta_info, sta_priv_dat->sta_idx,
                SSV6XXX_WSID_SEC_SW, SSV6XXX_WSID_OPS_HWWSID_PAIRWISE_SET_TYPE);
              hw_update_watch_wsid(sc, sta, sta_info, sta_priv_dat->sta_idx,
                SSV6XXX_WSID_SEC_SW, SSV6XXX_WSID_OPS_HWWSID_GROUP_SET_TYPE);       	 
    }
#endif        
}

static void ssv6051_del_fw_wsid(struct ssv_softc *sc, struct ieee80211_sta *sta,
    struct ssv_sta_info *sta_info)
{    

#ifdef FW_WSID_WATCH_LIST
    if (sta_info->hw_wsid >= SSV6051_NUM_HW_STA)
    {                
        hw_update_watch_wsid(sc, sta, sta_info, sta_info->hw_wsid, 0, SSV6XXX_WSID_OPS_DEL);
    }
#else
    if (sta_info->hw_wsid != -1) {
        BUG_ON(sta_info->hw_wsid >= SSV6051_NUM_HW_STA);
    }
#endif
}

static void ssv6051_enable_fw_wsid(struct ssv_softc *sc, struct ieee80211_sta *sta,
    struct ssv_sta_info *sta_info, enum SSV6XXX_WSID_SEC key_type)
{    
    #ifdef FW_WSID_WATCH_LIST
    int     wsid = sta_info->hw_wsid;
    
        // Freddie ToDo: WSID mapping separates support hardware security
        // availability for pairwise and group type
            
        if (wsid >= SSV6051_NUM_HW_STA)
        {
            hw_update_watch_wsid(sc, sta, sta_info, sta_info->hw_wsid
                , key_type, SSV6XXX_WSID_OPS_ENABLE_CAPS);
        }
    #endif
}

static void ssv6051_disable_fw_wsid(struct ssv_softc *sc, int key_idx,
    struct ssv_sta_priv_data *sta_priv, struct ssv_vif_priv_data *vif_priv)
{
#ifdef FW_WSID_WATCH_LIST    

    if (sta_priv)
    {
        struct ssv_sta_info *sta_info = &sc->sta_info[sta_priv->sta_idx];
        
        if ((key_idx == 0) && (sta_priv->has_hw_decrypt == true) && (sta_info->hw_wsid >= SSV6051_NUM_HW_STA))
        {        
            hw_update_watch_wsid(sc, sta_info->sta, sta_info, sta_priv->sta_idx, SSV6XXX_WSID_SEC_PAIRWISE
                , SSV6XXX_WSID_OPS_DISABLE_CAPS);
        }
    }
    
    if(vif_priv)
    {
        if((key_idx != 0) && !list_empty(&vif_priv->sta_list))
        {            
            struct ssv_sta_priv_data *sta_priv_iter;

            list_for_each_entry(sta_priv_iter, &vif_priv->sta_list, list)
            {
                if (((sta_priv_iter->sta_info->s_flags & STA_FLAG_VALID) == 0) 
                     || (sta_priv_iter->sta_info->hw_wsid < SSV6051_NUM_HW_STA))
                    continue;
                
                hw_update_watch_wsid(sc, sta_priv_iter->sta_info->sta, 
                    sta_priv_iter->sta_info, sta_priv_iter->sta_idx, SSV6XXX_WSID_SEC_GROUP
                    , SSV6XXX_WSID_OPS_DISABLE_CAPS);
            }
        }                   
    }
#endif // FW_WSID_WATCH_LIST
}

#ifdef FW_WSID_WATCH_LIST
static u32 ssv6051_hw_get_pair_type(struct ssv_hw *sh)
{
    u32 temp;
    SMAC_REG_READ(sh,ADR_SCRT_SET,&temp);
    temp &= PAIR_SCRT_MSK;
    temp = (temp >> PAIR_SCRT_SFT);
    SMAC_REG_WRITE(sh,ADR_SCRT_SET, temp);
    printk("==>%s: read cipher type %d from hw\n",__func__, temp);
    return temp;
}
#endif

static void ssv6051_set_fw_hwwsid_sec_type(struct ssv_softc *sc, struct ieee80211_sta *sta,
        struct ssv_sta_info *sta_info, struct ssv_vif_priv_data *vif_priv)
{
#ifdef FW_WSID_WATCH_LIST
    if (sta){
        struct ssv_sta_priv_data *sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
        int    sta_idx =  sta_priv->sta_idx;

        if (SSV6200_USE_HW_WSID(sta_idx)){
            if (SSV6XXX_USE_SW_DECRYPT(sta_priv)){
                u32 cipher_setting;
               
                cipher_setting = ssv6051_hw_get_pair_type(sc->sh);
                if (cipher_setting != ME_NONE) {
                    u32 val;
                
                    SMAC_REG_READ(sc->sh, ADR_RX_FLOW_DATA, &val);
                    
                    if (((val >>4) & 0xF) != M_ENG_CPU){
                        SMAC_REG_WRITE(sc->sh, ADR_RX_FLOW_DATA,
                            ((val & 0xf) | (M_ENG_CPU<<4)
                             | (val & 0xfffffff0) <<4));
                        dev_info(sc->dev, "orginal Rx_Flow %x , modified flow %x \n",
                            val, ((val & 0xf) | (M_ENG_CPU<<4) | (val & 0xfffffff0) <<4));    
                    } else {
                        printk(" doesn't need to change rx flow\n");
                    } 
                    
                }
            }
            
            if (sta_priv->has_hw_decrypt){
                //set hw_wsid to use hw cipher to fw for pairwise
                hw_update_watch_wsid(sc, sta, sta_info, sta_idx,
                    SSV6XXX_WSID_SEC_HW, SSV6XXX_WSID_OPS_HWWSID_PAIRWISE_SET_TYPE);
                printk("set hw wsid %d cipher mode to HW cipher for pairwise key\n", sta_idx);
            } 
        }

    } else { // group key
        struct ssv_vif_info *vif_info = &sc->vif_info[vif_priv->vif_idx];
 
        if (vif_info->if_type == NL80211_IFTYPE_STATION){ // for station type only
            struct ssv_sta_priv_data *first_sta_priv = 
                list_first_entry(&vif_priv->sta_list, struct ssv_sta_priv_data, list);

            if (first_sta_priv !=NULL)
            if  (SSV6200_USE_HW_WSID(first_sta_priv->sta_idx)){
                if (vif_priv->has_hw_decrypt){
                    //set hw_wsid to use hw cipher to fw for pairwise
                    hw_update_watch_wsid(sc, sta, sta_info, first_sta_priv->sta_idx,
                        SSV6XXX_WSID_SEC_HW, SSV6XXX_WSID_OPS_HWWSID_GROUP_SET_TYPE);
                    printk("set hw wsid %d cipher mode to HW cipher for group  key\n"
                        , first_sta_priv->sta_idx);
                }
            } 
        }
    }
#endif    
}

//  return true if wep use hw cipher
static bool ssv6051_wep_use_hw_cipher(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv)
{
    bool ret = false;
 
    if (sc->sh->cfg.use_sw_cipher){
        return ret;
    }    
 
    if (sc->sh->cfg.use_wpa2_only) {
        dev_warn(sc->dev, "Use WPA2 HW security mode only.\n");
    }
        
    if ((sc->sh->cfg.use_wpa2_only == 0) && (vif_priv->vif_idx == 0)) {
        ret = true;
    }
    
    return ret;
}

//  return true if pairwise wpa use hw cipher
static bool ssv6051_pairwise_wpa_use_hw_cipher(struct ssv_softc *sc, 
    struct ssv_vif_priv_data *vif_priv, enum SSV_CIPHER_E cipher,
    struct ssv_sta_priv_data *sta_priv)
{
    bool ret = false;
    struct ssv_vif_info *vif_info = &sc->vif_info[vif_priv->vif_idx];                                   
    bool tdls_link = false, tdls_use_sw_cipher = false, tkip_use_sw_cipher = false;                     
    bool use_non_ccmp = false;                                                                          
    int  another_vif_idx = ((vif_priv->vif_idx + 1) % 2);                                               
    struct ssv_vif_priv_data *another_vif_priv =                                                        
                                (struct ssv_vif_priv_data *)sc->vif_info[another_vif_idx].vif_priv;  

    if (sc->sh->cfg.use_sw_cipher){
        return ret;
    }
    if (sc->sh->cfg.use_wpa2_only)
    {
        dev_warn(sc->dev, "Use WPA2 HW security mode only.\n");
    }

    /* for use_wpa2_only =0, it should use sw cipher for tdls link if previous
     *  cipher is not ccmp.
     * for use_wpa2_only =1. It will use hw CCMP cipher for always
     */
    if (vif_info->if_type == NL80211_IFTYPE_STATION){
        struct ssv_sta_priv_data *first_sta_priv = 
                list_first_entry(&vif_priv->sta_list, struct ssv_sta_priv_data, list);
        
        if (first_sta_priv->sta_idx != sta_priv->sta_idx){
            // not first link for STA type, treat it as tdls link
            tdls_link = true;
        }
        printk("first sta idx %d, current sta idx %d\n",first_sta_priv->sta_idx,sta_priv->sta_idx);            
    }
    
    if ((tdls_link) && (vif_priv->pair_cipher != SSV_CIPHER_CCMP)
        && (sc->sh->cfg.use_wpa2_only == false)){
        tdls_use_sw_cipher = true; // current tdls link use sw cipher
    }
    
    if (another_vif_priv != NULL){
       	
        if ((another_vif_priv->pair_cipher != SSV_CIPHER_CCMP)
              && (another_vif_priv->pair_cipher != SSV_CIPHER_NONE)){
            use_non_ccmp = true;
            printk("another vif use none ccmp\n");
        }
    }
    
    /* For use_wpa2_only = 1. It should always use hw cipher for CCMP. And other
     * encryption mode should use sw cihper. If sta that use hw_wsid doesn't use 
     * CCMP, it should sw cipher. At the beginning the H/W cipher type is not set
     * , it will be ok to sent packet into HW cipher. After H/W cipher type is set.
     *  It should change the Rx data flow and let CPU decide whether send to HW
     * cihpher.
     * (((tdls_link) && (vif_priv->pair_cipher != SSV_CIPHER_CCMP))|| (use_non_ccmp)
     *    indicate previous link use non_CCMP mode
     * (sc->sh->cfg.use_wpa2_only == 1) && (cipher == SSV_CIPHER_CCMP)
     *    indicate current link use HW CCMP mode
     *  It should change Rx flow at this condition.
     */
    if ((((tdls_link) && (vif_priv->pair_cipher != SSV_CIPHER_CCMP)) || (use_non_ccmp)) 
        && (sc->sh->cfg.use_wpa2_only == 1) && (cipher == SSV_CIPHER_CCMP)){
        u32 val;
        
        SMAC_REG_READ(sc->sh, ADR_RX_FLOW_DATA, &val);
        
        if (((val >>4) & 0xF) != M_ENG_CPU){
            SMAC_REG_WRITE(sc->sh, ADR_RX_FLOW_DATA, ((val & 0xf) | (M_ENG_CPU<<4)
                | (val & 0xfffffff0) <<4));
            dev_info(sc->dev, "orginal Rx_Flow %x , modified flow %x \n", val,
            ((val & 0xf) | (M_ENG_CPU<<4) | (val & 0xfffffff0) <<4));    
        }   
    }
     
    
    if ((cipher == SSV_CIPHER_TKIP) && (sc->sh->cfg.use_wpa2_only == 1)){
    	 tkip_use_sw_cipher = true;
    }
    
    printk ("%s==> tkip use sw cipher %d\n",__func__,tkip_use_sw_cipher);
    
    if (   (   ((vif_priv->vif_idx == 0) && (tdls_use_sw_cipher == false) 
    	      && (tkip_use_sw_cipher == false)))
        || (   (cipher == SSV_CIPHER_CCMP) 
            && (sc->sh->cfg.use_wpa2_only == 1))){
        ret = true;
    }
    
    return ret;
}

//  return true if group wpa use hw cipher
static bool ssv6051_group_wpa_use_hw_cipher(struct ssv_softc *sc, 
    struct ssv_vif_priv_data *vif_priv, enum SSV_CIPHER_E cipher)
{
    int     ret =false;
    bool    tkip_use_sw_cipher = false;

    if (sc->sh->cfg.use_sw_cipher){
        return ret;
    }
    if (sc->sh->cfg.use_wpa2_only){
        dev_warn(sc->dev, "Use WPA2 HW security mode only.\n");
    }

    if ((cipher == SSV_CIPHER_TKIP) && (sc->sh->cfg.use_wpa2_only == 1)){
        tkip_use_sw_cipher = true;
    }
    // Group key encryption is set during pairwise key setting.

    if (((vif_priv->vif_idx == 0) && (tkip_use_sw_cipher == false))
        || ((cipher == SSV_CIPHER_CCMP) && (sc->sh->cfg.use_wpa2_only == 1))){
         
        ret = true;
    }  
    return  ret;
}

static void ssv6051_set_aes_tkip_hw_crypto_group_key (struct ssv_softc *sc,
                                               struct ssv_vif_info *vif_info,
                                               struct ssv_sta_info *sta_info,
                                               void *param)
{
    int                    wsid = sta_info->hw_wsid;
    struct ssv6xxx_hw_sec *sramKey = &(vif_info->sramKey);
    int                    index = *(u8 *)param;

    if (wsid == (-1))
        return;

    BUG_ON(index == 0);
    
    printk("Set CCMP/TKIP group key %d to WSID %d.\n", index, wsid);
    sramKey->sta_key[wsid].group_key_idx = index;

#ifdef SSV6200_ECO
    if (vif_info->vif_priv != NULL)
        dev_info(sc->dev, "Write group key %d to VIF %d \n",
                 index, vif_info->vif_priv->vif_idx);
    else
        dev_err(sc->dev, "NULL VIF.\n");
#endif

    ssv6051_write_key_to_hw(sc, vif_info->vif_priv, sramKey, wsid, index, SSV6XXX_WSID_SEC_GROUP);
    HAL_ENABLE_FW_WSID(sc, sta_info->sta, sta_info, SSV6XXX_WSID_SEC_GROUP);

} // end of - _set_aes_tkip_hw_crypto_key -

static void ssv6051_write_pairwise_keyidx_to_hw(struct ssv_hw *sh, int key_idx, int wsid)
{
    int     address = 0;
#ifdef SSV6200_ECO
    u32     sec_key_tbl_base = sh->hw_sec_key[0];
#else
    u32     sec_key_tbl_base = sh->hw_sec_key;
#endif
    u32     sec_key_tbl = sec_key_tbl_base;
   
    address =   sec_key_tbl
              + (3*sizeof(struct ssv6xxx_hw_key))
              + wsid*sizeof(struct ssv6xxx_hw_sta_key);
    #ifdef SSV6200_ECO
    //Base on 0x80000000 - 0x80070000
    address += (0x10000*wsid);
    #endif

    /* write to pairwise index to right position*/
    SMAC_REG_WRITE(sh, address, (u32)key_idx);   
}

static void ssv6051_write_hw_group_keyidx(struct ssv_hw *sh, struct ssv_vif_priv_data *vif_priv, int key_idx)
{
    struct ssv_softc *sc = sh->sc;
    
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support write hw group keyidx for this model!! \n");
}

static int ssv6051_write_pairwise_key_to_hw (struct ssv_softc *sc,
    int index, u8 algorithm, const u8 *key, int key_len,
    struct ieee80211_key_conf *keyconf,
    struct ssv_vif_priv_data *vif_priv,
    struct ssv_sta_priv_data *sta_priv)
{
    struct ssv6xxx_hw_sec *sramKey;
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

    sramKey = &(sc->vif_info[vif_priv->vif_idx].sramKey);

    sramKey->sta_key[wsid].pair_key_idx = 0;
    sramKey->sta_key[wsid].group_key_idx = vif_priv->group_key_idx;
        
    memcpy(sramKey->sta_key[wsid].pair.key, key, key_len);

    ssv6051_write_key_to_hw(sc, vif_priv, sramKey, wsid, index, SSV6XXX_WSID_SEC_PAIRWISE);
    HAL_ENABLE_FW_WSID(sc, sta_priv->sta_info->sta, sta_priv->sta_info, 
        SSV6XXX_WSID_SEC_PAIRWISE);
        
    return 0;
}

static int ssv6051_write_group_key_to_hw (struct ssv_softc *sc,
    int index, u8 algorithm, const u8 *key, int key_len,
    struct ieee80211_key_conf *keyconf, 
    struct ssv_vif_priv_data *vif_priv,
    struct ssv_sta_priv_data *sta_priv)
{
    struct ssv6xxx_hw_sec *sramKey;
#ifndef SSV6200_ECO
    u32                    sec_key_tbl_base = sc->sh->hw_sec_key;
    //u32                    sec_key_tbl;
    int                    address = 0;
    int                   *pointer = NULL;
    int                    i;
#endif
    int                    wsid = sta_priv ? sta_priv->sta_info->hw_wsid : (-1);
    int                    ret = 0;

    if (vif_priv == NULL)
    {
        dev_err(sc->dev, "Setting group key to NULL VIF\n");
        return -EOPNOTSUPP;
    }

    dev_info(sc->dev, "Setting VIF %d group key %d of length %d to WSID %d.\n",
             vif_priv->vif_idx, index, key_len, wsid);

    sramKey = &(sc->vif_info[vif_priv->vif_idx].sramKey);

    /*save group key index */
    vif_priv->group_key_idx = index;

    if (sta_priv)
        sta_priv->group_key_idx = index;

    memcpy(sramKey->group_key[index-1].key, key, key_len);

    #ifndef SSV6200_ECO
    address = sec_key_tbl_base + ((index-1)*sizeof(struct ssv6xxx_hw_key));
    pointer = (int *)&sramKey->group_key[index-1];

    /* write group key*/
    for (i = 0; i < (sizeof(struct ssv6xxx_hw_key)/4); i++)
         SMAC_REG_WRITE(sc->sh, address+(i*4), *(pointer++));
    #endif

    /* write group key index to all sta entity*/
    WARN_ON(sc->vif_info[vif_priv->vif_idx].vif_priv == NULL);
    ssv6xxx_foreach_vif_sta(sc, &sc->vif_info[vif_priv->vif_idx],
                            ssv6051_set_aes_tkip_hw_crypto_group_key, &index);
    ret = 0;

    return ret;
}

static void ssv6051_write_key_to_hw(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, 
    void *sram_ptr, int wsid, int key_idx, enum SSV6XXX_WSID_SEC key_type)
{
    struct ssv6xxx_hw_sec  *sram_key = (struct ssv6xxx_hw_sec *)sram_ptr;
    int     address = 0;
    int     *pointer = NULL;
#ifdef SSV6200_ECO
    u32     sec_key_tbl_base = sc->sh->hw_sec_key[0];
#else
    u32     sec_key_tbl_base = sc->sh->hw_sec_key;
#endif
    u32     sec_key_tbl = sec_key_tbl_base;
    int     i;
       
    switch (key_type)
    {
        case SSV6XXX_WSID_SEC_PAIRWISE:
            
            address =   sec_key_tbl
              + (3*sizeof(struct ssv6xxx_hw_key))
              + wsid*sizeof(struct ssv6xxx_hw_sta_key);
    #ifdef SSV6200_ECO
            //Base on 0x80000000 - 0x80070000
            address += (0x10000*wsid);
    #endif
            pointer = (int *)&sram_key->sta_key[wsid];
            /* write to pairwise key and index to right position*/
            for (i = 0; i < (sizeof(struct ssv6xxx_hw_sta_key)/4); i++)
                SMAC_REG_WRITE(sc->sh, address+(i*4), *(pointer++));  
            break;
            
        case SSV6XXX_WSID_SEC_GROUP:
            
    #ifdef SSV6200_ECO
            sec_key_tbl += (0x10000 * wsid);
            /* Write group key */
            address =   sec_key_tbl
                      + ((key_idx - 1) * sizeof(struct ssv6xxx_hw_key));
            pointer = (int *)&sram_key->group_key[key_idx - 1];

            for (i = 0; i < (sizeof(struct ssv6xxx_hw_key)/4); i++)
                SMAC_REG_WRITE(sc->sh, address+(i*4), *(pointer++));
    #endif // SSV6200_ECO

            /* write group key index to all sta entity*/
            address =   sec_key_tbl
                + (3*sizeof(struct ssv6xxx_hw_key))
                + (wsid*sizeof(struct ssv6xxx_hw_sta_key));
            pointer = (int *)&sram_key->sta_key[wsid];
            /*overwrite first 4 byte of sta_key entity */
            SMAC_REG_WRITE(sc->sh, address, *(pointer));
            break;
            
        default:
            printk(KERN_ERR "invalid key type %d.",key_type);
            break;
    }
    
}

static void ssv6051_set_pairwise_cipher_type(struct ssv_hw *sh, u8 cipher, u8 wsid)
{
    u32 temp;
                                                         
    SMAC_REG_READ(sh,ADR_SCRT_SET,&temp);                           
    temp = (temp & PAIR_SCRT_I_MSK);                                  
    temp |= (cipher << PAIR_SCRT_SFT);                                  
    SMAC_REG_WRITE(sh,ADR_SCRT_SET, temp);                            
    printk("==>%s: write pairwise cipher type %d into hw\n", __func__, cipher);        
}

static void ssv6051_set_group_cipher_type(struct ssv_hw *sh, struct ssv_vif_priv_data *vif_priv, u8 cipher)
{
    u32 temp;

    SMAC_REG_READ(sh,ADR_SCRT_SET,&temp);
    temp = temp & GRP_SCRT_I_MSK;
    temp |= (cipher << GRP_SCRT_SFT);
    SMAC_REG_WRITE(sh,ADR_SCRT_SET, temp);

    printk(KERN_ERR "Set group key type %d\n", cipher);
}

static bool ssv6051_chk_if_support_hw_bssid(struct ssv_softc *sc,
    int vif_idx)
{
    /*set to hw if it's first vif config*/
    if (!vif_idx)
        return true;

    printk(" %s: VIF %d doesn't support HW BSSID\n", __func__, vif_idx);
    return false;
}    

static void ssv6051_chk_dual_vif_chg_rx_flow(struct ssv_softc *sc, 
    struct ssv_vif_priv_data *vif_priv)
{   
    int  another_vif_idx = ((vif_priv->vif_idx + 1) % 2);
    struct ssv_vif_priv_data *another_vif_priv =
            (struct ssv_vif_priv_data *)sc->vif_info[another_vif_idx].vif_priv;

    if ( another_vif_priv != NULL){
        if (((SSV6XXX_USE_SW_DECRYPT(vif_priv) 
                && SSV6XXX_USE_HW_DECRYPT (another_vif_priv)))
            || ((SSV6XXX_USE_HW_DECRYPT (vif_priv)
                && (SSV6XXX_USE_SW_DECRYPT(another_vif_priv))))){
        u32 val;
        
        SMAC_REG_READ(sc->sh, ADR_RX_FLOW_DATA, &val);
            
            if (((val >>4) & 0xF) != M_ENG_CPU){
                SMAC_REG_WRITE(sc->sh, ADR_RX_FLOW_DATA,	((val & 0xf) | (M_ENG_CPU<<4)
                    | (val & 0xfffffff0) <<4));
                dev_info(sc->dev, "orginal Rx_Flow %x , modified flow %x \n", val,
                ((val & 0xf) | (M_ENG_CPU<<4) | (val & 0xfffffff0) <<4));    
            } else {
                printk(" doesn't need to change rx flow\n");
            } 
        }
    }
}

static void ssv6051_restore_rx_flow(struct ssv_softc *sc, 
    struct ssv_vif_priv_data *vif_priv, struct ieee80211_sta *sta)
{
    struct ssv_vif_info *vif_info = &sc->vif_info[vif_priv->vif_idx];
    int  another_vif_idx = ((vif_priv->vif_idx + 1) % 2);
    struct ssv_vif_priv_data *another_vif_priv =
            (struct ssv_vif_priv_data *)sc->vif_info[another_vif_idx].vif_priv;
    
    if (vif_info->if_type != NL80211_IFTYPE_AP) {
        if ((SSV6XXX_USE_SW_DECRYPT(vif_priv) 
                && SSV6XXX_USE_HW_DECRYPT (another_vif_priv))
            || (SSV6XXX_USE_SW_DECRYPT(another_vif_priv) 
                && SSV6XXX_USE_HW_DECRYPT (vif_priv))){ 
     
#ifdef CONFIG_SSV_HW_ENCRYPT_SW_DECRYPT
            HAL_SET_RX_FLOW(sc->sh, RX_DATA_FLOW, RX_HCI);
#else
            HAL_SET_RX_FLOW(sc->sh, RX_DATA_FLOW, RX_CIPHER_HCI);
#endif
            printk("redirect Rx flow for disconnect\n");            
        }
    }else if (vif_info->if_type == NL80211_IFTYPE_AP){ 
    
        //re-direct rx flow to normal for vif sw , sta hw
        if (sta == NULL) { //  for group key only
             if (SSV6XXX_USE_SW_DECRYPT(another_vif_priv) 
                && SSV6XXX_USE_HW_DECRYPT (vif_priv)){ 
                   
               // ssv6051_set_group_cipher_type(sc->sh, ME_NONE);
#ifdef CONFIG_SSV_HW_ENCRYPT_SW_DECRYPT
                HAL_SET_RX_FLOW(sc->sh, RX_DATA_FLOW, RX_HCI);
#else
                HAL_SET_RX_FLOW(sc->sh, RX_DATA_FLOW, RX_CIPHER_HCI);
#endif
                printk("redirect Rx flow for disconnect, and clear group key type\n");            
            }
        }
    }
}

#ifdef CONFIG_PM
void ssv6051_save_clear_trap_reason(struct ssv_softc *sc)
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

void ssv6051_restore_trap_reason(struct ssv_softc *sc)
{
	SMAC_REG_WRITE(sc->sh, ADR_REASON_TRAP0, sc->trap_data.reason_trap0);
	SMAC_REG_WRITE(sc->sh, ADR_REASON_TRAP1, sc->trap_data.reason_trap1);
}

void ssv6051_pmu_awake(struct ssv_softc *sc)
{
	SMAC_REG_SET_BITS(sc->sh, ADR_FN1_INT_CTRL_RESET, (1<<24), 0x01000000);
	mdelay(5);
	SMAC_REG_SET_BITS(sc->sh, ADR_FN1_INT_CTRL_RESET, (0<<24), 0x01000000);
}
#endif

static void ssv6051_set_wep_hw_crypto_pair_key (struct ssv_softc *sc,
    struct ssv_vif_info *vif_info, struct ssv_sta_info *sta_info,
    void *param)
{
    int                     wsid = sta_info->hw_wsid;
    struct ssv6xxx_hw_sec  *sram_key = (struct ssv6xxx_hw_sec *)param;
    u8                     *key = sram_key->sta_key[0].pair.key;
    u32                     key_len = *(u16 *)&sram_key->sta_key[0].reserve[0];

    struct ssv_sta_priv_data *sta_priv = (struct ssv_sta_priv_data *)sta_info->sta->drv_priv;
    struct ssv_vif_priv_data *vif_priv = (struct ssv_vif_priv_data *)vif_info->vif->drv_priv;

    if (wsid == (-1))
        return;

    sram_key->sta_key[wsid].pair_key_idx  = 0;
    sram_key->sta_key[wsid].group_key_idx = 0;

    sta_priv->has_hw_encrypt = vif_priv->has_hw_encrypt;
    sta_priv->has_hw_decrypt = vif_priv->has_hw_decrypt;
    sta_priv->need_sw_encrypt = vif_priv->need_sw_encrypt;
    sta_priv->need_sw_decrypt = vif_priv->need_sw_decrypt;

    if (wsid != 0)
        memcpy(sram_key->sta_key[wsid].pair.key, key, key_len);

    ssv6051_write_key_to_hw(sc, vif_priv, sram_key, wsid, 0, SSV6XXX_WSID_SEC_PAIRWISE);
} // end of - _set_wep_hw_crypto_pair_key -


static void ssv6051_set_wep_hw_crypto_group_key (struct ssv_softc *sc,
    struct ssv_vif_info *vif_info, struct ssv_sta_info *sta_info,
    void *param)
{
    int                     wsid = sta_info->hw_wsid;
    struct ssv6xxx_hw_sec  *sram_key = (struct ssv6xxx_hw_sec *)param;
    u32                     key_idx = sram_key->sta_key[0].pair_key_idx;    
    struct ssv_sta_priv_data *sta_priv = (struct ssv_sta_priv_data *)sta_info->sta->drv_priv;
    struct ssv_vif_priv_data *vif_priv = (struct ssv_vif_priv_data *)vif_info->vif->drv_priv;

    if (wsid == (-1))
        return;

    if (wsid != 0)
    {
        sram_key->sta_key[wsid].pair_key_idx = key_idx;
        sram_key->sta_key[wsid].group_key_idx = key_idx;
        sta_priv->has_hw_encrypt = vif_priv->has_hw_encrypt;
        sta_priv->has_hw_decrypt = vif_priv->has_hw_decrypt;
        sta_priv->need_sw_encrypt = vif_priv->need_sw_encrypt;
        sta_priv->need_sw_decrypt = vif_priv->need_sw_decrypt;
    }

    ssv6051_write_key_to_hw(sc, vif_priv, sram_key, wsid, key_idx, 
        SSV6XXX_WSID_SEC_GROUP);
} // end of - _set_wep_hw_crypto_group_key -

static int ssv6051_hw_crypto_key_write_wep(struct ssv_softc *sc,
    struct ieee80211_key_conf *keyconf, u8 algorithm, 
    struct ssv_vif_info *vif_info)
{
    struct ssv6xxx_hw_sec *sramKey = &vif_info->sramKey;
#ifndef SSV6200_ECO
    int address = 0x00;
    //int MAX_STA = SSV_NUM_HW_STA;
    int *pointer=NULL;
    u32  sec_key_tbl=sc->sh->hw_sec_key;
    int i;
#endif

    // Freddie ToDo: For multiple VIF, how to handle WEP coexist with other security modes?

    if (keyconf->keyidx == 0)
    {
        ssv6xxx_foreach_vif_sta(sc, vif_info, ssv6051_set_wep_hw_crypto_pair_key, sramKey);
    } 
    else // index != 0
    {
#ifndef SSV6200_ECO
        /* setp 1: write group key*/
        //memcpy(sramKey->group_key[key->keyidx-1].key, key->key, key->keylen);

        address =   sec_key_tbl
                  + ((keyconf->keyidx-1) * sizeof(struct ssv6xxx_hw_key));
        pointer = (int *)&sramKey->group_key[keyconf->keyidx-1];

        for (i=0;i<(sizeof(struct ssv6xxx_hw_key)/4);i++)
           SMAC_REG_WRITE(sc->sh, address+(i*4), *(pointer++));
#endif

         /* setp 2: write key index to each sta*/
        ssv6xxx_foreach_vif_sta(sc, vif_info, ssv6051_set_wep_hw_crypto_group_key, sramKey);
    }

    return 0; 
}

static void ssv6051_set_wep_hw_crypto_key(struct ssv_softc *sc,                                                                   
    struct ssv_sta_info *sta_info, struct ssv_vif_priv_data *vif_priv)
{
    struct ssv_vif_info *vif_info = &sc->vif_info[vif_priv->vif_idx];
    struct ssv6xxx_hw_sec *sramKey = &vif_info->sramKey;
        
    //No matter current key is, update the pairwise key first
    ssv6051_set_wep_hw_crypto_pair_key(sc, vif_info, sta_info, (void*)sramKey);
    if (sramKey->sta_key[0].pair_key_idx != 0)
    {
         // when current key is not pairwise key, update to the group key
        ssv6051_set_wep_hw_crypto_group_key(sc, vif_info, sta_info, (void*)sramKey);
    }
}

static void ssv6051_store_wep_key(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv,
    struct ssv_sta_priv_data *sta_priv, enum SSV_CIPHER_E cipher, struct ieee80211_key_conf *key)
{
    struct ssv_vif_info           *vif_info = &sc->vif_info[vif_priv->vif_idx];
    struct ssv6xxx_hw_sec         *sram_key = &vif_info->sramKey;
    
    // Use STA #0 to store key data.
    sram_key->sta_key[0].pair_key_idx = key->keyidx;
    sram_key->sta_key[0].group_key_idx = key->keyidx;
    *(u16 *)&sram_key->sta_key[0].reserve[0] = key->keylen;

    if (key->keyidx == 0) {
        memcpy(sram_key->sta_key[0].pair.key, key->key, key->keylen);
    } else {
        memcpy(sram_key->group_key[key->keyidx - 1].key, key->key, key->keylen);
    }

    if ( ssv6051_wep_use_hw_cipher(sc, vif_priv))
    {
        ssv6051_set_pairwise_cipher_type(sc->sh, cipher, sta_priv->sta_info->hw_wsid);
        ssv6051_set_group_cipher_type(sc->sh, vif_priv, cipher);
        
        ssv6051_hw_crypto_key_write_wep(sc, key, cipher, &sc->vif_info[vif_priv->vif_idx]);
    }
}

static bool ssv6051_put_mic_space_for_hw_ccmp_encrypt(struct ssv_softc *sc, struct sk_buff *skb) 
{
    //do nothing
    return false;
}


static void ssv6051_set_replay_ignore(struct ssv_hw *sh, u8 ignore)
{
    u32 temp;
    SMAC_REG_READ(sh,ADR_SCRT_SET,&temp);
    temp = temp & SCRT_RPLY_IGNORE_I_MSK;
    temp |= (ignore << SCRT_RPLY_IGNORE_SFT);
    SMAC_REG_WRITE(sh,ADR_SCRT_SET, temp);
}

static void ssv6051_update_decision_table_6(struct ssv_hw *sh, u32 value)
{
    SMAC_REG_WRITE(sh, ADR_MRX_FLT_TB6, value);
}

static int ssv6051_update_decision_table(struct ssv_softc *sc)
{
    int i;
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
    return 0;
}

static void ssv6051_get_fw_version(struct ssv_hw *sh, u32 *regval)
{
    SMAC_REG_READ(sh, ADR_TX_SEG, regval);
}

//6051 Only supports One OP mode register, force to update first interface's setting
static void ssv6051_set_op_mode(struct ssv_hw *sh, u32 op_mode, int vif_idx)
{
    if (vif_idx != 0) {
        printk("Does not support set OP mode to HW for VIF %d\n", vif_idx);
        return;
    }

    SMAC_REG_SET_BITS(sh, ADR_GLBLE_SET, op_mode, OP_MODE_MSK);
}

static void ssv6051_set_halt_mngq_util_dtim(struct ssv_hw *sh, bool val)
{
    if (val) {
        SMAC_REG_SET_BITS(sh, ADR_MTX_BCN_EN_MISC, 
            MTX_HALT_MNG_UNTIL_DTIM_MSK, MTX_HALT_MNG_UNTIL_DTIM_MSK);
    } else {
        SMAC_REG_SET_BITS(sh, ADR_MTX_BCN_EN_MISC,
            0, MTX_HALT_MNG_UNTIL_DTIM_MSK);
    }
}

static void ssv6051_set_dur_burst_sifs_g(struct ssv_hw *sh, u32 val)
{
    u32 regval;

    SMAC_REG_READ(sh, ADR_MTX_DUR_SIFS_G, &regval); 
    
    regval &= MTX_DUR_BURST_SIFS_G_I_MSK;
    regval |= val << MTX_DUR_BURST_SIFS_G_SFT ;
    SMAC_REG_WRITE(sh, ADR_MTX_DUR_SIFS_G, regval);
}

static void ssv6051_set_dur_slot(struct ssv_hw *sh, u32 val)
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

static void ssv6051_set_qos_enable(struct ssv_hw *sh, bool val)
{
   //set QoS status
    SMAC_REG_SET_BITS(sh, ADR_GLBLE_SET, 
            (val<<QOS_EN_SFT), QOS_EN_MSK);
    
}

static void ssv6051_set_wmm_param(struct ssv_softc *sc, 
    const struct ieee80211_tx_queue_params *params, u16 queue)
{
    u32 cw;
    u8 hw_txqid = sc->tx.hw_txqid[queue];
 
#if 1
/*
    Fix MAC TX backoff issue.
    http://192.168.1.30/mantis/view.php?id=36
 */
    cw = (params->aifs-1)&0xf;
#else
    cw = params->aifs&0xf;
#endif
    cw|= ((ilog2(params->cw_min+1))&0xf)<<TXQ1_MTX_Q_ECWMIN_SFT;//8;
    cw|= ((ilog2(params->cw_max+1))&0xf)<<TXQ1_MTX_Q_ECWMAX_SFT;//12;
    cw|= ((params->txop)&0xff)<<TXQ1_MTX_Q_TXOP_LIMIT_SFT;//16;

    SMAC_REG_WRITE(sc->sh, ADR_TXQ0_MTX_Q_AIFSN+0x100*hw_txqid, cw);   
}

static void ssv6051_init_tx_cfg(struct ssv_hw *sh)
{
	sh->tx_info.tx_id_threshold = SSV6200_ID_TX_THRESHOLD;
	sh->tx_info.tx_page_threshold = SSV6200_PAGE_TX_THRESHOLD;
	sh->tx_info.tx_lowthreshold_id_trigger = SSV6200_TX_LOWTHRESHOLD_ID_TRIGGER;
	sh->tx_info.tx_lowthreshold_page_trigger = SSV6200_TX_LOWTHRESHOLD_PAGE_TRIGGER;
	sh->tx_info.bk_txq_size = SSV6200_ID_AC_BK_OUT_QUEUE;
	sh->tx_info.be_txq_size = SSV6200_ID_AC_BE_OUT_QUEUE;
	sh->tx_info.vi_txq_size = SSV6200_ID_AC_VI_OUT_QUEUE;
	sh->tx_info.vo_txq_size = SSV6200_ID_AC_VO_OUT_QUEUE;
	sh->tx_info.manage_txq_size = SSV6200_ID_MANAGER_QUEUE;
    
    sh->tx_page_available = SSV6200_PAGE_TX_THRESHOLD;
	sh->ampdu_divider = SSV6200_AMPDU_DIVIDER;
	
	//info hci
	memcpy(&(sh->hci.hci_ctrl->tx_info), &(sh->tx_info), sizeof(struct ssv6xxx_tx_hw_info));
}

static void ssv6051_init_rx_cfg(struct ssv_hw *sh)
{
	sh->rx_info.rx_id_threshold = SSV6200_ID_RX_THRESHOLD;
	sh->rx_info.rx_page_threshold = SSV6200_PAGE_RX_THRESHOLD;
	sh->rx_info.rx_ba_ma_sessions = SSV6200_RX_BA_MAX_SESSIONS;
	
	//info hci
	memcpy(&(sh->hci.hci_ctrl->rx_info), &(sh->rx_info), sizeof(struct ssv6xxx_rx_hw_info));
}

// allocate pbuf    
static u32 ssv6051_alloc_pbuf(struct ssv_softc *sc, int size, int type)
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

inline bool ssv6051_mcu_input_full(struct ssv_softc *sc)
{
    u32 regval=0;
    
    SMAC_REG_READ(sc->sh, ADR_MCU_STATUS, &regval);
    return (CH0_FULL_MSK & regval);
}

// free pbuf
static bool ssv6051_free_pbuf(struct ssv_softc *sc, u32 pbuf_addr)
{
    u32  regval=0;
    u16  failCount=0;
    u8  *p_tx_page_cnt = &sc->sh->page_count[PACKET_ADDR_2_ID(pbuf_addr)];
    
    while (ssv6051_mcu_input_full(sc))
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

static void ssv6051_ampdu_auto_crc_en(struct ssv_hw *sh)
{
    // Enable HW_AUTO_CRC_32 ======================================
    SMAC_REG_SET_BITS(sh, ADR_MTX_MISC_EN, (0x1 << MTX_AMPDU_CRC_AUTO_SFT),
                    MTX_AMPDU_CRC_AUTO_MSK);
}

static void ssv6051_set_rx_ba(struct ssv_hw *sh, bool on, u8 *ta,
        u16 tid, u16 ssn, u8 buf_size)
{
      if (on) {
        u32 u32ta;
        u32ta = 0;
        u32ta |= (ta[0] & 0xff) << (8 * 0);
        u32ta |= (ta[1] & 0xff) << (8 * 1);
        u32ta |= (ta[2] & 0xff) << (8 * 2);
        u32ta |= (ta[3] & 0xff) << (8 * 3);
        SMAC_REG_WRITE(sh, ADR_BA_TA_0, u32ta);

        u32ta = 0;
        u32ta |= (ta[4] & 0xff) << (8 * 0);
        u32ta |= (ta[5] & 0xff) << (8 * 1);
        SMAC_REG_WRITE(sh, ADR_BA_TA_1, u32ta);

        SMAC_REG_WRITE(sh, ADR_BA_TID, tid);
        SMAC_REG_WRITE(sh, ADR_BA_ST_SEQ, ssn);

        SMAC_REG_WRITE(sh, ADR_BA_SB0, 0);
        SMAC_REG_WRITE(sh, ADR_BA_SB1, 0);

        //turn on ba session
        SMAC_REG_WRITE(sh, ADR_BA_CTRL, 
            (1 << BA_AGRE_EN_SFT)| (3 << BA_CTRL_SFT));
    } else {
        //turn off ba session
        SMAC_REG_WRITE(sh, ADR_BA_CTRL, 0x0);
    }  
}

static u8 ssv6051_read_efuse(struct ssv_hw *sh, u8 *pbuf)
{
    //extern struct ssv6xxx_cfg ssv_cfg;
    u32 val, i , j ;

    WARN_ON(sh == NULL);
    WARN_ON(sh->hci.hci_ops == NULL);
    
    SMAC_REG_WRITE(sh, ADR_PAD20, 0x11);

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

    //????
    SMAC_REG_WRITE(sh, ADR_PAD20,0x1800000a);
    return 1;

}

#define CLK_SRC_SYNTH_40M   3
static int ssv6051_chg_clk_src(struct ssv_hw *sh)
{
	int ret = 0;
	u32 regval;

    // note :ADR_TRX_DUMMY_REGISTER should be changed before /after clock src change for CABRIOE
    //Check SDIO command is work
    //Avoid SDIO issue.
    if (ret == 0) ret = SMAC_REG_WRITE(sh, ADR_TRX_DUMMY_REGISTER, 0xEAAAAAAA);
    SMAC_REG_READ(sh,ADR_TRX_DUMMY_REGISTER,&regval);
    if (regval != 0xEAAAAAAA)
    {
        printk("@@@@@@@@@@@@\n");
        printk(" SDIO issue -- please check 0xCE01008C %08x!!\n",regval);
        printk(" It shouble be 0xEAAAAAAA!!\n");
        printk("@@@@@@@@@@@@ \n");
    }
    
    //Switch clock to PLL output of RF
    //MAC and MCU clock selection :   00 : OSC clock   01 : RTC clock   10 : synthesis 80MHz clock   11 : synthesis 40MHz clock
    ret = SMAC_REG_WRITE(sh, ADR_CLOCK_SELECTION, CLK_SRC_SYNTH_40M);
    
    if (ret == 0) ret = SMAC_REG_WRITE(sh, ADR_TRX_DUMMY_REGISTER, 0xAAAAAAAA);
    
    return ret;
}

static enum ssv6xxx_beacon_type ssv6051_beacon_get_valid_cfg(struct ssv_hw *sh)
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

static void ssv6051_set_beacon_reg_lock(struct ssv_hw *sh, bool val)
{
	struct ssv_softc *sc=sh->sc;
	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
	    "ssv6xxx_beacon_reg_lock   val[0x:%08x]\n ", val);

	SMAC_REG_SET_BITS(sh, ADR_MTX_BCN_MISC, 
	    val<<MTX_BCN_PKTID_CH_LOCK_SFT, MTX_BCN_PKTID_CH_LOCK_MSK);
}
                                                                                       
static void ssv6051_set_beacon_id_dtim(struct ssv_softc *sc,
        enum ssv6xxx_beacon_type avl_bcn_type, int dtim_offset)
{
    u32 reg_tx_beacon_adr = ADR_MTX_BCN_CFG0;
    u32 val;
	
    val = (PBUF_MapPkttoID(sc->beacon_info[avl_bcn_type].pubf_addr) 
            | (dtim_offset << MTX_DTIM_OFST0_SFT));	       
	if(avl_bcn_type == SSV6xxx_BEACON_1)	    
		reg_tx_beacon_adr = ADR_MTX_BCN_CFG1;
	SMAC_REG_WRITE(sc->sh, reg_tx_beacon_adr,  val);
	
	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
	    "[A] update to register reg_tx_beacon_adr[%08x] val[%08x]\n", reg_tx_beacon_adr, val);

}

static void ssv6051_fill_beacon(struct ssv_softc *sc, u32 regaddr, struct sk_buff *skb)
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

static bool ssv6051_beacon_enable(struct ssv_softc *sc, bool bEnable)
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

static void ssv6051_set_beacon_info(struct ssv_hw *sh, u8 beacon_interval, u8 dtim_cnt)
{
    struct ssv_softc *sc=sh->sc;  
	u32 val;

	//if default is 0 set to our default
	if(beacon_interval==0)
		beacon_interval = 100;

	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_BEACON,
	    "[A] BSS_CHANGED_BEACON_INT beacon_int[%d] dtim_cnt[%d]\n",
	    beacon_interval, (dtim_cnt));

	val = (beacon_interval << MTX_BCN_PERIOD_SFT)
	        | (dtim_cnt << MTX_DTIM_NUM_SFT);
	SMAC_REG_WRITE(sh, ADR_MTX_BCN_PRD, val);
}

static bool ssv6051_get_bcn_ongoing(struct ssv_hw *sh)
{
    u32 regval;

	SMAC_REG_READ(sh, ADR_MTX_BCN_MISC, &regval);
	
	return ((MTX_AUTO_BCN_ONGOING_MSK & regval) >> MTX_AUTO_BCN_ONGOING_SFT);
}

static void ssv6051_beacon_loss_enable(struct ssv_hw *sh)
{
	return;
}

static void ssv6051_beacon_loss_disable(struct ssv_hw *sh)
{
	return;
}

static void ssv6051_beacon_loss_config(struct ssv_hw *sh, u16 beacon_int, const u8 *bssid)
{
	return;
}

static void ssv6051_update_txq_mask(struct ssv_hw *sh, u32 txq_mask)
{
     SMAC_REG_SET_BITS(sh, ADR_MTX_MISC_EN,
        (txq_mask << MTX_HALT_Q_MB_SFT), MTX_HALT_Q_MB_MSK);
}

static void ssv6051_readrg_hci_inq_info(struct ssv_hw *sh, int *hci_used_id, int *tx_use_page)
{
    return;
}

static bool ssv6051_readrg_txq_info(struct ssv_hw *sh, u32 *txq_info, int *hci_used_id)
{
    *hci_used_id = -1;  //ssv6051 don't update the max_count for tx
    return SMAC_REG_READ(sh, ADR_TX_ID_ALL_INFO, txq_info);
}

static bool ssv6051_readrg_txq_info2(struct ssv_hw *sh, u32 *txq_info2, int *hci_used_id)
{
    *hci_used_id = -1;  //ssv6051 don't update the max_count for tx
    return SMAC_REG_READ(sh, ADR_TX_ID_ALL_INFO2, txq_info2);
}

static bool ssv6051_dump_wsid(struct ssv_hw *sh)
{
    const u32 reg_wsid[]={ ADR_WSID0, ADR_WSID1 };
	const u32 reg_wsid_tid0[]={ ADR_WSID0_TID0_RX_SEQ, ADR_WSID1_TID0_RX_SEQ };
	const u32 reg_wsid_tid7[]={ ADR_WSID0_TID7_RX_SEQ, ADR_WSID1_TID7_RX_SEQ };
    const u8 *op_mode_str[]={"STA", "AP", "AD-HOC", "WDS"};
    const u8 *ht_mode_str[]={"Non-HT", "HT-MF", "HT-GF", "RSVD"}; 
    u32 addr, regval;
    int s;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;

    for (s = 0; s < SSV6051_NUM_HW_STA; s++) {
       
        SMAC_REG_READ(sh, reg_wsid[s], &regval);
        snprintf_res(cmd_data, "==>WSID[%d]\n\tvalid[%d] qos[%d] op_mode[%s] ht_mode[%s]\n",
            s, regval&0x1, (regval>>1)&0x1, op_mode_str[((regval>>2)&3)], ht_mode_str[((regval>>4)&3)]);
                                               
        SMAC_REG_READ(sh, reg_wsid[s]+4, &regval);
        snprintf_res(cmd_data, "\tMAC[%02x:%02x:%02x:%02x:",
               (regval&0xff), ((regval>>8)&0xff), ((regval>>16)&0xff), ((regval>>24)&0xff));

        SMAC_REG_READ(sh, reg_wsid[s]+8, &regval);
        snprintf_res(cmd_data, "%02x:%02x]\n",
               (regval&0xff), ((regval>>8)&0xff));

        for(addr = reg_wsid_tid0[s]; addr <= reg_wsid_tid7[s]; addr+=4){

            SMAC_REG_READ(sh, addr, &regval);
            
            snprintf_res(cmd_data, "\trx_seq%d[%d]\n", ((addr-reg_wsid_tid0[s])>>2), ((regval)&0xffff));
        }                                  
    }

    return 0;
}

static bool ssv6051_dump_decision(struct ssv_hw *sh)
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

static u32 ssv6051_get_ffout_cnt(u32 value, int tag)
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

static u32 ssv6051_get_in_ffcnt(u32 value, int tag)
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

static void ssv6051_read_ffout_cnt(struct ssv_hw *sh, 
    u32 *value, u32 *value1, u32 *value2)
{
    SMAC_REG_READ(sh, ADR_RD_FFOUT_CNT1, value);
    SMAC_REG_READ(sh, ADR_RD_FFOUT_CNT2, value1);
    SMAC_REG_READ(sh, ADR_RD_FFOUT_CNT3, value2);    
}

static void ssv6051_read_in_ffcnt(struct ssv_hw *sh, 
    u32 *value, u32 *value1)
{
    SMAC_REG_READ(sh, ADR_RD_IN_FFCNT1, value);
    SMAC_REG_READ(sh, ADR_RD_IN_FFCNT2, value1);   
}

static void ssv6051_read_id_len_threshold(struct ssv_hw *sh, 
    u32 *tx_len, u32 *rx_len)
{
	u32 regval = 0;

    if(SMAC_REG_READ(sh, ADR_ID_LEN_THREADSHOLD2, &regval));
	*tx_len = ((regval & TX_ID_ALC_LEN_MSK) >> TX_ID_ALC_LEN_SFT);
	*rx_len = ((regval & RX_ID_ALC_LEN_MSK) >> RX_ID_ALC_LEN_SFT);
}

static void ssv6051_read_tag_status(struct ssv_hw *sh, 
    u32 *ava_status)
{
	u32 regval = 0;
    
	if(SMAC_REG_READ(sh, ADR_TAG_STATUS, &regval));
	*ava_status = ((regval & AVA_TAG_MSK) >> AVA_TAG_SFT);
}

static void ssv6051_reset_mib(struct ssv_hw *sh)
{

    SMAC_REG_WRITE(sh, ADR_MIB_EN, 0);
    msleep(1);
    SMAC_REG_WRITE(sh, ADR_MIB_EN, 0xffffffff);

    HAL_RESET_MIB_PHY(sh);
}
   
static void ssv6051_list_mib(struct ssv_hw *sh)
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

static void ssv6051_dump_mib_rx(struct ssv_hw *sh)
{
    u32  value;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;

    snprintf_res(cmd_data, "%-10s\t\t%-10s\t\t%-10s\t\t%-10s\n",
        "MRX_FCS_SUCC", "MRX_FCS_ERR", "MRX_ALC_FAIL", "MRX_MISS");
  
    SMAC_REG_READ(sh, ADR_MRX_FCS_SUCC, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);

    
    SMAC_REG_READ(sh, ADR_MRX_FCS_ERR, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);
    
    SMAC_REG_READ(sh, ADR_MRX_ALC_FAIL, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);

    SMAC_REG_READ(sh, ADR_MRX_MISS, &value); 
    snprintf_res(cmd_data, "[%08x]\n", value);

    snprintf_res(cmd_data, "%-10s\t\t%-10s\t\t%-10s\t%-10s\n", 
        "MRX_MB_MISS", "MRX_NIDLE_MISS", "DBG_LEN_ALC_FAIL", "DBG_LEN_CRC_FAIL");

    SMAC_REG_READ(sh, ADR_MRX_MB_MISS, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);
    
    SMAC_REG_READ(sh, ADR_MRX_NIDLE_MISS, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);
    
    SMAC_REG_READ(sh, ADR_DBG_LEN_ALC_FAIL, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);
    
    SMAC_REG_READ(sh, ADR_DBG_LEN_CRC_FAIL, &value); 
    snprintf_res(cmd_data, "[%08x]\n\n", value);

    snprintf_res(cmd_data, "%-10s\t\t%-10s\t\t%-10s\t%-10s\n",
        "DBG_AMPDU_PASS", "DBG_AMPDU_FAIL", "ID_ALC_FAIL1", "ID_ALC_FAIL2");

    SMAC_REG_READ(sh, ADR_DBG_AMPDU_PASS, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);
 
    SMAC_REG_READ(sh, ADR_DBG_AMPDU_FAIL, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);
 
    SMAC_REG_READ(sh, ADR_ID_ALC_FAIL1, &value); 
    snprintf_res(cmd_data, "[%08x]\t\t", value);
 
    SMAC_REG_READ(sh, ADR_ID_ALC_FAIL2, &value); 
    snprintf_res(cmd_data, "[%08x]\n\n", value);
    
    HAL_DUMP_MIB_RX_PHY(sh);
}

static void ssv6051_cmd_mib(struct ssv_softc *sc, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sc->cmd_data;
    /**
        *  mib [reset|rx]
        * (1) mib reset
        * (2) mib rx
        */
    if ((argc == 2) && (!strcmp(argv[1], "reset"))) {
        ssv6051_reset_mib(sc->sh);
        snprintf_res(cmd_data, " => MIB reseted\n");

    } else if ((argc == 2) && (!strcmp(argv[1], "list"))) {
        ssv6051_list_mib(sc->sh);      
    } else if ((argc == 2) && (strcmp(argv[1], "rx") == 0)) {
        ssv6051_dump_mib_rx(sc->sh);
    } else {
        snprintf_res(cmd_data, "mib [reset|list|rx]\n\n");
    }   
}

static void ssv6051_cmd_power_saving(struct ssv_softc *sc, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sc->cmd_data;
    char *endp;
    /**
        *  ps [aid_value]
        * aid_value = 0: deep sleep mode
        * aid_value = 1: doze mode, aid = 1
        * aid_value = 2: doze mode, aid = 2
        * ...
        *
        */
    if ((argc == 2) && (argv[1])) {
        sc->ps_aid = simple_strtoul(argv[1], &endp, 10);              
    } else {
        snprintf_res(cmd_data, "ps [aid_value]\n\n");
    }   
    
    ssv6xxx_trigger_pmu(sc);
}


static void ssv6051_get_rd_id_adr(u32 *id_base_address)
{
        
    id_base_address[0] = ADR_RD_ID0;
    id_base_address[1] = ADR_RD_ID1;
    id_base_address[2] = ADR_RD_ID2;
    id_base_address[3] = ADR_RD_ID3;
}

static int ssv6051_burst_read_reg(struct ssv_hw *sh, u32 *addr, u32 *buf, u8 reg_amount)
{
    //not support
    return -EOPNOTSUPP;
}

static int ssv6051_burst_write_reg(struct ssv_hw *sh, u32 *addr, u32 *buf, u8 reg_amount)
{
    //not support
    return -EOPNOTSUPP;
}

static int ssv6051_auto_gen_nullpkt(struct ssv_hw *sh, int hwq)
{
    return -EOPNOTSUPP;
}

int ssv6051_ampdu_rx_start(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_sta *sta, 
        u16 tid, u16 *ssn, u8 buf_size)
{
    struct ssv_softc *sc = hw->priv;
    struct ssv_sta_priv_data *sta_priv;
    struct ssv_sta_info *sta_info;

#ifdef WIFI_CERTIFIED
    if (sc->rx_ba_session_count >= sc->sh->rx_info.rx_ba_ma_sessions) {
        /*  Workaround solution:
		 *	ASIC just support one AMPDU RX 
		 *	BA session, not partial state. 
		 *	Need to cancel previous BA session
	     */					
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,1,0)
        ieee80211_stop_rx_ba_session(vif, (1<<(sc->ba_tid)), sc->ba_ra_addr);
#endif
        sc->rx_ba_session_count--;
    }
#else
            
    /*
     *   Fix mantis issue 41 & 43.
     *   (AP mode)Repeatability of BA request can cause problems with the connection.
     */
    if ((sc->rx_ba_session_count >= sc->sh->rx_info.rx_ba_ma_sessions) && (sc->rx_ba_sta != sta)) {
        return -EBUSY;
    }
    else if ((sc->rx_ba_session_count >= sc->sh->rx_info.rx_ba_ma_sessions) && (sc->rx_ba_sta == sta)) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,1,0)
        ieee80211_stop_rx_ba_session(vif,(1<<(sc->ba_tid)),sc->ba_ra_addr);
#endif
        sc->rx_ba_session_count--;
    }
#endif //end of WIFI_CERTIFIED
    printk(KERN_ERR "IEEE80211_AMPDU_RX_START %02X:%02X:%02X:%02X:%02X:%02X %d.\n",
        sta->addr[0], sta->addr[1], sta->addr[2], sta->addr[3],
        sta->addr[4], sta->addr[5], tid);

    sc->rx_ba_session_count++;
    
    sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
    sta_info = (struct ssv_sta_info *)sta_priv->sta_info;
    sta_info->s_flags |= STA_FLAG_AMPDU_RX;

    sc->rx_ba_sta = sta;
    sc->ba_tid = tid;
    sc->ba_ssn = *ssn;
    memcpy(sc->ba_ra_addr, sta->addr, ETH_ALEN);

    return 0;
}

static void ssv6051_ampdu_ba_handler (struct ieee80211_hw *hw, struct sk_buff *skb,
    u32 tx_pkt_run_no)
{
    struct ssv_softc *sc = hw->priv;
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) (skb->data
                                                          + sc->sh->rx_desc_len);
    AMPDU_BLOCKACK *BA_frame = (AMPDU_BLOCKACK *) hdr;
    struct ieee80211_sta *sta;
    struct ssv_sta_priv_data *ssv_sta_priv;
    struct ampdu_ba_notify_data *ba_notification;
    u32 ssn, aggr_num = 0, acked_num = 0;
    u8 tid_no;
    u32 sn_bit_map[2];
    struct firmware_rate_control_report_data *report_data;
    HDR_HostEvent *host_evt;

    sta = ssv6xxx_find_sta_by_rx_skb(sc, skb);

    if (sta == NULL)
    {
        if (skb->len > AMPDU_BA_FRAME_LEN)
        {
            char strbuf[256];
            struct ampdu_ba_notify_data *ba_notification =
                    (struct ampdu_ba_notify_data *) (skb->data + skb->len
                                                     - sizeof(struct ampdu_ba_notify_data));
            ssv6200_dump_BA_notification(strbuf, ba_notification);
            prn_aggr_err(sc, "BA from not connected STA (%02X-%02X-%02X-%02X-%02X-%02X) (%s)\n",
                    BA_frame->ta_addr[0], BA_frame->ta_addr[1], BA_frame->ta_addr[2],
                    BA_frame->ta_addr[3], BA_frame->ta_addr[4], BA_frame->ta_addr[5], strbuf);
        }
        dev_kfree_skb_any(skb);
        return;
    }

    ssv_sta_priv = (struct ssv_sta_priv_data *) sta->drv_priv;
    ssn = BA_frame->BA_ssn;
    sn_bit_map[0] = BA_frame->BA_sn_bit_map[0];
    sn_bit_map[1] = BA_frame->BA_sn_bit_map[1];

    tid_no = BA_frame->tid_info;

    ssv_sta_priv->ampdu_mib_total_BA_counter++;

    if (ssv_sta_priv->ampdu_tid[tid_no].state == AMPDU_STATE_STOP)
    {
        prn_aggr_err(sc, "%s state == AMPDU_STATE_STOP.\n",__func__);
        dev_kfree_skb_any(skb);
        return;
    }

    ssv_sta_priv->ampdu_tid[tid_no].mib.ampdu_mib_BA_counter++;

    if (skb->len <= AMPDU_BA_FRAME_LEN)
    {
        /* ToDo: Check BA without BA notification.
         * In some cases, BA comes after timeout in FW. AMPDU is retried by FW
         * but BA is passed up without BA notification after timerout handler.
         * Process such kind of BA would help BA window shift sooner.
         */
        prn_aggr_err(sc, "b %d\n", ssn);
        dev_kfree_skb_any(skb);
        return;
    }

    // Get BA notify data at end of the packet.
    ba_notification =
            (struct ampdu_ba_notify_data *) (skb->data + skb->len
                                             - sizeof(struct ampdu_ba_notify_data));
                                             
    ssv6xxx_find_txpktrun_no_from_ssn(sc, ba_notification->seq_no[0], ssv_sta_priv);

    // Process BA map
    //prn_aggr_err(sc, "B %d\n", ssn);
    aggr_num = ssv6200_ba_map_walker(&(ssv_sta_priv->ampdu_tid[tid_no]), ssn,
                              sn_bit_map, ba_notification, &acked_num);
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    if (ssv_sta_priv->ampdu_tid[tid_no].debugfs_dir)
    {
        struct sk_buff *dup_skb;
        if (skb_queue_len(&ssv_sta_priv->ampdu_tid[tid_no].ba_q) > 24)
        {
            struct sk_buff *ba_skb = skb_dequeue(&ssv_sta_priv->ampdu_tid[tid_no].ba_q);
            if (ba_skb)
                dev_kfree_skb_any(ba_skb);
        }
        
        dup_skb = skb_clone(skb, GFP_ATOMIC);
        if (dup_skb)
            skb_queue_tail(&ssv_sta_priv->ampdu_tid[tid_no].ba_q, dup_skb);
    }
    #endif // CONFIG_SSV6XXX_DEBUGFS
    // Trim BA notification.
    skb_trim(skb, skb->len - sizeof(struct ampdu_ba_notify_data));
    //prn_aggr_err(sc, "B %d - %d - %d - %d\n", ssn, orig_len, skb->len, (u32)(SSV6XXX_RX_DESC_LEN));

    // Reuse skb as event for rate control
    host_evt = (HDR_HostEvent *) skb->data;
    //host_evt->c_type = HOST_EVENT;
    host_evt->h_event = SOC_EVT_RC_AMPDU_REPORT;
    //host_evt->len = evt_size;
    report_data =
            (struct firmware_rate_control_report_data *) &host_evt->dat[0];

    memcpy(report_data, ba_notification,
           sizeof(struct firmware_rate_control_report_data));

    report_data->ampdu_len = aggr_num;
    report_data->ampdu_ack_len = acked_num;

#ifdef RATE_CONTROL_HT_PERCENTAGE_TRACE
    if((acked_num) && (acked_num != aggr_num))
    {
        int i;
        for (i = 0; i < SSV62XX_TX_MAX_RATES ; i++) {
            if(report_data->rates[i].data_rate == -1)
                break;

            if(report_data->rates[i].count == 0) {
                    printk("*********************************\n");
                    printk("       Illegal HT report         \n");
                    printk("*********************************\n");
            }
            printk("        i=[%d] rate[%d] count[%d]\n",i,report_data->rates[i].data_rate,report_data->rates[i].count);
        }
        printk("AMPDU percentage = %d%% \n",acked_num*100/aggr_num);
    }
    else if(acked_num == 0)
    {
        printk("AMPDU percentage = 0%% aggr_num=%d acked_num=%d\n",aggr_num,acked_num);
    }
#endif

    skb_queue_tail(&sc->rc_report_queue, skb);
    if (sc->rc_report_sechedule == 0)
        queue_work(sc->rc_report_workqueue, &sc->rc_report_work);
} // end of - ssv6051_ampdu_BA_handler -
   
static void ssv6051_load_fw_enable_mcu(struct ssv_hw *sh)
{
	SMAC_REG_WRITE(sh, ADR_BRG_SW_RST, 0x1);
}

static int ssv6051_load_fw_disable_mcu(struct ssv_hw *sh)
{
	u32 clk_en;

	SMAC_REG_WRITE(sh, ADR_BRG_SW_RST, 0x0);
	SMAC_REG_WRITE(sh, ADR_BOOT, 0x0);
	SMAC_REG_READ(sh, ADR_PLATFORM_CLOCK_ENABLE, &clk_en);
	SMAC_REG_WRITE(sh, ADR_PLATFORM_CLOCK_ENABLE, (clk_en | (1 << 2)));

    return 0;
}

static int ssv6051_load_fw_set_status(struct ssv_hw *sh, u32 status)
{
	return SMAC_REG_WRITE(sh, ADR_TX_SEG, status);
}

static int ssv6051_load_fw_get_status(struct ssv_hw *sh, u32 *status)
{
	return SMAC_REG_READ(sh, ADR_TX_SEG, status);
}

static void ssv6051_load_fw_pre_config_device(struct ssv_hw *sh)
{
	HCI_LOAD_FW_PRE_CONFIG_DEVICE(sh->hci.hci_ctrl);
}

static void ssv6051_load_fw_post_config_device(struct ssv_hw *sh)
{
	HCI_LOAD_FW_POST_CONFIG_DEVICE(sh->hci.hci_ctrl);
}

static int ssv6051_reset_cpu(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support reset cpu for this model!! \n");
    return 0;
}

static void ssv6051_set_sram_mode(struct ssv_hw *sh, enum SSV_SRAM_MODE mode)
{
    struct ssv_softc *sc = sh->sc;
    
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support set sram mode for this model!! \n");
}

void ssv6051_enable_usb_acc(struct ssv_softc *sc, u8 epnum)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	     "Not support enable USB acc for this model!! \n");
}

void ssv6051_disable_usb_acc(struct ssv_softc *sc, u8 epnum)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	     "Not support disable USB acc for this model!! \n");
}

static void ssv6051_set_usb_lpm(struct ssv_softc *sc, u8 enable)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
        "Not support set usb lpm for this model!!\n");
}

static int ssv6051_jump_to_rom(struct ssv_softc *sc)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support jump to rom for this model!! \n");
    return 0;
}

static void ssv6051_adj_config(struct ssv_hw *sh)
{
    
    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ){
        printk("%s: clear 5G setting \n", __func__);
        sh->cfg.hw_caps = sh->cfg.hw_caps & (~(SSV6200_HW_CAP_5GHZ));
    }

    if (sh->cfg.hw_caps & SSV6200_HW_CAP_HT40){
        printk("%s: clear ht40 setting \n", __func__);
        sh->cfg.hw_caps = sh->cfg.hw_caps & (~(SSV6200_HW_CAP_HT40));
    }      

	if (sh->cfg.hw_caps & SSV6200_HW_CAP_HCI_RX_AGGR) {
        printk("%s: clear hci rx aggregation setting \n", __func__);
        sh->cfg.hw_caps = sh->cfg.hw_caps & (~(SSV6200_HW_CAP_HCI_RX_AGGR));
	}
    
    if (sh->cfg.rx_burstread) { 
        printk("%s: clear hci rx aggregation setting \n", __func__);
        sh->rx_mode = RX_BURSTREAD_MODE;
    }
    
    if (sh->cfg.tx_stuck_detect) {
        printk("%s: tx_stuck_detect set to 1, force it to 0 \n", __func__);
        sh->cfg.tx_stuck_detect = false;
    }
}

static void ssv6051_get_fw_name(u8 *fw_name)
{
    strcpy(fw_name, "ssv6051-sw.bin");
}

static void ssv6051_send_tx_poll_cmd(struct ssv_hw *sh, u32 type)
{
    return;
}

static bool ssv6051_need_sw_cipher(struct ssv_hw *sh)
{
    return true;
}

void ssv_attach_ssv6051_mac(struct ssv_hal_ops *hal_ops)
{
    hal_ops->adj_config = ssv6051_adj_config;
    hal_ops->need_sw_cipher = ssv6051_need_sw_cipher;
    hal_ops->init_mac = ssv6051_init_mac;
    hal_ops->reset_sysplf = ssv6051_reset_sysplf;
    hal_ops->alloc_hw = ssv6051_alloc_hw;
    hal_ops->init_hw_sec_phy_table = ssv6051_init_hw_sec_phy_table;
    hal_ops->write_mac_ini = ssv6051_write_mac_ini;
    hal_ops->use_hw_encrypt = ssv6051_use_hw_encrypt;
    hal_ops->set_rx_flow = ssv6051_set_rx_flow;
    hal_ops->set_rx_ctrl_flow = ssv6051_set_rx_ctrl_flow;
    hal_ops->set_macaddr = ssv6051_set_macaddr;
    hal_ops->set_bssid = ssv6051_set_bssid;
    hal_ops->get_ic_time_tag = ssv6051_get_ic_time_tag;
    hal_ops->get_chip_id = ssv6051_get_chip_id;
    hal_ops->if_chk_mac2 = ssv6051_if_chk_mac2;    

    hal_ops->save_hw_status = ssv6051_save_hw_status;
    hal_ops->get_wsid = ssv6051_get_wsid;
    hal_ops->set_hw_wsid = ssv6051_set_hw_wsid;
    hal_ops->del_hw_wsid = ssv6051_del_hw_wsid;
    hal_ops->add_fw_wsid = ssv6051_add_fw_wsid;
    hal_ops->del_fw_wsid = ssv6051_del_fw_wsid;
    hal_ops->enable_fw_wsid = ssv6051_enable_fw_wsid;
    hal_ops->disable_fw_wsid = ssv6051_disable_fw_wsid;
    hal_ops->set_fw_hwwsid_sec_type = ssv6051_set_fw_hwwsid_sec_type;
    hal_ops->wep_use_hw_cipher = ssv6051_wep_use_hw_cipher;
    hal_ops->pairwise_wpa_use_hw_cipher = ssv6051_pairwise_wpa_use_hw_cipher;
    hal_ops->group_wpa_use_hw_cipher = ssv6051_group_wpa_use_hw_cipher;
    hal_ops->set_aes_tkip_hw_crypto_group_key = ssv6051_set_aes_tkip_hw_crypto_group_key;
    hal_ops->write_pairwise_keyidx_to_hw = ssv6051_write_pairwise_keyidx_to_hw;
    hal_ops->write_group_keyidx_to_hw = ssv6051_write_hw_group_keyidx;    
    hal_ops->write_pairwise_key_to_hw = ssv6051_write_pairwise_key_to_hw;
    hal_ops->write_group_key_to_hw = ssv6051_write_group_key_to_hw;
    hal_ops->write_key_to_hw = ssv6051_write_key_to_hw;
    hal_ops->set_pairwise_cipher_type = ssv6051_set_pairwise_cipher_type;
    hal_ops->set_group_cipher_type = ssv6051_set_group_cipher_type;
    hal_ops->chk_if_support_hw_bssid = ssv6051_chk_if_support_hw_bssid;
    hal_ops->chk_dual_vif_chg_rx_flow = ssv6051_chk_dual_vif_chg_rx_flow;
    hal_ops->restore_rx_flow = ssv6051_restore_rx_flow;
#ifdef CONFIG_PM
	hal_ops->save_clear_trap_reason = ssv6051_save_clear_trap_reason;
	hal_ops->restore_trap_reason = ssv6051_restore_trap_reason;
	hal_ops->pmu_awake = ssv6051_pmu_awake;
#endif
    hal_ops->hw_crypto_key_write_wep = ssv6051_hw_crypto_key_write_wep;
    hal_ops->set_wep_hw_crypto_key = ssv6051_set_wep_hw_crypto_key;
    hal_ops->store_wep_key = ssv6051_store_wep_key;
    hal_ops->put_mic_space_for_hw_ccmp_encrypt = ssv6051_put_mic_space_for_hw_ccmp_encrypt;

    hal_ops->set_replay_ignore = ssv6051_set_replay_ignore;
    hal_ops->update_decision_table_6 = ssv6051_update_decision_table_6;
    hal_ops->update_decision_table = ssv6051_update_decision_table;
    hal_ops->get_fw_version = ssv6051_get_fw_version;
    hal_ops->set_op_mode = ssv6051_set_op_mode;
    hal_ops->set_halt_mngq_util_dtim = ssv6051_set_halt_mngq_util_dtim;
    hal_ops->set_dur_burst_sifs_g = ssv6051_set_dur_burst_sifs_g;
    hal_ops->set_dur_slot = ssv6051_set_dur_slot;
    hal_ops->set_qos_enable = ssv6051_set_qos_enable;
    hal_ops->set_wmm_param = ssv6051_set_wmm_param;
    hal_ops->init_tx_cfg = ssv6051_init_tx_cfg;
    hal_ops->init_rx_cfg = ssv6051_init_rx_cfg;
    hal_ops->alloc_pbuf = ssv6051_alloc_pbuf;
    hal_ops->free_pbuf = ssv6051_free_pbuf;
    hal_ops->ampdu_auto_crc_en = ssv6051_ampdu_auto_crc_en;
    hal_ops->set_rx_ba = ssv6051_set_rx_ba;
    hal_ops->read_efuse = ssv6051_read_efuse;
    hal_ops->chg_clk_src = ssv6051_chg_clk_src;
    
    hal_ops->beacon_get_valid_cfg = ssv6051_beacon_get_valid_cfg;
    hal_ops->set_beacon_reg_lock = ssv6051_set_beacon_reg_lock;
    hal_ops->set_beacon_id_dtim = ssv6051_set_beacon_id_dtim;
    hal_ops->fill_beacon = ssv6051_fill_beacon;
    hal_ops->beacon_enable = ssv6051_beacon_enable;
    hal_ops->set_beacon_info = ssv6051_set_beacon_info;
    hal_ops->get_bcn_ongoing = ssv6051_get_bcn_ongoing;
    hal_ops->beacon_loss_enable = ssv6051_beacon_loss_enable;
    hal_ops->beacon_loss_disable = ssv6051_beacon_loss_disable;
    hal_ops->beacon_loss_config = ssv6051_beacon_loss_config;
    
    hal_ops->update_txq_mask = ssv6051_update_txq_mask;
    hal_ops->readrg_hci_inq_info = ssv6051_readrg_hci_inq_info;
    hal_ops->readrg_txq_info = ssv6051_readrg_txq_info;
    hal_ops->readrg_txq_info2 = ssv6051_readrg_txq_info2;
    
    hal_ops->dump_wsid = ssv6051_dump_wsid;
    hal_ops->dump_decision = ssv6051_dump_decision;
	hal_ops->get_ffout_cnt = ssv6051_get_ffout_cnt;
	hal_ops->get_in_ffcnt = ssv6051_get_in_ffcnt;
    hal_ops->read_ffout_cnt = ssv6051_read_ffout_cnt;
    hal_ops->read_in_ffcnt = ssv6051_read_in_ffcnt;
    hal_ops->read_id_len_threshold = ssv6051_read_id_len_threshold;
    hal_ops->read_tag_status = ssv6051_read_tag_status;
    hal_ops->cmd_mib = ssv6051_cmd_mib;
    hal_ops->cmd_power_saving = ssv6051_cmd_power_saving;
    
    hal_ops->get_rd_id_adr = ssv6051_get_rd_id_adr;
    hal_ops->burst_read_reg = ssv6051_burst_read_reg;
    hal_ops->burst_write_reg = ssv6051_burst_write_reg;
    hal_ops->auto_gen_nullpkt = ssv6051_auto_gen_nullpkt;

    hal_ops->ampdu_rx_start = ssv6051_ampdu_rx_start;    
    hal_ops->ampdu_ba_handler = ssv6051_ampdu_ba_handler;    
    hal_ops->load_fw_enable_mcu = ssv6051_load_fw_enable_mcu;
    hal_ops->load_fw_disable_mcu = ssv6051_load_fw_disable_mcu;
    hal_ops->load_fw_set_status = ssv6051_load_fw_set_status;
    hal_ops->load_fw_get_status = ssv6051_load_fw_get_status;
    hal_ops->load_fw_pre_config_device = ssv6051_load_fw_pre_config_device;
    hal_ops->load_fw_post_config_device = ssv6051_load_fw_post_config_device;
    hal_ops->reset_cpu = ssv6051_reset_cpu;
    hal_ops->set_sram_mode = ssv6051_set_sram_mode;
    hal_ops->enable_usb_acc = ssv6051_enable_usb_acc;
    hal_ops->disable_usb_acc = ssv6051_disable_usb_acc;
    hal_ops->set_usb_lpm = ssv6051_set_usb_lpm;
    hal_ops->jump_to_rom = ssv6051_jump_to_rom;
    hal_ops->get_fw_name = ssv6051_get_fw_name;
    hal_ops->send_tx_poll_cmd = ssv6051_send_tx_poll_cmd;
}


void ssv_attach_ssv6051(struct ssv_softc *sc, struct ssv_hal_ops *hal_ops)
{
    printk(KERN_INFO"Load SSV6051 HAL MAC function \n");
    ssv_attach_ssv6051_mac(hal_ops);
    
    printk(KERN_INFO"Load SSV6051 HAL common PHY function \n");
    ssv_attach_ssv6051_phy(hal_ops);

    if ((strstr(sc->platform_dev->id_entry->name, SSV6051_CHIP) ||
        strstr(sc->platform_dev->id_entry->name, SSV6051_CHIP_ECO3))){
        printk(KERN_INFO"Load SSV6051 HAL CabrioE BB-RF function \n");
        ssv_attach_ssv6051_cabrioE_BBRF(hal_ops);
    } else {
        printk(KERN_INFO"Load SSV6051 HAL CabrioA BB-RF function \n");
        ssv_attach_ssv6051_cabrioA_BBRF(hal_ops);
    }
}

#endif
