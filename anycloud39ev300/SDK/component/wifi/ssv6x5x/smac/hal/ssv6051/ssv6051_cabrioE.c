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
#include <ssv6200_common.h>
#include <smac/dev.h>
#include <smac/init.h>
#include <smac/lib.h>
#include <hal.h>
#include <smac/ssv_skb.h>
#include <ssvdevice/ssv_cmd.h>

static bool ssv6051_cabrioE_set_rf_enable(struct ssv_hw *sh, bool val);

const u32 wifi_tx_gain[]={
    0x79807980,
    0x72797279,
    0x6C726C72,
    0x666C666C,
    0x60666066,
    0x5B605B60,
    0x565B565B,
    0x51565156,
    0x4C514C51,
    0x484C484C,
    0x44484448,
    0x40444044,
    0x3C403C40,
    0x3A3D3A3D,
    0x36393639,
};

#define IQK_CFG_LEN         (sizeof(struct ssv6xxx_iqk_cfg))

enum ssv_phy_setting_idx 
{
    
    PHY_SET_IDX_TX_GAIN_FACTOR,

    PHY_SET_IDX_MAX,
};

ssv_cabrio_reg ssv6051_cabrioE_phy_setting[]={
     // do not change the position of follow settings.
     // it might be patched by  cfg
     //---------------------------------
    {0xce0071bc, 0x565B565B},//TX gain 
     //----

    {0xce000000, 0x80000016},   // CabrioA/E difference
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
    {0xce000040, 0x01601400},
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
//    {0xce002008, 0x20230080}, //WSD-212, increase cca detect threshold
    {0xce002008, 0x20300050}, // modify default settings
    {0xce00200c, 0x00003467},
    {0xce002010, 0x00430000}, // modify default settings
    {0xce002014, 0x20304015},
    {0xce002018, 0x00390005}, // modify default settings
    {0xce00201c, 0x05555555}, // modify default settings
    {0xce002020, 0x00570057}, // modify default settings
    {0xce002024, 0x00570057},
    {0xce002028, 0x00236700},
    {0xce00202c, 0x000d1746},
    {0xce002030, 0x05061787}, // modify default settings
    {0xce002034, 0x07800000},
    {0xce00209c, 0x00900008}, // modify default settings
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
    {0xce004018, 0x20600000}, // modify default settings
    {0xce00401c, 0x0C010120},
    {0xce004020, 0x50505050},
    {0xce004024, 0x50000000},
    {0xce004028, 0x50505050},
    {0xce00402c, 0x506070A0},
    {0xce004030, 0xF0000000},
    {0xce004034, 0x00002424},
    {0xce004038, 0x00001420},
    {0xce00409c, 0x0000300A},
    {0xce0040c0, 0x20000280}, // modify default settings
    {0xce0040c4, 0x30023002},
    {0xce0040c8, 0x0000003a},
    {0xce004130, 0x40000000},
    {0xce004164, 0x009C007E},
    {0xce004180, 0x00044400},
    {0xce004188, 0x82000000},
    {0xce004190, 0x00000000}, // modify default settings
    {0xce004194, 0xffffffff},
    {0xce004380, 0x00700010},
    {0xce004384, 0x00007575},
    {0xce004388, 0x0001fe3e},
    {0xce00438c, 0x0000fe3e},
    {0xce0043f8, 0x00000001},
    /*
        RIFS  on, STBC off     0xce0043fc  0x000104E3
        RIFS  off, STBC on     0xce0043fc  0x000104E5
        RIFS  on, STBC on     0xce0043fc  0x000104E7
    */
    {0xce0043fc, 0x000104e5},       // CabrioA/E difference
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
    {0xce007044, 0x00028080},   // CabrioA/E difference
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

static const size_t ssv6051_cabrioE_phy_tbl_size = sizeof(ssv6051_cabrioE_phy_setting);

static void ssv6051_cabrioE_load_phy_table(ssv_cabrio_reg **phy_table)
{
    *phy_table = ssv6051_cabrioE_phy_setting;
}

static u32 ssv6051_cabrioE_get_phy_table_size(struct ssv_hw *sh)
{
    return (u32) ssv6051_cabrioE_phy_tbl_size;
}

enum ssv_rf_setting_idx 
{
    RF_ADR_LDO_REGISTER,//                       (CSR_RF_BASE+0x00000008)                    
    RF_ADR_TX_FE_REGISTER,//                     (CSR_RF_BASE+0x00000014)           
    RF_ADR_RX_FE_REGISTER_1,//                   (CSR_RF_BASE+0x00000018)           
    RF_ADR_RX_FE_GAIN_DECODER_REGISTER_1,//      (CSR_RF_BASE+0x0000001c)           
    RF_ADR_RX_FE_GAIN_DECODER_REGISTER_2,//      (CSR_RF_BASE+0x00000020)                    
    RF_ADR_RX_TX_FSM_REGISTER,//                 (CSR_RF_BASE+0x0000002c)                     
    RF_ADR_SYN_VCO_LOBF,//                       (CSR_RF_BASE+0x00000048)                    
    RF_ADR_SYN_KVCO_XO_FINE_TUNE_CBANK,//        (CSR_RF_BASE+0x00000050)

    //0xC0001D08
    RF_SET_IDX_PMU_2,
    RF_SET_IDX_MAX,
};

ssv_cabrio_reg ssv6051_cabrioE_rf_setting[]={
     // do not change the position of follow settings.
     // it might be patched by  cfg
     //---------------------------------
    //Default setting (4.2V & 6051Q)
    {0xCE010008, 0x000DF69B},
    {0xCE010014, 0x3D3E84FE},
    {0xCE010018, 0x01457D79},
    {0xCE01001C, 0x000103A7},
    {0xCE010020, 0x000103A6},
    {0xCE01002C, 0x00032CA8},
    {0xCE010048, 0xFCCCCF27},
    {0xCE010050, 0x00444000},

    {0xC0001D08, 0x00000001},
    //----------- 
    
    {0xCE010000, 0x40002000},
    {0xCE010004, 0x00020FC0},

    {0xCE01000C, 0x151558C5},
    {0xCE010010, 0x01011A88},
    {0xCE010024, 0x00012001},
    {0xCE010028, 0x00036000},
    {0xCE010030, 0x20EA0224},
    {0xCE010034, 0x44000755},
    {0xCE01003C, 0x55D89D8A},
    {0xCE010040, 0x005508BB},
    {0xCE010044, 0x07C08BFF},
    {0xCE01004C, 0x07700830},
    {0xCE010054, 0x00007FF4},
    {0xCE010058, 0x0000000E},
    {0xCE01005C, 0x00088018},
    {0xCE010064, 0x08820820},
    {0xCE010068, 0x00820820},
    {0xCE01006C, 0x00820820},
    {0xCE010070, 0x00820820},
    {0xCE010074, 0x00820820},
    {0xCE010078, 0x00820820},
    {0xCE01007C, 0x00820820},
    {0xCE010080, 0x00820820},
    {0xCE010084, 0x00004080},
    {0xCE010088, 0x200800FE},
    {0xCE01008C, 0xAAAAAAAA},
    {0xCE010090, 0xAAAAAAAA},
    {0xCE010094, 0x0000A487},
    {0xCE010098, 0x0000070E},
    {0xCE0100A4, 0x00000F43},
    {0xCE0100A8, 0x00098900},
    {0xCE0100AC, 0x00000000},
    //
    {0xC00003AC, 0x00000000},   // GPIO registers
    {0xC00003B0, 0x00000000},
    {0xC00003B4, 0x00000000},
    {0xC00003BC, 0x00000000},
    {0xC0001D00, 0x5E000040},   // PMU registers //WSD-191
    {0xC0001D04, 0x015D015D},

    {0xC0001D0C, 0x55550000},
    {0xC0001D20, 0x7FFF0000},
    {0xC0001D24, 0x00000003},
    {0xC0001D28, 0x00000000},
    {0xC0001D2C, 0x00000000},
};

static const size_t ssv6051_cabrioE_rf_tbl_size = sizeof(ssv6051_cabrioE_rf_setting);

static void ssv6051_cabrioE_load_rf_table(ssv_cabrio_reg **rf_table)
{
    *rf_table = ssv6051_cabrioE_rf_setting;
}

static u32 ssv6051_cabrioE_get_rf_table_size(struct ssv_hw *sh)
{
    return (u32) ssv6051_cabrioE_rf_tbl_size;
}

static u32 phy_info_6051z[] =
{
    /* PHY Infor Table: */
    0x18000000, 0x18000100, 0x18000200, 0x18000300, 0x18000140, 
    0x18000240, 0x18000340, 0x0C000001, 0x0C000101, 0x0C000201, 
    0x0C000301, 0x18000401, 0x18000501, 0x18000601, 0x18000701, 
    0x0C030002, 0x0C030102, 0x0C030202, 0x18030302, 0x18030402, 
    0x18030502, 0x18030602, 0x1C030702, 0x0C030082, 0x0C030182, 
    0x0C030282, 0x18030382, 0x18030482, 0x18030582, 0x18030682, 
    0x1C030782, 0x0C030042, 0x0C030142, 0x0C030242, 0x18030342, 
    0x18030442, 0x18030542, 0x18030642, 0x1C030742
};

static void ssv6051_cabrioE_init_PLL(struct ssv_hw *sh)
{
/*
        //Xctal setting
        Remodify RF setting For 24M 26M 40M or other xtals.

        #if SSV_XTAL!=24
            {0xCE010038, 0x0003E07C},   // xtal dependent           this register for SDIO clock
            {0xCE010060, 0x00406000},   // xtal dependent           this register for SDIO clock          
            {0xCE01009C, 0x00000024},   // xtal dependent          this register for SDIO clock
            {0xCE0100A0, 0x00EC4CC5},   // xtal dependent          this register for SDIO clock
        #else
            {0xCE010038, 0x0003E07C},   // xtal dependent           this register for SDIO clock
            {0xCE010060, 0x00406000},   // xtal dependent           this register for SDIO clock          
            {0xCE01009C, 0x00000028},   // xtal dependent          this register for SDIO clock
            {0xCE0100A0, 0x00},   // xtal dependent          this register for SDIO clock
        #endif
    */

    //0xCE010038
    SMAC_REG_WRITE(sh, ADR_SX_ENABLE_REGISTER, 0);//disable
    //0xCE010060
    SMAC_REG_WRITE(sh, ADR_DPLL_DIVIDER_REGISTER, 0x00406000);
    
    if(sh->cfg.crystal_type == SSV6XXX_IQK_CFG_XTAL_26M){
        sh->iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_26M;
        //Parser TX power
        printk("SSV6XXX_IQK_CFG_XTAL_26M\n");
        //0xCE01009C
        SMAC_REG_WRITE(sh, ADR_DPLL_FB_DIVIDER_REGISTERS_I, 0x00000024);
        //0xCE0100A0
        SMAC_REG_WRITE(sh, ADR_DPLL_FB_DIVIDER_REGISTERS_II, 0x00EC4CC5);
    } else if(sh->cfg.crystal_type == SSV6XXX_IQK_CFG_XTAL_40M) {
        sh->iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_40M;
        printk("SSV6XXX_IQK_CFG_XTAL_40M\n");
        SMAC_REG_WRITE(sh, ADR_DPLL_FB_DIVIDER_REGISTERS_I, 0x00000024);
        //0xCE0100A0
        SMAC_REG_WRITE(sh, ADR_DPLL_FB_DIVIDER_REGISTERS_II, 0x00EC4CC5);
    } else if(sh->cfg.crystal_type == SSV6XXX_IQK_CFG_XTAL_24M){
        printk("SSV6XXX_IQK_CFG_XTAL_24M\n");
        sh->iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_24M;
        //0xCE01009C
        SMAC_REG_WRITE(sh, ADR_DPLL_FB_DIVIDER_REGISTERS_I, 0x00000028);
        //0xCE0100A0
        SMAC_REG_WRITE(sh, ADR_DPLL_FB_DIVIDER_REGISTERS_II, 0x00000000);
        
    }else{
        printk("Illegal xtal setting \n");
        BUG_ON(1);
    }


    //Update XTAL/OSC offset form E-FUSE  
    
    SMAC_REG_SET_BITS(sh, ADR_SYN_KVCO_XO_FINE_TUNE_CBANK, 
            (sh->cfg.crystal_frequency_offset << RG_XOSC_CBANK_XO_SFT),
            RG_XOSC_CBANK_XO_I_MSK);
            
   //0xCE010038
    SMAC_REG_WRITE(sh, ADR_SX_ENABLE_REGISTER, 0x0003E07C);//enable

    msleep(10);

}

static void ssv6051_cabrioE_update_cfg_hw_patch(struct ssv_hw *sh, ssv_cabrio_reg *rf_tbl, 
    ssv_cabrio_reg *phy_tbl)
{
	u32 regval;

    if (sh->cfg.force_chip_identity)
    {
        printk("Force use external RF setting [%08x]\n",sh->cfg.force_chip_identity);
        sh->cfg.chip_identity = sh->cfg.force_chip_identity;
    }                                                        

    switch (sh->cfg.chip_identity) {
        case SSV6051Q_P1:
        case SSV6051Q_P2:
        case SSV6051Q:
            printk("SSV6051Q setting\n");
            if (rf_tbl[RF_ADR_LDO_REGISTER].address == ADR_LDO_REGISTER){
               	rf_tbl[RF_ADR_LDO_REGISTER].data = 0x008DF61B;//0xCE010008
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_TX_FE_REGISTER].address == ADR_TX_FE_REGISTER){
               	rf_tbl[RF_ADR_TX_FE_REGISTER].data = 0x3D3E84FE;//0xCE010014
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_REGISTER_1].address == ADR_RX_FE_REGISTER_1){
               	rf_tbl[RF_ADR_RX_FE_REGISTER_1].data = 0x01457D79;//0xCE010018
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_1].address == ADR_RX_FE_GAIN_DECODER_REGISTER_1){
               	rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_1].data = 0x000103A7;//0xCE01001c
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_2].address == ADR_RX_FE_GAIN_DECODER_REGISTER_2){
               	rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_2].data = 0x000103A6;//0xCE010020
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_TX_FSM_REGISTER].address == ADR_RX_TX_FSM_REGISTER){
               	rf_tbl[RF_ADR_RX_TX_FSM_REGISTER].data = 0x00032CA8;//0xCE01002c
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_SYN_VCO_LOBF].address == ADR_SYN_VCO_LOBF){
               	rf_tbl[RF_ADR_SYN_VCO_LOBF].data = 0xFCCCCF27;//0xCE010048
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_SYN_KVCO_XO_FINE_TUNE_CBANK].address == ADR_SYN_KVCO_XO_FINE_TUNE_CBANK){
               	rf_tbl[RF_ADR_SYN_KVCO_XO_FINE_TUNE_CBANK].data = 0x0047C000;//0xCE010050
            } else
                goto Wrong_index;
            break;
        case SSV6051Z:
            printk("SSV6051Z setting\n");
            if (rf_tbl[RF_ADR_LDO_REGISTER].address == ADR_LDO_REGISTER){
               	rf_tbl[RF_ADR_LDO_REGISTER].data = 0x004D561C;//0xCE010008
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_TX_FE_REGISTER].address == ADR_TX_FE_REGISTER){
               	rf_tbl[RF_ADR_TX_FE_REGISTER].data = 0x3D9E84FE;//0xCE010014
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_REGISTER_1].address == ADR_RX_FE_REGISTER_1){
               	rf_tbl[RF_ADR_RX_FE_REGISTER_1].data = 0x00457D79;//0xCE010018
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_1].address == ADR_RX_FE_GAIN_DECODER_REGISTER_1){
               	rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_1].data = 0x000103EB;//0xCE01001c
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_2].address == ADR_RX_FE_GAIN_DECODER_REGISTER_2){
               	rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_2].data = 0x000103EA;//0xCE010020
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_TX_FSM_REGISTER].address == ADR_RX_TX_FSM_REGISTER){
               	rf_tbl[RF_ADR_RX_TX_FSM_REGISTER].data = 0x00062CA8;//0xCE01002c
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_SYN_VCO_LOBF].address == ADR_SYN_VCO_LOBF){
               	rf_tbl[RF_ADR_SYN_VCO_LOBF].data = 0xFCCCCF27;//0xCE010048
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_SYN_KVCO_XO_FINE_TUNE_CBANK].address == ADR_SYN_KVCO_XO_FINE_TUNE_CBANK){
               	rf_tbl[RF_ADR_SYN_KVCO_XO_FINE_TUNE_CBANK].data = 0x0047C000;//0xCE010050
            } else
                goto Wrong_index;
            break;
        case SSV6051P:
            printk("SSV6051P setting\n");
            if (rf_tbl[RF_ADR_LDO_REGISTER].address == ADR_LDO_REGISTER){
               	rf_tbl[RF_ADR_LDO_REGISTER].data = 0x008B7C1C;//0xCE010008
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_TX_FE_REGISTER].address == ADR_TX_FE_REGISTER){
               	rf_tbl[RF_ADR_TX_FE_REGISTER].data = 0x3D7E84FE;//0xCE010014
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_REGISTER_1].address == ADR_RX_FE_REGISTER_1){
               	rf_tbl[RF_ADR_RX_FE_REGISTER_1].data = 0x01457D79;//0xCE010018
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_1].address == ADR_RX_FE_GAIN_DECODER_REGISTER_1){
               	rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_1].data = 0x000103EB;//0xCE01001c
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_2].address == ADR_RX_FE_GAIN_DECODER_REGISTER_2){
               	rf_tbl[RF_ADR_RX_FE_GAIN_DECODER_REGISTER_2].data = 0x000103EA;//0xCE010020
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_RX_TX_FSM_REGISTER].address == ADR_RX_TX_FSM_REGISTER){
               	rf_tbl[RF_ADR_RX_TX_FSM_REGISTER].data = 0x00032CA8;//0xCE01002c
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_SYN_VCO_LOBF].address == ADR_SYN_VCO_LOBF){
               	rf_tbl[RF_ADR_SYN_VCO_LOBF].data = 0xFCCCCC27;//0xCE010048
            } else
                goto Wrong_index;
            if (rf_tbl[RF_ADR_SYN_KVCO_XO_FINE_TUNE_CBANK].address == ADR_SYN_KVCO_XO_FINE_TUNE_CBANK){
               	rf_tbl[RF_ADR_SYN_KVCO_XO_FINE_TUNE_CBANK].data = 0x0047C000;//0xCE010050
            } else
                goto Wrong_index;
            break;
        default:
            printk("No RF setting\n");
            printk("Use default setting [SSV6051Q]\n");
            break;
    }

    //Resetting TX power
    if (phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].address == ADR_TX_GAIN_FACTOR){
        switch (sh->cfg.chip_identity) {
            case SSV6051Q_P1:
            case SSV6051Q_P2:
            case SSV6051Q:
                printk("SSV6051Q setting [0x5B606C72]\n");
                    phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data = 0x5B606C72;
                break;
            case SSV6051Z:
                printk("SSV6051Z setting [0x60606060]\n");
                    phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data = 0x60606060;
                break;
            case SSV6051P:
                printk("SSV6051P setting [0x6C726C72]\n");
                    phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data = 0x6C726C72;
                break;
            default:
                printk("Use default power setting\n");
                break;
        }
        //External B setting
        if (sh->cfg.wifi_tx_gain_level_b){
            phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data &= 0xffff0000;
            phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data |= wifi_tx_gain[sh->cfg.wifi_tx_gain_level_b] & 0x0000ffff;
        }
        //External G/N setting
        if (sh->cfg.wifi_tx_gain_level_gn){
            phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data &= 0x0000ffff;
            phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data |= wifi_tx_gain[sh->cfg.wifi_tx_gain_level_gn] & 0xffff0000;
        }

        printk("TX power setting 0x%x\n",phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data);
        //Setting TX power for RF calibration
        sh->iqk_cfg.cfg_def_tx_scale_11b = (phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data>>0) & 0xff;
        sh->iqk_cfg.cfg_def_tx_scale_11b_p0d5 = (phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data>>8) & 0xff;
        sh->iqk_cfg.cfg_def_tx_scale_11g = (phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data>>16) & 0xff;
        sh->iqk_cfg.cfg_def_tx_scale_11g_p0d5 = (phy_tbl[PHY_SET_IDX_TX_GAIN_FACTOR].data>>24) & 0xff;
    } else
            goto Wrong_index;

    //volt regulator
    if (rf_tbl[RF_SET_IDX_PMU_2].address == ADR_PMU_2){
    	regval = rf_tbl[RF_SET_IDX_PMU_2].data;
        regval &= 0xFFFFFFFE;
        //0xC0001D08
        if(sh->cfg.volt_regulator == SSV6XXX_VOLT_LDO_CONVERT){
            printk("Volt regulator LDO\n");

            regval |= 0x00000000;
            rf_tbl[RF_SET_IDX_PMU_2].data = regval;
        } else if(sh->cfg.volt_regulator == SSV6XXX_VOLT_DCDC_CONVERT) {
            printk("Volt regulator DCDC\n");

            regval |= 0x00000001;
            rf_tbl[RF_SET_IDX_PMU_2].data = regval;
        }
    }
    
    return;

