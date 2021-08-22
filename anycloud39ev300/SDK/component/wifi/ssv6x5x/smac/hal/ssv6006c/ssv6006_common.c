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
#include "ssv6006_cfg.h"
#include "ssv6006_mac.h"

#include <smac/dev.h>
#include <smac/ssv_rc_minstrel.h>
#include <hal.h>
#include "ssv6006_priv.h"
#include <smac/ssv_skb.h>
#include <hci/hctrl.h>
#include <ssvdevice/ssv_cmd.h>


static struct ssv_hw * ssv6006_alloc_hw (void)
{
    struct ssv_hw *sh;

    sh = kzalloc(sizeof(struct ssv_hw), GFP_KERNEL);
    if (sh == NULL)
        goto out;
    
    memset((void *)sh, 0, sizeof(struct ssv_hw));
   
    sh->page_count = (u8 *)kzalloc(sizeof(u8) * SSV6006_ID_NUMBER, GFP_KERNEL);
    if (sh->page_count == NULL) 
        goto out;
    
    memset(sh->page_count, 0, sizeof(u8) * SSV6006_ID_NUMBER);
    
    return sh;
out:
    if (sh->page_count)
        kfree(sh->page_count);
    if (sh)
        kfree(sh);

    return NULL;
}


static bool ssv6006_use_hw_encrypt(int cipher, struct ssv_softc *sc, 
        struct ssv_sta_priv_data *sta_priv, struct ssv_vif_priv_data *vif_priv )
{
   
    return true;                     
}

static bool ssv6006_if_chk_mac2(struct ssv_hw *sh)
{
    printk(" %s: is not need to check MAC addres 2 for this model \n",__func__); 
    return false;
}

static int ssv6006_get_wsid(struct ssv_softc *sc, struct ieee80211_vif *vif,
    struct ieee80211_sta *sta)
{    
    struct ssv_vif_priv_data *vif_priv = (struct ssv_vif_priv_data *)vif->drv_priv;
    int     s;
    struct ssv_sta_priv_data *sta_priv_dat=NULL;
    struct ssv_sta_info *sta_info;

    for (s = 0; s < SSV_NUM_STA; s++)
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


static void ssv6006_add_fw_wsid(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, 
    struct ieee80211_sta *sta, struct ssv_sta_info *sta_info)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);       
}

static void ssv6006_del_fw_wsid(struct ssv_softc *sc, struct ieee80211_sta *sta,
    struct ssv_sta_info *sta_info)
{    
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);
}

static void ssv6006_enable_fw_wsid(struct ssv_softc *sc, struct ieee80211_sta *sta,
    struct ssv_sta_info *sta_info, enum SSV6XXX_WSID_SEC key_type)
{       
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);
}

static void ssv6006_disable_fw_wsid(struct ssv_softc *sc, int key_idx,
    struct ssv_sta_priv_data *sta_priv, struct ssv_vif_priv_data *vif_priv)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);
}

static void ssv6006_set_fw_hwwsid_sec_type(struct ssv_softc *sc, struct ieee80211_sta *sta,
        struct ssv_sta_info *sta_info, struct ssv_vif_priv_data *vif_priv)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);
}

//  return true if wep use hw cipher, default always use hw cipher
static bool ssv6006_wep_use_hw_cipher(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv)
{
    bool ret = false;
    
    if (sc->sh->cfg.use_sw_cipher != 1) {
        ret = true;
    }
    
    return ret;
}

//  return true if wpa use hw cipher for pairwise, default always use hw cipher
static bool ssv6006_pairwise_wpa_use_hw_cipher(struct ssv_softc *sc, 
    struct ssv_vif_priv_data *vif_priv, enum SSV_CIPHER_E cipher,
    struct ssv_sta_priv_data *sta_priv)
{
    bool ret = false;
    
    if (sc->sh->cfg.use_sw_cipher != 1) {
        ret = true;
    }
    
    return ret;
}

