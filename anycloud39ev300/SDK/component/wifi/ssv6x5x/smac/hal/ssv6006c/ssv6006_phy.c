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
#include <ssv6200.h>
#include "ssv6006_mac.h"
#include "ssv6006C_reg.h"
#include "ssv6006C_aux.h"
#include <smac/dev.h>
#include <smac/ssv_rc_minstrel.h>
#include <smac/ssv_rc.h>
#include <smac/ssv_skb.h>
#include <hal.h>
#include "ssv6006_priv.h"
#include "ssv6006_priv_normal.h"
#include <ssvdevice/ssv_cmd.h>
#include <linux_80211.h>
#include "turismo_common.h"


#define DEBUG_MITIGATE_CCI
#define MAX_CCI_LEVEL 128


const size_t ssv6006_tx_desc_length = sizeof(struct ssv6006_tx_desc);
const size_t ssv6006_rx_desc_length = sizeof(struct ssv6006_rx_desc) + sizeof (struct ssv6006_rxphy_info);

static const u32 rc_b_data_rate[4] = {1000, 2000, 5500, 11000};
static const u32 rc_g_data_rate[8] = {6000, 9000, 12000, 18000, 24000, 36000, 48000, 54000};
static const u32 rc_n_ht20_lgi_data_rate[8] = { 6500, 13000, 19500, 26000, 39000,  52000,  58500,  65000};
static const u32 rc_n_ht20_sgi_data_rate[8] = { 7200, 14400, 21700, 28900, 43300,  57800,  65000,  72200};
static const u32 rc_n_ht40_lgi_data_rate[8] = {13500, 27000, 40500, 54000, 81000, 108000, 121500, 135000};
static const u32 rc_n_ht40_sgi_data_rate[8] = {15000, 30000, 45000, 60000, 90000, 120000, 135000, 150000};

static u8 ssv6006_get_tx_desc_drate(struct ssv6006_tx_desc *tx_desc, int idx)
{
    switch (idx) {
        case 0:
            return tx_desc->drate_idx0;
        case 1:
            return tx_desc->drate_idx1;
        case 2:
            return tx_desc->drate_idx2;
        case 3:
            return tx_desc->drate_idx3;
        default:
            return 0;
    }
}

static void ssv6006_set_tx_desc_drate(struct ssv6006_tx_desc *tx_desc, int idx, u8 drate)
{
    switch (idx) {
        case 0:
            tx_desc->drate_idx0 = drate;
            break;
        case 1:
            tx_desc->drate_idx1 = drate;
            break;
        case 2:
            tx_desc->drate_idx2 = drate;
            break;
        case 3:
            tx_desc->drate_idx3 = drate;
            break;
        default:
		    printk("%s: invalid rate index[%d]\n", __FUNCTION__, idx);
           break; 
    }
}

static void ssv6006_set_tx_desc_trycnt(struct ssv6006_tx_desc *tx_desc, int idx, u8 trycnt)
{
    switch (idx) {
        case 0:
            tx_desc->try_cnt0 = trycnt;
            break;
        case 1:
            tx_desc->try_cnt1 = trycnt;
            break;
        case 2:
            tx_desc->try_cnt2 = trycnt;
            break;
        case 3:
            tx_desc->try_cnt3 = trycnt;
            break;
        default:
		    printk("%s: invalid rate index[%d]\n", __FUNCTION__, idx);
           break; 
    }
}

static void ssv6006_set_tx_desc_crate(struct ssv6006_tx_desc *tx_desc, int idx, u8 crate)
{
    switch (idx) {
        case 0:
            tx_desc->crate_idx0 = crate;
            break;
        case 1:
            tx_desc->crate_idx1 = crate;
            break;
        case 2:
            tx_desc->crate_idx2 = crate;
            break;
        case 3:
            tx_desc->crate_idx3 = crate;
            break;
        default:
		    printk("%s: invalid rate index[%d]\n", __FUNCTION__, idx);
           break; 
    }
}

static bool ssv6006_get_tx_desc_last_rate(struct ssv6006_tx_desc *tx_desc, int idx)
{
    switch (idx) {
        case 0:
            return tx_desc->is_last_rate0;
        case 1:
            return tx_desc->is_last_rate1;
        case 2:
            return tx_desc->is_last_rate2;
        case 3:
            return tx_desc->is_last_rate3;
        default:
            return false;
    }
}

static u32 ssv6006_get_data_rates(struct ssv6006_rc_idx *rc_word)
{
    union  { 
        struct ssv6006_rc_idx   rcword;
        u8 val;
    } u;
    u.rcword = *rc_word;

    switch (rc_word->phy_mode){
        case SSV6006RC_B_MODE:
            return rc_b_data_rate[rc_word->rate_idx];

        case SSV6006RC_G_MODE:
            return rc_g_data_rate[rc_word->rate_idx];
        
        case SSV6006RC_N_MODE:
            if (rc_word->long_short == SSV6006RC_LONG){
                if (rc_word->ht40 == SSV6006RC_HT40){
                    return rc_n_ht40_lgi_data_rate[rc_word->rate_idx];
                } else {
                    return rc_n_ht20_lgi_data_rate[rc_word->rate_idx];
                }
            } else {
                if (rc_word->ht40 == SSV6006RC_HT40){
                    return rc_n_ht40_sgi_data_rate[rc_word->rate_idx];
                } else {
                    return rc_n_ht20_sgi_data_rate[rc_word->rate_idx];
                }                           
            }
         
        default:
            printk(" %s: invalid rate control word %x\n", __func__, u.val);
            memset(rc_word, 0 , sizeof(struct ssv6006_rc_idx));
            return 1000;             
    }
}

static void ssv6006_set_frame_duration(struct ssv_softc *sc, struct ieee80211_tx_info *info,
    struct ssv6006_rc_idx *drate_idx, struct ssv6006_rc_idx *crate_idx, 
    u16 len, struct ssv6006_tx_desc *tx_desc, u32 *nav)
{
    //struct ieee80211_tx_rate *tx_drate;
    // frame_consume_time is obosulete.
    u32 frame_time=0, ack_time = 0;// , frame_consume_time=0;
    u32 drate_kbps=0, crate_kbps=0;
    u32 rts_cts_nav[SSV6006RC_MAX_RATE_SERIES] = {0, 0, 0, 0};
    u32 l_length[SSV6006RC_MAX_RATE_SERIES] = {0, 0, 0, 0};
    bool ctrl_short_preamble=false, is_sgi, is_ht40;
    bool is_ht, is_gf, do_rts_cts;
    int d_phy ,c_phy, i, mcsidx;
    struct ssv6006_rc_idx *drate, *crate;
    bool last_rate = false;

    for (i = 0; i < SSV6006RC_MAX_RATE_SERIES ;i++)
    {   
        drate = &drate_idx[i];
        mcsidx = drate->rate_idx;
        is_sgi = drate->long_short;
        ctrl_short_preamble = drate->long_short;
	    is_ht40 = drate->ht40;
        is_ht = (drate->phy_mode == SSV6006RC_N_MODE);
        is_gf = drate->mf;
        drate_kbps = ssv6006_get_data_rates(drate);
        crate = &crate_idx[i];
        crate_kbps = ssv6006_get_data_rates(crate);
        frame_time = 0 ; 
        ack_time = 0;
        *nav = 0;

        /* Calculate data frame transmission time (include SIFS) */
        if (is_ht) {
            frame_time = ssv6xxx_ht_txtime(mcsidx, 
                    len, is_ht40, is_sgi, is_gf);
            d_phy = WLAN_RC_PHY_OFDM;//no need use this flags in n mode.
        } else {
            /**
             * Calculate frame transmission time for b/g mode:
             *     frame_time = TX_TIME(frame) + SIFS
             */
            if (drate->phy_mode == SSV6006RC_B_MODE)
                d_phy = WLAN_RC_PHY_CCK;
            else
                d_phy = WLAN_RC_PHY_OFDM;

            frame_time = ssv6xxx_non_ht_txtime(d_phy, drate_kbps, 
                len, ctrl_short_preamble);
        }

        /* get control frame phy 
         *n mode data frmaes also response g mode control frames.
         */

        if (crate->phy_mode == SSV6006RC_B_MODE)
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
            int threshold;

            if (info->flags & IEEE80211_TX_CTL_AMPDU){
                ack_time = ssv6xxx_non_ht_txtime(c_phy, 
                    crate_kbps, BA_LEN, crate->long_short);
            } else {  
                ack_time = ssv6xxx_non_ht_txtime(c_phy, 
                    crate_kbps, ACK_LEN, crate->long_short); 
            }
            sc->ack_counter++;

            /*CCI patch to improve competimg test performace */
            /*CCI_P1 patch for always*/
            /*CCI_P2 patch for 1/2 */
            if ((sc->sh->cfg.cci & CCI_P1) ||  (sc->sh->cfg.cci & CCI_P2)){
                if (sc->sh->cfg.cci & CCI_P2){
                    threshold = 1;
                } else {
                    threshold = 0;
                }
                if (sc->ack_counter > threshold) {
                    ack_time += 100;
                    sc->ack_counter = 0;
                }
            }
           // printk("ack_time[%d] d_phy[%d] drate_kbp[%d] c_phy[%d] crate_kbps[%d] \n ctrl_short_preamble[%d]\n",
           //    ack_time, d_phy, drate_kbps, c_phy, crate_kbps, ctrl_short_preamble);

            /* to do ..... */
        }

        /**
            * Calculate NAV for RTS/CTS-to-Self frame if RTS/CTS-to-Self
            * is needed for the frame transmission:
            *       RTS_NAV = cts_time + frame_time + ack_time
            *       CTS_NAV = frame_time + ack_time
            */
        switch (i){
            case 0:
                do_rts_cts = tx_desc->do_rts_cts0;
                break;
            case 1:
                do_rts_cts = tx_desc->do_rts_cts1;
                break;
            case 2:
                do_rts_cts = tx_desc->do_rts_cts2;
                break;
            case 3:
                do_rts_cts = tx_desc->do_rts_cts3;
                break;
            default:
                do_rts_cts = tx_desc->do_rts_cts0; 
                break;                               
        }   
        if (do_rts_cts & IEEE80211_TX_RC_USE_RTS_CTS) {
            rts_cts_nav[i] = frame_time;
            rts_cts_nav[i] += ack_time; 
            rts_cts_nav[i] += ssv6xxx_non_ht_txtime(c_phy, 
                crate_kbps, CTS_LEN, crate->long_short);

            /**
                    * frame consume time:
                    *     TxTime(RTS) + SIFS + TxTime(CTS) + SIFS + TxTime(DATA)
                    *     + SIFS + TxTime(ACK)
                    */
            //frame_consume_time = rts_cts_nav;
            // frame_consume_time += ssv6xxx_non_ht_txtime(c_phy, 
            //     crate_kbps, RTS_LEN, ctrl_short_preamble);
        }else if (do_rts_cts & IEEE80211_TX_RC_USE_CTS_PROTECT) {
            rts_cts_nav[i] = frame_time;
            rts_cts_nav[i] += ack_time;

            /**
                    * frame consume time:
                    *     TxTime(CTS) + SIFS + TxTime(DATA) + SIFS + TxTime(ACK)
                    */
           // frame_consume_time = rts_cts_nav;
           // frame_consume_time += ssv6xxx_non_ht_txtime(c_phy, 
           //     crate_kbps, CTS_LEN, ctrl_short_preamble);
        } else{
            rts_cts_nav[i] = 0;
        }

        /* Calculate L-Length if using HT mode */
        if (is_ht) {
            /**
                    * Calculate frame transmission time & L-Length if the 
                    * frame is transmitted using HT-MF/HT-GF format: 
                    *
                    *  [1]. ceil[TXTIME-T_SIGEXT-20)/4], plus 3 cause 
                    *         we need to get ceil
                    *  [2]. ceil[TXTIME-T_SIGEXT-20]/4]*3 -3
                    */
            l_length[i] = frame_time - HT_SIFS_TIME;
            l_length[i] = ((l_length[i]-(HT_SIGNAL_EXT+20))+3)>>2;
            l_length[i] += ((l_length[i]<<1) - 3);
        } else {
            l_length[i] = 0;
        }
        *nav++ = ack_time ;
        
        last_rate = ssv6006_get_tx_desc_last_rate(tx_desc, i);
        if (last_rate) 
            break;
    }
    
    tx_desc->rts_cts_nav0 = rts_cts_nav[0];
    tx_desc->rts_cts_nav1 = rts_cts_nav[1];
    tx_desc->rts_cts_nav2 = rts_cts_nav[2];
    tx_desc->rts_cts_nav3 = rts_cts_nav[3];
    
    //tx_desc->frame_consume_time = (frame_consume_time>>5)+1;;
    tx_desc->dl_length0 = l_length[0];
    tx_desc->dl_length1 = l_length[1];
    tx_desc->dl_length2 = l_length[2];
    tx_desc->dl_length3 = l_length[3];
}

// ssv6006_drword_to_crword convert rate control word in data frame to 
//  rate control word for control frame
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
static u8 ssv6006_drword_to_crword(struct ssv6006_tx_desc *tx_desc, int idx, enum nl80211_band band)
#else
static u8 ssv6006_drword_to_crword(struct ssv6006_tx_desc *tx_desc, int idx, enum ieee80211_band band)
#endif
{
	union {
		struct ssv6006_rc_idx rc_word;
		u8 val;
	} d_rate, c_rate;

    d_rate.val = ssv6006_get_tx_desc_drate(tx_desc, idx);
	c_rate.val = 0;

    switch (d_rate.rc_word.phy_mode) {
		case SSV6006RC_B_MODE:
				c_rate.rc_word.phy_mode = SSV6006RC_B_MODE;
				c_rate.rc_word.rate_idx = SSV6006RC_B_1M;
			break;

		case SSV6006RC_G_MODE:
				if (d_rate.rc_word.rate_idx == SSV6006RC_G_6M) {
                    if (band == INDEX_80211_BAND_5GHZ) {
				        c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
					    c_rate.rc_word.rate_idx = SSV6006RC_G_6M;
                    } else {
				        c_rate.rc_word.phy_mode = SSV6006RC_B_MODE;
					    c_rate.rc_word.rate_idx = SSV6006RC_B_1M;
                    }
                
                } else if ((d_rate.rc_word.rate_idx == SSV6006RC_G_9M) ||
                    (d_rate.rc_word.rate_idx == SSV6006RC_G_12M)) {
                    c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
					c_rate.rc_word.rate_idx = SSV6006RC_G_6M;
                
                } else if ((d_rate.rc_word.rate_idx == SSV6006RC_G_18M) || 
				    (d_rate.rc_word.rate_idx == SSV6006RC_G_24M)) {
                    c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
					c_rate.rc_word.rate_idx = SSV6006RC_G_12M;
				
                } else { 
                    c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
					c_rate.rc_word.rate_idx = SSV6006RC_G_24M;
                }
			break;

		case SSV6006RC_N_MODE:
				if (d_rate.rc_word.rate_idx == SSV6006RC_N_MCS0) {
                    if (band == INDEX_80211_BAND_5GHZ) {
				        c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
					    c_rate.rc_word.rate_idx = SSV6006RC_G_6M;
                    } else {
				        c_rate.rc_word.phy_mode = SSV6006RC_B_MODE;
					    c_rate.rc_word.rate_idx = SSV6006RC_B_1M;
                    }
                    
                } else if (d_rate.rc_word.rate_idx == SSV6006RC_N_MCS1) {
				    c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
					c_rate.rc_word.rate_idx = SSV6006RC_G_6M;
				
                } else if ((d_rate.rc_word.rate_idx == SSV6006RC_N_MCS2) || 
				    (d_rate.rc_word.rate_idx == SSV6006RC_N_MCS3)) {
				    c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
					c_rate.rc_word.rate_idx = SSV6006RC_G_12M;
				
                } else {
				    c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
					c_rate.rc_word.rate_idx = SSV6006RC_G_24M;
                }
			break;

		default:
				printk("%s:Don't support date rate[%02x]\n", __FUNCTION__, d_rate.val);
                if (band == INDEX_80211_BAND_5GHZ) 
				    c_rate.rc_word.phy_mode = SSV6006RC_G_MODE;
                
			break;
	}

    ssv6006_set_tx_desc_crate(tx_desc, idx, c_rate.val);
	return c_rate.val;
}

