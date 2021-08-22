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
#include <linux/version.h>
#if ((defined SSV_SUPPORT_HAL) && (defined SSV_SUPPORT_SSV6006))
#include <linux/nl80211.h>
#include <ssv6200.h>
#include "ssv6006C_reg.h"
#include "ssv6006C_aux.h"
#include <smac/dev.h>
#include <smac/ssv_skb.h>
#include <hal.h>
#include <ssvdevice/ssv_cmd.h>
#include "ssv6006_mac.h"
#include "ssv6006_priv.h"
#include "ssv6006_priv_safe.h"
#include "turismoC_rf_reg.c"
#include "turismoC_wifi_phy_reg.c"
#include "turismo_common.h"
#include "turismo_common.c"

#include <ssv6xxx_common.h>
#include <linux_80211.h>

static bool ssv6006_turismoC_set_rf_enable(struct ssv_hw *sh, bool val);

static const size_t ssv6006_turismoC_phy_tbl_size = sizeof(ssv6006_turismoC_phy_setting);

static void ssv6006_turismoC_load_phy_table(ssv_cabrio_reg **phy_table)
{
    *phy_table = ssv6006_turismoC_phy_setting;
}

static u32 ssv6006_turismoC_get_phy_table_size(struct ssv_hw *sh)
{
    return(u32) ssv6006_turismoC_phy_tbl_size;
}

static const size_t ssv6006_turismoC_rf_tbl_size = sizeof(ssv6006_turismoC_rf_setting);

static void ssv6006_turismoC_load_rf_table(ssv_cabrio_reg **rf_table)
{
    *rf_table = ssv6006_turismoC_rf_setting;
}

static u32 ssv6006_turismoC_get_rf_table_size(struct ssv_hw *sh)
{
    return (u32) ssv6006_turismoC_rf_tbl_size;
}

static int _get_pa_band(int ch){
    int pa_band = 0;
    
    if (ch <=14){
        pa_band =0;
    } else if (ch < 36) {
        pa_band = 1;
    } else if ((ch >= 36) && (ch < 100)){
        pa_band = 2;
    } else if ((ch >= 100) && (ch < 140)){
        pa_band = 3;
    } else if (ch >= 140){
        pa_band = 4;        
    }
    return pa_band;
}
#define USE_COMMON_MACRO

static void ssv6006_turismoC_init_PLL(struct ssv_hw *sh)
{
#ifdef USE_COMMON_MACRO
    TU_INIT_TURISMOC_PLL;
#else
    u32 regval , count = 0;                                                            
                                                                                       
    /*for turismo, it just needs to set register once , pll is initialized by hw auto*/
    SET_RG_LOAD_RFTABLE_RDY(0x1);
    do
    {
        MSLEEP(1);
        regval = REG32_R(ADR_PMU_STATE_REG);
        count ++ ;
        if (regval == 0x13)
            break;
        if (count > 100){
            PRINT(" PLL initial fails \r\n");
            break;
        }
    } while (1);                                                                       
                                                                                       
    MSLEEP(1);
    /* enable PHY clock*/
    REG32_W(ADR_WIFI_PHY_COMMON_SYS_REG, 0x80010000);
    /* do clock switch */
    REG32_W(ADR_CLOCK_SELECTION, 0x00000008);
    MSLEEP(1);  /* wait for clock settled*/
 
#endif
}
#ifndef USE_COMMON_MACRO
static void _set_turismoC_BW(struct ssv_hw *sh, enum nl80211_channel_type channel_type)
{
#ifdef USE_COMMON_MACRO
    TU_SET_TURISMOC_BW(channel_type);
#else
    /* Set channel type*/                                                                  
    switch (channel_type){
      case 	NL80211_CHAN_HT20:
      case  NL80211_CHAN_NO_HT:

            SET_MTX_BLOCKTX_IGNORE_TOMAC_CCA_ED_SECONDARY(1);
            
            SET_REG(ADR_WIFI_PHY_COMMON_SYS_REG,
                (0 << RG_PRIMARY_CH_SIDE_SFT) | (0 << RG_SYSTEM_BW_SFT), 0,
                (RG_PRIMARY_CH_SIDE_I_MSK & RG_SYSTEM_BW_I_MSK));

            SET_REG(ADR_DIGITAL_ADD_ON_0,
                (0 << RG_40M_MODE_SFT) | (0 << RG_LO_UP_CH_SFT), 0,
                (RG_40M_MODE_I_MSK & RG_LO_UP_CH_I_MSK));

            break;

	  case  NL80211_CHAN_HT40MINUS:                                                        

            SET_MTX_BLOCKTX_IGNORE_TOMAC_CCA_ED_SECONDARY(0);
            
            SET_REG(ADR_WIFI_PHY_COMMON_SYS_REG,
                (1 << RG_PRIMARY_CH_SIDE_SFT) | (1 << RG_SYSTEM_BW_SFT), 0,
                (RG_PRIMARY_CH_SIDE_I_MSK & RG_SYSTEM_BW_I_MSK));

            SET_REG(ADR_DIGITAL_ADD_ON_0,
                (1 << RG_40M_MODE_SFT) | (0 << RG_LO_UP_CH_SFT), 0,
                (RG_40M_MODE_I_MSK & RG_LO_UP_CH_I_MSK));
                       
            break;                                                                         
	  case  NL80211_CHAN_HT40PLUS:                                                         

            SET_MTX_BLOCKTX_IGNORE_TOMAC_CCA_ED_SECONDARY(0);

            SET_REG(ADR_WIFI_PHY_COMMON_SYS_REG,
                (0 << RG_PRIMARY_CH_SIDE_SFT) | (1 << RG_SYSTEM_BW_SFT), 0,
                (RG_PRIMARY_CH_SIDE_I_MSK & RG_SYSTEM_BW_I_MSK));

            SET_REG(ADR_DIGITAL_ADD_ON_0,
                (1 << RG_40M_MODE_SFT) | (1 << RG_LO_UP_CH_SFT), 0,
                (RG_40M_MODE_I_MSK & RG_LO_UP_CH_I_MSK));
            break;                   
      default:                                                                             
            break;                                                                         
    }
#endif                                                                                      
}

static void _set_2g_channel(struct ssv_hw *sh, int ch)
{
#ifdef USE_COMMON_MACRO
    TURISMOC_SET_2G_CHANNEL(ch);
#else
    SET_RG_RF_5G_BAND(0);

    /* set rf channel manual on*/
    SET_RG_MODE_MANUAL(1);

    /* set rf channel mapping on*/
    SET_RG_SX_RFCH_MAP_EN(1);

    /* set channel*/
    SET_RG_SX_CHANNEL(ch);
    
    /* set RG_MODE to IDLE mode */
    SET_RG_MODE(0);

    /* set RG_MODE to WIFI_RX */
    SET_RG_MODE(3);

    /* set RG_MODE_MANUAL off */
    SET_RG_MODE_MANUAL(0);
    
#endif
}

static void _set_5g_channel(struct ssv_hw *sh, int ch)
{
#ifdef USE_COMMON_MACRO
    TURISMOC_SET_5G_CHANNEL(ch);
#else
    SET_RG_RF_5G_BAND(1);

    /* set rf channel manual on*/
    SET_RG_MODE_MANUAL(1);

    /* set rf channel mapping on*/
    SET_RG_SX5GB_RFCH_MAP_EN(1);

    /* set channel*/
    SET_RG_SX5GB_CHANNEL(ch);

    /* set RG_MODE to IDLE mode */
    SET_RG_MODE(0);

    /* set RG_MODE to WIFI_RX 5G */
    SET_RG_MODE(7);

    /* set RG_MODE_MANUAL off */
    SET_RG_MODE_MANUAL(0);
    
#endif 
}

static void _turismoC_pre_cal(struct ssv_hw *sh)
{
    /* set RG_MODE_MANUAL ON*/
    SET_RG_MODE_MANUAL(1);

    /* set RG_MODE to Calibration mode */                                                 
    SET_RG_MODE(MODE_CALIBRATION);
}

static void _turismoC_post_cal(struct ssv_hw *sh)
{
    /* set RG_MODE to IDLE mode*/
    SET_RG_MODE(MODE_STANDBY);
    
    /*set RG_MODE_MANUAL off*/
    SET_RG_MODE_MANUAL(0);
}

static void _turismoC_2p4g_rxdc_cal(struct ssv_hw *sh)
{
    int i = 0, j ;                                                                               
    u32 wifi_dc_addr;
    int rg_rfg, rg_pgag;
    int adc_out_sum_i, adc_out_sumQ;
                                             
    SET_REG(ADR_SX_2_4GB_5GB_REGISTER_INT3BIT___CH_TABLE,
        (0x6 << RG_SX_CHANNEL_SFT) | (0x1 << RG_SX_RFCH_MAP_EN_SFT), 0,
        (RG_SX_CHANNEL_I_MSK & RG_SX_RFCH_MAP_EN_I_MSK));

    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);

    /*set RG_CAL_INDEX to WiFi DC 2.4G*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_RXDC);
    
    UDELAY(100);
    do {
        if (GET_RO_WF_DCCAL_DONE == 0)
            break;
        i ++;
        UDELAY(100);
    } while (i < 100);

    PRINT("--------------------------------------------%d\r\n",i);
    PRINT("--------- 2.4 G Rx DC Calibration result----------------");
    for (i = 0; i < 21; i++) {
       if (i %4 == 0)
          PRINT("\r\n");
       wifi_dc_addr = (ADR_WF_DCOC_IDAC_REGISTER1)+ (i << 2);
       PRINT("addr %x : val %x, ", wifi_dc_addr, REG32_R(wifi_dc_addr));
    }
    PRINT("\r\n"); 
    
    SET_RG_RX_GAIN_MANUAL(1);
    
    for(i = 1;i >= 0; i--){
        for(j = 15;j >= 0; j--){
            rg_rfg = i;
            rg_pgag = j;
            SET_RG_RFG(rg_rfg);
            SET_RG_PGAG(rg_pgag);
            adc_out_sum_i = GET_RO_DC_CAL_I;
            if (adc_out_sum_i>63) {
                adc_out_sum_i -= 128;
            }
            adc_out_sumQ = GET_RO_DC_CAL_Q;
            if(adc_out_sumQ>63){
                adc_out_sumQ -= 128;
            }
            PRINT("lna gain is %d, pga gain is %d, ADC_OUT_I is %d, ADC_OUT_Q is %d\n",
               rg_rfg, rg_pgag, adc_out_sum_i, adc_out_sumQ); 
        }
        PRINT("------------------------------------------------------------\n");
    }
    
    SET_RG_RX_GAIN_MANUAL(0);
    
    /* set RG_CAL_INDEX to NONE*/                                                         
    SET_RG_CAL_INDEX(CAL_IDX_NONE);

    /* set RG_MODE to IDLE mode*/
   /* SET_RG_MODE(0);*/
}

