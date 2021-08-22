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

#ifndef _DEV_H_
#define _DEV_H_

#include <linux/version.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#ifdef CONFIG_SSV_SUPPORT_ANDROID
#include <linux/wakelock.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif //CONFIG_HAS_EARLYSUSPEND
#endif
#ifdef SSV_MAC80211
#include "ssv_mac80211.h"
#else
#include <net/mac80211.h>
#endif
#include "ampdu.h"
#include "ssv_rc_common.h"
#include "drv_comm.h"
#include "sec.h"
#include "p2p.h"
#include <linux/kthread.h>

#define SSV_DRVER_NAME "SSV WLAN driver"

// define RF mode
#define RF_MODE_SHUTDOWN	0
#define RF_MODE_STANDBY   1
#define RF_MODE_TRX_EN    2

// define CCI switch
#define CCI_CTL     0x1
#define CCI_DBG     0x2
#define CCI_P1      0x4
#define CCI_P2      0x8
#define CCI_SMART   0x10

// define for green Tx
#define GT_PWR_START_MASK   0xFF
#define GT_ENABLE           0x100
/* rx flow*/
enum ssv_rx_flow{
    RX_DATA_FLOW,
    RX_MGMT_FLOW,
    RX_CTRL_FLOW,
};

#ifdef CONFIG_PM
struct ssv_trap_data {
	u32 reason_trap0;
	u32 reason_trap1;
};
#endif

struct ssv_dbg_log {
	int size;
	int totalsize;
	char *data;
	char *top;
	char *tail;
	char *end;
};

struct ssv_cmd_data{
    char *ssv6xxx_result_buf;
    u32  rsbuf_len;
    u32  rsbuf_size;
    bool cmd_in_proc;
	bool log_to_ram;
	struct ssv_dbg_log dbg_log;
	struct proc_dir_entry *proc_dev_entry;
};

// debug print
void dbgprint(struct ssv_cmd_data *cmd_data, u32 log_ctrl, u32 log_id, const char *fmt,...); 
void ssv6xxx_hci_dbgprint(void *argc, u32 log_id, const char *fmt,...);

// for AMPDU
#define AMPDU_BA_FRAME_LEN        (68) // 28 BA + 36 RX DESC + 4 PHY PADDING
#define INDEX_PKT_BY_SSN(tid, ssn) \
    ((tid)->aggr_pkts[(ssn) % SSV_AMPDU_BA_WINDOW_SIZE])
#define MAX_CONCUR_AMPDU            64

#define ampdu_skb_hdr(skb)        ((struct ieee80211_hdr*)((u8*)((skb)->data)+AMPDU_DELIMITER_LEN))
#define ampdu_skb_ssn(skb)        ((ampdu_skb_hdr(skb)->seq_ctrl)>>SSV_SEQ_NUM_SHIFT)
#define ampdu_hdr_ssn(hdr)        ((hdr)->seq_ctrl>>SSV_SEQ_NUM_SHIFT)

#if 1
#define prn_aggr_dbg(_sc, fmt, ...) \
    do { \
        dbgprint(&(_sc)->cmd_data, (_sc)->log_ctrl, LOG_AMPDU_DBG, KERN_DEBUG fmt, ##__VA_ARGS__); \
    } while (0)
#else
#undef prn_aggr_dbg
#define prn_aggr_dbg(sc, fmt, ...)
#endif

#if 1
#define prn_aggr_err(_sc, fmt, ...) \
    do { \
        dbgprint(&(_sc)->cmd_data, (_sc)->log_ctrl, LOG_AMPDU_ERR, KERN_ERR fmt, ##__VA_ARGS__);\
    } while (0)
#else
#define prn_aggr_err(fmt, ...)
#endif

//
#define RX_HCI            M_ENG_MACRX|(M_ENG_HWHCI<<4)
#define RX_CIPHER_HCI     M_ENG_MACRX|(M_ENG_ENCRYPT_SEC<<4)|(M_ENG_HWHCI<<8)
#define RX_CPU_HCI        M_ENG_MACRX|(M_ENG_CPU<<4)|(M_ENG_HWHCI<<8)
#define RX_TRASH          M_ENG_MACRX|(M_ENG_TRASH_CAN<<4)
#define RX_CIPHER_MIC_HCI M_ENG_MACRX|(M_ENG_ENCRYPT_SEC<<4)|(M_ENG_MIC_SEC<<8)|(M_ENG_HWHCI<<12)

#define SSV6200_MAX_HW_MAC_ADDR             2
#define SSV6200_MAX_VIF                     2

#define SSV6200_RX_BA_MAX_SESSIONS			1

enum SSV6XXX_OPMODE {
    SSV6XXX_OPMODE_STA   = 0,
    SSV6XXX_OPMODE_AP    = 1,
    SSV6XXX_OPMODE_IBSS  = 2,
    SSV6XXX_OPMODE_WDS   = 3
};

#define SSV6200_USE_HW_WSID(_sta_idx)    ((_sta_idx == 0) || (_sta_idx == 1))

#define HW_MAX_RATE_TRIES   7

/**
* Define the number of entries of mac decision table.
*
* @ MAC_DECITBL1_SIZE: The decision filtering table size
* @ MAC_DECITBL2_SIZE: The decision mask table size
*/
#define MAC_DECITBL1_SIZE               16
#define MAC_DECITBL2_SIZE               9

#ifdef CONFIG_SSV_CCI_IMPROVEMENT
struct ssv6xxx_b_cca_control {
    u32 down_level;
    u32 upper_level;
    //u32 base_level;
    u32 adjust_cca_control;//0xce0020a0
    u32 adjust_cca_1;//0xce002008
};

struct ssv6xxx_cca_control {
    u32 down_level;
    u32 upper_level;
    //u32 base_level;
    u32 adjust_cck_cca_control;//ADR_WIFI_11B_RX_REG_040
    u32 adjust_ofdm_cca_control;//ADR_WIFI_11GN_RX_REG_040
};
#endif

#ifndef USE_GENERIC_DECI_TBL
extern u16 ap_deci_tbl[];
extern u16 sta_deci_tbl[];
#else
extern u16 generic_deci_tbl[];
#define ap_deci_tbl     generic_deci_tbl
#define sta_deci_tbl    generic_deci_tbl
#endif // USE_GENERIC_DECI_TBL

/**
 *
 */
#define HT_SIGNAL_EXT				    6		//For 2.4G
#define HT_SIFS_TIME        		    10
#define BITS_PER_BYTE           8
#define HT_RC_2_STREAMS(_rc)    ((((_rc) & 0x78) >> 3) + 1)
#define ACK_LEN				    (14)	//include fcs
#define BA_LEN                  (32)
#define RTS_LEN					(20)	//include fcs
#define CTS_LEN					(14)	//include fcs
#define L_STF                   8
#define L_LTF                   8
#define L_SIG                   4
#define HT_SIG                  8
#define HT_STF                  4
#define HT_LTF(_ns)             (4 * (_ns))
#define SYMBOL_TIME(_ns)        ((_ns) << 2) /* ns * 4 us */
#define SYMBOL_TIME_HALFGI(_ns) (((_ns) * 18 + 4) / 5)  /* ns * 3.6 us */

#define CCK_SIFS_TIME        10
#define CCK_PREAMBLE_BITS   144
#define CCK_PLCP_BITS        48

#define OFDM_SIFS_TIME        16
#define OFDM_PREAMBLE_TIME    20
#define OFDM_PLCP_BITS        22
#define OFDM_SYMBOL_TIME      4

#define HOUSE_KEEPING_TIMEOUT       100
#define MAX_RX_IDLE_INTERVAL  3
#define HOUSE_KEEPING_1_SEC			10

//#define WLAN_RC_PHY_CCK         0
//#define WLAN_RC_PHY_OFDM        1

/* These must match mac80211 skb queue mapping numbers */
#define WMM_AC_VO       0
#define WMM_AC_VI       1
#define WMM_AC_BE       2
#define WMM_AC_BK       3
#define WMM_NUM_AC      4

/* The maximal number of support TID */
#define WMM_TID_NUM     8

/**
*
*/
#define TXQ_EDCA_0      0x01
#define TXQ_EDCA_1      0x02
#define TXQ_EDCA_2      0x04
#define TXQ_EDCA_3      0x08
#define TXQ_MGMT        0x10

#define IS_SSV_HT(dsc)             ((dsc)->rate_idx >= 15)
#define IS_SSV_SHORT_GI(dsc)       ((dsc)->rate_idx>=23 && (dsc)->rate_idx<=30)
#define IS_SSV_HT_GF(dsc)          ((dsc)->rate_idx >= 31)
#define IS_SSV_SHORT_PRE(dsc)      ((dsc)->rate_idx>=4 && (dsc)->rate_idx<=14)

// used to adjust rssi
#define RSSI_SMOOTHING_SHIFT        5
#define RSSI_DECIMAL_POINT_SHIFT    6

#ifndef SSV_SUPPORT_HAL

#include "ssv_reg_acc.h"

#endif

#define HCI_START(_sh) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_start(__sh->hci.hci_ctrl); \
        })
#define HCI_STOP(_sh) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_stop(__sh->hci.hci_ctrl); \
        })
#define HCI_WRITE_HW_CONFIG_ON(_sh) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_write_hw_config(__sh->hci.hci_ctrl, 1); \
        })
#define HCI_WRITE_HW_CONFIG_OFF(_sh) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_write_hw_config(__sh->hci.hci_ctrl, 0); \
        })
#define HCI_SEND(_sh, _sk, _q) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_tx(__sh->hci.hci_ctrl, _sk, _q, 0); \
        })
#define HCI_PAUSE(_sh, _mk) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_tx_pause(__sh->hci.hci_ctrl, _mk); \
        })
#define HCI_RESUME(_sh, _mk) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_tx_resume(__sh->hci.hci_ctrl, _mk); \
        })
#define HCI_TXQ_FLUSH(_sh, _mk) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_txq_flush(__sh->hci.hci_ctrl, _mk); \
        })
#define HCI_TXQ_FLUSH_BY_STA(_sh, _aid) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_txq_flush_by_sta(__sh->hci.hci_ctrl, _aid); \
        })
#define HCI_TXQ_EMPTY(_sh, _txqid) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_txq_empty(__sh->hci.hci_ctrl, _txqid); \
        })
#define HCI_WAKEUP_PMU(_sh) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_pmu_wakeup(__sh->hci.hci_ctrl); \
        })
#define HCI_SEND_CMD(_sh, _sk) \
        ({ \
            typeof(_sh) __sh = _sh; \
            __sh->hci.hci_ops->hci_send_cmd(__sh->hci.hci_ctrl, _sk); \
        })

#define SSV6XXX_SET_HW_TABLE(sh_, tbl_)                                 \
({                                                                      \
    int ret = 0;                                                        \
    u32 i=0;                                                            \
    for(; i<sizeof(tbl_)/sizeof(ssv_cabrio_reg); i++) {       \
        ret = SMAC_REG_WRITE(sh_, tbl_[i].address, tbl_[i].data);       \
        if (ret) break;                                                 \
    }                                                                   \
    ret;                                                                \
})

// define macro for STA/VIF using HW/SW cipher check

#define SSV6XXX_USE_HW_DECRYPT(_priv)       (_priv->has_hw_decrypt)    
#define SSV6XXX_USE_SW_DECRYPT(_priv)       (SSV6XXX_USE_LOCAL_SW_DECRYPT(_priv) || SSV6XXX_USE_MAC80211_DECRYPT(_priv)) 
#define SSV6XXX_USE_LOCAL_SW_DECRYPT(_priv)  (_priv->need_sw_decrypt)
#define SSV6XXX_USE_MAC80211_DECRYPT(_priv)  (_priv->use_mac80211_decrypt)

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define RTS_CTS_PROTECT(_flg) \
    ((_flg)&IEEE80211_TX_RC_USE_RTS_CTS)? 1: \
    ((_flg)&IEEE80211_TX_RC_USE_CTS_PROTECT)? 2: 0
#endif

struct ssv_softc;
#ifdef CONFIG_P2P_NOA
struct ssv_p2p_noa;
#endif

#define SSV6200_HT_TX_STREAMS			    1
#define SSV6200_HT_RX_STREAMS			    1
#define SSV6200_RX_HIGHEST_RATE             72


enum PWRSV_STATUS{
    PWRSV_DISABLE,
    PWRSV_ENABLE,
    PWRSV_PREPARE,
};

