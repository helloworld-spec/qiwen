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
#include <ssv6200.h>
#include "ssv6051_mac.h"
#include <ssv6200_reg.h>
#include <ssv6200_aux.h>
#include <smac/dev.h>
#include <smac/ssv_rc.h>
#include <smac/ssv_ht_rc.h>
#include <hal.h>
#include <smac/ssv_skb.h>
#include <ssvdevice/ssv_cmd.h>
#include <linux_80211.h>

extern const u16 ampdu_max_transmit_length[RATE_TABLE_SIZE];

static u32 ssv6051_set_frame_duration(struct ieee80211_tx_info *info,
            struct ssv_rate_info *ssv_rate, u16 len,
            struct ssv6051_tx_desc *tx_desc, struct fw_rc_retry_params *rc_params, 
            struct ssv_softc *sc)
{
    struct ieee80211_tx_rate *tx_drate;
    u32 frame_time=0, ack_time=0, rts_cts_nav=0, frame_consume_time=0;
    u32 l_length=0, drate_kbps=0, crate_kbps=0;
    bool ctrl_short_preamble=false, is_sgi, is_ht40;
    bool is_ht, is_gf;
    int d_phy ,c_phy, nRCParams, mcsidx;
    struct ssv_rate_ctrl *ssv_rc = NULL;

    /**
        * Decide TX rate according to the info from mac80211 protocol.
        * Here we always use the first data rate as the final tx rate.
        * Note the ieee80211_get_tx_rate() always use  info->control.rates[0].
        */      
    tx_drate = &info->control.rates[0];
	is_sgi = !!(tx_drate->flags & IEEE80211_TX_RC_SHORT_GI);
	is_ht40 = !!(tx_drate->flags & IEEE80211_TX_RC_40_MHZ_WIDTH);
    is_ht = !!(tx_drate->flags & IEEE80211_TX_RC_MCS);
    is_gf = !!(tx_drate->flags & IEEE80211_TX_RC_GREEN_FIELD);

    /**
        * We check if Short Premable is needed for RTS/CTS/ACK control
        * frames by checking BSS's global flag. This flags is updated by
        * BSS's beacon frames.
        */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	if ((info->control.short_preamble) || 
		(tx_drate->flags & IEEE80211_TX_RC_USE_SHORT_PREAMBLE))
		ctrl_short_preamble = true;
#else
    if ((info->control.vif &&
        info->control.vif->bss_conf.use_short_preamble) || 
		(tx_drate->flags & IEEE80211_TX_RC_USE_SHORT_PREAMBLE))
        ctrl_short_preamble = true;
#endif // >= 3.10.0

#ifdef FW_RC_RETRY_DEBUG
    printk("mcs = %d, data rate idx=%d\n",tx_drate->idx, tx_drate[3].count);
#endif
    
    for (nRCParams = 0; (nRCParams < SSV62XX_TX_MAX_RATES) ; nRCParams++)
    {
        if ((rc_params == NULL) || (sc == NULL))
        {
            mcsidx = tx_drate->idx;
            drate_kbps = ssv_rate->drate_kbps;
            crate_kbps = ssv_rate->crate_kbps;
        }
        else
        {
            if(rc_params[nRCParams].count == 0)
            {
                break;
            }
            
            ssv_rc = sc->rc;
            mcsidx = (rc_params[nRCParams].drate - SSV62XX_RATE_MCS_INDEX) % MCS_GROUP_RATES;
            drate_kbps = ssv_rc->rc_table[rc_params[nRCParams].drate].rate_kbps;
            crate_kbps = ssv_rc->rc_table[rc_params[nRCParams].crate].rate_kbps;
        }
        
        /* Calculate data frame transmission time (include SIFS) */
        if (tx_drate->flags & IEEE80211_TX_RC_MCS) {
            frame_time = ssv6xxx_ht_txtime(mcsidx, 
                    len, is_ht40, is_sgi, is_gf);
            d_phy = 0;//no need use this flags in n mode.
        }
        else {
            /**
                    * Calculate frame transmission time for b/g mode:
                    *     frame_time = TX_TIME(frame) + SIFS
                    */
            if ((info->band == INDEX_80211_BAND_2GHZ) &&
                        !(ssv_rate->d_flags & IEEE80211_RATE_ERP_G))
                            d_phy = WLAN_RC_PHY_CCK;
                    else
                            d_phy = WLAN_RC_PHY_OFDM;


            frame_time = ssv6xxx_non_ht_txtime(d_phy, drate_kbps, 
                len, ctrl_short_preamble);
        }


        /* get control frame phy 
         *n mode data frmaes also response g mode control frames.
         */

        if ((info->band == INDEX_80211_BAND_2GHZ) &&
                        !(ssv_rate->c_flags & IEEE80211_RATE_ERP_G))
                            c_phy = WLAN_RC_PHY_CCK;
                    else
                            c_phy = WLAN_RC_PHY_OFDM;

        /**
            * Calculate NAV duration for data frame. The NAV can be classified
            * into the following cases:
            *    [1] NAV = 0 if the frame addr1 is MC/BC or ack_policy = no_ack
            *    [2] NAV = TX_TIME(ACK) + SIFS if non-A-MPDU frame
            *    [3] NAV = TX_TIME(BA) + SIFS if A-MPDU frame
            */
        if (tx_desc->unicast) {
            if (info->flags & IEEE80211_TX_CTL_AMPDU){
                ack_time = ssv6xxx_non_ht_txtime(c_phy, 
                    crate_kbps, BA_LEN,  ctrl_short_preamble);
            } else {  
                ack_time = ssv6xxx_non_ht_txtime(c_phy, 
                    crate_kbps, ACK_LEN, ctrl_short_preamble); 
            }



    //        printk("ack_time[%d] d_phy[%d] drate_kbp[%d] c_phy[%d] crate_kbps[%d] \n ctrl_short_preamble[%d] ssv_rate->d_flags[%08x] ssv_rate->c_flags[%08x]\n",
    //           ack_time, d_phy, ssv_rate->drate_kbps, c_phy, ssv_rate->crate_kbps, ctrl_short_preamble, ssv_rate->d_flags, ssv_rate->c_flags);

            /* to do ..... */



        }

        /**
            * Calculate NAV for RTS/CTS-to-Self frame if RTS/CTS-to-Self
            * is needed for the frame transmission:
            *       RTS_NAV = cts_time + frame_time + ack_time
            *       CTS_NAV = frame_time + ack_time
            */
        if (tx_desc->do_rts_cts & IEEE80211_TX_RC_USE_RTS_CTS) {
            rts_cts_nav = frame_time;
            rts_cts_nav += ack_time; 
            rts_cts_nav += ssv6xxx_non_ht_txtime(c_phy, 
                crate_kbps, CTS_LEN, ctrl_short_preamble);

            /**
                    * frame consume time:
                    *     TxTime(RTS) + SIFS + TxTime(CTS) + SIFS + TxTime(DATA)
                    *     + SIFS + TxTime(ACK)
                    */
            frame_consume_time = rts_cts_nav;
            frame_consume_time += ssv6xxx_non_ht_txtime(c_phy, 
                crate_kbps, RTS_LEN, ctrl_short_preamble);
        }else if (tx_desc->do_rts_cts & IEEE80211_TX_RC_USE_CTS_PROTECT) {
            rts_cts_nav = frame_time;
            rts_cts_nav += ack_time;

            /**
                    * frame consume time:
                    *     TxTime(CTS) + SIFS + TxTime(DATA) + SIFS + TxTime(ACK)
                    */
            frame_consume_time = rts_cts_nav;
            frame_consume_time += ssv6xxx_non_ht_txtime(c_phy, 
                crate_kbps, CTS_LEN, ctrl_short_preamble);
        }
        else{;}



        /* Calculate L-Length if using HT mode */
        if (tx_drate->flags & IEEE80211_TX_RC_MCS) {
            /**
                    * Calculate frame transmission time & L-Length if the 
                    * frame is transmitted using HT-MF/HT-GF format: 
                    *
                    *  [1]. ceil[TXTIME-T_SIGEXT-20)/4], plus 3 cause 
                    *         we need to get ceil
                    *  [2]. ceil[TXTIME-T_SIGEXT-20]/4]*3 -3
                    */
            l_length = frame_time - HT_SIFS_TIME;
            l_length = ((l_length-(HT_SIGNAL_EXT+20))+3)>>2;
            l_length += ((l_length<<1) - 3);
        }
        
        if((rc_params == NULL) || (sc == NULL))
        {
            tx_desc->rts_cts_nav = rts_cts_nav;
            tx_desc->frame_consume_time = (frame_consume_time>>5)+1;;
            tx_desc->dl_length = l_length;
            break;
        }
        else
        {
            rc_params[nRCParams].rts_cts_nav = rts_cts_nav;
            rc_params[nRCParams].frame_consume_time = (frame_consume_time>>5)+1;
            rc_params[nRCParams].dl_length = l_length;

            if(nRCParams == 0)
            {
                // Overwrite the indexes of data rate and control rate in TxInfo
                // The values from tx_drate may differ to params from RC
                tx_desc->drate_idx = rc_params[nRCParams].drate;
                tx_desc->crate_idx = rc_params[nRCParams].crate;
                tx_desc->rts_cts_nav = rc_params[nRCParams].rts_cts_nav;
                tx_desc->frame_consume_time = rc_params[nRCParams].frame_consume_time;
                tx_desc->dl_length = rc_params[nRCParams].dl_length;
            }
        }
    }