static void _turismoC_5g_rxdc_cal(struct ssv_hw *sh)
{
    int i = 0, j;                                                                               
    u32 wifi_dc_addr;
    int rg_rfg, rg_pgag;
    int adc_out_sum_i, adc_out_sumQ;                                                                         

    SET_REG(ADR_SX_5GB_REGISTER_INT3BIT___CH_TABLE,
        (100 << RG_SX5GB_CHANNEL_SFT) | (0x1 << RG_SX5GB_RFCH_MAP_EN_SFT), 0,
        (RG_SX5GB_CHANNEL_I_MSK & RG_SX5GB_RFCH_MAP_EN_I_MSK));

    /* set RG_MODE to Calibration mode */                                                 
//    SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to WiFi DC 5G*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_RXDC);
    UDELAY(100);
    do {
        if (GET_RO_5G_DCCAL_DONE == 0)
            break;
        i ++;
        UDELAY(100);
    } while (i < 100);
    
    PRINT("--------------------------------------------%d\r\n",i);
    PRINT("--------- 5 G Rx DC Calibration result----------------");
    for (i = 0; i < 21; i++) {
       if (i %4 == 0)
          PRINT("\r\n");
       wifi_dc_addr = (ADR_5G_DCOC_IDAC_REGISTER1)+ (i << 2);
       PRINT("addr %x : val %x, ", wifi_dc_addr, REG32_R(wifi_dc_addr));
    }
    PRINT("\r\n");

    SET_RG_RX_GAIN_MANUAL(1);
    
    for(i = 1;i >= 0; i--){
        for(j = 15;j >= 0; j--){
            rg_rfg = i;
            rg_pgag = j;
            SET_RG_RFG(rg_rfg);
            SET_RG_PGAG(rg_pgag);
            adc_out_sum_i = GET_RO_DC_CAL_I;
            if (adc_out_sum_i>63) {
                adc_out_sum_i -= 128;
            }
            adc_out_sumQ = GET_RO_DC_CAL_Q;
            if(adc_out_sumQ>63){
                adc_out_sumQ -= 128;
            }
            PRINT("lna gain is %d, pga gain is %d, ADC_OUT_I is %d, ADC_OUT_Q is %d\n",
               rg_rfg, rg_pgag, adc_out_sum_i, adc_out_sumQ); 
        }
        PRINT("------------------------------------------------------------\n");
    }
    
    SET_RG_RX_GAIN_MANUAL(0);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(CAL_IDX_NONE);
    
    /* set RG_MODE to IDLE mode*/                                          
    //SET_RG_MODE(0);                                                        
}

static void _turismoC_bw20_rxrc_cal(struct ssv_hw *sh)
{
    int count = 0;

    PRINT("--------------------------------------------\r\n");
    PRINT("Before WiFi BW20 RG_WF_RX_ABBCTUNE: %d\n", GET_RG_WF_RX_ABBCTUNE);

    SET_RG_RX_RCCAL_DELAY(2);
 
    SET_RG_PHASE_17P5M(0x20d0);
    
    SET_REG(ADR_RF_D_CAL_TOP_6,
        (0x22c << RG_RX_RCCAL_TARG_SFT) | (0 << RG_RCCAL_POLAR_INV_SFT), 0,
        (RG_RX_RCCAL_TARG_I_MSK & RG_RCCAL_POLAR_INV_I_MSK));

    SET_RG_ALPHA_SEL(2);
    
    SET_RG_PGAG_RCCAL(3);
    
    SET_RG_TONE_SCALE(0x80);
    
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to BW20_RXRC*/
    SET_RG_CAL_INDEX(CAL_IDX_BW20_RXRC);

    UDELAY(100); 
    
    while (GET_RO_RCCAL_DONE == 0){
        count ++;
        if (count >1000) {
            break;
        }
        UDELAY(100);
    }

    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("WiFi BW20 RG_WF_RX_ABBCTUNE CAL RESULT: %d\n", GET_RG_WF_RX_ABBCTUNE);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(CAL_IDX_NONE);
   
}

static void _turismoC_bw40_rxrc_cal(struct ssv_hw *sh)
{
    int count = 0;
 
    
    PRINT("--------------------------------------------\r\n");
    PRINT("Before WiFi BW40 RG_WF_RX_N_ABBCTUNE: %d\n", GET_RG_WF_N_RX_ABBCTUNE);

    SET_RG_RX_N_RCCAL_DELAY(2);

    SET_RG_PHASE_35M(0x3fff);
    

    SET_REG(ADR_RF_D_CAL_TOP_6,
        (0x213 << RG_RX_RCCAL_40M_TARG_SFT) | (0 << RG_RCCAL_POLAR_INV_SFT), 0,
        (RG_RX_RCCAL_40M_TARG_I_MSK & RG_RCCAL_POLAR_INV_I_MSK));

    SET_RG_ALPHA_SEL(2);
   
    SET_RG_PGAG_RCCAL(3);

    SET_RG_TONE_SCALE(0x80);

    
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);

   
    /*set RG_CAL_INDEX to BW40_RXRC*/
    SET_RG_CAL_INDEX(CAL_IDX_BW40_RXRC);
    UDELAY(100);

    while (GET_RO_RCCAL_DONE == 0){
        count ++;
        if (count >1000) {
            break;
        }
        UDELAY(100);
    }
    if (count >= 1000){
        PRINT("%s: cal failed\n",__func__);
    }
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("WiFi BW40 RG_WF_N_RX_ABBCTUNE CAL RESULT: %d\n", GET_RG_WF_N_RX_ABBCTUNE);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(CAL_IDX_NONE);
    
}

static void _turismoC_txdc_cal(struct ssv_hw *sh)
{
    int count = 0;

    SET_REG(ADR_SX_2_4GB_5GB_REGISTER_INT3BIT___CH_TABLE,
        (0x6 << RG_SX_CHANNEL_SFT) | (0x1 << RG_SX_RFCH_MAP_EN_SFT), 0,
        (RG_SX_CHANNEL_I_MSK & RG_SX_RFCH_MAP_EN_I_MSK));

    PRINT("--------------------------------------------\r\n");
    PRINT("Before txdc calibration WiFi 2P4G Tx DAC IOFFSET: %d, QOFFSET %d\n", 
         GET_RG_WF_TX_DAC_IOFFSET, GET_RG_WF_TX_DAC_QOFFSET);

    SET_RG_TXGAIN_PHYCTRL(1);
    
    SET_REG(ADR_CALIBRATION_GAIN_REGISTER0,
        (0x6 << RG_TX_GAIN_TXCAL_SFT) | (0x3 << RG_PGAG_TXCAL_SFT), 0,
        (RG_TX_GAIN_TXCAL_I_MSK & RG_PGAG_TXCAL_I_MSK));

    SET_RG_TONE_SCALE(0x80);
    
    SET_RG_PRE_DC_AUTO(1);
    
    SET_RG_TX_IQCAL_TIME(1);
 
    SET_RG_PHASE_1M(0x0ccc);
    
    SET_RG_PHASE_RXIQ_1M(0x0ccc);
    
    SET_RG_ALPHA_SEL(2);
           
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to WIFI2P4G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_TXLO);
    UDELAY(200);    
    while (GET_RO_TXDC_DONE == 0){
        count ++;
        if (count >1000) {
            break;
        }
        UDELAY(100);
    }
    
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After txdc calibration WiFi 2P4G Tx DAC IOFFSET: %d, QOFFSET %d\n", 
         GET_RG_WF_TX_DAC_IOFFSET, GET_RG_WF_TX_DAC_QOFFSET);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(CAL_IDX_NONE);

}