struct aggr_ssn{
    u16 ssn[MAX_AGGR_NUM];
    u16 tx_pkt_run_no;
    u16 mpdu_num;
	u8  tid_no;
};

struct ssv6xxx_calib_table {
    u16 channel_id;
    u32 rf_ctrl_N;
    u32 rf_ctrl_F;
    u16 rf_precision_default;
};

#ifdef SSV_SUPPORT_HAL
struct ssv_hw;
struct ssv_sta_priv_data;
struct ssv_vif_priv_data;
struct ssv_sta_info;
struct ssv_vif_info;

int hw_update_watch_wsid(struct ssv_softc *sc, struct ieee80211_sta *sta, 
        struct ssv_sta_info *sta_info, int sta_idx, int rx_hw_sec, int ops);

void _set_wep_sw_crypto_key(struct ssv_softc *sc, struct ssv_vif_info *vif_info,
        struct ssv_sta_info *sta_info, void *param);

typedef enum
{
	CLK_SRC_OSC,
    CLK_SRC_RTC,
    CLK_SRC_SYNTH_80M,
    CLK_SRC_SYNTH_40M,    
    CLK_SRC_MAX,
} SSV_CLK_SRC;
/**
* struct ssv_hal - the structure for ssv6xxx HAL function.
*
* This structure is used to register HAL relation function
* for different chip.
*/
struct ssv_hal_ops {
    
    // MAC
    // initial rountine ----
    //adjust pre-set config
	void (*adj_config)(struct ssv_hw *sh);
	// need sw cipher?
	bool (*need_sw_cipher)(struct ssv_hw *sh);
    // init MAC
    int  (*init_mac)(struct ssv_hw *sh);
    // reset plf
    void  (*reset_sysplf)(struct ssv_hw *sh);
    // allocate HW
    struct ssv_hw * (*alloc_hw)(void);
    // init hw PHY and security table
    int  (*init_hw_sec_phy_table)(struct ssv_softc *sc);
    // write mac ini table value into HW
    int  (*write_mac_ini)(struct ssv_hw *sh);
    // check if ccmp/tkip use HW encryption
    bool (*use_hw_encrypt)(int cipher, struct ssv_softc *sc,
        struct ssv_sta_priv_data *sta_priv, struct ssv_vif_priv_data *vif_priv);
    // set rx flow
    int  (*set_rx_flow)(struct ssv_hw *sh, enum ssv_rx_flow type, u32 rxflow);
    // set rx ctrl flow
    int  (*set_rx_ctrl_flow)(struct ssv_hw *sh);
    // set mac addr
    int  (*set_macaddr)(struct ssv_hw *sh, int vif_idx);
    // set bssid
    int  (*set_bssid)(struct ssv_hw *sh, u8 *bssid, int vif_idx);
    // get chip time tag
    u64 (*get_ic_time_tag)(struct ssv_hw *sh);
    // get chip chip id
    void (*get_chip_id)(struct ssv_hw *sh);   
    // if need to check mac address 2
    bool (*if_chk_mac2)(struct ssv_hw *sh);
    // save hw status.
    void (*save_hw_status)(struct ssv_softc *sc);
    // init gpio cfg
    void (*init_gpio_cfg)(struct ssv_hw *sh);
	// security-related-----
    // init wsid and get wsid. return wsid.
    int (*get_wsid)(struct ssv_softc *sc, struct ieee80211_vif *vif,
        struct ieee80211_sta *sta);
    // set mac to hw wsid
    void (*set_hw_wsid)(struct ssv_softc *sc, struct ieee80211_vif *vif,
        struct ieee80211_sta *sta, int wsid);
    // del hw wsid
    void (*del_hw_wsid)(struct ssv_softc *sc, int hw_wsid);
    // add fw wsid
    void (*add_fw_wsid)(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv,
        struct ieee80211_sta *sta, struct ssv_sta_info *sta_info);
    // del fw wsid
    void (*del_fw_wsid)(struct ssv_softc *sc, struct ieee80211_sta *sta,
        struct ssv_sta_info *sta_info);
    // enable fw wsid
    void (*enable_fw_wsid)(struct ssv_softc *sc, struct ieee80211_sta *sta,
        struct ssv_sta_info *sta_info, enum SSV6XXX_WSID_SEC key_type);
    // disable fw wsid
    void (*disable_fw_wsid)(struct ssv_softc *sc, int key_idx,
        struct ssv_sta_priv_data *sta_priv, struct ssv_vif_priv_data *vif_priv);
    // set hw wsid security type(hw or sw) to fw    
    void (*set_fw_hwwsid_sec_type)(struct ssv_softc *sc, struct ieee80211_sta *sta,
        struct ssv_sta_info *sta_info, struct ssv_vif_priv_data *vif_priv);
    // check if wep use hw cipher    
    bool (*wep_use_hw_cipher)(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv);    
    // check if wpa (ccmp/tkip) use hw cipher for pairwise   
    bool (*pairwise_wpa_use_hw_cipher)(struct ssv_softc *sc, 
        struct ssv_vif_priv_data *vif_priv, enum SSV_CIPHER_E cipher,
        struct ssv_sta_priv_data *sta_priv);
    // check if wpa (ccmp/tkip) use hw cipher for group key   
    bool (*group_wpa_use_hw_cipher)(struct ssv_softc *sc, 
        struct ssv_vif_priv_data *vif_priv, enum SSV_CIPHER_E cipher);
    // write aes tkip hw crypto group key
    void (*set_aes_tkip_hw_crypto_group_key) (struct ssv_softc *sc, struct ssv_vif_info *vif_info,
        struct ssv_sta_info *sta_info, void *param);                                               
    // write pairwise key to hw.
    int (*write_pairwise_key_to_hw) (struct ssv_softc *sc, int index, u8 algorithm, const u8 *key, int key_len, 
        struct ieee80211_key_conf *keyconf, struct ssv_vif_priv_data *vif_priv, struct ssv_sta_priv_data *sta_priv);        
    // write group key to hw.
    int (*write_group_key_to_hw) (struct ssv_softc *sc, int index, u8 algorithm, const u8 *key, int key_len, 
        struct ieee80211_key_conf *keyconf, struct ssv_vif_priv_data *vif_priv, struct ssv_sta_priv_data *sta_priv);            
    // write pairwise key index to hw
    void (*write_pairwise_keyidx_to_hw)(struct ssv_hw *sh, int key_idx, int wsid);
    // write group key index to hw
    void (*write_group_keyidx_to_hw)(struct ssv_hw *sh, struct ssv_vif_priv_data *vif_priv, int key_idx);
    // write security key to hw.
    void (*write_key_to_hw)(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, 
        void *data_ptr, int wsid, int key_idx, enum SSV6XXX_WSID_SEC key_type);
    // set group cipher type
    void (*set_group_cipher_type)(struct ssv_hw *sh, struct ssv_vif_priv_data *vif_priv, u8 cipher);
    // set pairwise cipher type
    void (*set_pairwise_cipher_type)(struct ssv_hw *sh, u8 cipher, u8 wsid);
    // check this interface if support hw bssid
    bool (*chk_if_support_hw_bssid)(struct ssv_softc *sc, int vif_idx);
    // change rx flow to cpu if dual i/f use both hw sw cipher for group key
    void (*chk_dual_vif_chg_rx_flow)(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv);
    // restore rx flow for group key delete
    void (*restore_rx_flow)(struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, struct ieee80211_sta *sta);
    // set wep hw crypto key
    void (*set_wep_hw_crypto_key)(struct ssv_softc *sc, struct ssv_sta_info *sta_info, struct ssv_vif_priv_data *vif_priv);
    // write crypto wep key to hw
    int (*hw_crypto_key_write_wep) (struct ssv_softc *sc, struct ieee80211_key_conf *keyconf, u8 algorithm, struct ssv_vif_info *vif_info);
    //store wep key    
    void (*store_wep_key) (struct ssv_softc *sc, struct ssv_vif_priv_data *vif_priv, struct ssv_sta_priv_data *sta_priv, 
        enum SSV_CIPHER_E cipher, struct ieee80211_key_conf *key);
    // reserve skb for hw ccmp encrypt
    bool (*put_mic_space_for_hw_ccmp_encrypt) (struct ssv_softc *sc, struct sk_buff *skb);
#ifdef CONFIG_PM
	// save/clear trap reason
	void (*save_clear_trap_reason)(struct ssv_softc *sc);
    // restore trap reason
	void (*restore_trap_reason)(struct ssv_softc *sc);
    // pmu awake
	void (*pmu_awake)(struct ssv_softc *sc);
#endif
    // misc related----
    // set replay ignore
    void (*set_replay_ignore)(struct ssv_hw *sh, u8 ignore);
    // update_decision_table #6 to val
    void (*update_decision_table_6)(struct ssv_hw *sh, u32 val);
    // update_decision_table
    int  (*update_decision_table)(struct ssv_softc *sc);
    // get fw version to regval
    void (*get_fw_version)(struct ssv_hw *sh, u32 *regval);
    void (*set_mrx_flt)(struct ssv_hw *sh, u32 regval);
    void (*get_mrx_flt)(struct ssv_hw *sh, u32 *regval);
    // set OP_MODE (STA/AP/IBSS/WDS)
    void (*set_op_mode)(struct ssv_hw *sh, u32 opmode, int vif_idx);
    // set to halt management queue util DTIM when on, until beacon when off
    void (*set_halt_mngq_util_dtim)(struct ssv_hw *sh, bool val);
    // set The duration of burst SIFS (in us) of G mode 
    void (*set_dur_burst_sifs_g)(struct ssv_hw *sh, u32 val);
    // set The duration of slot time (in us) of G mode 
    void (*set_dur_slot)(struct ssv_hw *sh, u32 val);
    // set SIFS according to band
    void (*set_sifs)(struct ssv_hw *sh, int band);
    // set QOS on/off
    void (*set_qos_enable)(struct ssv_hw *sh, bool val);
    // set wmm parameters (aifs/cwin/txop
    void (*set_wmm_param)(struct ssv_softc *sc, 
        const struct ieee80211_tx_queue_params *params, u16 queue);
    void (*update_page_id)(struct ssv_hw *sh);
    // init tx cfg
    void (*init_tx_cfg)(struct ssv_hw *sh);
    // init rx cfg
    void (*init_rx_cfg)(struct ssv_hw *sh);
    // allocate pbuf
    u32 (*alloc_pbuf)(struct ssv_softc *sc, int size, int type);
    // free pbuf
    bool (*free_pbuf)(struct ssv_softc *sc, u32 pbuf_addr);
    // enable ampdu auto crc32
    void (*ampdu_auto_crc_en)(struct ssv_hw *sh);
    // set RX BA session
    void (*set_rx_ba)(struct ssv_hw *sh, bool on, u8 *ta,
        u16 tid, u16 ssn, u8 buf_size);
    // read efuse
    u8   (*read_efuse)(struct ssv_hw *sh, u8 *pbuf);
    // write efuse
    void (*write_efuse)(struct ssv_hw *sh, u8 *data, u8 data_length);
    // change clock source
    int (*chg_clk_src)(struct ssv_hw *sh);       
    // update efuse setting 
    int (*update_efuse_setting)(struct ssv_hw *sh);       
    // beacon related ----
    // read beacon config type from register
    enum ssv6xxx_beacon_type (*beacon_get_valid_cfg)(struct ssv_hw *sh);
    // set beacon regist locked
    void (*set_beacon_reg_lock)(struct ssv_hw *sh, bool val);
    // set addr id and dtim
    void (*set_beacon_id_dtim)(struct ssv_softc *sc,
        enum ssv6xxx_beacon_type avl_bcn_type, int dtim_offset);
    // fill beacon packet content
    void (*fill_beacon)(struct ssv_softc *sc, u32 regaddr, struct sk_buff *skb);
    // enable/disable beacon
    bool (*beacon_enable)(struct ssv_softc *sc, bool bEnable);
    // set beacon interval/DTIM count to HW
    void (*set_beacon_info)(struct ssv_hw *sh, u8 beacon_interval, u8 dtim_cnt);
    // read auto beacon ongoing
    bool (*get_bcn_ongoing)(struct ssv_hw *sh);
	// beacon loss enable
	void (*beacon_loss_enable)(struct ssv_hw *sh);
	// beacon loss disable
	void (*beacon_loss_disable)(struct ssv_hw *sh);
	// beacon loss config
	void (*beacon_loss_config)(struct ssv_hw *sh, u16 beacon_interval, const u8 *bssid);
    // hci related ----
    // write txq_mask into hw
    void (*update_txq_mask)(struct ssv_hw *sh, u32 txq_mask);
    // read hci_inq_info from register
    void (*readrg_hci_inq_info)(struct ssv_hw *sh, int *hci_used_id, int *tx_use_page);
    // read txq_info from register
    bool (*readrg_txq_info)(struct ssv_hw *sh, u32 *txq_info, int *hci_used_id);
    // read txq_info2 from register
    bool (*readrg_txq_info2)(struct ssv_hw *sh, u32 *txq_info2, int *hci_used_id);
    // ssv_cmd related ----
    // dump wsid to console.
    bool (*dump_wsid)(struct ssv_hw *sh);
    // dump decision to console.
    bool (*dump_decision)(struct ssv_hw *sh);
	// get mbox output queue status
	u32 (*get_ffout_cnt)(u32 value, int tag);
	// get mbox input queue status
	u32 (*get_in_ffcnt)(u32 value, int tag);
    // read mbox output queue fifo count to value/value1/value2
    void (*read_ffout_cnt)(struct ssv_hw *sh, u32 *value, u32 *value1, u32 *value2);
    // read mbox input queue fifo count to value/value1
    void (*read_in_ffcnt)(struct ssv_hw *sh, u32 *value, u32 *value1);
    // read in id length threshold to value
    void (*read_id_len_threshold)(struct ssv_hw *sh, u32 *tx_len, u32 *rx_len);
    // read tag status to value
    void (*read_tag_status)(struct ssv_hw *sh, u32 *ava_status);
    // cli cmd mib
    void (*cmd_mib)(struct ssv_softc *sc, int argc, char *argv[]);
    // cli cmd power saving     
    void (*cmd_power_saving)(struct ssv_softc *sc, int argc, char *argv[]);
    // cmd for debug information.
    void (*cmd_hwinfo)(struct ssv_hw *sh, int argc, char *argv[]);
    // cmd for enable/disable cci
    void (*cmd_cci)(struct ssv_hw *sh, int argc, char *argv[]);
    // cmd for tx_gen packet
    void (*cmd_txgen)(struct ssv_hw *sh);
    // cmd for changing parameters for rf tools.
    void (*cmd_rf)(struct ssv_hw *sh, int argc, char *argv[]);  
    // cmd for update rf pwr
    void (*update_rf_pwr)(struct ssv_softc *sc);
    // cmd for efuse.
    void (*cmd_efuse)(struct ssv_hw *sh, int argc, char *argv[]);
      
