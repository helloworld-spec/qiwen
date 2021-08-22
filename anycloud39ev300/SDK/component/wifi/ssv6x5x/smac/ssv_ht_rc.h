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

#ifndef _SSV_RC_HT_H_
#define _SSV_RC_HT_H_

#include "ssv_rc_common.h"

/* scaled fraction values */
#define MINSTREL_SCALE  16
#define MINSTREL_FRAC(val, div) (((val) << MINSTREL_SCALE) / div)
#define MINSTREL_TRUNC(val) ((val) >> MINSTREL_SCALE)



/* Sampling period for measuring percentage of failed frames in ms. */
//default 125ms 
#define SSV_RC_HT_INTERVAL             100

s32 ssv62xx_ht_rate_update(struct sk_buff *skb, struct ssv_softc *sc, struct fw_rc_retry_params *ar);
void ssv62xx_ht_rc_caps(const u16 ssv6xxx_rc_rate_set[RC_TYPE_MAX][13],struct ssv_sta_rc_info *rc_sta);
void ssv6xxx_ht_report_handler(struct ssv_softc *sc,struct sk_buff *skb,struct ssv_sta_rc_info *rc_sta);

#endif /* _SSV_RC_H_ */

