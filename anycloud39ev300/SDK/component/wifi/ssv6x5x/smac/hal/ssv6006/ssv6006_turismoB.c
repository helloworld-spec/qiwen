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
#include "ssv6006B_reg.h"
#include "ssv6006B_aux.h"
#include <smac/dev.h>
#include <hal.h>
#include <ssvdevice/ssv_cmd.h>
#include "../ssv6006c/ssv6006_priv.h"
#include "../ssv6006c/ssv6006_priv_normal.h"
#include "turismoB_rf_reg.c"
#include "turismoB_wifi_phy_reg.c"
#include "../ssv6006c/turismo_common.h"
#include "turismo_commonB.c"
#include <ssv6xxx_common.h>
#include <linux_80211.h>

static bool ssv6006_turismoB_set_rf_enable(struct ssv_hw *sh, bool val);

static const size_t ssv6006_turismoB_phy_tbl_size = sizeof(ssv6006_turismoB_phy_setting);

static void ssv6006_turismoB_load_phy_table(ssv_cabrio_reg **phy_table)
{
    *phy_table = ssv6006_turismoB_phy_setting;
}

static u32 ssv6006_turismoB_get_phy_table_size(struct ssv_hw *sh)
{
    return(u32) ssv6006_turismoB_phy_tbl_size;
}

static const size_t ssv6006_turismoB_rf_tbl_size = sizeof(ssv6006_turismoB_rf_setting);

static void ssv6006_turismoB_load_rf_table(ssv_cabrio_reg **rf_table)
{
    *rf_table = ssv6006_turismoB_rf_setting;
}

static u32 ssv6006_turismoB_get_rf_table_size(struct ssv_hw *sh)
{
    return (u32) ssv6006_turismoB_rf_tbl_size;
}

//#define USE_COMMON_MACRO
static void ssv6006_turismoB_init_PLL(struct ssv_hw *sh)
{
#ifdef USE_COMMON_MACRO
    TU_INIT_TURISMOB_PLL;
#else
    u32 regval , count = 0;                                                            
                                                                                       
    /*for turismo, it just needs to set register once , pll is initialized by hw auto*/
    REG32_W(ADR_PMU_REG_2, 0xa51a8820);
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
    REG32_W(ADR_CLOCK_SELECTION, 0x00000004);
    MSLEEP(1);  /* wait for clock settled*/
#endif
}

#ifndef USE_COMMON_MACRO
static void _set_turismoB_BW(struct ssv_hw *sh, enum nl80211_channel_type channel_type)
{
#ifdef USE_COMMON_MACRO
    TU_SET_TURISMOB_BW(channel_type);
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
    TURISMOB_SET_2G_CHANNEL(ch);
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
    
    /* enable 11B*/
    SET_RG_PHY11B_MD_EN(1);
#endif
}

static void _set_5g_channel(struct ssv_hw *sh, int ch)
{
#ifdef USE_COMMON_MACRO
    TURISMOB_SET_5G_CHANNEL(ch);
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
    
    /*disable 11b*/
    SET_RG_PHY11B_MD_EN(0);
#endif 
}

static void _turismoB_pre_cal(struct ssv_hw *sh)
{
    /* set RG_MODE_MANUAL ON*/
    SET_RG_MODE_MANUAL(1);

    /* set RG_MODE to Calibration mode */                                                 
    SET_RG_MODE(1);
}

static void _turismoB_post_cal(struct ssv_hw *sh)
{
    /* set RG_MODE to IDLE mode*/
    SET_RG_MODE(0);
    
    /*set RG_MODE_MANUAL off*/
    SET_RG_MODE_MANUAL(0);
}