static void ssv6006_force_lowest_rate(struct ieee80211_tx_info *info, 
            struct ssv6006_tx_desc *tx_desc, struct ssv6006_rc_idx *drate_idx)
{
    u8     i, *drate_idx_tmp, trycnt;
    
	drate_idx_tmp = (u8*) drate_idx;
    for (i = 0; i < SSV6006RC_MAX_RATE_SERIES; i++) {
        *drate_idx_tmp = 0;
        if ((info->band == INDEX_80211_BAND_5GHZ) || (info->control.vif->p2p == true)) {
    	    *drate_idx_tmp = (SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_6M << SSV6006RC_RATE_SFT); 
        }
        
        ssv6006_set_tx_desc_drate(tx_desc, i, *drate_idx_tmp);
        trycnt = (i == 0) ? 1 : 0 ;
        ssv6006_set_tx_desc_trycnt(tx_desc, i, trycnt);
    	
        drate_idx_tmp++; 
	}
    
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
static void ssv6006_set_data_rcword(struct ssv_softc *sc, struct ssv_sta_priv_data *ssv_sta_priv, 
        struct ssv6006_tx_desc *tx_desc, struct ieee80211_tx_info *info, 
        struct ssv6006_rc_idx *drate_idx, bool is_mgmt, bool is_nullfunc, enum nl80211_band band)
#else
static void ssv6006_set_data_rcword(struct ssv_softc *sc, struct ssv_sta_priv_data *ssv_sta_priv, 
        struct ssv6006_tx_desc *tx_desc, struct ieee80211_tx_info *info, 
        struct ssv6006_rc_idx *drate_idx, bool is_mgmt, bool is_nullfunc, enum ieee80211_band band)
#endif
{
    struct ieee80211_tx_rate *ar = info->control.rates;
	struct ssv_minstrel_sta_priv *minstrel_sta_priv = NULL;
	u8     i, *drate_idx_tmp;
    union  { 
        struct ssv6006_rc_idx   rc_word;
        u8 val;
    } u;
  
  	if (!ssv_sta_priv) {
		ssv6006_force_lowest_rate(info, tx_desc, drate_idx);
		return;
	}

	minstrel_sta_priv = (struct ssv_minstrel_sta_priv *)ssv_sta_priv->rc_info;
	if (!minstrel_sta_priv || is_mgmt || is_nullfunc) {
		ssv6006_force_lowest_rate(info, tx_desc, drate_idx);
		return;
	}

	drate_idx_tmp = (u8*) drate_idx;
	for (i = 0; i < SSV6006RC_MAX_RATE_SERIES; i++) {
    	*drate_idx_tmp = 0;  
    	drate_idx_tmp++;
    }

    drate_idx_tmp = (u8*) drate_idx;
	for (i = 0; i < SSV6006RC_MAX_RATE_SERIES; i++) {
		u.val = 0; // init rate desc
		
		if (ar[i].idx < 0) 
		    break;
		
		if (ar[i].flags & IEEE80211_TX_RC_MCS) {
			u.rc_word.phy_mode = SSV6006RC_N_MODE;
			u.rc_word.rate_idx = (ar[i].idx & SSV6006RC_RATE_MSK);
			u.rc_word.ht40 = (ar[i].flags & IEEE80211_TX_RC_40_MHZ_WIDTH) ? SSV6006RC_HT40 : SSV6006RC_HT20; 
			u.rc_word.long_short = (ar[i].flags & IEEE80211_TX_RC_SHORT_GI) ? SSV6006RC_SHORT : SSV6006RC_LONG;
			u.rc_word.mf = (ar[i].flags & IEEE80211_TX_RC_GREEN_FIELD) ? SSV6006RC_GREEN :SSV6006RC_MIX;
		} else {
            if (band == INDEX_80211_BAND_5GHZ) {
                u.rc_word.phy_mode = SSV6006RC_G_MODE;
		        u.rc_word.rate_idx = (ar[i].idx & SSV6006RC_RATE_MSK);
			} else {
                u.rc_word.phy_mode = (ar[i].idx < DOT11_G_RATE_IDX_OFFSET) ? SSV6006RC_B_MODE : SSV6006RC_G_MODE;
                if (ar[i].idx < DOT11_G_RATE_IDX_OFFSET)
			        u.rc_word.rate_idx = (ar[i].idx & 0x3); //b mode 4 rates
                else
			        u.rc_word.rate_idx = ((ar[i].idx - DOT11_G_RATE_IDX_OFFSET) & SSV6006RC_RATE_MSK);
			    
                if (ar[i].flags & IEEE80211_TX_RC_USE_SHORT_PREAMBLE)
			        u.rc_word.long_short = SSV6006RC_SHORT;
               
                /*
                 * Mac80211 layer will choose correct B mode short/long preamble.
                 * Therefore,we force to change B mode short/long preamble for debug.
                 */
                if ((u.rc_word.phy_mode == SSV6006RC_B_MODE) && (sc->sh->cfg.auto_rate_enable == false)) {
                    if (sc->sh->cfg.rc_long_short)
                        u.rc_word.long_short = SSV6006RC_SHORT;
                    else
                        u.rc_word.long_short = SSV6006RC_LONG;
                }
			}
		}
       
        ssv6006_set_tx_desc_drate(tx_desc, i, u.val); 
        ssv6006_set_tx_desc_trycnt(tx_desc, i, ar[i].count);
        *drate_idx_tmp = u.val;
        drate_idx_tmp++;
	}
}

static void ssv6006_update_txinfo (struct ssv_softc *sc, struct sk_buff *skb)
{
    struct ieee80211_hdr            *hdr;
    struct ieee80211_tx_info        *info = IEEE80211_SKB_CB(skb);
    struct ieee80211_sta            *sta = NULL;
    struct ssv_sta_info             *sta_info = NULL;
    struct ssv_sta_priv_data        *ssv_sta_priv = NULL;
    struct ssv_vif_priv_data        *vif_priv = (struct ssv_vif_priv_data *)info->control.vif->drv_priv;
    struct ssv6006_tx_desc          *tx_desc = (struct ssv6006_tx_desc *)skb->data;
    struct ieee80211_tx_rate        *tx_drate;
    int                              ac, hw_txqid;
    u32                              nav[SSV6006RC_MAX_RATE_SERIES] = {0, 0, 0, 0};
    union {
        struct ssv6006_rc_idx        rate_idx[SSV6006RC_MAX_RATE_SERIES];
        u8                           val[SSV6006RC_MAX_RATE_SERIES];
    } dr, cr, dr_to_cr; 
    struct ampdu_hdr_st *ampdu_hdr = (struct ampdu_hdr_st *)skb->head;
    bool ampdu_retry_frame = false; 

    if (info->flags & IEEE80211_TX_CTL_AMPDU){

        sta = ampdu_hdr->ampdu_tid->sta;
        hdr = (struct ieee80211_hdr *)(skb->data + TXPB_OFFSET + AMPDU_DELIMITER_LEN);
        ampdu_retry_frame = ieee80211_has_retry(hdr->frame_control);
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
    

    if (ampdu_retry_frame) {
        int i;

        //force drop rate for retry frame
        for (i = 0 ; i < SSV6006RC_MAX_RATE_SERIES ; i++){
            if (info->control.rates[i].idx > 0)
                info->control.rates[i].idx --;
            else
                break;
        }
        // force ampdu retry packet to hwq4        
        hw_txqid = 4;
        ac = 4;
    }
    
    /* init rc_idx word */
    memset(&dr, 0, sizeof(struct ssv6006_rc_idx) * SSV6006RC_MAX_RATE_SERIES);
    memset(&cr, 0, sizeof(struct ssv6006_rc_idx) * SSV6006RC_MAX_RATE_SERIES);
    
	/* rate setting */
    tx_drate = &info->control.rates[0];
	tx_desc->is_rate_stat_sample_pkt = (info->flags & IEEE80211_TX_CTL_RATE_CTRL_PROBE) ? 1 : 0;
	ssv6006_set_data_rcword(sc, ssv_sta_priv, tx_desc, info,
		&dr.rate_idx[0], ieee80211_is_mgmt(hdr->frame_control), ieee80211_is_nullfunc(hdr->frame_control), info->band);

    // mark last rate
    if (tx_desc->try_cnt1 == 0)
        tx_desc->is_last_rate0 = 1;   //set series0 as last rate.
    else if (tx_desc->try_cnt2 == 0)
        tx_desc->is_last_rate1 = 1;   //set series1 as last rate.
    else if (tx_desc->try_cnt3 == 0)
        tx_desc->is_last_rate2 = 1;   //set series2 as last rate.
    else
        tx_desc->is_last_rate3 = 1;   //set series3 as last rate.
  
    /**
     * Generate tx info (tx descriptor) in M2 format for outgoing frames.
     * The software MAC of ssv6200 uses M2 format.
     */
    tx_desc->len = skb->len;
    //tx_desc->len = skb->len - sc->sh->tx_desc_len; // Exclude TX descriptor length
    tx_desc->c_type = M2_TXREQ;
    tx_desc->f80211 = 1;
    tx_desc->qos = (ieee80211_is_data_qos(hdr->frame_control))? 1: 0;
    
    if (sc->sh->cfg.auto_rate_enable){
        if (tx_drate->flags & IEEE80211_TX_RC_MCS) {
           tx_desc->ht = 1;
        }
    } else {
       if (sc->sh->cfg.rc_phy_mode == SSV6006RC_N_MODE){
           tx_desc->ht = 1;
       } 
    }
    tx_desc->use_4addr = (ieee80211_has_a4(hdr->frame_control))? 1: 0;
    
    tx_desc->more_data = (ieee80211_has_morefrags(hdr->frame_control))? 1: 0;
    tx_desc->stype_b5b4 = (cpu_to_le16(hdr->frame_control)>>4)&0x3;

    tx_desc->frag = (tx_desc->more_data||(hdr->seq_ctrl&0xf))? 1: 0;    
    tx_desc->unicast = (is_multicast_ether_addr(hdr->addr1)) ? 0: 1;
        
    /* ToDo Liam tx_burst is obsolete. Should re-consider about this ???*/
    //tx_desc->tx_burst = (tx_desc->frag)? 1: 0;
    
    tx_desc->bssidx = vif_priv->vif_idx;

    tx_desc->wsid = (!sta_info || (sta_info->hw_wsid < 0)) ? 0x0F : sta_info->hw_wsid;
    tx_desc->txq_idx = hw_txqid;
    tx_desc->hdr_offset = TXPB_OFFSET; //SSV6XXX_TX_DESC_LEN
    tx_desc->hdr_len = ssv6xxx_frame_hdrlen(hdr, tx_desc->ht);
    // the hdr_len of ampdu frame contains 4 bytes MPDU delimiter length
    if (info->flags & IEEE80211_TX_CTL_AMPDU)
        tx_desc->hdr_len += AMPDU_DELIMITER_LEN;
    
    /* ToDo Liam payload_offset is obsolet, should re-consider about this??*/
    //tx_desc->payload_offset = tx_desc->hdr_offset + tx_desc->hdr_len;
	/* Set Rate Report action */
	tx_desc->rate_rpt_mode = ((tx_desc->unicast && ieee80211_is_data(hdr->frame_control))
	                          && (!ieee80211_is_nullfunc(hdr->frame_control))) ? 1 : 2;

    /* RTS-to-CTS */
    if ((tx_desc->aggr != 0) && (sc->sh->cfg.rts_thres_len != 0)) {
        if ((skb->len - sc->sh->tx_desc_len) > sc->sh->cfg.rts_thres_len) {
            tx_desc->do_rts_cts0 = IEEE80211_TX_RC_USE_RTS_CTS;
            tx_desc->do_rts_cts1 = IEEE80211_TX_RC_USE_RTS_CTS;
        }
    }
    // always turn on RTS for series 2,3 if no settings
    tx_desc->do_rts_cts2 = IEEE80211_TX_RC_USE_RTS_CTS;
    tx_desc->do_rts_cts3 = IEEE80211_TX_RC_USE_RTS_CTS;
	
    /* Check for RTS protection */
	if (sc->hw->wiphy->rts_threshold != (u32) -1) {
		if ((skb->len - sc->sh->tx_desc_len) > sc->hw->wiphy->rts_threshold) {
        	tx_desc->do_rts_cts0 = IEEE80211_TX_RC_USE_RTS_CTS;
        	tx_desc->do_rts_cts1 = IEEE80211_TX_RC_USE_RTS_CTS;
        	tx_desc->do_rts_cts2 = IEEE80211_TX_RC_USE_RTS_CTS;
        	tx_desc->do_rts_cts3 = IEEE80211_TX_RC_USE_RTS_CTS;
		}
	}

    dr_to_cr.val[0] = ssv6006_drword_to_crword(tx_desc, 0, info->band);
	dr_to_cr.val[1] = ssv6006_drword_to_crword(tx_desc, 1, info->band);
	dr_to_cr.val[2] = ssv6006_drword_to_crword(tx_desc, 2, info->band);
	dr_to_cr.val[3] = ssv6006_drword_to_crword(tx_desc, 3, info->band);

	/* CTS-to-self */
	if (sc->sc_flags & SC_OP_CTS_PROT) {
		tx_desc->do_rts_cts0 = IEEE80211_TX_RC_USE_CTS_PROTECT;
		tx_desc->do_rts_cts1 = IEEE80211_TX_RC_USE_CTS_PROTECT;
		tx_desc->do_rts_cts2 = IEEE80211_TX_RC_USE_CTS_PROTECT;
		tx_desc->do_rts_cts3 = IEEE80211_TX_RC_USE_CTS_PROTECT;
	}

    //if(tx_desc->do_rts_cts0 == IEEE80211_TX_RC_USE_CTS_PROTECT) {
    if (0){ /* for DIR 615 compatibility issue DIR615 can accept B mode data only 
               after receive B mode cts-to-self*/
    	/**
        * Note: if cts-to-self is used, always use B mode rate. Here
        * we use 1Mbps as control rate.
        *
		* All protection frames are transmited at 2Mb/s for 802,11g
		* otherwise we transmit them at 1Mb/s.
		*/
		tx_desc->crate_idx0 = 0;//1Mbs
		tx_desc->crate_idx1 = 0;//1Mbs
		tx_desc->crate_idx2 = 0;//1Mbs
		tx_desc->crate_idx3 = 0;//1Mbs
    } else {
        cr.val[0] = dr_to_cr.val[0];
	    cr.val[1] = dr_to_cr.val[1];
		cr.val[2] = dr_to_cr.val[2];
		cr.val[3] = dr_to_cr.val[3];
    }

    if ((tx_desc->unicast == 0) || (ieee80211_is_ctl(hdr->frame_control))){
                      
        tx_desc->ack_policy0 = 1; /* no ack */
        
    } else if (tx_desc->qos == 1) {
        tx_desc->ack_policy0 = (*ieee80211_get_qos_ctl(hdr)&0x60)>>5;
    }

    tx_desc->security = 0;

    tx_desc->fCmdIdx = 0;
    // Packet command flow
    //  - TX queue is always the last one.
    tx_desc->fCmd = (hw_txqid+M_ENG_TX_EDCA0);

    if ((info->flags & IEEE80211_TX_CTL_AMPDU) && (tx_desc->aggr == 2)) {
        // Ack policy for MAC TX is ack. 
        tx_desc->ack_policy0 = 0;
    }

    tx_desc->ack_policy1 = tx_desc->ack_policy0;    /*follow series0 setting*/    
    tx_desc->ack_policy2 = tx_desc->ack_policy0;    /*follow series0 setting*/    
    tx_desc->ack_policy3 = tx_desc->ack_policy0;    /*follow series0 setting*/
    
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
                tx_desc->fCmd = (tx_desc->fCmd << 4) | M_ENG_CPU;
                #endif

            }
            tx_desc->fCmd = (tx_desc->fCmd << 4) | M_ENG_ENCRYPT;

        }
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