Wrong_index:
    printk(" RF/PHY table patch index incorrect!! Should check it!! \n");


}

static void ssv6051_cabrioE_update_hw_config(struct ssv_hw *sh, ssv_cabrio_reg *rf_table, ssv_cabrio_reg *phy_table)
{
    int i , x = 0;
    extern struct ssv6xxx_cfg ssv_cfg;

    //Force replace configuration table for external configuration
    while(ssv_cfg.configuration[x][0])
    {
        //RF
        for(i = 0; i < ssv6051_cabrioE_rf_tbl_size/ sizeof(ssv_cabrio_reg); i++)
        {

            if(rf_table[i].address == ssv_cfg.configuration[x][0])
            {
                rf_table[i].data = ssv_cfg.configuration[x][1];
                break;
            }
        }
        //PHY
        for(i = 0; i < ssv6051_cabrioE_phy_tbl_size/sizeof(ssv_cabrio_reg); i++)
        {
            if(phy_table[i].address == ssv_cfg.configuration[x][0])
            {
                phy_table[i].data = ssv_cfg.configuration[x][1];
                break;
            }
        }

        x++;
    };

}

static int ssv6051_cabrioE_chg_pad_setting(struct ssv_hw *sh)
{
	int ret = 0 ;
           
    /* Cabrio E: GPIO setting */
    if (ret == 0) ret = SMAC_REG_WRITE(sh, ADR_PAD53, 0x21);        /* like debug-uart config ? */
    if (ret == 0) ret = SMAC_REG_WRITE(sh, ADR_PAD54, 0x3000);
    if (ret == 0) ret = SMAC_REG_WRITE(sh, ADR_PIN_SEL_0, 0x4000);

    /* TR switch: */
    if (ret == 0) ret = SMAC_REG_WRITE(sh, ADR_PAD7, 0x01);
    if (ret == 0) ret = SMAC_REG_WRITE(sh, ADR_PAD8, 0x01);

    return ret;
}