    // cmd to change software limit for hardware queue.
    void (*cmd_hwq_limit)(struct ssv_hw *sh, int argc, char *argv[]);    
    // get RD_IDx address value array
    void (*get_rd_id_adr)(u32 *id_base_address);
    // burst(mutiple) read registers
    int (*burst_read_reg)(struct ssv_hw *sh, u32 *addr, u32 *buf, u8 reg_amount);
    // burst(mutiple) write registers
    int (*burst_write_reg)(struct ssv_hw *sh, u32 *addr, u32 *buf, u8 reg_amount);
    // auto_gen_nullpkt
    int (*auto_gen_nullpkt)(struct ssv_hw *sh, int hwq);
    // exec ssv_cmd cali
    void (*cmd_cali)(struct ssv_hw *sh, int argc, char *argv[]);
	//mcu enable
	void (*load_fw_enable_mcu)(struct ssv_hw *sh);
	//mcu disable
	int (*load_fw_disable_mcu)(struct ssv_hw *sh);
	//set fw status
	int (*load_fw_set_status)(struct ssv_hw *sh, u32 status);
	//get fw status
	int (*load_fw_get_status)(struct ssv_hw *sh, u32 *status);
	//pre config hw interfce
	void (*load_fw_pre_config_device)(struct ssv_hw *sh);
	//post config hw interfce
	void (*load_fw_post_config_device)(struct ssv_hw *sh);
	//reset cpu
	int (*reset_cpu)(struct ssv_hw *sh);
	//set sram mode
	void (*set_sram_mode)(struct ssv_hw *sh, enum SSV_SRAM_MODE mode);
    // enable usb accelerator
    void (*enable_usb_acc)(struct ssv_softc *sc, u8 epnum);
    // disable usb accelerator
    void (*disable_usb_acc)(struct ssv_softc *sc, u8 epnum);
    // set usb lpm enable/disable to hw
    void (*set_usb_lpm)(struct ssv_softc *sc, u8 enable);    
    // jump to rom
    int (*jump_to_rom)(struct ssv_softc *sc);
    // get default fw name
    void (*get_fw_name)(u8 *fw_name);
    // send tx poll to fw
    void (*send_tx_poll_cmd)(struct ssv_hw *sh, u32 type);
    
    // PHY
    // tx descriptor related ----
    // add_txinfo :add a new tx descriptor
    void (*add_txinfo) (struct ssv_softc *sc, struct sk_buff *skb);
    // update_txinfo : setup tx descriptor
    void (*update_txinfo)(struct ssv_softc *sc, struct sk_buff *skb);
    // add txinfo for ampdu
    void (*update_ampdu_txinfo)(struct ssv_softc *sc, struct sk_buff *ampdu_skb);
    // add txinfo for ampdu
    void (*add_ampdu_txinfo)(struct ssv_softc *sc, struct sk_buff *ampdu_skb);
    // update null func txinfo
    int (*update_null_func_txinfo)(struct ssv_softc *sc, struct ieee80211_sta *sta, struct sk_buff *skb);
    // return tx desciptor size
    int (*get_tx_desc_size)(struct ssv_hw *sh);
    // get tx_desc->c_type from tx descriptor
    int (*get_tx_desc_ctype)(struct sk_buff *skb );
    // get tx_desc->reason from tx descriptor
    int (*get_tx_desc_reason)(struct sk_buff *skb );
    // get tx_desc->txq_idx from tx descriptor
    int (*get_tx_desc_txq_idx)(struct sk_buff *skb );    
    // tx rate update
    void (*tx_rate_update)(struct ssv_softc *sc, struct sk_buff *skb);
    // txtput_m2_set_tx_desc
    void (*txtput_set_desc)(struct ssv_hw *sh, struct sk_buff *skb ); 
    // fill tx desc for beacon
    void (*fill_beacon_tx_desc)(struct ssv_softc *sc, struct sk_buff* beacon_skb);  
    // fill tx desc for lpbk
    void (*fill_lpbk_tx_desc)(struct sk_buff *skb, int security, unsigned char rate);  
    // rx descriptor related ----
    // get rx_desc->reason from rx descriptor
    int (*get_tkip_mmic_err)(struct sk_buff *skb );    
    // get rx desciptor size (include rx phy info)    
    int (*get_rx_desc_size)(struct ssv_hw *sh);
    // get rx desc length (without rx phy info)
    int (*get_rx_desc_length)(struct ssv_hw *sh);
    // get rxdesc wsid
    u32 (*get_rx_desc_wsid)(struct sk_buff *skb);
    // get rxdesc rate index
    u32 (*get_rx_desc_rate_idx)(struct sk_buff *skb);
    // get rxdesc mng_used
    u32 (*get_rx_desc_mng_used)(struct sk_buff *skb);
    // get rxdesc rx report
    bool (*is_rx_aggr)(struct sk_buff *skb);
    // get rx packet type = rxdesc->c_type;
    u32 (*get_rx_desc_ctype)(struct sk_buff *skb); 
    // get rx packet type = rxdesc->hdr_offset;
    int (*get_rx_desc_hdr_offset)(struct sk_buff *skb); 
    // get rx rate description;
    int (*chk_lpbk_rx_rate_desc)(struct ssv_hw *sh, struct sk_buff *skb); 
    // get rx desc info: packet_len, ctype, tx_pktno
    void (*get_rx_desc_info)(struct sk_buff *skb, u32 *packet_len, u32 *ctype,
        u32 *_tx_pkt_run_no);     
    // rx frame filter
    bool (*nullfun_frame_filter)(struct sk_buff *skb);
    // common phy ---
    // set default phy mode without enable when on , clear when off
    void (*set_phy_mode)(struct ssv_hw *sh, bool val);
    // enable/disable phy
    void (*phy_enable)(struct ssv_hw *sh, bool val);
	// set rx secondary edcca cfg
	void (*edca_enable)(struct ssv_hw *sh, bool val);
	// get rx secondary edcca stat
	void (*edca_stat)(struct ssv_hw *sh, int *primary, int *secondary);
    // ssv_cmd related ----
    // reset mib phy part
    void (*reset_mib_phy)(struct ssv_hw *sh);
    // dump rx_mib phy part
    void (*dump_mib_rx_phy)(struct ssv_hw *sh);
    // rate control related---
	// set rate control algorithm
	void (*rc_algorithm)(struct ssv_softc *sc);
	// set max rate and rate retry 
	void (*set_80211_hw_rate_config)(struct ssv_softc *sc);
	// legacy bitrate to hw rate desc
	void (*rc_legacy_bitrate_to_rate_desc)(int bitrate, u8 *drate);
	// rate control rx data
	void (*rc_rx_data_handler)(struct ssv_softc *sc, struct sk_buff *skb, u32 rate_index);
	// rate report handler	
	void (*rate_report_handler)(struct ssv_softc *sc, struct sk_buff *skb);
	// update rate report
	void (*rc_process_rate_report)(struct ssv_softc *sc, struct sk_buff *skb);
	// update basic rate
	void (*rc_update_basic_rate)(struct ssv_softc *sc, u32 basic_rates);
	// update ht rate
	s32 (*rc_ht_update_update)(struct sk_buff *skb, struct ssv_softc *sc, struct fw_rc_retry_params *ar);
	// ht group current rate
	bool (*rc_ht_sta_current_rate_is_cck)(struct ieee80211_sta *sta);
    // fill mac80211 rx status
    void (*rc_mac80211_rate_idx)(struct ssv_softc *sc, 
            int hw_rate_idx, struct ieee80211_rx_status *rxs);
    // Update and Get RSSI
    void (*update_rssi)(struct ssv_softc *sc, 
            struct sk_buff *rx_skb, struct ieee80211_rx_status *rxs);
#ifdef CONFIG_SSV_CCI_IMPROVEMENT
    void (*update_scan_cci_setting)(struct ssv_softc *sc);
    void (*recover_scan_cci_setting)(struct ssv_softc *sc);
#endif
    // cmd for change fix rate parameters.
    void (*cmd_rc)(struct ssv_hw *sh, int argc, char *argv[]);
    // cmd for loopback test.
    void (*cmd_loopback)(struct ssv_hw *sh, int argc, char *argv[]);
    // start loopback 
    void (*cmd_loopback_start)(struct ssv_hw *sh);
    // set loopback env
    void (*cmd_loopback_setup_env)(struct ssv_hw *sh);
    //AMPDU
    // ampdu rx start
    int (*ampdu_rx_start)(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_sta *sta, 
            u16 tid, u16 *ssn, u8 buf_size);
    // block ack handler
    void (*ampdu_ba_handler)(struct ieee80211_hw *hw, struct sk_buff *skb, u32 tx_pkt_run_no);    
    bool (*is_legacy_rate)(struct ssv_softc *sc, struct sk_buff *skb);
   	int (*ampdu_max_transmit_length)(struct sk_buff *skb, int rate_idx);

