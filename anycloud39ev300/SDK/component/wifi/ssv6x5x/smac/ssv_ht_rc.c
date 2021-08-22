/*
 * Copyright (c) 2014 South Silicon Valley Microelectronics Inc.
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

#include <linux/version.h>
#include <ssv6200.h>
#include "dev.h"
#include "ssv_ht_rc.h"
#include "ssv_rc.h"
#include "ssv_skb.h"

#define SAMPLE_COUNT    4
#define HT_CW_MIN       15
#define HT_SEGMENT_SIZE 6000

#define AVG_PKT_SIZE    12000
#define SAMPLE_COLUMNS  10
#define EWMA_LEVEL      75

/* Number of bits for an average sized packet */
#define MCS_NBITS (AVG_PKT_SIZE << 3)

/* Number of symbols for a packet with (bps) bits per symbol */
#define MCS_NSYMS(bps) ((MCS_NBITS + (bps) - 1) / (bps))

/* Transmission time for a packet containing (syms) symbols */
#define MCS_SYMBOL_TIME(sgi, syms)                  \
    (sgi ?                              \
      ((syms) * 18 + 4) / 5 :   /* syms * 3.6 us */     \
      (syms) << 2           /* syms * 4 us */       \
    )

/* Transmit duration for the raw data part of an average sized packet */
#define MCS_DURATION(streams, sgi, bps) MCS_SYMBOL_TIME(sgi, MCS_NSYMS((streams) * (bps)))

/* MCS rate information for an MCS group */
#define MCS_GROUP(_streams, _sgi, _ht40) {              \
    .duration = {                           \
        MCS_DURATION(_streams, _sgi, _ht40 ? 54 : 26),      \
        MCS_DURATION(_streams, _sgi, _ht40 ? 108 : 52),     \
        MCS_DURATION(_streams, _sgi, _ht40 ? 162 : 78),     \
        MCS_DURATION(_streams, _sgi, _ht40 ? 216 : 104),    \
        MCS_DURATION(_streams, _sgi, _ht40 ? 324 : 156),    \
        MCS_DURATION(_streams, _sgi, _ht40 ? 432 : 208),    \
        MCS_DURATION(_streams, _sgi, _ht40 ? 486 : 234),    \
        MCS_DURATION(_streams, _sgi, _ht40 ? 540 : 260)     \
    }                               \
}

/*
 * To enable sufficiently targeted rate sampling, MCS rates are divided into
 * groups, based on the number of streams and flags (HT40, SGI) that they
 * use.
 */
const struct mcs_group minstrel_mcs_groups[] = {
    MCS_GROUP(1, 0, 0),
    MCS_GROUP(1, 1, 0),
};


static u8 sample_table[SAMPLE_COLUMNS][MCS_GROUP_RATES];

/*
 * Perform EWMA (Exponentially Weighted Moving Average) calculation
 */
static int minstrel_ewma(int old, int new, int weight)
{
    return (new * (100 - weight) + old * weight) / 100;
}


static inline struct minstrel_rate_stats *minstrel_get_ratestats(struct ssv62xx_ht *mi, int index)
{
    return &mi->groups.rates[index % MCS_GROUP_RATES];
}


/*
 * Recalculate success probabilities and counters for a rate using EWMA
 */
static void minstrel_calc_rate_ewma(struct minstrel_rate_stats *mr)
{
    if (unlikely(mr->attempts > 0)) {
        mr->sample_skipped = 0;
        mr->cur_prob = MINSTREL_FRAC(mr->success, mr->attempts);
        if (!mr->att_hist)
            mr->probability = mr->cur_prob;
        else
            mr->probability = minstrel_ewma(mr->probability,
                mr->cur_prob, EWMA_LEVEL);
        mr->att_hist += mr->attempts;
        mr->succ_hist += mr->success;
    } else {
        mr->sample_skipped++;
    }
    mr->last_success = mr->success;
    mr->last_attempts = mr->attempts;
    mr->success = 0;
    mr->attempts = 0;
}

/*
 * Calculate throughput based on the average A-MPDU length, taking into account
 * the expected number of retransmissions and their expected length
 */