    return ack_time;
}

static void ssv6051_update_txinfo (struct ssv_softc *sc, struct sk_buff *skb)
{
    struct ieee80211_hdr            *hdr;
    struct ieee80211_tx_info        *info = IEEE80211_SKB_CB(skb);
    struct ieee80211_sta            *sta;
    struct ssv_sta_info             *sta_info = NULL;
    struct ssv_sta_priv_data        *ssv_sta_priv = NULL;
    struct ssv_vif_priv_data        *vif_priv = (struct ssv_vif_priv_data *)info->control.vif->drv_priv;
    struct ssv6051_tx_desc          *tx_desc = (struct ssv6051_tx_desc *)skb->data;
    struct ieee80211_tx_rate        *tx_drate;
    struct ssv_rate_info             ssv_rate;
    int                              ac, hw_txqid;
    u32                              nav=0;

    if (info->flags & IEEE80211_TX_CTL_AMPDU){
        struct ampdu_hdr_st *ampdu_hdr = (struct ampdu_hdr_st *)skb->head;
        sta = ampdu_hdr->ampdu_tid->sta;
        hdr = (struct ieee80211_hdr *)(skb->data + TXPB_OFFSET + AMPDU_DELIMITER_LEN);
    } else {
        struct SKB_info_st *skb_info = (struct SKB_info_st *)skb->head;
        sta = skb_info->sta;
        hdr = (struct ieee80211_hdr *)(skb->data + TXPB_OFFSET);
    }

    /**
     * Note that the 'sta' may be NULL. In case of the NULL condition,
     * we assign WSID to 0x0F always. The NULL condition always
     * happens before associating to an AP.
     */
    if (sta) {
        ssv_sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
        sta_info = ssv_sta_priv->sta_info;
    }

    /**
     * Decide frame tid & hardware output queue for outgoing
     * frames. Management frames have a dedicate output queue
     * with higher priority in station mode.
     */
    if ((!sc->bq4_dtim) &&
        (ieee80211_is_mgmt(hdr->frame_control) || 
        ieee80211_is_nullfunc(hdr->frame_control) ||
        ieee80211_is_qos_nullfunc(hdr->frame_control))) {
        ac = 4;
        hw_txqid = 4;
    } else if((sc->bq4_dtim) &&
        info->flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM){
        
        /* In AP mode we use queue 4 to send broadcast frame, 
        when more than one station in sleep mode */
        hw_txqid = 4;
        ac = 4;
    } else {
        /* The skb_get_queue_mapping() returns AC */
        ac = skb_get_queue_mapping(skb);
        hw_txqid = sc->tx.hw_txqid[ac]; 
    }   

    /* Get rate */
    tx_drate = &info->control.rates[0];
    ssv6xxx_rc_hw_rate_idx(sc, info, &ssv_rate);

    /**
     * Generate tx info (tx descriptor) in M2 format for outgoing frames.
     * The software MAC of ssv6200 uses M2 format.
     */
    tx_desc->len = skb->len;
    //tx_desc->len = skb->len - sc->sh->tx_desc_len; // Exclude TX descriptor length
    tx_desc->c_type = M2_TXREQ;
    tx_desc->f80211 = 1;
    tx_desc->qos = (ieee80211_is_data_qos(hdr->frame_control))? 1: 0;
    
    if (tx_drate->flags & IEEE80211_TX_RC_MCS) {
        if (ieee80211_is_mgmt(hdr->frame_control) && 
            ieee80211_has_order(hdr->frame_control))
            tx_desc->ht = 1;
    }
    tx_desc->use_4addr = (ieee80211_has_a4(hdr->frame_control))? 1: 0;
    
    tx_desc->more_data = (ieee80211_has_morefrags(hdr->frame_control))? 1: 0;
    tx_desc->stype_b5b4 = (cpu_to_le16(hdr->frame_control)>>4)&0x3;

    tx_desc->frag = (tx_desc->more_data||(hdr->seq_ctrl&0xf))? 1: 0;    
    tx_desc->unicast = (is_multicast_ether_addr(hdr->addr1)) ? 0: 1;
    tx_desc->tx_burst = (tx_desc->frag)? 1: 0;

    tx_desc->wsid = (!sta_info || (sta_info->hw_wsid < 0)) ? 0x0F : sta_info->hw_wsid;
    tx_desc->txq_idx = hw_txqid;
    tx_desc->hdr_offset = TXPB_OFFSET; //SSV6XXX_TX_DESC_LEN
    tx_desc->hdr_len = ssv6xxx_frame_hdrlen(hdr, tx_desc->ht);
    tx_desc->payload_offset = tx_desc->hdr_offset + tx_desc->hdr_len;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	if(info->control.use_rts)
		tx_desc->do_rts_cts = IEEE80211_TX_RC_USE_RTS_CTS;
	else if(info->control.use_cts_prot)
		tx_desc->do_rts_cts = IEEE80211_TX_RC_USE_CTS_PROTECT;
#else
    tx_desc->do_rts_cts = RTS_CTS_PROTECT(tx_drate->flags);
#endif // >= 3.10.0

#if 1
    /*Avoid CTS issue*/
    if(tx_desc->do_rts_cts == IEEE80211_TX_RC_USE_CTS_PROTECT)
        tx_desc->do_rts_cts = IEEE80211_TX_RC_USE_RTS_CTS;
#endif
    if(tx_desc->do_rts_cts == IEEE80211_TX_RC_USE_CTS_PROTECT) {
    	/**
        * Note: if cts-to-self is used, always use B mode rate. Here
        * we use 1Mbps as control rate.
        *
		* All protection frames are transmited at 2Mb/s for 802,11g
		* otherwise we transmit them at 1Mb/s.
		*/
		tx_desc->crate_idx = 0;//1Mbs
    }
    else
        tx_desc->crate_idx = ssv_rate.crate_hw_idx;

    tx_desc->drate_idx = ssv_rate.drate_hw_idx;

    if (tx_desc->unicast == 0)               
        tx_desc->ack_policy = 1; /* no ack */
    else if (tx_desc->qos == 1)
        tx_desc->ack_policy = (*ieee80211_get_qos_ctl(hdr)&0x60)>>5;
    else if(ieee80211_is_ctl(hdr->frame_control))
        tx_desc->ack_policy = 1;/* no ack */

    tx_desc->security = 0;

    tx_desc->fCmdIdx = 0;
    // Packet command flow
    //  - TX queue is always the last one.
    tx_desc->fCmd = (hw_txqid+M_ENG_TX_EDCA0);

    // AMPDU TX frame needs MCU to add RTS protection.
    if (info->flags & IEEE80211_TX_CTL_AMPDU)
    {
        #ifdef AMPDU_HAS_LEADING_FRAME
        tx_desc->fCmd = (tx_desc->fCmd << 4) | M_ENG_CPU;
        #else
        tx_desc->RSVD_1 = 1;
        #endif // AMPDU_HAS_LEADING_FRAME
        tx_desc->aggregation = 1;
        // FW retry AMPDU. Ack policy for MAC TX is no-ack. 
        tx_desc->ack_policy = 0x01;

        // RTS/CTS to protect AMPDU
        if (   (tx_desc->do_rts_cts == 0)
            && (   (sc->hw->wiphy->rts_threshold == (-1)) // Use RTS/CTS for A-MPDU, if threshold is no set.
                || ((skb->len - sc->sh->tx_desc_len) > sc->hw->wiphy->rts_threshold)))
        {
            // Raise RTS/CTS such that NAV would be calculate later by ssv6xxx_set_frame_duration.
            tx_drate->flags |= IEEE80211_TX_RC_USE_RTS_CTS;
            tx_desc->do_rts_cts = 1;
        }
    }

    // Check if need HW encryption
    if (   ieee80211_has_protected(hdr->frame_control)
        && (   ieee80211_is_data_qos(hdr->frame_control)
            || ieee80211_is_data(hdr->frame_control)))
    {
        if (   (tx_desc->unicast && ssv_sta_priv && ssv_sta_priv->has_hw_encrypt)
            || (!tx_desc->unicast && vif_priv && vif_priv->has_hw_encrypt))
        {
            // For multicast frame, find first STA's WSID for group key.
            if (!tx_desc->unicast && !list_empty(&vif_priv->sta_list))
            {
                struct ssv_sta_priv_data *one_sta_priv;
                int                       hw_wsid;
                one_sta_priv = list_first_entry(&vif_priv->sta_list, struct ssv_sta_priv_data, list);
                hw_wsid = one_sta_priv->sta_info->hw_wsid;
                if (hw_wsid != (-1))
                {
                    tx_desc->wsid = hw_wsid;
                }
                //vif_info->
                #if 0
                printk(KERN_ERR "HW ENC %d %02X:%02X:%02X:%02X:%02X:%02X\n",
                       tx_desc->wsid,
                       hdr->addr1[0], hdr->addr1[1], hdr->addr1[2],
                       hdr->addr1[3], hdr->addr1[4], hdr->addr1[5]);
                _ssv6xxx_hexdump("M ", (const u8 *)skb->data, (skb->len > 128) ? 128 : skb->len);
                //tx_desc->tx_report = 1;
                tx_desc->fCmd = (tx_desc->fCmd << 4) | M_ENG_CPU;
                #endif

            }
            tx_desc->fCmd = (tx_desc->fCmd << 4) | M_ENG_ENCRYPT;
            // Debug code
            #if 0
            if (dump_count++ < 10)
            {
                printk(KERN_ERR "HW ENC %d %02X:%02X:%02X:%02X:%02X:%02X\n",
                       tx_desc->wsid,
                       hdr->addr1[0], hdr->addr1[1], hdr->addr1[2],
                       hdr->addr1[3], hdr->addr1[4], hdr->addr1[5]);
                tx_desc->tx_report = 1;
                _ssv6xxx_hexdump("M ", (const u8 *)skb->data, (skb->len > 128) ? 128 : skb->len);
            }
            #endif
            // Debug code
        }
        else if (ssv_sta_priv->need_sw_encrypt)
        {
            //printk(KERN_ERR "LOCAL ENC\n");
        }
        else
        {
            //printk(KERN_ERR "SW ENC\n");
        }
    }
    else
    {
        //printk(KERN_ERR "NO SEC\n");
    }

    //  - HCI is always at the first position.
    tx_desc->fCmd = (tx_desc->fCmd << 4) | M_ENG_HWHCI;
    // Debug code
    #if 0
    if (   ieee80211_is_data_qos(hdr->frame_control)
        || ieee80211_is_data(hdr->frame_control))
    #endif
    #if 0
    if (ieee80211_is_probe_resp(hdr->frame_control))
    {
        //if (dump_count++ < 30)
        {
            printk(KERN_ERR "Probe Resp %d %02X:%02X:%02X:%02X:%02X:%02X\n",
                   tx_desc->wsid,
                   hdr->addr1[0], hdr->addr1[1], hdr->addr1[2],
                   hdr->addr1[3], hdr->addr1[4], hdr->addr1[5]);
            _ssv6xxx_hexdump("M ", (const u8 *)skb->data, (skb->len > 128) ? 128 : skb->len);
        }
    }
    #endif

#if 0
    /* check if need hw security module to encrypt */
    if (   (sc->sh->cfg.hw_caps & SSV6200_HW_CAP_SECURITY)
        && (sc->algorithm != ME_NONE)) {
/*
        //This function is for M0 format.
        if(sta && sta->drv_priv){
            sta_idx = ((struct ssv_sta_priv_data *)sta->drv_priv)->sta_idx;
            if(sc->sta_info[sta_idx].s_flags & STA_FLAG_ENCRYPT) {
                if(ieee80211_is_data(hdr->frame_control))
                    tx_desc->security = 1;
            }
        }
*/
        /* Offload broadcast frame or wep frame to 
                   hw security module and let HW module use
                   WSID 0 to encrypt frame. (AP mode)
               */
        if (   (tx_desc->unicast == 0)
            || (sc->algorithm == ME_WEP104 || sc->algorithm == ME_WEP40))
        {
            tx_desc->wsid = 0;
            //This control bit is for M0 format.
            //tx_desc->security = 1;
        }
    }
#endif

#if 0
    /* Tx-AMPDU patch: 
        * (1) do_rts_cts shall be set to 0 for outgoing AMPDU frame 
        * (2) Sahll not enable hardware security.
        * (3) ack_policy shall be set to 0x01
        */
    if (tx_desc->aggregation) {
        tx_desc->do_rts_cts = 0;
        tx_desc->fCmd = M_ENG_HWHCI|((hw_txqid+M_ENG_TX_EDCA0)<<4); 
        tx_desc->ack_policy = 0x01;
    }
#endif
    /* Calculate all time duration */
    
    if (tx_desc->aggregation == 1)
    {
        struct ampdu_hdr_st *ampdu_hdr = (struct ampdu_hdr_st *)skb->head;
        // Return Hardware rate index (0-38)
        //ssv62xx_ht_rate_update(skb, sc, &tx_desc->rc_params[0]);

        memcpy(&tx_desc->rc_params[0], ampdu_hdr->rates, sizeof(tx_desc->rc_params));
        nav = ssv6051_set_frame_duration(info, &ssv_rate, (skb->len+FCS_LEN), tx_desc, &tx_desc->rc_params[0], sc);
        #ifdef FW_RC_RETRY_DEBUG        
        //for (i = 0; i < SSV62XX_TX_MAX_RATES; i++)
        {
            printk("[FW_RC]:param[0]: drate =%d, count =%d, crate=%d, dl_length =%d, frame_consume_time =%d, rts_cts_nav=%d\n", 
                tx_desc->rc_params[0].drate,tx_desc->rc_params[0].count,tx_desc->rc_params[0].crate, 
                tx_desc->rc_params[0].dl_length, tx_desc->rc_params[0].frame_consume_time, tx_desc->rc_params[0].rts_cts_nav);
            printk("[FW_RC]:param[1]: drate =%d, count =%d, crate=%d, dl_length =%d, frame_consume_time =%d, rts_cts_nav=%d\n", 
                tx_desc->rc_params[1].drate,tx_desc->rc_params[1].count,tx_desc->rc_params[1].crate, 
                tx_desc->rc_params[1].dl_length, tx_desc->rc_params[1].frame_consume_time, tx_desc->rc_params[1].rts_cts_nav);
            printk("[FW_RC]:param[2]: drate =%d, count =%d, crate=%d, dl_length =%d, frame_consume_time =%d, rts_cts_nav=%d\n", 
                tx_desc->rc_params[2].drate,tx_desc->rc_params[2].count,tx_desc->rc_params[2].crate, 
                tx_desc->rc_params[2].dl_length, tx_desc->rc_params[2].frame_consume_time, tx_desc->rc_params[2].rts_cts_nav);
        }
        #endif // FW_RC_RETRY_DEBUG
    }
    else
    {
        nav = ssv6051_set_frame_duration(info, &ssv_rate, (skb->len+FCS_LEN), tx_desc, NULL, NULL);
    }
    
    /**
        * Assign NAV for outgoing frame. Note that we calculate NAV by driver
        * for HT-GF/HT-MF. The b/g mode NAV is calculated by mac80211
        * stack.
        */
		
	//mac80211 duration calculation is error. need to calculate by driver.		
    if (/*(tx_drate->flags & IEEE80211_TX_RC_MCS) && */(tx_desc->aggregation==0)) {
        if (tx_desc->tx_burst == 0) {
            if (tx_desc->ack_policy != 0x01)
                hdr->duration_id = nav;
//debug
//            printk("duration_id = %d\n", hdr->duration_id);
        }
        else {
            /* tx burst for fragmenetation */
        }
    }
} // end of - ssv6051_update_txinfo -

