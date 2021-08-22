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

#ifndef _SSV_RC_MINSTREL_H_
#define _SSV_RC_MINSTREL_H_

#include "ssv_rc_minstrel_ht.h"

struct ssv_minstrel_rate {
	int bitrate;
	int rix;
	u32 flags;
	u8  hw_rate_desc; 
	unsigned int perfect_tx_time;
	unsigned int ack_time;
	int sample_limit;
	unsigned int retry_count;
	unsigned int retry_count_cts;
	unsigned int retry_count_rtscts;
	unsigned int adjusted_retry_count;
	u32 success;
	u32 attempts;
	u32 last_attempts;
	u32 last_success;
	u32 cur_prob;
	u32 probability;
	u32 cur_tp;
	u64 succ_hist;
	u64 att_hist;
	unsigned long last_jiffies;
};

struct ssv_minstrel_sta_info {
	unsigned long stats_update;
	unsigned int sp_ack_dur;
	unsigned int rate_avg;
	unsigned int lowest_rix;
	unsigned int max_tp_rate;
	unsigned int max_tp_rate2;
	unsigned int max_prob_rate;
	u64 packet_count;
	u64 sample_count;
	int sample_deferred;
	unsigned int sample_idx;
	unsigned int sample_column;
	int n_rates;
	int g_rates_offset;
	bool prev_sample;
};

struct ssv_minstrel_debugfs_info {
	size_t len;
	char buf[];
};

struct ssv_minstrel_sta_priv {
	union {
		struct ssv_minstrel_sta_info legacy;
		struct ssv_minstrel_ht_sta ht;
	};
	struct ieee80211_sta *sta;
	bool is_ht;
    bool update_aggr_check;
	struct ssv_minstrel_rate *ratelist;
	u8 *sample_table;
#ifdef CONFIG_MAC80211_DEBUGFS
	struct dentry *dbg_stats;
#endif
};

struct ssv_minstrel_priv {
	bool has_mrr;
	unsigned int cw_min;
	unsigned int cw_max;
	unsigned int max_rates;
	unsigned int max_retry;
	unsigned int segment_size;
	unsigned int update_interval;
	unsigned int lookaround_rate;
	unsigned int lookaround_rate_mrr;
    u8 cck_rates[4];
};

int minstrel_ewma(int old, int new, int weight);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
int ieee80211_frame_duration(enum nl80211_band band, size_t len,
			int rate, int erp, int short_preamble);
#else
int ieee80211_frame_duration(enum ieee80211_band band, size_t len,
			int rate, int erp, int short_preamble);
#endif
void ssv_minstrel_set_fix_data_rate(struct ssv_softc *sc, 
            struct ssv_minstrel_sta_priv *minstrel_sta_priv, struct ieee80211_tx_rate *ar); 
int ssv6xxx_minstrel_rate_control_register(void);
void ssv6xxx_minstrel_rate_control_unregister(void);
#endif