static void _turismoB_2p4g_rxdc_cal(struct ssv_hw *sh)
{
    int i ;                                                                               
    u32 wifi_dc_addr;                                                                     

    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);

    /*set RG_CAL_INDEX to WiFi DC 2.4G*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_RXDC);

    MSLEEP(10); /* sleep 10 ms, for DC calibration done*/

    PRINT("--------------------------------------------\r\n");
    PRINT("--------- 2.4 G Rx DC Calibration result----------------");
    for (i = 0; i < 21; i++) {
       if (i %4 == 0)
          PRINT("\r\n");
       wifi_dc_addr = (ADR_WF_DCOC_IDAC_REGISTER1)+ (i << 2);
       PRINT("addr %x : val %x, ", wifi_dc_addr, REG32_R(wifi_dc_addr));
    }
    PRINT("\r\n");
    /* set RG_CAL_INDEX to NONE*/                                                         
    SET_RG_CAL_INDEX(0);

    /* set RG_MODE to IDLE mode*/
   // SET_RG_MODE(0);
}

static void _turismoB_5g_rxdc_cal(struct ssv_hw *sh)
{
    int i ;                                                                               
    u32 wifi_dc_addr;                                                                     

    /* set RG_MODE to Calibration mode */                                                 
//    SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to WiFi DC 5G*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_RXDC);
    
    MSLEEP(10); /* sleep 10 ms, for DC calibration done*/
    
    PRINT("--------------------------------------------\r\n");
    PRINT("--------- 5 G Rx DC Calibration result----------------");
    for (i = 0; i < 21; i++) {
       if (i %4 == 0)
          PRINT("\r\n");
       wifi_dc_addr = (ADR_5G_DCOC_IDAC_REGISTER1)+ (i << 2);
       PRINT("addr %x : val %x, ", wifi_dc_addr, REG32_R(wifi_dc_addr));
    }
    PRINT("\r\n");
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);
    
    /* set RG_MODE to IDLE mode*/                                          
    //SET_RG_MODE(0);                                                        


}

static void _turismoB_bw20_rxrc_cal(struct ssv_hw *sh)
{
    int count = 0;

    PRINT("--------------------------------------------\r\n");
    PRINT("Before WiFi BW20 RG_WF_RX_ABBCTUNE: %d\n", GET_RG_WF_RX_ABBCTUNE);

    SET_RG_RX_RCCAL_DELAY(2);
    
    SET_REG(ADR_RF_D_CAL_TOP_6,
        (0xe5 << RG_RX_RCCAL_TARG_SFT) | (0 << RG_RCCAL_POLAR_INV_SFT), 0,
        (RG_RX_RCCAL_TARG_I_MSK & RG_RCCAL_POLAR_INV_I_MSK));
    
    SET_RG_PGAG_RCCAL(3);
    
    SET_RG_TONE_SCALE(0x80);
    
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to BW20_RXRC*/
    SET_RG_CAL_INDEX(CAL_IDX_BW20_RXRC);
    
    while (GET_RO_RCCAL_DONE == 0){
        count ++;
        if (count >100) {
            break;
        }
        MSLEEP(1);
    }
    
    MSLEEP(10);
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("WiFi BW20 RG_WF_RX_ABBCTUNE CAL RESULT: %d\n", GET_RG_WF_RX_ABBCTUNE);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);
   
}

static void _turismoB_bw40_rxrc_cal(struct ssv_hw *sh)
{
    int count = 0;
 
    
    PRINT("--------------------------------------------\r\n");
    PRINT("Before WiFi BW40 RG_WF_RX_N_ABBCTUNE: %d\n", GET_RG_WF_N_RX_ABBCTUNE);

    SET_RG_RX_N_RCCAL_DELAY(2);
    
    SET_RG_PHASE_2P5M(0x800);

    SET_REG(ADR_RF_D_CAL_TOP_6,
        (0x197 << RG_RX_RCCAL_40M_TARG_SFT) | (0 << RG_RCCAL_POLAR_INV_SFT), 0,
        (RG_RX_RCCAL_40M_TARG_I_MSK & RG_RCCAL_POLAR_INV_I_MSK));

    SET_RG_ALPHA_SEL(2);

    
    SET_RG_PHASE_35M(0x5800);

    
    SET_RG_PGAG_RCCAL(3);

    SET_RG_TONE_SCALE(0x80);

    
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);

   
    /*set RG_CAL_INDEX to BW40_RXRC*/
    SET_RG_CAL_INDEX(CAL_IDX_BW40_RXRC);

    while (GET_RO_RCCAL_DONE == 0){
        count ++;
        if (count >100) {
            break;
        }
        MSLEEP(1);
    }
    
    MSLEEP(10);
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("WiFi BW40 RG_WF_N_RX_ABBCTUNE CAL RESULT: %d\n", GET_RG_WF_N_RX_ABBCTUNE);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);
    
}

