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

// Include defines from config.mak to feed eclipse defines from ccflags-y
#ifdef ECLIPSE
#include <ssv_mod_conf.h>
#endif // ECLIPSE
#include <linux/version.h>
#ifdef SSV_SUPPORT_HAL
#include <linux/nl80211.h>
#include <ssv6200.h>
#include <ssv6200_reg.h>
#include <ssv6200_aux.h>
#include <smac/dev.h>
#include <hal.h>
#include <ssvdevice/ssv_cmd.h>

static bool ssv6051_cabrioA_set_rf_enable(struct ssv_hw *sh, bool val);

ssv_cabrio_reg ssv6051_cabrioA_phy_setting[]={
    {0xce000000, 0x0000001e},   // CabrioA/E difference
    {0xce000008, 0x0000006a},
    {0xce00000c, 0x00000064},
    {0xce000010, 0x00007FFF},
    {0xce000014, 0x00000003},
    {0xce000018, 0x0055003C},
    {0xce00001c, 0x00000064},
    {0xce000020, 0x20000000},
    {0xce00002c, 0x00000000},
    {0xce000030, 0x80046072},
    {0xce000034, 0x1f300f6f},
    {0xce000038, 0x660F36D0},
    {0xce00003c, 0x106C0004},
    {0xce000040, 0x01600500},
    {0xce000044, 0x00600008},
    {0xce000048, 0xff000160},
    {0xce00004c, 0x00000840},
    {0xce000060, 0x01000405},
    {0xce000064, 0x06090813},
    {0xce000068, 0x12070000},
    {0xce00006c, 0x01000405},
    {0xce000070, 0x06090813},
    {0xce000074, 0x12010000},
    {0xce000078, 0x00000000},
    {0xce00007c, 0x10110003},
    {0xce000080, 0x0110000F},
    {0xce000084, 0x00000000},
    {0xce000088, 0x00000000},
    {0xce000094, 0x01012425},
    {0xce000098, 0x01010101},
    {0xce00009c, 0x00000011},
    {0xce0000a0, 0x1fff0000},
    {0xce0000a4, 0x1fff0000},
    {0xce0000a8, 0x1fff0000},
    {0xce0000ac, 0x1fff0000},
    {0xce0000b8, 0x0000fe3e},
    {0xce0000fc, 0xffffffff},
    {0xce000108, 0x0ead04f5},
    {0xce00010c, 0x0fd60080},
    {0xce000110, 0x00000009},
    {0xce0010a4, 0x0000002c},
    {0xce0010b4, 0x00003001},
    {0xce0010d4, 0x00000001},
    {0xce002000, 0x00000044},
    {0xce002004, 0x00040000},
    {0xce002008, 0x20400050},
    {0xce00200c, 0x00003467},
    {0xce002010, 0x00540000},
    {0xce002014, 0x20304015},
    {0xce002018, 0x00390002},
    {0xce00201c, 0x02333567},
    {0xce002020, 0x00350046},
    {0xce002024, 0x00570057},
    {0xce002028, 0x00236700},
    {0xce00202c, 0x000d1746},
    {0xce002030, 0x04061787},
    {0xce002034, 0x07800000},
    {0xce00209c, 0x00C0000A},
    {0xce0020a0, 0x00000000},
    {0xce0023f8, 0x00000000},
    {0xce0023fc, 0x00000001},
    {0xce0030a4, 0x00001901},
    {0xce0030b8, 0x5d08908e},
    {0xce004000, 0x00000044},
    {0xce004004, 0x00750075},
    {0xce004008, 0x00000075},
    {0xce00400c, 0x10000075},
    {0xce004010, 0x3F384905},
    {0xce004014, 0x40182000},
    {0xce004018, 0x18580000},
    {0xce00401c, 0x0C010120},
    {0xce004020, 0x50505050},
    {0xce004024, 0x50000000},
    {0xce004028, 0x50505050},
    {0xce00402c, 0x506070A0},
    {0xce004030, 0xF0000000},
    {0xce004034, 0x00002424},
    {0xce004038, 0x00001420},
    {0xce00409c, 0x0000300A},
    {0xce0040c0, 0x40000280},
    {0xce0040c4, 0x30023002},
    {0xce0040c8, 0x0000003a},
    {0xce004130, 0x40000000},
    {0xce004164, 0x009C007E},
    {0xce004180, 0x00044400},
    {0xce004188, 0x82000000},
    {0xce004190, 0x00001820},
    {0xce004194, 0xffffffff},
    {0xce004380, 0x00700010},
    {0xce004384, 0x00007575},
    {0xce004388, 0x0001fe3e},
    {0xce00438c, 0x0000fe3e},
    {0xce0043f8, 0x00000001},
    {0xce0043fc, 0x000004e1},    // CabrioA/E difference
    // {0xce007044, 0x00028080},    // should be set in iq-k registers
    // {0xce0071bc, 0x80808080},    // should be set in iq-k registers
    // {0xce000004, 0x0000017f},    // mode & enable, shoud not be included in default config
    {0xce007000, 0x00000000},
    {0xce007004, 0x00008000},
    {0xce007008, 0x00000000},
    {0xce00700c, 0x00000000},
    {0xce007010, 0x00000000},
    {0xce007014, 0x00000000},
    {0xce007018, 0x00000000},
    {0xce00701c, 0x00000000},
    {0xce007020, 0x00000000},
    {0xce007024, 0x00000000},
    {0xce007028, 0x00000000},
    {0xce00702c, 0x00000000},
    {0xce007030, 0x00000000},
    {0xce007034, 0x00000000},
    {0xce007038, 0x00000000},
    {0xce00703c, 0x00000000},
    {0xce007040, 0x02000200},
    {0xce007044, 0x00038080},   // CabrioA/E difference
    {0xce007048, 0x00000000},
    {0xce00704c, 0x00000000},
    {0xce007050, 0x00000000},
    {0xce007054, 0x00000000},
    {0xce007058, 0x000028ff},
    {0xce00705c, 0x00000000},
    {0xce007060, 0x00000000},
    {0xce007064, 0x00000000},
    {0xce007068, 0x00000000},
    {0xce00706c, 0x00000202},
    {0xce007070, 0x80ffc200},
    {0xce007074, 0x00000000},
    {0xce007078, 0x00000000},
    {0xce00707c, 0x00000000},
    {0xce007080, 0x00000000},
    {0xce007084, 0x00000000},
    {0xce007088, 0x00000000},
    {0xce00708c, 0x00000000},
    {0xce007090, 0x00000000},
    {0xce007094, 0x00000000},
    {0xce007098, 0x00000000},
    {0xce00709c, 0x00000000},
    {0xce0070a0, 0x00000000},
    {0xce0070a4, 0x00000000},
    {0xce0070a8, 0x00000000},
    {0xce0070ac, 0x00000000},
    {0xce0070b0, 0x00000000},
    {0xce0070b4, 0x00000000},
    {0xce0070b8, 0x00000000},
    {0xce0070bc, 0x00000000},
    {0xce0070c0, 0x00000000},
    {0xce0070c4, 0x00000000},
    {0xce0070c8, 0x00000000},
    {0xce0070cc, 0x00000000},
    {0xce0070d0, 0x00000000},
    {0xce0070d4, 0x00000000},
    {0xce0070d8, 0x00000000},
    {0xce0070dc, 0x00000000},
    {0xce0070e0, 0x00000000},
    {0xce0070e4, 0x00000000},
    {0xce0070e8, 0x00000000},
    {0xce0070ec, 0x00000000},
    {0xce0070f0, 0x00000000},
    {0xce0070f4, 0x00000000},
    {0xce0070f8, 0x00000000},
    {0xce0070fc, 0x00000000},
    {0xce007100, 0x00000000},
    {0xce007104, 0x00000000},
    {0xce007108, 0x00000000},
    {0xce00710c, 0x00000000},
    {0xce007110, 0x00000000},
    {0xce007114, 0x00000000},
    {0xce007118, 0x00000000},
    {0xce00711c, 0x00000000},
    {0xce007120, 0x02000200},
    {0xce007124, 0x02000200},
    {0xce007128, 0x02000200},
    {0xce00712c, 0x02000200},
    {0xce007130, 0x02000200},
    {0xce007134, 0x02000200},
    {0xce007138, 0x02000200},
    {0xce00713c, 0x02000200},
    {0xce007140, 0x02000200},
    {0xce007144, 0x02000200},
    {0xce007148, 0x02000200},
    {0xce00714c, 0x02000200},
    {0xce007150, 0x02000200},
    {0xce007154, 0x02000200},
    {0xce007158, 0x00000000},
    {0xce00715c, 0x00000000},
    {0xce007160, 0x00000000},
    {0xce007164, 0x00000000},
    {0xce007168, 0x00000000},
    {0xce00716c, 0x00000000},
    {0xce007170, 0x00000000},
    {0xce007174, 0x00000000},
    {0xce007178, 0x00000000},
    {0xce00717c, 0x00000000},
    {0xce007180, 0x00000000},
    {0xce007184, 0x00000000},
    {0xce007188, 0x00000000},
    {0xce00718c, 0x00000000},
    {0xce007190, 0x00000000},
    {0xce007194, 0x00000000},
    {0xce007198, 0x00000000},
    {0xce00719c, 0x00000000},
    {0xce0071a0, 0x00000000},
    {0xce0071a4, 0x00000000},
    {0xce0071a8, 0x00000000},
    {0xce0071ac, 0x00000000},
    {0xce0071b0, 0x00000000},
    {0xce0071b4, 0x00000100},
    {0xce0071b8, 0x00000000},
    {0xce0071bc, 0x79807980},//TX gain
    {0xce0071c0, 0x00000000},
    {0xce0071c4, 0x00000000},
    {0xce0071c8, 0x00000000},
    {0xce0071cc, 0x00000000},
    {0xce0071d0, 0x00000000},
    {0xce0071d4, 0x00000000},
    {0xce0071d8, 0x00000000},
    {0xce0071dc, 0x00000000},
    {0xce0071e0, 0x00000000},
    {0xce0071e4, 0x00000000},
    {0xce0071e8, 0x00000000},
    {0xce0071ec, 0x00000000},
    {0xce0071f0, 0x00000000},
    {0xce0071f4, 0x00000000},
    {0xce0071f8, 0x00000000},
    {0xce0071fc, 0x00000000},
};

