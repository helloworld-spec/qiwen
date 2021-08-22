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

#ifndef _SSV_PKTDEF_H_
#define _SSV_PKTDEF_H_

typedef struct _ssv6200_tx_desc ssv6200_tx_desc;
typedef struct _ssv6200_rx_desc ssv6200_rx_desc;
typedef struct _ssv6200_txphy_info ssv6200_txphy_info;
typedef struct _ssv6200_rxphy_info ssv6200_rxphy_info;


#define ENABLE                          0x1
#define DISABLE                         0x0

#define ADDRESS_OFFSET           16
#define HW_ID_OFFSET            7

/*
    Reference with firmware
*/

/* Hardware Offload Engine ID */
#define M_ENG_CPU                       0x00
#define M_ENG_HWHCI                     0x01
#define M_ENG_EMPTY                     0x02
#define M_ENG_ENCRYPT                   0x03
#define M_ENG_MACRX                     0x04  
#define M_ENG_MIC                       0x05
#define M_ENG_TX_EDCA0                  0x06
#define M_ENG_TX_EDCA1                  0x07
#define M_ENG_TX_EDCA2                  0x08
#define M_ENG_TX_EDCA3                  0x09
#define M_ENG_TX_EDCA4                  0x0A
#define M_ENG_ENCRYPT_SEC               0x0B
#define M_ENG_MIC_SEC                   0x0C
#define M_ENG_RESERVED_1                0x0D
#define M_ENG_TX_EDCA5                  0x0E
#define M_ENG_TRASH_CAN                 0x0F
#define M_ENG_MAX                      (M_ENG_TRASH_CAN+1)


/* Software Engine ID: */
#define M_CPU_HWENG                     0x00
#define M_CPU_TXL34CS                   0x01
#define M_CPU_RXL34CS                   0x02
#define M_CPU_DEFRAG                    0x03
#define M_CPU_EDCATX                    0x04
#define M_CPU_RXDATA                    0x05
#define M_CPU_RXMGMT                    0x06
#define M_CPU_RXCTRL                    0x07
#define M_CPU_FRAG                      0x08


/**
 *  The flag definition for c_type (command type) field of PKTInfo:
 *
 *      @ M0_TXREQ:         
 *      @ M1_TXREQ
 *      @ M2_TXREQ
 *      @ M0_RXEVENT
 *      @ M1_RXEVENT
 *      @ HOST_CMD
 *      @ HOST_EVENT
 *
 */
#define M0_TXREQ                0
#define M1_TXREQ                1
#define M2_TXREQ                2
#define M0_RXEVENT              3
#define M2_RXEVENT              4
#define HOST_CMD                5
#define HOST_EVENT              6
#define TEST_CMD                7

#define M0_HDR_LEN		4
#define M1_HDR_LEN		8
#define M2_HDR_LEN		16

#define RX_M0_HDR_LEN		24


/**
 *
 *  Offset Table (register):
 *
 *    c_type            hdr_len     
 *  ----------     ----------
 *  M0_TXREQ         8-bytes
 *  M1_TXREQ         12-bytes   
 *  M2_TXREQ         sizeof(PKT_TXInfo)
 *  M0_RXEVENT      
 *  M1_RXEVENT
 *  HOST_CMD
 *  HOSt_EVENT
 *
 *
 */


/*  WMM_Specification_1-1 : Table 14  802.1D Priority to AC mappings

    UP		Access Category
    -------------------------
    1, 2	AC_BK
    0, 3	AC_BE
    4, 5	AC_VI
    6, 7	AC_VO
*/
#define	AC_BK	 0
#define	AC_BE	 1
#define	AC_VI	 2
#define	AC_VO	 3

#define GET_PARSE_RESULT_ID(p)		((((p)->parse_result) >> 4) & 0x000f)
#define GET_PARSE_RESULT_TBL(p)		((((p)->parse_result)) & 0x000f)

/**
 * Define constants for do_rts_cts field of PKT_TxInfo structure 
 * 
 * @ TX_NO_RTS_CTS
 * @ TX_RTS_CTS
 * @ TX_CTS
 */
#define TX_NO_RTS_CTS                   0
#define TX_RTS_CTS                      1
#define TX_CTS                          2

#define SSV6XXX_RX_DESC_LEN                     \
        (sizeof(struct ssv6200_rx_desc) +       \
         sizeof(struct ssv6200_rxphy_info))