static void ssv6051_dump_tx_desc(struct sk_buff *skb)
{
    struct ssv6051_tx_desc *tx_desc;
    int s;
    u8 *dat;
    
    tx_desc = (struct ssv6051_tx_desc *)skb->data;
    printk("\n>> TX desc:\n");
    for(s = 0, dat= (u8 *)skb->data; s < sizeof(struct ssv6051_tx_desc) /4; s++) {
        printk("%02x%02x%02x%02x ", dat[4*s+3], dat[4*s+2], dat[4*s+1], dat[4*s]);
        if (((s+1)& 0x03) == 0)
            printk("\n");
    }    
    printk(">> Tx Frame:\n");
    for(s = 0, dat = skb->data; s < tx_desc->hdr_len; s++) {
        printk("%02x ", dat[sizeof(struct ssv6051_tx_desc)+s]);
        if (((s+1)& 0x0F) == 0)
            printk("\n");
    }
    printk("\nlength: %d, c_type=%d, f80211=%d, qos=%d, ht=%d, use_4addr=%d, sec=%d\n", 
        tx_desc->len, tx_desc->c_type, tx_desc->f80211, tx_desc->qos, tx_desc->ht,
        tx_desc->use_4addr, tx_desc->security);
    printk("more_data=%d, sub_type=%x, extra_info=%d\n", tx_desc->more_data,
        tx_desc->stype_b5b4, tx_desc->extra_info);
    printk("fcmd=0x%08x, hdr_offset=%d, frag=%d, unicast=%d, hdr_len=%d\n",
        tx_desc->fCmd, tx_desc->hdr_offset, tx_desc->frag, tx_desc->unicast,
        tx_desc->hdr_len);
    printk("tx_burst=%d, ack_policy=%d, do_rts_cts=%d, reason=%d, payload_offset=%d\n", 
        tx_desc->tx_burst, tx_desc->ack_policy, tx_desc->do_rts_cts, 
        tx_desc->reason, tx_desc->payload_offset);
    printk("fcmdidx=%d, wsid=%d, txq_idx=%d\n",
         tx_desc->fCmdIdx, tx_desc->wsid, tx_desc->txq_idx);
    printk("RTS/CTS Nav=%d, frame_time=%d, crate_idx=%d, drate_idx=%d, dl_len=%d\n",
        tx_desc->rts_cts_nav, tx_desc->frame_consume_time, tx_desc->crate_idx, tx_desc->drate_idx,
        tx_desc->dl_length);

}

