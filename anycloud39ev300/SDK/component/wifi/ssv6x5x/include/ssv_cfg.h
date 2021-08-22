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

#ifndef _SSV_CFG_H_
#define _SSV_CFG_H_



/**
* SSV6200 Hardware Capabilities:
*
* @ SSV6200_HW_CAP_HT: hardware supports HT capability.
* @ SSV6200_HW_CAP_LDPC:
* @ SSV6200_HW_CAP_2GHZ:
* @ SSV6200_HW_CAP_5GHZ:
* @ SSV6200_HW_CAP_DFS:
* @ SSV6200_HW_CAP_SECUR:
*/
#define SSV6200_HW_CAP_HT                   0x00000001
#define SSV6200_HW_CAP_GF                   0x00000002
#define SSV6200_HW_CAP_2GHZ                 0x00000004
#define SSV6200_HW_CAP_5GHZ                 0x00000008
#define SSV6200_HW_CAP_SECURITY             0x00000010
#define SSV6200_HW_CAP_SGI                  0x00000020
#define SSV6200_HW_CAP_HT40                 0x00000040
#define SSV6200_HW_CAP_AP                   0x00000080
#define SSV6200_HW_CAP_P2P                  0x00000100
#define SSV6200_HW_CAP_AMPDU_RX             0x00000200
#define SSV6200_HW_CAP_AMPDU_TX             0x00000400
#define SSV6200_HW_CAP_TDLS                 0x00000800
#define SSV6200_HW_CAP_STBC                 0x00001000
#define SSV6200_HW_CAP_HCI_RX_AGGR          0x00002000
#define SSV6200_HW_CAP_BEACON               0x00004000

#define EXTERNEL_CONFIG_SUPPORT             64


/* define USB HW RESOURCE CHECK*/
#define USB_HW_RESOURCE_CHK_NONE            0x00000000
#define USB_HW_RESOURCE_CHK_TXID            0x00000001
#define USB_HW_RESOURCE_CHK_TXPAGE          0x00000002
#define USB_HW_RESOURCE_CHK_SCAN            0x00000004
#define USB_HW_RESOURCE_CHK_FORCE_OFF       0x00000008

/* define Online Reset */
#define ONLINE_RESET_ENABLE                 0x00000100
#define ONLINE_RESET_EDCA_THRESHOLD_MASK    0x000000ff
#define ONLINE_RESET_EDCA_THRESHOLD_SFT     0

/* patch structure for N/B rate control*/
struct rc_setting{
    u16     aging_period;
    u16     target_success_67;
    u16     target_success_5;
    u16     target_success_4;
    u16     target_success;
    u16     up_pr;
    u16     up_pr3;
    u16     up_pr4;
    u16     up_pr5;
    u16     up_pr6;
    u16     forbid;
    u16     forbid3;
    u16     forbid4;
    u16     forbid5;
    u16     forbid6;
    u16     sample_pr_4;
    u16     sample_pr_5;
    u16     force_sample_pr;
};

/***************************************************************************
 * ssv6xxx driver configuration
 * 
 * Note:
 *   1. Use u32 for flag because the compiled code is more efficient then using
 *      u8.
 ***************************************************************************/
struct ssv6xxx_cfg {
    /**
     * ssv6200 hardware capabilities sets.
     */
    u32     hw_caps;

    /**
     * The default channel once the wifi system is up.
     */
    u32     def_chan;

    //0-26M 1-40M 2-24M
    u32     crystal_type;
    //(DCDC-0 LDO-1)
    u32     volt_regulator;
    u32     force_chip_identity;

    u32     ignore_firmware_version;
	
    /**
     * The mac address of Wifi STA .
     */
    u8      maddr[2][6];
    u32     n_maddr;
    // Force to use sw cipher only
    u32     use_sw_cipher;
    // Force to use WPA2 only such that all virtual interfaces use hardware security.
    u32     use_wpa2_only;
    // online reset for rx stuck
    // bit[8] disable/enable, bit[0~7] edca threshold
    u32     online_reset;
    // detect tx stuck
    bool    tx_stuck_detect; 

    //E-fuse configuration
    u32     r_calbration_result;
    u32     sar_result;
    u32     crystal_frequency_offset;
    //u16 iq_calbration_result;
    u32     tx_power_index_1;
    u32     tx_power_index_2;
    u32     chip_identity;
    u32     rate_table_1;
    u32     rate_table_2;

    u32     wifi_tx_gain_level_gn;
    u32     wifi_tx_gain_level_b;

    u32     configuration[EXTERNEL_CONFIG_SUPPORT+1][2];

    //Firmware path
    //Read from wifi.cfg
    u8      firmware_path[128];
    u8      external_firmware_name[128];

    //MAC address
    //Read from wifi.cfg
    u8      mac_address_path[128];
    u8      mac_output_path[128];
    u32     ignore_efuse_mac;
    u32     mac_address_mode;
    
    u32     beacon_rssi_minimal;
   
    // turismo minstrel_ht_rc support cck rates
    u32     rc_ht_support_cck;
    // turismo fix rate control
    u32     auto_rate_enable;
    u32     rc_rate_idx_set;
    u32     rc_retry_set;
    u32     rc_mf;
    u32     rc_long_short;
    u32     rc_ht40;
    u32     rc_phy_mode;
	u32		rc_log;

	//tx buff threshold
	u32 	tx_id_threshold;
	u32 	tx_page_threshold;
	
	//max rx aggr size
	u32     max_rx_aggr_size;
    
    bool    rx_burstread;
    //HW rx aggregation
    u32     hw_rx_agg_cnt;
    bool    hw_rx_agg_method_3;
    u32     hw_rx_agg_timer_reload;

    //SW take care of the HW resource for USB
    u32     usb_hw_resource;    // bit 0: check hci resource, bit 1: check tx page resouce.

    // turismo lpbk 
    u32     lpbk_pkt_cnt;
    u32     lpbk_type;
    u32     lpbk_sec;
    u32     lpbk_mode;

    // clock source
    bool    clk_src_80m;
    
    // rts threshold len
    u32     rts_thres_len;
    
    // cci
    u32     cci;

    //hwq limit
    u32     bk_txq_size;
    u32     be_txq_size;
    u32     vi_txq_size;
    u32     vo_txq_size;
    u32     manage_txq_size;
    
    //green tx
    u32     greentx;

    //rc_settings
    struct rc_setting   rc_setting;

    // directly ack threshold
    u32 directly_ack_low_threshold;
    u32 directly_ack_high_threshold;
    // performance tuning
    u32 txrxboost_prio;
    u32 txrxboost_low_threshold;
    u32 txrxboost_high_threshold;
    u32 rx_threshold;
};

#endif /* _SSV_CFG_H_ */