//  return true if group wpa use hw cipher
static bool ssv6006_group_wpa_use_hw_cipher(struct ssv_softc *sc, 
    struct ssv_vif_priv_data *vif_priv, enum SSV_CIPHER_E cipher)
{
    int     ret =false;
    
    if (sc->sh->cfg.use_sw_cipher != 1) {
        ret = true;
    }
    
    return ret;
}


static bool ssv6006_chk_if_support_hw_bssid(struct ssv_softc *sc,
    int vif_idx)
{
    if ((vif_idx >= 0) && (vif_idx < SSV6006_NUM_HW_BSSID))
        return true;

    printk(" %s: VIF %d doesn't support HW BSSID\n", __func__, vif_idx);
    return false;
}

static void ssv6006_chk_dual_vif_chg_rx_flow(struct ssv_softc *sc, 
    struct ssv_vif_priv_data *vif_priv)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);   
}

static void ssv6006_restore_rx_flow(struct ssv_softc *sc, 
    struct ssv_vif_priv_data *vif_priv, struct ieee80211_sta *sta)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);   
}

static int ssv6006_hw_crypto_key_write_wep(struct ssv_softc *sc,
    struct ieee80211_key_conf *keyconf, u8 algorithm, 
    struct ssv_vif_info *vif_info)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);
    return 0;
}

static void ssv6006_set_wep_hw_crypto_key(struct ssv_softc *sc,                                                                   
    struct ssv_sta_info *sta_info, struct ssv_vif_priv_data *vif_priv)
{
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    " %s: is not need for this model \n",__func__);
}


#define CCMP_MIC_LEN 8
// SSV6006, when CCMP security use HW cipher and ccmp header was generated from SW.
// Allocates CCMP MIC field but not process it.
static bool ssv6006_put_mic_space_for_hw_ccmp_encrypt(struct ssv_softc *sc, struct sk_buff *skb) 
{
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    struct ieee80211_key_conf *hw_key = info->control.hw_key;
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
    struct SKB_info_st *skb_info = (struct SKB_info_st *)skb->head;
    struct ieee80211_sta *sta = skb_info->sta;
    struct ssv_vif_priv_data *vif_priv = (struct ssv_vif_priv_data *)info->control.vif->drv_priv;
    struct ssv_sta_priv_data *ssv_sta_priv = sta ? (struct ssv_sta_priv_data *)sta->drv_priv : NULL;
            
    if ( (!ieee80211_is_data_qos(hdr->frame_control) 
          && !ieee80211_is_data(hdr->frame_control))
       || !ieee80211_has_protected(hdr->frame_control) )
        return false;
    
    if (hw_key)
    {
        if(hw_key->cipher != WLAN_CIPHER_SUITE_CCMP)
            return false;
    }

    // Broadcast or Multicast frame
    if (!is_unicast_ether_addr(hdr->addr1))
    {
        if (vif_priv->is_security_valid
            && vif_priv->has_hw_encrypt)
        {
            skb_put(skb, CCMP_MIC_LEN);
            return true;
        }
    }
    // Unicast
    else if (ssv_sta_priv != NULL)
    {
        if (ssv_sta_priv->has_hw_encrypt)
        {
            skb_put(skb, CCMP_MIC_LEN);
            return true;
        }
    }

    return false;
}