static void ssv6051_dump_rx_desc(struct sk_buff *skb)
{
    struct ssv6051_rx_desc *rx_desc;
    struct ssv6051_rxphy_info *rxphy;
    int s;
    u8 *dat;

    rx_desc = (struct ssv6051_rx_desc *)skb->data;
    printk(">> RX Descriptor:\n");
    for(s = 0, dat= (u8 *)skb->data; s < sizeof(struct ssv6051_rx_desc) /4; s++) {
        printk("%02x%02x%02x%02x ", dat[4*s+3], dat[4*s+2], dat[4*s+1], dat[4*s]);
        if (((s+1)& 0x03) == 0)
            printk("\n");
    }
    printk("\n>> RX Phy Info:\n");
    for(s =0 , dat= (u8 *)skb->data; s < sizeof(struct ssv6051_rxphy_info) /4; s++) {
        printk("%02x%02x%02x%02x ", dat[4*s+3+ sizeof(*rx_desc)], dat[4*s+2+ sizeof(*rx_desc)], 
            dat[4*s+1+ sizeof(*rx_desc)], dat[4*s+ sizeof(*rx_desc)]);
        if (((s+1)& 0x03) == 0)
            printk("\n");
    }  
    printk("\nlen=%d, c_type=%d, f80211=%d, qos=%d, ht=%d, use_4addr=%d, l3cs_err=%d, l4_cs_err=%d\n",
        rx_desc->len, rx_desc->c_type, rx_desc->f80211, rx_desc->qos, rx_desc->ht, rx_desc->use_4addr,
        rx_desc->l3cs_err, rx_desc->l4cs_err);
    printk("align2=%d, psm=%d, stype_b5b4=%d, extra_info=%d\n", 
        rx_desc->align2, rx_desc->psm, rx_desc->stype_b5b4, rx_desc->extra_info);
    printk("hdr_offset=%d, reason=%d, rx_result=%d\n", rx_desc->hdr_offset,
        rx_desc->reason, rx_desc->RxResult);
    
    rxphy = (struct ssv6051_rxphy_info *)(skb->data + sizeof(*rx_desc));
    printk("phy_rate = %d, aggregate=%d, l_length=%d, l_rate=%d, snr=%d\n", 
        rxphy->rate, rxphy->aggregate, rxphy->l_length, rxphy->l_rate, rxphy->snr);

}

static void ssv6051_add_txinfo (struct ssv_softc *sc, struct sk_buff *skb)
{
    struct ssv6051_tx_desc          *tx_desc;

    /* Request more spaces in front of the payload for ssv6200 tx info: */
    skb_push(skb, TXPB_OFFSET);
    tx_desc = (struct ssv6051_tx_desc *)skb->data;
    memset((void *)tx_desc, 0, TXPB_OFFSET);
    ssv6051_update_txinfo(sc, skb);
    if (sc->log_ctrl & LOG_TX_DESC)
        ssv6051_dump_tx_desc(skb);

} // end of - ssv6051_add_txinfo -

static void ssv6051_update_ampdu_txinfo(struct ssv_softc *sc, struct sk_buff *ampdu_skb)
{
	return;
}

static void ssv6051_add_ampdu_txinfo(struct ssv_softc *sc, struct sk_buff *ampdu_skb)
{
    struct ssv6051_tx_desc *tx_desc;

    ssv6051_add_txinfo(sc, ampdu_skb);

    tx_desc = (struct ssv6051_tx_desc *) ampdu_skb->data;

    // Set TX report for firmware retry.
    tx_desc->tx_report = 1;
}

static int ssv6051_update_null_func_txinfo(struct ssv_softc *sc, struct ieee80211_sta *sta, struct sk_buff *skb)
{
    return -1;
}

static int ssv6051_get_tx_desc_size(struct ssv_hw *sh)
{
	return sizeof(struct ssv6051_tx_desc);
}

static int ssv6051_get_tx_desc_ctype(struct sk_buff *skb)
{
	struct ssv6051_tx_desc *tx_desc = (struct ssv6051_tx_desc *) skb->data;

    return tx_desc->c_type ;
}

static int ssv6051_get_tx_desc_reason(struct sk_buff *skb)
{
	struct ssv6051_tx_desc *tx_desc = (struct ssv6051_tx_desc *) skb->data;

    return tx_desc->reason;
}

static int ssv6051_get_tx_desc_txq_idx(struct sk_buff *skb)
{
	struct ssv6051_tx_desc *tx_desc = (struct ssv6051_tx_desc *) skb->data;

    return tx_desc->txq_idx ;
}

static void ssv6051_tx_rate_update( struct ssv_softc *sc, struct sk_buff *skb)
{
    struct ieee80211_hdr *hdr;
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    struct ssv6051_tx_desc *tx_desc = (struct ssv6051_tx_desc *)skb->data;
    struct ssv_rate_info ssv_rate;
    u32 nav=0;
    int ret = 0;

    //hdr = (struct ieee80211_hdr *)(skb->data+(((info->flags & IEEE80211_TX_CTL_AMPDU)? AMPDU_DELIMITER_LEN: 0)+SSV6XXX_TX_DESC_LEN));
 
    if(tx_desc->c_type > M2_TXREQ)
        return;

    if (!(info->flags & IEEE80211_TX_CTL_AMPDU)) {

        hdr = (struct ieee80211_hdr *)(skb->data +  sizeof(struct ssv6051_tx_desc));

        if (   (   ieee80211_is_data_qos(hdr->frame_control)
                || ieee80211_is_data(hdr->frame_control))
            && (tx_desc->wsid < SSV_RC_MAX_HARDWARE_SUPPORT))
        {
            ret = ssv6xxx_rc_hw_rate_update_check(skb, sc, tx_desc->do_rts_cts);

            if (ret & RC_FIRMWARE_REPORT_FLAG) {
                //Free
                //tx_desc->RSVD_0 = SSV6XXX_RC_COUNTER_CLEAR;
                //Send result
                tx_desc->RSVD_0 = SSV6XXX_RC_REPORT;
                tx_desc->tx_report = 1;
                ret &= 0xf;
            }

            if(ret) {
                /* Get rate */
                //tx_drate = &info->control.rates[0];
                ssv6xxx_rc_hw_rate_idx(sc, info, &ssv_rate);

                tx_desc->crate_idx = ssv_rate.crate_hw_idx;
                tx_desc->drate_idx = ssv_rate.drate_hw_idx;
            
                nav = ssv6051_set_frame_duration(info, &ssv_rate, skb->len+FCS_LEN, tx_desc, NULL, NULL);
                if (tx_desc->tx_burst == 0) {
                    if (tx_desc->ack_policy != 0x01)
                        hdr->duration_id = nav;
                }
            }
        }
    }
}