static void minstrel_ht_calc_tp(struct ssv62xx_ht *mi, struct ssv_sta_rc_info *rc_sta, int rate)
{
    struct minstrel_rate_stats *mr;
    unsigned int usecs,group_id;
    if(rc_sta->ht_rc_type == RC_TYPE_HT_LGI_20)
        group_id = 0;
    else
        group_id = 1;

    mr = &mi->groups.rates[rate];

    if (mr->probability < MINSTREL_FRAC(1, 10)) {
        mr->cur_tp = 0;
        return;
    }

    usecs = mi->overhead / MINSTREL_TRUNC(mi->avg_ampdu_len);
    usecs += minstrel_mcs_groups[group_id].duration[rate];
    mr->cur_tp = MINSTREL_TRUNC((1000000 / usecs) * mr->probability);
}

/*
 * Update rate statistics and select new primary rates
 *
 * Rules for rate selection:
 *  - max_prob_rate must use only one stream, as a tradeoff between delivery
 *    probability and throughput during strong fluctuations
 *  - as long as the max prob rate has a probability of more than 3/4, pick
 *    higher throughput rates, even if the probablity is a bit lower
 */
static void rate_control_ht_sample(struct ssv62xx_ht *mi,struct ssv_sta_rc_info *rc_sta)
{
    struct minstrel_mcs_group_data *mg;
    struct minstrel_rate_stats *mr;
    int cur_prob, cur_prob_tp, cur_tp, cur_tp2;
    int i, index;

    if (mi->ampdu_packets > 0) {
        mi->avg_ampdu_len = minstrel_ewma(mi->avg_ampdu_len,
            MINSTREL_FRAC(mi->ampdu_len, mi->ampdu_packets), EWMA_LEVEL);
        mi->ampdu_len = 0;
        mi->ampdu_packets = 0;
    }
    else
        return;

    mi->sample_slow = 0;
    mi->sample_count = 0;
    //mi->max_tp_rate = 0;
    //mi->max_tp_rate2 = 0;
    //mi->max_prob_rate = 0;

    //for (group = 0; group < ARRAY_SIZE(minstrel_mcs_groups); group++)
    {
        cur_prob = 0;
        cur_prob_tp = 0;
        cur_tp = 0;
        cur_tp2 = 0;

        mg = &mi->groups;
        //if (!mg->supported)
            //continue;

        mg->max_tp_rate = 0;
        mg->max_tp_rate2 = 0;
        mg->max_prob_rate = 0;
        //mi->sample_count++;

        for (i = 0; i < MCS_GROUP_RATES; i++) {
            if (!(rc_sta->ht_supp_rates & BIT(i)))
                continue;

            mr = &mg->rates[i];
            //mr->retry_updated = false;
            index = i;
            minstrel_calc_rate_ewma(mr);
            minstrel_ht_calc_tp(mi, rc_sta, i);
#ifdef RATE_CONTROL_HT_PARAMETER_DEBUG
            if(mr->cur_prob)
                printk("rate[%d]probability[%08d]cur_prob[%08d]TP[%04d]\n",i,mr->probability,mr->cur_prob,mr->cur_tp);
#endif

#ifdef RATE_CONTROL_HT_STUPID_DEBUG
            printk("HT sample result max_tp_rate[%d]max_tp_rate2[%d]max_prob_rate[%d]\n",mg->max_tp_rate,mg->max_tp_rate2,mg->max_prob_rate);
            printk("rate[%d]probability[%08d]TP[%d]\n",i,mr->probability,mr->cur_tp);
#endif

            if (!mr->cur_tp)
                continue;

            /* ignore the lowest rate of each single-stream group */
            //if (!i && minstrel_mcs_groups[group].streams == 1)
                //continue;

#ifdef RATE_CONTROL_HT_STUPID_DEBUG
            printk("HT--1 mr->cur_tp[%d]cur_prob_tp[%d]\n",mr->cur_tp,cur_prob_tp);
#endif

            if ((mr->cur_tp > cur_prob_tp && mr->probability >
                 MINSTREL_FRAC(3, 4)) || mr->probability > cur_prob) {
                mg->max_prob_rate = index;
                cur_prob = mr->probability;
                cur_prob_tp = mr->cur_tp;
            }

#ifdef RATE_CONTROL_HT_STUPID_DEBUG
            printk("HT--2 mr->cur_tp[%d]cur_tp[%d]\n",mr->cur_tp,cur_tp);
#endif

            if (mr->cur_tp > cur_tp) {
                swap(index, mg->max_tp_rate);
                cur_tp = mr->cur_tp;
                mr = minstrel_get_ratestats(mi, index);
            }

#ifdef RATE_CONTROL_HT_STUPID_DEBUG
            if(index != i)
                printk("HT--3 index[%d]i[%d]mg->max_tp_rate[%d]\n",index,i,mg->max_tp_rate);
#endif

            if (index >= mg->max_tp_rate)
                continue;

#ifdef RATE_CONTROL_HT_STUPID_DEBUG
            if(index != i)
                printk("HT--4 mr->cur_tp[%d]cur_tp2[%d]\n",mr->cur_tp,cur_tp2);
#endif

            if (mr->cur_tp > cur_tp2) {
                mg->max_tp_rate2 = index;
                cur_tp2 = mr->cur_tp;
            }
        }
    }

    /* try to sample up to half of the available rates during each interval */
    mi->sample_count = SAMPLE_COUNT;
#if 0
    cur_prob = 0;
    cur_prob_tp = 0;
    cur_tp = 0;
    cur_tp2 = 0;
    //for (group = 0; group < ARRAY_SIZE(minstrel_mcs_groups); group++)
    {
        mg = &mi->groups;
        //if (!mg->supported)
            //continue;

        mr = minstrel_get_ratestats(mi, mg->max_prob_rate);
        //if (cur_prob_tp < mr->cur_tp && minstrel_mcs_groups[group].streams == 1)
        if (cur_prob_tp < mr->cur_tp) {
            mi->max_prob_rate = mg->max_prob_rate;
            cur_prob = mr->cur_prob;
            cur_prob_tp = mr->cur_tp;
        }

        mr = minstrel_get_ratestats(mi, mg->max_tp_rate);
        if (cur_tp < mr->cur_tp) {
            mi->max_tp_rate2 = mi->max_tp_rate;
            cur_tp2 = cur_tp;
            mi->max_tp_rate = mg->max_tp_rate;
            cur_tp = mr->cur_tp;
        }

        mr = minstrel_get_ratestats(mi, mg->max_tp_rate2);
        if (cur_tp2 < mr->cur_tp) {
            mi->max_tp_rate2 = mg->max_tp_rate2;
            cur_tp2 = mr->cur_tp;
        }
    }
#else
    mi->max_tp_rate = mg->max_tp_rate;
    mi->max_tp_rate2 = mg->max_tp_rate2;
    mi->max_prob_rate = mg->max_prob_rate;
#endif

#ifdef RATE_CONTROL_HT_STUPID_DEBUG
        //printk("HT sample result max_tp_rate[%d]max_tp_rate2[%d]max_prob_rate[%d]\n",mg->max_tp_rate,mg->max_tp_rate2,mg->max_prob_rate);
        printk("HT sample result max_tp_rate[%d]max_tp_rate2[%d]max_prob_rate[%d]\n",mi->max_tp_rate,mi->max_tp_rate2,mi->max_prob_rate);
#endif
    //printk("HT sample result max_tp_rate[%d]max_tp_rate2[%d]max_prob_rate[%d]\n",mi->max_tp_rate,mi->max_tp_rate2,mi->max_prob_rate);

    mi->stats_update = jiffies;
}
#if 0
static void minstrel_calc_retransmit(struct ssv62xx_ht *mi,int index, struct ssv_sta_rc_info *rc_sta)
{
    struct minstrel_rate_stats *mr;
    const struct mcs_group *group;
    unsigned int tx_time, tx_time_rtscts, tx_time_data;
    unsigned int cw = HT_CW_MIN;
    unsigned int cw_max = 1023;
    unsigned int ctime = 0;
    unsigned int t_slot = 9; /* FIXME */
    unsigned int ampdu_len = MINSTREL_TRUNC(mi->avg_ampdu_len);
    unsigned int group_id;
    if(rc_sta->ht_rc_type == RC_TYPE_HT_LGI_20)
        group_id = 0;
    else
        group_id = 1;

    mr = minstrel_get_ratestats(mi, index);
    if (mr->probability < MINSTREL_FRAC(1, 10)) {
        mr->retry_count = 1;
        mr->retry_count_rtscts = 1;
        return;
    }

    mr->retry_count = 2;
    mr->retry_count_rtscts = 2;
    mr->retry_updated = true;

    group = &minstrel_mcs_groups[group_id];
    tx_time_data = group->duration[index % MCS_GROUP_RATES] * ampdu_len;

    /* Contention time for first 2 tries */
    ctime = (t_slot * cw) >> 1;
    cw = min((cw << 1) | 1, cw_max);
    ctime += (t_slot * cw) >> 1;
    cw = min((cw << 1) | 1, cw_max);

    /* Total TX time for data and Contention after first 2 tries */
    tx_time = ctime + 2 * (mi->overhead + tx_time_data);
    tx_time_rtscts = ctime + 2 * (mi->overhead_rtscts + tx_time_data);

    /* See how many more tries we can fit inside segment size */
    do {
        /* Contention time for this try */
        ctime = (t_slot * cw) >> 1;
        cw = min((cw << 1) | 1, cw_max);

        /* Total TX time after this try */
        tx_time += ctime + mi->overhead + tx_time_data;
        tx_time_rtscts += ctime + mi->overhead_rtscts + tx_time_data;

        if (tx_time_rtscts < HT_SEGMENT_SIZE)
            mr->retry_count_rtscts++;
    } while ((tx_time < HT_SEGMENT_SIZE) &&
             (++mr->retry_count < HW_MAX_RATE_TRIES));
}
#endif
static void minstrel_ht_set_rate(struct ssv62xx_ht *mi,
                     struct fw_rc_retry_params *rate, int index,
                     bool sample, bool rtscts, struct ssv_sta_rc_info *rc_sta,
                     struct ssv_rate_ctrl *ssv_rc)
{
    //const struct mcs_group *group = &minstrel_mcs_groups[index / MCS_GROUP_RATES];
    struct minstrel_rate_stats *mr;