    /* Calculate all time duration */

    ssv6006_set_frame_duration(sc ,info, &dr.rate_idx[0], &dr_to_cr.rate_idx[0], (skb->len+FCS_LEN), tx_desc, &nav[0]);
 
    /**
        * Assign NAV for outgoing frame. Note that we calculate NAV by driver
        * for HT-GF/HT-MF. The b/g mode NAV is calculated by mac80211
        * stack.
        */

	//mac80211 duration calculation is error. need to calculate by driver.

    if (tx_desc->ack_policy0 != 0x01)
        hdr->duration_id = nav[0]; 
    if (tx_desc->ack_policy1 != 0x01)
        tx_desc->rateidx1_data_duration = nav[1];
    if (tx_desc->ack_policy2 != 0x01)
        tx_desc->rateidx2_data_duration = nav[2];  
    if (tx_desc->ack_policy3 != 0x01)
        tx_desc->rateidx3_data_duration = nav[3]; 
} // end of - ssv6006_update_txinfo -

static void ssv6006_dump_ssv6006_txdesc(struct sk_buff *skb)
{   // dump ssv6006_tx_desc structure.
    struct ssv6006_tx_desc *tx_desc;
    int s;
    u8 *dat;
    
    tx_desc = (struct ssv6006_tx_desc *)skb->data;

    printk("\n>> TX desc:\n");
    for(s = 0, dat= (u8 *)skb->data; s < sizeof(struct ssv6006_tx_desc) /4; s++) {
        printk("%02x%02x%02x%02x ", dat[4*s+3], dat[4*s+2], dat[4*s+1], dat[4*s]);
        if (((s+1)& 0x03) == 0)
            printk("\n");
    }    
}    

static void ssv6006_dump_ssv6006_txframe(struct sk_buff *skb)
{
    struct ssv6006_tx_desc *tx_desc;
    int s;
    u8 *dat;
    
    tx_desc = (struct ssv6006_tx_desc *)skb->data;
    printk(">> Tx Frame:\n");
    for(s = 0, dat = skb->data; s < (tx_desc->len - TXPB_OFFSET); s++) {
        printk("%02x ", dat[TXPB_OFFSET+s]);
        if (((s+1)& 0x0F) == 0)
            printk("\n");
    }    

}

static void ssv6006_dump_tx_desc(struct sk_buff *skb)
{
    struct ssv6006_tx_desc *tx_desc;
    
    tx_desc = (struct ssv6006_tx_desc *)skb->data;

    ssv6006_dump_ssv6006_txdesc(skb);

    printk("\nlength: %d, c_type=%d, f80211=%d, qos=%d, ht=%d, use_4addr=%d, sec=%d\n", 
        tx_desc->len, tx_desc->c_type, tx_desc->f80211, tx_desc->qos, tx_desc->ht,
        tx_desc->use_4addr, tx_desc->security);
    printk("more_data=%d, sub_type=%x, extra_info=%d, aggr = %d\n", tx_desc->more_data,
        tx_desc->stype_b5b4, tx_desc->extra_info, tx_desc->aggr);
    printk("fcmd=0x%08x, hdr_offset=%d, frag=%d, unicast=%d, hdr_len=%d\n",
        tx_desc->fCmd, tx_desc->hdr_offset, tx_desc->frag, tx_desc->unicast,
        tx_desc->hdr_len);
    printk("ack_policy0=%d, do_rts_cts0=%d, ack_policy1=%d, do_rts_cts1=%d, reason=%d\n", 
        tx_desc->ack_policy0, tx_desc->do_rts_cts0,tx_desc->ack_policy1, tx_desc->do_rts_cts1, 
        tx_desc->reason);
    printk("ack_policy2=%d, do_rts_cts2=%d,ack_policy3=%d, do_rts_cts3=%d\n", 
         tx_desc->ack_policy2, tx_desc->do_rts_cts2,tx_desc->ack_policy3, tx_desc->do_rts_cts3);
    printk("fcmdidx=%d, wsid=%d, txq_idx=%d\n",
         tx_desc->fCmdIdx, tx_desc->wsid, tx_desc->txq_idx);
    printk("0:RTS/CTS Nav=%d, crate_idx=%d, drate_idx=%d, dl_len=%d, retry=%d, last_rate %d \n",
        tx_desc->rts_cts_nav0, tx_desc->crate_idx0, tx_desc->drate_idx0,
        tx_desc->dl_length0, tx_desc->try_cnt0, tx_desc->is_last_rate0);
    printk("1:RTS/CTS Nav=%d, crate_idx=%d, drate_idx=%d, dl_len=%d, retry=%d, last_rate %d  \n",
        tx_desc->rts_cts_nav1, tx_desc->crate_idx1, tx_desc->drate_idx1,
        tx_desc->dl_length1, tx_desc->try_cnt1, tx_desc->is_last_rate1);
    printk("2:RTS/CTS Nav=%d, crate_idx=%d, drate_idx=%d, dl_len=%d, retry=%d, last_rate %d  \n",
        tx_desc->rts_cts_nav2, tx_desc->crate_idx2, tx_desc->drate_idx2,
        tx_desc->dl_length2, tx_desc->try_cnt2, tx_desc->is_last_rate2);
    printk("3:RTS/CTS Nav=%d, crate_idx=%d, drate_idx=%d, dl_len=%d, retry=%d , last_rate %d \n",
        tx_desc->rts_cts_nav3, tx_desc->crate_idx3, tx_desc->drate_idx3,
        tx_desc->dl_length3, tx_desc->try_cnt3, tx_desc->is_last_rate3);

}

static void ssv6006_add_txinfo (struct ssv_softc *sc, struct sk_buff *skb)
{
    struct ssv6006_tx_desc          *tx_desc;

    /* Request more spaces in front of the payload for ssv6006 tx info: */
    skb_push(skb, TXPB_OFFSET);
    tx_desc = (struct ssv6006_tx_desc *)skb->data;
    memset((void *)tx_desc, 0, TXPB_OFFSET);
    ssv6006_update_txinfo(sc, skb);
    if (sc->log_ctrl & LOG_TX_FRAME){
        ssv6006_dump_ssv6006_txframe(skb);
    }

    if (sc->log_ctrl & LOG_TX_DESC){
        printk(" dump tx desciptor after tx add info:\n");
        ssv6006_dump_tx_desc(skb);
    }
} // end of - ssv6006_add_txinfo -

static void ssv6006_update_ampdu_txinfo(struct ssv_softc *sc, struct sk_buff *ampdu_skb)
{
    struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *)ampdu_skb->data;

    tx_desc->tx_pkt_run_no = sc->tx_pkt_run_no;

}

static void ssv6006_add_ampdu_txinfo(struct ssv_softc *sc, struct sk_buff *ampdu_skb)
{
    struct ssv6006_tx_desc *tx_desc;

    /* Request more spaces in front of the payload for ssv6006 tx info: */
    skb_push(ampdu_skb, TXPB_OFFSET);
    tx_desc = (struct ssv6006_tx_desc *)ampdu_skb->data;
    memset((void *)tx_desc, 0, TXPB_OFFSET);
    tx_desc->aggr = 2;      // enable ampdu tx 1.2
    ssv6006_update_txinfo(sc, ampdu_skb);
    
    if (sc->log_ctrl & LOG_TX_FRAME){
        ssv6006_dump_ssv6006_txframe(ampdu_skb);
    }
    
    if (sc->log_ctrl & LOG_TX_DESC){
        printk(" dump tx desciptor after tx ampdu add info:\n");
        ssv6006_dump_tx_desc(ampdu_skb);
    }
}

void ssv6006_fill_lpbk_tx_desc(struct sk_buff *skb, int security, unsigned char rate)
{
    struct ssv6006_tx_desc *tx_desc;
    u32 frame_time = 0, l_length = 0;
    u8 phy, mcsidx, ht40, sgi, gf;

    //update l_length
    phy = ((rate & SSV6006RC_PHY_MODE_MSK) >> SSV6006RC_PHY_MODE_SFT);
    ht40 = ((rate & SSV6006RC_20_40_MSK) >> SSV6006RC_20_40_SFT);
    sgi = ((rate & SSV6006RC_LONG_SHORT_MSK) >> SSV6006RC_LONG_SHORT_SFT);
    gf = ((rate & SSV6006RC_MF_MSK) >> SSV6006RC_MF_SFT);
    mcsidx = ((rate & SSV6006RC_RATE_MSK) >> SSV6006RC_RATE_SFT);
    if (phy == SSV6006RC_N_MODE) {
        frame_time = ssv6xxx_ht_txtime(mcsidx, skb->len - sizeof(struct ssv6006_tx_desc), ht40, sgi, gf);
        l_length = frame_time - HT_SIFS_TIME;
        l_length = ((l_length - (HT_SIGNAL_EXT + 20)) + 3 ) >> 2;
        l_length += ((l_length << 1) - 3);
    }

    tx_desc = (struct ssv6006_tx_desc *)skb->data;
	memset((void *)tx_desc, 0x0, sizeof(struct ssv6006_tx_desc));
    tx_desc->len    = skb->len;
	tx_desc->c_type = M2_TXREQ;
    tx_desc->f80211 = 1;
    tx_desc->qos = 1;
    tx_desc->ht = 1;
    tx_desc->security = (security == SSV6006_CMD_LPBK_SEC_OPEN) ? 0 : 1;
	tx_desc->fCmd = (M_ENG_TX_EDCA0 << 12)|(M_ENG_ENCRYPT << 8)|(M_ENG_MIC<<4)|M_ENG_HWHCI;
	tx_desc->hdr_offset = 80;
    tx_desc->unicast = 1;
    tx_desc->hdr_len = sizeof(struct ieee80211_hdr_3addr);
    tx_desc->tx_pkt_run_no = 88;
    tx_desc->wsid = security; 
    tx_desc->dl_length0 = l_length;
    tx_desc->drate_idx0 = rate;
    tx_desc->try_cnt0 = 1;
    tx_desc->is_last_rate0 = 1;
    tx_desc->ack_policy0 = 1;
    tx_desc->reason = ID_TRAP_SW_TXTPUT;    //host will drop the packet for the reason
    tx_desc->rate_rpt_mode = 2;             //no rate_report
}

static int ssv6006_update_null_func_txinfo(struct ssv_softc *sc, struct ieee80211_sta *sta, struct sk_buff *skb)
{

    struct ieee80211_hdr            *hdr = (struct ieee80211_hdr *)(skb->data + TXPB_OFFSET);
    struct ssv_sta_priv_data        *ssv_sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
    struct ssv_sta_info             *sta_info = (struct ssv_sta_info *)ssv_sta_priv->sta_info;
    struct ssv6006_tx_desc          *tx_desc = (struct ssv6006_tx_desc *)skb->data;
    int                              hw_txqid = 4;
    
    memset(tx_desc, 0x0, sizeof(struct ssv6006_tx_desc));
    tx_desc->len = skb->len;
    tx_desc->c_type = M2_TXREQ;
    tx_desc->f80211 = 1;
    tx_desc->unicast = 1;
    /* 
     * tx_pkt_run_no
     * 0 : mpdu packet
     * 1 ~ 127 : ampdu packet
     * 128 : nullfunc packet
     */
    tx_desc->tx_pkt_run_no = SSV6XXX_PKT_RUN_TYPE_NULLFUN;  
    tx_desc->wsid = sta_info->hw_wsid;
    tx_desc->txq_idx = hw_txqid;
    tx_desc->hdr_offset = TXPB_OFFSET; //SSV6XXX_TX_DESC_LEN
    tx_desc->hdr_len = ssv6xxx_frame_hdrlen(hdr, false);
    tx_desc->drate_idx0 = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_6M << SSV6006RC_RATE_SFT));   
    tx_desc->crate_idx0 = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_6M << SSV6006RC_RATE_SFT));   
    tx_desc->try_cnt0 = 0xf;
    tx_desc->is_last_rate0 = 1;   
	tx_desc->rate_rpt_mode = 1;
	
    tx_desc->fCmdIdx = 0;
    tx_desc->fCmd = (hw_txqid+M_ENG_TX_EDCA0);
    tx_desc->fCmd = (tx_desc->fCmd << 4) | M_ENG_HWHCI;
    
    //set duration time
    hdr->duration_id = ssv6xxx_non_ht_txtime(WLAN_RC_PHY_OFDM, 6000, ACK_LEN, false);//always G mode, 6M


    return 0;
}

static void ssv6006_dump_rx_desc(struct sk_buff *skb)
{
    struct ssv6006_rx_desc *rx_desc;
    struct ssv6006_rxphy_info *rxphy;
    int s;
    u8 *dat;

    rx_desc = (struct ssv6006_rx_desc *)skb->data;

    printk(">> RX Descriptor:\n");
    for(s = 0, dat= (u8 *)skb->data; s < sizeof(struct ssv6006_rx_desc) /4; s++) {
        printk("%02x%02x%02x%02x ", dat[4*s+3], dat[4*s+2], dat[4*s+1], dat[4*s]);
        if (((s+1)& 0x03) == 0)
            printk("\n");
    }      
    printk(">> RX Phy Info:\n");
    for(s =0, dat= (u8 *)skb->data; s < sizeof(struct ssv6006_rxphy_info) /4; s++) {
        printk("%02x%02x%02x%02x ", dat[4*s+3+ sizeof(*rx_desc)], dat[4*s+2+ sizeof(*rx_desc)], 
            dat[4*s+1+ sizeof(*rx_desc)], dat[4*s+ sizeof(*rx_desc)]);
        if (((s+1)& 0x03) == 0)
            printk("\n");
    }
    printk("\nlen=%d, c_type=%d, f80211=%d, qos=%d, ht=%d, use_4addr=%d\n",
        rx_desc->len, rx_desc->c_type, rx_desc->f80211, rx_desc->qos, rx_desc->ht, rx_desc->use_4addr);
    printk("psm=%d, stype_b5b4=%d, reason=%d, rx_result=%d, channel = %d\n", 
        rx_desc->psm, rx_desc->stype_b5b4, rx_desc->reason, rx_desc->RxResult, rx_desc->channel);
    
    rxphy = (struct ssv6006_rxphy_info *)(skb->data + sizeof(*rx_desc));
    printk("phy_rate=0x%x, aggregate=%d, l_length=%d, l_rate=%d, rssi = %d\n", 
        rxphy->phy_rate, rxphy->aggregate, rxphy->l_length, rxphy->l_rate, rxphy->rssi);
    printk("snr=%d, rx_freq_offset=%d, timestamp 0x%x\n", rxphy->snr, rxphy->rx_freq_offset, rxphy->rx_time_stamp);    
       
}

