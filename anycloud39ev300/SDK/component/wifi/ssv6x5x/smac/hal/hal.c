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

#ifdef ECLIPSE
#include <ssv_mod_conf.h>
#endif // ECLIPSE
#include <linux/version.h>
#include <linux/platform_device.h>

#ifdef SSV_SUPPORT_HAL
#include <linux/string.h>
#include <ssv6200.h>
#include <smac/dev.h>
#include <hal.h>
#include <smac/ssv_skb.h>

static int ssv6xxx_do_iq_cal(struct ssv_hw *sh, struct ssv6xxx_iqk_cfg *p_cfg)
{
	struct ssv_softc *sc = sh->sc;
	dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Do not need IQ CAL for this model!! \n");
	return 0;
}

static void ssv6xxx_dpd_enable(struct ssv_hw *sh, bool val)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support DPD for this model!! \n");
}

static void ssv6xxx_init_ch_cfg(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support set channel dependant cfg for this model!! \n");
}

static void ssv6xxx_init_iqk(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support so save default iqk cfg for this model!! \n");
}

static void ssv6xxx_save_default_ipd_chcfg(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support to change phy info according to ipd for this model!! \n");
}

static void ssv6xxx_chg_ipd_phyinfo(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "Not support to save default channel cfg for ipd for this model!! \n");
}

static void ssv6xxx_update_cfg_hw_patch(struct ssv_hw *sh, 
    ssv_cabrio_reg *rf_table, ssv_cabrio_reg *phy_table)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_update_hw_config(struct ssv_hw *sh, 
    ssv_cabrio_reg *rf_table, ssv_cabrio_reg *phy_table)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static int ssv6xxx_chg_pad_setting(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
    return 0;
}

