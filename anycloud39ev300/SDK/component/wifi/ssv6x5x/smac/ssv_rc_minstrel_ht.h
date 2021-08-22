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

#ifndef _SSV_RC_MINSTREL_HT_H_
#define _SSV_RC_MINSTREL_HT_H_

#define EWMA_LEVEL      75

#define MINSTREL_MAX_STREAMS    1
#define MINSTREL_STREAM_GROUPS  4

/* scaled fraction values */
#define MINSTREL_SCALE  16
#define MINSTREL_FRAC(val, div) (((val) << MINSTREL_SCALE) / div)
#define MINSTREL_TRUNC(val) ((val) >> MINSTREL_SCALE)

#define MCS_GROUP_RATES 8

#define SSV_MINSTREL_AMPDU_RATE_RPTS 128

#define GET_RATE_INDEX( _max_tp_rate)   ( _max_tp_rate % MCS_GROUP_RATES)
#define GET_CCK_RATE_INDEX( _max_tp_rate)   ( _max_tp_rate % 4)

struct ssv_minstrel_ht_rpt {
	u8 dword;
	u8 count;
	int success;
	int last;
};

struct ssv_minstrel_ampdu_rate_rpt {
	bool used;
	int pkt_no;
	int ampdu_len;
	int ampdu_ack_len;
	bool is_sample;
	struct ssv_minstrel_ht_rpt rate_rpt[4]; // reference to SSV6006RC_MAX_RATE_SERIES
};

struct ssv_mcs_ht_group {
	u32 flags;
	unsigned int streams;
	unsigned int duration[MCS_GROUP_RATES];
};

struct ssv_minstrel_ht_rate_stats {
	unsigned int attempts, last_attempts;
	unsigned int success, last_success;
	u64 att_hist, succ_hist;
	unsigned int cur_tp;
	unsigned int cur_prob, probability;
	unsigned int retry_count;
	unsigned int retry_count_rtscts;
	bool retry_updated;
	u16 sample_skipped;
	unsigned long last_jiffies;
};

struct ssv_minstrel_ht_mcs_group_data {
	u8 index;
	u8 column;
	u8 supported;
	unsigned int max_tp_rate;
	unsigned int max_tp_rate2;
	unsigned int max_prob_rate;
	struct ssv_minstrel_ht_rate_stats rates[MCS_GROUP_RATES];
};

struct ssv_minstrel_ht_sta {
	unsigned int ampdu_len;
	unsigned int ampdu_packets;
	unsigned int avg_ampdu_len;
	unsigned int max_tp_rate;
	unsigned int max_tp_rate2;
	unsigned int max_prob_rate;
	unsigned long stats_update;
	unsigned int overhead;
	unsigned int overhead_rtscts;
	u64 total_packets;
	u64 sample_packets;
	u32 tx_flags;
	u8 sample_wait;
	u8 sample_tries;
	u8 sample_count;
	u8 sample_slow;
	u8 sample_group;
	u8 group_idx;
	u32	secondary_channel_clear;
    u8 cck_supported;
    u8 cck_supported_short;
 	struct ssv_minstrel_ht_mcs_group_data groups[MINSTREL_MAX_STREAMS * MINSTREL_STREAM_GROUPS + 1];
	struct ssv_minstrel_ampdu_rate_rpt ampdu_rpt_list[SSV_MINSTREL_AMPDU_RATE_RPTS];
};

void ssv_minstrel_ht_init_sample_table(void);
int ssv_minstrel_ht_update_rate(struct ssv_softc *sc, struct sk_buff *skb);
bool ssv_minstrel_ht_sta_is_cck_rates(struct ieee80211_sta *sta);

void ssv_minstrel_ht_tx_status(struct ssv_softc *sc, void *rc_info, 
		struct ssv_minstrel_ht_rpt ht_rpt[], int ht_rpt_num, 
		int ampdu_len, int ampdu_ack_len, bool is_sample);
void ssv_minstrel_ht_get_rate(void *priv, struct ieee80211_sta *sta, void *priv_sta,
			struct ieee80211_tx_rate_control *txrc);
void ssv_minstrel_ht_update_caps(void *priv, struct ieee80211_supported_band *sband,
			struct ieee80211_sta *sta, void *priv_sta, enum nl80211_channel_type oper_chan_type);
#endif