static int ssv6006_get_tx_desc_size(struct ssv_hw *sh)
{
	return  sizeof(struct ssv6006_tx_desc);
}

static int ssv6006_get_tx_desc_ctype(struct sk_buff *skb)
{
	struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *) skb->data;

    return tx_desc->c_type ;
}

static int ssv6006_get_tx_desc_reason(struct sk_buff *skb)
{
	struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *) skb->data;

    return tx_desc->reason ;
}

static int ssv6006_get_tx_desc_txq_idx(struct sk_buff *skb)
{
	struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *) skb->data;

    return tx_desc->txq_idx ;
}

static void ssv6006_tx_rate_update( struct ssv_softc *sc, struct sk_buff *skb)
{
    // not need for turismo
}

static void ssv6006_txtput_set_desc(struct ssv_hw *sh, struct sk_buff *skb )
{
    struct ssv6006_tx_desc *tx_desc;

	tx_desc = (struct ssv6006_tx_desc *)skb->data;
	memset((void *)tx_desc, 0xff, sizeof(struct ssv6006_tx_desc));
	tx_desc->len    = skb->len;
	tx_desc->c_type = M2_TXREQ;
	tx_desc->fCmd = (M_ENG_CPU << 4) | M_ENG_HWHCI;
	tx_desc->reason = ID_TRAP_SW_TXTPUT;
}

static void ssv6006_fill_beacon_tx_desc(struct ssv_softc *sc, struct sk_buff* beacon_skb)
{
	struct ssv6006_tx_desc *tx_desc;
        
	/* Insert description space */
	skb_push(beacon_skb, TXPB_OFFSET);

    tx_desc = (struct ssv6006_tx_desc *)beacon_skb->data;
	memset(tx_desc,0, TXPB_OFFSET);

    tx_desc->len            = beacon_skb->len-TXPB_OFFSET;
	tx_desc->c_type         = M2_TXREQ;
    tx_desc->f80211         = 1;
	tx_desc->ack_policy0    = 1;//no ack;
	tx_desc->ack_policy1    = 1;//no ack;
	tx_desc->ack_policy2    = 1;//no ack;
	tx_desc->ack_policy3    = 1;//no ack;	
    tx_desc->hdr_offset 	= TXPB_OFFSET;					
    tx_desc->hdr_len 		= 24;									

	tx_desc->wsid 			= 0xf;
	if ((sc->cur_channel->band == INDEX_80211_BAND_5GHZ) || (sc->ap_vif->p2p == true)){
        tx_desc->drate_idx0 = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) 
                            | (SSV6006RC_G_6M << SSV6006RC_RATE_SFT));   
        tx_desc->crate_idx0 = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) 
                            | (SSV6006RC_G_6M << SSV6006RC_RATE_SFT));     
	}else{
	    tx_desc->drate_idx0 = ((SSV6006RC_B_MODE << SSV6006RC_PHY_MODE_SFT) 
                            | (SSV6006RC_B_1M << SSV6006RC_RATE_SFT));   
        tx_desc->crate_idx0 = ((SSV6006RC_B_MODE << SSV6006RC_PHY_MODE_SFT) 
                            | (SSV6006RC_B_1M << SSV6006RC_RATE_SFT));  
    }
	tx_desc->try_cnt0 		= 1;
	tx_desc->is_last_rate0  = 1;
}    

static int ssv6006_get_tkip_mmic_err(struct sk_buff *skb)
{
	struct ssv6006_rx_desc *rx_desc = (struct ssv6006_rx_desc *) skb->data;

    return rx_desc->tkip_mmic_err ;
}

static int ssv6006_get_rx_desc_size(struct ssv_hw *sh)
{
	return sizeof(struct ssv6006_rx_desc) + sizeof (struct ssv6006_rxphy_info);
}

static int ssv6006_get_rx_desc_length(struct ssv_hw *sh)
{
	return sizeof(struct ssv6006_rx_desc);
}

static u32 ssv6006_get_rx_desc_wsid(struct sk_buff *skb)
{
    struct ssv6006_rx_desc *rx_desc = (struct ssv6006_rx_desc *)skb->data;
    
    return rx_desc->wsid;
}

static u32 ssv6006_get_rx_desc_rate_idx(struct sk_buff *skb)
{
    struct ssv6006_rxphy_info      *rxphy;
    struct ssv6006_rx_desc *rxdesc = (struct ssv6006_rx_desc *)skb->data;
    
    rxphy = (struct ssv6006_rxphy_info *)(skb->data + sizeof(*rxdesc));
    
    return rxphy->phy_rate;
}

static u32 ssv6006_get_rx_desc_mng_used(struct sk_buff *skb)
{
    struct ssv6006_rx_desc *rx_desc = (struct ssv6006_rx_desc *)skb->data;
    
    return rx_desc->mng_used;
}

static bool ssv6006_is_rx_aggr(struct sk_buff *skb)
{
    struct ssv6006_rxphy_info      *rxphy;
    
    rxphy = (struct ssv6006_rxphy_info *)(skb->data + sizeof(struct ssv6006_rx_desc));
    
    return rxphy->aggregate;
}

static void ssv6006_get_rx_desc_info(struct sk_buff *skb, u32 *packet_len, u32 *c_type,
        u32 *tx_pkt_run_no)
{
    struct ssv6006_rx_desc *rxdesc = (struct ssv6006_rx_desc *)skb->data;
      
    *packet_len = rxdesc->len;
    *c_type = rxdesc->c_type;
    *tx_pkt_run_no = rxdesc->rx_pkt_run_no;
} 

static u32 ssv6006_get_rx_desc_ctype(struct sk_buff *skb)
{
    struct ssv6006_rx_desc *rxdesc = (struct ssv6006_rx_desc *)skb->data;
       
    return rxdesc->c_type;
}

static int ssv6006_get_rx_desc_hdr_offset(struct sk_buff *skb)
{
    struct ssv6006_rx_desc *rxdesc = (struct ssv6006_rx_desc *)skb->data;
       
    return rxdesc->hdr_offset;
}

static int ssv6006_chk_lpbk_rx_rate_desc(struct ssv_hw *sh, struct sk_buff *skb)
{
    struct ssv6006_rxphy_info *rxphy = (struct ssv6006_rxphy_info *)(skb->data + sizeof(struct ssv6006_rx_desc)); 
    u8 rate = 0;
   
    // for fixed rate && phy loopback
    // it should check rx rate description
    if (sh->cfg.lpbk_mode && (sh->cfg.lpbk_type == SSV6006_CMD_LPBK_TYPE_PHY)) { 
        rate = ((sh->cfg.rc_rate_idx_set & SSV6006RC_RATE_MSK) << SSV6006RC_RATE_SFT) |
               (sh->cfg.rc_long_short << SSV6006RC_LONG_SHORT_SFT) | 
               (sh->cfg.rc_ht40 << SSV6006RC_20_40_SFT) | 
               (sh->cfg.rc_phy_mode << SSV6006RC_PHY_MODE_SFT);
        return ((rate != rxphy->phy_rate) ? -1 : 0);
    }  
    
    return 0;
}

static bool ssv6006_nullfun_frame_filter(struct sk_buff *skb)
{
    struct ssv6006_tx_desc *txdesc = (struct ssv6006_tx_desc *)skb->data;
    
    if (txdesc->tx_pkt_run_no == SSV6XXX_PKT_RUN_TYPE_NULLFUN)
        return true;

    return false;
}

static void ssv6006_phy_enable(struct ssv_hw *sh, bool val)
{

    SMAC_REG_SET_BITS(sh, ADR_WIFI_PHY_COMMON_ENABLE_REG, (val << RG_PHY_MD_EN_SFT), RG_PHY_MD_EN_MSK);
}

static void ssv6006_set_phy_mode(struct ssv_hw *sh, bool val)
{
    if (val) { // set phy mode on without enable
        SMAC_REG_WRITE(sh, ADR_WIFI_PHY_COMMON_ENABLE_REG,(RG_PHYRX_MD_EN_MSK | RG_PHYTX_MD_EN_MSK |
            RG_PHY11GN_MD_EN_MSK | RG_PHY11B_MD_EN_MSK | RG_PHYRXFIFO_MD_EN_MSK | 
            RG_PHYTXFIFO_MD_EN_MSK | RG_PHY11BGN_MD_EN_MSK));
    } else { //clear phy mode
        SMAC_REG_WRITE(sh, ADR_WIFI_PHY_COMMON_ENABLE_REG, 0x00000000);         
    } 
}

static void ssv6006_edca_enable(struct ssv_hw *sh, bool val)
{
    if (val) {
	    SET_RG_EDCCA_AVG_T(SSV6006_EDCCA_AVG_T_25US);	
	    SET_RG_EDCCA_STAT_EN(0);
	    udelay(100);
        SET_RG_EDCCA_STAT_EN(1);
    } else {
	    SET_RG_EDCCA_STAT_EN(0);
    }
}

static u32 ssv6006_edca_ewma(int old, int new)
{
	//weight old 7/8, new 1/8
    return ((new + ((old << 3) - old)) >> 3);
}

static void ssv6006_edca_stat(struct ssv_hw *sh, int *primary, int *secondary)
{
#define SSV_EDCA_SCALE  10
#define SSV_EDCA_FRAC(val, div) (((val) << SSV_EDCA_SCALE) / div)
#define SSV_EDCA_TRUNC(val) ((val) >> SSV_EDCA_SCALE)
    struct ssv_softc *sc = sh->sc;
    u32	regval;
    int stat = 0, period = 0, percentage = 0;

    //primary chan
    SMAC_REG_READ(sh, ADR_WIFI_PHY_COMMON_EDCCA_1, &regval);
    period = ((regval & RO_EDCCA_PRIMARY_PRD_MSK) >> RO_EDCCA_PRIMARY_PRD_SFT);
	stat = ((regval & RO_PRIMARY_EDCCA_MSK) >> RO_PRIMARY_EDCCA_SFT);
	if (period)
	    percentage = SSV_EDCA_FRAC(stat, period);   
    sc->primary_edca_mib = ssv6006_edca_ewma(sc->primary_edca_mib, percentage);
    *primary = SSV_EDCA_TRUNC(sc->primary_edca_mib * 100);
    //printk("primary edca=%d, stat=%d, period=%d\n", *primary, stat, period);
    
    //secondary chan
    percentage = 0;
    SMAC_REG_READ(sh, ADR_WIFI_PHY_COMMON_EDCCA_2, &regval);
    period = ((regval & RO_EDCCA_SECONDARY_PRD_MSK) >> RO_EDCCA_SECONDARY_PRD_SFT);
	stat = ((regval & RO_SECONDARY_EDCCA_MSK) >> RO_SECONDARY_EDCCA_SFT);
	if (period)
	    percentage = SSV_EDCA_FRAC(stat, period);   
    sc->secondary_edca_mib = ssv6006_edca_ewma(sc->secondary_edca_mib, percentage);
    *secondary = SSV_EDCA_TRUNC(sc->secondary_edca_mib * 100);
    //printk("secondary edca=%d, stat=%d, period=%d\n", *secondary, stat, period);

    // restart edca
    SET_RG_EDCCA_STAT_EN(0);
	MDELAY(1);
	SET_RG_EDCCA_STAT_EN(1);
}

static void ssv6006_reset_mib_phy(struct ssv_hw *sh)
{
    //Reset PHY MIB
    SET_RG_MRX_EN_CNT_RST_N(0);
    SET_RG_PACKET_STAT_EN_11B_RX(0);
    SET_RG_PACKET_STAT_EN_11GN_RX(0);
    //printk("%s: %d, %d, %d\n", __func__, GET_RG_MRX_EN_CNT_RST_N, GET_RG_PACKET_STAT_EN_11B_RX, GET_RG_PACKET_STAT_EN_11GN_RX);
    msleep(1);
    SET_RG_MRX_EN_CNT_RST_N(1);
    SET_RG_PACKET_STAT_EN_11B_RX(1);   
    SET_RG_PACKET_STAT_EN_11GN_RX(1);     
}

static void ssv6006_dump_mib_rx_phy(struct ssv_hw *sh)
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
   
    snprintf_res(cmd_data, "PHY total Rx\t:[%08x]\n", GET_RO_MRX_EN_CNT );

    snprintf_res(cmd_data, "PHY B mode:\n");
    snprintf_res(cmd_data, "%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", "SFD_CNT","CRC_CNT","PKT_ERR","CCA","PKT_CNT");
    snprintf_res(cmd_data, "[%08x]\t", GET_RO_11B_SFD_CNT);
    snprintf_res(cmd_data, "[%08x]\t", GET_RO_11B_CRC_CNT);
    snprintf_res(cmd_data, "[%08x]\t", GET_RO_11B_PACKET_ERR_CNT);
    snprintf_res(cmd_data, "[%08x]\t", GET_RO_11B_CCA_CNT);
    snprintf_res(cmd_data, "[%08x]\t\n", GET_RO_11B_PACKET_CNT);

    snprintf_res(cmd_data, "PHY G/N mode:\n");

    snprintf_res(cmd_data, "%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n","AMPDU ERR", "AMPDU PKT","PKT_ERR","CCA","PKT_CNT");
    snprintf_res(cmd_data, "[%08x]\t", GET_RO_AMPDU_PACKET_ERR_CNT);
    snprintf_res(cmd_data, "[%08x]\t", GET_RO_AMPDU_PACKET_CNT);
    snprintf_res(cmd_data, "[%08x]\t", GET_RO_11GN_PACKET_ERR_CNT);
    snprintf_res(cmd_data, "[%08x]\t", GET_RO_11GN_CCA_CNT);   
    snprintf_res(cmd_data, "[%08x]\t\n\n", GET_RO_11GN_PACKET_CNT);
  
}

void ssv6006_rc_mac80211_rate_idx(struct ssv_softc *sc, 
            int hw_rate_idx, struct ieee80211_rx_status *rxs)
{
    
    if (((hw_rate_idx & SSV6006RC_PHY_MODE_MSK)  >>
        SSV6006RC_PHY_MODE_SFT)== SSV6006RC_N_MODE){
        rxs->flag |= RX_FLAG_HT;
        
        if (((hw_rate_idx & SSV6006RC_20_40_MSK) >> 
            SSV6006RC_20_40_SFT) == SSV6006RC_HT40){
            rxs->flag |= RX_FLAG_40MHZ;
        } 
        
        if (((hw_rate_idx & SSV6006RC_LONG_SHORT_MSK) >> 
            SSV6006RC_LONG_SHORT_SFT) == SSV6006RC_SHORT){
            rxs->flag |= RX_FLAG_SHORT_GI;
        }     
    } else {
        if (((hw_rate_idx & SSV6006RC_LONG_SHORT_MSK) >> 
            SSV6006RC_LONG_SHORT_SFT) == SSV6006RC_SHORT){
            rxs->flag |= RX_FLAG_SHORTPRE;
        }
    }

    rxs->rate_idx = (hw_rate_idx & SSV6006RC_RATE_MSK) >> 
            SSV6006RC_RATE_SFT;
    /* 
     * For legacy, g mode rate index need to conside b mode rate (rate index: 0 ~ 3)
     * Therefore, g mode rate index range is from 4 to 11 for mac80211 layer in kernel.
     */
    if ((((hw_rate_idx & SSV6006RC_PHY_MODE_MSK)  >> SSV6006RC_PHY_MODE_SFT)== SSV6006RC_G_MODE) &&
        (rxs->band == INDEX_80211_BAND_2GHZ)) {
        rxs->rate_idx += DOT11_G_RATE_IDX_OFFSET;
    }
}

