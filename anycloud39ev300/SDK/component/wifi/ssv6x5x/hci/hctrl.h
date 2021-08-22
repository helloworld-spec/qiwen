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

#ifndef _HCTRL_H_
#define _HCTRL_H_

#define SSV6XXX_HCI_OP_INVALID      0x00000001  // deregister hci 
#define SSV6XXX_HCI_OP_IFERR        0x00000002  

#define SSV6XXX_INT_RX              0x00000001  //1<<0
/*Workaround solution: We use this interrupt bit for Queue 4(MNG) */
#define SSV6XXX_INT_TX              0x00000002  //1<<1
#define SSV6XXX_INT_SOC             0x00000004  //1<<2
#define SSV6XXX_INT_LOW_EDCA_0      0x00000008  //1<<3
#define SSV6XXX_INT_LOW_EDCA_1      0x00000010  //1<<4
#define SSV6XXX_INT_LOW_EDCA_2      0x00000020  //1<<5
#define SSV6XXX_INT_LOW_EDCA_3      0x00000040  //1<<6
#define SSV6XXX_INT_RESOURCE_LOW    0x00000080  //1<<7


#define IFDEV(_ct)                      ((_ct)->shi->dev)
#define IFOPS(_ct)                      ((_ct)->shi->if_ops)
#define HCI_REG_READ(_ct, _adr, _val)   IFOPS(_ct)->readreg(IFDEV(_ct), _adr, _val)
#define HCI_REG_WRITE(_ct,_adr, _val)   IFOPS(_ct)->writereg(IFDEV(_ct), _adr, _val)
#define HCI_REG_SAFE_READ(_ct, _adr, _val)   IFOPS(_ct)->safe_readreg(IFDEV(_ct), _adr, _val)
#define HCI_REG_SAFE_WRITE(_ct,_adr, _val)   IFOPS(_ct)->safe_writereg(IFDEV(_ct), _adr, _val)
#define HCI_BURST_REG_READ(_ct, _adr, _val, _num)   IFOPS(_ct)->burst_readreg(IFDEV(_ct), _adr, _val, _num)
#define HCI_BURST_REG_WRITE(_ct,_adr, _val, _num)   IFOPS(_ct)->burst_writereg(IFDEV(_ct), _adr, _val, _num)
#define HCI_BURST_REG_SAFE_READ(_ct, _adr, _val, _num)   IFOPS(_ct)->burst_safe_readreg(IFDEV(_ct), _adr, _val, _num)
#define HCI_BURST_REG_SAFE_WRITE(_ct,_adr, _val, _num)   IFOPS(_ct)->burst_safe_writereg(IFDEV(_ct), _adr, _val, _num)
#define HCI_REG_SET_BITS(_ct, _reg, _set, _clr)     \
{                                                   \
    u32 _regval;                                    \
    if(HCI_REG_READ(_ct, _reg, &_regval));          \
    _regval &= ~(_clr);                             \
    _regval |= (_set);                              \
    if(HCI_REG_WRITE(_ct, _reg, _regval));          \
}

#define IF_SEND(_ct, _bf, _len, _qid)   		IFOPS(_ct)->write(IFDEV(_ct), _bf, _len, _qid)
#define IF_RECV(_ct, _bf, _len, _mode)     		IFOPS(_ct)->read(IFDEV(_ct), _bf, _len, _mode)
#define HCI_LOAD_FW(_ct, _addr, _data, _size)   IFOPS(_ct)->load_fw(IFDEV(_ct), _addr, _data, _size)

struct ssv6xxx_hci_ctrl {

    struct ssv6xxx_hci_info *shi;
    u32 hci_flags;
    /* save hw register value */
    int write_hw_config;

    /* SDIO interrupt status */
    spinlock_t int_lock;
    u32 int_status;
    
    /* bit3(VO) bit4(VI) bit5(BE) bit6(BK) r/w this field need to use "int_lock"*/
    u32 int_mask;