static void _turismoC_txiq_cal(struct ssv_hw *sh)
{
    int count = 0;
    
    SET_REG(ADR_SX_2_4GB_5GB_REGISTER_INT3BIT___CH_TABLE,
        (0x6 << RG_SX_CHANNEL_SFT) | (0x1 << RG_SX_RFCH_MAP_EN_SFT), 0,
        (RG_SX_CHANNEL_I_MSK & RG_SX_RFCH_MAP_EN_I_MSK));

 
    PRINT("--------------------------------------------\r\n");
    PRINT("before tx iq 2.4G calibration, tx alpha: %d, tx theta %d\n", 
         GET_RG_TX_IQ_2500_ALPHA, GET_RG_TX_IQ_2500_THETA);
    
    SET_RG_TXGAIN_PHYCTRL(1);

    SET_REG(ADR_CALIBRATION_GAIN_REGISTER0,
        (0x6 << RG_TX_GAIN_TXCAL_SFT) | (0x3 << RG_PGAG_TXCAL_SFT), 0,
        (RG_TX_GAIN_TXCAL_I_MSK & RG_PGAG_TXCAL_I_MSK));

    SET_RG_TONE_SCALE(0x80);

    SET_RG_PRE_DC_AUTO(1);
    
    SET_RG_TX_IQCAL_TIME(1);
 
    SET_RG_PHASE_1M(0xccc);
    SET_RG_PHASE_RXIQ_1M(0xccc);
    
    SET_RG_ALPHA_SEL(2);
       
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
    
    /*set RG_CAL_INDEX to WIFI2P4G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_TXIQ);
    UDELAY(200);    
    while (GET_RO_TXIQ_DONE == 0){
        count ++;
        if (count >1000) {
            break;
        }
        UDELAY(100);
    }
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After tx iq calibration, tx alpha: %d, tx theta %d\n", 
         GET_RG_TX_IQ_2500_ALPHA, GET_RG_TX_IQ_2500_THETA);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(CAL_IDX_NONE);   
}

static void _turismoC_rxiq_cal(struct ssv_hw *sh)
{
    int count = 0;
    u32 regval, regval1;

    SET_REG(ADR_SX_2_4GB_5GB_REGISTER_INT3BIT___CH_TABLE,
        (0x6 << RG_SX_CHANNEL_SFT) | (0x1 << RG_SX_RFCH_MAP_EN_SFT), 0,
        (RG_SX_CHANNEL_I_MSK & RG_SX_RFCH_MAP_EN_I_MSK));


    PRINT("--------------------------------------------\r\n");
    PRINT("Before rx iq calibration, rx alpha: %d, rx theta %d\n", 
         GET_RG_RX_IQ_2500_ALPHA, GET_RG_RX_IQ_2500_THETA);

    SET_RG_TXGAIN_PHYCTRL(1);

    SET_RG_RFG_RXIQCAL(0x0);
    
    SET_RG_PGAG_RXIQCAL(0x3);
    
    SET_RG_TX_GAIN_RXIQCAL(0x6);
 
    SET_RG_TONE_SCALE(0x80);
    
    SET_RG_PRE_DC_AUTO(1);
    
    SET_RG_TX_IQCAL_TIME(1);

    SET_RG_PHASE_1M(0xccc);
    SET_RG_PHASE_RXIQ_1M(0xccc);

    SET_RG_ALPHA_SEL(2);
    
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);

   
    /*set RG_CAL_INDEX to WIFI2P4G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_RXIQ);
    UDELAY(200);
    while (GET_RO_RXIQ_DONE == 0){
        count ++;
        if (count >1000) {
            break;
        }
        UDELAY(100);
    }

    SET_RG_PHASE_STEP_VALUE(0xccc);
    SET_RG_SPECTRUM_EN(1);

    SET_REG(ADR_RF_D_CAL_TOP_7,
        (0x1 << RG_SPECTRUM_PWR_UPDATE_SFT) | (0x1 << RG_SPECTRUM_LO_FIX_SFT), 0,
        (RG_SPECTRUM_PWR_UPDATE_I_MSK & RG_SPECTRUM_LO_FIX_I_MSK));

    MDELAY(10);
    
    SET_REG(ADR_RF_D_CAL_TOP_7,
        (0x0 << RG_SPECTRUM_PWR_UPDATE_SFT) | (0x1 << RG_SPECTRUM_LO_FIX_SFT), 0,
        (RG_SPECTRUM_PWR_UPDATE_I_MSK & RG_SPECTRUM_LO_FIX_I_MSK));

    regval1 = GET_RG_SPECTRUM_PWR_UPDATE;
    regval = GET_RO_SPECTRUM_IQ_PWR_31_0;
    PRINT("The spectrum power is 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
        ((regval1 >> 4) & 0xf), (regval1 & 0xf), ((regval >> 28) & 0xf), ((regval >> 24) & 0xf),
        ((regval >> 20) & 0xf), ((regval >> 16) & 0xf), ((regval >> 12) & 0xf), ((regval >> 8) & 0xf),
        ((regval >> 4) & 0xf), (regval & 0xf));

    SET_RG_PHASE_STEP_VALUE(0xF334);
    SET_RG_SPECTRUM_EN(1);

    SET_REG(ADR_RF_D_CAL_TOP_7,
        (0x1 << RG_SPECTRUM_PWR_UPDATE_SFT) | (0x1 << RG_SPECTRUM_LO_FIX_SFT), 0,
        (RG_SPECTRUM_PWR_UPDATE_I_MSK & RG_SPECTRUM_LO_FIX_I_MSK));

    MDELAY(10);
    
    SET_REG(ADR_RF_D_CAL_TOP_7,
        (0x0 << RG_SPECTRUM_PWR_UPDATE_SFT) | (0x1 << RG_SPECTRUM_LO_FIX_SFT), 0,
        (RG_SPECTRUM_PWR_UPDATE_I_MSK & RG_SPECTRUM_LO_FIX_I_MSK));

    regval1 = GET_RG_SPECTRUM_PWR_UPDATE;
    regval = GET_RO_SPECTRUM_IQ_PWR_31_0;
    PRINT("The spectrum power is 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
        ((regval1 >> 4) & 0xf), (regval1 & 0xf), ((regval >> 28) & 0xf), ((regval >> 24) & 0xf),
        ((regval >> 20) & 0xf), ((regval >> 16) & 0xf), ((regval >> 12) & 0xf), ((regval >> 8) & 0xf),
        ((regval >> 4) & 0xf), (regval & 0xf));

    SET_RG_SPECTRUM_EN(0);

    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After rx iq calibration, rx alpha: %d, rx theta %d\n", 
         GET_RG_RX_IQ_2500_ALPHA, GET_RG_RX_IQ_2500_THETA);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(CAL_IDX_NONE);
}

static void _turismoC_5g_txdc_cal(struct ssv_hw *sh)
{
    int count = 0;
    
    SET_REG(ADR_SX_5GB_REGISTER_INT3BIT___CH_TABLE,
        (100 << RG_SX5GB_CHANNEL_SFT) | (0x1 << RG_SX5GB_RFCH_MAP_EN_SFT), 0,
        (RG_SX5GB_CHANNEL_I_MSK & RG_SX5GB_RFCH_MAP_EN_I_MSK));

    PRINT("--------------------------------------------\r\n");
    PRINT("Before 5G txdc calibration WiFi 5G Tx DAC IOFFSET: %d, QOFFSET %d\n", 
         GET_RG_5G_TX_DAC_IOFFSET, GET_RG_5G_TX_DAC_QOFFSET);

    SET_RG_TXGAIN_PHYCTRL(1);

    SET_RG_TONE_SCALE(0x80);
    
    SET_REG(ADR_5G_CALIBRATION_TIMER_GAIN_REGISTER,
        (0x2 << RG_5G_TX_GAIN_TXCAL_SFT) | (0x3 << RG_5G_PGAG_TXCAL_SFT), 0,
        (RG_5G_TX_GAIN_TXCAL_I_MSK & RG_5G_PGAG_TXCAL_I_MSK));
   
    SET_RG_PRE_DC_AUTO(1);

    SET_RG_TX_IQCAL_TIME(1);
 
    SET_RG_PHASE_1M(0xCCC);
    SET_RG_PHASE_RXIQ_1M(0xCCC);
    
    SET_RG_ALPHA_SEL(2);
       
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);

    /*set RG_CAL_INDEX to WIFI5G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_TXLO);
    UDELAY(250);    
    while (GET_RO_5G_TXDC_DONE == 0){
        count ++;
        if (count >1000) {
            break;
        }
        UDELAY(100);
    }
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After 5G txdc calibration WiFi 5G Tx DAC IOFFSET: %d, QOFFSET %d\n", 
         GET_RG_5G_TX_DAC_IOFFSET, GET_RG_5G_TX_DAC_QOFFSET);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(CAL_IDX_NONE);
}

static void _turismoC_5g_txiq_cal(struct ssv_hw *sh)
{
    int count = 0;
    int band;


    SET_RG_SX5GB_RFCH_MAP_EN(1);
    
    PRINT("--------------------------------------------\r\n");
    PRINT("before 5G tx iq calibration, tx alpha: %d %d %d %d, tx theta %d %d %d %d\n", 
         GET_RG_TX_IQ_5100_ALPHA, GET_RG_TX_IQ_5100_THETA,
         GET_RG_TX_IQ_5500_ALPHA, GET_RG_TX_IQ_5500_THETA,
         GET_RG_TX_IQ_5700_ALPHA, GET_RG_TX_IQ_5700_THETA,
         GET_RG_TX_IQ_5900_ALPHA, GET_RG_TX_IQ_5900_THETA);
    
    SET_RG_TXGAIN_PHYCTRL(1);
    
    SET_RG_TONE_SCALE(0x80);
    
    SET_RG_5G_PGAG_TXCAL(0x3);

    SET_RG_PRE_DC_AUTO(1);
    
    SET_RG_TX_IQCAL_TIME(1);
    
    SET_RG_PHASE_1M(0xccc);
    SET_RG_PHASE_RXIQ_1M(0xccc);
    
    SET_RG_ALPHA_SEL(2);
       
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
 
    for (band = 0; band < 4; band ++){
        SET_RG_SX5GB_CHANNEL(cal_ch_5g[band]);
        if( band == 2 ) {
            SET_RG_5G_TX_GAIN_TXCAL(0x2);
        } else {
            SET_RG_5G_TX_GAIN_TXCAL(0x0);
        }
        UDELAY(1);        

        /*set RG_CAL_INDEX to WIFI5G_TXIQ*/
        SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_TXIQ);
        UDELAY(250);        
        while (GET_RO_5G_TXIQ_DONE == 0){
            count ++;
            if (count >1000) {
                break;
            }
            UDELAY(100);
        }
         /* set RG_CAL_INDEX to NONE*/
        SET_RG_CAL_INDEX(CAL_IDX_NONE);
    }   

    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("after 5G tx iq calibration, tx alpha: %d %d %d %d, tx theta %d %d %d %d\n", 
         GET_RG_TX_IQ_5100_ALPHA, GET_RG_TX_IQ_5100_THETA,
         GET_RG_TX_IQ_5500_ALPHA, GET_RG_TX_IQ_5500_THETA,
         GET_RG_TX_IQ_5700_ALPHA, GET_RG_TX_IQ_5700_THETA,
         GET_RG_TX_IQ_5900_ALPHA, GET_RG_TX_IQ_5900_THETA);
}

static void _turismoC_5g_rxiq_cal(struct ssv_hw *sh)
{
    int count = 0;
    int band;
    int regval, regval1;    

    SET_RG_SX5GB_RFCH_MAP_EN(1);   
    PRINT("--------------------------------------------\r\n");
    PRINT("before 5G rx iq calibration, rx alpha: %d %d %d %d, rx theta %d %d %d %d\n", 
         GET_RG_RX_IQ_5100_ALPHA, GET_RG_RX_IQ_5100_THETA,
         GET_RG_RX_IQ_5500_ALPHA, GET_RG_RX_IQ_5500_THETA,
         GET_RG_RX_IQ_5700_ALPHA, GET_RG_RX_IQ_5700_THETA,
         GET_RG_RX_IQ_5900_ALPHA, GET_RG_RX_IQ_5900_THETA);

    SET_RG_TXGAIN_PHYCTRL(1);
    
    //SET_RG_5G_TX_GAIN_RXIQCAL(0x2);
    
    SET_RG_5G_RFG_RXIQCAL(0x0);
    
    SET_RG_5G_PGAG_RXIQCAL(0x3);
 
    SET_RG_TONE_SCALE(0x80);
    
    SET_RG_PRE_DC_AUTO(1);

    SET_RG_TX_IQCAL_TIME(1);

    SET_RG_PHASE_1M(0xccc);
    SET_RG_PHASE_RXIQ_1M(0xccc);


    SET_RG_ALPHA_SEL(2);
      
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
    
    for (band = 0; band < 4; band ++){
        SET_RG_SX5GB_CHANNEL(cal_ch_5g[band]);
        if( band == 2 ) {
            SET_RG_5G_TX_GAIN_RXIQCAL(0x2);
        } else {
            SET_RG_5G_TX_GAIN_RXIQCAL(0x0);
        }
        UDELAY(1);       
    
        /*set RG_CAL_INDEX to CAL_IDX_WIFI5G_RXIQ*/
        SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_RXIQ);
        UDELAY(250);       
        while (GET_RO_5G_RXIQ_DONE == 0){
            count ++;
            if (count >1000) {
                break;
            }
            UDELAY(100);
        }
               
        SET_RG_PHASE_STEP_VALUE(0xccc);
        SET_RG_SPECTRUM_EN(1);
       
        SET_REG(ADR_RF_D_CAL_TOP_7,
            (0x1 << RG_SPECTRUM_PWR_UPDATE_SFT) | (0x1 << RG_SPECTRUM_LO_FIX_SFT), 0,
            (RG_SPECTRUM_PWR_UPDATE_I_MSK & RG_SPECTRUM_LO_FIX_I_MSK));
       
        MDELAY(10);
        
        SET_REG(ADR_RF_D_CAL_TOP_7,
            (0x0 << RG_SPECTRUM_PWR_UPDATE_SFT) | (0x1 << RG_SPECTRUM_LO_FIX_SFT), 0,
            (RG_SPECTRUM_PWR_UPDATE_I_MSK & RG_SPECTRUM_LO_FIX_I_MSK));
       
        regval1 = GET_RG_SPECTRUM_PWR_UPDATE;
        regval = GET_RO_SPECTRUM_IQ_PWR_31_0;
        PRINT("The spectrum power is 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
            ((regval1 >> 4) & 0xf), (regval1 & 0xf), ((regval >> 28) & 0xf), ((regval >> 24) & 0xf),
            ((regval >> 20) & 0xf), ((regval >> 16) & 0xf), ((regval >> 12) & 0xf), ((regval >> 8) & 0xf),
            ((regval >> 4) & 0xf), (regval & 0xf));
       
        SET_RG_PHASE_STEP_VALUE(0xF334);
        SET_RG_SPECTRUM_EN(1);
       
        SET_REG(ADR_RF_D_CAL_TOP_7,
            (0x1 << RG_SPECTRUM_PWR_UPDATE_SFT) | (0x1 << RG_SPECTRUM_LO_FIX_SFT), 0,
            (RG_SPECTRUM_PWR_UPDATE_I_MSK & RG_SPECTRUM_LO_FIX_I_MSK));
       
        MDELAY(10);
        
        SET_REG(ADR_RF_D_CAL_TOP_7,
            (0x0 << RG_SPECTRUM_PWR_UPDATE_SFT) | (0x1 << RG_SPECTRUM_LO_FIX_SFT), 0,
            (RG_SPECTRUM_PWR_UPDATE_I_MSK & RG_SPECTRUM_LO_FIX_I_MSK));
       
        regval1 = GET_RG_SPECTRUM_PWR_UPDATE;
        regval = GET_RO_SPECTRUM_IQ_PWR_31_0;
        PRINT("The spectrum power is 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
            ((regval1 >> 4) & 0xf), (regval1 & 0xf), ((regval >> 28) & 0xf), ((regval >> 24) & 0xf),
            ((regval >> 20) & 0xf), ((regval >> 16) & 0xf), ((regval >> 12) & 0xf), ((regval >> 8) & 0xf),
            ((regval >> 4) & 0xf), (regval & 0xf));
       
        SET_RG_SPECTRUM_EN(0);        
              
         /* set RG_CAL_INDEX to NONE*/
        SET_RG_CAL_INDEX(CAL_IDX_NONE);
    }     
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After 5G rx iq calibration, rx alpha: %d %d %d %d, rx theta %d %d %d %d\n", 
         GET_RG_RX_IQ_5100_ALPHA, GET_RG_RX_IQ_5100_THETA,
         GET_RG_RX_IQ_5500_ALPHA, GET_RG_RX_IQ_5500_THETA,
         GET_RG_RX_IQ_5700_ALPHA, GET_RG_RX_IQ_5700_THETA,
         GET_RG_RX_IQ_5900_ALPHA, GET_RG_RX_IQ_5900_THETA);
}