static void ssv6006_init_tx_cfg(struct ssv_hw *sh)
{
    u32 dev_type = HCI_DEVICE_TYPE(sh->hci.hci_ctrl);
    
    if ((sh->cfg.tx_id_threshold == 0) || (sh->cfg.tx_id_threshold > SSV6006_ID_TX_THRESHOLD)) {
        if ((dev_type == SSV_HWIF_INTERFACE_USB) &&
             ( (sh->cfg.usb_hw_resource & USB_HW_RESOURCE_CHK_TXID ) == 0)) {
            sh->tx_info.tx_id_threshold = SSV6006_ID_USB_TX_THRESHOLD;
        } else {
     
		    sh->tx_info.tx_id_threshold = SSV6006_ID_TX_THRESHOLD;
	    }
		sh->tx_info.tx_lowthreshold_id_trigger = SSV6006_TX_LOWTHRESHOLD_ID_TRIGGER;
	} else {
		sh->tx_info.tx_id_threshold = sh->cfg.tx_id_threshold;
		sh->tx_info.tx_lowthreshold_id_trigger = (sh->tx_info.tx_id_threshold - 1);
	}

	if ((sh->cfg.tx_page_threshold == 0) || (sh->cfg.tx_page_threshold > SSV6006_PAGE_TX_THRESHOLD)) {
		sh->tx_info.tx_page_threshold = SSV6006_PAGE_TX_THRESHOLD;
		sh->tx_info.tx_lowthreshold_page_trigger = SSV6006_TX_LOWTHRESHOLD_PAGE_TRIGGER;
	} else {
		sh->tx_info.tx_page_threshold = sh->cfg.tx_page_threshold;
		sh->tx_info.tx_lowthreshold_page_trigger = 
				(sh->tx_info.tx_page_threshold - (sh->tx_info.tx_page_threshold/SSV6006_AMPDU_DIVIDER));
	}

    /* For USB, sw resoure should take care of USB FIFO */
	if (dev_type == SSV_HWIF_INTERFACE_USB) {
        sh->tx_info.tx_page_threshold = SSV6006_PAGE_TX_THRESHOLD - SSV6006_USB_FIFO;
    }
	//printk ("%s: usb_hw_resource %d, tx_id_threshold %d, tx_page threshold %d\n", __func__,sh->cfg.usb_hw_resource, 
	//    sh->tx_info.tx_id_threshold , sh->tx_info.tx_page_threshold );

	sh->tx_info.bk_txq_size = SSV6006_ID_AC_BK_OUT_QUEUE;
	sh->tx_info.be_txq_size = SSV6006_ID_AC_BE_OUT_QUEUE;
	sh->tx_info.vi_txq_size = SSV6006_ID_AC_VI_OUT_QUEUE;
	sh->tx_info.vo_txq_size = SSV6006_ID_AC_VO_OUT_QUEUE;
	sh->tx_info.manage_txq_size = SSV6006_ID_MANAGER_QUEUE;

    sh->tx_page_available = sh->tx_info.tx_page_threshold;
	sh->ampdu_divider = SSV6006_AMPDU_DIVIDER;
	
	//info hci
	memcpy(&(sh->hci.hci_ctrl->tx_info), &(sh->tx_info), sizeof(struct ssv6xxx_tx_hw_info));
}