    /* pause/resume */
    struct mutex txq_mask_lock;
    u32 txq_mask;
    
    /* hardware tx queue: 0 has the lowest priority */
    struct ssv_hw_txq hw_txq[SSV_HW_TXQ_NUM];

    /* ASIC int mask need to be protected by it */
    struct mutex hci_mutex;
    
    bool hci_start;

    /**
     * After first time firmware download, later ones called "redownload"
     * After first time firmware download, it always be 1
     */
    bool redownload;
    
    /**
        * Always hold an empty skbuff so that the incoming data
        * could be receive.
        */
    struct sk_buff *rx_buf;
    u32 rx_pkt;

    struct workqueue_struct *hci_work_queue;
    struct work_struct hci_rx_work;    
#ifdef CONFIG_SSV_TX_LOWTHRESHOLD
    struct work_struct hci_tx_work;
#else
    struct work_struct hci_tx_work[SSV_HW_TXQ_NUM];
#endif
	wait_queue_head_t   tx_wait_q;
	struct task_struct *hci_tx_task;


    /**
        * HCI debug statistical counters:
        */
    u32 read_rs0_info_fail;
    u32 read_rs1_info_fail;
    u32 rx_work_running;
    u32 isr_running;
    u32 xmit_running;
	u32 isr_disable;
	
    u32 isr_summary_eable;
    u32 isr_routine_time;    
    u32 isr_tx_time;
    u32 isr_rx_time;
    u32 isr_idle_time;
    u32 isr_rx_idle_time;
    /* neither tx or tx->should tx be happen*/
    u32 isr_miss_cnt;

    unsigned long prev_isr_jiffes;
    unsigned long prev_rx_isr_jiffes;
    
	struct work_struct isr_reset_work;    
#ifdef CONFIG_SSV6XXX_DEBUGFS
	struct dentry *debugfs_dir;
	u32       isr_mib_enable;
	u32       isr_mib_reset;
	long long isr_total_time;
	long long isr_tx_io_time;
	long long isr_rx_io_time;
	u32       isr_rx_io_count;
	u32       isr_tx_io_count;
	long long isr_rx_proc_time;
#endif // CONFIG_SSV6XXX_DEBUGFS	
#ifdef CONFIG_IRQ_DEBUG_COUNT
    bool irq_enable;
    u32 irq_count;
    u32 invalid_irq_count;
    u32 tx_irq_count;
    u32 real_tx_irq_count;
    u32 rx_irq_count;
    u32 irq_rx_pkt_count;
    u32 irq_tx_pkt_count;
#endif // CONFIG_IRQ_DEBUG_COUNT


	struct ssv6xxx_tx_hw_info	tx_info;
	struct ssv6xxx_rx_hw_info	rx_info;
};



struct ssv6xxx_hci_txq_info {
	u32 tx_use_page:8;
    u32 tx_use_id:6;
    u32 txq0_size:4;
	u32 txq1_size:4;
	u32 txq2_size:5;
	u32 txq3_size:5;
};

struct ssv6xxx_hci_txq_info2 {
	u32 tx_use_page:9;
    u32 tx_use_id:8;
	u32 txq4_size:4;
    u32 rsvd:11;
};

struct ssv6xxx_hw_resource
{
	u32 free_tx_page;
	u32 free_tx_id;
	int max_tx_frame[SSV_HW_TXQ_NUM];
};

static inline void ssv6xxx_hwif_irq_request(struct ssv6xxx_hci_ctrl *hctrl, irq_handler_t irq_handler)
{
	if(hctrl->shi->if_ops->irq_request)
		hctrl->shi->if_ops->irq_request(IFDEV(hctrl), irq_handler, hctrl);
}

static inline void ssv6xxx_hwif_irq_enable(struct ssv6xxx_hci_ctrl *hctrl)
{
	if(hctrl->shi->if_ops->irq_enable)
		hctrl->shi->if_ops->irq_enable(IFDEV(hctrl));
}