    // BB RF		
    // load PHY ini table pointer
    void (*load_phy_table)(ssv_cabrio_reg **phy_table);
    // get phy table size
    u32 (*get_phy_table_size)(struct ssv_hw *sh);
    // load RF ini table pointer
    void (*load_rf_table)(ssv_cabrio_reg **rf_table);
    // get RF table size
    u32 (*get_rf_table_size)(struct ssv_hw *sh);
    // init PLL
    void (*init_pll)(struct ssv_hw *sh);
    // update RF/phy table according sta.cfg 
    void (*update_cfg_hw_patch)(struct ssv_hw *sh, ssv_cabrio_reg *rf_table, 
        ssv_cabrio_reg *phy_table);
    // update RF/phy table according to configuration
    void (*update_hw_config)(struct ssv_hw *sh, ssv_cabrio_reg *rf_table, 
        ssv_cabrio_reg *phy_table);
    // change PAD settings.   
    int (*chg_pad_setting)(struct ssv_hw *sh);
    // set channel freq to RF
    int (*set_channel)(struct ssv_softc *sc, struct ieee80211_channel *channel, enum nl80211_channel_type);
    // do iq cal
    int (*do_iq_cal)(struct ssv_hw *sh, struct ssv6xxx_iqk_cfg *p_cfg);
    // write pht rf ini table to HW and initial pll.
    int  (*set_pll_phy_rf)(struct ssv_hw *sh, ssv_cabrio_reg *rf_tbl, ssv_cabrio_reg *phy_tbl);
    // enable/disable DPD
    void (*dpd_enable)(struct ssv_hw *sh, bool val);
    // init channel cfg
    void (*init_ch_cfg)(struct ssv_hw *sh);
    // init iqk
    void (*init_iqk)(struct ssv_hw *sh);
    // save default channel cfg for ipd
    void (*save_default_ipd_chcfg)(struct ssv_hw *sh);
    // change phy info for ipd.
    void (*chg_ipd_phyinfo)(struct ssv_hw *sh);
    // set RF on/off
    bool (*set_rf_enable)(struct ssv_hw *sh, bool val);
    // ssv_command related
    // dump phy_register to console.
    bool (*dump_phy_reg)(struct ssv_hw *sh);
    // dump rf_register to console.
    bool (*dump_rf_reg)(struct ssv_hw *sh);
    // return true/false for support iqk cmd or not
    bool (*support_iqk_cmd)(struct ssv_hw *sh);
    // set on3 on/off
    void (*set_on3_enable)(struct ssv_hw *sh, bool val);

};
#endif

struct ssv6xxx_umac_ops {
    int (*umac_rx_raw)(struct sk_buff *);
};

struct ssv_hw_cfg {
    u32 addr;
    u32 value;
    struct list_head list;
};

//define for turismo PADPD use
#define     PADPDBAND   5
#define     MAX_PADPD_TONE  26

struct ssv6006dpd{
    u32     am[MAX_PADPD_TONE/2];
    u32     pm[MAX_PADPD_TONE/2];
};

#define NORMAL_PWR      0
#define ENHANCE_PWR     1
#define GREEN_PWR       1

struct ssv6006_padpd{
    bool    dpd_done[PADPDBAND];
    bool    pwr_mode;           /* 0: normal mode, 1: enhance_mode*/
    u8      current_band;
    u8      wf_rx_abbctune;
    struct  ssv6006dpd val[PADPDBAND];
    u8      bbscale[PADPDBAND];
};

/**
* struct ssv_hw - the structure for ssv6200 hardware information.
*
* This structure is shared between ssv6200 hw/sw mac & HCI/SDIO
* drivers. hw/sw mac registers this structure to HCI/SDIO.
*/
struct ssv_hw {
    struct ssv_softc *sc; /* back point to ssv_softc */
    struct ssv6xxx_platform_data *priv;
    struct ssv6xxx_hci_info hci;

    char   chip_id[SSV6XXX_CHIP_ID_LENGTH];
    u64    chip_tag;

#ifdef SSV_SUPPORT_HAL
    struct ssv_hal_ops hal_ops;
#endif
//    u32 hw_caps;
//    u8 mac_addr[6];
//    int default_channel;

    // efuse category bitmap
    u32 efuse_bitmap;

    /* parameter settings for ssv6200 mac */
    u32 tx_desc_len; /* include tx_phy_info length = 0 */
    u32 rx_desc_len; /* include rx_phy_info length  */ 
    u32 rx_pinfo_pad; /* after the payload */

    u32 tx_page_available;
    u32 ampdu_divider;

	struct ssv6xxx_tx_hw_info tx_info;
	struct ssv6xxx_rx_hw_info rx_info;
    
	/**
        * on chip init, a packet buffer is allocated for both
        * security and phy info table space.
        */
#ifdef SSV6200_ECO
	//Set key base on VIF. 
    u32 hw_buf_ptr[SSV_RC_MAX_STA];
    u32 hw_sec_key[SSV_RC_MAX_STA];
#else
    u32 hw_buf_ptr;
    u32 hw_sec_key;
#endif
    u32 hw_pinfo; 

    /**
     * ssv6200 hardware configuration from external module,
     * such as flash/eeprom...,etc.
     */
    struct ssv6xxx_cfg cfg;

    /* # of MAC addresses this device supports */
    u32    n_addresses;
    /* all mac addresses that this device support */
    struct mac_address maddr[SSV6200_MAX_HW_MAC_ADDR];

    //For restore channel 1-12 default value
    u8  ipd_channel_touch;
    struct ssv6xxx_ch_cfg *p_ch_cfg;
    u32 ch_cfg_size;

	//for rx
    int rx_mode;
    int rx_burstread_size_type;
    int rx_burstread_cnt;

    u8  *page_count; /* Pages allocated by host. Index by packet ID */

    /* hw config */
    struct list_head hw_cfg;
    void (*write_hw_config_cb)(void *param, u32 addr, u32 value);
    void *write_hw_config_args;
    //iqk  
    struct ssv6xxx_iqk_cfg iqk_cfg;
    
    // default tx _power
    u8  default_txgain[PADPDBAND];
};


/**
* struct ssv_tx - tx queue for outgoing frames through interface.
* Each AC queue uniquely associates with a hardware tx queue.
*/
struct ssv_tx {
    u16 seq_no;
    int hw_txqid[WMM_NUM_AC];
    int ac_txqid[WMM_NUM_AC];
    u32 flow_ctrl_status; /* bit wise */

    /* statistical counters: */
    u32 tx_pkt[SSV_HW_TXQ_NUM];
    u32 tx_frag[SSV_HW_TXQ_NUM];

    struct list_head ampdu_tx_que;
    spinlock_t ampdu_tx_que_lock;
    u16 ampdu_tx_group_id;
};



/**
* struct ssv_rx - rx queue for queuing incoming frames from 
*                         interface (SDIO).
* The queue is processed in the background by a work job which
* is scheduled upon frames received.
*/
struct ssv_rx {
    
    /**
         * Hold an empty packet buffer to store 
         * incoming data from interface (SDIO). 
         * move to HCI, do ned to remove ?????????????????????????????????????????
         */
    struct sk_buff *rx_buf;
    
    spinlock_t rxq_lock;
    struct sk_buff_head rxq_head;
    u32 rxq_count;
};


#ifdef MULTI_THREAD_ENCRYPT
/*Encryption for multi-threading*/
struct ssv_encrypt_task_list {
    struct task_struct* encrypt_task;
    wait_queue_head_t   encrypt_wait_q;
    volatile int started;
    volatile int running;
    volatile int paused;
    volatile int cpu_offline;
    u32 cpu_no;
    struct list_head list;
};

#endif

/* Macros for struct ssv_sta_info */
#define SSV6XXX_GET_STA_INFO(_sc, _s) \
    &(_sc)->sta_info[((struct ssv_sta_priv_data *)((_s)->drv_priv))->sta_idx]



/**
* Constatnt value defined for s_flag which indicates a station's 
* current capabilities.
*/
#define STA_FLAG_VALID                  0x00001
#define STA_FLAG_QOS                    0x00002
#define STA_FLAG_AMPDU                  0x00004
#define STA_FLAG_ENCRYPT                0x00008
#define STA_FLAG_AMPDU_RX               0x00010


/**
* struct ssv_sta_info - structure to hold station info.
*/
struct ssv_sta_info {
    u16 aid;
    u16 s_flags;

    int hw_wsid;    /* -1: only software */
    
    struct ieee80211_sta *sta;
    struct ieee80211_vif *vif;

    bool sleeping;
    bool tim_set;
    //#ifdef USE_LOCAL_CRYPTO
    #if 0
    struct ssv_crypto_ops *crypt;
    void *crypt_priv;
    u32  KeySelect;
    bool ampdu_ccmp_encrypt;
    #endif // USE_LOCAL_CRYPTO
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    struct dentry *debugfs_dir;
	#endif // CONFIG_SSV6XXX_DEBUGFS
};


/**
* struct ssv_vif_info - structure to hold vif info.
*/
struct ssv_vif_info {
    struct ieee80211_vif      *vif;
    struct ssv_vif_priv_data  *vif_priv;
    enum                       nl80211_iftype if_type;
//    u8                         bssid[6];

    struct ssv6xxx_hw_sec      sramKey;

    #ifdef CONFIG_SSV6XXX_DEBUGFS
    struct dentry             *debugfs_dir;
    #endif // CONFIG_SSV6XXX_DEBUGFS
};

struct rx_stats {
    u64 n_pkts[8];
    u64 g_pkts[8];
    u64 cck_pkts[4];
    u8  phy_mode;
    u8  ht40;
};

/* struct ssv_sta_priv_data - private data structure for ieee80211_*/
struct ssv_sta_priv_data {
    int                                sta_idx;
    int                                rc_idx;
    int                                rx_data_rate;
    //struct ssv6xxx_sta_rc_info         sta_rc_info;
    struct ssv_sta_info               *sta_info;

    struct list_head                   list; // Hook to VIF's STA list.

    u32                                ampdu_mib_total_BA_counter;
    AMPDU_TID                          ampdu_tid[WMM_TID_NUM];
    bool                               has_hw_encrypt;    // Pairwise frame has hardware encryptor
    bool                               need_sw_encrypt;   // Pairwise frame needs software for encription
    bool                               has_hw_decrypt;    // Pairwise frame has hardware decryptor
    bool                               need_sw_decrypt;   // Pairwise frame needs software for decription
    bool                               use_mac80211_decrypt;  // use mac80211 to decrypt to decrypt group frame
    u8                                 group_key_idx;
    #ifdef USE_LOCAL_CRYPTO
    // Local crypto setting for unicast frames.
    struct ssv_crypto_data              crypto_data;
    //struct ssv_crypto_ops             *crypt;
    //void                              *crypt_priv;

    //u32                                KeySelect;
    //bool                               ampdu_ccmp_encrypt;
    #endif // USE_LOCAL_CRYPTO
    u32  beacon_rssi;
    struct  aggr_ssn                   ampdu_ssn[MAX_CONCUR_AMPDU];
	void *rc_info;
    struct rx_stats                     rxstats;
};

struct wep_hw_key {
    u8 keylen;
    u8 key[SECURITY_KEY_LEN];
};

/**
 * struct ssv_vif_priv_data - data per Virtual Interface, it is a MAC context
 * @id: between 0 and 1
 *
 */
struct ssv_vif_priv_data {
    int                      vif_idx;

    //u16                      id;
    // struct ssv_sta_info      sta_info[SSV_NUM_STA];
    struct list_head         sta_list; // STA list added to this VIF.

    u32                      sta_asleep_mask;

    u32                      pair_cipher;  // Security type for unicast data of this VIF.
    u32                      group_cipher; // Security type for multicast data of this VIF.

    bool                     is_security_valid; // For VIF-wide security.
    bool                     has_hw_encrypt;    // Group frame has hardware encryptor
    bool                     need_sw_encrypt;   // Group frame needs software for encription
    bool                     has_hw_decrypt;    // Group frame has hardware decryptor
    bool                     need_sw_decrypt;   // Group frame needs software for decription
    bool                     use_mac80211_decrypt;  // use mac80211 to decrypt to decrypt group frame
    bool                     force_sw_encrypt; // Freddie ToDo: temporary solution for AP mode.
    u8                       group_key_idx;

    s8                       wep_idx;         // Current wep key index, it is used for HW or SW encryption (one of them)
    int                      wep_cipher;      // Current wep cipher

    #ifdef USE_LOCAL_CRYPTO
    // Local crypto setting for broadcast frame.
    struct ssv_crypto_data              crypto_data;
    //struct ssv_crypto_ops   *crypt;
    //void                    *crypt_priv;

    //u32                      KeySelect;
    //bool                     ampdu_ccmp_encrypt;
    #endif // USE_LOCAL_CRYPTO
}; // struct ssv_vif_priv_data

/**
* Constant values defined for sc_flags which indicates the 
* current status of WiFi driver.
*
* @ SC_OP_DEV_READY: This flag should be set to indicate device is ready to use
* @ SC_OP_HW_RESET
*
*/
#define SC_OP_DEV_READY             BIT(0)
#define SC_OP_HW_RESET              BIT(1)
#define SC_OP_OFFCHAN               BIT(2)
#define SC_OP_FIXED_RATE            BIT(3)
#define SC_OP_SHORT_PREAMBLE        BIT(4)
#define SC_OP_CTS_PROT              BIT(5)
#define SC_OP_DIRECTLY_ACK          BIT(6)
#define SC_OP_BLOCK_CNTL            BIT(7)
#define SC_OP_FORCE_HT20            BIT(8)