static void _turismoB_txdc_cal(struct ssv_hw *sh)
{
    int count = 0;
 
    
    PRINT("--------------------------------------------\r\n");
    PRINT("Before txdc calibration WiFi 2P4G Tx DAC IOFFSET: %d, QOFFSET %d\n", 
         GET_RG_WF_TX_DAC_IOFFSET, GET_RG_WF_TX_DAC_QOFFSET);

    SET_RG_TONE_SCALE(0x80);
    
    SET_REG(ADR_CALIBRATION_GAIN_REGISTER0,
        (0x6 << RG_TX_GAIN_TXCAL_SFT) | (0x0 << RG_PGAG_TXCAL_SFT), 0,
        (RG_TX_GAIN_TXCAL_I_MSK & RG_PGAG_TXCAL_I_MSK));

    SET_RG_PRE_DC_AUTO(1);
    
    SET_RG_TX_IQCAL_TIME(1);
 
    SET_RG_PHASE_1M(0x7FF);
    SET_RG_PHASE_RXIQ_1M(0x7FF);
    
    SET_RG_ALPHA_SEL(2);
       
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to WIFI2P4G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_TXLO);
    
    while (GET_RO_TXDC_DONE == 0){
        count ++;
        if (count >100) {
            break;
        }
        MSLEEP(1);
    }
    
    MSLEEP(10);
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After txdc calibration WiFi 2P4G Tx DAC IOFFSET: %d, QOFFSET %d\n", 
         GET_RG_WF_TX_DAC_IOFFSET, GET_RG_WF_TX_DAC_QOFFSET);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);
    
}


