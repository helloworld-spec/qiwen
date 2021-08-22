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

#ifndef _SSV_HCI_H_
#define _SSV_HCI_H_


#define SSV_SC(_ctrl_hci)      (_ctrl_hci->shi->sc)

#define TX_PAGE_NOT_LIMITED 255

/**
* The number of SSV6200 hardware TX queue. The
* higher queue value has the higher priority. 
* (BK) (BE) (VI) (VO) (MNG)
*/
#define SSV_HW_TXQ_NUM              5

/**
* The size of the each hardware tx queue.
*/
#define SSV_HW_TXQ_MAX_SIZE         64
#define SSV_HW_TXQ_RESUME_THRES     ((SSV_HW_TXQ_MAX_SIZE >> 2) *3)

/* 
 * hardware tx resource configuration
 */
#define SSV6XXX_ID_TX_THRESHOLD(_hctl)				((_hctl)->tx_info.tx_id_threshold)
#define SSV6XXX_PAGE_TX_THRESHOLD(_hctl)			((_hctl)->tx_info.tx_page_threshold)
#define SSV6XXX_TX_LOWTHRESHOLD_ID_TRIGGER(_hctl)	((_hctl)->tx_info.tx_lowthreshold_id_trigger)
#define SSV6XXX_TX_LOWTHRESHOLD_PAGE_TRIGGER(_hctl)	((_hctl)->tx_info.tx_lowthreshold_page_trigger)
#define SSV6XXX_ID_AC_BK_OUT_QUEUE(_hctl)			((_hctl)->tx_info.bk_txq_size)
#define SSV6XXX_ID_AC_BE_OUT_QUEUE(_hctl)			((_hctl)->tx_info.be_txq_size)
#define SSV6XXX_ID_AC_VI_OUT_QUEUE(_hctl)			((_hctl)->tx_info.vi_txq_size)
#define SSV6XXX_ID_AC_VO_OUT_QUEUE(_hctl)			((_hctl)->tx_info.vo_txq_size)
#define SSV6XXX_ID_MANAGER_QUEUE(_hctl)				((_hctl)->tx_info.manage_txq_size)
#define SSV6XXX_ID_USB_AC_BK_OUT_QUEUE(_hctl)			((_hctl)->shi->sh->cfg.bk_txq_size)
#define SSV6XXX_ID_USB_AC_BE_OUT_QUEUE(_hctl)			((_hctl)->shi->sh->cfg.be_txq_size)
#define SSV6XXX_ID_USB_AC_VI_OUT_QUEUE(_hctl)			((_hctl)->shi->sh->cfg.vi_txq_size)
#define SSV6XXX_ID_USB_AC_VO_OUT_QUEUE(_hctl)			((_hctl)->shi->sh->cfg.vo_txq_size)
#define SSV6XXX_ID_USB_MANAGER_QUEUE(_hctl)			((_hctl)->shi->sh->cfg.manage_txq_size)
#define SSV6XXX_ID_HCI_INPUT_QUEUE  				8


/**
* Define flags for enqueue API 
*/
#define HCI_FLAGS_ENQUEUE_HEAD	   0x00000001
#define HCI_FLAGS_NO_FLOWCTRL	   0x00000002