void _update_green_tx(struct ssv_softc *sc, u16 rssi)
{
    u8  atteneuation_pwr = 0;
    u8  gt_pwr_start; 

    if (!(sc->sh->cfg.greentx & GT_ENABLE))
        return;
    
    gt_pwr_start = sc->sh->cfg.greentx & GT_PWR_START_MASK;
    
    if ((rssi < gt_pwr_start) && (rssi >= 0)) {
        
        atteneuation_pwr = (gt_pwr_start - rssi)*2; // 0.5 db resolution.
        sc->dpd.pwr_mode = GREEN_PWR;
        if (atteneuation_pwr != sc->green_pwr){
            sc->green_pwr = atteneuation_pwr;
            HAL_UPDATE_RF_PWR(sc);
        }

    } else {
        if ( sc->dpd.pwr_mode == GREEN_PWR) {
            sc->dpd.pwr_mode  = NORMAL_PWR;
            sc->green_pwr = 0;
            HAL_UPDATE_RF_PWR(sc);
        }
    }  
}

#ifdef CONFIG_SSV_CCI_IMPROVEMENT

static struct ssv6xxx_cca_control adjust_cci[] = {
    {0 , 43, 0x01162000, 0x20000180},
    {40, 48, 0x01161000, 0x10000180},
    {45, 53, 0x01160800, 0x08000180},
    {50, 63, 0x01160400, 0x04000180},
    {60, 68, 0x01160200, 0x02000180},
    {65, 73, 0x01160100, 0x01000180},
    {70, 128, 0x00000000, 0x00000000},
};

void ssv6006_update_scan_cci_setting(struct ssv_softc *sc)
{

    if (sc->cci_set){
        SMAC_REG_WRITE(sc->sh, ADR_WIFI_11B_RX_REG_040, 0);
        SMAC_REG_WRITE(sc->sh, ADR_WIFI_11GN_RX_REG_040, 0);
        sc->cci_set = false; 
    }
    if (sc->cci_current_level == 0) {
        sc->cci_current_level = MAX_CCI_LEVEL;
        sc->cci_current_gate = (sizeof(adjust_cci)/sizeof(adjust_cci[0])) - 1;
    }

}
void ssv6006_recover_scan_cci_setting(struct ssv_softc *sc)
{
    if ((sc->cci_set == false) && (sc->cci_current_level != MAX_CCI_LEVEL)){
       SMAC_REG_WRITE(sc->sh, ADR_WIFI_11B_RX_REG_040, 
           adjust_cci[sc->cci_current_gate].adjust_cck_cca_control);
       SMAC_REG_WRITE(sc->sh, ADR_WIFI_11GN_RX_REG_040, 
           adjust_cci[sc->cci_current_gate].adjust_ofdm_cca_control);
       sc->cci_set = true;

    }
}
void ssv6006_update_data_cci_setting
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

        if ((sc->sh->cfg.cci & CCI_CTL) == 0){
            return;
        }
        if (sc->cci_current_level == 0) {
            sc->cci_current_level = MAX_CCI_LEVEL;
            sc->cci_current_gate = (sizeof(adjust_cci)/sizeof(adjust_cci[0])) - 1;
            sc->cci_set = true;

        }
        sc->cci_start = true;
#ifdef  DEBUG_MITIGATE_CCI
        if (sc->sh->cfg.cci & CCI_DBG){
            printk("jiffies=%lu, input_level=%d\n", jiffies, input_level);
        }