static void _turismoB_txiq_cal(struct ssv_hw *sh)
{
    int count = 0;
 
//    SET_RG_RF_5G_BAND(0);
    PRINT("--------------------------------------------\r\n");
    PRINT("before tx iq calibration, tx alpha: %d, tx theta %d\n", 
         GET_RO_TX_IQ_ALPHA, GET_RO_TX_IQ_THETA);

    SET_RG_TONE_SCALE(0x80);
    
    SET_REG(ADR_CALIBRATION_GAIN_REGISTER0,
        (0x6 << RG_TX_GAIN_TXCAL_SFT) | (0x0 << RG_PGAG_TXCAL_SFT), 0,
        (RG_TX_GAIN_TXCAL_I_MSK & RG_PGAG_TXCAL_I_MSK));

    SET_RG_PRE_DC_AUTO(1);
    
    SET_RG_TX_IQCAL_TIME(1);
 
    SET_RG_PHASE_1M(0x7FF);
    SET_RG_PHASE_RXIQ_1M(0x7FF);
    
    SET_RG_ALPHA_SEL(2);
       
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
    
    /*set RG_CAL_INDEX to WIFI2P4G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_TXIQ);
    
    while (GET_RO_TXIQ_DONE == 0){
        count ++;
        if (count >100) {
            break;
        }
        MSLEEP(1);
    }
    
    MSLEEP(10);
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After tx iq calibration, tx alpha: %d, tx theta %d\n", 
         GET_RO_TX_IQ_ALPHA, GET_RO_TX_IQ_THETA);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);
    
}

static void _turismoB_rxiq_cal(struct ssv_hw *sh)
{
    int count = 0;
 
//    SET_RG_RF_5G_BAND(0);
    PRINT("--------------------------------------------\r\n");
    PRINT("Before rx iq calibration, rx alpha: %d, rx theta %d\n", 
         GET_RO_RX_IQ_ALPHA, GET_RO_RX_IQ_THETA);

    SET_RG_RFG_RXIQCAL(0x0);
    
    SET_RG_PGAG_RXIQCAL(0x3);
    
    SET_RG_TX_GAIN_RXIQCAL(0x6);
 
    SET_RG_TONE_SCALE(0x80);
    
    SET_RG_PRE_DC_AUTO(1);
    
    SET_RG_TX_IQCAL_TIME(1);

    SET_RG_PHASE_1M(0x7FF);
    SET_RG_PHASE_RXIQ_1M(0x7FF);

    SET_RG_ALPHA_SEL(2);
    
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);

   
    /*set RG_CAL_INDEX to WIFI2P4G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI2P4G_RXIQ);

    while (GET_RO_RXIQ_DONE == 0){
        count ++;
        if (count >100) {
            break;
        }
        MSLEEP(1);
    }
    
    MSLEEP(10);
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After rx iq calibration, rx alpha: %d, rx theta %d\n", 
         GET_RO_RX_IQ_ALPHA, GET_RO_RX_IQ_THETA);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);

}

static void _turismoB_5g_txdc_cal(struct ssv_hw *sh)
{
    int count = 0;
 

    PRINT("--------------------------------------------\r\n");
    PRINT("Before 5G txdc calibration WiFi 5G Tx DAC IOFFSET: %d, QOFFSET %d\n", 
         GET_RG_5G_TX_DAC_IOFFSET, GET_RG_5G_TX_DAC_QOFFSET);

    SET_RG_TONE_SCALE(0x80);
    
    SET_REG(ADR_5G_CALIBRATION_TIMER_GAIN_REGISTER,
        (0xe << RG_5G_TX_GAIN_TXCAL_SFT) | (0x3 << RG_5G_PGAG_TXCAL_SFT), 0,
        (RG_5G_TX_GAIN_TXCAL_I_MSK & RG_5G_PGAG_TXCAL_I_MSK));
   
    SET_RG_PRE_DC_AUTO(1);

    SET_RG_TX_IQCAL_TIME(1);
 
    SET_RG_PHASE_1M(0x7FF);
    SET_RG_PHASE_RXIQ_1M(0x7FF);
    
    SET_RG_ALPHA_SEL(2);
       
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to WIFI5G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_TXLO);
    
    while (GET_RO_5G_TXDC_DONE == 0){
        count ++;
        if (count >100) {
            break;
        }
        MSLEEP(1);
    }
    
    MSLEEP(10);
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After 5G txdc calibration WiFi 5G Tx DAC IOFFSET: %d, QOFFSET %d\n", 
         GET_RG_5G_TX_DAC_IOFFSET, GET_RG_5G_TX_DAC_QOFFSET);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);
    
}


static void _turismoB_5g_txiq_cal(struct ssv_hw *sh)
{
    int count = 0;
 
//    SET_RG_RF_5G_BAND(1);
    PRINT("--------------------------------------------\r\n");
    PRINT("before 5G tx iq calibration, tx alpha: %d, tx theta %d\n", 
         GET_RO_TX_IQ_ALPHA, GET_RO_TX_IQ_THETA);

    SET_RG_TONE_SCALE(0x80);
    
    SET_REG(ADR_5G_CALIBRATION_TIMER_GAIN_REGISTER,
        (0xe << RG_5G_TX_GAIN_TXCAL_SFT) | (0x3 << RG_5G_PGAG_TXCAL_SFT), 0,
        (RG_5G_TX_GAIN_TXCAL_I_MSK & RG_5G_PGAG_TXCAL_I_MSK));
   
    SET_RG_PRE_DC_AUTO(1);
    
    SET_RG_TX_IQCAL_TIME(1);
 
    SET_RG_PHASE_1M(0x7FF);
    SET_RG_PHASE_RXIQ_1M(0x7FF);
    
    SET_RG_ALPHA_SEL(2);
       
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to WIFI5G_TXIQ*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_TXIQ);

    while (GET_RO_5G_TXIQ_DONE == 0){
        count ++;
        if (count >100) {
            break;
        }
        MSLEEP(1);
    }
    
    MSLEEP(10);
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After 5G tx iq calibration, tx alpha: %d, tx theta %d\n", 
         GET_RO_TX_IQ_ALPHA, GET_RO_TX_IQ_THETA);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);
    
}

static void _turismoB_5g_rxiq_cal(struct ssv_hw *sh)
{
    int count = 0;
 
//    SET_RG_RF_5G_BAND(1);   
    PRINT("--------------------------------------------\r\n");
    PRINT("Before 5G rx iq calibration, rx alpha: %d, rx theta %d\n", 
         GET_RO_RX_IQ_ALPHA, GET_RO_RX_IQ_THETA);

    SET_RG_5G_RFG_RXIQCAL(0x0);
    
    SET_RG_5G_PGAG_RXIQCAL(0x3);
    
    SET_RG_5G_TX_GAIN_RXIQCAL(0xe);
 
    SET_RG_TONE_SCALE(0x80);
    
    SET_RG_PRE_DC_AUTO(1);

    SET_RG_TX_IQCAL_TIME(1);

    SET_RG_PHASE_1M(0x7FF);
    SET_RG_PHASE_RXIQ_1M(0x7FF);


    SET_RG_ALPHA_SEL(2);
      
    /* set RG_MODE to Calibration mode */                                                 
    //SET_RG_MODE(1);
   
    /*set RG_CAL_INDEX to WIFI2P4G_TXLO*/
    SET_RG_CAL_INDEX(CAL_IDX_WIFI5G_RXIQ);
    
    while (GET_RO_5G_RXIQ_DONE == 0){
        count ++;
        if (count >100) {
            break;
        }
        MSLEEP(1);
    }
    
    MSLEEP(10);
    PRINT("--------------------------------------------%d\r\n", count);
    PRINT("After 5G rx iq calibration, rx alpha: %d, rx theta %d\n", 
         GET_RO_RX_IQ_ALPHA, GET_RO_RX_IQ_THETA);
    
    /* set RG_CAL_INDEX to NONE*/
    SET_RG_CAL_INDEX(0);

}

