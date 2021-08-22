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
#if ((defined SSV_SUPPORT_HAL) && (defined SSV_SUPPORT_SSV6006) && \
    (defined SSV6006_SUPPORT_CABRIOA))
#include <linux/nl80211.h>
#include <ssv6200.h>
#include "ssv6006B_reg.h"
#include "ssv6006B_aux.h"
#include <smac/dev.h>
#include <hal.h>
#include <ssvdevice/ssv_cmd.h>
#include "ssv6006_priv.h"
#include "ssv6006_priv_normal.h"
static bool ssv6006_cabrioA_set_rf_enable(struct ssv_hw *sh, bool val);
  
// this table is from turismo fpga released package.
// original file name is "turismo_wifi_phy_initialize_ht20.js"
ssv_cabrio_reg ssv6006_cabrioA_phy_setting[]={
    // turn off phy
    // can be ignored already turn off at init_hw before load table.
    //{0xccb0e004,0x00000000};
    
    // Tx packet generator
    {0xccb0e010,0x00000FFF},
    {0xccb0e018,0x0055003C},
    {0xccb0e01C,0x00000064},
    {0xccb0e020,0x20000000},
    
    // AGC
    {0xccb0e030,0x80046072},
    {0xccb0e034,0x1F300F6f},
    {0xccb0e038,0x660F36D0},
    {0xccb0e03C,0x106c0004},
    
    // DAGC 11b
    {0xccb0e040,0x01600400},
    {0xccb0e044,0x00080868},
    
    // DAGC 11gn HT20
    {0xccb0e048,0xFF000160},
    {0xccb0e04C,0x00100040},
    
    // DAGC 11gn HT40
    {0xccb0e12C,0x00000160},
    {0xccb0e130,0x00100040},
    
    {0xccb0e134,0x00100010},
    
    // RSSI
    {0xccb0e080,0x0110000F},
    
    // RF ramp up ramp down
    {0xccb0e094,0x01012425},
    {0xccb0e098,0x01010101},
    
    // Tx 11gn setting
    {0xccb0ecA4,0x00008001},
    {0xccb0ecB8,0xFF0C50CC},
    
    // Tx IQ swap
    {0xccb0fc44,0x00038080}, // Tx IQ swap                         
    // comment out , since it is not loop back mode
 //   {0xccb0fc44,0x00028080}, // loop back do not need Tx IQ swap,
    
    // RX 11gn setting
    {0xccb0f00c,0x10000075},
    {0xccb0f010,0x3F384905},
    {0xccb0f014,0x40182000}, // modify auto 32 threshold for better symbol boundary
    {0xccb0f018,0x28680000},
    {0xccb0f01C,0x0c010080},
    {0xccb0f03C,0x00000040},
    
    {0xccb0f020,0x50505050},
    {0xccb0f024,0x50000000},
    {0xccb0f028,0x50505050},
    {0xccb0f02c,0x50505050},
    {0xccb0f030,0x50000000},
    {0xccb0f034,0x00002424},
    
    {0xccb0f09c,0x0000300A},
    {0xccb0f0C0,0x40000280},
    {0xccb0f0C4,0x30023002},
    
    {0xccb0f130,0x40000000},
    {0xccb0f164,0x009E0090},
    {0xccb0f188,0x83000000},
    {0xccb0f190,0x00000020}, // modify pkt format detection threshold
    {0xccb0f3F8,0x00100001},
    {0xccb0f3FC,0x00010421},
    
    // 11b Rx
    {0xccb0e804,0x00040000},
    {0xccb0e808,0x20380050}, // b_CCA with index check  
    {0xccb0e814,0x20304015},
    {0xccb0e818,0x00390002},
    {0xccb0e81C,0x02333333},
    {0xccb0e830,0x04061787},
    {0xccb0e89c,0x00c0000A},
    {0xccb0e8A0,0x00000000},
    {0xccb0ebF8,0x00100000},
    {0xccb0ebFC,0x00000001},
           
    // turn on phy, 
    // can be ignored already at last step in init_hw    
    //{0xccb0e004,0x0000017F},
    
    //end
};

static const size_t ssv6006_cabrioA_phy_tbl_size = sizeof(ssv6006_cabrioA_phy_setting);

static void ssv6006_cabrioA_load_phy_table(ssv_cabrio_reg **phy_table)
{
    *phy_table = ssv6006_cabrioA_phy_setting;
}

static u32 ssv6006_cabrioA_get_phy_table_size(struct ssv_hw *sh)
{
    return(u32) ssv6006_cabrioA_phy_tbl_size;
}