static const struct ssv6xxx_calib_table vt_tbl[SSV6XXX_IQK_CFG_XTAL_MAX][14]=
{
    /* Table for Cabrio E: for XOC_26 only */
    {
        {  1, 0xB9, 0x89D89E, 3859},
        {  2, 0xB9, 0xEC4EC5, 3867},
        {  3, 0xBA, 0x4EC4EC, 3875},
        {  4, 0xBA, 0xB13B14, 3883},
        {  5, 0xBB, 0x13B13B, 3891},
        {  6, 0xBB, 0x762762, 3899},
        {  7, 0xBB, 0xD89D8A, 3907},
        {  8, 0xBC, 0x3B13B1, 3915},
        {  9, 0xBC, 0x9D89D9, 3923},
        { 10, 0xBD, 0x000000, 3931},
        { 11, 0xBD, 0x627627, 3939},
        { 12, 0xBD, 0xC4EC4F, 3947},
        { 13, 0xBE, 0x276276, 3955},
        { 14, 0xBF, 0x13B13B, 3974},
    },
    /* Table for Cabrio E: for XOC_40 only */
    {
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
    },
    /* Table for Cabrio E: for XOC_24 only */
    {
        {  1, 0xC9, 0x000000, 3859},
        {  2, 0xC9, 0x6AAAAB, 3867},
        {  3, 0xC9, 0xD55555, 3875},
        {  4, 0xCA, 0x400000, 3883},
        {  5, 0xCA, 0xAAAAAB, 3891},
        {  6, 0xCB, 0x155555, 3899},
        {  7, 0xCB, 0x800000, 3907},
        {  8, 0xCB, 0xEAAAAB, 3915},
        {  9, 0xCC, 0x555555, 3923},
        { 10, 0xCC, 0xC00000, 3931},
        { 11, 0xCD, 0x2AAAAB, 3939},
        { 12, 0xCD, 0x955555, 3947},
        { 13, 0xCE, 0x000000, 3955},
        { 14, 0xCF, 0x000000, 3974},
    }
};