#define MULTIPLIER  1024
static void _start_padpd(struct ssv_hw *sh, struct ssv6006dpd *val, int pa_band)
{
    int i, rg_tx_scale, regval;
    int am, pm;
    int slope_ini = 0, phase_ini = 0;
    int padpd_am = 0, padpd_pm = 0;
    u32 addr, mask;
    int band[4]={6, 36, 100, 140};
    
    PRINT("start PA DPD on band %d\n", pa_band);
    SET_RG_MODE_MANUAL(1);
    SET_RG_MODE(MODE_STANDBY);
    SET_RG_SX5GB_RFCH_MAP_EN(1);
    SET_RG_SX5GB_CHANNEL(band[pa_band]);
    if (pa_band == 0) {
        SET_RG_TX_GAIN_DPDCAL(7);
        SET_RG_PGAG_DPDCAL(3);
        SET_RG_RFG_DPDCAL(0);
        //SET_RG_TX_PAFB_EN(0);             ///??? what to set in 2.4G
    } else {
        SET_RG_5G_TX_GAIN_DPDCAL(7);
        SET_RG_5G_PGAG_DPDCAL(1);
        SET_RG_5G_RFG_DPDCAL(0);
        SET_RG_5G_TX_PAFB_EN(0);
    }
    SET_RG_BB_SIG_EN(1);
    SET_RG_DC_RM_BYP(1);
    SET_RG_TX_IQ_SRC(2);
    SET_RG_TX_BB_SCALE_MANUAL(1);
    SET_RG_TX_SCALE(0x80);
    SET_RG_TONE_1_RATE(0x7ff);
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
        addr = padpd_am_table[ (i >> 1)].addr;
        if (i & 0x1) {
            mask = padpd_am_table[ (i >> 1)].mask1;
        } else {
            mask = padpd_am_table[ (i >> 1)].mask0;
        }
        regval = REG32_R(addr);
        REG32_W(addr, (regval & mask) | ((padpd_am) << ((i & 0x1)*16)) );
        if (i & 0x1){
            val->am[ (i >> 1)] = (regval & mask) | ((padpd_am) << ((i & 0x1)*16));
        }
        
        
        addr = padpd_pm_table[ (i >> 1)].addr;
        if (i & 0x1) {
            mask = padpd_pm_table[ (i >> 1)].mask1;
        } else {
            mask = padpd_pm_table[ (i >> 1)].mask0;
        }
        regval = REG32_R(addr);
        REG32_W(addr, (regval & mask) | ((padpd_pm) << ((i & 0x1)*16)) );
        
        if (i & 0x1){
            val->pm[ (i >> 1)] = (regval & mask) | ((padpd_pm) << ((i & 0x1)*16));
        }
    }
    
    SET_RG_CAL_INDEX(CAL_IDX_NONE);
    SET_RG_MODE(MODE_STANDBY);
    SET_RG_MODE_MANUAL(0);
    
    SET_RG_BB_SIG_EN(0);
    SET_RG_BB_SIG_EN(0);
    SET_RG_DC_RM_BYP(0);
    SET_RG_TX_IQ_SRC(0);
    SET_RG_TX_BB_SCALE_MANUAL(0);

    SET_RG_TONE_SEL(0);
    
    SET_RG_RX_PADPD_EN(0);
     
    SET_RG_DPD_AM_EN(1);
    
    PRINT("PA DPD done\n");
}