#define MULTIPLIER  1024
static void _start_padpd(struct ssv_hw *sh, struct ssv6006dpd *val, int pa_band, int init_gain, int *ret)
{
    int i, rg_tx_scale, regval;
    int am, pm;
    int slope_ini = 0, phase_ini = 0;
    int padpd_am = 0, padpd_pm = 0;
    u32 addr_am = 0, addr_pm = 0 , mask_am = 0, mask_pm = 0;
    
    PRINT("start PA DPD on band %d\n", pa_band);
    SET_RG_DPD_AM_EN(0);
    SET_RG_TXGAIN_PHYCTRL(1);
    
    SET_RG_MODE_MANUAL(1);
    SET_RG_MODE(MODE_STANDBY);

    if (pa_band == 0){
        SET_REG(ADR_SX_2_4GB_5GB_REGISTER_INT3BIT___CH_TABLE,
            (0x6 << RG_SX_CHANNEL_SFT) | (0x1 << RG_SX_RFCH_MAP_EN_SFT), 0,
        (RG_SX_CHANNEL_I_MSK & RG_SX_RFCH_MAP_EN_I_MSK));
        SET_RG_TX_GAIN_DPDCAL(6);
        SET_RG_PGAG_DPDCAL(init_gain);
        SET_RG_RFG_DPDCAL(0);
        SET_RG_TX_GAIN(PAPDP_GAIN_SETTING_2G);
        SET_RG_DPD_BB_SCALE_2500(0x80);
    } else {
        SET_RG_SX5GB_RFCH_MAP_EN(1);
        SET_RG_SX5GB_CHANNEL(cal_ch_5g[pa_band-1]);
 
        SET_RG_5G_PGAG_DPDCAL(init_gain);
        SET_RG_5G_RFG_DPDCAL(0);
        
        switch (pa_band){
            case BAND_5100:    
                SET_RG_5G_TX_PAFB_EN_F0(0);
                SET_RG_5G_TX_GAIN_F0(PAPDP_GAIN_SETTING);
                SET_RG_5G_TX_GAIN_DPDCAL(PAPDP_GAIN_SETTING);
                SET_RG_DPD_BB_SCALE_5100(0x80);
                break;
            case BAND_5500:    
                SET_RG_5G_TX_PAFB_EN_F1(0);
                SET_RG_5G_TX_GAIN_F1(PAPDP_GAIN_SETTING);
                SET_RG_5G_TX_GAIN_DPDCAL(PAPDP_GAIN_SETTING);
                SET_RG_DPD_BB_SCALE_5500(0x80);
                break;
            case BAND_5700:    
                SET_RG_5G_TX_PAFB_EN_F2(0);
                SET_RG_5G_TX_GAIN_F2(PAPDP_GAIN_SETTING_F2);
                SET_RG_5G_TX_GAIN_DPDCAL(PAPDP_GAIN_SETTING_F2);
                SET_RG_DPD_BB_SCALE_5700(0x80);
                break;
            case BAND_5900:    
                SET_RG_5G_TX_PAFB_EN_F3(0);
                SET_RG_5G_TX_GAIN_F3(PAPDP_GAIN_SETTING);
                SET_RG_5G_TX_GAIN_DPDCAL(PAPDP_GAIN_SETTING);
                SET_RG_DPD_BB_SCALE_5900(0x80);
                break;
            default:
                break;
        }
    }
    SET_RG_BB_SIG_EN(1);
    
    SET_RG_DC_RM_BYP(1);
    
    SET_RG_TX_IQ_SRC(2);   
    SET_RG_TX_BB_SCALE_MANUAL(1);

    SET_RG_TX_SCALE(0x80);    
    SET_RG_TONE_1_RATE(0xccc);
    SET_RG_TONE_SEL(1);
    
    SET_RG_RX_PADPD_TONE_SEL(0);
    SET_RG_RX_PADPD_DATA_SEL(0);
    SET_RG_RX_PADPD_LEAKY_FACTOR(7);
    SET_RG_RX_PADPD_EN(1);
    

    SET_RG_MODE(MODE_CALIBRATION);
    
    if (pa_band == 0) {
       SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_PADPD); 
    } else {
       SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_PADPD);
    }

    UDELAY(100);
    for (i = 0; i < MAX_PADPD_TONE; i++) {    
        
        if( i < 6 ){
            rg_tx_scale = (i+1)*8;
        } else {
            rg_tx_scale = 48+(i-5)*4;
        }
        SET_RG_TX_SCALE(rg_tx_scale);
        MDELAY(10);
        SET_RG_RX_PADPD_LATCH(1);
        UDELAY(50);
        SET_RG_RX_PADPD_LATCH(0);
        regval= REG32_R(ADR_WIFI_PADPD_CAL_RX_RO);
        am = (regval >>16) & 0x1ff;
        pm = regval & 0x1fff;
        if ( i == 0) {
            slope_ini = (am * MULTIPLIER) / rg_tx_scale;
            phase_ini = pm;
            PRINT("slope is (%d/%d), initial phase is %d \n", slope_ini, MULTIPLIER, pm);
        }
        PRINT(" tx scale is %d, am-am data is %d, am-pm data is %d\n", rg_tx_scale, am, pm);
        if (am != 0)
           padpd_am = (512 * rg_tx_scale * slope_ini ) / (am*MULTIPLIER);

        padpd_pm = (phase_ini >= pm) ? (phase_ini - pm) : (phase_ini - pm + 8192);
        
        PRINT(" index %d, padpd_am 0x%x, padpd_pm 0x%x\n", i, padpd_am, padpd_pm);
        
        switch (pa_band){
            case BAND_2G:
                addr_am = padpd_am_table_2G[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_am = padpd_am_table_2G[ (i >> 1)].mask1;
                } else {
                    mask_am = padpd_am_table_2G[ (i >> 1)].mask0;
                }
                addr_pm = padpd_pm_table_2G[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_pm = padpd_pm_table_2G[ (i >> 1)].mask1;
                } else {
                    mask_pm = padpd_pm_table_2G[ (i >> 1)].mask0;
                }
                break; 
            case BAND_5100:
                addr_am = padpd_am_table_5100[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_am = padpd_am_table_5100[ (i >> 1)].mask1;
                } else {
                    mask_am = padpd_am_table_5100[ (i >> 1)].mask0;
                }
                addr_pm = padpd_pm_table_5100[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_pm = padpd_pm_table_5100[ (i >> 1)].mask1;
                } else {
                    mask_pm = padpd_pm_table_5100[ (i >> 1)].mask0;
                }
                 break;
            case BAND_5500:
                 addr_am = padpd_am_table_5500[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_am = padpd_am_table_5500[ (i >> 1)].mask1;
                } else {
                    mask_am = padpd_am_table_5500[ (i >> 1)].mask0;
                }
                addr_pm = padpd_pm_table_5500[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_pm = padpd_pm_table_5500[ (i >> 1)].mask1;
                } else {
                    mask_pm = padpd_pm_table_5500[ (i >> 1)].mask0;
                }
                break; 
            case BAND_5700:
                addr_am = padpd_am_table_5700[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_am = padpd_am_table_5700[ (i >> 1)].mask1;
                } else {
                    mask_am = padpd_am_table_5700[ (i >> 1)].mask0;
                }
                addr_pm = padpd_pm_table_5700[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_pm = padpd_pm_table_5700[ (i >> 1)].mask1;
                } else {
                    mask_pm = padpd_pm_table_5700[ (i >> 1)].mask0;
                }
                break; 
            case BAND_5900:
                addr_am = padpd_am_table_5900[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_am = padpd_am_table_5900[ (i >> 1)].mask1;
                } else {
                    mask_am = padpd_am_table_5900[ (i >> 1)].mask0;
                }
                addr_pm = padpd_pm_table_5900[ (i >> 1)].addr;
                if (i & 0x1) {
                    mask_pm = padpd_pm_table_5900[ (i >> 1)].mask1;
                } else {
                    mask_pm = padpd_pm_table_5900[ (i >> 1)].mask0;
                }
                break; 
            default:
                break; 
        }
        regval = REG32_R(addr_am);
        REG32_W(addr_am, (regval & mask_am) | ((padpd_am) << ((i & 0x1)*16)) );
        if (i & 0x1){
            val->am[ (i >> 1)] = (regval & mask_am) | ((padpd_am) << ((i & 0x1)*16));
        }
        
        regval = REG32_R(addr_pm);
        REG32_W(addr_pm, (regval & mask_pm) | ((padpd_pm) << ((i & 0x1)*16)) );
        
        if (i & 0x1){
            val->pm[ (i >> 1)] = (regval & mask_pm) | ((padpd_pm) << ((i & 0x1)*16));
        }
        if (am >=510) {
            *ret = 1;
            break;
        }
    }
    
    SET_RG_CAL_INDEX(CAL_IDX_NONE);
    SET_RG_MODE(MODE_STANDBY);
    SET_RG_MODE_MANUAL(0);
    
    SET_RG_BB_SIG_EN(0);
    SET_RG_DC_RM_BYP(0);
    SET_RG_TX_IQ_SRC(0);
    SET_RG_TX_BB_SCALE_MANUAL(0);

    SET_RG_TX_SCALE(0x80);

    SET_RG_TONE_SEL(0);
    
    SET_RG_RX_PADPD_EN(0);
    
    switch (pa_band){
        case BAND_2G:
            SET_RG_DPD_BB_SCALE_2500(DEFAULT_DPD_BBSCALE_2500);
            break;
        case BAND_5100:
            SET_RG_DPD_BB_SCALE_5100(DEFAULT_DPD_BBSCALE_5100);
            break;
        case BAND_5500:    
            SET_RG_DPD_BB_SCALE_5500(DEFAULT_DPD_BBSCALE_5500);
            break;
        case BAND_5700:    
            SET_RG_DPD_BB_SCALE_5700(DEFAULT_DPD_BBSCALE_5700);
            break;
        case BAND_5900:    
            SET_RG_DPD_BB_SCALE_5900(DEFAULT_DPD_BBSCALE_5900);
            break;
        default:
            break;
    }    
    
    
    SET_RG_DPD_AM_EN(1);
    SET_RG_TXGAIN_PHYCTRL(0);
        
    PRINT("PA DPD done\n");

}