struct ssv6xxx_ch_cfg ch_cfg_z[] = {
    //0XCE01000C
	{ADR_ABB_REGISTER_1,  0, 0x151559fc},
	//0XCE010008
	{ADR_LDO_REGISTER,    0, 0x00eb7c1c},
	//0XCE010030
	{ADR_RX_ADC_REGISTER, 0, 0x20d000d2}
};

struct ssv6xxx_ch_cfg ch_cfg_p[] = {
    //0XCE01000C
	{ADR_ABB_REGISTER_1,  0, 0x151559fc},
	//0XCE010030
	{ADR_RX_ADC_REGISTER, 0, 0x20d000d2}
};


#define FAIL_MAX 100
#define RETRY_MAX 20

static int ssv6051_cabrioE_set_channel(struct ssv_softc *sc, struct ieee80211_channel *chan, 
    enum nl80211_channel_type channel_type)
{
    struct ssv_hw *sh = sc->sh;
    int ch;
    int retry_cnt, fail_cnt=0;
    u32 regval;
    int ret = -1;
    int  chidx;
    bool chidx_vld = 0;

    ch = chan->hw_value;  

    dev_dbg(sc->dev, "Setting channel to %d\n", ch);

    if((sh->cfg.chip_identity == SSV6051Z) || (sh->cfg.chip_identity == SSV6051P))
    {
        //Channel 13/14
        if((ch == 13) || (ch == 14))
        {
            if(sh->ipd_channel_touch == 0)
            {
                for (chidx = 0; chidx < sh->ch_cfg_size; chidx++)
                {
                    SMAC_REG_WRITE(sh, sh->p_ch_cfg[chidx].reg_addr, sh->p_ch_cfg[chidx].ch13_14_value);
                }
                sh->ipd_channel_touch = 1;
            }
        }
        else
        {
            if(sh->ipd_channel_touch)
            {
                for (chidx = 0; chidx < sh->ch_cfg_size; chidx++)
                {
                    SMAC_REG_WRITE(sh, sh->p_ch_cfg[chidx].reg_addr, sh->p_ch_cfg[chidx].ch1_12_value);
                }
                sh->ipd_channel_touch = 0;
            }
        }
    }