static void _check_padpd(struct ssv_hw *sh, struct ssv6006_padpd *dpd, int ch)
{
    int pa_band = 0;
    
    if (ch <=14){
        pa_band =0;
    } else if ((ch >=36) && (ch <=64)) {
        pa_band = 1;
    } else if ((ch >=100) && (ch<=136)){
        pa_band = 2;
    } else if (ch >= 140) {
        pa_band = 3;
    }
    if (pa_band == 0){
        if (dpd->pwr_mode != NORMAL_PWR ){
            REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_1, DEFAULT_PWR_SETTING);
            REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_2, DEFAULT_PWR_SETTING);
            REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_3, DEFAULT_PWR_SETTING);
            dpd->pwr_mode = NORMAL_PWR;
        }
    } else{
        if (dpd->pwr_mode != ENHANCE_PWR ){
            REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_1, PADPD_5G_PWR_SETTING);
            REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_2, PADPD_5G_PWR_SETTING);
            REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_3, PADPD_5G_PWR_SETTING);
            dpd->pwr_mode = ENHANCE_PWR;
        }
    }  
    if ( dpd->dpd_done[pa_band] == false) {
 
        SET_RG_DPD_AM_EN(0);
        printk("Start PADPD on band %d ,ADR_WIFI_PHY_COMMON_ENABLE_REG 0x%x \n", pa_band, REG32_R(ADR_WIFI_PHY_COMMON_ENABLE_REG));
        _start_padpd(sh, &dpd->val[pa_band], pa_band);
        dpd->dpd_done[pa_band] = true; 
    } else {
       // if (!sh->ssv6006mp){ // restore padpd for shuttle while done before
            int i;
            u32 addr;
            
            for ( i = 0; i < (MAX_PADPD_TONE /2); i++){
                addr = padpd_am_table[i].addr;
                REG32_W(addr, dpd->val[pa_band].am[i]);
                addr = padpd_pm_table[i].addr;
                REG32_W(addr, dpd->val[pa_band].pm[i]);
            }           
       // } 
    }

}
#endif

static int ssv6006_turismoB_set_channel(struct ssv_softc *sc, struct ieee80211_channel  *chan, 
    enum nl80211_channel_type channel_type)
{
    int ch = chan->hw_value;
    struct ssv_hw *sh = sc->sh;

#ifdef USE_COMMON_MACRO
    TU_CHANGE_TURISMOB_CHANNEL(ch, channel_type);
#else
    const char *chan_type[]={"NL80211_CHAN_NO_HT",
	    "NL80211_CHAN_HT20",
	    "NL80211_CHAN_HT40MINUS",
	    "NL80211_CHAN_HT40PLUS"};

    PRINT("%s: ch %d, type %s\r\n", __func__, ch, chan_type[channel_type]);
    