ssv_cabrio_reg ssv6006_cabrioA_rf_setting[]=
{
	{0xcb110000,0x5F00EFCE},
	{0xcb110004,0x00001FC0},
	{0xcb110008,0x1C96CA3A},
	{0xcb11000c,0x15155A74},
	{0xcb110010,0x01011A88},
	{0xcb110014,0x3CBF703C},
	{0xcb110018,0x00057579},
	{0xcb11001c,0x000103A7},
	{0xcb110020,0x000103A6},
	{0xcb110024,0x00012001},
	{0xcb110028,0x00036000},
	{0xcb11002c,0x00000CA8},
	{0xcb110030,0x002A0224},
	{0xcb110034,0x00001E55},
	{0xcb110038,0x00006C7C},
	{0xcb11003c,0x55666666},
	{0xcb110040,0x005508F8},
	{0xcb110044,0x07C08BFF},
	{0xcb110048,0xF1111A27},
	{0xcb11004c,0x2773F53C},
	{0xcb110050,0x00000A7C},
	{0xcb110054,0x00087FF8},
	{0xcb110058,0x00103014},
	{0xcb11005c,0x0000848A},
	{0xcb110060,0x00406030},
	{0xcb110064,0x00820820},
	{0xcb110068,0x00820820},
	{0xcb11006c,0x00820820},
	{0xcb110070,0x00820820},
	{0xcb110074,0x00820820},
	{0xcb110078,0x00820820},
	{0xcb11007c,0x00820820},
	{0xcb110080,0x00820820},
	{0xcb110084,0x00004080},
	{0xcb110088,0x00003EAA},
	{0xcb11008c,0x5E00FFEB},
	{0xcb110090,0xAAAAAAAA},
	{0xcb110094,0x0000243F},
	{0xcb110098,0x00018B10},

	{0xcb120080,0x00000000},
	{0xcb120084,0x00000000},
	{0xcb120088,0x00000000},
	{0xcb120090,0x00000813},
	{0xcb120094,0x00000000},
	{0xcb1203f8,0xFF000000},
};

static const size_t ssv6006_cabrioA_rf_tbl_size = sizeof(ssv6006_cabrioA_rf_setting);

static void ssv6006_cabrioA_load_rf_table(ssv_cabrio_reg **rf_table)
{
    *rf_table = ssv6006_cabrioA_rf_setting;
}

static u32 ssv6006_cabrioA_get_rf_table_size(struct ssv_hw *sh)
{
    return (u32) ssv6006_cabrioA_rf_tbl_size;
}

static void ssv6006_cabrioA_init_PLL(struct ssv_hw *sh)
{

    //0xccb0b004
    //ToDo : Liam 0xccb0b004 register name might be changed.
    // for turismo, it just needs to set register once , pll is initialized by hw auto.
    SMAC_REG_WRITE(sh, ADR_PMU_REG_2, 0xa51a8810);
    msleep(10);
}

static const struct ssv6xxx_calib_table vt_tbl[] =
{
    /* Table for Cabrio A  40M */
    {  1, 0xf1, 0x333333, 3859},
    {  2, 0xf1, 0xB33333, 3867},
    {  3, 0xf2, 0x333333, 3875},
    {  4, 0xf2, 0xB33333, 3883},
    {  5, 0xf3, 0x333333, 3891},
    {  6, 0xf3, 0xB33333, 3899},
    {  7, 0xf4, 0x333333, 3907},
    {  8, 0xf4, 0xB33333, 3915},
    {  9, 0xf5, 0x333333, 3923},
    { 10, 0xf5, 0xB33333, 3931},
    { 11, 0xf6, 0x333333, 3939},
    { 12, 0xf6, 0xB33333, 3947},
    { 13, 0xf7, 0x333333, 3955},
    { 14, 0xf8, 0x666666, 3974},
};

static int ssv6006_cabrioA_set_channel(struct ssv_softc *sc, struct ieee80211_channel  *chan, 
    enum nl80211_channel_type channel_type)
{
    struct ssv_hw *sh = sc->sh;
    int ch;
    int retry_cnt, fail_cnt=0;
    u32 regval;
    int ret = 0;
    int  chidx;
    bool chidx_vld = 0;

    ch = chan->hw_value;  

    for(chidx = 0; chidx < (sizeof(vt_tbl)/sizeof(vt_tbl[0])); chidx++) {
        if (vt_tbl[chidx].channel_id == ch) {
            chidx_vld = 1;
            break;
        }
    }

