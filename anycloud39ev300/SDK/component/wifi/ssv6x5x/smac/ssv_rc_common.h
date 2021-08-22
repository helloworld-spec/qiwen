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

#ifndef _SSV_RC_COM_H_
#define _SSV_RC_COM_H_

#define SSV_RC_MAX_STA                      8
#define MCS_GROUP_RATES                     8
#define SSV_HT_RATE_MAX                     8

#define TDIFS 34
#define TSLOT 9


/**
* Define flags for rc_flags:
*/
#define RC_FLAG_INVALID                 0x00000001
#define RC_FLAG_LEGACY                  0x00000002
#define RC_FLAG_HT                      0x00000004
#define RC_FLAG_HT_SGI                  0x00000008
#define RC_FLAG_HT_GF                   0x00000010
#define RC_FLAG_SHORT_PREAMBLE          0x00000020



enum ssv6xxx_rc_phy_type {
    WLAN_RC_PHY_CCK,
    WLAN_RC_PHY_OFDM,
    WLAN_RC_PHY_HT_20_SS_LGI,
    WLAN_RC_PHY_HT_20_SS_SGI,
    WLAN_RC_PHY_HT_20_SS_GF,
};



#define RATE_TABLE_SIZE             39




/**
* 
*/
#define RC_STA_VALID                    0x00000001
#define RC_STA_CAP_HT                   0x00000002
#define RC_STA_CAP_GF                   0x00000004
#define RC_STA_CAP_SGI_20               0x00000008
#define RC_STA_CAP_SHORT_PREAMBLE       0x00000010


#define SSV62XX_G_RATE_INDEX                    7
#define SSV62XX_RATE_MCS_INDEX                  15
#define SSV62XX_RATE_MCS_LGI_INDEX              15
#define SSV62XX_RATE_MCS_SGI_INDEX              23
#define SSV62XX_RATE_MCS_GREENFIELD_INDEX       31

/**
*
*/
enum ssv_rc_rate_type {
    RC_TYPE_B_ONLY=0,
    RC_TYPE_LEGACY_GB,
    RC_TYPE_SGI_20,
    RC_TYPE_LGI_20,
    RC_TYPE_HT_SGI_20,
    RC_TYPE_HT_LGI_20,
    RC_TYPE_HT_GF,
    RC_TYPE_MAX,
};

/**
* struct ssv_rate_info - ssv6xxx rate control infomation.
* This structure is used to map mac80211 rate to ssv6xxx
* rate table index.
*/
struct ssv_rate_info {
    int crate_kbps;
    int crate_hw_idx;
    int drate_kbps;
    int drate_hw_idx;
    u32 d_flags;
    u32 c_flags;
};

/**
* struct ssv_rc_rate - ssv6xxx rate control table entry.
* There are totally 39 rates for ssv6xxx wifi device. Each entry
* maps uniquely to ssv6xxx hardware rate.
*
* @ rc_flags:
* @ phy_type:
* @ rate_kbps:
* @ dot11_rate_idx:
* @ ctrl_rate_idx:
* @ hw_rate_idx:
*
*/
struct ssv_rc_rate {
    u32     rc_flags;
    u16     phy_type;
    u32     rate_kbps;
    u8      dot11_rate_idx;
    u8      ctrl_rate_idx;
    u8      hw_rate_idx;
    u8      arith_shift;
    u8      target_pf;
};

struct rc_pid_sta_info {
    //unsigned long last_change;
    unsigned long last_sample;
    unsigned long last_report;

    u16 tx_num_failed;
    u16 tx_num_xmit;

    u8 probe_report_flag;
    u8 probe_wating_times;

    u8 real_hw_index;
    int txrate_idx;

    /* Last framed failes percentage sample. */
    u8 last_pf;

    /* Average failed frames percentage error (i.e. actual vs. target
	    * percentage), scaled by RC_PID_SMOOTHING. This value is computed
	    * using using an exponential weighted average technique:
	    *
	    *           (RC_PID_SMOOTHING - 1) * err_avg_old + err
	    * err_avg = ------------------------------------------
	    *                       RC_PID_SMOOTHING
	    *
	    * w    u16 atx_num_failed;
    u16 atx_num_xmit;here err_avg is the new approximation, err_avg_old the previous one
	    * and err is the error w.r.t. to the current failed frames percentage
	    * sample. Note that the bigger RC_PID_SMOOTHING the more weight is
	    * given to the previous estimate, resulting in smoother behavior (i.e.
	    * corresponding to a longer integration window).
	    *
	    * For computation, we actually don't use the above formula, but this
	    * one:
	    *
	    * err_avg_scaled = err_avg_old_scaled - err_avg_old + err
	    *
	    * where:
	    * 	err_avg_scaled = err * RC_PID_SMOOTHING
	    * 	err_avg_old_scaled = err_avg_old * RC_PID_SMOOTHING
	    *
	    * This avoids floating point numbers and the per_failed_old value can
	    * easily be obtained by shifting per_failed_old_scaled right by
	    * RC_PID_SMOOTHING_SHIFT.
	    */
    s32 err_avg_sc;



    /* Sharpening needed. */
    //u8 sharp_cnt;
    int last_dlr;
    //int fail_probes; 
    u8 feedback_probes;
    u8 monitoring;
    u8 oldrate;
    //int n_rates;
    u8 tmp_rate_idx;
    u8 probe_cnt;
};