    if (REG32_R(ADR_WIFI_PHY_COMMON_ENABLE_REG) != 0) {
    /* check padpd only when phy enable */    
        _check_padpd(sh, &sc->dpd, ch);
    }
    _set_turismoB_BW(sh, channel_type);
    
    if ((ch <=14) && (ch >=1)){
         
        HAL_SET_SIFS(sh, INDEX_80211_BAND_2GHZ); 
        _set_2g_channel(sh, ch);

    } else if (ch >=34){
    	
        HAL_SET_SIFS(sh, INDEX_80211_BAND_5GHZ); 
        _set_5g_channel(sh, ch);

    } else {
        
        PRINT("invalid channel %d\n", ch);                                                 
    }
#endif
    return 0;
}

static void ssv6006_turismoB_init_cali (struct ssv_hw *sh)
{
#ifdef USE_COMMON_MACRO
    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ){
        TU_INIT_TURISMOB_CALI;
    } else {
        TU_INIT_TURISMOB_2G_CALI;
    }
#else                                                                    

    _turismoB_pre_cal(sh);

    _turismoB_2p4g_rxdc_cal(sh);

    _turismoB_bw20_rxrc_cal(sh);
    
    _turismoB_bw40_rxrc_cal(sh);
    
    _turismoB_txdc_cal(sh);

    _turismoB_txiq_cal(sh);    

    _turismoB_rxiq_cal(sh);
    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ) {
        _turismoB_5g_rxdc_cal(sh);
        
        _turismoB_5g_txdc_cal(sh);

        _turismoB_5g_txiq_cal(sh);
    
        _turismoB_5g_rxiq_cal(sh);
        
    }
    _turismoB_post_cal(sh);
#endif

}

#if 0
void ssv6006_turismoB_write_rf_table(struct ssv_hw *sh )
{                                                                
                                                 
    //u32 i = 0;
                                                    
    //for( i = 0; i < sizeof(ssv6006_turismoB_rf_setting)/sizeof(ssv_cabrio_reg); i++) {
    //   SMAC_REG_WRITE(sh, ssv6006_turismoB_rf_setting[i].address, ssv6006_turismoB_rf_setting[i].data );
    //   udelay(50); // should delay a while when set external RF                              
    //}                                                            
                                                         
}
#endif

#ifndef USE_COMMON_MACRO
static void _update_rf_patch(struct ssv_hw *sh, int xtal)
{
    // default settings for xtal is 26m, change setting for other case
    if (xtal != XTAL26M){
        SET_RG_DP_XTAL_FREQ(xtal);
        SET_RG_SX_XTAL_FREQ(xtal);
    }
}
#endif

static int ssv6006_turismoB_set_pll_phy_rf(struct ssv_hw *sh
    , ssv_cabrio_reg *rf_tbl, ssv_cabrio_reg *phy_tbl)
{	
    int  ret = 0;
    int  xtal;

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


#ifdef USE_COMMON_MACRO

    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ) {
       INIT_TURISMOB_SYS(xtal, AG_BAND_BOTH);
    } else {
       INIT_TURISMOB_SYS(xtal,G_BAND_ONLY);
    }
#else
    ret = SSV6XXX_SET_HW_TABLE(sh, ssv6006_turismoB_rf_setting);
    _update_rf_patch(sh, xtal);    

    ssv6006_turismoB_init_PLL(sh);
    REG32_W(ADR_WIFI_PHY_COMMON_ENABLE_REG, 0);
    ret = SSV6XXX_SET_HW_TABLE(sh, ssv6006_turismoB_phy_setting);
    ssv6006_turismoB_init_cali(sh);
#endif
    return ret;
}

static bool ssv6006_turismoB_set_rf_enable(struct ssv_hw *sh, bool val)
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