#define SSV6XXX_TX_DESC_LEN                     \
        (sizeof(struct ssv6200_tx_desc) + 0)    \

#define SSV6XXX_RX_PHY_LEN 28 //byte
#define SSV6XXX_RX_REV_LEN 36 //byte
#define SSV6XXX_TX_PHY_LEN 28 //byte
#define SSV6XXX_TX_REV_LEN 24 //byte

/**
* struct ssv6200_tx_desc - ssv6200 tx frame descriptor.
* This descriptor is shared with ssv6200 hardware and driver.
*/

struct _ssv6200_tx_desc
{
    unsigned int    len:16; //phytxdesc_word0[15:00]
    unsigned int    c_type:3; //phytxdesc_word0[18:16]
    unsigned int    f80211:1; //phytxdesc_word0[19:19]
    unsigned int    qos:1; //phytxdesc_word0[20:20]
    unsigned int    ht:1; //phytxdesc_word0[21:21]
    unsigned int    use_4addr:1; //phytxdesc_word0[22:22]
    unsigned int    RSVDTX_0:1; //phytxdesc_word0[23:23]
    unsigned int    padding:2; //phytxdesc_word0[25:24]
    unsigned int    bc_que:1; //phytxdesc_word0[26:26]
    unsigned int    security:1; //phytxdesc_word0[27:27]
    unsigned int    more_data:1; //phytxdesc_word0[28:28]
    unsigned int    sub_type10:2; //phytxdesc_word0[30:29]
    unsigned int    extra_info:1; //phytxdesc_word0[31:31]

    unsigned int    fcmd; //phytxdesc_word1[31:00]

    unsigned int    hdr_offset:8; //phytxdesc_word2[07:00]
    unsigned int    RSVDTX_2a:1; //phytxdesc_word2[08:08]
    unsigned int    unicast:1; //phytxdesc_word2[09:09]
    unsigned int    hdr_len:6; //phytxdesc_word2[15:10]
    unsigned int    no_pkt_buf_reduction:1; //phytxdesc_word2[16:16]
    unsigned int    tx_burst:1; //phytxdesc_word2[17:17]
    unsigned int    ack_policy:2; //phytxdesc_word2[19:18]
    unsigned int    ampdu_tx_mode:2; //phytxdesc_word2[21:20]
    unsigned int    RSVDTX_1:1; //phytxdesc_word2[23:22]
    unsigned int    is_rate_stat_sample_pkt:1; 
    unsigned int    bssidx:2;
    unsigned int    reason:6; //phytxdesc_word2[31:26]

    unsigned int    payload_offset:8; //phytxdesc_word3[07:00]
    unsigned int    tx_pkt_run_no:8; //phytxdesc_word3[15:08]
    unsigned int    fcmdidx:3; //phytxdesc_word3[18:16]
    unsigned int    wsid:4; //phytxdesc_word3[22:19]
    unsigned int    txq_idx:3; //phytxdesc_word3[25:23]
    unsigned int    txf_id:6; //phytxdesc_word3[31:26]

    unsigned int    rateidx1_data_duration:16; //phytxdesc_word4[15:00]
    unsigned int    rateidx2_data_duration:16; //phytxdesc_word4[31:16]

    unsigned int    rateidx3_data_duration:16; //phytxdesc_word5[15:00]
    unsigned int    RSVDTX_05:2; //phytxdesc_word5[17:16]
    unsigned int    rate_rpt_mode:2; //phytxdesc_word5[19:18]
    unsigned int    ampdutxv1p3_tx_ssn:12; //phytxdesc_word5[31:20]

    unsigned int    rateidx0_main_rate_idx:8; //phytxdesc_word6[07:00]
    unsigned int    rateidx0_ctrl_rate_idx:8; //phytxdesc_word6[15:08]
    unsigned int    rateidx0_ctrl_duration:16; //phytxdesc_word6[31:16]

    unsigned int    rateidx0_phy_11n_htmix_l_length:12; //phytxdesc_word7[11:00]
    unsigned int    rateidx0_trycnt:4; //phytxdesc_word7[15:12]
    unsigned int    rateidx0_ack_policy:2; //phytxdesc_word7[17:16]
    unsigned int    rateidx0_do_rts_cts:2; //phytxdesc_word7[19:18]
    unsigned int    rateidx0_is_last_rate:1; //phytxdesc_word7[20]
    unsigned int    RSVDTX_07b:1; //phytxdesc_word7[23:20]
    unsigned int    rateidx0_rpt_result:2;
    unsigned int    rateidx0_rpt_trycnt:4; //phytxdesc_word7[27:24]
    unsigned int    rateidx0_rpt_noctstrycnt:4; //phytxdesc_word7[31:28]