static void ssv6051_txtput_set_desc(struct ssv_hw *sh, struct sk_buff *skb )
{
    struct ssv6051_tx_desc *tx_desc;

	tx_desc = (struct ssv6051_tx_desc *)skb->data;
	memset((void *)tx_desc, 0xff, sizeof(struct ssv6051_tx_desc));
	tx_desc->len    = skb->len;
	tx_desc->c_type = M2_TXREQ;
	tx_desc->fCmd = (M_ENG_CPU << 4) | M_ENG_HWHCI;
	tx_desc->reason = ID_TRAP_SW_TXTPUT;
}

static void ssv6051_fill_beacon_tx_desc(struct ssv_softc *sc, struct sk_buff* beacon_skb)
{
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(beacon_skb);
	struct ssv6051_tx_desc *tx_desc;
	struct ssv_rate_info ssv_rate;

	/* Insert description space */
	skb_push(beacon_skb, TXPB_OFFSET);

	tx_desc = (struct ssv6051_tx_desc *)beacon_skb->data;
	memset(tx_desc,0, TXPB_OFFSET);

	ssv6xxx_rc_hw_rate_idx(sc, tx_info, &ssv_rate);
	//length
	tx_desc->len            = beacon_skb->len-TXPB_OFFSET;
	tx_desc->c_type         = M2_TXREQ;
	tx_desc->f80211         = 1;
	tx_desc->ack_policy     = 1;//no ack;
	tx_desc->hdr_offset     = TXPB_OFFSET;
	tx_desc->hdr_len        = 24;
	tx_desc->payload_offset = tx_desc->hdr_offset + tx_desc->hdr_len;
	tx_desc->crate_idx      = ssv_rate.crate_hw_idx;
	tx_desc->drate_idx      = ssv_rate.drate_hw_idx;
	//Insert 4 bytes for FCS
	skb_put(beacon_skb, 4);
}    

void ssv6051_fill_lpbk_tx_desc(struct sk_buff *skb, int security, unsigned char rate)
{
    return;
}

static int ssv6051_get_rx_desc_size(struct ssv_hw *sh)
{
	return sizeof(struct ssv6051_rx_desc) + sizeof (struct ssv6051_rxphy_info);
}

static int ssv6051_get_rx_desc_length(struct ssv_hw *sh)
{
	return sizeof(struct ssv6051_rx_desc);
}

static u32 ssv6051_get_rx_desc_wsid(struct sk_buff *skb)
{
    struct ssv6051_rx_desc *rx_desc = (struct ssv6051_rx_desc *)skb->data;
    
    return rx_desc->wsid;
}

static u32 ssv6051_get_rx_desc_rate_idx(struct sk_buff *skb)
{
    struct ssv6051_rx_desc *rx_desc = (struct ssv6051_rx_desc *)skb->data;
    
    return rx_desc->rate_idx;
}

static u32 ssv6051_get_rx_desc_mng_used(struct sk_buff *skb)
{
    struct ssv6051_rx_desc *rx_desc = (struct ssv6051_rx_desc *)skb->data;
    
    return rx_desc->mng_used;
}

static bool ssv6051_is_rx_aggr(struct sk_buff *skb)
{
    struct ssv6051_rxphy_info      *rxphy;
    struct ssv6051_rx_desc *rxdesc = (struct ssv6051_rx_desc *)skb->data;
    
    rxphy = (struct ssv6051_rxphy_info *)(skb->data + sizeof(*rxdesc));
    
    return rxphy->aggregate;
}

static u32 ssv6051_get_rx_desc_ctype(struct sk_buff *skb)
{
    struct ssv6051_rx_desc *rxdesc = (struct ssv6051_rx_desc *)skb->data;
    
    return rxdesc->c_type;
}

static void ssv6051_get_rx_desc_info(struct sk_buff *skb, u32 *packet_len, u32 *c_type,
        u32 *tx_pkt_run_no)
{
    struct ssv6051_rx_desc *rxdesc = (struct ssv6051_rx_desc *)skb->data;
      
    *packet_len = rxdesc->len;
    *c_type = rxdesc->c_type;
    *tx_pkt_run_no = SSV6XXX_PKT_RUN_TYPE_NOTUSED;     // no tx_pkt_run_no for 6051
} 

static bool ssv6051_nullfun_frame_filter(struct sk_buff *skb)
{
    return false;
} 

static void ssv6051_phy_enable(struct ssv_hw *sh, bool val)
{
    SMAC_REG_SET_BITS(sh, ADR_PHY_EN_1, (val << RG_PHY_MD_EN_SFT), RG_PHY_MD_EN_MSK);
}

static void ssv6051_set_phy_mode(struct ssv_hw *sh, bool val)
{
    if (val) { // set phy mode on without enable
        SMAC_REG_WRITE(sh, ADR_PHY_EN_1,(RG_PHYRX_MD_EN_MSK | RG_PHYTX_MD_EN_MSK |
            RG_PHY11GN_MD_EN_MSK | RG_PHY11B_MD_EN_MSK | RG_PHYRXFIFO_MD_EN_MSK | 
            RG_PHYTXFIFO_MD_EN_MSK | RG_PHY11BGN_MD_EN_MSK));
    } else { //clear phy mode
        SMAC_REG_WRITE(sh, ADR_PHY_EN_1, 0x00000000);         
    } 
}

static void ssv6051_edca_enable(struct ssv_hw *sh, bool val)
{
	return;
}

static void ssv6051_edca_stat(struct ssv_hw *sh, int *primary, int *secondary)
{
    *primary = 0;
    *secondary = 0;

	return;
}

static void ssv6051_dump_mib_rx_phy(struct ssv_hw *sh)
{
    u32 value;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
   
    snprintf_res(cmd_data, "PHY B mode:\n");

    snprintf_res(cmd_data, "%-10s\t\t%-10s\t\t%-10s\n", "CRC error","CCA","counter");

    SMAC_REG_READ(sh, ADR_RX_11B_PKT_ERR_AND_PKT_ERR_CNT, &value);
    snprintf_res(cmd_data, "[%08x]\t\t", value & B_PACKET_ERR_CNT_MSK);

    SMAC_REG_READ(sh, ADR_RX_11B_PKT_CCA_AND_PKT_CNT, &value);
    snprintf_res(cmd_data, "[%08x]\t\t", (value & B_CCA_CNT_MSK) >> B_CCA_CNT_SFT);
    
    snprintf_res(cmd_data, "[%08x]\t\t\n\n", value & B_PACKET_CNT_MSK);

    snprintf_res(cmd_data, "PHY G/N mode:\n");

    snprintf_res(cmd_data, "%-10s\t\t%-10s\t\t%-10s\n", "CRC error","CCA","counter");

    SMAC_REG_READ(sh, ADR_RX_11GN_PKT_ERR_CNT, &value);
    snprintf_res(cmd_data, "[%08x]\t\t", value & GN_PACKET_ERR_CNT_MSK);

    SMAC_REG_READ(sh, ADR_RX_11GN_PKT_CCA_AND_PKT_CNT, &value);
    snprintf_res(cmd_data, "[%08x]\t\t", (value & GN_CCA_CNT_MSK) >> GN_CCA_CNT_SFT);

    snprintf_res(cmd_data, "[%08x]\t\t\n\n", value & GN_PACKET_CNT_MSK);
  
}

static void ssv6051_reset_mib_phy(struct ssv_hw *sh)
{
    //Reset PHY MIB
    SMAC_REG_WRITE(sh, ADR_RX_11B_PKT_STAT_EN, 0);         
    msleep(1);                                 
    SMAC_REG_WRITE(sh, ADR_RX_11B_PKT_STAT_EN, RG_PACKET_STAT_EN_11B_MSK);

    SMAC_REG_WRITE(sh, ADR_RX_11GN_STAT_EN, 0);         
    msleep(1);                                 
    SMAC_REG_WRITE(sh, ADR_RX_11GN_STAT_EN, RG_PACKET_STAT_EN_11GN_MSK);

    SMAC_REG_WRITE(sh, ADR_PHY_REG_20_MRX_CNT, 0);         
    msleep(1);                                 
    SMAC_REG_WRITE(sh, ADR_PHY_REG_20_MRX_CNT, RG_MRX_EN_CNT_RST_N_MSK);

}