#define IS_ALLOW_SCAN                    (sc->p2p_status)
#define IS_NON_AP_MODE                   (sc->ap_vif == NULL)
#define IS_NONE_STA_CONNECTED_IN_AP_MODE (list_empty(&((struct ssv_vif_priv_data *)sc->ap_vif->drv_priv)->sta_list))
#define IS_BLOCK_CNTL(_sc)          (_sc->sc_flags & SC_OP_BLOCK_CNTL)
#define IS_MGMT_AND_BLOCK_CNTL(_sc, _hdr) ((sc->sc_flags & SC_OP_BLOCK_CNTL) &&  (ieee80211_is_mgmt(_hdr->frame_control)))

struct ssv6xxx_beacon_info {
	u32 pubf_addr;
	u16 len;
	u8 tim_offset;
	u8 tim_cnt;	
};


#define SSV6200_MAX_BCAST_QUEUE_LEN 16
struct ssv6xxx_bcast_txq {	
    spinlock_t txq_lock;
    struct sk_buff_head qhead;       
    int cur_qsize;         
};

// #define DEBUG_AMPDU_FLUSH

#ifdef DEBUG_AMPDU_FLUSH
typedef struct AMPDU_TID_st AMPDU_TID;
#define MAX_TID     (24)
#endif // DEBUG_AMPDU_FLUSH

struct _ssv6xxx_txtput{
    struct task_struct *txtput_tsk;
    struct sk_buff *skb;
    u32 size_per_frame;
    u32 loop_times;
    u32 occupied_tx_pages;
};

/**
* struct ssv_softc - hold the whole wifi driver data structure.
*
*/
struct ssv_softc {
    struct ieee80211_hw    *hw;
    struct device          *dev;
    struct platform_device *platform_dev;

    //Force disable directly ack tx frame
    bool force_disable_directly_ack_tx;

    //Force reset
    u32 restart_counter;
    bool force_triger_reset;
    struct work_struct hw_restart_work;

    unsigned long   sdio_throughput_timestamp;
    unsigned long   sdio_rx_evt_size;
    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
    struct ieee80211_supported_band sbands[NUM_NL80211_BANDS];
#else
    struct ieee80211_supported_band sbands[IEEE80211_NUM_BANDS];
#endif
    struct ieee80211_channel *cur_channel;
    u16 hw_chan;
    u16 hw_chan_type;

    /* for sc configuration */
    struct mutex mutex;
    

    /* hardware configuration: */
    struct ssv_hw *sh;

    struct ssv_tx tx;
    struct ssv_rx rx;

    //struct ieee80211_vif *vif_ptr[SSV_NUM_VIF];
    struct ssv_vif_info vif_info[SSV_NUM_VIF];

    struct ssv_sta_info sta_info[SSV_NUM_STA];

    /*just allow one AP vif in the same time */
    struct ieee80211_vif *ap_vif;
    //struct ieee80211_vif *vif[SSV6200_MAX_VIF];
    u8     nvif;

    u32 sc_flags;

    /* mac80211 state*/
    bool mac80211_dev_started;  // device is started. It corresponds to to parameter "started" in struct ieee80211_local

    /* rate control algorithm */
    void *rc;

    /**
     * For fixed rate, max_rate_idx is the fixed rate to use.
     * For auto rate, max_rate_idx is the auto rate.
     */
    int max_rate_idx;


    /*  security algorithm  the variable just use only for wep mode now. 
     *  we just keep the algorithm of pairwise key or group key.
     */
    //u8 algorithm;
    //u8 group_key_idx;
    
    //bool bSecurity_wapi;

    //Rate control timer
    struct workqueue_struct *rc_report_workqueue;
    struct sk_buff_head rc_report_queue;
    struct work_struct rc_report_work;
    //struct ssv_sta_info sta_info[SSV_NUM_STA];
    #ifdef DEBUG_AMPDU_FLUSH
    struct AMPDU_TID_st *tid[MAX_TID];
    #endif // DEBUG_AMPDU_FLUSH
    u16 rc_report_sechedule;

    
    /* ssv6200 mac decision table */
    u16 *mac_deci_tbl;

    /* hanlde mac80211 interface work queue*/
    struct workqueue_struct *config_wq;

    /*Is Q4 send frames after DTIM*/
    /* true->queue 4 send after dtim
     * false->queue4 is just a normal queue.
     **/
    bool bq4_dtim;

//---------------------------------------------------------
    /* Beacon related function*/
    struct work_struct set_tim_work;//change tim element of beacon

    u8 enable_beacon;	        //BEACON_WAITING_ENABLED/BEACON_ENABLED
    u8 beacon_interval;
    u8 beacon_dtim_cnt;			//Maxium DTIM counter number, not DTIM

    //DRV_BCN_BCN0 bit 0
    //DRV_BCN_BCN1 bit 1
    u8 beacon_usage;		
    //double beacon buffer
    struct ssv6xxx_beacon_info beacon_info[2];
    //cache latest beacon
    struct sk_buff *beacon_buf; 
    // beacon miss handler
    struct work_struct	beacon_miss_work;
	//station keep beacon info
    struct sk_buff *beacon_container;
    unsigned long  beacon_container_update_time;

    /* Broadcast related */
    struct work_struct	bcast_start_work;
    struct delayed_work	bcast_stop_work;
    struct delayed_work	bcast_tx_work;

    bool aid0_bit_set;
    u8 hw_mng_used;
    // u32 sta_asleep_mask;
    struct ssv6xxx_bcast_txq bcast_txq;
    int bcast_interval;

    /*=================================================================*/
    /*  <= Hardware related resource end*/
    /*=================================================================*/
    u8 bssid[2][6];

    /* for asic pbuf allocate/release*/
    struct mutex mem_mutex;
	
    /* Protect power save state */
    spinlock_t ps_state_lock; 			
	
    /* Station mode related*/
    //u8 sta_count;
    u8 hw_wsid_bit;

    /* CCMP header source select*/
    bool ccmp_h_sel;

    #if 0
	// CCMP encryption in SSV6xxx driver.
    struct work_struct ampdu_tx_encry_work;
    bool ampdu_encry_work_scheduled;
    bool ampdu_ccmp_encrypt;
    // Sync key to HW encrption engine when mixing software and hardware encryption in one session.
    struct work_struct sync_hwkey_work;
    bool sync_hwkey_write;
    struct ssv_sta_info *key_sync_sta_info;

    AMPDU_REKEY_PAUSE_STATE  ampdu_rekey_pause;
    #endif // 0
    /* ampdu rx related*/
    int rx_ba_session_count;
    struct ieee80211_sta *rx_ba_sta;
    u8  rx_ba_bitmap;
    u8 	ba_ra_addr[ETH_ALEN];
    u16 ba_tid;
    u16 ba_ssn;
    struct work_struct set_ampdu_rx_add_work;
	struct work_struct set_ampdu_rx_del_work;


    bool isAssoc;
    u16 channel_center_freq;		//The channel we use in station mode    
    /*
    * p2p_status:
    * special flag for RK 3036 DLNA mode. 
    * 1: indicate softap is started and allow to scan
    */    
    u8 p2p_status;
    bool bScanning;
    
    /*power saving*/
    int ps_status;
    u16 ps_aid;  
#ifdef CONFIG_SSV_SUPPORT_ANDROID
#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
#endif //CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_HAS_WAKELOCK
    struct wake_lock ssv_wake_lock_;
#endif /* WAKELOCK */

#endif

    // TX handling thread
    u16                 tx_wait_q_woken;
    wait_queue_head_t   tx_wait_q;
    struct sk_buff_head tx_skb_q;
#ifdef CONFIG_SSV6XXX_DEBUGFS
    u32                 max_tx_skb_q_len;
#endif // CONFIG_SSV6XXX_DEBUGFS
    struct task_struct *tx_task;
    bool                tx_q_empty; // HCI queue empty
    struct sk_buff_head tx_done_q;

    // RX handling thread
    u16                 rx_wait_q_woken;
    wait_queue_head_t   rx_wait_q;
    struct sk_buff_head rx_skb_q;
    struct task_struct *rx_task;

#ifdef MULTI_THREAD_ENCRYPT
    struct list_head      encrypt_task_head;
    struct notifier_block cpu_nfb;
    // Encryption threads
    struct sk_buff_head   preprocess_q; // all crypto pkts are queued here
    struct sk_buff_head   crypted_q; // all crypted pkts are queued here
    spinlock_t            crypt_st_lock;
#ifdef CONFIG_SSV6XXX_DEBUGFS
    // MIBs
    u32                   max_preprocess_q_len;
    u32                   max_crypted_q_len;
#endif // CONFIG_SSV6XXX_DEBUGFS
#endif // MULTI_THREAD_ENCRYPT
    
    /**
        * MAC debug counters:
        */
    bool dbg_rx_frame;
    bool dbg_tx_frame;

#ifdef CONFIG_SSV6XXX_DEBUGFS
    /*
     * DebugFS
     */
    struct dentry *debugfs_dir;
    // struct dentry *vif_debugfs_dir;
#endif // CONFIG_SSV6XXX_DEBUGFS

    
#ifdef CONFIG_P2P_NOA
    //p2p_related    
    struct ssv_p2p_noa p2p_noa;
#endif
    /* 
     * Debug Desc
     */
    u32 log_ctrl;

    // for ssv_cmd.
    struct ssv_cmd_data cmd_data;

#ifdef CONFIG_PM
	// for trap_reason
	struct ssv_trap_data trap_data;
#endif

    struct ssv6xxx_umac_ops *umac;
    
    struct ssv6006_padpd dpd;
    
    // atteneuation pwr for green_tx
    u8 green_pwr;
    u8 current_pwr[5];
    
    // for tx desc
    u8 tx_pkt_run_no;
    spinlock_t tx_pkt_run_no_lock;

    // Wait for firmware ready
    wait_queue_head_t   fw_wait_q;
    u32                 iq_cali_done; 

    // remove sta protection
    bool    rxq_in_process;
    bool    remove_sta_in_process;
    
#ifdef CONFIG_SMARTLINK
    u32 ssv_smartlink_status;
    u32 ssv_usr_pid;
#endif

#ifdef CONFIG_SSV_CCI_IMPROVEMENT
    u32 pre_11b_cca_control;
    u32 pre_11b_cca_1;
    u32 pre_11b_rx_abbctune;

    unsigned long cci_last_jiffies;
    u32 cci_current_level;
    u32 cci_current_gate;
    bool cci_set;
    bool cci_start;
    u8  cci_counter;
    u8  ack_counter;
#endif

    // house keeping
    struct timer_list house_keeping;
    /* hanlde house keeping work queue*/
    struct workqueue_struct *house_keeping_wq;
        
    //rx stuck
    struct work_struct rx_stuck_work;
    int rx_stuck_idle_time;
    unsigned long rx_stuck_reset_time;

    //edca mib
    struct work_struct mib_edca_work;
    int primary_edca_mib;
    int secondary_edca_mib;

    //lpbk test  
    bool lpbk_enable;
    int lpbk_tx_pkt_cnt;
    int lpbk_rx_pkt_cnt;
    int lpbk_err_cnt;
    
    // tx poll 
    struct work_struct tx_poll_work;
    
    //cci
    struct work_struct cci_clean_work;

    //ratecontrol word for rf tool.
    u8  rf_rc;

    //txtput
    struct _ssv6xxx_txtput ssv_txtput;
    
    atomic_t ampdu_tx_frame;
    int directly_ack_low_threshold;
    int directly_ack_high_threshold;
};

enum {
    IQ_CALI_RUNNING,
    IQ_CALI_OK,
    IQ_CALI_FAILED
};
//typedef struct cfg_host_event HDR_HostEvent;

void ssv6xxx_txbuf_free_skb(struct sk_buff *skb , void *args);

void ssv6200_rx_process(struct work_struct *work);
#if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
int ssv6200_rx(struct sk_buff_head *rx_skb_q, void *args);
#else			
int ssv6200_rx(struct sk_buff *rx_skb, void *args);
#endif
int ssv6200_is_rx_q_full(void *args);
void ssv6xxx_post_tx_cb(struct sk_buff_head *skb_head, void *args);
void ssv6xxx_pre_tx_cb(struct sk_buff *skb, void *args);
#ifdef RATE_CONTROL_REALTIME_UPDATA
void ssv6xxx_tx_rate_update(struct sk_buff *skb, void *args);
#endif
int ssv6200_tx_flow_control(void *dev, int hw_txqid, bool fc_en, int debug);
void ssv6xxx_tx_q_empty_cb (u32 txq_no, void *);