    unsigned int    rateidx1_main_rate_idx:8; //phytxdesc_word8[07:00]
    unsigned int    rateidx1_ctrl_rate_idx:8; //phytxdesc_word8[15:08]
    unsigned int    rateidx1_ctrl_duration:16; //phytxdesc_word8[31:16]

    unsigned int    rateidx1_phy_11n_htmix_l_length:12; //phytxdesc_word9[11:00]
    unsigned int    rateidx1_trycnt:4; //phytxdesc_word9[15:12]
    unsigned int    rateidx1_ack_policy:2; //phytxdesc_word9[17:16]
    unsigned int    rateidx1_do_rts_cts:2; //phytxdesc_word9[19:18]
    unsigned int    rateidx1_is_last_rate:1; //phytxdesc_word9[20]
    unsigned int    RSVDTX_09b:1; //phytxdesc_word9[23:20]
    unsigned int    rateidx1_rpt_result:2;
    unsigned int    rateidx1_rpt_trycnt:4; //phytxdesc_word9[27:24]
    unsigned int    rateidx1_rpt_noctstrycnt:4; //phytxdesc_word9[31:28]

    unsigned int    rateidx2_main_rate_idx:8; //phytxdesc_word10[07:00]
    unsigned int    rateidx2_ctrl_rate_idx:8; //phytxdesc_word10[15:08]
    unsigned int    rateidx2_ctrl_duration:16; //phytxdesc_word10[31:16]

    unsigned int    rateidx2_phy_11n_htmix_l_length:12; //phytxdesc_word11[11:00]
    unsigned int    rateidx2_trycnt:4; //phytxdesc_word11[15:12]
    unsigned int    rateidx2_ack_policy:2; //phytxdesc_word11[17:16]
    unsigned int    rateidx2_do_rts_cts:2; //phytxdesc_word11[19:18]
    unsigned int    rateidx2_is_last_rate:1; //phytxdesc_word11[20]
    unsigned int    RSVDTX_11b:1; //phytxdesc_word11[23:20]
    unsigned int    rateidx2_rpt_result:2;
    unsigned int    rateidx2_rpt_trycnt:4; //phytxdesc_word11[27:24]
    unsigned int    rateidx2_rpt_noctstrycnt:4; //phytxdesc_word11[31:28]

    unsigned int    rateidx3_main_rate_idx:8; //phytxdesc_word12[07:00]
    unsigned int    rateidx3_ctrl_rate_idx:8; //phytxdesc_word12[15:08]
    unsigned int    rateidx3_ctrl_duration:16; //phytxdesc_word12[31:16]

    unsigned int    rateidx3_phy_11n_htmix_l_length:12; //phytxdesc_word13[11:00]
    unsigned int    rateidx3_trycnt:4; //phytxdesc_word13[15:12]
    unsigned int    rateidx3_ack_policy:2; //phytxdesc_word13[17:16]
    unsigned int    rateidx3_do_rts_cts:2; //phytxdesc_word13[19:18]
    unsigned int    rateidx3_is_last_rate:1; //phytxdesc_word13[20]
    unsigned int    RSVDTX_13b:1; //phytxdesc_word13[23:20]
    unsigned int    rateidx3_rpt_result:2; 
    unsigned int    rateidx3_rpt_trycnt:4; //phytxdesc_word13[27:24]
    unsigned int    rateidx3_rpt_noctstrycnt:4; //phytxdesc_word13[31:28]

    unsigned int    ampdutxv1p3_whole_pkt_length:16; //phytxdesc_word14[15:00]
    unsigned int    ampdutxv1p3_next_pkt_ptr:8; //phytxdesc_word14[23:16]
    unsigned int    ampdutxv1p3_last_pkt:1; //phytxdesc_word14[24:24]
    unsigned int    RSVDTX_14a:3; //phytxdesc_word14[29:25]
    unsigned int    amptutxv1p3_dmydelimiter_num:4;
 