    for(chidx = 0; chidx < 14; chidx++) {
        if (vt_tbl[sh->cfg.crystal_type][chidx].channel_id == ch) {
            chidx_vld = 1;
            break;
        }
    }

    if (chidx_vld == 0) {
        dev_dbg(sc->dev, "%s(): fail! channel_id not found in vt_tbl\n", __FUNCTION__);
        goto exit;
    }

    //printk("Turning off SSV WiFi RF.\n");
    if ((ret = HAL_SET_RF_ENABLE(sc->sh, false)) != 0) 
        goto exit;

    do {

        // not necessary since already be defaults.         
        //if ((ret = SMAC_RF_REG_SET_BITS(sc->sh, ADR_SX_LCK_BIN_REGISTERS_I,
        //    (0x01<<19), (0x01<<19))) != 0) break;

        regval = vt_tbl[sh->cfg.crystal_type][chidx].rf_ctrl_F;
        if ((ret = SMAC_RF_REG_SET_BITS(sc->sh, ADR_SYN_REGISTER_1,
            (regval<<0), (0x00ffffff<<0))) != 0) break;
        
        regval = vt_tbl[sh->cfg.crystal_type][chidx].rf_ctrl_N;
        if ((ret = SMAC_RF_REG_SET_BITS(sc->sh, ADR_SYN_REGISTER_2,
            (regval<<0), (0x07ff<<0))) != 0) break;

        if ((ret = SMAC_RF_REG_READ(sc->sh, ADR_SX_LCK_BIN_REGISTERS_I, &regval)) != 0) break;
        regval = vt_tbl[sh->cfg.crystal_type][chidx].rf_precision_default;
        if ((ret = SMAC_RF_REG_SET_BITS(sc->sh, ADR_SX_LCK_BIN_REGISTERS_II,
            (regval<<0), (0x1fff<<0))) != 0) break;

        //calibration
        if ((ret = SMAC_RF_REG_SET_BITS(sc->sh, ADR_MANUAL_ENABLE_REGISTER,
            (0x00<<14), (0x01<<14))) != 0) break;
        if ((ret = SMAC_RF_REG_SET_BITS(sc->sh, ADR_MANUAL_ENABLE_REGISTER,
            (0x01<<14), (0x01<<14))) != 0) break;

        retry_cnt = 0;
        do
        {
            mdelay(1);
            if ((ret = SMAC_RF_REG_READ(sc->sh, ADR_READ_ONLY_FLAGS_1, &regval)) != 0) break;
            if (regval & 0x00000002)
            {
                 if ((ret = SMAC_RF_REG_READ(sc->sh, ADR_READ_ONLY_FLAGS_2, &regval)) != 0) break;
                /* rf on */
                ret = HAL_SET_RF_ENABLE(sc->sh, true);
//Calibration Debug message
#if 0
                printk("Lock to channel %d ([0xce010098]=%x)!!\n", vt_tbl[sh->cfg.crystal_type][chidx].channel_id, regval);
                printk("crystal_type [%d]\n",sh->cfg.crystal_type);

                SMAC_REG_READ(sc->sh, 0xce010040, &regval);
                printk("0xce010040 [%x]\n",regval);
                SMAC_REG_READ(sc->sh, 0xce0100a4, &regval);
                printk("0xce0100a4 [%x]\n",regval);
                SMAC_REG_READ(sc->sh, ADR_DPLL_DIVIDER_REGISTER, &regval);
                printk("0xce010060 [%x]\n",regval);
                SMAC_REG_READ(sc->sh, ADR_SX_ENABLE_REGISTER, &regval);
                printk("0xce010038 [%x]\n",regval);
                SMAC_REG_READ(sc->sh, 0xce01003C, &regval);
                printk("0xce01003C [%x]\n",regval);
                SMAC_REG_READ(sc->sh, ADR_DPLL_FB_DIVIDER_REGISTERS_I, &regval);
                printk("0xce01009c [%x]\n",regval);
                SMAC_REG_READ(sc->sh, ADR_DPLL_FB_DIVIDER_REGISTERS_II, &regval);
                printk("0xce0100a0 [%x]\n",regval);
                printk("[%x][%x][%x]\n",vt_tbl[sh->cfg.crystal_type][chidx].rf_ctrl_N,vt_tbl[sh->cfg.crystal_type][chidx].rf_ctrl_F,vt_tbl[sh->cfg.crystal_type][chidx].rf_precision_default);
#endif
                dev_info(sc->dev, "Lock to channel %d ([0xce010098]=%x)!!\n", vt_tbl[sh->cfg.crystal_type][chidx].channel_id, regval);
                sc->hw_chan = ch;
                goto exit;
            }
            retry_cnt++;  
        }
        while(retry_cnt < RETRY_MAX);

        fail_cnt++;
        printk("calibation fail:[%d]\n", fail_cnt);
    }
    while((fail_cnt < FAIL_MAX) && (ret == 0));

exit:

    //Update TX gain offset from E-FUSE
    //index 1 is the setting for channel 1~channel 7
    if(ch <= 7)
    {
        if(sh->cfg.tx_power_index_1)
        {
            SMAC_RF_REG_READ(sc->sh, ADR_RX_TX_FSM_REGISTER, &regval);
            regval &= RG_TX_GAIN_OFFSET_I_MSK;
            regval |= (sh->cfg.tx_power_index_1 << RG_TX_GAIN_OFFSET_SFT);
            SMAC_REG_WRITE(sc->sh, ADR_RX_TX_FSM_REGISTER, regval);
        }
        else if(sh->cfg.tx_power_index_2)
        {
            SMAC_RF_REG_READ(sc->sh, ADR_RX_TX_FSM_REGISTER, &regval);
            regval &= RG_TX_GAIN_OFFSET_I_MSK;
            SMAC_REG_WRITE(sc->sh, ADR_RX_TX_FSM_REGISTER, regval);
        }
    }
    //index 2 is the setting for channel 8~channel 14
    else
    {
        if(sh->cfg.tx_power_index_2)
        {
            SMAC_RF_REG_READ(sc->sh, ADR_RX_TX_FSM_REGISTER, &regval);
            regval &= RG_TX_GAIN_OFFSET_I_MSK;
            regval |= (sh->cfg.tx_power_index_2 << RG_TX_GAIN_OFFSET_SFT);
            SMAC_REG_WRITE(sc->sh, ADR_RX_TX_FSM_REGISTER, regval);
        }
        else if(sh->cfg.tx_power_index_1)
        {
            SMAC_RF_REG_READ(sc->sh, ADR_RX_TX_FSM_REGISTER, &regval);
            regval &= RG_TX_GAIN_OFFSET_I_MSK;
            SMAC_REG_WRITE(sc->sh, ADR_RX_TX_FSM_REGISTER, regval);
        }
    }
    return ret;
}