#ifndef SSV_SUPPORT_HAL
int ssv6xxx_rf_disable(struct ssv_hw *sh);
int ssv6xxx_rf_enable(struct ssv_hw *sh);
int ssv6xxx_set_channel(struct ssv_softc *sc, struct ieee80211_channel *channel, enum nl80211_channel_type);
int ssv6xxx_set_on3_enable(struct ssv_hw *sh, bool val);
#define HAL_SET_CHANNEL(_sc, _ch, _type)  ssv6xxx_set_channel(_sc, _ch, _type)
#endif

#ifdef CONFIG_SMARTLINK
int ssv6xxx_get_channel(struct ssv_softc *sc, int *pch);
int ssv6xxx_set_promisc(struct ssv_softc *sc, int accept);
int ssv6xxx_get_promisc(struct ssv_softc *sc, int *paccept);
#endif

int ssv6xxx_tx_task (void *data);
int ssv6xxx_rx_task (void *data);

/* house keeping & related work */
void ssv6xxx_house_keeping(unsigned long argv);
void ssv6xxx_rx_stuck_process(struct work_struct *work);
void ssv6xxx_mib_edca_process(struct work_struct *work);
void ssv6xxx_tx_poll_process(struct work_struct *work);
void ssv6xxx_cci_clean_process(struct work_struct *work);

#ifndef SSV_SUPPORT_HAL
u32 ssv6xxx_pbuf_alloc(struct ssv_softc *sc, int size, int type);
bool ssv6xxx_pbuf_free(struct ssv_softc *sc, u32 pbuf_addr);
void ssv6xxx_add_txinfo(struct ssv_softc *sc, struct sk_buff *skb);
void ssv6xxx_update_txinfo (struct ssv_softc *sc, struct sk_buff *skb);
int ssv6xxx_update_decision_table(struct ssv_softc *sc);
#endif

void ssv6xxx_rc_report_work(struct work_struct *work);
void ssv6xxx_ps_callback_func(unsigned long data);
void ssv6xxx_enable_ps(struct ssv_softc *sc);
void ssv6xxx_disable_ps(struct ssv_softc *sc);
int ssv6xxx_skb_encrypt(struct sk_buff *mpdu,struct ssv_softc *sc);
int ssv6xxx_skb_decrypt(struct sk_buff *mpdu, struct ieee80211_sta *sta,struct ssv_softc *sc);
void ssv6200_sync_hw_key_sequence(struct ssv_softc *sc, struct ssv_sta_info* sta_info, bool bWrite);
struct ieee80211_sta *ssv6xxx_find_sta_by_rx_skb (struct ssv_softc *sc, struct sk_buff *skb);
struct ieee80211_sta *ssv6xxx_find_sta_by_addr (struct ssv_softc *sc, u8 addr[6]);
void ssv6xxx_foreach_sta (struct ssv_softc *sc,
                          void (*sta_func)(struct ssv_softc *,
                                           struct ssv_sta_info *,
                                           void *),
                          void *param);
void ssv6xxx_foreach_vif_sta (struct ssv_softc *sc,
                              struct ssv_vif_info *vif_info,
                              void (*sta_func)(struct ssv_softc *,
                                               struct ssv_vif_info *,
                                               struct ssv_sta_info *,
                                               void *),
                              void *param);

#ifdef CONFIG_SSV_SUPPORT_ANDROID
#ifdef CONFIG_HAS_EARLYSUSPEND
void ssv6xxx_early_suspend(struct early_suspend *h);
void ssv6xxx_late_resume(struct early_suspend *h);
#endif //CONFIG_HAS_EARLYSUSPEND
#endif

#ifdef USE_LOCAL_CRYPTO
#ifdef MULTI_THREAD_ENCRYPT
struct ssv_crypto_data *ssv6xxx_skb_get_tx_cryptops(struct sk_buff *mpdu);
struct ssv_crypto_data *ssv6xxx_skb_get_rx_cryptops(struct ssv_softc *sc,
                                                    struct ieee80211_sta *sta,
                                                    struct sk_buff *mpdu);
int ssv6xxx_skb_pre_encrypt(struct sk_buff *mpdu, struct ssv_softc *sc);
int ssv6xxx_skb_pre_decrypt(struct sk_buff *mpdu, struct ieee80211_sta *sta, struct ssv_softc *sc);
int ssv6xxx_encrypt_task (void *data);
#endif // MULTI_THREAD_ENCRYPT