static bool ssv6006_turismoB_dump_phy_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6006_turismoB_phy_setting;

    snprintf_res(cmd_data, ">> PHY Register Table:\n");

    for(s = 0; s < ssv6006_turismoB_phy_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);      
    }
    snprintf_res(cmd_data, ">>PHY Table version: %s\n", SSV6006_TURISMOB_PHY_TABLE_VER);
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6006_turismoB_dump_rf_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6006_turismoB_rf_setting;

    snprintf_res(cmd_data, ">> RF Register Table:\n");

    for(s = 0; s < ssv6006_turismoB_rf_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);      
    }
    snprintf_res(cmd_data, ">>RF Table version: %s\n", SSV6006_TURISMOB_RF_TABLE_VER);
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6006_turismoB_support_iqk_cmd(struct ssv_hw *sh)
{
    return false;
}

static void ssv6006_cmd_turismoB_cali(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    struct ssv_softc *sc = sh->sc;
    
    if(!strcmp(argv[1], "do")){
        
        ssv6006_turismoB_init_cali(sh);
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
                        snprintf_res(cmd_data,"\t 5G 36 <= channel <= 100");
                        break;
                    case 2:
                        snprintf_res(cmd_data,"\t 5G 100<= channel <= 136");
                        break;
                    case 3:
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
                        snprintf_res(cmd_data, "0x%04x 0x%04x ", sc->dpd.val[pa_band].am[i] & 0xffff,
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
            if ( sc->dpd.current_band == 0){
                SET_RG_EN_TX_DPD(1);
            } else {
                SET_RG_5G_EN_TX_DPD(1);
            }  
            SET_RG_DPD_AM_EN(1);
            
            if (sc->dpd.current_band != 0){
                if (sc->dpd.pwr_mode != ENHANCE_PWR ){
                    REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_1, PADPD_5G_PWR_SETTING);
                    REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_2, PADPD_5G_PWR_SETTING);
                    REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_3, PADPD_5G_PWR_SETTING);
                    sc->dpd.pwr_mode = ENHANCE_PWR;
                }
            }
            snprintf_res(cmd_data,"enable DPD\r\n");
        } else if(!strcmp(argv[2], "disable")){
            if ( sc->dpd.current_band == 0){
                SET_RG_EN_TX_DPD(0);
            } else {
                SET_RG_5G_EN_TX_DPD(0);
            }  
            SET_RG_DPD_AM_EN(0);
            
            if (sc->dpd.pwr_mode != NORMAL_PWR ){
                REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_1, DEFAULT_PWR_SETTING);
                REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_2, DEFAULT_PWR_SETTING);
                REG32_W(ADR_WIFI_PHY_COMMON_RF_PWR_REG_3, DEFAULT_PWR_SETTING);
                sc->dpd.pwr_mode = NORMAL_PWR;
            }
            snprintf_res(cmd_data,"disable DPD\r\n");
        } else {
            snprintf_res(cmd_data,"\n cali [do|show|dpd(show |enable|disable)] \n");    
        }
         
    }else {
        snprintf_res(cmd_data,"\n cali [do|show|dpd(show |enable|disable)] \n");
    }     
    
}

void ssv_attach_ssv6006_turismoB_BBRF(struct ssv_hal_ops *hal_ops)
{
    hal_ops->load_phy_table = ssv6006_turismoB_load_phy_table;
    hal_ops->get_phy_table_size = ssv6006_turismoB_get_phy_table_size;
    hal_ops->get_rf_table_size = ssv6006_turismoB_get_rf_table_size;
    hal_ops->load_rf_table = ssv6006_turismoB_load_rf_table;
    hal_ops->init_pll = ssv6006_turismoB_init_PLL;
    hal_ops->set_channel = ssv6006_turismoB_set_channel;
    hal_ops->set_pll_phy_rf = ssv6006_turismoB_set_pll_phy_rf;
    hal_ops->set_rf_enable = ssv6006_turismoB_set_rf_enable;
    
    hal_ops->dump_phy_reg = ssv6006_turismoB_dump_phy_reg;
    hal_ops->dump_rf_reg = ssv6006_turismoB_dump_rf_reg;
    hal_ops->support_iqk_cmd = ssv6006_turismoB_support_iqk_cmd;
    hal_ops->cmd_cali = ssv6006_cmd_turismoB_cali;
}
#endif