#define HCI_DBG_PRINT(_hci_ctrl, fmt, ...) \
    do { \
        (_hci_ctrl)->shi->dbgprint((_hci_ctrl)->shi->sc, LOG_HCI, fmt, ##__VA_ARGS__); \
    } while (0)


/**
* struct ssv_hw_txq - ssv6200 hardware tx queue.
* The outgoing frames are finally queued here and wait for
* tx thread to send to hardware through interface (SDIO).
*/
struct ssv_hw_txq {
    u32 txq_no;
    
    //spinlock_t txq_lock;
    struct sk_buff_head qhead;
    int max_qsize;
    int resum_thres;
//    int cur_qsize;
    bool paused;

    /* statistic counters: */
    u32 tx_pkt;
    u32 tx_flags;
};


struct ssv6xxx_hci_ctrl;
/**
* struct ssv_hci_ops - the interface between ssv hci and upper driver.
*
*/
struct ssv6xxx_hci_ops {

//    int (*hci_irq_enable)(void);
//    int (*hci_irq_disable)(void);
    int (*hci_start)(struct ssv6xxx_hci_ctrl *hctrl);
    int (*hci_stop)(struct ssv6xxx_hci_ctrl *hctrl);
    
    void (*hci_write_hw_config)(struct ssv6xxx_hci_ctrl *hctrl, int val);
    int (*hci_read_word)(struct ssv6xxx_hci_ctrl *hctrl, u32 addr, u32 *regval);
    int (*hci_write_word)(struct ssv6xxx_hci_ctrl *hctrl, u32 addr, u32 regval);
    int (*hci_safe_read_word)(struct ssv6xxx_hci_ctrl *hctrl, u32 addr, u32 *regval);
    int (*hci_safe_write_word)(struct ssv6xxx_hci_ctrl *hctrl, u32 addr, u32 regval);
    int (*hci_burst_read_word)(struct ssv6xxx_hci_ctrl *hctrl, u32 *addr, u32 *regval, u8 reg_amount);
    int (*hci_burst_write_word)(struct ssv6xxx_hci_ctrl *hctrl, u32 *addr, u32 *regval, u8 reg_amount);
    int (*hci_burst_safe_read_word)(struct ssv6xxx_hci_ctrl *hctrl, u32 *addr, u32 *regval, u8 reg_amount);
    int (*hci_burst_safe_write_word)(struct ssv6xxx_hci_ctrl *hctrl, u32 *addr, u32 *regval, u8 reg_amount);
    int (*hci_load_fw)(struct ssv6xxx_hci_ctrl *hctrl, u8 *firmware_name, u8 openfile);

    
    /**
        * This function is assigned by HCI driver at initial time and is called 
        * from the drivers above the HCI layer if upper layer has tx frames
        * to send. The return value of this function maybe one of:
        * @ len: after accepting the current frame, return the queue len
        * @ -1: failed
        */
    int (*hci_tx)(struct ssv6xxx_hci_ctrl *hctrl, struct sk_buff *, int, u32);

#if 0
    /**
        * This function is assigned by the drivers above the HCI layer and 
        * is called from HCI driver once it receives frames from interface
        * (SDIO).
        */
    int (*hci_rx)(struct ssv6xxx_hci_ctrl *hctrl, struct sk_buff *);
#endif

    int (*hci_tx_pause)(struct ssv6xxx_hci_ctrl *hctrl, u32 txq_mask);

    /**
        * If HCI queue is full, HCI will prevent upper layer from transmitting 
        * frames. This function is used by HCI to signal upper layer to resume
        * frame transmission.
        */
    int (*hci_tx_resume)(struct ssv6xxx_hci_ctrl *hctrl, u32 txq_mask);

    /**
        * This function is used by upper layer to discard the specified txq 
        * frames. If the parameter is NULL, all txq in HCI will be discarded.
        */
    int (*hci_txq_flush)(struct ssv6xxx_hci_ctrl *hctrl, u32 txq_mask);


    /**
        * Called from upper layer to flush tx frames which are dedicated to
        * a explicitly specify station AID. This function is normally used on
        * AP mode.
        */
    int (*hci_txq_flush_by_sta)(struct ssv6xxx_hci_ctrl *hctrl, int aid);

   
    /**
        * Function provided for query of queue status by upper layer. The
        * parameter maybe one of
        * @ NULL :        indicate all queues
        * @ non-NULL: indicate the specify queue
        */
    bool (*hci_txq_empty)(struct ssv6xxx_hci_ctrl *hctrl, int txqid);
 
    int (*hci_pmu_wakeup)(struct ssv6xxx_hci_ctrl *hctrl);
 
    int (*hci_send_cmd)(struct ssv6xxx_hci_ctrl *hctrl, struct sk_buff *);

#ifdef CONFIG_SSV6XXX_DEBUGFS
    bool (*hci_init_debugfs)(struct ssv6xxx_hci_ctrl *hctrl, struct dentry *dev_deugfs_dir);
    void (*hci_deinit_debugfs)(struct ssv6xxx_hci_ctrl *hctrl);
#endif // CONFIG_SSV6XXX_DEBUGFS
    int (*hci_write_sram)(struct ssv6xxx_hci_ctrl *hctrl, u32 addr, u8* data, u32 size);
    int (*hci_interface_reset)(struct ssv6xxx_hci_ctrl *hctrl);
    int (*hci_sysplf_reset)(struct ssv6xxx_hci_ctrl *hctrl, u32 addr, u32 value);
};



/**
* struct ssv6xxx_hci_info - ssv6xxx hci registration interface.
*
* This structure shall be allocated from registrar and register to
* ssv6xxx hci.
* @ dev
* @ if_ops : sdio/spi operation
* @ hci_ops : hci operation
* @ hci_rx_cb
*/
struct ssv6xxx_hci_info {

    struct device *dev;
    struct ssv6xxx_hwif_ops *if_ops;
    struct ssv6xxx_hci_ops *hci_ops;
    struct ssv6xxx_hci_ctrl *hci_ctrl;
    struct ssv_softc *sc;
    struct ssv_hw    *sh;
    
    /* Rx callback function */

    #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
    int (*hci_rx_cb)(struct sk_buff_head *, void *);
    #else
    int (*hci_rx_cb)(struct sk_buff *, void *);
    #endif // USE_THREAD_RX
    int (*hci_is_rx_q_full)(void *);

	/* Pre Tx callback function */
    void (*hci_pre_tx_cb)(struct sk_buff *, void *);
    
	/* Post Tx callback function */
    void (*hci_post_tx_cb)(struct sk_buff_head *, void *);
    
    /* Flow control callback function */
    int (*hci_tx_flow_ctrl_cb)(void *, int, bool, int debug);

    /* Tx buffer function */
    void (*hci_tx_buf_free_cb)(struct sk_buff *, void *);

    /* Rate control update */
    void (*hci_skb_update_cb)(struct sk_buff *, void *);

    /* HCI queue empty */
    void (*hci_tx_q_empty_cb)(u32 txq_no, void *);

	/* HCI rx mode */
    int (*hci_rx_mode_cb)(void *);
    
	/* HCI peek next packet len */
    int (*hci_peek_next_pkt_len_cb)(struct sk_buff *, void *);
	
    /* DBG print */
	void (*dbgprint)(void *, u32 log_id, const char *fmt,...);

    struct sk_buff *(*skb_alloc) (void *app_param, s32 len);
    void (*skb_free) (void *app_param, struct sk_buff *skb);    
	
    /* HW configuration */
    void (*write_hw_config_cb)(void *param, u32 addr, u32 value);
};


int ssv6xxx_hci_deregister(struct ssv6xxx_hci_info *);
int ssv6xxx_hci_register(struct ssv6xxx_hci_info *);

#if (defined(CONFIG_SSV_SUPPORT_ANDROID)||defined(CONFIG_SSV_BUILD_AS_ONE_KO))
int ssv6xxx_hci_init(void);
void ssv6xxx_hci_exit(void);
#endif

#ifdef SSV_SUPPORT_HAL
#define SSV_READRG_HCI_INQ_INFO(_hci_ctrl, _used_id, _tx_use_page)  \
                                                      HAL_READRG_HCI_INQ_INFO((_hci_ctrl)->shi->sh, _used_id, _tx_use_page)
#define SSV_LOAD_FW_ENABLE_MCU(_hci_ctrl)             HAL_LOAD_FW_ENABLE_MCU((_hci_ctrl)->shi->sh)
#define SSV_LOAD_FW_DISABLE_MCU(_hci_ctrl)            HAL_LOAD_FW_DISABLE_MCU((_hci_ctrl)->shi->sh)
#define SSV_LOAD_FW_SET_STATUS(_hci_ctrl, _status)    HAL_LOAD_FW_SET_STATUS((_hci_ctrl)->shi->sh, (_status))
#define SSV_LOAD_FW_GET_STATUS(_hci_ctrl, _status)    HAL_LOAD_FW_GET_STATUS((_hci_ctrl)->shi->sh, (_status))
#define SSV_RESET_CPU(_hci_ctrl)                      HAL_RESET_CPU((_hci_ctrl)->shi->sh)
#define SSV_SET_SRAM_MODE(_hci_ctrl, _mode)           HAL_SET_SRAM_MODE((_hci_ctrl)->shi->sh, _mode)
#define SSV_LOAD_FW_PRE_CONFIG_DEVICE(_hci_ctrl)      HAL_LOAD_FW_PRE_CONFIG_DEVICE((_hci_ctrl)->shi->sh)
#define SSV_LOAD_FW_POST_CONFIG_DEVICE(_hci_ctrl)     HAL_LOAD_FW_POST_CONFIG_DEVICE((_hci_ctrl)->shi->sh)
#else
void ssv6xxx_hci_hci_inq_info(struct ssv6xxx_hci_ctrl *ctrl_hci, int *used_id);
void ssv6xxx_hci_load_fw_enable_mcu(struct ssv6xxx_hci_ctrl *ctrl_hci);
int ssv6xxx_hci_load_fw_disable_mcu(struct ssv6xxx_hci_ctrl *ctrl_hci);
int ssv6xxx_hci_load_fw_set_status(struct ssv6xxx_hci_ctrl *ctrl_hci, int status);
int ssv6xxx_hci_load_fw_get_status(struct ssv6xxx_hci_ctrl *ctrl_hci, int *status);
void ssv6xxx_hci_load_fw_pre_config_device(struct ssv6xxx_hci_ctrl *ctrl_hci);
void ssv6xxx_hci_load_fw_post_config_device(struct ssv6xxx_hci_ctrl *ctrl_hci);

#define SSV_READRG_HCI_INQ_INFO(_hci_ctrl, _used_id, _tx_use_page)   \
                                                       ssv6xxx_hci_hci_inq_info(_hci_ctrl, _used_id)
#define SSV_LOAD_FW_ENABLE_MCU(_hci_ctrl)              ssv6xxx_hci_load_fw_enable_mcu((_hci_ctrl))
#define SSV_LOAD_FW_DISABLE_MCU(_hci_ctrl)             ssv6xxx_hci_load_fw_disable_mcu((_hci_ctrl))
#define SSV_LOAD_FW_SET_STATUS(_hci_ctrl, _status)     ssv6xxx_hci_load_fw_set_status((_hci_ctrl), (_status))
#define SSV_LOAD_FW_GET_STATUS(_hci_ctrl, _status)     ssv6xxx_hci_load_fw_get_status((_hci_ctrl), (_status))
#define SSV_RESET_CPU(_hci_ctrl)                       ssv6xxx_hci_reset_cpu((_hci_ctrl))
#define SSV_SET_SRAM_MODE(_hci_ctrl, _mode)
#define SSV_LOAD_FW_PRE_CONFIG_DEVICE(_hci_ctrl)       ssv6xxx_hci_load_fw_pre_config_device(_hci_ctrl)
#define SSV_LOAD_FW_POST_CONFIG_DEVICE(_hci_ctrl)      ssv6xxx_hci_load_fw_post_config_device(_hci_ctrl)
#endif

#endif /* _SSV_HCI_H_ */