    mr = minstrel_get_ratestats(mi, index);
#if 0
    if (!mr->retry_updated)
        minstrel_calc_retransmit(mi, index, rc_sta);

    if (sample)
        rate->count = 1;
    else if (mr->probability < MINSTREL_FRAC(20, 100))
        rate->count = 2;
    else if (rtscts)
        rate->count = mr->retry_count_rtscts;
    else
        rate->count = mr->retry_count;
#endif
    //rate->flags = IEEE80211_TX_RC_MCS | group->flags;
    //if (rtscts)
        //rate->flags |= IEEE80211_TX_RC_USE_RTS_CTS;
    //rate->idx = index % MCS_GROUP_RATES + (group->streams - 1) * MCS_GROUP_RATES;
    rate->drate = ssv_rc->rc_table[mr->rc_index].hw_rate_idx;
    rate->crate = ssv_rc->rc_table[mr->rc_index].ctrl_rate_idx;
}

static inline int minstrel_get_duration(int index, struct ssv_sta_rc_info *rc_sta)
{
    unsigned int group_id;
    const struct mcs_group *group;
    if(rc_sta->ht_rc_type == RC_TYPE_HT_LGI_20)
        group_id = 0;
    else
        group_id = 1;

    group = &minstrel_mcs_groups[group_id];
    return group->duration[index % MCS_GROUP_RATES];
}