static void _check_padpd(struct ssv_hw *sh, struct ssv6006_padpd *dpd, int ch)
{
    int pa_band = 0, ret = 0;
    struct ssv6006dpd *val;    
    
    pa_band = _get_pa_band(ch);
    
    if ( dpd->dpd_done[pa_band] == false) {
        int init_gain = 5;
        
        PRINT("Start PADPD on band %d ,init gain %d\n", pa_band, init_gain);  
        while (1){
            val = &dpd->val[pa_band];
            ret = 0;
            _start_padpd(sh, val, pa_band, init_gain, &ret);
            if (!ret){
                break;
            }              
            init_gain--;
            PRINT("Start PADPD on band %d ,init gain %d\n", pa_band, init_gain);  
            if (init_gain < 0) {
                SET_RG_DPD_AM_EN(0);
                SET_RG_TXGAIN_PHYCTRL(1);
                PRINT("WARNING:PADPD FAIL\n");
                break;
            }
        }
        dpd->dpd_done[pa_band] = true; 
    }
}
#endif

static int ssv6006_turismoC_set_channel(struct ssv_softc *sc, struct ieee80211_channel  *chan, 
    enum nl80211_channel_type channel_type)
{
    int ch = chan->hw_value;
    struct ssv_hw *sh = sc->sh;
    struct ssv6006_padpd *dpd = &sc->dpd;


#ifdef USE_COMMON_MACRO
    TU_CHANGE_TURISMOC_CHANNEL(ch, channel_type, dpd );
#else
    const char *chan_type[]={"NL80211_CHAN_NO_HT",
	    "NL80211_CHAN_HT20",
	    "NL80211_CHAN_HT40MINUS",
	    "NL80211_CHAN_HT40PLUS"};

    PRINT("%s: ch %d, type %s\r\n", __func__, ch, chan_type[channel_type]);
    
    if (REG32_R(ADR_WIFI_PHY_COMMON_ENABLE_REG) != 0) {
    /* check padpd only when phy enable */    
        _check_padpd(sh, dpd, ch);
    }
    _set_turismoC_BW(sh, channel_type);
    
    if ((ch <=14) && (ch >=1)){
         
    	SET_SIFS(10);
    	SET_SIGEXT(6);
    	
    	if (dpd->wf_rx_abbctune == 0) {
    	    dpd->wf_rx_abbctune = GET_RG_WF_RX_ABBCTUNE;
    	}
    	
    	if ((ch >=13) || ((ch >= 9) && (channel_type == NL80211_CHAN_HT40PLUS)))
    	    /* set for ch13, 14 spur*/
    	    SET_RG_EN_RX_PADSW(1);
    	else
    	    SET_RG_EN_RX_PADSW(0);

        if ((ch >= 13)
            && ((channel_type == NL80211_CHAN_NO_HT) || (channel_type == NL80211_CHAN_NO_HT))){
            SET_RG_WF_RX_ABBCTUNE(0x3F);
        } else {
            SET_RG_WF_RX_ABBCTUNE(dpd->wf_rx_abbctune);
        } 

        if ((ch == 13)
            && ((channel_type == NL80211_CHAN_NO_HT) || (channel_type == NL80211_CHAN_NO_HT))){
            SET_RG_SC_CTRL0(1);
            SET_RG_ERASE_SC_NUM0(22); /*remove spur #22*/
            SET_RG_SC_CTRL1(1);
            SET_RG_ERASE_SC_NUM1(23); /*remove spur #23*/
        } else if (((ch == 9) && (channel_type == NL80211_CHAN_HT40PLUS))
                    || ((ch == 13) && (channel_type == NL80211_CHAN_HT40MINUS))) {
            SET_RG_SC_CTRL0(1);
            SET_RG_ERASE_SC_NUM0(52); /*remove spur #52*/
            SET_RG_SC_CTRL1(1);
            SET_RG_ERASE_SC_NUM1(53); /*remove spur #53*/
        } else {
            SET_RG_SC_CTRL0(0);
            SET_RG_ERASE_SC_NUM0(0x7f); /*disable remove spur*/
            SET_RG_SC_CTRL1(0);
            SET_RG_ERASE_SC_NUM1(0x7f); /*disable remove spur*/
       }

        _set_2g_channel(sh, ch);

    } else if (ch >=34){
    	
    	SET_SIFS(16);
    	SET_SIGEXT(0);

        _set_5g_channel(sh, ch);

    } else {
        
        PRINT("invalid channel %d\n", ch);
    }
#endif
    sc->dpd.pwr_mode  = NORMAL_PWR;
    sc->green_pwr = 0;    
    HAL_UPDATE_RF_PWR(sc);
    return 0;
}

static void ssv6006_turismoC_init_cali (struct ssv_hw *sh)
{
#ifdef USE_COMMON_MACRO
    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ){
        TU_INIT_TURISMOC_CALI;
    } else {
        TU_INIT_TURISMOC_2G_CALI;
    }
#else                                                                    

    /* disable dpd before cali */
    SET_RG_DPD_AM_EN(0);
    SET_RG_TXGAIN_PHYCTRL(1);
    
    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ) 
        REG32_W(ADR_WIFI_PADPD_5G_BB_GAIN_REG, 0x80808080);

    _turismoC_pre_cal(sh);

    _turismoC_2p4g_rxdc_cal(sh);

    _turismoC_bw20_rxrc_cal(sh);
    
    _turismoC_bw40_rxrc_cal(sh);
    
    _turismoC_txdc_cal(sh);

    _turismoC_txiq_cal(sh);    

    _turismoC_rxiq_cal(sh);
    
    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ) {
        _turismoC_5g_rxdc_cal(sh);
        
        _turismoC_5g_txdc_cal(sh);

        _turismoC_5g_txiq_cal(sh);
    
        _turismoC_5g_rxiq_cal(sh);
        
    }
    _turismoC_post_cal(sh);
#endif

}

#if 0
void ssv6006_turismoC_write_rf_table(struct ssv_hw *sh )
{                                                                
                                                 
    //u32 i = 0;
                                                    
    //for( i = 0; i < sizeof(ssv6006_turismoC_rf_setting)/sizeof(ssv_cabrio_reg); i++) {
    //   SMAC_REG_WRITE(sh, ssv6006_turismoC_rf_setting[i].address, ssv6006_turismoC_rf_setting[i].data );
    //   udelay(50); // should delay a while when set external RF                              
    //}                                                            
                                                         
}
#endif

#ifndef USE_COMMON_MACRO
static void _update_rf_patch(struct ssv_hw *sh, int xtal)
{
    // default settings for xtal is 26m, change setting for other case
    SET_RG_DP_XTAL_FREQ(xtal);
    SET_RG_SX_XTAL_FREQ(xtal);
}
#endif

static int ssv6006_turismoC_set_pll_phy_rf(struct ssv_hw *sh
    , ssv_cabrio_reg *rf_tbl, ssv_cabrio_reg *phy_tbl)
{	
    int  ret = 0;
    int  xtal;
    struct ssv6006_patch patch;

    switch (sh->cfg.crystal_type){
        case SSV6XXX_IQK_CFG_XTAL_16M:
            xtal = XTAL16M;
            break;
        case SSV6XXX_IQK_CFG_XTAL_24M:
            xtal = XTAL24M;
            break;
        case SSV6XXX_IQK_CFG_XTAL_26M:
            xtal = XTAL26M;
            break;
        case SSV6XXX_IQK_CFG_XTAL_40M:
            xtal = XTAL40M;
            break;
        case SSV6XXX_IQK_CFG_XTAL_12M:
            xtal = XTAL12M;
            break;
        case SSV6XXX_IQK_CFG_XTAL_20M:
            xtal = XTAL20M;
            break;
        case SSV6XXX_IQK_CFG_XTAL_25M:
            xtal = XTAL25M;
            break;
        case SSV6XXX_IQK_CFG_XTAL_32M:
            xtal = XTAL32M;
            break; 
        default:
            printk("Please redefine xtal_clock(wifi.cfg)!!\n");
            WARN_ON(1);
            return 1;
            break;      
    }

    patch.xtal = xtal;
    if (sh->cfg.clk_src_80m)
        patch.cpu_clk = CLK_80M;
    else
        patch.cpu_clk = CLK_40M;
   

#ifdef USE_COMMON_MACRO
    PRINT("%s: use common macro\n",__func__);
    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ) {
       INIT_TURISMOC_SYS(patch, AG_BAND_BOTH);
    } else {
       INIT_TURISMOC_SYS(patch,G_BAND_ONLY);
    }
#else  
    PRINT("%s: Not use common macro\n",__func__);
    ret = SSV6XXX_SET_HW_TABLE(sh, ssv6006_turismoC_rf_setting);
    _update_rf_patch(sh, xtal);    
    SET_RG_EN_IOTADC_160M(0);
    ssv6006_turismoC_init_PLL(sh);
    REG32_W(ADR_WIFI_PHY_COMMON_ENABLE_REG, 0);
    ret = SSV6XXX_SET_HW_TABLE(sh, ssv6006_turismoC_phy_setting);
    /* do clock switch */
    SET_CLK_DIGI_SEL( patch.cpu_clk);
    MSLEEP(1);  /* wait for clock settled*/
    ssv6006_turismoC_init_cali(sh);
#endif
    sh->default_txgain[BAND_2G] = GET_RG_TX_GAIN;
    sh->default_txgain[BAND_5100] = GET_RG_5G_TX_GAIN_F0;
    sh->default_txgain[BAND_5500] = GET_RG_5G_TX_GAIN_F1;
    sh->default_txgain[BAND_5700] = GET_RG_5G_TX_GAIN_F2;
    sh->default_txgain[BAND_5900] = GET_RG_5G_TX_GAIN_F3;

    return ret;
}

static bool ssv6006_turismoC_set_rf_enable(struct ssv_hw *sh, bool val)
{

	// set rf channel manual on
    SMAC_REG_SET_BITS(sh, ADR_MODE_REGISTER, 1 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_MSK);
    msleep(1); // for hs3w send
    
    if (val){
        // set RG_MODE to WIFI_RX
        SMAC_REG_SET_BITS(sh, ADR_MODE_REGISTER, 3 << RG_MODE_SFT, RG_MODE_MSK);
	} else {
        // set RG_MODE to IDLE mode        SMAC_REG_READ(sh, ADR_MODE_REGISTER, &regval);// dummy read
        SMAC_REG_SET_BITS(sh, ADR_MODE_REGISTER, 0 << RG_MODE_SFT, RG_MODE_MSK);
	}      

    msleep(1); // for hs3w send

    //set RG_MODE_MANUAL off
    SMAC_REG_SET_BITS(sh, ADR_MODE_REGISTER, 0 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_MSK);         
  
	return true;   
}

static bool ssv6006_turismoC_dump_phy_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6006_turismoC_phy_setting;

    snprintf_res(cmd_data, ">> PHY Register Table:\n");

    for(s = 0; s < ssv6006_turismoC_phy_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);      
    }
    snprintf_res(cmd_data, ">>PHY Table version: %s\n", SSV6006_TURISMOC_PHY_TABLE_VER);
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6006_turismoC_dump_rf_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6006_turismoC_rf_setting;

    snprintf_res(cmd_data, ">> RF Register Table:\n");

    for(s = 0; s < ssv6006_turismoC_rf_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);      
    }
    snprintf_res(cmd_data, ">>RF Table version: %s\n", SSV6006_TURISMOC_RF_TABLE_VER);
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6006_turismoC_support_iqk_cmd(struct ssv_hw *sh)
{
    return false;
}