#endif
        if(( input_level >= adjust_cci[sc->cci_current_gate].down_level) && (input_level <= adjust_cci[sc->cci_current_gate].upper_level)) {
            sc->cci_current_level = input_level;
#ifdef  DEBUG_MITIGATE_CCI
            if (sc->sh->cfg.cci & CCI_DBG){
                printk("Keep the ADR_WIFI_11B_RX_REG_040[%x] ADR_WIFI_11GN_RX_REG_040[%x]!!\n",
                adjust_cci[sc->cci_current_gate].adjust_cck_cca_control, 
                adjust_cci[sc->cci_current_gate].adjust_ofdm_cca_control);
            }
#endif
        }
        else {
            // [current_level]30 -> [input_level]75
            if(sc->cci_current_level < input_level) {
                for (i = 0; i < sizeof(adjust_cci)/sizeof(adjust_cci[0]); i++) {
                    if (input_level <= adjust_cci[i].upper_level) {
#ifdef  DEBUG_MITIGATE_CCI
                    if (sc->sh->cfg.cci & CCI_DBG){
                        printk("gate=%d, input_level=%d, adjust_cci[%d].upper_level=%d, cck value=%08x, ofdm value= %08x\n", 
                                sc->cci_current_gate, input_level, i, adjust_cci[i].upper_level
                                , adjust_cci[i].adjust_cck_cca_control, adjust_cci[i].adjust_ofdm_cca_control);
                    }
#endif
                        sc->cci_current_level = input_level;
                        sc->cci_current_gate = i;

                        SMAC_REG_WRITE(sc->sh, ADR_WIFI_11B_RX_REG_040, adjust_cci[i].adjust_cck_cca_control);
                        SMAC_REG_WRITE(sc->sh, ADR_WIFI_11GN_RX_REG_040, adjust_cci[i].adjust_ofdm_cca_control);
#ifdef  DEBUG_MITIGATE_CCI
                        if (sc->sh->cfg.cci & CCI_DBG){
                            printk("Set to ADR_WIFI_11B_RX_REG_040[%x] ADR_WIFI_11GN_RX_REG_040[%x]!!\n",
                                adjust_cci[sc->cci_current_gate].adjust_cck_cca_control, 
                                adjust_cci[sc->cci_current_gate].adjust_ofdm_cca_control);
                        }
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
                    if (sc->sh->cfg.cci & CCI_DBG){
                        printk("gate=%d, input_level=%d, adjust_cci[%d].upper_level=%d, cck value=%08x, ofdm value= %08x\n", 
                                sc->cci_current_gate, input_level, i, adjust_cci[i].upper_level
                                , adjust_cci[i].adjust_cck_cca_control, adjust_cci[i].adjust_ofdm_cca_control);
                    }
#endif
                        sc->cci_current_level = input_level;
                        sc->cci_current_gate = i;

                        SMAC_REG_WRITE(sc->sh, ADR_WIFI_11B_RX_REG_040, adjust_cci[i].adjust_cck_cca_control);
                        SMAC_REG_WRITE(sc->sh, ADR_WIFI_11GN_RX_REG_040, adjust_cci[i].adjust_ofdm_cca_control);
#ifdef  DEBUG_MITIGATE_CCI
                        if (sc->sh->cfg.cci & CCI_DBG){
                            printk("Set to ADR_WIFI_11B_RX_REG_040[%x] ADR_WIFI_11GN_RX_REG_040[%x]!!\n",
                                adjust_cci[sc->cci_current_gate].adjust_cck_cca_control, 
                                adjust_cci[sc->cci_current_gate].adjust_ofdm_cca_control);
                        }
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

void ssv6006_update_rssi(struct ssv_softc *sc, 
            struct sk_buff *rx_skb, struct ieee80211_rx_status *rxs)
{
    struct ssv6006_rxphy_info *rxphy; 
    struct ieee80211_sta *sta = NULL;
    struct ssv_sta_priv_data *sta_priv = NULL; 
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(rx_skb->data + sc->sh->rx_desc_len);     
    
    rxphy = (struct ssv6006_rxphy_info *)(rx_skb->data + sizeof(struct ssv6006_rx_desc));
        
        //max:0 min:-127
     // By Ryan's comment, no need to check it now
//    if ((rxdesc->rate_idx < SSV62XX_G_RATE_INDEX && rxphypad->RSVD == 0) ||
//        (rxdesc->rate_idx >= SSV62XX_G_RATE_INDEX && rxphy->service == 0))
    {
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
                printk("beacon %02X:%02X:%02X:%02X:%02X:%02X rxphy->rssi=%d\n",
                   hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
                   hdr->addr2[3], hdr->addr2[4], hdr->addr2[5], rxphy->rssi);
#endif
                if(sta_priv->beacon_rssi)
                {
                    sta_priv->beacon_rssi = ((rxphy->rssi<< RSSI_DECIMAL_POINT_SHIFT)
                        + ((sta_priv->beacon_rssi<<RSSI_SMOOTHING_SHIFT) - sta_priv->beacon_rssi)) >> RSSI_SMOOTHING_SHIFT;
                    rxphy->rssi = (sta_priv->beacon_rssi >> RSSI_DECIMAL_POINT_SHIFT);
                }
                else
                    sta_priv->beacon_rssi = (rxphy->rssi<< RSSI_DECIMAL_POINT_SHIFT);
#ifdef SSV_RSSI_DEBUG
                printk("Beacon smoothing RSSI %d\n",rxphy->rssi);
#endif
#ifdef CONFIG_SSV_CCI_IMPROVEMENT
                assoc = ssvxxx_get_sta_assco_cnt(sc);
                if ((sc->ap_vif == NULL) && (assoc == 1)){
                    ssv6006_update_data_cci_setting(sc, sta_priv, rxphy->rssi);
                    _update_green_tx(sc, rxphy->rssi);
                } else {
                    ssv6006_update_scan_cci_setting(sc);
                    sc->dpd.pwr_mode  = NORMAL_PWR;
                    sc->green_pwr = 0;
                    HAL_UPDATE_RF_PWR(sc);
                }
#endif

            }

            if (sc->cci_set == false) {
                ssv6006_recover_scan_cci_setting(sc);
            }
            if ( sc->sh->cfg.beacon_rssi_minimal )
            {
                if ( rxphy->rssi > sc->sh->cfg.beacon_rssi_minimal )
                    rxphy->rssi = sc->sh->cfg.beacon_rssi_minimal;
            }
#if 0 /*For debug*/
            printk("beacon %02X:%02X:%02X:%02X:%02X:%02X rxphypad-rpci=%d RxResult=%x wsid=%x\n",
                   hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
                   hdr->addr2[3], hdr->addr2[4], hdr->addr2[5], rxdesc->rssi, rxdesc->RxResult, wsid);
#endif
        }
        rxs->signal = (-rxphy->rssi);
        
        if (ieee80211_is_data(hdr->frame_control)){
            sta = ssv6xxx_find_sta_by_rx_skb(sc, rx_skb);    
            if (sta){
                sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
            
                sta_priv->rxstats.phy_mode = ((rxphy-> phy_rate & SSV6006RC_PHY_MODE_MSK) >> SSV6006RC_PHY_MODE_SFT);
                sta_priv->rxstats.ht40 = ((rxphy-> phy_rate & SSV6006RC_20_40_MSK) >> SSV6006RC_20_40_SFT);
                if (sta_priv->rxstats.phy_mode == SSV6006RC_B_MODE) {
                    sta_priv->rxstats.cck_pkts[(rxphy-> phy_rate) & SSV6006RC_B_RATE_MSK] ++;
                } else if (sta_priv->rxstats.phy_mode == SSV6006RC_N_MODE) {
                    sta_priv->rxstats.n_pkts[(rxphy-> phy_rate) & SSV6006RC_RATE_MSK] ++;
                } else {
                    sta_priv->rxstats.g_pkts[(rxphy-> phy_rate) & SSV6006RC_RATE_MSK] ++;
                }
                //printk("phy rate %x, phy mode %d, phy_rate idx  %d\n",rxphy-> phy_rate, sta_priv->rxstats.phy_mode, (rxphy-> phy_rate) & SSV6006RC_RATE_MSK);
            }
        }
/*    } else {


       // for NL80211_IFTYPE_STATION, wsid would be 0, 1, e.
        // for NL80211_IFTYPE_AP, wisd would be 0, 1, f for no WSID WATCH LIST
        // for NL80211_IFTYPE_AP, wisd would be 0~7 for WSID WATCH list_head
        sta = ssv6xxx_find_sta_by_rx_skb(sc, rx_skb);

        // Encrypted RX packet must be for connected STA.
        if(sta)
        {
            sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
            rxs->signal = -(sta_priv->beacon_rssi >> RSSI_DECIMAL_POINT_SHIFT);
        }*/

    }
}

static void _cmd_rc_setting(struct ssv_hw *sh, int argc, char *argv[]){
    char *endp;
    int  val;                                          
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    struct rc_setting *rc_setting = &sh->cfg.rc_setting; 
 
    val = simple_strtoul(argv[4], &endp, 0);
    snprintf_res(cmd_data, "\n set rc %s to %d\n", argv[3], val); 
    
    if (!strcmp(argv[3], "force_sample_pr")){
        rc_setting->force_sample_pr = val; 
    } else if (!strcmp(argv[3], "aging_period")){
        rc_setting->aging_period = val; 
    } else if (!strcmp(argv[3], "target_success_67")){
        rc_setting->target_success_67 = val; 
    } else if (!strcmp(argv[3], "target_success_5")){
        rc_setting->target_success_5 = val; 
    } else if (!strcmp(argv[3], "target_success_4")){
        rc_setting->target_success_4 = val; 
    } else if (!strcmp(argv[3], "target_success")){
        rc_setting->target_success = val; 
    } else if (!strcmp(argv[3], "up_pr")){
        rc_setting->up_pr = val; 
    } else if (!strcmp(argv[3], "up_pr3")){
        rc_setting->up_pr3 = val; 
    } else if (!strcmp(argv[3], "up_pr4")){
        rc_setting->up_pr4 = val; 
    } else if (!strcmp(argv[3], "up_pr5")){
        rc_setting->up_pr5 = val; 
    } else if (!strcmp(argv[3], "up_pr6")){
        rc_setting->up_pr6 = val; 
    } else if (!strcmp(argv[3], "forbid")){
        rc_setting->forbid = val; 
    } else if (!strcmp(argv[3], "forbid3")){
        rc_setting->forbid3 = val; 
    } else if (!strcmp(argv[3], "forbid4")){
        rc_setting->forbid4 = val; 
    } else if (!strcmp(argv[3], "forbid5")){
        rc_setting->forbid5 = val; 
    } else if (!strcmp(argv[3], "forbid6")){
        rc_setting->forbid6 = val;
    } else if (!strcmp(argv[3], "sample_pr_4")){
        rc_setting->sample_pr_4 = val; 
    } else if (!strcmp(argv[3], "sample_pr_5")){
        rc_setting->sample_pr_5 = val;
    } else {
        snprintf_res(cmd_data, "\n rc set rc_setting [patch] [val]\n");
        return;
    }    
   
    snprintf_res(cmd_data, "current rc setting:\n");
    
    snprintf_res(cmd_data, "\t aging_period %d\t force_sample_pr %d\t sample_pr_4 %d \t sample_pr_5 %d\n",
        rc_setting->aging_period, rc_setting->force_sample_pr, 
        rc_setting->sample_pr_4, rc_setting->sample_pr_5);

    snprintf_res(cmd_data, "\t target_success_67 %d \t target_success_5 %d\t target_success_4 %d\t target_success %d\n",
        rc_setting->target_success_67, rc_setting->target_success_5,
        rc_setting->target_success_4, rc_setting->target_success);
        
    snprintf_res(cmd_data, "\t up_pr %d\t up_pr3 %d \t up_pr4 %d\t up_pr5 %d\t up_pr6 %d\n",
        rc_setting->up_pr, rc_setting->up_pr3, rc_setting->up_pr4, 
        rc_setting->up_pr5, rc_setting->up_pr6);
        
    snprintf_res(cmd_data, "\t forbid %d\t forbid3 %d \t forbid5 %d\t forbid5 %d\t forbid6 %d\n",
        rc_setting->forbid, rc_setting->forbid3, rc_setting->forbid4, 
        rc_setting->forbid5, rc_setting->forbid6);      
}

static void _sta_txstats(struct ssv_softc *sc, struct ssv_sta_priv_data *sta_priv) {
    struct ssv_minstrel_sta_priv *minstrel_sta_priv = sta_priv->rc_info;
	struct ssv_minstrel_sta_info *legacy;
	struct ssv_minstrel_ht_sta *ht;
	struct ssv_minstrel_ht_mcs_group_data *mg;
	unsigned int i, j, tp, prob, eprob;
    unsigned int max_mcs = MINSTREL_MAX_STREAMS * MINSTREL_STREAM_GROUPS;
    int bitrates[4] = { 10, 20, 55, 110 };
    int max_ht_groups = 0;
    struct ssv_cmd_data *cmd_data = &sc->cmd_data;
  
    if (!minstrel_sta_priv->is_ht) {
		legacy = (struct ssv_minstrel_sta_info *)&minstrel_sta_priv->legacy;
		snprintf_res(cmd_data, "rate     throughput  ewma prob   this prob  "
				"this succ/attempt   success    attempts\n");

		for (i = 0; i < legacy->n_rates; i++) {
			struct ssv_minstrel_rate *mr = &minstrel_sta_priv->ratelist[i];

			snprintf_res(cmd_data,"%c", (i == legacy->max_tp_rate) ? 'T' : ' ');
			snprintf_res(cmd_data,"%c", (i == legacy->max_tp_rate2) ? 't' : ' ');
			snprintf_res(cmd_data,"%c",  (i == legacy->max_prob_rate) ? 'P' : ' ');
			
			snprintf_res(cmd_data, "%3u%s", mr->bitrate / 10, (mr->bitrate & 1 ? ".5" : "  "));
			tp = mr->cur_tp/(1024*1024);
			prob = MINSTREL_TRUNC(mr->cur_prob * 1000);
			eprob = MINSTREL_TRUNC(mr->probability * 1000);

			snprintf_res(cmd_data, "  %6u.%1u   %6u.%1u   %6u.%1u        "
					"%3u(%3u)   %8llu    %8llu\n",
					tp / 10, tp % 10,
					eprob / 10, eprob % 10,
					prob / 10, prob % 10,
					mr->last_success,
					mr->last_attempts,
					(unsigned long long)mr->succ_hist,
					(unsigned long long)mr->att_hist);
		}
		snprintf_res(cmd_data, "\nTotal packet count::    total %llu      lookaround %llu\n\n",
				legacy->packet_count, legacy->sample_count);
	} else {
		ht = (struct ssv_minstrel_ht_sta *)&minstrel_sta_priv->ht;
		snprintf_res(cmd_data, "%4s %8s %12s %12s %12s %20s %10s %10s\n", 
                "type", "rate", "throughput", "ewma prob", "this prob", "this succ/attempt", "success", "attempts");
        
        max_ht_groups = MINSTREL_MAX_STREAMS * MINSTREL_STREAM_GROUPS + 1;
		for (i = 0; i < max_ht_groups; i++) {
			mg = &ht->groups[i];
			if (!mg->supported)
				continue;

            for (j = 0; j < MCS_GROUP_RATES; j++) {
				struct ssv_minstrel_ht_rate_stats *mhr = &ht->groups[i].rates[j];
                int idx = i * MCS_GROUP_RATES + j;
                
				if (!(ht->groups[i].supported & BIT(j)))
					continue;

			    snprintf_res(cmd_data,"%c", (idx == ht->max_tp_rate) ? 'T' : ' ');
			    snprintf_res(cmd_data,"%c", (idx == ht->max_tp_rate2) ? 't' : ' ');
			    snprintf_res(cmd_data,"%c",  (idx == ht->max_prob_rate) ? 'P' : ' ');
			    	
                if (i == max_mcs) {     
                    int r = bitrates[j % 4];
                    snprintf_res(cmd_data, "  %2u.%1uM/%c", r / 10, r % 10, (ht->cck_supported_short ? 'S' : 'L'));
                } else 
                    snprintf_res(cmd_data, "    MCS%-2u", j);

				tp = mhr->cur_tp / 10;
				prob = MINSTREL_TRUNC(mhr->cur_prob * 1000);
				eprob = MINSTREL_TRUNC(mhr->probability * 1000);

				snprintf_res(cmd_data, "     %6u.%1u     %6u.%1u     %6u.%1u            %3u(%3u)    %8llu   %8llu\n",
						tp / 10, tp % 10,
						eprob / 10, eprob % 10,
						prob / 10, prob % 10,
						mhr->last_success,
						mhr->last_attempts,
						(unsigned long long)mhr->succ_hist,
						(unsigned long long)mhr->att_hist);
			}
		}
		snprintf_res(cmd_data, "\nTotal packet count::    total %llu      lookaround %llu\n",
				ht->total_packets, ht->sample_packets);
	}    
}

static void _cmd_txstats(struct ssv_softc *sc){
    int j, sta_idx = 0;
    struct ssv_cmd_data *cmd_data = &sc->cmd_data;

    for (j = 0; j < SSV6200_MAX_VIF; j++) {
        struct ieee80211_vif *vif = sc->vif_info[j].vif;
        struct ssv_vif_priv_data *priv_vif;
        struct ssv_sta_priv_data *sta_priv_iter;

        if (vif == NULL){
            snprintf_res(cmd_data, "    VIF: %d is not used.\n", j);
            continue; 
        }
        snprintf_res(cmd_data,
                "Tx statistics: VIF: %d - [%02X:%02X:%02X:%02X:%02X:%02X] type[%d] p2p[%d]\n", j,
                vif->addr[0], vif->addr[1], vif->addr[2],
                vif->addr[3], vif->addr[4], vif->addr[5], vif->type, vif->p2p);

        priv_vif = (struct ssv_vif_priv_data *)(vif->drv_priv);

        list_for_each_entry(sta_priv_iter, &priv_vif->sta_list, list){
            snprintf_res(cmd_data,"    sta_idx %d \n", sta_idx);
            if ((sta_priv_iter->sta_info->s_flags & STA_FLAG_VALID) == 0) {
                snprintf_res(cmd_data, "    STA: %d  is not valid.\n", sta_idx);

                continue;
            }
            _sta_txstats(sc, sta_priv_iter);
            sta_idx++;
        }
    }  
 
}

static void _sta_rxstats(struct ssv_softc *sc, struct ssv_sta_priv_data *sta_priv) {
    struct ssv_cmd_data *cmd_data = &sc->cmd_data;
    int i;
    
    snprintf_res(cmd_data," \t last data packet mode %d, ht40 %d \n", sta_priv->rxstats.phy_mode,
        sta_priv->rxstats.ht40);
    
    snprintf_res(cmd_data,"\t N packet statistics\n");
    
    snprintf_res(cmd_data,"\t MCS0 \t MCS1 \t MCS2 \t MCS3 ");
    snprintf_res(cmd_data,"\t MCS4 \t MCS5 \t MCS6 \t MCS7\n");
    for (i = 0; i < SSV6006RC_MAX_RATE; i++) {
        snprintf_res(cmd_data,"\t %llu", sta_priv->rxstats.n_pkts[i]);
    }

    snprintf_res(cmd_data,"\n\t G packet statistics\n"); 
    snprintf_res(cmd_data,"\t 6M   \t 9M   \t 12M  \t 18M  ");
    snprintf_res(cmd_data,"\t 24M  \t 36M  \t 48M  \t 54M\n");
    for (i = 0; i < SSV6006RC_MAX_RATE; i++) {
        snprintf_res(cmd_data,"\t %llu", sta_priv->rxstats.g_pkts[i]);
    }
    
    snprintf_res(cmd_data,"\n\t cck packet statistics\n");
    
    snprintf_res(cmd_data,"\t 1M   \t 2M  \t 5.5M \t 11M\n");
    for (i = 0; i < SSV6006RC_B_MAX_RATE; i++) {
        snprintf_res(cmd_data,"\t %llu", sta_priv->rxstats.cck_pkts[i]);
    }
    snprintf_res(cmd_data,"\n");
     
}

static void _cmd_rxstats(struct ssv_softc *sc){
    int j, sta_idx = 0;
    struct ssv_cmd_data *cmd_data = &sc->cmd_data;

    for (j = 0; j < SSV6200_MAX_VIF; j++) {
        struct ieee80211_vif *vif = sc->vif_info[j].vif;
        struct ssv_vif_priv_data *priv_vif;
        struct ssv_sta_priv_data *sta_priv_iter;

        if (vif == NULL){
            snprintf_res(cmd_data, "    VIF: %d is not used.\n", j);
            continue; 
        }
        snprintf_res(cmd_data,
                "Rx statistics VIF: %d - [%02X:%02X:%02X:%02X:%02X:%02X] type[%d] p2p[%d]\n", j,
                vif->addr[0], vif->addr[1], vif->addr[2],
                vif->addr[3], vif->addr[4], vif->addr[5], vif->type, vif->p2p);

        priv_vif = (struct ssv_vif_priv_data *)(vif->drv_priv);

        list_for_each_entry(sta_priv_iter, &priv_vif->sta_list, list){
            snprintf_res(cmd_data,"    sta_idx %d \n", sta_idx);
            if ((sta_priv_iter->sta_info->s_flags & STA_FLAG_VALID) == 0) {
                snprintf_res(cmd_data, "    STA: %d  is not valid.\n", sta_idx);

                continue;
            }
            _sta_rxstats(sc, sta_priv_iter);
            sta_idx++;
        }
    }  
 
}

static void ssv6006_cmd_rc(struct ssv_hw *sh, int argc, char *argv[])
{
    char *endp;
    int  val;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data; 
        
    if (argc < 2) {
        snprintf_res(cmd_data, "\n rc show | set | txstats \n");
        return;
    }    

    if (!strcmp(argv[1], "show")){
        ;
    }else if (!strcmp(argv[1], "set")){
       
        if (argc == 4){
        
            val = simple_strtoul(argv[3], &endp, 0);
            snprintf_res(cmd_data, "\n set rc %s to %d\n", argv[2], val);    
            
            if (!strcmp(argv[2], "auto")){         
                sh->cfg.auto_rate_enable = val;
                            
            } else if (!strcmp(argv[2], "auto")){
                sh->cfg.rc_rate_idx_set = val;
            
            } else if (!strcmp(argv[2], "rate")){
                sh->cfg.rc_rate_idx_set = val;
            
            } else if (!strcmp(argv[2], "retry")){
                sh->cfg.rc_retry_set = val;
            
            } else if (!strcmp(argv[2], "green")){
                sh->cfg.rc_mf = val;
            
            } else if (!strcmp(argv[2], "short")){
                sh->cfg.rc_long_short = val;
            
            } else if (!strcmp(argv[2], "ht40")){
                sh->cfg.rc_ht40 = val;
            
            } else if (!strcmp(argv[2], "phy")){
                sh->cfg.rc_phy_mode = val;
            
            } else if (!strcmp(argv[2], "rc_log")){
                sh->cfg.rc_log = val;
            
			} else {
                snprintf_res(cmd_data, "\n rc set auto| rate | retry | green | short | ht40 | phy | rc_log [val]\n");
                return;
            }    
        } else if (argc ==5){
            if (!strcmp(argv[2], "rc_setting")){
                _cmd_rc_setting(sh, argc, argv);
            } else {
                snprintf_res(cmd_data, "\n rc set rc_setting [patch] [val]\n");
            }
            return; 
        } else {
          snprintf_res(cmd_data, "\n rc set auto| rate | retry | green | short | ht40 | phy | rc_log |rc_setting [val]\n");
          return;    
        }
    } else if (!strcmp(argv[1], "txstats")){   
        _cmd_txstats(sh->sc);
        return;  
    } else if (!strcmp(argv[1], "rxstats")){   
        _cmd_rxstats(sh->sc);
        return;  
    }else {
        snprintf_res(cmd_data, "\n rc show | set \n");
        return;
    }
    snprintf_res(cmd_data, "\n fix rate control parameters for ssv6006\n"); 
    snprintf_res(cmd_data, " auto rate enable         : %s\n", (sh->cfg.auto_rate_enable) ? "True": "False");
    snprintf_res(cmd_data, " rc_rate_idx_set          : 0x%04x\n", sh->cfg.rc_rate_idx_set);
    snprintf_res(cmd_data, " rc_retry_set             : 0x%04x\n", sh->cfg.rc_retry_set);
    snprintf_res(cmd_data, " rc mix mode              : %s\n",(sh->cfg.rc_mf) ? "Green": "Mix");
    snprintf_res(cmd_data, " rc long/short GI/Preambe : %s\n", (sh->cfg.rc_long_short ) ? "Short": "Long");
    snprintf_res(cmd_data, " rc ht40/h20              : %s\n", sh->cfg.rc_ht40 ? "HT40":"HT20");
    snprintf_res(cmd_data, " rc phy mode              :");
    if (sh->cfg.rc_phy_mode == SSV6006RC_B_MODE){
        snprintf_res(cmd_data, " B mode\n");
    }else if (sh->cfg.rc_phy_mode == SSV6006RC_G_MODE){
        snprintf_res(cmd_data, " G mode\n");
    }else {
        snprintf_res(cmd_data, " %s\n", (sh->cfg.rc_phy_mode == SSV6006RC_N_MODE) ? "N mode" : "Invalid");
    }   
    snprintf_res(cmd_data, " rc log                   : %s\n", (sh->cfg.rc_log) ? "True": "False");
    snprintf_res(cmd_data, " force ht20               : %s\n", (sh->sc->sc_flags & SC_OP_FORCE_HT20) ? "True": "False");
        
    return;
}

bool ssv6006_is_legacy_rate(struct ssv_softc *sc, struct sk_buff *skb)
{
    bool ret = true;   
   	struct ieee80211_tx_info *info;

    info = IEEE80211_SKB_CB(skb);
	if ((info->control.rates[0].flags & IEEE80211_TX_RC_MCS) &&
		(info->control.rates[1].flags & IEEE80211_TX_RC_MCS) &&
		(info->control.rates[2].flags & IEEE80211_TX_RC_MCS)) {
		ret = false;
    } 

    return ret;
} 

static void ssv6006_rc_algorithm(struct ssv_softc *sc)
{
	struct ieee80211_hw *hw=sc->hw;

	hw->rate_control_algorithm = "ssv_minstrel";
}

static void ssv6006_set_80211_hw_rate_config(struct ssv_softc *sc)
{
	struct ieee80211_hw *hw=sc->hw;

	hw->max_rates = SSV6006RC_MAX_RATE_SERIES;
	hw->max_rate_tries = SSV6006RC_MAX_RATE_RETRY;
}

static void ssv6006_rc_rx_data_handler(struct ssv_softc *sc, struct sk_buff *skb, u32 rate_index)
{
    if (sc->log_ctrl & LOG_RX_DESC){
	    struct ieee80211_sta *sta;
	    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(skb->data + sc->sh->rx_desc_len);

        if (!(ieee80211_is_beacon(hdr->frame_control))){   // log for not beacon frame
            sta = ssv6xxx_find_sta_by_rx_skb(sc, skb);
            if (sta != NULL)
                ssv6006_dump_rx_desc(skb);
        }
    }
}

static void ssv6006_rate_report_nullfunc_handler(struct ssv_softc *sc, struct sk_buff *skb)
{
    struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *)skb->data;
    struct sk_buff *beacon;
    struct ieee80211_mgmt *hdr;
    struct hci_rx_aggr_info *rx_aggr_info;
    unsigned int time_diff;
    unsigned char *pdata = NULL;
    int rx_mode = sc->sh->rx_mode; 
    int alignment_size = 0;
#if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
    struct sk_buff_head rx_list;
#endif

    if (sc->beacon_container == NULL)
        return;

    /* just use first rate series */
    if (tx_desc->rpt_result0 != 0) {
        beacon = ssv_skb_alloc(sc, sc->beacon_container->len + sizeof(struct hci_rx_aggr_info) + 4); //4byte for alignment
        if (!skb)
            return;

        if ((rx_mode == RX_HW_AGG_MODE) || (rx_mode == RX_HW_AGG_MODE_METH3)) {
            pdata = skb_put(beacon, sizeof(struct hci_rx_aggr_info));
            rx_aggr_info = (struct hci_rx_aggr_info *)pdata;
            memset(rx_aggr_info, 0, sizeof(struct hci_rx_aggr_info));
            rx_aggr_info->jmp_mpdu_len = ((sc->beacon_container->len + 3) / 4) * 4  + sizeof(struct hci_rx_aggr_info);
            rx_aggr_info->accu_rx_len = ((sc->beacon_container->len + 3) / 4) * 4  + sizeof(struct hci_rx_aggr_info);
        }

        pdata = skb_put(beacon, sc->beacon_container->len);
        memcpy(pdata, sc->beacon_container->data, sc->beacon_container->len);
        /* update beacon timestamp */
        hdr = (struct ieee80211_mgmt *)(pdata + sc->sh->rx_desc_len);
        time_diff = jiffies_to_msecs(abs(jiffies - sc->beacon_container_update_time));
        hdr->u.beacon.timestamp += time_diff;
        
        if ((rx_mode == RX_HW_AGG_MODE) || (rx_mode == RX_HW_AGG_MODE_METH3)) {
            alignment_size = rx_aggr_info->jmp_mpdu_len - beacon->len;
            if (alignment_size != 0)
                skb_put(beacon, alignment_size);
        }
    
    #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
        skb_queue_head_init(&rx_list);
        __skb_queue_tail(&rx_list, beacon);
        ssv6200_rx(&rx_list, sc);
    #else
        ssv6200_rx(beacon, sc);
    #endif
    } 
}

static bool ssv6006_rate_report_filter(struct ssv_softc *sc, struct sk_buff *skb)
{
    struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *)skb->data;

    if (tx_desc->tx_pkt_run_no == SSV6XXX_PKT_RUN_TYPE_NULLFUN) { 
        ssv6006_rate_report_nullfunc_handler(sc, skb);
        return true;
    }
    
    return false;
}