/* Algorithm parameters. We keep them on a per-algorithm approach, so they can
 * be tuned individually for each interface.
 */
struct rc_pid_rateinfo {
	/* Did we do any measurement on this rate? */
	//bool valid;
    u16 rc_index;

    u16 index;

	/* Comparison with the lowest rate. */
	s32 diff;

	//int bitrate;
	u16 perfect_tx_time;
	u32 throughput;

	unsigned long this_attempt;
	unsigned long this_success;
	unsigned long this_fail;
	u64 attempt;
	u64 success;
	u64 fail;
};



struct rc_pid_info {

	/* The failed frames percentage target. */
	unsigned int target;

	/* Rate at which failed frames percentage is sampled in 0.001s. */
	//unsigned int sampling_period;
#if 0
	/* P, I and D coefficients. */
	u8 coeff_p;
	u8 coeff_i;
	u8 coeff_d;

	/* Exponential averaging shift. */
	u8 smoothing_shift;

	/* Sharpening factor and duration. */
	u8 sharpen_factor;
	u8 sharpen_duration;

	/* Normalization offset. */
	u8 norm_offset;
#endif
	/* Index of the last used rate. */
	int oldrate;

	/* Rates information. */
	struct rc_pid_rateinfo rinfo[12];
};

struct mcs_group {
    //u32 flags;
    //unsigned int streams;
    unsigned int duration[MCS_GROUP_RATES];
};

struct minstrel_rate_stats {
    u16 rc_index;
    /* current / last sampling period attempts/success counters */
    unsigned int attempts, last_attempts;
    unsigned int success, last_success;

    /* total attempts/success counters */
    u64 att_hist, succ_hist;

    /* current throughput */
    unsigned int cur_tp;

    /* packet delivery probabilities */
    unsigned int cur_prob, probability;

    /* maximum retry counts */
    unsigned int retry_count;
    unsigned int retry_count_rtscts;

    //bool retry_updated;/* Sampling period for measuring percentage of failed frames in ms. */
    u8 sample_skipped;
};

struct minstrel_mcs_group_data {
    u8 index;
    u8 column;

    /* bitfield of supported MCS rates of this group */
    //u8 supported;

    /* selected primary rates */
    unsigned int max_tp_rate;
    unsigned int max_tp_rate2;
    unsigned int max_prob_rate;

    /* MCS rate statistics */
    struct minstrel_rate_stats rates[MCS_GROUP_RATES];
};

struct ssv62xx_ht {
    /* ampdu length (average, per sampling interval) */
    unsigned int ampdu_len;
    unsigned int ampdu_packets;

    /* ampdu length (EWMA) */
    unsigned int avg_ampdu_len;

    /* best throughput rate */
    unsigned int max_tp_rate;

    /* second best throughput rate */
    unsigned int max_tp_rate2;

    /* best probability rate */
    unsigned int max_prob_rate;

    int first_try_count;
    int second_try_count;
    int other_try_count;

    /* time of last status update */
    unsigned long stats_update;

    /* overhead time in usec for each frame */
    unsigned int overhead;
    unsigned int overhead_rtscts;

    unsigned int total_packets;
    unsigned int sample_packets;

    /* tx flags to add for frames for this sta */
    //u32 tx_flags;

    u8 sample_wait;
    u8 sample_tries;
    u8 sample_count;
    u8 sample_slow;

    /* current MCS group to be sampled */
    //u8 sample_group;

    /* MCS rate group info and statistics */
    struct minstrel_mcs_group_data groups;
};


/**
* struct ssv_sta_rc_info - per station rate control information.
*
* @ rc_valid: valid or invalid entry.
* @ rc_type: defined as enum ssv_rc_rate_type.
* @ num_rate: the max number of rates to choose.
* @ rc_wsid: the hardware wsid value.
* @ supp_rates:
* @ pinfo:
* @ spinfo:
*/
struct ssv_sta_rc_info {
    u8 rc_valid;
    u8 rc_type;
    u8 rc_num_rate;
    s8 rc_wsid;

    u8 ht_rc_type;
    u8 is_ht;
    u32 rc_supp_rates;
    u32 ht_supp_rates;

    struct rc_pid_info pinfo;
    struct rc_pid_sta_info spinfo;



    struct ssv62xx_ht ht;
};

/**
* struct ssv_rate_ctrl - rate control structure.
*
*/
struct ssv_rate_ctrl {
    struct ssv_rc_rate *rc_table;
    struct ssv_sta_rc_info sta_rc_info[SSV_RC_MAX_STA];
};


//default 100*5 125 *4ms 
#define HT_RC_UPDATE_INTERVAL             1000

//#define DISABLE_RATE_CONTROL_SAMPLE
//#define RATE_CONTROL_DEBUG          1

//#define RATE_CONTROL_PARAMETER_DEBUG   1
//#define RATE_CONTROL_PERCENTAGE_TRACE    1
//#define RATE_CONTROL_STUPID_DEBUG   1

//#define RATE_CONTROL_HT_PARAMETER_DEBUG   1
//#define RATE_CONTROL_HT_PERCENTAGE_TRACE    1
//#define RATE_CONTROL_HT_STUPID_DEBUG   1

#endif /* _SSV_RC_H_ */