static void ssv6051_rc_mac80211_rate_idx(struct ssv_softc *sc, 
            int hw_rate_idx, struct ieee80211_rx_status *rxs)
{
    struct ssv_rate_ctrl *ssv_rc=sc->rc;
    struct ssv_rc_rate *rc_rate;
    
    BUG_ON(hw_rate_idx>=RATE_TABLE_SIZE &&
        hw_rate_idx < 0);

    rc_rate = &ssv_rc->rc_table[hw_rate_idx];
    if (rc_rate->rc_flags & RC_FLAG_HT) {
        rxs->flag |= RX_FLAG_HT;
        if (rc_rate->rc_flags & RC_FLAG_HT_SGI)
            rxs->flag |= RX_FLAG_SHORT_GI;
//#if LINUX_VERSION_CODE >= 0x030400        
//        if (rc_rate->rc_flags & RC_FLAG_HT_GF)
//            rxs->flag |= RX_FLAG_HT_GF;
//#endif        
    }
    else {
        if (rc_rate->rc_flags & RC_FLAG_SHORT_PREAMBLE)
            rxs->flag |= RX_FLAG_SHORTPRE;
    }
    rxs->rate_idx = rc_rate->dot11_rate_idx;
}

#ifdef CONFIG_SSV_CCI_IMPROVEMENT

#define RX_11B_CCA_IN_SCAN      0x20300080
#define MAX_CCI_LEVEL 128

struct ssv6xxx_b_cca_control adjust_cci[] = {
/*
    {0 ,44 ,40, 0x00162000},
    {41, 49, 45, 0x00161000},
    {46, 54, 50, 0x00160800},
    {51, 59, 55, 0x00160400},
    {56, 64, 60, 0x00160400},
    {61, 69, 65, 0x00160200},
    {66, 74, 70, 0x00160100},
    {71, 128, 75, 0x00000000},
*/
    {0 , 43, 0x00162000, 0x20400080},
    {40, 48, 0x00161000, 0x20400080},
    {45, 53, 0x00160800, 0x20400080},
    {50, 63, 0x00160400, 0x20400080},
    {60, 68, 0x00160200, 0x20400080},
    {65, 73, 0x00160100, 0x20400080},
    {70, 128, 0x00000000, 0x20300080},
};
void ssv6051_update_scan_cci_setting(struct ssv_softc *sc)
{

   if (sc->cci_set){
       SMAC_REG_READ(sc->sh, 0xCE01000C, &sc->pre_11b_rx_abbctune);
       SMAC_REG_READ(sc->sh, ADR_RX_11B_CCA_CONTROL, &sc->pre_11b_cca_control);
       SMAC_REG_READ(sc->sh, ADR_RX_11B_CCA_1, &sc->pre_11b_cca_1);
       
       SMAC_REG_WRITE(sc->sh, 0xCE01000C, (sc->pre_11b_rx_abbctune | (0x3F << 3)));
       SMAC_REG_WRITE(sc->sh, ADR_RX_11B_CCA_CONTROL, 0x0);
       SMAC_REG_WRITE(sc->sh, ADR_RX_11B_CCA_1, RX_11B_CCA_IN_SCAN);
       sc->cci_set = true;
   }
   if (sc->cci_current_level == 0) {
       sc->cci_current_level = MAX_CCI_LEVEL;
       sc->cci_current_gate = (sizeof(adjust_cci)/sizeof(adjust_cci[0])) - 1;
   }

    return;
}
void ssv6051_recover_scan_cci_setting(struct ssv_softc *sc)
{
    if (sc->cci_set == false) {
        SMAC_REG_WRITE(sc->sh, 0xCE01000C, sc->pre_11b_rx_abbctune);
        SMAC_REG_WRITE(sc->sh, ADR_RX_11B_CCA_CONTROL, sc->pre_11b_cca_control);
        SMAC_REG_WRITE(sc->sh, ADR_RX_11B_CCA_1, sc->pre_11b_cca_1);
        sc->cci_set = true;
    }

    return;
}

//#define DEBUG_MITIGATE_CCI
void ssv6051_update_data_cci_setting
(struct ssv_softc *sc, struct ssv_sta_priv_data *sta_priv, u32 input_level)
{
    s32 i;
    struct ssv_vif_priv_data               *vif_priv = NULL;
    struct ieee80211_vif                   *vif = NULL;

    if (time_after(jiffies, sc->cci_last_jiffies + msecs_to_jiffies(3000))) {

        if (input_level > MAX_CCI_LEVEL) {
            printk("mitigate_cci input error[%d]!!\n",input_level);
            return;
        }
        sc->cci_last_jiffies = jiffies;
        /*
            Only support first interfac.
            We expect the first one is the staion mode or AP mode not P2P. 
        */
        vif = sta_priv->sta_info->vif;
        vif_priv = (struct ssv_vif_priv_data *)vif->drv_priv;
        if (vif_priv->vif_idx){
            printk("Interface skip CCI[%d]!!\n",vif_priv->vif_idx);
            return;
        }
        if (vif->p2p == true) {
            printk("Interface skip CCI by P2P!!\n");
            return;
        }

        if (sc->cci_current_level == 0) {
            sc->cci_current_level = MAX_CCI_LEVEL;
            sc->cci_current_gate = (sizeof(adjust_cci)/sizeof(adjust_cci[0])) - 1;

            sc->cci_set = true;
        }
        sc->cci_start = true;
#ifdef  DEBUG_MITIGATE_CCI
        printk("jiffies=%lu, input_level=%d\n", jiffies, input_level);
#endif
        if(( input_level >= adjust_cci[sc->cci_current_gate].down_level) && (input_level <= adjust_cci[sc->cci_current_gate].upper_level)) {
            sc->cci_current_level = input_level;
#ifdef  DEBUG_MITIGATE_CCI
            printk("Keep the 0xce0020a0[%x] 0xce002008[%x]!!\n"
                ,adjust_cci[sc->cci_current_gate].adjust_cca_control,adjust_cci[sc->cci_current_gate].adjust_cca_1);
#endif
        }
        else {
            // [current_level]30 -> [input_level]75
            if(sc->cci_current_level < input_level) {
                for (i = 0; i < sizeof(adjust_cci)/sizeof(adjust_cci[0]); i++) {
                    if (input_level <= adjust_cci[i].upper_level) {
#ifdef  DEBUG_MITIGATE_CCI
                    printk("gate=%d, input_level=%d, adjust_cci[%d].upper_level=%d, value=%08x\n", 
                                sc->cci_current_gate, input_level, i, adjust_cci[i].upper_level, adjust_cci[i].adjust_cca_control);
#endif
                        sc->cci_current_level = input_level;
                        sc->cci_current_gate = i;

                        SMAC_REG_WRITE(sc->sh, ADR_RX_11B_CCA_CONTROL, adjust_cci[i].adjust_cca_control);
                        SMAC_REG_WRITE(sc->sh, ADR_RX_11B_CCA_1, adjust_cci[i].adjust_cca_1);
#ifdef  DEBUG_MITIGATE_CCI
                        printk("##Set to the 0xce0020a0[%x] 0xce002008[%x]##!!\n"
                            ,adjust_cci[sc->cci_current_gate].adjust_cca_control,adjust_cci[sc->cci_current_gate].adjust_cca_1);
#endif
                        return;
                    }
                }
            }
            // [current_level]75 -> [input_level]30
            else {
                for (i = (sizeof(adjust_cci)/sizeof(adjust_cci[0]) -1); i >= 0; i--) {
                    if (input_level >= adjust_cci[i].down_level) {
#ifdef  DEBUG_MITIGATE_CCI
                        printk("gate=%d, input_level=%d, adjust_cci[%d].down_level=%d, value=%08x\n", 
                                sc->cci_current_gate, input_level, i, adjust_cci[i].down_level, adjust_cci[i].adjust_cca_control);
#endif
                        sc->cci_current_level = input_level;
                        sc->cci_current_gate = i;

                        SMAC_REG_WRITE(sc->sh, ADR_RX_11B_CCA_CONTROL, adjust_cci[i].adjust_cca_control);
                        SMAC_REG_WRITE(sc->sh, ADR_RX_11B_CCA_1, adjust_cci[i].adjust_cca_1);
#ifdef  DEBUG_MITIGATE_CCI
                        printk("##Set to the 0xce0020a0[%x] 0xce002008[%x]##!!\n"
                            ,adjust_cci[sc->cci_current_gate].adjust_cca_control,adjust_cci[sc->cci_current_gate].adjust_cca_1);
#endif
                        return;
                    }
                }
            }
        }
    }
    return;
}
#endif