static void ssv6xxx_cmd_cali(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_cmd_rc(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_cmd_efuse(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_set_sifs(struct ssv_hw *sh, int band)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_cmd_loopback(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_cmd_loopback_start(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_cmd_loopback_setup_env(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static int ssv6xxx_chk_lpbk_rx_rate_desc(struct ssv_hw *sh, struct sk_buff *skb)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
    return 0;
}

static void ssv6xxx_cmd_hwinfo(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}
static int ssv6xxx_get_tkip_mmic_err(struct sk_buff *skb)
{
   return 0 ;
}

static void ssv6xxx_cmd_cci(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_cmd_txgen(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_cmd_rf(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_cmd_hwq_limit(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static void ssv6xxx_update_rf_pwr(struct ssv_softc *sc){
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}    

static void ssv6xxx_init_gpio_cfg(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}    

static void ssv6xxx_write_efuse(struct ssv_hw *sh, u8 *data, u8 data_length)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}

static int ssv6xxx_update_efuse_setting(struct ssv_hw *sh)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
    return 0;
}    

void ssv6xxx_set_on3_enable(struct ssv_hw *sh, bool val)
{
    struct ssv_softc *sc = sh->sc;
    dbgprint(&sc->cmd_data, sc->log_ctrl, LOG_HAL, 
	    "%s is not supported for this model!!\n",__func__);
}    

static void ssv6xxx_attach_common_hal (struct ssv_hal_ops  *hal_ops)
{
    hal_ops->do_iq_cal = ssv6xxx_do_iq_cal;
    hal_ops->dpd_enable = ssv6xxx_dpd_enable;
    hal_ops->init_ch_cfg = ssv6xxx_init_ch_cfg;
    hal_ops->init_iqk = ssv6xxx_init_iqk;
    hal_ops->save_default_ipd_chcfg = ssv6xxx_save_default_ipd_chcfg;
    hal_ops->chg_ipd_phyinfo = ssv6xxx_chg_ipd_phyinfo;
    hal_ops->update_cfg_hw_patch = ssv6xxx_update_cfg_hw_patch;
    hal_ops->update_hw_config = ssv6xxx_update_hw_config;
    hal_ops->chg_pad_setting = ssv6xxx_chg_pad_setting;
    hal_ops->cmd_cali = ssv6xxx_cmd_cali;
    hal_ops->cmd_rc = ssv6xxx_cmd_rc;
    hal_ops->cmd_efuse = ssv6xxx_cmd_efuse;
    hal_ops->set_sifs = ssv6xxx_set_sifs;
	hal_ops->cmd_loopback = ssv6xxx_cmd_loopback;
	hal_ops->cmd_loopback_start = ssv6xxx_cmd_loopback_start;
	hal_ops->cmd_loopback_setup_env = ssv6xxx_cmd_loopback_setup_env;
    hal_ops->chk_lpbk_rx_rate_desc = ssv6xxx_chk_lpbk_rx_rate_desc;
	hal_ops->cmd_hwinfo = ssv6xxx_cmd_hwinfo;
	hal_ops->get_tkip_mmic_err = ssv6xxx_get_tkip_mmic_err;
	hal_ops->cmd_cci = ssv6xxx_cmd_cci;
	hal_ops->cmd_txgen = ssv6xxx_cmd_txgen;
	hal_ops->cmd_rf = ssv6xxx_cmd_rf;
	hal_ops->cmd_hwq_limit = ssv6xxx_cmd_hwq_limit;
	hal_ops->update_rf_pwr = ssv6xxx_update_rf_pwr;
    hal_ops->init_gpio_cfg = ssv6xxx_init_gpio_cfg;
    hal_ops->write_efuse = ssv6xxx_write_efuse;
    hal_ops->update_efuse_setting = ssv6xxx_update_efuse_setting;
    hal_ops->set_on3_enable = ssv6xxx_set_on3_enable;
}


int ssv6xxx_init_hal(struct ssv_softc *sc)
{
    struct ssv_hw *sh;
    int ret = 0;
    struct ssv_hal_ops *hal_ops = NULL;
    extern void ssv_attach_ssv6051(struct ssv_softc *sc, struct ssv_hal_ops *hal_ops);
    extern void ssv_attach_ssv6006(struct ssv_softc *sc, struct ssv_hal_ops *hal_ops);
    bool chip_supportted = false;

	// alloc hal_ops memory
	hal_ops = kzalloc(sizeof(struct ssv_hal_ops), GFP_KERNEL);
	if (hal_ops == NULL) {
		printk("%s(): Fail to alloc hal_ops\n", __FUNCTION__);
		return -ENOMEM;
	}

    // load common HAL layer function;
    ssv6xxx_attach_common_hal(hal_ops);

#ifdef SSV_SUPPORT_SSV6051
    // load individual HAL function && initialize MAC register
    if (   strstr(sc->platform_dev->id_entry->name, SSV6051_CHIP)
        || strstr(sc->platform_dev->id_entry->name, SSV6051_CHIP_ECO3)) {
        printk(KERN_INFO"Attach SSV6051 family HAL function \n");
        ssv_attach_ssv6051(sc, hal_ops);
        chip_supportted = true;
    }
#endif
#ifdef SSV_SUPPORT_SSV6006
    if (   strstr(sc->platform_dev->id_entry->name, SSV6006) 
        	 || strstr(sc->platform_dev->id_entry->name, SSV6006MP)
        	 || strstr(sc->platform_dev->id_entry->name, SSV6166)) {
        
    	if (strstr(sc->platform_dev->id_entry->name, SSV6006MP)) {
            printk(KERN_INFO"SSV6006 MP \n");
        } else if (strstr(sc->platform_dev->id_entry->name, SSV6166)){
            printk(KERN_INFO"SSV6166 \n");
        }
        printk(KERN_INFO"Attach SSV6006 family HAL function  \n");

        ssv_attach_ssv6006(sc, hal_ops);
        chip_supportted = true;
    }
#endif    
    if (!chip_supportted) {

        printk(KERN_ERR "Chip \"%s\" is not supported by this driver\n", sc->platform_dev->id_entry->name);
        ret = -EINVAL;
        goto out;
    }

    sh = hal_ops->alloc_hw();
    if (sh == NULL) {
        ret = -ENOMEM;
        goto out;
    }

    memcpy(&sh->hal_ops, hal_ops, sizeof(struct ssv_hal_ops));
    sc->sh = sh;
    sh->sc = sc;
    INIT_LIST_HEAD(&sh->hw_cfg);
    sh->priv = sc->dev->platform_data;
    sh->hci.dev = sc->dev;
    sh->hci.if_ops = sh->priv->ops;
    sh->hci.skb_alloc = ssv_skb_alloc;
    sh->hci.skb_free = ssv_skb_free;
    sh->hci.hci_rx_cb = ssv6200_rx;
    sh->hci.hci_is_rx_q_full = ssv6200_is_rx_q_full;

    sh->priv->skb_alloc = ssv_skb_alloc_ex;
    sh->priv->skb_free = ssv_skb_free;
    sh->priv->skb_param = sc;

	// Set pm suspend/resume functions for HWIF
#ifdef CONFIG_PM
	sh->priv->suspend = ssv6xxx_power_sleep;
	sh->priv->resume = ssv6xxx_power_awake;
	sh->priv->pm_param = sc;
#endif

    // Set jump to rom functions for HWIF
    sh->priv->enable_usb_acc = ssv6xxx_enable_usb_acc;    
    sh->priv->disable_usb_acc = ssv6xxx_disable_usb_acc;
    sh->priv->jump_to_rom = ssv6xxx_jump_to_rom;
    sh->priv->usb_param = sc;
    
    // Rx burstread size for HWIF
    sh->priv->rx_burstread_size = ssv6xxx_rx_burstread_size;
    sh->priv->rx_burstread_param = sc;

    sh->hci.sc = sc;
    sh->hci.sh = sh;

out:
	kfree(hal_ops);
    return ret;
}

#endif // SSV_SUPPORT_HAL