    unsigned int    ampdutxv1p3_tx_bitmap_lw:32; //phytxdesc_word15[31:00]
    unsigned int    ampdutxv1p3_tx_bitmap_hw:32; //phytxdesc_word16[31:00]
    unsigned int    tx_scrt_pn_lw; 
    unsigned int    tx_scrt_pn_hw; 
    unsigned char   phy_info[4];

}__attribute__((packed));

struct _ssv6200_txphy_info {
    /* The definition of WORD_1: */
    unsigned int    Llength:8;
    unsigned int    Mlength:8;
    unsigned int    rate:8;
    unsigned int    smoothing:1;
    unsigned int    no_sounding:1;
    unsigned int    aggregation:1;
    unsigned int    stbc:2;
    unsigned int    fec:1;
    unsigned int    n_ess:2;
    /* The definition of WORD_2: */
    unsigned int    L_l_length:8;
    unsigned int    M_l_length:4;
    unsigned int    l_rate:3;
    unsigned int    cont_mode:1;
    unsigned int    tf_tx_power_level:8;
    unsigned int    bb_dac_scale:8;
    /* The definition of WORD_3: */
    unsigned int    L_tx_freq_offset:8;
    unsigned int    M_tx_freq_offset:8;
    unsigned int    L_service:8;
    unsigned int    M_service:8;

}__attribute__((packed));

/**
* struct ssv6200_rx_desc - ssv6200 rx frame descriptor.
* This descriptor is shared with ssv6200 hardware and driver.
*/
struct _ssv6200_rx_desc
{
    /* The definition of WORD_1: */
    unsigned int    len:16;
    unsigned int    c_type:3;
    unsigned int    f80211:1;
    unsigned int    qos:1;          /* 0: without qos control field, 1: with qos control field */
    unsigned int    ht:1;           /* 0: without ht control field, 1: with ht control field */
    unsigned int    use_4addr:1;
    unsigned int    RSVD_0:1;
    unsigned int    Running_No:4;
    unsigned int    psm:1;
    unsigned int    stype_b5b4:2;
    unsigned int    RSVD_1:1;

    /* The definition of WORD_2: */
    unsigned int    fCmd;

    /* The definition of WORD_3: */
    unsigned int    hdr_offset:8;
    unsigned int    frag:1;
    unsigned int    unicast:1;
    unsigned int    hdr_len:6;
    unsigned int    parse_result:8;
    unsigned int    bssidx:2;
    unsigned int    reason:6;

    /* The definition of WORD_4: */
    unsigned int    rsvdrx_3a:8;
    unsigned int    channel:8;
    unsigned int    fcmdidx:3;
    unsigned int    wsid:4;
    unsigned int    ravdrx_3b:8;

    unsigned char     phy_info[12];
    unsigned char     reserved[52];

}__attribute__((packed));

struct _ssv6200_rxphy_info {

    /* The definition of WORD_1: */
    unsigned int    Llength:8;
    unsigned int    Mlength:8;
    unsigned int    rate:8;
    unsigned int    smoothing:1;
    unsigned int    no_sounding:1;
    unsigned int    aggregation:1;
    unsigned int    stbc:2;
    unsigned int    fec:1;
    unsigned int    n_ess:2;
    /* The definition of WORD_2: */
    unsigned int    L_l_length:8;
    unsigned int    M_l_length:4;
    unsigned int    l_rate:3;
    unsigned int    reserved:1;
    unsigned int    rssi:8;
    unsigned int    snr:8;
    /* The definition of WORD_3: */
    unsigned int    L_rx_freq_offset:8;
    unsigned int    M_rx_freq_offset:8;
    unsigned int    L_service:8;
    unsigned int    M_service:8;

}__attribute__((packed));

struct ssv_hci_agg_hdr {
    unsigned int jmp_mpdu_len:16;
    unsigned int accu_rx_len:16;

    unsigned int RSVD0:15;
    unsigned int tx_page_remain:9;
    unsigned int tx_id_remain:8;

    unsigned int edca0:4;
    unsigned int edca1:5;
    unsigned int edca2:5;
    unsigned int edca3:5;
    unsigned int edca4:4;
    unsigned int edca5:5;
    unsigned int RSVD1:4;
}__attribute__((packed));
#define SIZE_HCI_AGGR_HDR   12

#endif	/* _SSV_PKTDEF_H_ */