static inline void ssv6xxx_hwif_irq_disable(struct ssv6xxx_hci_ctrl *hctrl)
{
	if(hctrl->shi->if_ops->irq_disable)
		hctrl->shi->if_ops->irq_disable(IFDEV(hctrl), false);
}

static inline int ssv6xxx_hwif_irq_getstatus(struct ssv6xxx_hci_ctrl *hctrl, int *status)
{
	if(hctrl->shi->if_ops->irq_getstatus)
		return	hctrl->shi->if_ops->irq_getstatus(IFDEV(hctrl), status);
	
	return 0;
}

static inline void ssv6xxx_hwif_irq_setmask(struct ssv6xxx_hci_ctrl *hctrl, int mask)
{
	if(hctrl->shi->if_ops->irq_setmask)
		hctrl->shi->if_ops->irq_setmask(IFDEV(hctrl), mask);
}

static inline void ssv6xxx_hwif_irq_trigger(struct ssv6xxx_hci_ctrl *hctrl)
{
	if(hctrl->shi->if_ops->irq_trigger)
		hctrl->shi->if_ops->irq_trigger(IFDEV(hctrl));
}

static inline void ssv6xxx_hwif_pmu_wakeup(struct ssv6xxx_hci_ctrl *hctrl)
{
	if(hctrl->shi->if_ops->pmu_wakeup)
		hctrl->shi->if_ops->pmu_wakeup(IFDEV(hctrl));
}

static inline int ssv6xxx_hwif_write_sram(struct ssv6xxx_hci_ctrl *hctrl, u32 addr, u8 *data, u32 size)
{
	if(hctrl->shi->if_ops->write_sram)
		return hctrl->shi->if_ops->write_sram(IFDEV(hctrl), addr, data, size);
	
	return 0;
}

static inline bool ssv6xxx_hwif_ready(struct ssv6xxx_hci_ctrl *hctrl)
{
	if(hctrl->shi->if_ops->property)
		return hctrl->shi->if_ops->is_ready(IFDEV(hctrl));
	
	return false;
}

static inline int ssv6xxx_hwif_property(struct ssv6xxx_hci_ctrl *hctrl)
{
	if(hctrl->shi->if_ops->property)
		return hctrl->shi->if_ops->property(IFDEV(hctrl));
	
	return 0;
}

static inline void ssv6xxx_hwif_load_fw_pre_config_device(struct ssv6xxx_hci_ctrl *hctrl)
{
	if(hctrl->shi->if_ops->load_fw_pre_config_device)
		hctrl->shi->if_ops->load_fw_pre_config_device(IFDEV(hctrl));
}

static inline void ssv6xxx_hwif_load_fw_post_config_device(struct ssv6xxx_hci_ctrl *hctrl)
{
	if(hctrl->shi->if_ops->load_fw_post_config_device)
		hctrl->shi->if_ops->load_fw_post_config_device(IFDEV(hctrl));
}

#if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
static inline void ssv6xxx_hwif_rx_task(struct ssv6xxx_hci_ctrl *hctrl, int (*rx_cb)(struct sk_buff_head *rxq, void *args), 
					int (*is_rx_q_full)(void *args), void *args, u32 *pkt)
#else
static inline void ssv6xxx_hwif_rx_task(struct ssv6xxx_hci_ctrl *hctrl, int (*rx_cb)(struct sk_buff *rx_skb, void *args), 
					int (*is_rx_q_full)(void *args), void *args, u32 *pkt)
#endif
{
	if(hctrl->shi->if_ops->hwif_rx_task)
		hctrl->shi->if_ops->hwif_rx_task(IFDEV(hctrl), rx_cb, is_rx_q_full, args, pkt);
}

static inline void ssv6xxx_hwif_interface_reset(struct ssv6xxx_hci_ctrl *hctrl)
{
    if(hctrl->shi->if_ops->interface_reset)
        hctrl->shi->if_ops->interface_reset(IFDEV(hctrl));
}