void ssv6051_update_rssi(struct ssv_softc *sc, 
            struct sk_buff *rx_skb, struct ieee80211_rx_status *rxs)
{
    struct ssv6051_rx_desc *rxdesc = (struct ssv6051_rx_desc *)rx_skb->data;
    struct ssv6051_rxphy_info *rxphy; 
    struct ssv6051_rxphy_info_padding *rxphypad;
    struct ieee80211_sta *sta = NULL;
    struct ssv_sta_priv_data *sta_priv = NULL; 
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(rx_skb->data + sc->sh->rx_desc_len);
    u8     rpci;   
    
    rxphy = (struct ssv6051_rxphy_info *)(rx_skb->data + sizeof(struct ssv6051_rx_desc));
    rxphypad = (struct ssv6051_rxphy_info_padding *)
        (rx_skb->data + rx_skb->len - sizeof(struct ssv6051_rxphy_info_padding));
        
        //max:0 min:-127
    if ((rxdesc->rate_idx < SSV62XX_G_RATE_INDEX && rxphypad->RSVD == 0) ||
        (rxdesc->rate_idx >= SSV62XX_G_RATE_INDEX && rxphy->service == 0))
    {
        if (rxdesc->rate_idx < SSV62XX_G_RATE_INDEX)
            rpci = rxphypad->rpci;
        else
            rpci = rxphy->rpci;        
        if((ieee80211_is_beacon(hdr->frame_control))||(ieee80211_is_probe_resp(hdr->frame_control)))
        {
            // for NL80211_IFTYPE_STATION, wsid would be 0, 1, e.
            // for NL80211_IFTYPE_AP, wisd would be 0, 1, f for no WSID WATCH LIST
            // for NL80211_IFTYPE_AP, wisd would be 0~7 for WSID WATCH list_head
            sta = ssv6xxx_find_sta_by_rx_skb(sc, rx_skb);

            // Encrypted RX packet must be for connected STA.
            if(sta)
            {
                int assoc;
                sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
#ifdef SSV_RSSI_DEBUG /*For debug*/
                printk("beacon %02X:%02X:%02X:%02X:%02X:%02X rxphypad-rpci=%d\n",
                   hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
                   hdr->addr2[3], hdr->addr2[4], hdr->addr2[5], rpci);
#endif
                if(sta_priv->beacon_rssi)
                {
                    sta_priv->beacon_rssi = ((rpci<< RSSI_DECIMAL_POINT_SHIFT)
                        + ((sta_priv->beacon_rssi<<RSSI_SMOOTHING_SHIFT) - sta_priv->beacon_rssi)) >> RSSI_SMOOTHING_SHIFT;
                    rpci = (sta_priv->beacon_rssi >> RSSI_DECIMAL_POINT_SHIFT);
                }
                else
                    sta_priv->beacon_rssi = (rpci<< RSSI_DECIMAL_POINT_SHIFT);
#ifdef SSV_RSSI_DEBUG
                printk("Beacon smoothing RSSI %d\n",rpci);
#endif
#ifdef CONFIG_SSV_CCI_IMPROVEMENT
                #ifdef CONFIG_SSV_CCI_IMPROVEMENT
                
                assoc = ssvxxx_get_sta_assco_cnt(sc);
                
                if ((sc->ap_vif == NULL) && (assoc == 1))
                    ssv6051_update_data_cci_setting(sc, sta_priv, rpci);
                else
                    ssv6051_update_scan_cci_setting(sc);
                #endif
#endif
            }
            
            if (sc->cci_set == false) {
                ssv6051_recover_scan_cci_setting(sc);
            }
            if ( sc->sh->cfg.beacon_rssi_minimal )
            {
                if (rpci > sc->sh->cfg.beacon_rssi_minimal )
                    rpci = sc->sh->cfg.beacon_rssi_minimal;
            }
#if 0 /*For debug*/
            printk("beacon %02X:%02X:%02X:%02X:%02X:%02X rxphypad-rpci=%d RxResult=%x wsid=%x\n",
                   hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
                   hdr->addr2[3], hdr->addr2[4], hdr->addr2[5], pci, rxdesc->RxResult, wsid);
#endif
        }
        rxs->signal = (-rpci);
    } else {

#ifdef SSV_RSSI_DEBUG /*For debug*/
        printk("########unicast: %d, phy: %d, phypad: %d###############\n", rxdesc->unicast, (-rxphy->rpci), (-rxphypad->rpci));
        printk("RSSI, %d, rate_idx, %d\n", rxs->signal, rate_idx);
        printk("rxdesc->RxResult = %x,rxdesc->wsid = %d\n",rxdesc->RxResult,wsid);
#endif

        // for NL80211_IFTYPE_STATION, wsid would be 0, 1, e.
        // for NL80211_IFTYPE_AP, wisd would be 0, 1, f for no WSID WATCH LIST
        // for NL80211_IFTYPE_AP, wisd would be 0~7 for WSID WATCH list_head
        sta = ssv6xxx_find_sta_by_rx_skb(sc, rx_skb);

        // Encrypted RX packet must be for connected STA.
        if(sta)
        {
            sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
            rxs->signal = -(sta_priv->beacon_rssi >> RSSI_DECIMAL_POINT_SHIFT);
        }

#ifdef SSV_RSSI_DEBUG /*For debug*/
        printk("Others signal %d\n",rxs->signal);
#endif
    }
}

bool ssv6051_is_legacy_rate(struct ssv_softc *sc, struct sk_buff *skb)
{
    bool ret = false;
    
    if (ssv6xxx_get_real_index(sc, skb) < SSV62XX_RATE_MCS_INDEX)   // legacy
        ret = true;
        
    return ret;
} 

int ssv6051_ampdu_max_transmit_length(struct sk_buff *skb, int rate_idx)
{
	return ampdu_max_transmit_length[rate_idx];
}

static void ssv6051_rc_algorithm(struct ssv_softc *sc)
{
	struct ieee80211_hw *hw=sc->hw;
	hw->rate_control_algorithm = "ssv6xxx_rate_control";
}

static void ssv6051_set_80211_hw_rate_config(struct ssv_softc *sc)
{
	struct ieee80211_hw *hw=sc->hw;

	hw->max_rates = 4;
	hw->max_rate_tries = HW_MAX_RATE_TRIES;
}

static void ssv6051_rc_rx_data_handler(struct ssv_softc *sc, struct sk_buff *skb, u32 rate_index)
{
	struct ieee80211_hdr *hdr;
	struct ieee80211_sta *sta;
	struct ssv_sta_priv_data *ssv_sta_priv;
	u32 wsid = HAL_GET_RX_DESC_WSID(sc->sh, skb);

    if (sc->log_ctrl & LOG_RX_DESC){
        hdr = (struct ieee80211_hdr *)(skb->data + sc->sh->rx_desc_len);
        
        if (!(ieee80211_is_beacon(hdr->frame_control))){   // log for not beacon frame
            sta = ssv6xxx_find_sta_by_rx_skb(sc, skb);
            if (sta != NULL)
                ssv6051_dump_rx_desc(skb);
        }
    }

	if (wsid >= SSV_RC_MAX_HARDWARE_SUPPORT) {
		
		hdr = (struct ieee80211_hdr *)(skb->data + HAL_GET_RX_DESC_SIZE(sc->sh));
		if (   (ieee80211_is_data(hdr->frame_control))
			&& (!(ieee80211_is_nullfunc(hdr->frame_control)))) {
				
			sta = ssv6xxx_find_sta_by_rx_skb(sc, skb);
			if(sta == NULL)
				return;
	
			ssv_sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
			ssv_sta_priv->rx_data_rate = rate_index;
		}
	}
}

static void ssv6051_rate_report_handler(struct ssv_softc *sc, struct sk_buff *skb)
{
	/* for CARBIO, it doesn't appear the rate report packet */
	WARN_ON(1);
    dev_kfree_skb_any(skb);
}