static void ssv6006_cmd_turismoC_cali(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    struct ssv_softc *sc = sh->sc;
    
    if(!strcmp(argv[1], "do")){
        
        ssv6006_turismoC_init_cali(sh);
        snprintf_res(cmd_data,"\n   CALIRATION DONE\n");
    
    } else if(!strcmp(argv[1], "show")) {
        u32 i, regval = 0, wifi_dc_addr;
        bool chgband = false;

        snprintf_res(cmd_data,"---------2.4G DC Calibration result-----------");
        for (i = 0; i < 21; i++) {
           if (i %4 == 0)
              snprintf_res(cmd_data,"\n");
           wifi_dc_addr = ADR_WF_DCOC_IDAC_REGISTER1+ (i << 2); 
           SMAC_REG_READ(sh, wifi_dc_addr, &regval);
           snprintf_res(cmd_data,"addr %x : val %x, ", wifi_dc_addr, regval);
        }
        snprintf_res(cmd_data,"\n--------------------------------------------\n");
        snprintf_res(cmd_data,"WiFi BW20 RG_WF_RX_ABBCTUNE: %d\n", GET_RG_WF_RX_ABBCTUNE);
        snprintf_res(cmd_data,"WiFi BW40 RG_WF_RX_N_ABBCTUNE: %d\n", GET_RG_WF_N_RX_ABBCTUNE);
        snprintf_res(cmd_data,"TxDC calibration WiFi 2P4G Tx DAC IOFFSET: %d, QOFFSET %d\n",
            GET_RG_WF_TX_DAC_IOFFSET, GET_RG_WF_TX_DAC_QOFFSET);
        snprintf_res(cmd_data,"Tx iq calibration, tx alpha: %d, tx theta %d\r\n",
            GET_RO_TX_IQ_ALPHA, GET_RO_TX_IQ_THETA);
        snprintf_res(cmd_data,"Rx iq calibration, rx alpha: %d, rx theta %d\r\n",
            GET_RO_RX_IQ_ALPHA, GET_RO_RX_IQ_THETA);
        
        if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ) {
            snprintf_res(cmd_data,"--------------------------------------------\r\n");                              
            snprintf_res(cmd_data,"--------- 5 G Rx DC Calibration result----------------");                        
                for (i = 0; i < 21; i++) {                                                              
                   if (i %4 == 0)                                                                       
                      snprintf_res(cmd_data,"\r\n");                                                                    
                   wifi_dc_addr = (ADR_5G_DCOC_IDAC_REGISTER1)+ (i << 2);                               
                   snprintf_res(cmd_data,"addr %x : val %x, ", wifi_dc_addr, REG32_R(wifi_dc_addr));                    
                }                                                                                       
                snprintf_res(cmd_data,"\r\n");           
            if (GET_RG_RF_5G_BAND == 0) {
                SET_RG_RF_5G_BAND(1);
                chgband = true;
            }
            snprintf_res(cmd_data,"5G txdc calibration WiFi 5G Tx DAC IOFFSET: %d, QOFFSET %d\r\n",
                GET_RG_5G_TX_DAC_IOFFSET, GET_RG_5G_TX_DAC_QOFFSET);      
            snprintf_res(cmd_data,"5G tx iq calibration, tx alpha: %d, tx theta %d\r\n",
                GET_RO_TX_IQ_ALPHA, GET_RO_TX_IQ_THETA); 
            snprintf_res(cmd_data,"5G rx iq calibration, rx alpha: %d, rx theta %d\r\n",
                GET_RO_RX_IQ_ALPHA, GET_RO_RX_IQ_THETA); 
            if (chgband) SET_RG_RF_5G_BAND(0);
        } 
    }else if(!strcmp(argv[1], "dpd")){
        if(!strcmp(argv[2], "show")){
            int pa_band;
            
            snprintf_res(cmd_data, "\n DPD result: \n");
            for (pa_band = 0; pa_band < PADPDBAND; pa_band++){

                switch (pa_band){
                    case 0:
                        snprintf_res(cmd_data,"\t 2G       channel <= 14 ");
                        break;
                    case 1:
                        snprintf_res(cmd_data,"\t 5G       channel <  36");
                        break;
                    case 2:
                        snprintf_res(cmd_data,"\t 5G 36 <= channel < 100");
                        break;
                    case 3:
                        snprintf_res(cmd_data,"\t 5G 100<= channel < 140");
                        break;
                    case 4:
                        snprintf_res(cmd_data,"\t 5G 140<= channel");
                        break;
                    default:
                        break;
                }
                if (pa_band == sc->dpd.current_band){
                    snprintf_res(cmd_data,":current band");
                }
                snprintf_res(cmd_data,"\n");
                if (sc->dpd.dpd_done[pa_band]){
                    int i;
                    
                    snprintf_res(cmd_data,"\t\tam_am:");
                    for (i = 0 ; i < MAX_PADPD_TONE/2; i ++){
                        if (i %4 == 0)
                            snprintf_res(cmd_data,"\n\t\t");
                        snprintf_res(cmd_data, "%03d %03d ", sc->dpd.val[pa_band].am[i] & 0xffff,
                            (sc->dpd.val[pa_band].am[i] >>16) & 0xffff);

                    }
                    snprintf_res(cmd_data,"\n\t\tam_pm:");
                    for (i = 0 ; i < MAX_PADPD_TONE/2; i ++){
                        if (i %4 == 0)
                            snprintf_res(cmd_data,"\n\t\t");
                        snprintf_res(cmd_data, "0x%04x 0x%04x ", sc->dpd.val[pa_band].pm[i] & 0xffff,
                            (sc->dpd.val[pa_band].pm[i] >>16) & 0xffff);

                    }
                    snprintf_res(cmd_data,"\n");
                } else {
                    snprintf_res(cmd_data,"\t\t DPD result not available\n");
                }
            }
        } else if(!strcmp(argv[2], "enable")){
 
            SET_RG_DPD_AM_EN(1);
            SET_RG_TXGAIN_PHYCTRL(0);
            snprintf_res(cmd_data,"enable DPD\r\n");
            
        } else if(!strcmp(argv[2], "disable")){
 
            SET_RG_DPD_AM_EN(0);
            SET_RG_TXGAIN_PHYCTRL(1);
            snprintf_res(cmd_data,"disable DPD\r\n");

        } else if(!strcmp(argv[2], "do")){ 
            int pa_band = 0, ch = sc->hw_chan;
            struct ssv6006_padpd *dpd = &sc->dpd;   
            
            pa_band = _get_pa_band(ch);
            dpd->dpd_done[pa_band] = false;
        #ifdef USE_COMMON_MACRO
            CHECK_PADPD(dpd, ch);
        #else
            _check_padpd(sh, dpd, ch);
        #endif
            snprintf_res(cmd_data,"DPD done\r\n");

        } else {
            snprintf_res(cmd_data,"\n cali [do|show|dpd(do|show|enable|disable)] \n");    
        }
         
    }else {
        snprintf_res(cmd_data,"\n cali [do|show|dpd(show |enable|disable)] \n");
    }     
    
}

static void ssv6006c_cmd_lpbk_setup_env_sec_talbe(struct ssv_hw *sh)
{
    int i, address = 0;
    u32 temp;
    u32 sec_key_tbl_base = sh->hw_sec_key[0];
    u32 sec_key_tbl = sec_key_tbl_base;
               
    u8 sec_tbl[] = {
        //group key 1
        0x01, 0x03, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0x58, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x87, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x87, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03, 0x59, 0xce, 0x11, 0x7c,
        0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f, 0x53, 0x90, 0xe8, 0x34,
        0x88, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03, 0x88, 0xbb, 0xdd, 0x0a,
        0x7f, 0x6c, 0x52, 0x03, 0x5a, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41,
        0xc7, 0x5d, 0xc4, 0x5f, 0x53, 0x90, 0xe8, 0x34, 0x89, 0xbb, 0xdd, 0x0a,
        0x7f, 0x6c, 0x52, 0x03, 0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x5a, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //group key 2
        0x01, 0x03, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0x58, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x87, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x87, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03, 0x59, 0xce, 0x11, 0x7c,
        0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f, 0x53, 0x90, 0xe8, 0x34,
        0x88, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03, 0x88, 0xbb, 0xdd, 0x0a,
        0x7f, 0x6c, 0x52, 0x03, 0x5a, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41,
        0xc7, 0x5d, 0xc4, 0x5f, 0x53, 0x90, 0xe8, 0x34, 0x89, 0xbb, 0xdd, 0x0a,
        0x7f, 0x6c, 0x52, 0x03, 0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x5a, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x89, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //sta1 wep64bit
        0x00, 0x01, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x5e, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //sta2 wep128bit
        0x00, 0x02, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x5e, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //sta3 tkip
        0x00, 0x03, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x5e, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        //sta4 aes
        0x00, 0x04, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x5e, 0xce, 0x11, 0x7c, 0x54, 0x74, 0x37, 0x41, 0xc7, 0x5d, 0xc4, 0x5f,
        0x53, 0x90, 0xe8, 0x34, 0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
        0x8d, 0xbb, 0xdd, 0x0a, 0x7f, 0x6c, 0x52, 0x03,
    };

    address = sec_key_tbl;
    for (i=0; i < sizeof(sec_tbl); i+=4) { 
        memcpy(&temp, &sec_tbl[i], sizeof(u32)); 
        SMAC_REG_WRITE(sh, address, temp);
        address += 4;
    }
}