static void ssv6006_init_rx_cfg(struct ssv_hw *sh)
{
	u32 dev_type = HCI_DEVICE_TYPE(sh->hci.hci_ctrl);

    // Set rx id
    sh->rx_info.rx_id_threshold = SSV6006_TOTAL_ID - (sh->tx_info.tx_id_threshold + SSV6006_ID_TX_USB + SSV6006_ID_SEC);
	// Set rx page
    sh->rx_info.rx_page_threshold = SSV6006_TOTAL_PAGE - (sh->tx_info.tx_page_threshold + SSV6006_RESERVED_SEC_PAGE);
	//If usb, it is necessary to reserve 4 page to usb pri-in. 
	if (dev_type == SSV_HWIF_INTERFACE_USB) 
		sh->rx_info.rx_page_threshold = sh->rx_info.rx_page_threshold - (SSV6006_RESERVED_USB_PAGE + SSV6006_USB_FIFO);	
	
	sh->rx_info.rx_ba_ma_sessions = SSV6006_RX_BA_MAX_SESSIONS;
	
	//info hci
	memcpy(&(sh->hci.hci_ctrl->rx_info), &(sh->rx_info), sizeof(struct ssv6xxx_rx_hw_info));
    //printk ("%s: usb_hw_resource %d, rx_id_threshold %d, rx_page threshold %d\n", __func__,sh->cfg.usb_hw_resource, 
    //	    sh->rx_info.rx_id_threshold , sh->rx_info.rx_page_threshold );
}

           
static u32 ssv6006_ba_map_walker (struct AMPDU_TID_st *ampdu_tid, u32 start_ssn,
    u32 sn_bit_map[2], u32 *p_acked_num, struct aggr_ssn *ampdu_ssn, struct ssv_softc *sc)
{
    int i = 0;
    u32 ssn = ampdu_ssn->ssn[0];
    u32 word_idx = (-1), bit_idx = (-1);
    bool found = ssv6xxx_ssn_to_bit_idx(start_ssn, ssn, &word_idx, &bit_idx);
    bool first_found = found;
    u32 aggr_num = 0;
    u32 acked_num = 0;

    // Debug code
    if (found && (word_idx >= 2 || bit_idx >= 32))
        prn_aggr_err(sc, "idx error 1: %d %d %d %d\n",
                     start_ssn, ssn, word_idx, bit_idx);

    while ((i < MAX_AGGR_NUM) && (ssn < SSV_AMPDU_MAX_SSN))
    {
        u32 cur_ssn;
        struct sk_buff *skb = INDEX_PKT_BY_SSN(ampdu_tid, ssn);
        u32 skb_ssn = (skb == NULL) ? (-1) : ampdu_skb_ssn(skb);
        struct SKB_info_st *skb_info;

        aggr_num++;

        if (skb_ssn != ssn){
            prn_aggr_err(sc, "Unmatched SSN packet: %d - %d - %d\n",
                         ssn, skb_ssn, start_ssn);
        } else {
            
            skb_info = (struct SKB_info_st *) (skb->head);
           	if (found && (sn_bit_map[word_idx] & (1 << bit_idx))) {
				if (skb_info->ampdu_tx_status == AMPDU_ST_SENT) { 
               		dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,"%s: mark ssn %d done\n", __func__, ssn);           
               		skb_info->ampdu_tx_status = AMPDU_ST_DONE;
				} else {
               		dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,"%s: Find a MPDU of status %d! ssn %d\n", 
						__func__, skb_info->ampdu_tx_status, ssn);
				}
               	acked_num++;
            } else  {
				if (skb_info->ampdu_tx_status == AMPDU_ST_SENT) { 
               		dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,"%s: mark ssn %d retry\n", __func__, ssn);
               		ssv6xxx_mark_skb_retry(sc, skb_info, skb);
				} else {
               		dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,"%s: Find a MPDU of status %d! ssn %d\n", 
						__func__, skb_info->ampdu_tx_status, ssn);
				}
            }
        }

        cur_ssn = ssn;

        if ((++i >= MAX_AGGR_NUM) || (i >= ampdu_ssn->mpdu_num))
            break;

        ssn = ampdu_ssn->ssn[i];
        if (ssn >= SSV_AMPDU_MAX_SSN)
            break;

        if (first_found) {
            u32 old_word_idx, old_bit_idx;
            found = ssv6xxx_inc_bit_idx(sc, cur_ssn, ssn, &word_idx, &bit_idx);
            old_word_idx = word_idx;
            old_bit_idx = bit_idx;
            if (found && (word_idx >= 2 || bit_idx >= 32)) {
                prn_aggr_err(sc,
                        "idx error 2: %d 0x%08X 0X%08X %d %d (%d %d) (%d %d)\n",
                        start_ssn, sn_bit_map[1], sn_bit_map[0], cur_ssn, ssn, word_idx, bit_idx, old_word_idx, old_bit_idx);
                found = false;
            }else if (!found) {
                int j;
                prn_aggr_err(sc, "SN out-of-order: %d\n", start_ssn);
                for (j = 0; j < ampdu_ssn->mpdu_num; j++)
                   prn_aggr_err(sc, " %d", ampdu_ssn->ssn[j]);
                prn_aggr_err(sc, "\n");
                  
            }
        } else {
            found = ssv6xxx_ssn_to_bit_idx(start_ssn, ssn, &word_idx, &bit_idx);
            first_found = found;
            if (found && (word_idx >= 2 || bit_idx >= 32))
                prn_aggr_err(sc, "idx error 3: %d %d %d %d\n",
                             cur_ssn, ssn, word_idx, bit_idx);
        }
    }
    // BA window head is ack-ed. Release leading ack-ed MPDUs in window.
    //prn_aggr_err(sc, "Br %d\n", start_ssn);
    ssv6xxx_release_frames(ampdu_tid);

    if (p_acked_num != NULL)
        *p_acked_num = acked_num;
    return aggr_num;
} // end of - _ba_map_walker -

