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

#ifndef _SSV_RC_H_
#define _SSV_RC_H_

#include "ssv_rc_common.h"



#define RC_PID_REPORT_INTERVAL      40

/* Sampling period for measuring percentage of failed frames in ms. */
//default 125ms 
#define RC_PID_INTERVAL             125

/* Arithmetic right shift for positive and negative values for ISO C. */
#define RC_PID_DO_ARITH_RIGHT_SHIFT(x, y) \
	((x) < 0 ? -((-(x)) >> (y)) : (x) >> (y))

/* Fixed point arithmetic shifting amount. */
//Be care this value affects adj
//#define RC_PID_ARITH_SHIFT        8

/* Target failed frames rate for the PID controller. NB: This effectively gives
 * maximum failed frames percentage we're willing to accept. If the wireless
 * link quality is good, the controller will fail to adjust failed frames
 * percentage to the target. This is intentional.
 */
//Default value 14
//#define RC_PID_TARGET_PF      14

/* Rate behaviour normalization quantity over time. */
#define RC_PID_NORM_OFFSET      3

/* Exponential averaging smoothness (used for I part of PID controller) */
#define RC_PID_SMOOTHING_SHIFT  1
#define RC_PID_SMOOTHING        (1 << RC_PID_SMOOTHING_SHIFT)

/* Proportional PID component coefficient.[15] */
#define RC_PID_COEFF_P          15
/* Integral PID component coefficient.[9] */
#define RC_PID_COEFF_I          15
/* Derivative PID component coefficient.[15] */
#define RC_PID_COEFF_D          5

#define MAXPROBES 3

#define SSV_DRATE_IDX   (2)
#define SSV_CRATE_IDX   (3)

struct ssv_softc;

struct ssv_rc_rate *ssv6xxx_rc_get_rate(int rc_index);
        
#ifdef RATE_CONTROL_REALTIME_UPDATA
u8 ssv6xxx_rc_hw_rate_update_check(struct sk_buff *skb, struct ssv_softc *sc, u32 do_rts_cts);
#endif
#ifndef SSV_SUPPORT_HAL
void ssv6xxx_rc_mac80211_rate_idx(struct ssv_softc *sc, int hw_rate_idx, 
                struct ieee80211_rx_status *rxs);
void ssv6xxx_rc_hw_reset(struct ssv_softc *sc, int rc_idx, int hwidx);
void ssv6xxx_rc_rx_data_handler(struct ieee80211_hw *hw, struct sk_buff *skb, u32 rate_index);
void ssv6xxx_rc_update_basic_rate(struct ssv_softc *sc, u32 basic_rates);
#endif
void ssv6xxx_legacy_report_handler(struct ssv_softc *sc,struct sk_buff *skb,struct ssv_sta_rc_info *rc_sta);

#if (!defined(SSV_SUPPORT_HAL)||defined(SSV_SUPPORT_SSV6051))
int ssv6xxx_pid_rate_control_register(void);
void ssv6xxx_pid_rate_control_unregister(void);

void ssv6xxx_rc_hw_rate_idx(struct ssv_softc *sc,
            struct ieee80211_tx_info *info, struct ssv_rate_info *sr);
#endif

int pide_frame_duration(size_t len, int rate, int short_preamble, int flags);


#endif /* _SSV_RC_H_ */