static void minstrel_next_sample_idx(struct ssv62xx_ht *mi)
{
    struct minstrel_mcs_group_data *mg;

    for (;;) {
        mg = &mi->groups;
/*
        if (!mg->supported)
            continue;
*/
        if (++mg->index >= MCS_GROUP_RATES) {
            mg->index = 0;
            if (++mg->column >= ARRAY_SIZE(sample_table))
                mg->column = 0;
        }
        break;
    }
}

static int minstrel_get_sample_rate(struct ssv62xx_ht *mi, struct ssv_sta_rc_info *rc_sta)
{
    struct minstrel_rate_stats *mr;
    struct minstrel_mcs_group_data *mg;
    int sample_idx = 0;

    if (mi->sample_wait > 0) {
        mi->sample_wait--;
        return -1;
    }

    if (!mi->sample_tries)
        return -1;

    //printk("@@...sample_wait[%d]sample_tries[%d]\n",mi->sample_wait,mi->sample_tries);

    mi->sample_tries--;
    mg = &mi->groups;
    sample_idx = sample_table[mg->column][mg->index];
    mr = &mg->rates[sample_idx];
    minstrel_next_sample_idx(mi);

    /*
     * When not using MRR, do not sample if the probability is already
     * higher than 95% to avoid wasting airtime
     */
    //if (!mp->has_mrr && (mr->probability > MINSTREL_FRAC(95, 100)))
        //return -1;

    /*
     * Make sure that lower rates get sampled only occasionally,
     * if the link is working perfectly.
     */
    if (minstrel_get_duration(sample_idx, rc_sta) >
        minstrel_get_duration(mi->max_tp_rate, rc_sta)) {
        if (mr->sample_skipped < 20)
        {
            //printk("@-@...mr->sample_skipped < 20\n");
            return -1;
        }

        if (mi->sample_slow++ > 2)
        {
            //printk("@-@...mi->sample_slow++ > 2\n");
            return -1;
        }
    }
    //printk("YA...sample_idx %d\n",sample_idx);
    return sample_idx;
}