static void ssv6006_ampdu_ba_notify(struct ssv_softc *sc, struct ieee80211_sta *sta, 
		u32 pkt_no, int ampdu_len, int ampdu_ack_len)
{
	struct ssv_sta_priv_data *ssv_sta_priv;
	struct ssv_minstrel_sta_priv *minstrel_sta_priv;
	struct ssv_minstrel_ht_sta *mhs;
	struct ssv_minstrel_ht_rpt ht_rpt[SSV6006RC_MAX_RATE_SERIES];
	struct ssv_minstrel_ampdu_rate_rpt *ampdu_rpt;
	int i, rpt_idx = -1;

	ssv_sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
	if((minstrel_sta_priv = (struct ssv_minstrel_sta_priv *)ssv_sta_priv->rc_info) == NULL)
		return;

	if (!minstrel_sta_priv->is_ht)	
		return;
	
	mhs = (struct ssv_minstrel_ht_sta *)&minstrel_sta_priv->ht;
	if (!ssv6006_rc_get_previous_ampdu_rpt(mhs, pkt_no, &rpt_idx))
	{
		//add rate rpt to report list
		ssv6006_rc_add_ampdu_rpt_to_list(sc, mhs, NULL, pkt_no, ampdu_len, ampdu_ack_len);
	} else {
		ampdu_rpt = (struct ssv_minstrel_ampdu_rate_rpt *)&mhs->ampdu_rpt_list[rpt_idx];
		for (i = 0; i < SSV6006RC_MAX_RATE_SERIES; i++) {
			ht_rpt[i].dword = ampdu_rpt->rate_rpt[i].dword;
			ht_rpt[i].count = ampdu_rpt->rate_rpt[i].count;
			ht_rpt[i].success = ampdu_rpt->rate_rpt[i].success;
			ht_rpt[i].last = ampdu_rpt->rate_rpt[i].last;
		}
		ssv_minstrel_ht_tx_status(sc, ssv_sta_priv->rc_info, ht_rpt, SSV6006RC_MAX_RATE_SERIES, 
				ampdu_len, ampdu_ack_len, ampdu_rpt->is_sample);
		ampdu_rpt->used = false;
	}	
}

int ssv6006_ampdu_rx_start(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_sta *sta, 
        u16 tid, u16 *ssn, u8 buf_size)
{
    struct ssv_softc *sc = hw->priv;
    struct ssv_sta_info *sta_info;
    int i = 0;
    bool find_peer = false;

    for (i = 0; i < SSV_NUM_STA; i++) {
        sta_info = &sc->sta_info[i];
        if ((sta_info->s_flags & STA_FLAG_VALID) && (sta == sta_info->sta)) {
            find_peer = true;
            break;
        }
    }
    
    if ((find_peer == false) || (sc->rx_ba_session_count > sc->sh->rx_info.rx_ba_ma_sessions)) 
        return -EBUSY;
    
    // notify mac80211 amdpu rx session success
    if (sta_info->s_flags & STA_FLAG_AMPDU_RX)
        return 0;
    
    printk(KERN_ERR "IEEE80211_AMPDU_RX_START %02X:%02X:%02X:%02X:%02X:%02X %d.\n",
        sta->addr[0], sta->addr[1], sta->addr[2], sta->addr[3],
        sta->addr[4], sta->addr[5], tid);

    sc->rx_ba_session_count++;
    sta_info->s_flags |= STA_FLAG_AMPDU_RX;

    return 0;
}

