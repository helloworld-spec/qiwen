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
#ifndef _HALSSV6051_H_
#define _HALSSV6051_H_

#define SSV6051_NUM_HW_STA  2

#define PBUF_BASE_ADDR	            0x80000000
#define PBUF_ADDR_SHIFT	            16

#define PBUF_MapPkttoID(_pkt)		(((u32)_pkt&0x0FFF0000)>>PBUF_ADDR_SHIFT)	
#define PBUF_MapIDtoPkt(_id)		(PBUF_BASE_ADDR|((_id)<<PBUF_ADDR_SHIFT))
/**
* struct ssv6051_tx_desc - ssv6051 tx frame descriptor.
* This descriptor is shared with ssv6051 hardware and driver.
*/
struct ssv6051_tx_desc
{
    /* The definition of WORD_1: */
    u32             len:16;
    u32             c_type:3;
    u32             f80211:1;
    u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
    u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
    u32             use_4addr:1;
    u32             RSVD_0:3;//used for rate control report event.
    u32             bc_que:1;
    u32             security:1;
    u32             more_data:1;
    u32             stype_b5b4:2;
    u32             extra_info:1;   /* 0: don't trap to cpu after parsing, 1: trap to cpu after parsing */

    /* The definition of WORD_2: */
    u32             fCmd;

    /* The definition of WORD_3: */
    u32             hdr_offset:8;
    u32             frag:1;
    u32             unicast:1;
    u32             hdr_len:6;
    u32             tx_report:1;
    u32             tx_burst:1;     /* 0: normal, 1: burst tx */
    u32             ack_policy:2;   /* See Table 8-6, IEEE 802.11 Spec. 2012 */
    u32             aggregation:1;
    u32             RSVD_1:3;//Used for AMPDU retry counter
    u32             do_rts_cts:2;   /* 0: no RTS/CTS, 1: need RTS/CTS */
                                    /* 2: CTS protection, 3: RSVD */
    u32             reason:6;

    /* The definition of WORD_4: */
    u32             payload_offset:8;
    u32             RSVD_4:7;
    u32             RSVD_2:1;
    u32             fCmdIdx:3;
    u32             wsid:4;
    u32             txq_idx:3;
    u32             TxF_ID:6;

    /* The definition of WORD_5: */
    u32             rts_cts_nav:16;
    u32             frame_consume_time:10;  //32 units
    u32             crate_idx:6;

    /* The definition of WORD_6: */
    u32             drate_idx:6;
    u32             dl_length:12;
    u32             RSVD_3:14;
    /* The definition of WORD_7~15: */
    u32             RESERVED[8];
    /* The definition of WORD_16~20: */
    struct fw_rc_retry_params rc_params[SSV62XX_TX_MAX_RATES];
};

/**
* struct ssv6051_rx_desc - ssv6051 rx frame descriptor.
* This descriptor is shared with ssv6051 hardware and driver.
*/
struct ssv6051_rx_desc
{
    /* The definition of WORD_1: */
    u32             len:16;
    u32             c_type:3;
    u32             f80211:1;
    u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
    u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
    u32             use_4addr:1;
    u32             l3cs_err:1;
    u32             l4cs_err:1;
    u32             align2:1;
    u32             RSVD_0:2;
    u32             psm:1;
    u32             stype_b5b4:2;
    u32             extra_info:1;  

    /* The definition of WORD_2: */
    u32             edca0_used:4;
    u32             edca1_used:5;
    u32             edca2_used:5;
    u32             edca3_used:5;
    u32             mng_used:4;
    u32             tx_page_used:9;

    /* The definition of WORD_3: */
    u32             hdr_offset:8;
    u32             frag:1;
    u32             unicast:1;
    u32             hdr_len:6;
    u32             RxResult:8;
    u32             wildcard_bssid:1;
    u32             RSVD_1:1;
    u32             reason:6;

    /* The definition of WORD_4: */
    u32             payload_offset:8;
    u32             tx_id_used:8;
    u32             fCmdIdx:3;
    u32             wsid:4;
    u32             RSVD_3:3;
    u32             rate_idx:6;

};

struct ssv6051_rxphy_info {
    /* WORD 1: */
    u32             len:16;
    u32             rsvd0:16;

    /* WORD 2: */
    u32             mode:3;
    u32             ch_bw:3;
    u32             preamble:1;
    u32             ht_short_gi:1;
    u32             rate:7;
    u32             rsvd1:1;
    u32             smoothing:1;
    u32             no_sounding:1;
    u32             aggregate:1;
    u32             stbc:2;
    u32             fec:1;
    u32             n_ess:2;
    u32             rsvd2:8;

    /* WORD 3: */
    u32             l_length:12;
    u32             l_rate:3;
    u32             rsvd3:17;

    /* WORD 4: */
    u32             rsvd4;

    /* WORD 5: G, N mode only */
    u32             rpci:8;     /* RSSI */
    u32             snr:8;
    u32             service:16;

};

struct ssv6051_rxphy_info_padding {

/* WORD 1: for B, G, N mode */
u32             rpci:8;     /* RSSI */
u32             snr:8;
u32             RSVD:16;
};
#endif