static void ssv6006c_cmd_loopback_setup_env(struct ssv_hw *sh)
{
    u32 mac0, mac1;
    struct ieee80211_channel chan;
    
    //phy enable
    SSV_PHY_ENABLE(sh, 1);

    // HCI setup
    SET_RX_2_HOST(1);
    SET_TX_INFO_SIZE(80);
    SET_RX_INFO_SIZE(80);

    // Secutiry setup
    SET_LUT_SEL_V2(1);
    ssv6006c_cmd_lpbk_setup_env_sec_talbe(sh);

    // sta mac & bssid & peer
    mac0 = GET_STA_MAC_31_0;
    mac1 = GET_STA_MAC_47_32;
    SET_BSSID_31_0(mac0);
    SET_BSSID_47_32(mac1);
    SET_PEER_MAC0_31_0(mac0);
    SET_PEER_MAC0_47_32(mac1);
    SET_VALID0(1);

    // mrx packet flow
    HAL_SET_RX_FLOW(sh, RX_DATA_FLOW, RX_CIPHER_MIC_HCI);
    HAL_SET_RX_FLOW(sh, RX_MGMT_FLOW, RX_HCI);
    HAL_SET_RX_FLOW(sh, RX_CTRL_FLOW, RX_HCI);
   
    
    // Mac lpbk setup
    switch (sh->cfg.lpbk_type) {
        case SSV6006_CMD_LPBK_TYPE_MAC:
            // hci lpbk default
            SET_TX_PKT_SEND_TO_RX(0);
            // mac lpbk 
            SET_RG_MAC_LPBK(1);
            SET_MTX_MTX2PHY_SLOW(1);
            SET_MTX_M2M_SLOW_PRD(3);
            // phy lpbk default
            SET_RG_PMDLBK(0);
            break;
        case SSV6006_CMD_LPBK_TYPE_PHY: 
            // hci lpbk default
            SET_TX_PKT_SEND_TO_RX(0);
            // mac lpbk defualt 
            SET_RG_MAC_LPBK(0);
            SET_MTX_MTX2PHY_SLOW(0);
            SET_MTX_M2M_SLOW_PRD(0);
            // phy lpbk 
            SET_RG_PMDLBK(1);
            break;
        case SSV6006_CMD_LPBK_TYPE_HCI:
            // hci lpbk, it is necessary to disable phy
            // Otherwise, the packet from phy will affects the lpbk. 
            SSV_PHY_ENABLE(sh, 0);
            SET_TX_PKT_SEND_TO_RX(1);
            // mac lpbk defualt 
            SET_RG_MAC_LPBK(0);
            SET_MTX_MTX2PHY_SLOW(0);
            SET_MTX_M2M_SLOW_PRD(0);
            // phy lpbk default
            SET_RG_PMDLBK(0);
            break;
        case SSV6006_CMD_LPBK_TYPE_2GRF:
            // channel setting
            memset(&chan, 0 , sizeof( struct ieee80211_channel));
            chan.hw_value = 6;
            HAL_SET_CHANNEL(sh->sc, &chan, NL80211_CHAN_HT20);
            SET_RG_TXGAIN_PHYCTRL(1);
            SET_RG_BW_MANUAL(1);
            SET_RG_BW_HT40(0);
            // rf setting
            SET_RG_TX_GAIN_DPDCAL(0x0c);
            SET_RG_PGAG_DPDCAL(0x3);
            SET_RG_RFG_DPDCAL(0);
            SET_RG_BB_SIG_EN(0x1);
            SET_RG_MODE_MANUAL(0x1);
            SET_RG_MODE(1); //calibration mode
            SET_RG_CAL_INDEX(7);
            // phy setting
            SSV_PHY_ENABLE(sh, 0);
            SET_RG_LBK_DIG_SEL(0);
            SET_RG_LBK_ANA_PATH(1);
            SET_RG_PMDLBK(1);
            SET_RG_SYSTEM_BW(0);
            SET_RG_PRIMARY_CH_SIDE(1);
            SSV_PHY_ENABLE(sh, 1);
            break;
        case SSV6006_CMD_LPBK_TYPE_5GRF:
            // channel setting
            memset(&chan, 0 , sizeof( struct ieee80211_channel));
            chan.hw_value = 100;
            HAL_SET_CHANNEL(sh->sc, &chan, NL80211_CHAN_HT20);
            SET_RG_TXGAIN_PHYCTRL(0x1);
            SET_RG_BW_MANUAL(0x1);
            SET_RG_BW_HT40(0);
            // rf setting
            SET_RG_5G_TX_GAIN_DPDCAL(0xc);
            SET_RG_5G_PGAG_DPDCAL(0x3);
            SET_RG_5G_RFG_DPDCAL(0);
            SET_RG_BB_SIG_EN(0x1);
            SET_RG_MODE_MANUAL(1);
            SET_RG_MODE(1);
            SET_RG_CAL_INDEX(15);
            // phy setting
            SSV_PHY_ENABLE(sh, 0);
            SET_RG_LBK_DIG_SEL(0);
            SET_RG_LBK_ANA_PATH(1);
            SET_RG_PMDLBK(1);
            SET_RG_SYSTEM_BW(0);
            SET_RG_PRIMARY_CH_SIDE(1);
            SSV_PHY_ENABLE(sh, 1);
            break;
        default:
            printk("LPBK invalid setting!!!\n");
            break;
    }

    // setup lpbk
    msleep(10);
    sh->sc->lpbk_enable = true;
}

static void ssv6006_cmd_turismoC_loopback(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data; 
    char *lpbk_types[] = {"phy", "mac", "hci", "2grf", "5grf"};
    char *lpbk_secs[] = {"open/security", "wep64", "wep128", "tkip", "aes", "open"};
    char *endp;
    int  val;
    
    if (argc < 2) {
        snprintf_res(cmd_data, "\n lpbk [show|set|start]\n");
        return;
    }
        
    if (!strcmp(argv[1], "show")) {
        snprintf_res(cmd_data, "\n lpbk parameters for ssv6006\n");
        snprintf_res(cmd_data, " lpbk packet count    = %d\n", (!sh->cfg.lpbk_pkt_cnt) ? 10: sh->cfg.lpbk_pkt_cnt);
        snprintf_res(cmd_data, " lpbk type            = %s\n", lpbk_types[sh->cfg.lpbk_type]);
        snprintf_res(cmd_data, " lpbk security        = %s\n", lpbk_secs[sh->cfg.lpbk_sec]);
        snprintf_res(cmd_data, " lpbk fixed rate      = %s\n", (!sh->cfg.lpbk_mode ? "all rates" : "sample rate"));
    
    } else if (!strcmp(argv[1], "set")) {
        if (argc == 4) {
            
            val = simple_strtoul(argv[3], &endp, 0);
            snprintf_res(cmd_data, "\n set lpbk %s to %d\n", argv[2], val);

            if (!strcmp(argv[2], "pkt_cnt")) {
                sh->cfg.lpbk_pkt_cnt = val;

            } else if (!strcmp(argv[2], "type")) {
                sh->cfg.lpbk_type = (val >= MAX_SSV6006_CMD_LPBK_TYPE) ? 0 : val;

            } else if (!strcmp(argv[2], "security")) {
                sh->cfg.lpbk_sec = (val >= MAX_SSV6006_CMD_LPBK_SEC) ? 0 : val;

            } else if (!strcmp(argv[2], "fixed_rate")) {
                sh->cfg.lpbk_mode = (val > 0) ? 1 : 0;

            } else {
                snprintf_res(cmd_data, "\n lpbk set [env_setup|pkt_cnt|type|security|fixed_rate]\n");
            }
        } else {
            snprintf_res(cmd_data, "\n lpbk set [env_setup|pkt_cnt|type|security|fixed_rate]\n");
        }
    } else if (!strcmp(argv[1], "start")) {
        HAL_CMD_LOOPBACK_SETUP_ENV(sh);
        HAL_CMD_LOOPBACK_START(sh);
    
    } else {
        snprintf_res(cmd_data, "\n lpbk [show|set|start]\n");
    }
}

static void ssv6006_cmd_turismoC_txgen(struct ssv_hw *sh)
{
    struct sk_buff *skb = NULL;
    int    len = (int) sizeof(pkt1614) ;
    unsigned char *data = NULL;
    struct ssv6006_tx_desc *tx_desc;
    struct ssv_softc *sc = sh->sc;
    
    skb = ssv_skb_alloc(sc, len);
    if (!skb)
        goto out;

    data = skb_put(skb, len);
    memcpy(data, pkt1614, len);
    tx_desc = (struct ssv6006_tx_desc *)data;
    tx_desc->drate_idx0 = sc->rf_rc;

    SET_RG_TXD_SEL(0);  // set tx gen path disable
    SET_RG_TX_START(0); // disable trigger tx!!
    HCI_SEND_CMD(sc->sh, skb);

out:
    if (skb)
        ssv_skb_free(sc, skb); 

}