static void ssv6006_ampdu_ba_handler (struct ieee80211_hw *hw, struct sk_buff *skb,
    u32 tx_pkt_run_no)
{
    struct ssv_softc *sc = hw->priv;
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) (skb->data
                                                          + sc->sh->rx_desc_len);
    AMPDU_BLOCKACK *BA_frame = (AMPDU_BLOCKACK *) hdr;
    struct ieee80211_sta *sta;
    struct ssv_sta_priv_data *ssv_sta_priv;
    int i;
    u32 ssn, aggr_num = 0, acked_num = 0;
    u8 tid_no;
    u32 sn_bit_map[2];

    sta = ssv6xxx_find_sta_by_rx_skb(sc, skb);

    if (sta == NULL) {
        dev_kfree_skb_any(skb);
        return;
    }

    ssv_sta_priv = (struct ssv_sta_priv_data *) sta->drv_priv;
    ssn = BA_frame->BA_ssn;
    sn_bit_map[0] = BA_frame->BA_sn_bit_map[0];
    sn_bit_map[1] = BA_frame->BA_sn_bit_map[1];

    tid_no = BA_frame->tid_info;

    ssv_sta_priv->ampdu_mib_total_BA_counter++;

    if (ssv_sta_priv->ampdu_tid[tid_no].state == AMPDU_STATE_STOP){
        prn_aggr_err(sc, "%s: state == AMPDU_STATE_STOP.\n", __func__);
        dev_kfree_skb_any(skb);
        return;
    }
    
    if ( (tx_pkt_run_no < SSV6XXX_PKT_RUN_TYPE_AMPDU_START) ||(tx_pkt_run_no > SSV6XXX_PKT_RUN_TYPE_AMPDU_END)){
        dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,
            "%s:\n Invalid tx_pkt_run_no %d\n", __func__, tx_pkt_run_no);
        dev_kfree_skb_any(skb);
        return;
    }

    for (i = 0; i < MAX_CONCUR_AMPDU; i ++){
        if (ssv_sta_priv->ampdu_ssn[i].tx_pkt_run_no == tx_pkt_run_no){ // empty location
            dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,
                "%s:\n BA tx_pkt_run_no %d: found at slot %d \n", __func__, tx_pkt_run_no, i); 
            break;
        }     
    }

    if (i == MAX_CONCUR_AMPDU) {
        dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,"%s:tx_pkt_run_no %d not found on sent list.\n", 
				__func__,tx_pkt_run_no );
        dev_kfree_skb_any(skb);
        return;
    }
    
    if (ssv_sta_priv->ampdu_ssn[i].mpdu_num == 0) {
        dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,"%s: invalid sent list\n",__func__ );
        dev_kfree_skb_any(skb);
        return;
    }

    ssv_sta_priv->ampdu_tid[tid_no].mib.ampdu_mib_BA_counter++;

    // Process BA map
    //prn_aggr_err(sc, "B %d\n", ssn);
    aggr_num = ssv6006_ba_map_walker(&(ssv_sta_priv->ampdu_tid[tid_no]), ssn,
                              sn_bit_map, &acked_num, &ssv_sta_priv->ampdu_ssn[i], sc);
    
    //ssv_sta_priv->ampdu_ssn[i].tx_pkt_run_no = 0; // release memory location 
    memset((void*) &ssv_sta_priv->ampdu_ssn[i], 0, sizeof(struct aggr_ssn));
	
	//notify rate control to update status
	ssv6006_ampdu_ba_notify(sc, sta, tx_pkt_run_no, aggr_num, acked_num);
    dev_kfree_skb_any(skb);   

} // end of - ssv6006_ampdu_BA_handler -