#ifdef HAS_CRYPTO_LOCK
    #define INIT_WRITE_CRYPTO_DATA(data, init) \
        struct ssv_crypto_data      *data = (init); \
        unsigned long                data##_flags;

    #define START_WRITE_CRYPTO_DATA(data) \
        do { \
            write_lock_irqsave(&(data)->lock, data##_flags); \
        } while (0)

    #define END_WRITE_CRYPTO_DATA(data) \
        do { \
            write_unlock_irqrestore(&(data)->lock, data##_flags); \
        } while (0)

    #define START_READ_CRYPTO_DATA(data)  \
        do { \
            read_lock(&(data)->lock); \
        } while (0)

    #define END_READ_CRYPTO_DATA(data) \
        do { \
            read_unlock(&(data)->lock); \
        } while (0)

#else

    #define INIT_WRITE_CRYPTO_DATA(data, init) \
        struct ssv_crypto_data      *data = (init);
    #define START_WRITE_CRYPTO_DATA(data) do { } while (0)
    #define END_WRITE_CRYPTO_DATA(data)   do { } while (0)
    #define START_READ_CRYPTO_DATA(data)  do { } while (0)
    #define END_READ_CRYPTO_DATA(data)   do { } while (0)

#endif // HAS_CRYPTO_LOCK
#endif // USE_LOCAL_CRYPTO

#ifdef CONFIG_SSV6XXX_DEBUGFS
ssize_t ssv6xxx_tx_queue_status_dump (struct ssv_softc *sc, char *status_buf,
                                      ssize_t buf_size);
#endif // CONFIG_SSV6XXX_DEBUGFS

// macro for SSV_SUPPORT_HAL not defined
#ifdef SSV_SUPPORT_HAL
#define SSV_UPDATE_PAGE_ID(_sh)                 HAL_UPDATE_PAGE_ID(_sh)
#define SSV_RESET_SYSPLF(_sh)                   HAL_RESET_SYSPLF(_sh)
#define SSV_INIT_IQK(_sh)                       HAL_INIT_IQK(_sh)
#define SSV_SAVE_HW_STATUS(_sc)                 HAL_SAVE_HW_STATUS(_sc)
#define SSV_IF_CHK_MAC2(_sh)					HAL_IF_CHK_MAC2(_sh)
#define SSV_GET_IC_TIME_TAG(_sh)                HAL_GET_IC_TIME_TAG(_sh)
#define SSV_RC_ALGORITHM(_sc)					HAL_RC_ALGORITHM(_sc)
#define SSV_SET_80211HW_RATE_CONFIG(_sc)		HAL_SET_80211HW_RATE_CONFIG(_sc)
#define SSV_RC_LEGACY_BITRATE_TO_RATE_DESC(_sc, _bitrate, _drate)		    \
			HAL_RC_LEGACY_BITRATE_TO_RATE_DESC(_sc, _bitrate, _drate)
#define SSV_DISABLE_FW_WSID(_sc, _index, _sta_priv, _vif_priv)                  \
            HAL_DISABLE_FW_WSID(_sc, _index, _sta_priv, _vif_priv)
#define SSV_ENABLE_FW_WSID(_sc, _sta, _sta_info, _type)                         \
            HAL_ENABLE_FW_WSID(_sc, _sta, _sta_info, _type)
#define SSV_WEP_USE_HW_CIPHER(_sc, _vif_priv)   HAL_WEP_USE_HW_CIPHER(_sc, _vif_priv)
#define SSV_PAIRWISE_WPA_USE_HW_CIPHER( _sc, _vif_priv, _cipher, _sta_priv)     \
            HAL_PAIRWISE_WPA_USE_HW_CIPHER( _sc, _vif_priv, _cipher, _sta_priv)
#define SSV_USE_HW_ENCRYPT(_cipher, _sc, _sta_priv, _vif_priv)                  \
            HAL_USE_HW_ENCRYPT(_cipher, _sc, _sta_priv, _vif_priv)
#define SSV_GROUP_WPA_USE_HW_CIPHER( _sc, _vif_priv, _cipher)                   \
            HAL_GROUP_WPA_USE_HW_CIPHER( _sc, _vif_priv, _cipher)
#define SSV_SET_AES_TKIP_HW_CRYPTO_GROUP_KEY(_sc, _vif_info, _sta_info, _param) \
            HAL_SET_AES_TKIP_HW_CRYPTO_GROUP_KEY(_sc, _vif_info, _sta_info, _param)
#define SSV_WRITE_PAIRWISE_KEYIDX_TO_HW(_sh, _key_idx, _wsid)                   \
            HAL_WRITE_PAIRWISE_KEYIDX_TO_HW(_sh, _key_idx, _wsid)
#define SSV_WRITE_GROUP_KEYIDX_TO_HW(_sh, _key_idx, _wsid)                      \
            HAL_WRITE_GROUP_KEYIDX_TO_HW(_sh, _key_idx, _wsid)
#define SSV_WRITE_PAIRWISE_KEY_TO_HW(_sc, _key_idx, _alg, _key, _key_len, _keyconf, _vif_priv, _sta_priv)   \
            HAL_WRITE_PAIRWISE_KEY_TO_HW(_sc, _key_idx, _alg, _key, _key_len, _keyconf, _vif_priv, _sta_priv)
#define SSV_WRITE_GROUP_KEY_TO_HW(_sc, _key_idx, _alg, _key, _key_len, _keyconf, _vif_priv, _sta_priv)      \
            HAL_WRITE_GROUP_KEY_TO_HW(_sc, _key_idx, _alg, _key, _key_len, _keyconf, _vif_priv, _sta_priv)
#define SSV_WRITE_KEY_TO_HW(_sc, _vif_priv, _sram_ptr, _wsid, _key_idx,  _key_type)   \
            HAL_WRITE_KEY_TO_HW(_sc, _vif_priv, _sram_ptr, _wsid, _key_idx,  _key_type)
#define SSV_SET_PAIRWISE_CIPHER_TYPE( _sh, _cipher, _wsid)                      \
            HAL_SET_PAIRWISE_CIPHER_TYPE( _sh, _cipher, _wsid) 
#define SSV_SET_GROUP_CIPHER_TYPE( _sh, _vif_priv, _cipher)                     \
            HAL_SET_GROUP_CIPHER_TYPE( _sh, _vif_priv, _cipher)
#define SSV_CHK_IF_SUPPORT_HW_BSSID( _sc, _vif_idx)                             \
            HAL_CHK_IF_SUPPORT_HW_BSSID( _sc, _vif_idx)
#define SSV_CHK_DUAL_VIF_CHG_RX_FLOW( _sc, _vif_priv)                           \
            HAL_CHK_DUAL_VIF_CHG_RX_FLOW( _sc, _vif_priv)
#define SSV_SET_FW_HWWSID_SEC_TYPE( _sc, _sta, _sta_info, _vif_priv)            \
            HAL_SET_FW_HWWSID_SEC_TYPE( _sc, _sta, _sta_info, _vif_priv)
#define SSV_SET_RX_CTRL_FLOW(_sh)              HAL_SET_RX_CTRL_FLOW(_sh)
#define SSV_RESTORE_RX_FLOW( _sc, _vif_priv, _sta)                              \
            HAL_RESTORE_RX_FLOW( _sc, _vif_priv, _sta)
#ifdef CONFIG_PM
#define SSV_SAVE_CLEAR_TRAP_REASON(_sc)			HAL_SAVE_CLEAR_TRAP_REASON(_sc)
#define SSV_RESTORE_TRAP_REASON(_sc)			HAL_RESTORE_TRAP_REASON(_sc);
#define SSV_PMU_AWAKE(_sc)						HAL_PMU_AWAKE(_sc)
#endif
#define SSV_ENABLE_USB_ACC(_sc, _epnum)         HAL_ENABLE_USB_ACC(_sc, _epnum)
#define SSV_DISABLE_USB_ACC(_sc, _epnum)        HAL_DISABLE_USB_ACC(_sc, _epnum)
#define SSV_SET_USB_LPM( _sc, _enable)          HAL_SET_USB_LPM( _sc, _enable)
#define SSV_JUMP_TO_ROM(_sc)                    HAL_JUMP_TO_ROM(_sc)

#define SSV_STORE_WEP_KEY( _sc, _vif_priv, _sta_priv, _cipher, _key)            \
            HAL_STORE_WEP_KEY( _sc, _vif_priv, _sta_priv,_cipher, _key)
#define SSV_PUT_MIC_SPACE_FOR_HW_CCMP_ENCRYPT( _sc, _skb)                       \
            HAL_PUT_MIC_SPACE_FOR_HW_CCMP_ENCRYPT( _sc, _skb)
#define SSV_GET_TX_DESC_CTYPE(_sh, _skb)        HAL_GET_TX_DESC_CTYPE(_sh, _skb) 
#define SSV_GET_TX_DESC_SIZE( _sh)              HAL_GET_TX_DESC_SIZE( _sh)
#define SSV_UPDATE_NULL_FUNC_TXINFO(_sc, _sta, _skb)                            \
                                                HAL_UPDATE_NULL_FUNC_TXINFO(_sc, _sta, _skb)
#define SSV_ADD_TXINFO( _sc, _skb)              HAL_ADD_TXINFO( _sc, _skb)
#define SSV_GET_TX_DESC_TXQ_IDX( _sh, _skb)     HAL_GET_TX_DESC_TXQ_IDX( _sh, _skb)
#define SSV_SAVE_DEFAULT_IPD_CHCFG( _sh)        HAL_SAVE_DEFAULT_IPD_CHCFG( _sh)
#define SSV_SET_MACADDR( _sh, _vif_idx)         HAL_SET_MACADDR( _sh, _vif_idx)
#define SSV_SET_BSSID( _sh, _bssid, _vif_idx)   HAL_SET_BSSID( _sh, _bssid, _vif_idx)
#define SSV_SET_OP_MODE( _sh, _opmode, _vif_idx) HAL_SET_OP_MODE( _sh, _opmode, _vif_idx)
#define SSV_HALT_MNGQ_UNTIL_DTIM( _sh)          HAL_HALT_MNGQ_UNTIL_DTIM( _sh, true)
#define SSV_UNHALT_MNGQ_UNTIL_DTIM( _sh)        HAL_HALT_MNGQ_UNTIL_DTIM( _sh, false)
#define SSV_SET_DUR_BURST_SIFS_G( _sh, _val)    HAL_SET_DUR_BURST_SIFS_G( _sh, _val)
#define SSV_SET_DUR_SLOT( _sh, _val)            HAL_SET_DUR_SLOT( _sh, _val)
#define SSV_SET_BCN_IFNO( _sh, _beacon_interval, _beacon_dtim_cnt)              \
            HAL_SET_BCN_IFNO( _sh, _beacon_interval, _beacon_dtim_cnt)
#define SSV_BEACON_ENABLE( _sc, _bool)          HAL_BEACON_ENABLE( _sc, _bool)
#define SSV_SET_HW_WSID( _sc, _vif, _sta, _s)   HAL_SET_HW_WSID( _sc, _vif, _sta, _s)
#define SSV_ADD_FW_WSID( _sc, _vif_priv, _sta, _sta_info)                       \
            HAL_ADD_FW_WSID( _sc, _vif_priv, _sta, _sta_info)
#define SSV_DEL_HW_WSID( _sc, _hw_wsid)         HAL_DEL_HW_WSID( _sc, _hw_wsid)
#define SSV_SET_QOS_ENABLE( _sh, _qos)          HAL_SET_QOS_ENABLE( _sh, _qos)
#define SSV_SET_WMM_PARAM(_sc, _params, _queue) HAL_SET_WMM_PARAM( _sc, _params, _queue)
#define SSV_RC_MAC80211_RATE_IDX( _sc, _rate_idx, _rxs)                         \
            HAL_RC_MAC80211_RATE_IDX( _sc, _rate_idx, _rxs)
#define SSV_INIT_TX_CFG(_sh)         		    HAL_INIT_TX_CFG(_sh)
#define SSV_INIT_RX_CFG(_sh)         		    HAL_INIT_RX_CFG(_sh)
#define SSV_FREE_PBUF(_sc, _hw_buf_ptr)         HAL_FREE_PBUF(_sc, _hw_buf_ptr)
#define SSV_GET_TKIP_MMIC_ERR(_sh, _rx_skb)     HAL_GET_TKIP_MMIC_ERR(_sh, rx_skb);
#define SSV_GET_RX_DESC_INFO( _sh, _skb, _packet_len, _c_type, _tx_pkt_run_no)   \
            HAL_GET_RX_DESC_INFO( _sh, _skb, _packet_len, _c_type, _tx_pkt_run_no)
#define SSV_GET_RX_DESC_HDR_OFFSET(_sh, _skb)                                              \
            HAL_GET_RX_DESC_HDR_OFFSET(_sh, _skb)
#define SSV_IS_LEGACY_RATE(_sc, _skb)           HAL_IS_LEGACY_RATE(_sc, _skb)
#define SSV_RC_HT_STA_CURRENT_RATE_IS_CCK(_sc, _sta)                            \
            HAL_RC_HT_STA_CURRENT_RATE_IS_CCK(_sc, _sta)
#define SSV_RC_UPDATE_BASIC_RATE(_sc, _basic_rates)                             \
			HAL_RC_UPDATE_BASIC_RATE(_sc, _basic_rates)
#define SSV_NULLFUN_FRAME_FILTER(_sh, _skb)     HAL_NULLFUN_FRAME_FILTER(_sh, _skb)
#define SSV_RATE_REPORT_HANDLER(_sc, _skb)		HAL_RATE_REPORT_HANDLER(_sc, _skb)
#define SSV_ADJ_CONFIG(_sh)                     HAL_ADJ_CONFIG(_sh)
#define SSV_BEACON_LOSS_ENABLE(_sh) 			HAL_BEACON_LOSS_ENABLE(_sh)
#define SSV_BEACON_LOSS_DISABLE(_sh) 			HAL_BEACON_LOSS_DISABLE(_sh)
#define SSV_BEACON_LOSS_CONFIG(_sh, _beacon_int, _bssid)						\
			HAL_BEACON_LOSS_CONFIG(_sh, _beacon_int, _bssid)
#define SSV_PHY_ENABLE(_sh, _val)               HAL_PHY_ENABLE(_sh, _val)               
#define SSV_EDCA_ENABLE(_sh, _val)  		    HAL_EDCA_ENABLE(_sh, _val)
#define SSV_EDCA_STAT(_sh, _primary, _secondary)  \
			HAL_EDCA_STAT(_sh, _primary, _secondary)
#define SSV_FILL_LPBK_TX_DESC(_sc, _skb, _sec, _rate)		\
            HAL_FILL_LPBK_TX_DESC(_sc, _skb, _sec, _rate)
#define SSV_CHK_LPBK_RX_RATE_DESC(_sh, _skb)    HAL_CHK_LPBK_RX_RATE_DESC(_sh, _skb)
#define SSV_SEND_TX_POLL_CMD(_sh, _type)        HAL_SEND_TX_POLL_CMD(_sh, _type)
#define SSV_UPDATE_EFUSE_SETTING(_sh)           HAL_UPDATE_EFUSE_SETTING(_sh)
#else
int ssv6xxx_get_tx_desc_ctype(struct sk_buff *skb);
int ssv6xxx_get_tx_desc_txq_idx(struct sk_buff *skb);
int ssv6xxx_update_null_func_txinfo(struct ssv_softc *sc, struct ieee80211_sta *sta, struct sk_buff *skb);
void ssv6xxx_rc_algorithm(struct ssv_softc *sc);
void ssv6xxx_set_80211_hw_rate_config(struct ssv_softc *sc);
void ssv6xxx_phy_enable(struct ssv_hw *sh, bool enable);
bool ssv6xxx_nullfun_frame_filter(struct ssv_hw *sh, struct sk_buff *skb);
void ssv6xxx_reset_sysplf(struct ssv_hw *sh);
void ssv6xxx_init_iqk(struct ssv_hw *sh);
void ssv6xxx_save_hw_status(struct ssv_softc *sc);
u64 ssv6xxx_get_ic_time_tag(struct ssv_hw *sh);
void ssv6xxx_edca_enable(struct ssv_hw *sh, bool val);
void ssv6xxx_edca_stat(struct ssv_hw *sh, int *primary, int *secondary);
void ssv6xxx_send_tx_poll_cmd(struct ssv_hw *sh, u32 type);
int ssv6xxx_set_rx_ctrl_flow(struct ssv_hw *sh);
int ssv6xxx_get_rx_desc_hdr_offset(struct sk_buff *skb);
void ssv6xxx_fill_lpbk_tx_desc(struct sk_buff *skb, int security, unsigned char rate);
int ssv6xxx_chk_rx_rate_desc(struct ssv_hw *sh, struct sk_buff *skb);
int ssv6xxx_update_efuse_setting(struct ssv_hw *sh);
#ifdef CONFIG_PM
void ssv6xxx_save_clear_trap_reason(struct ssv_softc *sc);
void ssv6xxx_restore_trap_reason(struct ssv_softc *sc);
void ssv6xxx_pmu_awake(struct ssv_softc *sc);
#endif
#define SSV_UPDATE_PAGE_ID(_sh)
#define SSV_RESET_SYSPLF(_sh)                   ssv6xxx_reset_sysplf(_sh)
#define SSV_INIT_IQK(_sh)                       ssv6xxx_init_iqk(_sh)
#define SSV_SAVE_HW_STATUS(_sc)                 ssv6xxx_save_hw_status(_sc) 
#define SSV_IF_CHK_MAC2(_sh)					(true)
#define SSV_RC_ALGORITHM(_sc)					ssv6xxx_rc_algorithm(_sc)
#define SSV_SET_80211HW_RATE_CONFIG(_sc)		ssv6xxx_set_80211_hw_rate_config(_sc)
#define SSV_GET_IC_TIME_TAG(_sh)                ssv6xxx_get_ic_time_tag(_sh)
#define SSV_DISABLE_FW_WSID(_sc, _index, _sta_priv, _vif_priv)                  \
            ssv6xxx_disable_fw_wsid(_sc, _index, _sta_priv, _vif_priv)
#define SSV_ENABLE_FW_WSID(_sc, _sta, _sta_info, _type)                         \
            ssv6xxx_enable_fw_wsid(_sc, _sta, _sta_info, _type)
#define SSV_WEP_USE_HW_CIPHER(_sc, _vif_priv)   ssv6xxx_wep_use_hw_cipher(sc, vif_priv)
#define SSV_PAIRWISE_WPA_USE_HW_CIPHER( _sc, _vif_priv, _cipher, _sta_priv)     \
            ssv6xxx_pairwise_wpa_use_hw_cipher( _sc, _vif_priv, _cipher, _sta_priv)
#define SSV_USE_HW_ENCRYPT(_cipher, _sc, _sta_priv, _vif_priv)                  \
            ssv6xxx_use_hw_encrypt(_cipher, _sc, _sta_priv, _vif_priv)
#define SSV_GROUP_WPA_USE_HW_CIPHER( _sc, _vif_priv, _cipher)                   \
            ssv6xxx_group_wpa_use_hw_cipher( _sc, _vif_priv, _cipher) 
#define SSV_SET_AES_TKIP_HW_CRYPTO_GROUP_KEY(_sc, _vif_info, _sta_info, _param) \
            _set_aes_tkip_hw_crypto_group_key(_sc, _vif_info, _sta_info, _param)
#define SSV_WRITE_PAIRWISE_KEYIDX_TO_HW(_sh, _key_idx, _wsid)                   \
            ssv6xxx_write_pairwise_keyidx_to_hw(_sh, _key_idx, _wsid)
#define SSV_WRITE_GROUP_KEYIDX_TO_HW(_sh, _key_idx, _wsid)
#define SSV_WRITE_PAIRWISE_KEY_TO_HW(_sc, _key_idx, _alg, _key, _key_len, _keyconf, _vif_priv, _sta_priv)   \
            _write_pairwise_key_to_hw(_sc, _key_idx, _alg, _key, _key_len, _keyconf, _vif_priv, _sta_priv)
#define SSV_WRITE_GROUP_KEY_TO_HW(_sc, _key_idx, _alg, _key, _key_len, _keyconf, _vif_priv, _sta_priv)      \
            _write_group_key_to_hw(_sc, _key_idx, _alg, _key, _key_len, _keyconf, _vif_priv, _sta_priv)
#define SSV_WRITE_KEY_TO_HW(_sc, _vif_priv, _sram_ptr, _wsid, _key_idx,  _key_type)   \
            ssv6xxx_write_key_to_hw(_sc, _sram_ptr, _wsid, _key_idx,  _key_type)
#define SSV_SET_PAIRWISE_CIPHER_TYPE( _sh, _cipher, _wsid)                      \
            ssv6200_hw_set_pair_type( _sh, _cipher)
#define SSV_SET_GROUP_CIPHER_TYPE( _sh, _vif_priv, _cipher)                     \
            ssv6200_hw_set_group_type( _sh, _cipher)
#define SSV_CHK_IF_SUPPORT_HW_BSSID( _sc, _vif_idx)                             \
            ssv6xxx_chk_if_support_hw_bssid( _sc, _vif_idx)            
#define SSV_CHK_DUAL_VIF_CHG_RX_FLOW( _sc, _vif_priv)                           \
            ssv6xxx_chk_dual_vif_chg_rx_flow( _sc, _vif_priv)
#define SSV_SET_FW_HWWSID_SEC_TYPE( _sc, _sta, _sta_info, _vif_priv)            \
            ssv6xxx_set_fw_hwwsid_sec_type( _sc, _sta, _sta_info, _vif_priv)
#define SSV_SET_RX_CTRL_FLOW(_sh)               ssv6xxx_set_rx_ctrl_flow(_sh)
#define SSV_RESTORE_RX_FLOW( _sc, _vif_priv, _sta)                              \
            ssv6xxx_restore_rx_flow( _sc, _vif_priv, _sta)
#ifdef CONFIG_PM
#define SSV_SAVE_CLEAR_TRAP_REASON(_sc)			ssv6xxx_save_clear_trap_reason(_sc)
#define SSV_RESTORE_TRAP_REASON(_sc)			ssv6xxx_restore_trap_reason(_sc)
#define SSV_PMU_AWAKE(_sc)						ssv6xxx_pmu_awake(_sc)
#endif
#define SSV_ENABLE_USB_ACC(_sc, _epnum)         ssv6xxx_none_func(_sc)
#define SSV_DISABLE_USB_ACC(_sc, _epnum)        ssv6xxx_none_func(_sc)
#define SSV_SET_USB_LPM( _sc, _enable)          ssv6xxx_none_func(_sc)
#define SSV_JUMP_TO_ROM(_sc)                    ssv6xxx_none_func(_sc)
#define SSV_HW_CRYPTO_KEY_WRITE_WEP( _sc, _keyconf, _alg, _vif_info)            \
            hw_crypto_key_write_wep( _sc, _keyconf, _alg, _vif_info)
#define SSV_STORE_WEP_KEY( _sc, _vif_priv, _sta_priv, _cipher, _key)            \
            _store_wep_key( _sc, _vif_priv, _sta_priv, _cipher, _key)
#define SSV_PUT_MIC_SPACE_FOR_HW_CCMP_ENCRYPT( _sc, _skb)
#define SSV_GET_TX_DESC_CTYPE(_sh, _skb)        ssv6xxx_get_tx_desc_ctype( _skb)
#define SSV_GET_TX_DESC_SIZE( _sh)              SSV6XXX_TX_DESC_LEN
#define SSV_ADD_TXINFO( _sc, _skb)              ssv6xxx_add_txinfo( _sc, _skb)
#define SSV_UPDATE_NULL_FUNC_TXINFO(_sc, _sta, _skb)                            \
                                                ssv6xxx_update_null_func_txinfo(_sc, _sta, _skb)
#define SSV_GET_TX_DESC_TXQ_IDX( _sh, _skb)     ssv6xxx_get_tx_desc_txq_idx( _skb)
#define SSV_SET_MACADDR( _sh, _vif_idx)         ssv6xxx_set_macaddr( _sh, _vif_idx)
#define SSV_SET_BSSID( _sh, _bssid, _vif_idx)   ssv6xxx_set_bssid( _sh, _bssid)
#define SSV_SET_OP_MODE( _sh, _opmode, _vif_idx) ssv6xxx_set_op_mode( _sh, _opmode)
#define SSV_HALT_MNGQ_UNTIL_DTIM( _sh)                                          \
            SMAC_REG_SET_BITS( _sh, ADR_MTX_BCN_EN_MISC,                        \
                MTX_HALT_MNG_UNTIL_DTIM_MSK, MTX_HALT_MNG_UNTIL_DTIM_MSK)
#define SSV_UNHALT_MNGQ_UNTIL_DTIM( _sh)                                        \
            SMAC_REG_SET_BITS( _sh, ADR_MTX_BCN_EN_MISC, 0, MTX_HALT_MNG_UNTIL_DTIM_MSK)
#define SSV_SET_DUR_BURST_SIFS_G( _sh, _val)    ssv6xxx_set_dur_burst_sifs_g( _sh, _val)
#define SSV_SET_DUR_SLOT( _sh, _val)            ssv6xxx_set_dur_slot( _sh, _val)
#define SSV_SET_BCN_IFNO( _sh, _beacon_interval, _beacon_dtim_cnt)              \
            ssv6xxx_beacon_set_info( _sh, _beacon_interval, _beacon_dtim_cnt)
#define SSV_BEACON_ENABLE( _sc, _bool)          ssv6xxx_beacon_enable( _sc, _bool)
#define SSV_SET_HW_WSID( _sc, _vif, _sta, _s)   ssv6xxx_set_hw_wsid( _sc, _vif, _sta, _s)
#define SSV_ADD_FW_WSID( _sc, _vif_priv, _sta, _sta_info)                       \
            ssv6xxx_add_fw_wsid( _sc, _vif_priv, _sta, _sta_info)
#define SSV_DEL_HW_WSID( _sc, _hw_wsid)         ssv6xxx_del_hw_wsid( _sc, _hw_wsid)
#define SSV_SET_QOS_ENABLE( _sh, _qos)                                          \
            SMAC_REG_SET_BITS( _sh, ADR_GLBLE_SET, ( (_qos)<<QOS_EN_SFT), QOS_EN_MSK)
#define SSV_SET_WMM_PARAM(_sc, _params, _queue) ssv6xxx_set_wmm_param( _sc, _params, _queue)
#define SSV_RC_MAC80211_RATE_IDX( _sc, _rate_idx, _rxs)                         \
            ssv6xxx_rc_mac80211_rate_idx( _sc, _rate_idx, _rxs)
#define SSV_FREE_PBUF(_sc, _hw_buf_ptr)         ssv6xxx_pbuf_free(_sc, _hw_buf_ptr)
#define SSV_GET_RX_DESC_INFO( _sh, _skb, _packet_len, _c_type, _tx_pkt_run_no)   \
            ssv6xxx_get_rx_desc_info( _skb, _packet_len, _c_type, _tx_pkt_run_no)
#define SSV_GET_RX_DESC_HDR_OFFSET(_sh, _skb)                                              \
            ssv6xxx_get_rx_desc_hdr_offset(_sh, _skb)
#define SSV_IS_LEGACY_RATE(_sc, _skb)           (ssv6xxx_get_real_index(sc, skb) < SSV62XX_RATE_MCS_INDEX)
#define SSV_RC_HT_STA_CURRENT_RATE_IS_CCK(_sc, _sta)              (false)
#define SSV_RC_UPDATE_BASIC_RATE(_sc, _basic_rates)                             \
			ssv6xxx_rc_update_basic_rate(_sc, _basic_rates)
#define SSV_PHY_ENABLE(_sh, _val)               ssv6xxx_phy_enable(_sh, _val)
#define SSV_NULLFUN_FRAME_FILTER(_sh, _skb)     ssv6xxx_nullfun_frame_filter(_sh, _skb)              
#define SSV_EDCA_ENABLE(_sh, _val)			     ssv6xxx_edca_enable(_sh, _val) 
#define SSV_EDCA_STAT(_sh, _primary, _secondary) ssv6xxx_edca_stat(_sh, _primary, _secondary)			    
#define SSV_FILL_LPBK_TX_DESC(_sc, _skb, _sec, _rate)		\
            ssv6xxx_fill_lpbk_tx_desc(_skb, _sec, _rate)
#define SSV_CHK_LPBK_RX_RATE_DESC(_sh, _skb)    ssv6xxx_chk_rx_rate_desc(_sh, _skb)
#define SSV_SEND_TX_POLL_CMD(_sh, _type)        ssv6xxx_send_tx_poll_cmd(_sh, _type)
#define SSV_UPDATE_EFUSE_SETTING(_sh)           ssv6xxx_update_efuse_setting(_sh)
#define SSV_RATE_REPORT_HANDLER(_sc, _skb)
#define SSV_RC_LEGACY_BITRATE_TO_RATE_DESC(_sc, _bitrate, _drate)
#define SSV_BEACON_LOSS_ENABLE(_sh) 			
#define SSV_BEACON_LOSS_DISABLE(_sh) 		
#define SSV_BEACON_LOSS_CONFIG(_sh, _beacon_int, _bssid)
#define SSV_GET_TKIP_MMIC_ERR(_sh, _rx_skb)    0						

#ifdef CONFIG_SSV_CABRIO_E
#define SSV_SAVE_DEFAULT_IPD_CHCFG( _sh)        ssv6xxx_save_default_ipd_chcfg( _sh)
#else
#define SSV_SAVE_DEFAULT_IPD_CHCFG( _sh)
#endif

#define SSV_ADJ_CONFIG(_sh)
#endif

//function header.
struct SKB_info_st;
struct ampdu_ba_notify_data;
int  ssv6xxx_frame_hdrlen(struct ieee80211_hdr *hdr, bool is_ht);
u32  ssv6xxx_ht_txtime(u8 rix, int pktlen, int width, int half_gi, bool is_gf);
u32  ssv6xxx_non_ht_txtime(u8 phy, int kbps, u32 frameLen, bool shortPreamble);
u32  ssv6200_ba_map_walker (struct AMPDU_TID_st *ampdu_tid, u32 start_ssn,
        u32 sn_bit_map[2], struct ampdu_ba_notify_data *ba_notify_data, u32 *p_acked_num);
int  ssv6200_dump_BA_notification (char *buf, struct ampdu_ba_notify_data *ba_notification);
int  ssv6xxx_get_real_index(struct ssv_softc *sc, struct sk_buff *skb);
bool ssv6xxx_ssn_to_bit_idx (u32 start_ssn, u32 mpdu_ssn, u32 *word_idx,
         u32 *bit_idx);
void ssv6xxx_mark_skb_retry (struct ssv_softc *sc, struct SKB_info_st *skb_info, struct sk_buff *skb);
bool ssv6xxx_inc_bit_idx (struct ssv_softc *sc, u32 ssn_1st, u32 ssn_next, u32 *word_idx,
         u32 *bit_idx);   
void ssv6xxx_release_frames (struct AMPDU_TID_st *ampdu_tid);
void ssv6xxx_find_txpktrun_no_from_BA(struct ssv_softc *sc, u32 start_ssn, u32 sn_bit_map[2]
    , struct ssv_sta_priv_data *ssv_sta_priv);
void ssv6xxx_find_txpktrun_no_from_ssn(struct ssv_softc *sc, u32 ssn,
    struct ssv_sta_priv_data *ssv_sta_priv);
void _ssv6xxx_hexdump(const char *title, const u8 *buf, size_t len);
#ifdef CONFIG_PM
void ssv6xxx_power_sleep(void *param);
void ssv6xxx_power_awake(void *param);
#endif
int ssv6xxx_trigger_pmu(struct ssv_softc *sc);

void ssv6xxx_enable_usb_acc(void *param, u8 epnum);
void ssv6xxx_disable_usb_acc(void *param, u8 epnum);
void ssv6xxx_jump_to_rom(void *param);
int ssv6xxx_rx_burstread_size(void *param);
int ssv6xxx_peek_next_pkt_len(struct sk_buff *skb, void *param);
void ssv6xxx_beacon_miss_work(struct work_struct *work);

#endif /* _DEV_H_ */