static inline int ssv6xxx_hwif_start_usb_acc(struct ssv6xxx_hci_ctrl *hctrl, u8 epnum)
{
    if(hctrl->shi->if_ops->start_usb_acc)
        return hctrl->shi->if_ops->start_usb_acc(IFDEV(hctrl), epnum);

    return 0;
}

static inline int ssv6xxx_hwif_stop_usb_acc(struct ssv6xxx_hci_ctrl *hctrl, u8 epnum)
{
    if(hctrl->shi->if_ops->stop_usb_acc)
        return hctrl->shi->if_ops->stop_usb_acc(IFDEV(hctrl), epnum);

    return 0;
}

static inline int ssv6xxx_hwif_jump_to_rom(struct ssv6xxx_hci_ctrl *hctrl)
{
    if(hctrl->shi->if_ops->jump_to_rom)
        return hctrl->shi->if_ops->jump_to_rom(IFDEV(hctrl));

    return 0;
}

static inline void ssv6xxx_hwif_sysplf_reset(struct ssv6xxx_hci_ctrl *hctrl, u32 addr, u32 value)
{
    if(hctrl->shi->if_ops->sysplf_reset)
        hctrl->shi->if_ops->sysplf_reset(IFDEV(hctrl), addr, value);
}

#define HCI_IRQ_REQUEST(ct, hdle)               ssv6xxx_hwif_irq_request(ct, hdle)
#define HCI_IRQ_ENABLE(ct)                      ssv6xxx_hwif_irq_enable(ct)
#define HCI_IRQ_DISABLE(ct)                     ssv6xxx_hwif_irq_disable(ct)
#define HCI_IRQ_STATUS(ct, sts)                 ssv6xxx_hwif_irq_getstatus(ct, sts)
#define HCI_IRQ_SET_MASK(ct, mk)                ssv6xxx_hwif_irq_setmask(ct, mk)
#define HCI_IRQ_TRIGGER(ct)                     ssv6xxx_hwif_irq_trigger(ct)
#define HCI_PMU_WAKEUP(ct)                      ssv6xxx_hwif_pmu_wakeup(ct)
#define HCI_SRAM_WRITE(ct, adr, dat, size)      ssv6xxx_hwif_write_sram(ct, adr, dat, size)
#define HCI_HWIF_READY(ct)                      ssv6xxx_hwif_ready(ct)
#define HCI_HWIF_PROPERTY(ct)                   ssv6xxx_hwif_property(ct)
#define HCI_LOAD_FW_PRE_CONFIG_DEVICE(ct)       ssv6xxx_hwif_load_fw_pre_config_device(ct)
#define HCI_LOAD_FW_POST_CONFIG_DEVICE(ct)      ssv6xxx_hwif_load_fw_post_config_device(ct)
#define HCI_RX_TASK(ct, rx_cb, is_rx_q_full, args, pkt)        ssv6xxx_hwif_rx_task(ct, rx_cb, is_rx_q_full, args, pkt)
#define HCI_IFC_RESET(ct)                       ssv6xxx_hwif_interface_reset(ct)
#define HCI_START_USB_ACC(ct, epnum)            ssv6xxx_hwif_start_usb_acc(ct, epnum)
#define HCI_STOP_USB_ACC(ct, epnum)             ssv6xxx_hwif_stop_usb_acc(ct, epnum)
#define HCI_JUMP_TO_ROM(ct)                     ssv6xxx_hwif_jump_to_rom(ct)
#define HCI_SYSPLF_RESET(ct, addr, value)       ssv6xxx_hwif_sysplf_reset(ct, addr, value)

#define HCI_DEVICE_TYPE(_hci_ctrl)              (HCI_HWIF_PROPERTY(_hci_ctrl) & SSV_HWIF_INTERFACE_MASK)
#endif