static void ssv6006_adj_config(struct ssv_hw *sh)
{
	int dev_type;

        /* 
         * SV6155P: single band HT40 USB
         * SV6156P: single band HT40 SDIO
         * SV6255P: dual band HT40 USB
         * SV6256P: dual band HT40 SDIO
         */
    if (sh->cfg.force_chip_identity) 
        sh->cfg.chip_identity = sh->cfg.force_chip_identity;
            
    if ((sh->cfg.chip_identity == SV6255P) || (sh->cfg.chip_identity == SV6256P)) {
            sh->cfg.hw_caps |= SSV6200_HW_CAP_5GHZ;
    } else {
        printk("not support 5G for this chip!! \n");
        sh->cfg.hw_caps = sh->cfg.hw_caps & (~(SSV6200_HW_CAP_5GHZ));
    }

    if (sh->cfg.use_wpa2_only){
        printk("%s: use_wpa2_only set to 1, force it to 0 \n", __func__);
        sh->cfg.use_wpa2_only = 0;
    }
   
    if (sh->cfg.rx_burstread) {
        printk("%s: rx_burstread set to 1, force it to 0 \n", __func__);
        sh->cfg.rx_burstread = false;
    }

	dev_type = HCI_DEVICE_TYPE(sh->hci.hci_ctrl);
    if (dev_type == SSV_HWIF_INTERFACE_USB){
	    if (sh->cfg.hw_caps & SSV6200_HW_CAP_HCI_RX_AGGR){
            printk("%s: clear hci rx aggregation setting \n", __func__);
        
            sh->cfg.hw_caps = sh->cfg.hw_caps & (~(SSV6200_HW_CAP_HCI_RX_AGGR));
        }
        /* for usb , it can't use 26M or 24M xtal, change to 40M)*/
        if ((sh->cfg.crystal_type == SSV6XXX_IQK_CFG_XTAL_26M) || (sh->cfg.crystal_type == SSV6XXX_IQK_CFG_XTAL_24M)) {            
            sh->cfg.crystal_type = SSV6XXX_IQK_CFG_XTAL_40M;
            printk("%s: for USB, change iqk config crystal_type to %d \n", __func__, sh->cfg.crystal_type);
        }
	}
    
    if ((sh->cfg.tx_stuck_detect) && (dev_type == SSV_HWIF_INTERFACE_SDIO)) {
        printk("%s: tx_stuck_detect set to 1, force it to 0 \n", __func__);
        sh->cfg.tx_stuck_detect = false;
    }
  
    if (strstr(sh->sc->platform_dev->id_entry->name, SSV6006MP)) {
        printk("%s: clear hw beacon \n", __func__);
        sh->cfg.hw_caps &= ~SSV6200_HW_CAP_BEACON;
    }
}

static void ssv6006_get_fw_name(u8 *fw_name)
{
    strcpy(fw_name, "ssv6x5x-sw.bin");
}

static bool ssv6006_need_sw_cipher(struct ssv_hw *sh)
{
    if (sh->cfg.use_sw_cipher){
        return true;
    } else {
        return false;
    }
}

static void ssv6006_send_tx_poll_cmd(struct ssv_hw *sh, u32 type)
{
	struct sk_buff *skb;
	struct cfg_host_cmd *host_cmd;
	int retval = 0;

    if (!sh->cfg.tx_stuck_detect)
        return;

	skb = ssv_skb_alloc(sh->sc, HOST_CMD_HDR_LEN);
	if (!skb) {
		printk("%s(): Fail to alloc cmd buffer.\n", __FUNCTION__);
	}
	
	skb_put(skb, HOST_CMD_HDR_LEN);
	host_cmd = (struct cfg_host_cmd *)skb->data;
    memset(host_cmd, 0x0, sizeof(struct cfg_host_cmd));
	host_cmd->c_type = HOST_CMD;
	host_cmd->RSVD0 = type;
	host_cmd->h_cmd = (u8)SSV6XXX_HOST_CMD_TX_POLL;
	host_cmd->len = skb->len;
	retval = HCI_SEND_CMD(sh, skb);
    if (retval)
        printk("%s(): Fail to send tx polling cmd\n", __FUNCTION__);
    
    ssv_skb_free(sh->sc, skb);
}