static const size_t ssv6051_cabrioA_phy_tbl_size = sizeof(ssv6051_cabrioA_phy_setting);

static void ssv6051_cabrioA_load_phy_table(ssv_cabrio_reg **phy_table)
{
    *phy_table = ssv6051_cabrioA_phy_setting;
}

static u32 ssv6051_cabrioA_get_phy_table_size(struct ssv_hw *sh)
{
    return (u32) ssv6051_cabrioA_phy_tbl_size;
}

ssv_cabrio_reg ssv6051_cabrioA_rf_setting[]=
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

static const size_t ssv6051_cabrioA_rf_tbl_size = sizeof(ssv6051_cabrioA_rf_setting);

static void ssv6051_cabrioA_load_rf_table(ssv_cabrio_reg **rf_table)
{
    *rf_table = ssv6051_cabrioA_rf_setting;
}

static u32 ssv6051_cabrioA_get_rf_table_size(struct ssv_hw *sh)
{
    return (u32) ssv6051_cabrioA_rf_tbl_size;
}

static void ssv6051_cabrioA_init_PLL(struct ssv_hw *sh)
{
   HAL_SET_PHY_MODE(sh, false); /* disable all phy mode */

   /*
        //Xctal setting allway 40M for CABRIO A
            {0xCE010038, 0x0003E07C},   // xtal dependent           this register for SDIO clock
            {0xCE010060, 0x00406000},   // xtal dependent           this register for SDIO clock          
            {0xCE01009C, 0x00000024},   // xtal dependent          this register for SDIO clock
            {0xCE0100A0, 0x00EC4CC5},   // xtal dependent          this register for SDIO clock
 
     */
    //0xCE010038
    SMAC_REG_WRITE(sh, ADR_SX_ENABLE_REGISTER, 0); //disable

    //0xCE010060
    SMAC_REG_WRITE(sh, ADR_DPLL_DIVIDER_REGISTER, 0x00406000);
    //0xCE01009C
    SMAC_REG_WRITE(sh, ADR_DPLL_FB_DIVIDER_REGISTERS_I, 0x00000024);
    //0xCE0100A0
    SMAC_REG_WRITE(sh, ADR_DPLL_FB_DIVIDER_REGISTERS_II, 0x00EC4CC5);

    //0xCE010038
    SMAC_REG_WRITE(sh, ADR_SX_ENABLE_REGISTER, 0x0003E07C); //enable register after setting
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

static int ssv6051_cabrioA_set_channel(struct ssv_softc *sc, struct ieee80211_channel *chan, 
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
        
        ssv6051_cabrioA_set_rf_enable(sc->sh, false);
        
        /* step 1: SET_CBR_RG_SX_REFBYTWO*/
        if ((ret = SMAC_REG_SET_BITS(sc->sh, ADR_CBR_SYN_DIV_SDM_XOSC, 
            (0x01<<13), (0x01<<13))) != 0) break;
        
        /* step 2: SET_CBR_RG_SX_RFCTRL_F */
        regval = vt_tbl[chidx].rf_ctrl_F;
        if ((ret = SMAC_REG_SET_BITS(sc->sh, ADR_CBR_SYN_RGISTER_1,
            (regval << 0), 0x00ffffff)) != 0) break;

        /* SET_CBR_RG_SX_RFCTRL_CH */
        regval = vt_tbl[chidx].rf_ctrl_N;
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_CBR_SYN_RGISTER_2,
            (regval<<0), 0x000007ff)) != 0) break;

        /* SET_CBR_RG_SX_SUB_SEL_CWD */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_CBR_MANUAL_REGISTER,
            (64<<1), (0x000007f<<1))) != 0) break;

        /* SET_CBR_RG_SX_SUB_SEL_CWR */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_CBR_MANUAL_REGISTER,
            (1<<0), 0x00000001)) != 0) break;

        /* SET_CBR_RG_SX_SUB_SEL_CWR */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_CBR_MANUAL_REGISTER,
            (0<<0), 0x00000001)) != 0) break;
        
        /* step 3: calibration, SET_CBR_RG_EN_SX_VT_MON */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_CBR_SX_ENABLE_RGISTER,
            (1<<11), 0x00000800)) != 0) break;
        
        /* SET_CBR_RG_EN_SX_VT_MON_DG */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_CBR_SX_ENABLE_RGISTER,
            (0<<12), 0x00001000)) != 0) break;

        /* SET_CBR_RG_EN_SX_VT_MON_DG */
        if ((ret = SMAC_REG_SET_BITS(sh, ADR_CBR_SX_ENABLE_RGISTER,
            (1<<12), 0x00001000)) != 0) break;

        for(retry_cnt=20; retry_cnt>0; retry_cnt--){
            mdelay(20);

            /* GET_CBR_VT_MON_RDY */
            if ((ret = SMAC_REG_READ(sh, ADR_CBR_READ_ONLY_FLAGS_1, &regval)) != 0) break;
            if (regval & 0x00000004){
                /* SET_CBR_RG_EN_SX_VT_MON_DG */
                if ((ret = SMAC_REG_SET_BITS(sh, ADR_CBR_SX_ENABLE_RGISTER,
                (0<<12), 0x00001000)) != 0) break;

                if ((ret = SMAC_REG_READ(sh, ADR_CBR_READ_ONLY_FLAGS_1, &regval)) != 0) break;
                if ((regval & 0x00001800) == 0){
                    ssv6051_cabrioA_set_rf_enable(sh, true);
//                    printk("%s(): Lock channel %d success !\n", __FUNCTION__, vt_tbl[chidx].channel_id);

                    return 0;
                } else { // dbg code add by bernie, begin
                    printk("%s(): Lock channel %d fail!\n", __FUNCTION__, vt_tbl[chidx].channel_id);

                    if ((ret = SMAC_REG_READ(sh, ADR_CBR_READ_ONLY_FLAGS_1, &regval)) != 0) break;
                    printk("%s(): dbg: vt-mon read out as %d when rdy\n", __FUNCTION__,  ((regval & 0x00001800) >> 11));

                    if ((ret = SMAC_REG_READ(sh, ADR_CBR_READ_ONLY_FLAGS_2, &regval)) != 0) break;
                    printk("%s(): dbg: sub-sel read out as %d when rdy\n", __FUNCTION__, ((regval & 0x00000fe0) >>  5));

                    if ((ret = SMAC_REG_READ(sh, ADR_CBR_SYN_DIV_SDM_XOSC, &regval)) != 0) break;
                    printk("%s(): dbg: RG_SX_REFBYTWO read out as %d when rdy\n", __FUNCTION__, ((regval & 0x00002000) >>  13));

                    if ((ret = SMAC_REG_READ(sh, ADR_CBR_SYN_RGISTER_1, &regval)) != 0) break;
                    printk("%s(): dbg: RG_SX_RFCTRL_F read out as 0x%08x when rdy\n", __FUNCTION__, ((regval & 0x00ffffff) >>  0));

                    if ((ret = SMAC_REG_READ(sh, ADR_CBR_SYN_RGISTER_2, &regval)) != 0) break;
                    printk("%s(): dbg: RG_SX_RFCTRL_CH read out as 0x%08x when rdy\n", __FUNCTION__, ((regval & 0x000007ff) >>  0));

                    if ((ret = SMAC_REG_READ(sh, ADR_CBR_SX_ENABLE_RGISTER, &regval)) != 0) break;
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

static int ssv6051_cabrioA_set_pll_phy_rf(struct ssv_hw *sh
    , ssv_cabrio_reg *rf_tbl, ssv_cabrio_reg *phy_tbl)
{
    int  ret;

    ret = SSV6XXX_SET_HW_TABLE(sh, ssv6051_cabrioA_rf_setting);
    HAL_INIT_PLL(sh);
    HAL_SET_PHY_MODE(sh, false); /* disable all phy mode */
    if (ret == 0) ret = SSV6XXX_SET_HW_TABLE(sh, ssv6051_cabrioA_phy_setting);
    return ret;
} 

static bool ssv6051_cabrioA_set_rf_enable(struct ssv_hw *sh, bool val)
{
	  if (val){
        return SMAC_REG_SET_BITS(sh, ADR_CBR_HARD_WIRE_PIN_REGISTER,
                (RF_MODE_TRX_EN << CBR_RG_MODE_SFT), CBR_RG_MODE_MSK);
	  } else {
	      return SMAC_REG_SET_BITS(sh,ADR_CBR_HARD_WIRE_PIN_REGISTER,
                (RF_MODE_STANDBY << CBR_RG_MODE_SFT), CBR_RG_MODE_MSK);
	  } 
}

static bool ssv6051_cabrioA_dump_phy_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6051_cabrioA_phy_setting;

    snprintf_res(cmd_data, ">> PHY Register Table:\n");
    
    for(s = 0; s < ssv6051_cabrioA_phy_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);
    }
    snprintf_res(cmd_data, "\n\n");
    return 0;
}

static bool ssv6051_cabrioA_dump_rf_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
 
    raw = ssv6051_cabrioA_rf_setting;

    snprintf_res(cmd_data, ">> RF Register Table:\n");

    for(s = 0; s < ssv6051_cabrioA_rf_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);
    }
    snprintf_res(cmd_data, "\n\n");

    return 0;
}