static void ssv6006_cmd_turismoC_rf(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    u32 regval = 0;
    char *endp;
    struct ssv_softc *sc = sh->sc;
    int ch = sc->hw_chan;
    int pa_band =0;
    unsigned char rate_tbl[] = {
        0x00,0x01,0x02,0x03,                        // B mode long preamble [0~3]
        0x00,0x12,0x13,                             // B mode short preamble [4~6], no 2M short preamble
        0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,    // G mode [7~14]
        0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,    // N mode HT20 long GI mixed format [15~22]
        0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,    // N mode HT20 short GI mixed format  [23~30]
        0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,    // N mode HT40 long GI mixed format [31~38]
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,    // N mode HT40 short GI mixed format  [39~46]
    };
#define SIZE_RATE_TBL   (sizeof(rate_tbl) / sizeof((rate_tbl)[0]))

    pa_band = _get_pa_band(ch);
    
    if ( argc < 2) goto out;

    if(!strcmp(argv[1], "bbscale")){
               
        if (argc == 4){
            regval = simple_strtoul(argv[3], &endp, 0);
            snprintf_res(cmd_data,"Set bbscale to 0x%x\n", regval);
            if (!strcmp(argv[2], "ht40")){
                REG32_W(ADR_WIFI_PHY_COMMON_BB_SCALE_REG_3, regval);
            } else if(!strcmp(argv[2], "ht20")){
                REG32_W(ADR_WIFI_PHY_COMMON_BB_SCALE_REG_2, regval);
            } else if(!strcmp(argv[2], "g")){
                REG32_W(ADR_WIFI_PHY_COMMON_BB_SCALE_REG_1, regval);
            } else if(!strcmp(argv[2], "b")){
                SET_RG_BB_SCALE_BARKER_CCK(regval);
            } else if(!strcmp(argv[2], "dpd")){
                    
                switch (pa_band){
                    case BAND_2G:
                        SET_RG_DPD_BB_SCALE_2500(regval);
                        break;
                    case BAND_5100:
                        SET_RG_DPD_BB_SCALE_5100(regval);
                        break;
                    case BAND_5500:    
                        SET_RG_DPD_BB_SCALE_5500(regval);                   
                        break;
                    case BAND_5700:    
                        SET_RG_DPD_BB_SCALE_5700(regval);                    
                        break;
                    case BAND_5900:    
                        SET_RG_DPD_BB_SCALE_5900(regval);                   
                        break;
                    default:
                        break;
                }                       
            }
            return;

        } else {
            snprintf_res(cmd_data,"./cli rf bbscale ht40|ht20|g|b|dpd [value]\n");
            return;
        } 

    } else if(!strcmp(argv[1], "ack")){
           
        if (argc == 3) {
            if (!strcmp(argv[2], "disable")){
                SET_RG_TXD_SEL(1);
                snprintf_res(cmd_data,"\n set %s %s\n", argv[1], argv[2]);
            } else if (!strcmp(argv[2], "enable")){
                SET_RG_TXD_SEL(0);
                snprintf_res(cmd_data,"\n set %s %s\n", argv[1], argv[2]);
            }
        } else {
            snprintf_res(cmd_data,"\n\t incorrect set ack format\n");
        }
            
        return; 

    } else if(!strcmp(argv[1], "ifs")) {
            
        if (argc == 3){
            regval = simple_strtoul(argv[2], &endp, 0);
            SET_RG_IFS_TIME((regval & 0x3f));
            SET_RG_IFS_TIME_EXT((regval >> 6));
        }
        snprintf_res(cmd_data,"\n set ifs to %d us\n", regval);
        return;

    } else if(!strcmp(argv[1], "rate")){
        if (argc == 3){
            regval = simple_strtoul(argv[2], &endp, 0);
            sc->rf_rc = 0;
            if ((regval != 4) && (regval < SIZE_RATE_TBL)) {
                sc->rf_rc = rate_tbl[regval];
                    
                switch ((sc->rf_rc & SSV6006RC_PHY_MODE_MSK) >> SSV6006RC_PHY_MODE_SFT){
        
                    case 0:
                        SET_RG_PKT_MODE(0); //B
                        break;
                    case 2:
                        SET_RG_PKT_MODE(1); //G
                        break;
                    case 3:
                        SET_RG_PKT_MODE(2); //N
                        break;            
                    default:
                        snprintf_res(cmd_data,"\t %s\n", "Invalid phy mode");
                        break;
                }
            
                SET_RG_CH_BW(((sc->rf_rc & SSV6006RC_20_40_MSK) >> SSV6006RC_20_40_SFT)) ; //set BW
                SET_RG_SHORTGI((sc->rf_rc & SSV6006RC_LONG_SHORT_MSK) >> SSV6006RC_LONG_SHORT_SFT); // set SGI
                SET_RG_RATE((sc->rf_rc & SSV6006RC_RATE_MSK) >> SSV6006RC_RATE_SFT); // set rate
                    
                snprintf_res(cmd_data,"Set rate to 0x%x\n", sc->rf_rc);
                return;
            } else {
                snprintf_res(cmd_data,"Not support rf rate index %d\n", regval);
                return;
            }
            
        } else {
            snprintf_res(cmd_data,"\n\t Incorrect rf rate set format\n");
            return;
        }

    } else if(!strcmp(argv[1], "freq")){
        if (argc == 3){
            regval = simple_strtoul(argv[2], &endp, 0);
            snprintf_res(cmd_data,"Set cbanki/cbanko to 0x%x\n", regval);
            SET_RG_XO_CBANKI(regval);
            SET_RG_XO_CBANKO(regval);
            return;
        } else{
            snprintf_res(cmd_data,"./cli rf freq [value]\n");
            return;
        }
        
    } else if(!strcmp(argv[1], "rfreq")){
        if (argc == 2){
            snprintf_res(cmd_data,"Get freq 0x%x/0x%x\n", GET_RG_XO_CBANKI, GET_RG_XO_CBANKO);
            return;
        } else{
            snprintf_res(cmd_data,"./cli rf rfreq\n");
            return;
        }
        
    } else if(!strcmp(argv[1], "greentx")) {
        if (argc == 3) {
            if(!strcmp(argv[2], "enable")) {
                sh->cfg.greentx |= GT_ENABLE;
                snprintf_res(cmd_data,"\n\t Green Tx enabled, start attenuation from -%d dB\n"
                    , sh->cfg.greentx & GT_PWR_START_MASK);
            } else if(!strcmp(argv[2], "disable")) {
                sh->cfg.greentx &= GT_ENABLE;
                snprintf_res(cmd_data,"\n\t Green Tx disabled");
            }
        } else {
            snprintf_res(cmd_data,"\n\t Incorrect rf greentx format\n");
        }
        return; 
    
    } else  if (!strcmp(argv[1], "rgreentx")){
        snprintf_res(cmd_data,"\n cfg.greentx 0x%x, Tx Gain : %d %d %d %d %d\n",
            sh->cfg.greentx,  GET_RG_TX_GAIN, GET_RG_5G_TX_GAIN_F0,
            GET_RG_5G_TX_GAIN_F1, GET_RG_5G_TX_GAIN_F2, GET_RG_5G_TX_GAIN_F3);
        return;

    } else if(!strcmp(argv[1], "rssi")){
        snprintf_res(cmd_data,"\n ofdm RSSI -%d, B mode RSSI -%d\n", 
            GET_RO_11GN_RCPI, GET_RO_11B_RCPI);
        return;

    } else if(!strcmp(argv[1], "sar")){
        //Eanble sensor
        SET_RG_SARADC_THERMAL(0);
        SET_RG_EN_SARADC(0);
        SET_RG_SARADC_THERMAL(1);
        SET_RG_EN_SARADC(1);
           
        do{
            if (GET_SAR_ADC_FSM_RDY)
                break;
        }while(1);
 
        snprintf_res(cmd_data,"\nuSarCode[%d] \n", GET_DB_DA_SARADC_BIT);
        return;

    } else if (!strcmp(argv[1], "phy_txgen")){
        if  (argc == 3){
            SET_RG_TX_START(0);
            regval = simple_strtoul(argv[2], &endp, 0);
            SET_RG_LENGTH(1500);
            SET_RG_TX_CNT_TARGET(regval);
            SET_RG_TXD_SEL(1);  // set tx gen path enable
            SET_RG_TX_START(1); // trigger tx!! 
        }
        regval = (GET_RG_IFS_TIME)+ (GET_RG_IFS_TIME_EXT << 6); 
        snprintf_res(cmd_data,"\n\t phy_txgen triggered!! ifs %d us\n", regval);
        return;
    } else if(!strcmp(argv[1], "block")){

        sc->sc_flags |= SC_OP_BLOCK_CNTL;
        snprintf_res(cmd_data,"\n\t block control form system\n");
        return;
               
    } else if(!strcmp(argv[1], "unblock")){

        sc->sc_flags &= ~SC_OP_BLOCK_CNTL;
        SET_RG_TXD_SEL(0);  // set tx gen path disable
        SET_RG_TX_START(0); // disable trigger tx!!
        snprintf_res(cmd_data,"\n\t unblock control form system\n");
        return;
               
    } else if(!strcmp(argv[1], "count")){
        if  (argc == 3) {
            int     count = 0,  err = 0, integer= 0, point = 0;
            bool valid =false;
            if (!strcmp(argv[2], "0")){
                count = GET_RO_11B_PACKET_CNT;
                err = GET_RO_11B_PACKET_ERR_CNT;
                valid = true;  
            } else if(!strcmp(argv[2], "1")){
                count = GET_RO_11GN_PACKET_CNT;
                err = GET_RO_11GN_PACKET_ERR_CNT;                
                valid = true;   
            }
            if (count != 0) {
                integer = (err * 100)/count;
                point = ((err*10000)/count)%100;
            }
            if (valid) {
                snprintf_res(cmd_data,"count = %d\n", count);
                snprintf_res(cmd_data,"err = %d\n", err);
                snprintf_res(cmd_data,"err_rate = %01d.%02d%\n", integer, point);
                return; 
            }
        }            
        snprintf_res(cmd_data,"\n\t./cli rf count 0|1\n");
        return;
    
    } else {
        snprintf_res(cmd_data, 
            "\n\t./cli rf phy_txgen|block|unblock|count|ack|freq|rfreq|sar|rssi|rgreentx|greentx|rate|bbscale|ifs\n");
        return;
    }
out:
    snprintf_res(cmd_data,"\n\t Current RF tool settings: ch %d, pa_band %d\n", ch, pa_band);
    snprintf_res(cmd_data,"\t bbscale:\n");
    
    if (sc->sc_flags &= SC_OP_BLOCK_CNTL) {
        snprintf_res(cmd_data,"\t system control is blocked\n");
    } else {
        snprintf_res(cmd_data,"\t WARNING system control is not blocked\n");
    }     
    snprintf_res(cmd_data,"\t\t HT40 0x%08x, HT20 0x%08x, Legacy 0x%08x, B 0x%02x\n",
        REG32(ADR_WIFI_PHY_COMMON_BB_SCALE_REG_3), REG32(ADR_WIFI_PHY_COMMON_BB_SCALE_REG_2),
        REG32(ADR_WIFI_PHY_COMMON_BB_SCALE_REG_1), GET_RG_BB_SCALE_BARKER_CCK);
        
    switch (pa_band){
    case BAND_2G:
        regval = GET_RG_DPD_BB_SCALE_2500;
        break;
    case BAND_5100:
        regval = GET_RG_DPD_BB_SCALE_5100;
        break;
    case BAND_5500:    
        regval = GET_RG_DPD_BB_SCALE_5500;                   
        break;
    case BAND_5700:    
        regval = GET_RG_DPD_BB_SCALE_5700;                    
        break;
    case BAND_5900:    
        regval = GET_RG_DPD_BB_SCALE_5900;                   
        break;
    default:
        break;
    }    
    snprintf_res(cmd_data,"\t current band dpd bbscale: 0x%x\n", regval);
    
    snprintf_res(cmd_data,"\t cbank:\n");
    snprintf_res(cmd_data,"\t\t CBANKI %d, CBANKO %d\n", GET_RG_XO_CBANKI, GET_RG_XO_CBANKO);        
    
    snprintf_res(cmd_data,"\t tx gen rate: 0x%x\n", sc->rf_rc);
    snprintf_res(cmd_data,"\t\t phy mode:");
    switch ((sc->rf_rc & SSV6006RC_PHY_MODE_MSK) >> SSV6006RC_PHY_MODE_SFT){
    
        case 0:
            snprintf_res(cmd_data,"\t %s\n", "B");
            break;
        case 2:
            snprintf_res(cmd_data,"\t %s\n", "A/G");
            break;
        case 3:
            snprintf_res(cmd_data,"\t %s\n", "N");
            break;            
        default:
            snprintf_res(cmd_data,"\t %s\n", "Invalid");
            break;
    }
    snprintf_res(cmd_data,"\t\t HT40/HT20:\t %s\n",
        ((sc->rf_rc & SSV6006RC_20_40_MSK) >> SSV6006RC_20_40_SFT) ? "HT40":"HT20") ;
    snprintf_res(cmd_data,"\t\t SHORT/LONG:\t %s\n",
        ((sc->rf_rc & SSV6006RC_LONG_SHORT_MSK) >> SSV6006RC_LONG_SHORT_SFT) ?"short":"long") ;
    snprintf_res(cmd_data,"\t\t rate index:\t %d\n", (sc->rf_rc & SSV6006RC_RATE_MSK) >> SSV6006RC_RATE_SFT) ;            
}

void ssv6006_turismoC_update_rf_pwr(struct ssv_softc *sc){
    int pa_band;
    int txpwr;
    struct ssv_hw *sh = sc->sh;

    if (!(sc->sh->cfg.greentx & GT_ENABLE))
        return;
    pa_band = _get_pa_band(sc->hw_chan);
    txpwr = sh->default_txgain[pa_band];
                                                 
    if (sc->dpd.pwr_mode  != NORMAL_PWR){ 
        if (txpwr < sc->green_pwr )
            txpwr = sc->green_pwr;
    }
    
    if (txpwr != sc->current_pwr[pa_band])
    	sc->current_pwr[pa_band] = txpwr;
    else
    	return;
	
    switch (pa_band){
        case 0:
            SET_RG_TX_GAIN(txpwr);
            break;
        case 1:
            SET_RG_5G_TX_GAIN_F0(txpwr);
            break;
        case 2:
            SET_RG_5G_TX_GAIN_F1(txpwr);
            break;
        case 3:
            SET_RG_5G_TX_GAIN_F2(txpwr);
            break;
        case 4:
            SET_RG_5G_TX_GAIN_F3(txpwr);
            break;
        default:
            break;
    }
    //printk("update tx power %d  to pa_band %d\n", txpwr, pa_band); 
}

void ssv_attach_ssv6006_turismoC_BBRF(struct ssv_hal_ops *hal_ops)
{
    hal_ops->load_phy_table = ssv6006_turismoC_load_phy_table;
    hal_ops->get_phy_table_size = ssv6006_turismoC_get_phy_table_size;
    hal_ops->get_rf_table_size = ssv6006_turismoC_get_rf_table_size;
    hal_ops->load_rf_table = ssv6006_turismoC_load_rf_table;
    hal_ops->init_pll = ssv6006_turismoC_init_PLL;
    hal_ops->set_channel = ssv6006_turismoC_set_channel;
    hal_ops->set_pll_phy_rf = ssv6006_turismoC_set_pll_phy_rf;
    hal_ops->set_rf_enable = ssv6006_turismoC_set_rf_enable;
    
    hal_ops->dump_phy_reg = ssv6006_turismoC_dump_phy_reg;
    hal_ops->dump_rf_reg = ssv6006_turismoC_dump_rf_reg;
    hal_ops->support_iqk_cmd = ssv6006_turismoC_support_iqk_cmd;
    hal_ops->cmd_cali = ssv6006_cmd_turismoC_cali;
    hal_ops->cmd_loopback_setup_env = ssv6006c_cmd_loopback_setup_env;
    hal_ops->cmd_loopback = ssv6006_cmd_turismoC_loopback;
    hal_ops->cmd_txgen = ssv6006_cmd_turismoC_txgen;
    hal_ops->cmd_rf = ssv6006_cmd_turismoC_rf;
    
    hal_ops->update_rf_pwr = ssv6006_turismoC_update_rf_pwr;
}
#endif