static void ssv6006_rate_report_handler(struct ssv_softc *sc, struct sk_buff *skb)
{
    if (sc->log_ctrl & LOG_RATE_REPORT){
        printk(" Dump tx desc from rate report:\n");    
        ssv6006_dump_ssv6006_txdesc(skb);
    }

    if (ssv6006_rate_report_filter(sc, skb)) {
        dev_kfree_skb_any(skb);
        return;
    } 

	skb_queue_tail(&sc->rc_report_queue, skb);
	if (sc->rc_report_sechedule == 0)
		queue_work(sc->rc_report_workqueue, &sc->rc_report_work);
}

static int ssv6006_get_tx_desc_rate_rpt(struct ssv_softc *sc, struct ssv6006_tx_desc *tx_desc, 
		int idx, u8 *drate, u8 *success, u8 *trycnt)
{
	switch (idx) {
		case 0:
			*drate = tx_desc->drate_idx0;
			*success = tx_desc->rpt_result0;
			*trycnt = tx_desc->rpt_trycnt0;
			return tx_desc->is_last_rate0;
		case 1:
			*drate = tx_desc->drate_idx1;
			*success = tx_desc->rpt_result1;
			*trycnt = tx_desc->rpt_trycnt1;
			return tx_desc->is_last_rate1;
		case 2:
			*drate = tx_desc->drate_idx2;
			*success = tx_desc->rpt_result2;
			*trycnt = tx_desc->rpt_trycnt2;
			return tx_desc->is_last_rate2;
		case 3:
			*drate = tx_desc->drate_idx3;
			*success = tx_desc->rpt_result3;
			*trycnt = tx_desc->rpt_trycnt3;
			return tx_desc->is_last_rate3;
		default:
			*drate = -1;
			*success = 0;
			*trycnt = 0;
			return 0;
	}
}

static void _update_ht_rpt(struct ssv_softc *sc, struct ssv6006_tx_desc *tx_desc, 
    struct ssv_minstrel_ht_rpt *ht_rpt)
{
    u8 drate, success, rpt_trycnt, last;
    int i;
    
    for (i = 0; i < SSV6006RC_MAX_RATE_SERIES; i++) {
		last = ssv6006_get_tx_desc_rate_rpt(sc, tx_desc, i, &drate, &success, &rpt_trycnt);
		ht_rpt[i].dword = drate;
		ht_rpt[i].count = rpt_trycnt;
		ht_rpt[i].success = success;
		ht_rpt[i].last = last;
		
		if ((drate < 0) || success)
			ht_rpt[i].last = 1;

		if (ht_rpt[i].last)
			break;
	}
    
}
static void ssv6006_rc_no_ba_handler(struct ssv_softc *sc, struct sk_buff *report)
{
    struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *)report->data;
	struct ssv_sta_info *ssv_sta;
    struct ssv_sta_priv_data *ssv_sta_priv;
	struct AMPDU_TID_st *ampdu_tid;
	int i, cur_ampdu_idx;
	struct ssv_minstrel_ht_rpt ht_rpt[SSV6006RC_MAX_RATE_SERIES];
	int mpdu_num = 0;
	u8 tid_no = 0;

	if (tx_desc->wsid >= SSV_NUM_STA) {
		dev_warn(sc->dev, "%s(): wsid[%d] is invaild!!\n", __FUNCTION__, tx_desc->wsid);
		return;
	}
	
	ssv_sta = &sc->sta_info[tx_desc->wsid];
	if (!ssv_sta->sta) {
		dev_warn(sc->dev, "%s(): Cannot find the station\n", __FUNCTION__);
		return;
	}

	ssv_sta_priv = (struct ssv_sta_priv_data *)ssv_sta->sta->drv_priv;
    for (i = 0; i < MAX_CONCUR_AMPDU; i ++){
        if (ssv_sta_priv->ampdu_ssn[i].tx_pkt_run_no == tx_desc->tx_pkt_run_no){
    		cur_ampdu_idx = i;
    		mpdu_num = ssv_sta_priv->ampdu_ssn[i].mpdu_num;
            break;
        }     
    }

	if (i == MAX_CONCUR_AMPDU) {
		dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN,"%s:tx_pkt_run_no %d not found on sent list.\n", 
				__FUNCTION__, tx_desc->tx_pkt_run_no );
		return;
	}

	
	tid_no = ssv_sta_priv->ampdu_ssn[cur_ampdu_idx].tid_no;
	ampdu_tid = &(ssv_sta_priv->ampdu_tid[tid_no]);
	for (i = 0; i < ssv_sta_priv->ampdu_ssn[cur_ampdu_idx].mpdu_num; i++) {
		u16 ssn = ssv_sta_priv->ampdu_ssn[cur_ampdu_idx].ssn[i];
		struct sk_buff *skb;
		u32 skb_ssn;
		struct SKB_info_st *skb_info;

		skb = INDEX_PKT_BY_SSN(ampdu_tid, ssn);
		skb_ssn = (skb == NULL) ? (-1) : ampdu_skb_ssn(skb);
		if (skb_ssn != ssn) {
			dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN, "%s(): Unmatched SSN packet: %d - %d\n",
					__FUNCTION__, ssn, skb_ssn);
			continue;
		}

		skb_info = (struct SKB_info_st *) (skb->head);
		if (skb_info->ampdu_tx_status == AMPDU_ST_SENT) {
			if (skb_info->mpdu_retry_counter < SSV_AMPDU_retry_counter_max) {
				if (skb_info->mpdu_retry_counter == 0) {
					struct ieee80211_hdr *skb_hdr = ampdu_skb_hdr(skb);
					skb_hdr->frame_control |= cpu_to_le16(IEEE80211_FCTL_RETRY);
				}
				
				skb_info->ampdu_tx_status = AMPDU_ST_RETRY;
				skb_info->mpdu_retry_counter++;
			} else {
				skb_info->ampdu_tx_status = AMPDU_ST_DROPPED;
				dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN, "%s(), drop skb ssn[%d]\n", __FUNCTION__, skb_ssn);
			}
		} else {
			dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN, "%s() skb ssn[%d] status[%d]\n", 
				__FUNCTION__, skb_ssn, skb_info->ampdu_tx_status);
		}
		
	}
	ssv6xxx_release_frames(ampdu_tid);

	memset((void*)&ssv_sta_priv->ampdu_ssn[cur_ampdu_idx], 0, sizeof(struct aggr_ssn));
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_AMPDU_SSN, "%s(): ampdu slot=%d, tx_pkt_run_no=%d\n", 
			__FUNCTION__, ((cur_ampdu_idx <MAX_CONCUR_AMPDU) ? cur_ampdu_idx : (-1)), tx_desc->tx_pkt_run_no);
	
	//update statics
    _update_ht_rpt(sc, tx_desc, ht_rpt);	
    
	if (sc->sh->cfg.auto_rate_enable == true) {
		ssv_minstrel_ht_tx_status(sc, ssv_sta_priv->rc_info, ht_rpt, SSV6006RC_MAX_RATE_SERIES, 
			mpdu_num, 0, tx_desc->is_rate_stat_sample_pkt);
	}	
}

bool ssv6006_rc_get_previous_ampdu_rpt(struct ssv_minstrel_ht_sta *mhs, int pkt_no, int *rpt_idx)
{
	bool ret = false;
	int i;

	for (i = 0; i < SSV_MINSTREL_AMPDU_RATE_RPTS; i++) {
		if ((mhs->ampdu_rpt_list[i].used) && (pkt_no == mhs->ampdu_rpt_list[i].pkt_no)) {
			*rpt_idx = i;
			ret = true;
			break;
		}
	}

	return ret;
}

void ssv6006_rc_add_ampdu_rpt_to_list(struct ssv_softc *sc, struct ssv_minstrel_ht_sta *mhs, void *rate_rpt, 
			int pkt_no, int ampdu_len, int ampdu_ack_len)
{
	int i = 0, idx = -1;
    struct ssv6006_tx_desc *tx_desc;
	struct ssv_minstrel_ampdu_rate_rpt *ampdu_rpt;
	u8 drate, success, rpt_trycnt;
	int last = 0;

	for (i = 0; i < SSV_MINSTREL_AMPDU_RATE_RPTS; i++) {
		if (mhs->ampdu_rpt_list[i].used == false) {
			idx = i;
			break;
		}
	}

	if (idx == -1) {
		printk("AMPDU rpt list is full\n");
		WARN_ON(1);
		return;
	}

	ampdu_rpt = (struct ssv_minstrel_ampdu_rate_rpt *)&mhs->ampdu_rpt_list[idx];
	if (!rate_rpt) {
		ampdu_rpt->pkt_no = pkt_no;
		ampdu_rpt->ampdu_len = ampdu_len;
		ampdu_rpt->ampdu_ack_len = ampdu_ack_len;
		ampdu_rpt->used = true;
	} else {
		tx_desc = (struct ssv6006_tx_desc *)rate_rpt;
		ampdu_rpt->pkt_no = tx_desc->tx_pkt_run_no;
		ampdu_rpt->is_sample = tx_desc->is_rate_stat_sample_pkt;
		for (i = 0; i < SSV6006RC_MAX_RATE_SERIES; i++) {
			last = ssv6006_get_tx_desc_rate_rpt(sc, tx_desc, i, &drate, &success, &rpt_trycnt);
			ampdu_rpt->rate_rpt[i].dword = drate;
			ampdu_rpt->rate_rpt[i].count = rpt_trycnt;
			ampdu_rpt->rate_rpt[i].success = success;
			ampdu_rpt->rate_rpt[i].last = last;
		
			if ((drate < 0) || success)
				ampdu_rpt->rate_rpt[i].last = 1;

			if (ampdu_rpt->rate_rpt[i].last)
				break;
		}	
		ampdu_rpt->used = true;
	}
}

static void ssv6006_rc_ba_handler(struct ssv_softc *sc, void *rate_rpt, u32 wsid)
{
    struct ssv6006_tx_desc *tx_desc;
	struct ssv_sta_priv_data *ssv_sta_priv;
	struct ssv_sta_info *ssv_sta;
	struct ssv_minstrel_sta_priv *minstrel_sta_priv;
	struct ssv_minstrel_ht_sta *mhs;
	struct ssv_minstrel_ht_rpt ht_rpt[SSV6006RC_MAX_RATE_SERIES];
	struct ssv_minstrel_ampdu_rate_rpt *ampdu_rpt;
	int    rpt_idx = -1;

	if ((wsid < 0) || (wsid >= SSV_NUM_STA)) 
		return;
	
	ssv_sta = &sc->sta_info[wsid];
	if (!ssv_sta->sta) 
		return;
	
	ssv_sta_priv = (struct ssv_sta_priv_data *)ssv_sta->sta->drv_priv;
	if((minstrel_sta_priv = (struct ssv_minstrel_sta_priv *)ssv_sta_priv->rc_info) == NULL)
		return;

	if (!minstrel_sta_priv->is_ht)	
		return;
	
	mhs = (struct ssv_minstrel_ht_sta *)&minstrel_sta_priv->ht;
   	tx_desc = (struct ssv6006_tx_desc *)rate_rpt;
	if (!ssv6006_rc_get_previous_ampdu_rpt(mhs, tx_desc->tx_pkt_run_no, &rpt_idx))
	{
		//add rate rpt to report list
		ssv6006_rc_add_ampdu_rpt_to_list(sc, mhs, tx_desc, 0, 0, 0);
	} else {
		ampdu_rpt = (struct ssv_minstrel_ampdu_rate_rpt *)&mhs->ampdu_rpt_list[rpt_idx];

		_update_ht_rpt(sc, tx_desc, ht_rpt);	
    	
		if (sc->sh->cfg.auto_rate_enable == true) {
			ssv_minstrel_ht_tx_status(sc, ssv_sta_priv->rc_info, ht_rpt, SSV6006RC_MAX_RATE_SERIES, 
				ampdu_rpt->ampdu_len, ampdu_rpt->ampdu_ack_len, 
				tx_desc->is_rate_stat_sample_pkt);
		}
		ampdu_rpt->used = false;
	}
}