static bool ssv6051_cabrioA_support_iqk_cmd(struct ssv_hw *sh)
{
    return false;
}

void ssv_attach_ssv6051_cabrioA_BBRF(struct ssv_hal_ops *hal_ops)
{
    hal_ops->load_phy_table = ssv6051_cabrioA_load_phy_table;
    hal_ops->get_phy_table_size = ssv6051_cabrioA_get_phy_table_size;
    hal_ops->load_rf_table = ssv6051_cabrioA_load_rf_table;
    hal_ops->get_rf_table_size = ssv6051_cabrioA_get_rf_table_size;
    hal_ops->init_pll = ssv6051_cabrioA_init_PLL;
    hal_ops->set_channel = ssv6051_cabrioA_set_channel;
    hal_ops->set_pll_phy_rf = ssv6051_cabrioA_set_pll_phy_rf;
    hal_ops->set_rf_enable = ssv6051_cabrioA_set_rf_enable;
    
    hal_ops->dump_phy_reg = ssv6051_cabrioA_dump_phy_reg;
    hal_ops->dump_rf_reg = ssv6051_cabrioA_dump_rf_reg;
    hal_ops->support_iqk_cmd = ssv6051_cabrioA_support_iqk_cmd;
}
#endif // SSV_SUPPORT_HAL