    if (chidx_vld == 0) {
        printk("%s(): fail! channel_id not found in vt_tbl\n", __FUNCTION__);
        return -1;
    }

    do {
        //this set the clock width of spi to phy
        //from 6 --> 5
        if ((ret = SMAC_REG_READ(sh, ADR_SPI_TO_PHY_PARAM1, &regval)) != 0) break;
        if ((ret = SMAC_REG_WRITE(sh,ADR_SPI_TO_PHY_PARAM1,(regval&~0xffff)|3)) != 0) break;
        
        ssv6006_cabrioA_set_rf_enable(sc->sh, false);
        
        /* step 1: SET_FPGA_RG_SX_REFBYTWO*/
        if ((ret = SMAC_REG_SET_BITS(sc->sh, ADR_FPGA_SYN_DIV_SDM_XOSC, 
            (0x01<<13), (0x01<<13))) != 0) break;
        
        /* step 2: SET_FPGA_RG_SX_RFCTRL_F */
        regval = vt_tbl[chidx].rf_ctrl_F;
        if ((ret = SMAC_REG_SET_BITS(sc->sh, ADR_FPGA_SYN_RGISTER_1,
            (regval << 0), 0x00ffffff)) != 0) break;

        /* SET_FPGA_RG_SX_RFCTRL_CH */
        regval = vt_tbl[chidx].rf_ctrl_N;
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_FPGA_SYN_RGISTER_2,
            (regval<<0), 0x000007ff)) != 0) break;

        /* SET_FPGA_RG_SX_SUB_SEL_CWD */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_FPGA_MANUAL_REGISTER,
            (64<<1), (0x000007f<<1))) != 0) break;

        /* SET_FPGA_RG_SX_SUB_SEL_CWR */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_FPGA_MANUAL_REGISTER,
            (1<<0), 0x00000001)) != 0) break;

        /* SET_FPGA_RG_SX_SUB_SEL_CWR */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_FPGA_MANUAL_REGISTER,
            (0<<0), 0x00000001)) != 0) break;
        
        /* step 3: calibration, SET_FPGA_RG_EN_SX_VT_MON */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_FPGA_SX_ENABLE_RGISTER,
            (1<<11), 0x00000800)) != 0) break;
        
        /* SET_FPGA_RG_EN_SX_VT_MON_DG */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_FPGA_SX_ENABLE_RGISTER,
            (0<<12), 0x00001000)) != 0) break;

        /* SET_FPGA_RG_EN_SX_VT_MON_DG */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_FPGA_SX_ENABLE_RGISTER,
            (1<<12), 0x00001000)) != 0) break;

        for(retry_cnt=20; retry_cnt>0; retry_cnt--){
            MDELAY(20);

            /* GET_FPGA_VT_MON_RDY */
            if ((ret = SMAC_REG_READ(sh, ADR_FPGA_READ_ONLY_FLAGS_1, &regval)) != 0) break;
            if (regval & 0x00000004){
                /* SET_FPGA_RG_EN_SX_VT_MON_DG */
                if ((ret = SMAC_REG_SET_BITS(sh, ADR_FPGA_SX_ENABLE_RGISTER,
                (0<<12), 0x00001000)) != 0) break;

                if ((ret = SMAC_REG_READ(sh, ADR_FPGA_READ_ONLY_FLAGS_1, &regval)) != 0) break;
                if ((regval & 0x00001800) == 0){
                    ssv6006_cabrioA_set_rf_enable(sh, true);
//                    printk("%s(): Lock channel %d success !\n", __FUNCTION__, vt_tbl[chidx].channel_id);

                    return 0;
                } else { // dbg code add by bernie, begin
                    printk("%s(): Lock channel %d fail!\n", __FUNCTION__, vt_tbl[chidx].channel_id);

                    if ((ret = SMAC_REG_READ(sh, ADR_FPGA_READ_ONLY_FLAGS_1, &regval)) != 0) break;
                    printk("%s(): dbg: vt-mon read out as %d when rdy\n", __FUNCTION__,  ((regval & 0x00001800) >> 11));

                    if ((ret = SMAC_REG_READ(sh, ADR_FPGA_READ_ONLY_FLAGS_2, &regval)) != 0) break;
                    printk("%s(): dbg: sub-sel read out as %d when rdy\n", __FUNCTION__, ((regval & 0x00000fe0) >>  5));

                    if ((ret = SMAC_REG_READ(sh, ADR_FPGA_SYN_DIV_SDM_XOSC, &regval)) != 0) break;
                    printk("%s(): dbg: RG_SX_REFBYTWO read out as %d when rdy\n", __FUNCTION__, ((regval & 0x00002000) >>  13));

                    if ((ret = SMAC_REG_READ(sh, ADR_FPGA_SYN_RGISTER_1, &regval)) != 0) break;
                    printk("%s(): dbg: RG_SX_RFCTRL_F read out as 0x%08x when rdy\n", __FUNCTION__, ((regval & 0x00ffffff) >>  0));

                    if ((ret = SMAC_REG_READ(sh, ADR_FPGA_SYN_RGISTER_2, &regval)) != 0) break;
                    printk("%s(): dbg: RG_SX_RFCTRL_CH read out as 0x%08x when rdy\n", __FUNCTION__, ((regval & 0x000007ff) >>  0));

                    if ((ret = SMAC_REG_READ(sh, ADR_FPGA_SX_ENABLE_RGISTER, &regval)) != 0) break;
                    printk("%s(): dbg: RG_EN_SX_VT_MON_DG read out as %d when rdy\n", __FUNCTION__, ((regval & 0x00001000) >>  12));
                }
                // dbg code add by bernie, end
            }
        }
        
        fail_cnt++;
        printk("%s(): calibration fail [%d] rounds!!\n", 
                __FUNCTION__, fail_cnt);
        if (fail_cnt == 100)
            return -1;

    } while (ret == 0);

    return ret;
}