static void ssv6006_cmd_set_hwq_limit(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    char *endp;
    
    if ( argc != 3) return;

    if (!strcmp(argv[1], "bk")) {
        sh->cfg.bk_txq_size = simple_strtoul(argv[2], &endp, 0);
    } else if (!strcmp(argv[1], "be")) {
        sh->cfg.be_txq_size = simple_strtoul(argv[2], &endp, 0);
    } else if (!strcmp(argv[1], "vi")) {
        sh->cfg.vi_txq_size = simple_strtoul(argv[2], &endp, 0);
    } else if (!strcmp(argv[1], "vo")) {
        sh->cfg.vo_txq_size = simple_strtoul(argv[2], &endp, 0);
    } else if (!strcmp(argv[1], "mng")) {
        sh->cfg.manage_txq_size = simple_strtoul(argv[2], &endp, 0);
    } else {
        snprintf_res(cmd_data,"\t\t %s is unknown!\n", argv[1]);
    }
}

void ssv_attach_ssv6006_common(struct ssv_hal_ops *hal_ops)
{
    hal_ops->alloc_hw = ssv6006_alloc_hw;
    hal_ops->use_hw_encrypt = ssv6006_use_hw_encrypt;
    hal_ops->if_chk_mac2= ssv6006_if_chk_mac2;

    hal_ops->get_wsid = ssv6006_get_wsid;    
    hal_ops->add_fw_wsid = ssv6006_add_fw_wsid;
    hal_ops->del_fw_wsid = ssv6006_del_fw_wsid;
    hal_ops->enable_fw_wsid = ssv6006_enable_fw_wsid;
    hal_ops->disable_fw_wsid = ssv6006_disable_fw_wsid;

    hal_ops->set_fw_hwwsid_sec_type = ssv6006_set_fw_hwwsid_sec_type;
    hal_ops->wep_use_hw_cipher = ssv6006_wep_use_hw_cipher;
    hal_ops->pairwise_wpa_use_hw_cipher = ssv6006_pairwise_wpa_use_hw_cipher;
    hal_ops->group_wpa_use_hw_cipher = ssv6006_group_wpa_use_hw_cipher;
    hal_ops->chk_if_support_hw_bssid = ssv6006_chk_if_support_hw_bssid;
    hal_ops->chk_dual_vif_chg_rx_flow = ssv6006_chk_dual_vif_chg_rx_flow;
    hal_ops->restore_rx_flow = ssv6006_restore_rx_flow;
    hal_ops->hw_crypto_key_write_wep = ssv6006_hw_crypto_key_write_wep;
    hal_ops->set_wep_hw_crypto_key = ssv6006_set_wep_hw_crypto_key;

    hal_ops->put_mic_space_for_hw_ccmp_encrypt = ssv6006_put_mic_space_for_hw_ccmp_encrypt;

    hal_ops->init_tx_cfg = ssv6006_init_tx_cfg;
    hal_ops->init_rx_cfg = ssv6006_init_rx_cfg;

    hal_ops->ampdu_rx_start = ssv6006_ampdu_rx_start;    
    hal_ops->ampdu_ba_handler = ssv6006_ampdu_ba_handler;
    hal_ops->adj_config = ssv6006_adj_config;
    hal_ops->get_fw_name = ssv6006_get_fw_name;   
    hal_ops->need_sw_cipher = ssv6006_need_sw_cipher;
	hal_ops->send_tx_poll_cmd = ssv6006_send_tx_poll_cmd;
    hal_ops->cmd_hwq_limit = ssv6006_cmd_set_hwq_limit;
}



#endif