void ssv6006_rc_legacy_bitrate_to_rate_desc(int bitrate, u8 *drate)
{
	/* 
	 * build the mapping with mac80211 rate index and ssv rate descript.
	 * mac80211 rate index   <---->   ssv rate descript
	 * b mode 1M             <---->   0x00
	 * b mode 2M             <---->   0x01
	 * b mode 5.5M           <---->   0x02
	 * b mode 11M            <---->   0x03
	 * g mode 6M             <---->   0x80
	 * g mode 9M             <---->   0x81
	 */
	*drate = 0;

	switch (bitrate) {
		case 10:
				*drate = ((SSV6006RC_B_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_B_1M << SSV6006RC_RATE_SFT));
			return;
		case 20:
				*drate = ((SSV6006RC_B_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_B_2M << SSV6006RC_RATE_SFT));
			return;
		case 55:
				*drate = ((SSV6006RC_B_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_B_5_5M << SSV6006RC_RATE_SFT));
			return;
		case 110:
				*drate = ((SSV6006RC_B_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_B_11M << SSV6006RC_RATE_SFT));
			return;
		case 60:
				*drate = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_6M << SSV6006RC_RATE_SFT));
			return;
		case 90:
				*drate = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_9M << SSV6006RC_RATE_SFT));
			return;
		case 120:
				*drate = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_12M << SSV6006RC_RATE_SFT));
			return;
		case 180:
				*drate = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_18M << SSV6006RC_RATE_SFT));
			return;
		case 240:
				*drate = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_24M << SSV6006RC_RATE_SFT));
			return;
		case 360:
				*drate = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_36M << SSV6006RC_RATE_SFT));
			return;
		case 480:
				*drate = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_48M << SSV6006RC_RATE_SFT));
			return;
		case 540:
				*drate = ((SSV6006RC_G_MODE << SSV6006RC_PHY_MODE_SFT) | (SSV6006RC_G_54M << SSV6006RC_RATE_SFT));
			return;
		default:
				printk("For B/G mode, it doesn't support the bitrate %d kbps\n", bitrate * 100);
				WARN_ON(1);
			return;
	}
}

static int ssv6006_rc_legacy_drate_to_ndx(struct ssv_minstrel_sta_priv *minstrel_sta_priv, u8 drate)
{
	/* 
	 * Find the rate index in minstrel rate control from ssv rate descript.
	 * In order to update correct rate status.
	 */
	int i, ndx = -1;
	int rix = drate & SSV6006RC_RATE_MSK;
	int band = ((drate & SSV6006RC_PHY_MODE_MSK) >> SSV6006RC_PHY_MODE_SFT);
	struct ssv_minstrel_sta_info *smi = (struct ssv_minstrel_sta_info *)&minstrel_sta_priv->legacy;

	if (band & SSV6006RC_G_MODE)
		ndx = rix + smi->g_rates_offset;
	else
		ndx = rix;

	for (i = ndx; i >= 0; i--)
		if ((minstrel_sta_priv->ratelist[i].hw_rate_desc & SSV6006RC_RATE_MSK) == rix)
			break;
	return i;
}

static void ssv6006_rc_process_rate_report(struct ssv_softc *sc, struct sk_buff *skb)
{
    struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *)skb->data;
	struct ssv_minstrel_sta_priv *minstrel_sta_priv;
	struct ssv_minstrel_sta_info *smi;
	struct ssv_sta_priv_data *sta_priv;
	struct ssv_sta_info *ssv_sta;
	struct ssv_minstrel_ht_rpt ht_rpt[SSV6006RC_MAX_RATE_SERIES];
	int i, ndx;
	u8 drate, success, rpt_trycnt;
	int last = 0;
	/* 
	 * Rate Report Result
	 * 0: typical max retry fail
	 * 1: typcial success
	 * 2: not used
	 * 3: power saving mode, reject tx
	 * */
	if ((tx_desc->rpt_result0 == 3) || 
		(tx_desc->rpt_result1 == 3) ||
		(tx_desc->rpt_result2 == 3) ||
		(tx_desc->rpt_result3 == 3)) {
		printk("Enter power saving mode and drop the report\n");
		/* TODO: notify smac to do retransmission*/
		return;
	}
	
	/* AMPDU rate report handler*/
	if (tx_desc->aggr != 0) {
		if ((tx_desc->rpt_result0 == 0) && 
			(tx_desc->rpt_result1 == 0) && 
			(tx_desc->rpt_result2 == 0) && 
			(tx_desc->rpt_result3 == 0)) {
			ssv6006_rc_no_ba_handler(sc, skb);
		} else {
			ssv6006_rc_ba_handler(sc, tx_desc, tx_desc->wsid);
		}
		return;
	}

    if (sc->sh->cfg.auto_rate_enable == false)
		return;

	if ((tx_desc->wsid < 0) || (tx_desc->wsid >= SSV_NUM_STA)) {
		dev_warn(sc->dev, "%s(): wsid[%d] is invaild!!\n", __FUNCTION__, tx_desc->wsid);
		return;
	}
	
	ssv_sta = &sc->sta_info[tx_desc->wsid];
	if (!ssv_sta->sta) {
		dev_warn(sc->dev, "%s(): Cannot find the station\n", __FUNCTION__);
		return;
	}

	sta_priv = (struct ssv_sta_priv_data *)ssv_sta->sta->drv_priv;
	minstrel_sta_priv = (struct ssv_minstrel_sta_priv *)sta_priv->rc_info;
	if (!minstrel_sta_priv) {
		dev_warn(sc->dev, "%s(): Cannot find the station's minstrel data\n", __FUNCTION__);
		return;
	}

	/* MPDU rate report handler */
	if (!minstrel_sta_priv->is_ht) {
		smi = (struct ssv_minstrel_sta_info *)&minstrel_sta_priv->legacy;
		for (i = 0; i < SSV6006RC_MAX_RATE_SERIES; i++) {
			last = ssv6006_get_tx_desc_rate_rpt(sc, tx_desc, i, &drate, &success, &rpt_trycnt);	
			
			if (drate < 0)
				continue;

			ndx = ssv6006_rc_legacy_drate_to_ndx(minstrel_sta_priv, drate);
			if (ndx < 0)
				continue;

			minstrel_sta_priv->ratelist[ndx].attempts += rpt_trycnt;
			minstrel_sta_priv->ratelist[ndx].success += success;
			if (sc->sh->cfg.rc_log) {
				printk("%s(), bitrate[%d], success=%d, attempts=%d\n", __FUNCTION__, 
					minstrel_sta_priv->ratelist[ndx].bitrate, 
					minstrel_sta_priv->ratelist[ndx].attempts,
					minstrel_sta_priv->ratelist[ndx].success);
			}

			if (last) 
				break;
		}
		if (tx_desc->is_rate_stat_sample_pkt)
			smi->sample_count++;

		if (smi->sample_deferred > 0)
			smi->sample_deferred--;
	} else {
		for (i = 0; i < SSV6006RC_MAX_RATE_SERIES; i++) {
			last = ssv6006_get_tx_desc_rate_rpt(sc, tx_desc, i, &drate, &success, &rpt_trycnt);
			ht_rpt[i].dword = drate;
			ht_rpt[i].count = rpt_trycnt;
			ht_rpt[i].success = success;
			ht_rpt[i].last = last;
			
			if ((drate < 0) || success)
				ht_rpt[i].last = 1;

			if (ht_rpt[i].last)
				break;
		}
		ssv_minstrel_ht_tx_status(sc, sta_priv->rc_info, ht_rpt, SSV6006RC_MAX_RATE_SERIES, 
					1, 1, tx_desc->is_rate_stat_sample_pkt);
	}
}

static void ssv6006_rc_update_basic_rate(struct ssv_softc *sc, u32 basic_rates)
{
	return;
}

s32 ssv6006_rc_ht_update_update(struct sk_buff *skb, struct ssv_softc *sc, struct fw_rc_retry_params *ar)
{
	return ssv_minstrel_ht_update_rate(sc, skb);
}

bool ssv6006_rc_ht_sta_current_rate_is_cck(struct ieee80211_sta *sta)
{
    return ssv_minstrel_ht_sta_is_cck_rates(sta);
}

#define NUMBER_OF_MCS 8
int ssv6006_ampdu_max_transmit_length(struct sk_buff *skb, int rate_idx)
{
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_tx_rate *ar;
	int    length = 4429; // min value for default

    u32 ampdu_max_transmit_length_lgi[NUMBER_OF_MCS] = {4429, 8860, 13291, 17723, 26586, 35448, 39880, 44311};// for LGI
    u32 ampdu_max_transmit_length_sgi[NUMBER_OF_MCS] = {4921, 9844, 14768, 19692, 29539, 39387, 44311, 49234};
 
	ar = &info->control.rates[2];
	
	if (ar->flags & IEEE80211_TX_RC_MCS) {
		if (ar->flags & IEEE80211_TX_RC_SHORT_GI) {
		    length = ampdu_max_transmit_length_sgi[ar->idx];
		} else { 
		    length = ampdu_max_transmit_length_lgi[ar->idx];
		}
        if (ar->flags & IEEE80211_TX_RC_40_MHZ_WIDTH){
            length = length*2;  // for 40M
        } 
	}

	return length;
}

static void ssv6006_cmd_cci(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;

    if ((argc ==2) || (argc ==3)){
        if(!strcmp(argv[1], "enable")){
            
            sh->cfg.cci |= CCI_CTL;
            snprintf_res(cmd_data,"\n\t turn on CCI detection\n");
        } else if(!strcmp(argv[1], "disable")){
            
            sh->cfg.cci = 0;
            SMAC_REG_WRITE(sh, ADR_WIFI_11B_RX_REG_040, 0);
            SMAC_REG_WRITE(sh, ADR_WIFI_11GN_RX_REG_040, 0);
            sh->sc->cci_current_level = 0;
            snprintf_res(cmd_data,"\n\t turn off CCI detection\n");
        } else if(!strcmp(argv[1], "dbg")){
            if(!strcmp(argv[2], "enable")){
                sh->cfg.cci |= CCI_DBG;
                snprintf_res(cmd_data,"\n\t turn on CCI DBG\n");
            } else if(!strcmp(argv[2], "disable")){
                sh->cfg.cci &= (~CCI_DBG);
                snprintf_res(cmd_data,"\n\t turn off CCI DBG\n");
            }         
        } else if(!strcmp(argv[1], "p1")){
            if(!strcmp(argv[2], "enable")){
                sh->cfg.cci |= CCI_P1;
                snprintf_res(cmd_data,"\n\t turn on CCI P1\n");
            } else if(!strcmp(argv[2], "disable")){
                sh->cfg.cci &= (~CCI_P1);
                snprintf_res(cmd_data,"\n\t turn off CCI P1\n");
            }         
        } else if(!strcmp(argv[1], "p2")){
            if(!strcmp(argv[2], "enable")){
                sh->cfg.cci |= CCI_P2;
                snprintf_res(cmd_data,"\n\t turn on CCI P2\n");
            } else if(!strcmp(argv[2], "disable")){
                sh->cfg.cci &= (~CCI_P2);
                snprintf_res(cmd_data,"\n\t turn off CCI P2\n");
            }         
        } else if(!strcmp(argv[1], "smart")){
            if(!strcmp(argv[2], "enable")){
                sh->cfg.cci |= CCI_SMART;
                snprintf_res(cmd_data,"\n\t turn on CCI SMART\n");
            } else if(!strcmp(argv[2], "disable")){
                sh->cfg.cci &= (~CCI_SMART);
                snprintf_res(cmd_data,"\n\t turn off CCI SMART\n");
            }
        } else {
            snprintf_res(cmd_data,"\n\t./cli cci [dbg|p1|p2|smart] enable|disable\n");
        }
    } else {
        snprintf_res(cmd_data,"\n\t./cli cci [dbg|p1|p2|smart] enable|disable\n");
    }
        
    snprintf_res(cmd_data,"\n\t CCI setting 0x%x\n", sh->cfg.cci);
    
}

void ssv_attach_ssv6006_phy(struct ssv_hal_ops *hal_ops)
{
    hal_ops->add_txinfo = ssv6006_add_txinfo;
    hal_ops->update_txinfo = ssv6006_update_txinfo;
    hal_ops->update_ampdu_txinfo = ssv6006_update_ampdu_txinfo;
    hal_ops->add_ampdu_txinfo = ssv6006_add_ampdu_txinfo;
    hal_ops->update_null_func_txinfo = ssv6006_update_null_func_txinfo;
    hal_ops->get_tx_desc_size = ssv6006_get_tx_desc_size;
    hal_ops->get_tx_desc_ctype = ssv6006_get_tx_desc_ctype;
    hal_ops->get_tx_desc_reason = ssv6006_get_tx_desc_reason;
    hal_ops->get_tx_desc_txq_idx = ssv6006_get_tx_desc_txq_idx;
    hal_ops->tx_rate_update = ssv6006_tx_rate_update;
    hal_ops->txtput_set_desc = ssv6006_txtput_set_desc;
    hal_ops->fill_beacon_tx_desc = ssv6006_fill_beacon_tx_desc;
    hal_ops->fill_lpbk_tx_desc = ssv6006_fill_lpbk_tx_desc;
    hal_ops->chk_lpbk_rx_rate_desc = ssv6006_chk_lpbk_rx_rate_desc;

    hal_ops->get_tkip_mmic_err = ssv6006_get_tkip_mmic_err;
    hal_ops->get_rx_desc_size = ssv6006_get_rx_desc_size;    
    hal_ops->get_rx_desc_length = ssv6006_get_rx_desc_length;
    hal_ops->get_rx_desc_wsid = ssv6006_get_rx_desc_wsid;
    hal_ops->get_rx_desc_rate_idx = ssv6006_get_rx_desc_rate_idx;
    hal_ops->get_rx_desc_mng_used = ssv6006_get_rx_desc_mng_used;
    hal_ops->is_rx_aggr = ssv6006_is_rx_aggr;
    hal_ops->get_rx_desc_ctype = ssv6006_get_rx_desc_ctype;
    hal_ops->get_rx_desc_hdr_offset = ssv6006_get_rx_desc_hdr_offset;
    hal_ops->get_rx_desc_info = ssv6006_get_rx_desc_info;
    hal_ops->nullfun_frame_filter = ssv6006_nullfun_frame_filter;

    hal_ops->phy_enable = ssv6006_phy_enable;
    hal_ops->set_phy_mode = ssv6006_set_phy_mode;
    hal_ops->edca_enable = ssv6006_edca_enable;
    hal_ops->edca_stat = ssv6006_edca_stat;
    hal_ops->reset_mib_phy = ssv6006_reset_mib_phy;
    hal_ops->dump_mib_rx_phy = ssv6006_dump_mib_rx_phy;
    hal_ops->rc_mac80211_rate_idx = ssv6006_rc_mac80211_rate_idx;
    hal_ops->update_rssi = ssv6006_update_rssi;
#ifdef CONFIG_SSV_CCI_IMPROVEMENT
    hal_ops->update_scan_cci_setting = ssv6006_update_scan_cci_setting;
    hal_ops->recover_scan_cci_setting = ssv6006_recover_scan_cci_setting;
#endif
    hal_ops->cmd_rc = ssv6006_cmd_rc;

    hal_ops->is_legacy_rate = ssv6006_is_legacy_rate;
	hal_ops->ampdu_max_transmit_length = ssv6006_ampdu_max_transmit_length;

	hal_ops->rc_algorithm = ssv6006_rc_algorithm;
	hal_ops->set_80211_hw_rate_config = ssv6006_set_80211_hw_rate_config;
	hal_ops->rc_legacy_bitrate_to_rate_desc = ssv6006_rc_legacy_bitrate_to_rate_desc;
	hal_ops->rc_rx_data_handler = ssv6006_rc_rx_data_handler;
	hal_ops->rate_report_handler = ssv6006_rate_report_handler;
	hal_ops->rc_process_rate_report = ssv6006_rc_process_rate_report;
	hal_ops->rc_update_basic_rate = ssv6006_rc_update_basic_rate;
	hal_ops->rc_ht_update_update = ssv6006_rc_ht_update_update;
	hal_ops->rc_ht_sta_current_rate_is_cck = ssv6006_rc_ht_sta_current_rate_is_cck;
	hal_ops->cmd_cci = ssv6006_cmd_cci;
}
#endif