static int ssv6006_cabrioA_set_pll_phy_rf (struct ssv_hw *sh
    , ssv_cabrio_reg *rf_tbl, ssv_cabrio_reg *phy_tbl)
{	
    int  ret;

    ret = SSV6XXX_SET_HW_TABLE(sh, ssv6006_cabrioA_rf_setting);    
    HAL_INIT_PLL(sh);
    HAL_SET_PHY_MODE(sh, false); /* disable all phy mode */
    if (ret == 0) ret = SSV6XXX_SET_HW_TABLE(sh, ssv6006_cabrioA_phy_setting);
    return ret;
}

static bool ssv6006_cabrioA_set_rf_enable(struct ssv_hw *sh, bool val)
{
	  if (val){
        return SMAC_REG_SET_BITS(sh, ADR_FPGA_HARD_WIRE_PIN_REGISTER,
                (RF_MODE_TRX_EN << FPGA_RG_MODE_SFT), FPGA_RG_MODE_MSK);
	  } else {
	      return SMAC_REG_SET_BITS(sh,ADR_FPGA_HARD_WIRE_PIN_REGISTER,
                (RF_MODE_STANDBY << FPGA_RG_MODE_SFT), FPGA_RG_MODE_MSK);
	  } 
}

static bool ssv6006_cabrioA_dump_phy_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;    
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6006_cabrioA_phy_setting;

    snprintf_res(cmd_data, ">> PHY Register Table:\n"); 
    
    for(s = 0; s < ssv6006_cabrioA_phy_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);
     }
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6006_cabrioA_dump_rf_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6006_cabrioA_rf_setting;

    snprintf_res(cmd_data, ">> RF Register Table:\n");

    for(s = 0; s < ssv6006_cabrioA_rf_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);      
    }
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6006_cabrioA_support_iqk_cmd(struct ssv_hw *sh)
{
    return false;
}

void ssv_attach_ssv6006_cabrioA_BBRF(struct ssv_hal_ops *hal_ops)
{
    hal_ops->load_phy_table = ssv6006_cabrioA_load_phy_table;
    hal_ops->get_phy_table_size = ssv6006_cabrioA_get_phy_table_size;
    hal_ops->get_rf_table_size = ssv6006_cabrioA_get_rf_table_size;
    hal_ops->load_rf_table = ssv6006_cabrioA_load_rf_table;
    hal_ops->init_pll = ssv6006_cabrioA_init_PLL;
    hal_ops->set_channel = ssv6006_cabrioA_set_channel;
    hal_ops->set_pll_phy_rf = ssv6006_cabrioA_set_pll_phy_rf;
    hal_ops->set_rf_enable = ssv6006_cabrioA_set_rf_enable;
    
    hal_ops->dump_phy_reg = ssv6006_cabrioA_dump_phy_reg;
    hal_ops->dump_rf_reg = ssv6006_cabrioA_dump_rf_reg;
    hal_ops->support_iqk_cmd = ssv6006_cabrioA_support_iqk_cmd;
}
#endif