static int ssv6051_cabrioE_do_iq_cal(struct ssv_hw *sh, struct ssv6xxx_iqk_cfg *p_cfg)
{
    struct sk_buff          *skb;
    struct cfg_host_cmd     *host_cmd;
    int ret = 0;

    printk("# Do init_cali (iq)\n");

    // make command packet
    skb = ssv_skb_alloc(sh->sc, HOST_CMD_HDR_LEN + IQK_CFG_LEN + ssv6051_cabrioE_phy_tbl_size + ssv6051_cabrioE_rf_tbl_size);

    if (skb == NULL) {
        printk("init ssv6xxx_do_iq_calib fail!!!\n");
    }

    if ((ssv6051_cabrioE_phy_tbl_size > MAX_PHY_SETTING_TABLE_SIZE) ||
        (ssv6051_cabrioE_rf_tbl_size > MAX_RF_SETTING_TABLE_SIZE)) {
        printk("Please recheck RF or PHY table size!!! \n");
        BUG_ON(1);
    }

    skb->data_len = HOST_CMD_HDR_LEN + IQK_CFG_LEN + ssv6051_cabrioE_phy_tbl_size + ssv6051_cabrioE_rf_tbl_size;
    skb->len      = skb->data_len;

    host_cmd = (struct cfg_host_cmd *)skb->data;

    host_cmd->c_type = HOST_CMD;
    host_cmd->h_cmd  = (u8)SSV6XXX_HOST_CMD_INIT_CALI;
    host_cmd->len    = skb->data_len;

    p_cfg->phy_tbl_size = ssv6051_cabrioE_phy_tbl_size;
    p_cfg->rf_tbl_size = ssv6051_cabrioE_rf_tbl_size;

    memcpy(host_cmd->dat32, p_cfg, IQK_CFG_LEN);

    memcpy(host_cmd->dat8+IQK_CFG_LEN, ssv6051_cabrioE_phy_setting, ssv6051_cabrioE_phy_tbl_size);
    memcpy(host_cmd->dat8+IQK_CFG_LEN+ssv6051_cabrioE_phy_tbl_size, ssv6051_cabrioE_rf_setting, ssv6051_cabrioE_rf_tbl_size);

    sh->hci.hci_ops->hci_send_cmd(sh->hci.hci_ctrl, skb);

    ssv_skb_free(sh->sc, skb);

    // Wait firmware finish IQ calibration.
    {
    u32 timeout;
    sh->sc->iq_cali_done = IQ_CALI_RUNNING;
    set_current_state(TASK_INTERRUPTIBLE);
    timeout = wait_event_interruptible_timeout(sh->sc->fw_wait_q,
                                               sh->sc->iq_cali_done,
                                               msecs_to_jiffies(500));

    set_current_state(TASK_RUNNING);
    if (timeout == 0)
        return -ETIME;
    if (sh->sc->iq_cali_done != IQ_CALI_OK)
        return (-1);
    }

    return ret;
}

static int ssv6051_cabrioE_set_pll_phy_rf(struct ssv_hw *sh
    , ssv_cabrio_reg *rf_tbl, ssv_cabrio_reg *phy_tbl)
{
    int  ret;
    u32 regval , i;
    
    HAL_INIT_PLL(sh);
    
    HAL_SET_PHY_MODE(sh, false); /* disable all phy mode */

    // set rf_tbl                                                                            
    for( i = 0; i < (ssv6051_cabrioE_rf_tbl_size/ sizeof(ssv_cabrio_reg)); i++) {
        ret = SMAC_REG_WRITE(sh, rf_tbl[i].address, rf_tbl[i].data);      
        if (ret) break;                                                
    }                                                                             

    //check if clock switch bit had been set. ADR_PHY_EN_0[31] = ce000000
    SMAC_REG_READ(sh, ADR_PHY_EN_0, &regval);
    if (regval & (1<<RG_RF_BB_CLK_SEL_SFT)) {
        printk("already do clock switch\n");
    }    
    else {
        printk("reset PLL\n");
        //reset pll. ADR_DPLL_CP_PFD_REGISTER[9] = ce01005c
        SMAC_REG_READ(sh, ADR_DPLL_CP_PFD_REGISTER, &regval);
        regval |= ((1<<RG_DP_BBPLL_PD_SFT) | (1<<RG_DP_BBPLL_SDM_EDGE_SFT));
        SMAC_REG_WRITE(sh, ADR_DPLL_CP_PFD_REGISTER, regval);
        regval &= ~((1<<RG_DP_BBPLL_PD_SFT) | (1<<RG_DP_BBPLL_SDM_EDGE_SFT));
        SMAC_REG_WRITE(sh, ADR_DPLL_CP_PFD_REGISTER, regval);
        mdelay(10);    
    } 

    if (ret== 0) ret=SSV6XXX_SET_HW_TABLE(sh, phy_tbl);


    // set phy_tbl                                                                            
    for(i = 0; i < (ssv6051_cabrioE_phy_tbl_size/sizeof(ssv_cabrio_reg)); i++) {
        ret = SMAC_REG_WRITE(sh, phy_tbl[i].address, phy_tbl[i].data);      
        if (ret) break;                                                
    }                                                                             
    
    return ret;
}

static void ssv6051_cabrioE_dpd_enable(struct ssv_hw *sh, bool val)
{
    if (val) {
        //DPD function on
        //0xCE010014
        SMAC_REG_WRITE(sh, ADR_TX_FE_REGISTER, 0x3CBE84FE);
        //0xCE010018
        SMAC_REG_WRITE(sh, ADR_RX_FE_REGISTER_1, 0x4507F9);
        SMAC_REG_WRITE(sh, ADR_DPD_CONTROL, 0x3);
    } else  {
        //DPD function off
        //0xCE010014
        SMAC_REG_WRITE(sh, ADR_TX_FE_REGISTER, 0x3D3E84FE);
        //0xCE010018
        SMAC_REG_WRITE(sh, ADR_RX_FE_REGISTER_1, 0x1457D79);
        SMAC_REG_WRITE(sh, ADR_DPD_CONTROL, 0x0);
    }
}

static void ssv6051_cabrioE_init_ch_cfg(struct ssv_hw *sh)
{
    if(sh->cfg.chip_identity == SSV6051Z)
    {
        sh->p_ch_cfg = &ch_cfg_z[0];
        sh->ch_cfg_size = sizeof(ch_cfg_z) / sizeof(struct ssv6xxx_ch_cfg);
    }
    else if(sh->cfg.chip_identity == SSV6051P)
    {
        sh->p_ch_cfg = &ch_cfg_p[0];
        sh->ch_cfg_size = sizeof(ch_cfg_p) / sizeof(struct ssv6xxx_ch_cfg);
    }

}