// Fill up rates in TX info with highest rate.
static void _fill_txinfo_rates (struct ssv_rate_ctrl *ssv_rc, struct sk_buff *skb, struct fw_rc_retry_params *ar)
{
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

    info->control.rates[0].idx = ssv_rc->rc_table[ar[0].drate].dot11_rate_idx;
    info->control.rates[0].count = 1;

    info->control.rates[SSV_DRATE_IDX].count = ar[0].drate;
    info->control.rates[SSV_CRATE_IDX].count = ar[0].crate;
} // end of - _fill_txinfo_rates -

extern const u16 ssv6xxx_rc_rate_set[RC_TYPE_MAX][13];

s32 ssv62xx_ht_rate_update(struct sk_buff *skb, struct ssv_softc *sc, struct fw_rc_retry_params *ar)
{
    struct ssv_rate_ctrl *ssv_rc = sc->rc;
    struct SKB_info_st *skb_info = (struct SKB_info_st *)skb->head;
    struct ieee80211_sta *sta = skb_info->sta;
    //struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    struct ssv62xx_ht *mi = NULL;
    int sample_idx;
    bool sample = false;
    struct ssv_sta_rc_info *rc_sta;
    //struct ssv_rc_rate *rc_rate = NULL;
    struct ssv_sta_priv_data *sta_priv;
    struct rc_pid_sta_info *spinfo;
    int ret = 0;
    //struct ssv6200_tx_desc *tx_desc = (struct ssv6200_tx_desc *)skb->data;

    if (sc->sc_flags & SC_OP_FIXED_RATE)
    {
        /*
            Must fill fix rate table
        */
        ar[0].count = 3;
        ar[0].drate = ssv_rc->rc_table[sc->max_rate_idx].hw_rate_idx;
        ar[0].crate = ssv_rc->rc_table[sc->max_rate_idx].ctrl_rate_idx;
        ar[1].count = 2;
        ar[1].drate = ssv_rc->rc_table[sc->max_rate_idx].hw_rate_idx;
        ar[1].crate = ssv_rc->rc_table[sc->max_rate_idx].ctrl_rate_idx;
        ar[2].count = 2;
        ar[2].drate = ssv_rc->rc_table[sc->max_rate_idx].hw_rate_idx;
        ar[2].crate = ssv_rc->rc_table[sc->max_rate_idx].ctrl_rate_idx;

        _fill_txinfo_rates(ssv_rc, skb, ar);
        return ssv_rc->rc_table[sc->max_rate_idx].hw_rate_idx;
    }

    if(sta == NULL)
    {
        printk("@Q@...station NULL\n");
        BUG_ON(1);
    }

    sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;
    rc_sta = &ssv_rc->sta_rc_info[sta_priv->rc_idx];

    spinfo= &rc_sta->spinfo;
#if 0
    if ((rc_sta->rc_wsid >= SSV_RC_MAX_HARDWARE_SUPPORT) || (rc_sta->rc_wsid < 0))
    {
        struct ssv_sta_priv_data *ssv_sta_priv;
        int rateidx=99;

        ssv_sta_priv = (struct ssv_sta_priv_data *)sta->drv_priv;

        // if (ssv_sta_priv->rx_data_rate)
        {
            //Avoid use B mode rate in AMPDU session
            if ((rc_sta->ht_rc_type >= RC_TYPE_HT_SGI_20) &&
                (ssv_sta_priv->rx_data_rate < SSV62XX_RATE_MCS_INDEX))
            {
                //Sync to HT rate (MCS0 index)
                if(ssv6xxx_rc_rate_set[rc_sta->ht_rc_type][0] == 12)
                    rateidx = (int)rc_sta->pinfo.rinfo[4].rc_index;
                    //spinfo->txrate_idx = 4;
                else
                    //spinfo->txrate_idx = 0;
                    rateidx = (int)rc_sta->pinfo.rinfo[0].rc_index;
        #if 0
                printk("RC %d rx %d tx %d\n", ssv_sta_priv->sta_idx,
                       ssv_sta_priv->rx_data_rate, rateidx);
        #endif
            }
            else
            {
                rateidx = (int)ssv_sta_priv->rx_data_rate;
                rateidx -= SSV62XX_RATE_MCS_INDEX;
                rateidx %= 8;
                if(rc_sta->ht_rc_type == RC_TYPE_HT_SGI_20)
                    rateidx += SSV62XX_RATE_MCS_SGI_INDEX;
                else if(rc_sta->ht_rc_type == RC_TYPE_HT_LGI_20)
                    rateidx += SSV62XX_RATE_MCS_LGI_INDEX;
                else
                    rateidx += SSV62XX_RATE_MCS_GREENFIELD_INDEX;
            }
            //if(rateidx!=22)
                //printk("wsid=%d, rate %d\n",rc_sta->rc_wsid,rateidx);
        }

       /*
                Must fill fix rate table
            */
        ar[0].count = 3;
        ar[2].drate = ar[1].drate = ar[0].drate = ssv_rc->rc_table[rateidx].hw_rate_idx;
        ar[2].crate = ar[1].crate = ar[0].crate = ssv_rc->rc_table[rateidx].ctrl_rate_idx;
        ar[1].count = 2;
        ar[2].count = 2;
        _fill_txinfo_rates(ssv_rc, skb, ar);
        return rateidx;
    }
#endif
    mi = &rc_sta->ht;

    /* Don't use EAPOL frames for sampling on non-mrr hw */
/*
    if (mp->hw->max_rates == 1 &&
        txrc->skb->protocol == cpu_to_be16(ETH_P_PAE))
        sample_idx = -1;
    else
        sample_idx = minstrel_get_sample_rate(mi);
*/
    sample_idx = minstrel_get_sample_rate(mi, rc_sta);

    if (sample_idx >= 0) {
        sample = true;
        minstrel_ht_set_rate(mi, &ar[0], sample_idx,
            true, false, rc_sta, ssv_rc);
        //info->flags |= IEEE80211_TX_CTL_RATE_CTRL_PROBE;
        //printk("@@...sample_idx [%d]\n",sample_idx);
    } else {
        minstrel_ht_set_rate(mi, &ar[0], mi->max_tp_rate,
            false, false, rc_sta, ssv_rc);
    }
    ar[0].count = mi->first_try_count;
    ret = ar[0].drate;

    //if (mp->hw->max_rates >= 3)
    {
        /*
         * At least 3 tx rates supported, use
         * sample_rate -> max_tp_rate -> max_prob_rate for sampling and
         * max_tp_rate -> max_tp_rate2 -> max_prob_rate by default.
         */
        if (sample_idx >= 0)
            minstrel_ht_set_rate(mi, &ar[1], mi->max_tp_rate,
                false, false, rc_sta, ssv_rc);
        else
            minstrel_ht_set_rate(mi, &ar[1], mi->max_tp_rate2,
                false, true, rc_sta, ssv_rc);
        ar[1].count = mi->second_try_count;
        if(ret > ar[1].drate)
            ret = ar[1].drate;

        minstrel_ht_set_rate(mi, &ar[2], mi->max_prob_rate,
                     false, !sample, rc_sta, ssv_rc);
        ar[2].count = mi->other_try_count;
        if(ret > ar[2].drate)
            ret = ar[2].drate;

        //SSV62XX_TX_MAX_RATES 4
        //ar[3].count = 0;
        //ar[3].drate = -1;
    }

    mi->total_packets++;

    /* wraparound */
    if (mi->total_packets == ~0) {
        mi->total_packets = 0;
        mi->sample_packets = 0;
    }

    if(spinfo->real_hw_index < SSV62XX_RATE_MCS_INDEX)
        return spinfo->real_hw_index;

    _fill_txinfo_rates(ssv_rc, skb, ar);
    return ret;
}