static void ssv6051_rc_process_rate_report(struct ssv_softc *sc, struct sk_buff *skb)
{
	struct ssv_rate_ctrl *ssv_rc = sc->rc;
	struct cfg_host_event *host_event;
	struct ssv_sta_rc_info *rc_sta = NULL;
	struct firmware_rate_control_report_data *report_data;
	struct ssv_sta_info *ssv_sta;
	u8 hw_wsid = 0;

	host_event = (struct cfg_host_event *)skb->data;
	if ((host_event->h_event == SOC_EVT_RC_AMPDU_REPORT) || 
		(host_event->h_event == SOC_EVT_RC_MPDU_REPORT)) {
		report_data = (struct firmware_rate_control_report_data *)&host_event->dat[0];
		hw_wsid = report_data->wsid;
	} else {
		printk("RC work get garbage!!\n");
		return;
	}

	// Cabrio-E only support 2 WSIDs for hardware MPDU rate control
	if ((hw_wsid >= SSV_RC_MAX_HARDWARE_SUPPORT) && 
		(host_event->h_event == SOC_EVT_RC_MPDU_REPORT))
		return;

	if (hw_wsid >= SSV_NUM_STA) {
		dev_warn(sc->dev, "[RC]hw_wsid[%d] is invaild!!\n", hw_wsid);
		return;
	}
	
	//Please Check WSID match
	ssv_sta = &sc->sta_info[hw_wsid];
	if (ssv_sta->sta == NULL) {
		dev_err(sc->dev, "Null STA %d for RC report.\n", hw_wsid);
		rc_sta = NULL;
	} else {
		struct ssv_sta_priv_data *ssv_sta_priv = (struct ssv_sta_priv_data *)ssv_sta->sta->drv_priv;
		rc_sta = &ssv_rc->sta_rc_info[ssv_sta_priv->rc_idx];
	}

	if (rc_sta == NULL) {
		dev_err(sc->dev, "[RC]rc_sta %d is NULL pointer Check-1!!\n",hw_wsid);
		return;
	}

	if (rc_sta->is_ht) {
		if (hw_wsid < SSV_RC_MAX_HARDWARE_SUPPORT)
			ssv6xxx_legacy_report_handler(sc, skb, rc_sta);
		ssv6xxx_ht_report_handler(sc,skb,rc_sta);
	} else {
		if (hw_wsid < SSV_RC_MAX_HARDWARE_SUPPORT)
			ssv6xxx_legacy_report_handler(sc, skb, rc_sta);
	}
}

static int ssv6051_rc_update_bmode_ctrl_rate(struct ssv_softc *sc, int rate_tbl_idx, int ctrl_rate_idx)
{
	u32 addr, temp32;
	struct ssv_hw *sh = sc->sh;
	struct ssv_rate_ctrl *ssv_rc = (struct ssv_rate_ctrl *)sc->rc;

	//step 1, update rate table
	addr = sh->hw_pinfo+rate_tbl_idx*4;
	ssv_rc->rc_table[rate_tbl_idx].ctrl_rate_idx = ctrl_rate_idx;

	//step 2, update PHY
	SMAC_REG_READ(sh, addr, &temp32);
	//update ack rate in phy table
	temp32 = (temp32 & 0xfffffc0f) | (ctrl_rate_idx << 4);
	SMAC_REG_WRITE(sh, addr, temp32);
	SMAC_REG_CONFIRM(sh, addr, temp32);
	
	return 0;
}

static void ssv6051_rc_update_basic_rate(struct ssv_softc *sc, u32 basic_rates)
{
	int i, rate_idx, pre_rate_idx = 0;

	//only b mode rate
	for (i=0; i<4; i++) {
		if (((basic_rates>>i) & 0x01)) {
			rate_idx = i;
			pre_rate_idx = i;
		} else
			rate_idx = pre_rate_idx;

		ssv6051_rc_update_bmode_ctrl_rate(sc, i, rate_idx);
		/* 
		 * for rate table, b mode index is 0, 1, 2, 3, 4, 5, 6 
		 * index 1 <--> 4
		 * index 2 <--> 5
		 * index 3 <--> 6
		 * */
		if(i)
			ssv6051_rc_update_bmode_ctrl_rate(sc, i+3, rate_idx);
	}
}

s32 ssv6051_rc_ht_update_update(struct sk_buff *skb, struct ssv_softc *sc, struct fw_rc_retry_params *ar)
{
	return ssv62xx_ht_rate_update(skb, sc, ar);
}

bool ssv6051_rc_ht_sta_current_rate_is_cck(struct ieee80211_sta *sta)
{
    /* For Cabrio, ht station just choose mcs rates */
	return false;
}

void ssv_attach_ssv6051_phy(struct ssv_hal_ops *hal_ops)
{
    hal_ops->add_txinfo = ssv6051_add_txinfo;
    hal_ops->update_txinfo = ssv6051_update_txinfo;
    hal_ops->update_ampdu_txinfo = ssv6051_update_ampdu_txinfo;
    hal_ops->add_ampdu_txinfo = ssv6051_add_ampdu_txinfo;
    hal_ops->update_null_func_txinfo = ssv6051_update_null_func_txinfo;
    hal_ops->get_tx_desc_size = ssv6051_get_tx_desc_size;
    hal_ops->get_tx_desc_ctype = ssv6051_get_tx_desc_ctype;
    hal_ops->get_tx_desc_reason = ssv6051_get_tx_desc_reason;
    hal_ops->get_tx_desc_txq_idx = ssv6051_get_tx_desc_txq_idx;
    hal_ops->tx_rate_update = ssv6051_tx_rate_update;
    hal_ops->txtput_set_desc = ssv6051_txtput_set_desc;
    hal_ops->fill_beacon_tx_desc = ssv6051_fill_beacon_tx_desc;
    hal_ops->fill_lpbk_tx_desc = ssv6051_fill_lpbk_tx_desc;
    
    hal_ops->get_rx_desc_size = ssv6051_get_rx_desc_size;
    hal_ops->get_rx_desc_length = ssv6051_get_rx_desc_length;
    hal_ops->get_rx_desc_wsid = ssv6051_get_rx_desc_wsid;
    hal_ops->get_rx_desc_rate_idx = ssv6051_get_rx_desc_rate_idx;
    hal_ops->get_rx_desc_mng_used = ssv6051_get_rx_desc_mng_used;
    hal_ops->is_rx_aggr = ssv6051_is_rx_aggr;
    hal_ops->get_rx_desc_ctype = ssv6051_get_rx_desc_ctype;
    hal_ops->get_rx_desc_info = ssv6051_get_rx_desc_info;
    hal_ops->nullfun_frame_filter = ssv6051_nullfun_frame_filter;
    
    hal_ops->phy_enable = ssv6051_phy_enable;
    hal_ops->set_phy_mode = ssv6051_set_phy_mode;
    hal_ops->edca_enable = ssv6051_edca_enable;
    hal_ops->edca_stat = ssv6051_edca_stat;
    hal_ops->reset_mib_phy = ssv6051_reset_mib_phy;
    hal_ops->dump_mib_rx_phy = ssv6051_dump_mib_rx_phy;
    hal_ops->rc_mac80211_rate_idx = ssv6051_rc_mac80211_rate_idx;
    hal_ops->update_rssi = ssv6051_update_rssi;
#ifdef CONFIG_SSV_CCI_IMPROVEMENT
    hal_ops->update_scan_cci_setting = ssv6051_update_scan_cci_setting;
    hal_ops->recover_scan_cci_setting = ssv6051_recover_scan_cci_setting;
#endif
    hal_ops->is_legacy_rate = ssv6051_is_legacy_rate;
	hal_ops->ampdu_max_transmit_length = ssv6051_ampdu_max_transmit_length;

	hal_ops->rc_algorithm = ssv6051_rc_algorithm;
	hal_ops->set_80211_hw_rate_config = ssv6051_set_80211_hw_rate_config;
	hal_ops->rc_rx_data_handler = ssv6051_rc_rx_data_handler;
	hal_ops->rate_report_handler = ssv6051_rate_report_handler;
	hal_ops->rc_process_rate_report = ssv6051_rc_process_rate_report;
	hal_ops->rc_update_basic_rate = ssv6051_rc_update_basic_rate;
	hal_ops->rc_ht_update_update = ssv6051_rc_ht_update_update;
	hal_ops->rc_ht_sta_current_rate_is_cck = ssv6051_rc_ht_sta_current_rate_is_cck;
}

#endif