static void ssv6051_cabrioE_init_iqk(struct ssv_hw *sh)
{
    sh->iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_26M;
#ifdef CONFIG_SSV_DPD
    sh->iqk_cfg.cfg_pa = SSV6XXX_IQK_CFG_PA_LI_MPB;
#else
    sh->iqk_cfg.cfg_pa = SSV6XXX_IQK_CFG_PA_DEF;
#endif
    sh->iqk_cfg.cfg_pabias_ctrl = 0;
    sh->iqk_cfg.cfg_pacascode_ctrl = 0;
    sh->iqk_cfg.cfg_tssi_trgt = 26;
    sh->iqk_cfg.cfg_tssi_div = 3;
    sh->iqk_cfg.cfg_def_tx_scale_11b = 0x75;
    sh->iqk_cfg.cfg_def_tx_scale_11b_p0d5 = 0x75;
    sh->iqk_cfg.cfg_def_tx_scale_11g = 0x80;
    sh->iqk_cfg.cfg_def_tx_scale_11g_p0d5 = 0x80;
    sh->iqk_cfg.cmd_sel = SSV6XXX_IQK_CMD_INIT_CALI;
    sh->iqk_cfg.fx_sel = SSV6XXX_IQK_TEMPERATURE 
                       + SSV6XXX_IQK_RXDC 
                       + SSV6XXX_IQK_RXRC 
                       + SSV6XXX_IQK_TXDC 
                       + SSV6XXX_IQK_TXIQ
                       + SSV6XXX_IQK_RXIQ;
#ifdef CONFIG_SSV_DPD
    sh->iqk_cfg.fx_sel += SSV6XXX_IQK_PAPD;
#endif
}

static void ssv6051_cabrioE_save_default_ipd_chcfg(struct ssv_hw *sh)
{
    int i;
    if((sh->cfg.chip_identity == SSV6051Z) || (sh->cfg.chip_identity == SSV6051P)){
        for (i = 0; i < sh->ch_cfg_size; i++) {
            SMAC_REG_READ(sh, sh->p_ch_cfg[i].reg_addr, &sh->p_ch_cfg[i].ch1_12_value);
        }
    }
}


static void ssv6051_cabrioE_chg_ipd_phyinfo(struct ssv_hw *sh)
{
    extern u32 phy_info_tbl[];

    if(sh->cfg.chip_identity == SSV6051Z)
        memcpy(phy_info_tbl, phy_info_6051z, sizeof(phy_info_6051z));
}

static bool ssv6051_cabrioE_set_rf_enable(struct ssv_hw *sh, bool val)
{
	  if (val){
        return SMAC_REG_SET_BITS(sh, ADR_CBR_HARD_WIRE_PIN_REGISTER,
                (RF_MODE_TRX_EN << CBR_RG_MODE_SFT), CBR_RG_MODE_MSK);
	  } else {
	      return SMAC_REG_SET_BITS(sh,ADR_CBR_HARD_WIRE_PIN_REGISTER,
                (RF_MODE_STANDBY << CBR_RG_MODE_SFT), CBR_RG_MODE_MSK);
	  } 
}

static bool ssv6051_cabrioE_dump_phy_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;

    ssv_cabrio_reg *raw;   
    
    raw = ssv6051_cabrioE_phy_setting;

    snprintf_res(cmd_data,">> PHY Register Table:\n");
    
    for(s = 0; s < ssv6051_cabrioE_phy_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);
    }
    snprintf_res(cmd_data,"\n\n");

    return 0;
}

static bool ssv6051_cabrioE_dump_rf_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6051_cabrioE_rf_setting;

    snprintf_res(cmd_data,">> RF Register Table:\n");

    for(s = 0; s < ssv6051_cabrioE_rf_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", raw->address, regval);
    }
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6051_cabrioE_support_iqk_cmd(struct ssv_hw *sh)
{
    return true;
}

void ssv_attach_ssv6051_cabrioE_BBRF(struct ssv_hal_ops *hal_ops)
{
    hal_ops->load_phy_table = ssv6051_cabrioE_load_phy_table;
    hal_ops->get_phy_table_size = ssv6051_cabrioE_get_phy_table_size;
    hal_ops->load_rf_table = ssv6051_cabrioE_load_rf_table;
    hal_ops->get_rf_table_size = ssv6051_cabrioE_get_rf_table_size;
    hal_ops->init_pll = ssv6051_cabrioE_init_PLL;
    hal_ops->update_cfg_hw_patch = ssv6051_cabrioE_update_cfg_hw_patch;
    hal_ops->update_hw_config = ssv6051_cabrioE_update_hw_config;
    hal_ops->chg_pad_setting = ssv6051_cabrioE_chg_pad_setting;
    hal_ops->set_channel = ssv6051_cabrioE_set_channel;
    hal_ops->do_iq_cal = ssv6051_cabrioE_do_iq_cal;
    hal_ops->set_pll_phy_rf = ssv6051_cabrioE_set_pll_phy_rf;
    hal_ops->dpd_enable = ssv6051_cabrioE_dpd_enable;
    hal_ops->init_ch_cfg = ssv6051_cabrioE_init_ch_cfg;
    hal_ops->init_iqk = ssv6051_cabrioE_init_iqk;
    hal_ops->save_default_ipd_chcfg = ssv6051_cabrioE_save_default_ipd_chcfg;
    hal_ops->chg_ipd_phyinfo = ssv6051_cabrioE_chg_ipd_phyinfo;
    hal_ops->set_rf_enable = ssv6051_cabrioE_set_rf_enable;
    hal_ops->dump_phy_reg = ssv6051_cabrioE_dump_phy_reg;
    hal_ops->dump_rf_reg = ssv6051_cabrioE_dump_rf_reg;
    hal_ops->support_iqk_cmd = ssv6051_cabrioE_support_iqk_cmd;
}
#endif