static void init_sample_table(void)
{
    int col, i, new_idx;
    u8 rnd[MCS_GROUP_RATES];

    memset(sample_table, 0xff, sizeof(sample_table));
    for (col = 0; col < SAMPLE_COLUMNS; col++) {
        for (i = 0; i < MCS_GROUP_RATES; i++) {
            get_random_bytes(rnd, sizeof(rnd));
            new_idx = (i + rnd[i]) % MCS_GROUP_RATES;

            while (sample_table[col][new_idx] != 0xff)
                new_idx = (new_idx + 1) % MCS_GROUP_RATES;

            sample_table[col][new_idx] = i;
        }
    }
}

void ssv62xx_ht_rc_caps(const u16 ssv6xxx_rc_rate_set[RC_TYPE_MAX][13],struct ssv_sta_rc_info *rc_sta)
{
    struct ssv62xx_ht *mi = &rc_sta->ht;
    int ack_dur;
    int i;
#if 1
    unsigned int group_id;

    if(rc_sta->ht_rc_type == RC_TYPE_HT_LGI_20)
        group_id = 0;
    else
        group_id = 1;

    for (i = 0; i < MCS_GROUP_RATES; i++) {
            printk("[RC]HT duration[%d][%d]\n",i,minstrel_mcs_groups[group_id].duration[i]);
    }
#endif

    init_sample_table();

    memset(mi, 0, sizeof(*mi));
    mi->stats_update = jiffies;

    ack_dur = pide_frame_duration( 10, 60, 0, 0);
    mi->overhead = pide_frame_duration( 0, 60, 0, 0) + ack_dur;
    mi->overhead_rtscts = mi->overhead + 2 * ack_dur;

    mi->avg_ampdu_len = MINSTREL_FRAC(1, 1);

    /* When using MRR, sample more on the first attempt, without delay */
/*
    if (mp->has_mrr) {
        mi->sample_count = 16;
        mi->sample_wait = 0;
    } else {
        mi->sample_count = 8;
        mi->sample_wait = 8;
    }
*/
    mi->sample_count = 16;
    mi->sample_wait = 0;
    mi->sample_tries = 4;

#ifdef DISABLE_RATE_CONTROL_SAMPLE
    mi->max_tp_rate = MCS_GROUP_RATES - 1;
    mi->max_tp_rate2 = MCS_GROUP_RATES - 1;
    mi->max_prob_rate = MCS_GROUP_RATES - 1;
#endif

#if (HW_MAX_RATE_TRIES == 7)
    {
        mi->first_try_count = 3;
        mi->second_try_count = 2;
        mi->other_try_count = 2;
    }
#else
    {
        mi->first_try_count = 2;
        mi->second_try_count = 1;
        mi->other_try_count = 1;
    }
#endif


    for (i = 0; i < MCS_GROUP_RATES; i++) {
        mi->groups.rates[i].rc_index = ssv6xxx_rc_rate_set[rc_sta->ht_rc_type][i+1];
    }

}

static bool minstrel_ht_txstat_valid(struct ssv62xx_tx_rate *rate)
{
    if (!rate->count)
        return false;

    if (rate->data_rate < 0)
        return false;

    return true;
}

void ssv6xxx_ht_report_handler(struct ssv_softc *sc,struct sk_buff *skb,struct ssv_sta_rc_info *rc_sta)
{
    struct cfg_host_event *host_event;
    struct firmware_rate_control_report_data *report_data;
    struct ssv62xx_ht *mi;
    struct minstrel_rate_stats *rate;
    //struct minstrel_rate_stats *rate, *rate2;
    bool last = false;
    int i = 0;
    u16 report_ampdu_packets = 0;
    unsigned long period;

    host_event = (struct cfg_host_event *)skb->data;
    report_data = (struct firmware_rate_control_report_data *)&host_event->dat[0];

    if(host_event->h_event == SOC_EVT_RC_AMPDU_REPORT) {
#if 0
        printk("SC HT AMPDU wsid[%d]ampdu_len[%d]ampdu_ack_len[%d]\n",report_data->wsid,report_data->ampdu_len,report_data->ampdu_ack_len);
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
#endif
        report_ampdu_packets = 1;
    }
    else if(host_event->h_event == SOC_EVT_RC_MPDU_REPORT) {
        report_data->ampdu_len = 1;
        report_ampdu_packets = report_data->ampdu_len;
#if 0
        printk("SC MPDU wsid[%d]ampdu_len[%d]ampdu_ack_len[%d]\n",report_data->wsid,report_data->ampdu_len,report_data->ampdu_ack_len);
        for (i = 0; i < SSV62XX_TX_MAX_RATES ; i++) {
            if(report_data->rates[i].data_rate == -1)
                break;

            if(report_data->rates[i].count == 0) {
                    printk("*********************************\n");
                    printk("       Illegal MPDU report       \n");
                    printk("*********************************\n");
            }
            printk("        i=[%d] rate[%d] count[%d]\n",i,report_data->rates[i].data_rate,report_data->rates[i].count);
        }
#endif
    }
    else
    {
        printk("RC work get garbage!!\n");
        return;
    }

    mi = &rc_sta->ht;

    mi->ampdu_packets += report_ampdu_packets;
    mi->ampdu_len += report_data->ampdu_len;

    if (!mi->sample_wait && !mi->sample_tries && mi->sample_count > 0) {
        mi->sample_wait = 16 + 2 * MINSTREL_TRUNC(mi->avg_ampdu_len);
        mi->sample_tries = 2;
        mi->sample_count--;
    }
/*
    if (info->flags & IEEE80211_TX_CTL_RATE_CTRL_PROBE)
    {
        mi->sample_packets += report_ampdu_packets;
        //mi->sample_packets += info->status.ampdu_len;
    }
*/
    //Check report status
    for (i = 0; !last; i++) {
        last = (i == SSV62XX_TX_MAX_RATES - 1) ||
               !minstrel_ht_txstat_valid(&report_data->rates[i+1]);

        if (!minstrel_ht_txstat_valid(&report_data->rates[i]))
            break;

#ifdef RATE_CONTROL_DEBUG
        if((report_data->rates[i].data_rate < SSV62XX_RATE_MCS_INDEX) || (report_data->rates[i].data_rate >= SSV62XX_RATE_MCS_GREENFIELD_INDEX)) {
            printk("[RC]ssv6xxx_ht_report_handler get error report rate[%d]\n",report_data->rates[i].data_rate);
            break;
        }
#endif

        rate = &mi->groups.rates[(report_data->rates[i].data_rate - SSV62XX_RATE_MCS_INDEX) % MCS_GROUP_RATES];

        if (last)
            rate->success += report_data->ampdu_ack_len;

        rate->attempts += report_data->rates[i].count * report_data->ampdu_len;
    }
#if 0
    /*
     * check for sudden death of spatial multiplexing,
     * downgrade to a lower number of streams if necessary.
     */
    rate = minstrel_get_ratestats(mi, mi->max_tp_rate);
    if (rate->attempts > 30 &&
        MINSTREL_FRAC(rate->success, rate->attempts) <
        MINSTREL_FRAC(20, 100))
        minstrel_downgrade_rate(mi, &mi->max_tp_rate, true);

    rate2 = minstrel_get_ratestats(mi, mi->max_tp_rate2);
    if (rate2->attempts > 30 &&
        MINSTREL_FRAC(rate2->success, rate2->attempts) <
        MINSTREL_FRAC(20, 100))
        minstrel_downgrade_rate(mi, &mi->max_tp_rate2, false);
#endif

    period = msecs_to_jiffies(SSV_RC_HT_INTERVAL/2);
    if (time_after(jiffies, mi->stats_update + period)) {
        rate_control_ht_sample(mi,rc_sta);
    }
#if 0
    period = msecs_to_jiffies(HT_RC_UPDATE_INTERVAL);
    if (time_after(jiffies, mi->stats_update + period)) {
        struct rc_pid_sta_info *spinfo;
        spinfo = &rc_sta->spinfo;
        
#if 1
        printk("AMPDU rate update time!!\n");
#endif
        if(rc_sta->rc_num_rate == 12)
        {
            if(spinfo->txrate_idx >= 4)
                rc_sta->ht.max_tp_rate = spinfo->txrate_idx - 4;
            else
                rc_sta->ht.max_tp_rate = 0;
        }
        else
            rc_sta->ht.max_tp_rate = spinfo->txrate_idx;

        mi->stats_update = jiffies;
    }
#endif
}



