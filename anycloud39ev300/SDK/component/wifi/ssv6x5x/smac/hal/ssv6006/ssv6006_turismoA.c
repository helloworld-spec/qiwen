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
#if ((defined SSV_SUPPORT_HAL) && (defined SSV_SUPPORT_SSV6006) && (defined SSV_SUPPORT_TURISMOA))
#include <linux/nl80211.h>
#include <ssv6200.h>
#include "ssv6006B_reg.h"
#include "ssv6006B_aux.h"
#include <smac/dev.h>
#include <hal.h>
#include <ssvdevice/ssv_cmd.h>
#include "../ssv6006c/ssv6006_priv.h"
#include "../ssv6006c/ssv6006_priv_normal.h"
#include "turismoA_wifi_phy_reg.c"
#include "turismoA_rf_reg.c"
#include "../ssv6006c/turismo_common.h"
#include <linux_80211.h>

static bool ssv6006_turismoA_set_rf_enable(struct ssv_hw *sh, bool val);
  

static const size_t ssv6006_turismoA_phy_tbl_size = sizeof(ssv6006_turismoA_phy_setting);

static void ssv6006_turismoA_load_phy_table(ssv_cabrio_reg **phy_table)
{
    *phy_table = ssv6006_turismoA_phy_setting;
}

static u32 ssv6006_turismoA_get_phy_table_size(struct ssv_hw *sh)
{
    return(u32) ssv6006_turismoA_phy_tbl_size;
}

static const size_t ssv6006_turismoA_rf_tbl_size = sizeof(ssv6006_turismoA_rf_setting);

static void ssv6006_turismoA_load_rf_table(ssv_cabrio_reg **rf_table)
{
    *rf_table = ssv6006_turismoA_rf_setting;
}

static u32 ssv6006_turismoA_get_rf_table_size(struct ssv_hw *sh)
{
    return (u32) ssv6006_turismoA_rf_tbl_size;
}

//#define USE_COMMON_MACRO

static void ssv6006_turismoA_init_PLL(struct ssv_hw *sh)
{
#ifdef USE_COMMON_MACRO
    TU_INIT_PLL;
#else
    u32 regval , count = 0;

    // for turismo, it just needs to set register once , pll is initialized by hw auto.
    SMAC_REG_WRITE(sh, ADR_PMU_REG_2, 0xa51a8800);

    do
    {
        msleep(1);
        SMAC_REG_READ(sh, ADR_PMU_STATE_REG, &regval);
        count ++ ;
        if (regval == 3)
            break;
        if (count > 100){
            printk(" PLL initial fails \n");
            WARN_ON(1);
            break;
        }
    } while (1);

    msleep(1);
    //SMAC_REG_WRITE(sh, 0xccb0e000, 0x80000000);
    SMAC_REG_WRITE(sh, ADR_WIFI_PHY_COMMON_SYS_REG, 0x80000000);        // enable phy clock
    SMAC_REG_WRITE(sh, ADR_CLOCK_SELECTION, 0x00000004);                // do clock switch
    msleep(1);                                                          // wait for clock settled.    
#endif
}

#if 0
static void _turimismoA_set_5G_HT40_channel(struct ssv_hw *sh, int ch)
{
    SMAC_REG_SET_BITS(sh, ADR_WIFI_PHY_COMMON_SYS_REG,
        1 << RG_RF_5G_BAND_SFT, RG_RF_5G_BAND_MSK);  


    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_DIGITAL_ADD_ON_0,
        (0 << RG_TURISMO_TRX_40M_MODE_SFT) | (0 << RG_TURISMO_TRX_LO_UP_CH_SFT),
        (RG_TURISMO_TRX_LO_UP_CH_MSK | RG_TURISMO_TRX_40M_MODE_MSK));
    udelay(50); // should delay a while when set external RF

    // set rf channel manual on        
    SMAC_REG_SET_BITS(sh, ADR_MODE_REGISTER, 1 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_MSK);
    msleep(1); // for hs3w send

    // set rf channel mapping on
    SMAC_REG_SET_BITS(sh, ADR_SX_5GB_REGISTER_INT3BIT___CH_TABLE, 
        1 << RG_SX5GB_RFCH_MAP_EN_SFT, RG_SX5GB_RFCH_MAP_EN_MSK);  // set rf channel mapping on
    msleep(1); // for hs3w send
      
  
    // set channel
    SMAC_REG_SET_BITS(sh, ADR_SX_5GB_REGISTER_INT3BIT___CH_TABLE, 
    ch << RG_SX5GB_CHANNEL_SFT, RG_SX5GB_CHANNEL_MSK);
    msleep(1); // for hs3w send
       
    // set RG_MODE to IDLE mode
    SMAC_REG_SET_BITS(sh, ADR_MODE_REGISTER, 0 << RG_MODE_SFT, RG_MODE_MSK);
    msleep(1); // for hs3w send
    
    // set RG_MODE to WIFI_5G_RX
    SMAC_REG_SET_BITS(sh, ADR_MODE_REGISTER, 7 << RG_MODE_SFT, RG_MODE_SFT);
    msleep(1); // for hs3w send

    //set RG_MODE_MANUAL off
    SMAC_REG_SET_BITS(sh, ADR_MODE_REGISTER, 0 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_MSK);
    msleep(1); // for hs3w send    
}
#endif

#ifndef USE_COMMON_MACRO   
static void _turismoA_set_5G_txpwr(struct ssv_hw *sh, int ch)
{

    /* settings for 5G_TXPGA_CAPSW, 5G_PABIAS_CTRL, 5G_TX_PA1_VCAS
                    5G_TX_PA2_VCAS, 5G_TX_PA3_VCAS                     */
    u8 pwr_paras[3][7] =  {{ 7, 8, 7, 7, 7},   /* freq below 5400*/ 
                           { 4, 8, 4, 4, 7},   /* freq below 5500*/
                           { 3, 8, 3, 3, 3}};  /* freq above 5500*/
    enum    band { FRQ5400LO, FRQ5500LO, FRQ5500HI};
    int     idx;
    
    if (ch <=64) {
        idx = FRQ5400LO;
    } else if (ch < 100) {
        idx = FRQ5500LO;
    } else {
        idx = FRQ5500HI;
    }
    SET_RG_TURISMO_TRX_TX_GAIN_MANUAL(1);
    udelay(50);
    
    SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_5G_TX_FE_REGISTER, 
        ((1 << RG_TURISMO_TRX_5G_TXPGA_CAPSW_MANUAL_SFT) | 
         (pwr_paras[idx][0] << RG_TURISMO_TRX_5G_TXPGA_CAPSW_SFT) |
         (pwr_paras[idx][1] << RG_TURISMO_TRX_5G_PABIAS_CTRL_SFT) |
         (pwr_paras[idx][2] << RG_TURISMO_TRX_5G_TX_PA1_VCAS_SFT) |
         (pwr_paras[idx][3] << RG_TURISMO_TRX_5G_TX_PA2_VCAS_SFT) |
         (pwr_paras[idx][4] << RG_TURISMO_TRX_5G_TX_PA3_VCAS_SFT)), 
        (RG_TURISMO_TRX_5G_TXPGA_CAPSW_MANUAL_MSK | RG_TURISMO_TRX_5G_TXPGA_CAPSW_MSK |
         RG_TURISMO_TRX_5G_PABIAS_CTRL_MSK | RG_TURISMO_TRX_5G_TX_PA1_VCAS_MSK |
         RG_TURISMO_TRX_5G_TX_PA2_VCAS_MSK | RG_TURISMO_TRX_5G_TX_PA3_VCAS_MSK));
    udelay(50);

    SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_5G_TX_REGISTER, 
        ((0x3f <<RG_TURISMO_TRX_5G_TXPGA_MAIN_SFT) | 
         (0 <<  RG_TURISMO_TRX_5G_TXPGA_STEER_SFT) |
         (2 <<  RG_TURISMO_TRX_5G_TXMOD_GMCELL_SFT) |
         (3<<  RG_TURISMO_TRX_5G_TX_GAIN_SFT)), 
        ( RG_TURISMO_TRX_5G_TXPGA_MAIN_MSK |  RG_TURISMO_TRX_5G_TXPGA_STEER_MSK |
          RG_TURISMO_TRX_5G_TXMOD_GMCELL_MSK |  RG_TURISMO_TRX_5G_TX_GAIN_MSK));
    udelay(50);
    
    SET_RG_TURISMO_TRX_5G_TX_GAIN(3);
    udelay(50);
    
    SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_2_4G_TX_REGISTER, 
        (( 0 << RG_TURISMO_TRX_TX_VTOI_CURRENT_SFT | 3 << RG_TURISMO_TRX_TX_VTOI_GM_SFT)), 
        (RG_TURISMO_TRX_TX_VTOI_CURRENT_MSK | RG_TURISMO_TRX_TX_VTOI_GM_MSK));
    udelay(50); /* should delay a while when set external RF */  
    
    SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_5G_TX_DAC_REGISTER, 
        (( 0 <<  RG_TURISMO_TRX_5G_TX_DACLPF_ICOARSE_SFT | 0xc <<  RG_TURISMO_TRX_5G_TX_DAC_QOFFSET_SFT |
          0xc <<  RG_TURISMO_TRX_5G_TX_DAC_IOFFSET_SFT)), 
        ( RG_TURISMO_TRX_5G_TX_DACLPF_ICOARSE_MSK |  RG_TURISMO_TRX_5G_TX_DAC_QOFFSET_MSK |
          RG_TURISMO_TRX_5G_TX_DAC_IOFFSET_MSK));
    udelay(50); /* should delay a while when set external RF */           

}

static void _turismoA_set_5G_channel(struct ssv_hw *sh, int ch)
{
    _turismoA_set_5G_txpwr(sh, ch);
    
    SMAC_REG_SAFE_SET_BITS(sh, ADR_WIFI_PHY_COMMON_SYS_REG,
        1 << RG_RF_5G_BAND_SFT, RG_RF_5G_BAND_MSK);  

    // set rf channel manual on        
    SMAC_REG_SAFE_SET_BITS(sh, ADR_MODE_REGISTER, 1 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_MSK);
    udelay(50); // for hs3w send

    // set rf channel mapping on
    SMAC_REG_SAFE_SET_BITS(sh, ADR_SX_5GB_REGISTER_INT3BIT___CH_TABLE, 
        1 << RG_SX5GB_RFCH_MAP_EN_SFT, RG_SX5GB_RFCH_MAP_EN_MSK);  // set rf channel mapping on
    udelay(50); // for hs3w send
    
    // set channel
    SMAC_REG_SAFE_SET_BITS(sh, ADR_SX_5GB_REGISTER_INT3BIT___CH_TABLE, 
    ch << RG_SX5GB_CHANNEL_SFT, RG_SX5GB_CHANNEL_MSK);
    udelay(50); // for hs3w send
       
    // set RG_MODE to IDLE mode
    SMAC_REG_SAFE_SET_BITS(sh, ADR_MODE_REGISTER, 0 << RG_MODE_SFT, RG_MODE_MSK);
    udelay(50); // for hs3w send
    
    // set RG_MODE to WIFI_5G_RX
    SMAC_REG_SAFE_SET_BITS(sh, ADR_MODE_REGISTER, 7 << RG_MODE_SFT, RG_MODE_SFT);
    udelay(50); // for hs3w send

    //set RG_MODE_MANUAL off
    SMAC_REG_SAFE_SET_BITS(sh, ADR_MODE_REGISTER, 0 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_MSK);
    udelay(50); // for hs3w send
}    

static void _turismoA_set_2G_txpwr(struct ssv_hw *sh)
{
    SET_RG_TURISMO_TRX_TX_GAIN_MANUAL(0);
    udelay(50);

    SET_RG_TURISMO_TRX_TX_GAIN(3);
    udelay(50);
    
    SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_2_4G_TX_REGISTER, 
        (( 0 << RG_TURISMO_TRX_TX_VTOI_CURRENT_SFT | 1 << RG_TURISMO_TRX_TX_VTOI_GM_SFT)), 
        (RG_TURISMO_TRX_TX_VTOI_CURRENT_MSK | RG_TURISMO_TRX_TX_VTOI_GM_MSK) );
    udelay(50); /* should delay a while when set external RF */      
}
static void _turismoA_set_2G_channel(struct ssv_hw *sh, int ch)
{
    
    _turismoA_set_2G_txpwr(sh);
    
    SMAC_REG_SAFE_SET_BITS(sh, ADR_WIFI_PHY_COMMON_SYS_REG,
        0 << RG_RF_5G_BAND_SFT, RG_RF_5G_BAND_MSK);  
    
    // set rf channel manual on        
    SMAC_REG_SAFE_SET_BITS(sh, ADR_MODE_REGISTER, 1 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_MSK);
    udelay(50); // for hs3w send

    // set rf channel mapping on
    SMAC_REG_SAFE_SET_BITS(sh, ADR_SX_2_4GB_5GB_REGISTER_INT3BIT___CH_TABLE, 
        1 << RG_SX_RFCH_MAP_EN_SFT, RG_SX_RFCH_MAP_EN_MSK);  // set rf channel mapping on
    udelay(50); // for hs3w send
    
    // set channel
    SMAC_REG_SAFE_SET_BITS(sh, ADR_SX_2_4GB_5GB_REGISTER_INT3BIT___CH_TABLE, 
    ch << RG_SX_CHANNEL_SFT, RG_SX_CHANNEL_MSK);
    udelay(50); // for hs3w send
       
    // set RG_MODE to IDLE mode
    SMAC_REG_SAFE_SET_BITS(sh, ADR_MODE_REGISTER, 0 << RG_MODE_SFT, RG_MODE_MSK);
    udelay(50); // for hs3w send
    
    // set RG_MODE to WIFI_2.4G_RX
    SMAC_REG_SAFE_SET_BITS(sh, ADR_MODE_REGISTER, 3 << RG_MODE_SFT, RG_MODE_SFT);
    udelay(50); // for hs3w send

    //set RG_MODE_MANUAL off
    SMAC_REG_SAFE_SET_BITS(sh, ADR_MODE_REGISTER, 0 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_MSK);
    udelay(50); // for hs3w send
} 
#endif
static int ssv6006_turismoA_set_channel(struct ssv_softc *sc, struct ieee80211_channel  *chan, 
    enum nl80211_channel_type channel_type)
{
    // update according to 02Turismo_rf_d_wifi_sx_channel_20160310.js
    struct ssv_hw *sh = sc->sh;
	    
#ifdef USE_COMMON_MACRO

    TU_CHANGE_TURISMOA_CHANNEL(chan->hw_value, channel_type);
#else
    // update according to 02Turismo_rf_d_wifi_sx_channel_20160310.js

    int ch;
    const char *ch_type[]={"NL80211_CHAN_NO_HT",
	    "NL80211_CHAN_HT20",
	    "NL80211_CHAN_HT40MINUS",
	    "NL80211_CHAN_HT40PLUS"};


    ch = chan->hw_value;
    printk("%s: ch %d, type %s\n", __func__, ch, ch_type[channel_type]);
    //HAL_PHY_ENABLE(sc->sh, false);
   //Set channel type
    switch (channel_type){

      case 	NL80211_CHAN_HT20:
      case  NL80211_CHAN_NO_HT:
            SET_MTX_BLOCKTX_IGNORE_TOMAC_CCA_ED_SECONDARY(1);
            
            SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
                (0 << RG_TURISMO_TRX_BW_HT40_SFT) | (1 << RG_TURISMO_TRX_BW_MANUAL_SFT),
                (RG_TURISMO_TRX_BW_HT40_MSK | RG_TURISMO_TRX_BW_MANUAL_MSK));
            udelay(50); // should delay a while when set external RF

            SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_DIGITAL_ADD_ON_0,
                (0 << RG_TURISMO_TRX_40M_MODE_SFT) | (0 << RG_TURISMO_TRX_LO_UP_CH_SFT),
                (RG_TURISMO_TRX_LO_UP_CH_MSK | RG_TURISMO_TRX_40M_MODE_MSK));
            udelay(50); // should delay a while when set external RF
            
            SMAC_REG_SAFE_SET_BITS(sh, ADR_WIFI_PHY_COMMON_SYS_REG,
                (0 << RG_SYSTEM_BW_SFT) | (0 << RG_PRIMARY_CH_SIDE_SFT),
                (RG_SYSTEM_BW_MSK | RG_PRIMARY_CH_SIDE_MSK));

            break;

	    case    NL80211_CHAN_HT40MINUS:
            SET_MTX_BLOCKTX_IGNORE_TOMAC_CCA_ED_SECONDARY(0);
            
            SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
                (1 << RG_TURISMO_TRX_BW_HT40_SFT) | (1 << RG_TURISMO_TRX_BW_MANUAL_SFT),
                (RG_TURISMO_TRX_BW_HT40_MSK | RG_TURISMO_TRX_BW_MANUAL_MSK));
            udelay(50); // should delay a while when set external RF
                        
            SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_DIGITAL_ADD_ON_0,
                (1 << RG_TURISMO_TRX_40M_MODE_SFT) | (0 << RG_TURISMO_TRX_LO_UP_CH_SFT),
                (RG_TURISMO_TRX_LO_UP_CH_MSK | RG_TURISMO_TRX_40M_MODE_MSK));
            udelay(50); // should delay a while when set external RF
            
            SMAC_REG_SAFE_SET_BITS(sh, ADR_WIFI_PHY_COMMON_SYS_REG,
                (1 << RG_SYSTEM_BW_SFT) | (1 << RG_PRIMARY_CH_SIDE_SFT),
                (RG_SYSTEM_BW_MSK | RG_PRIMARY_CH_SIDE_MSK));
               
	        break;

	    case    NL80211_CHAN_HT40PLUS:
            SET_MTX_BLOCKTX_IGNORE_TOMAC_CCA_ED_SECONDARY(0);
            
            SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
                (1 << RG_TURISMO_TRX_BW_HT40_SFT) | (1 << RG_TURISMO_TRX_BW_MANUAL_SFT),
                (RG_TURISMO_TRX_BW_HT40_MSK | RG_TURISMO_TRX_BW_MANUAL_MSK));
            udelay(50); // should delay a while when set external RF	        
                        
            SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_DIGITAL_ADD_ON_0,
                (1 << RG_TURISMO_TRX_40M_MODE_SFT) | (1 << RG_TURISMO_TRX_LO_UP_CH_SFT),
                (RG_TURISMO_TRX_LO_UP_CH_MSK | RG_TURISMO_TRX_40M_MODE_MSK));
            udelay(50); // should delay a while when set external RF
                        
            SMAC_REG_SAFE_SET_BITS(sh, ADR_WIFI_PHY_COMMON_SYS_REG,
                (1 << RG_SYSTEM_BW_SFT) | (0 << RG_PRIMARY_CH_SIDE_SFT),
                (RG_SYSTEM_BW_MSK | RG_PRIMARY_CH_SIDE_MSK));

            break;
      default:
            break;
    }
    udelay(50);    

    if ( ch <=14 && ch >=1){
    	HAL_SET_SIFS(sh, INDEX_80211_BAND_2GHZ);    
        _turismoA_set_2G_channel(sh, ch);
    } else if (ch >=34){
        HAL_SET_SIFS(sh, INDEX_80211_BAND_5GHZ);
        // TODO Liam: this is a patch for TurismoA RF channel mapping bug on HT40
        if ((channel_type == NL80211_CHAN_HT40MINUS)|| (channel_type == NL80211_CHAN_HT40PLUS)){
            
            SMAC_REG_SAFE_SET_BITS(sh, ADR_TURISMO_TRX_DIGITAL_ADD_ON_0,
                (0 << RG_TURISMO_TRX_40M_MODE_SFT) | (0 << RG_TURISMO_TRX_LO_UP_CH_SFT),
                (RG_TURISMO_TRX_LO_UP_CH_MSK | RG_TURISMO_TRX_40M_MODE_MSK));
            udelay(50); // should delay a while when set external RF
            
            if (channel_type == NL80211_CHAN_HT40MINUS) {
                ch = ch - 2;
            } else {
                ch = ch + 2;
            }     
        }
        _turismoA_set_5G_channel(sh, ch);
    } else {
        printk("invalid channel %d\n", ch);
    }
    //HAL_PHY_ENABLE(sc->sh, true);
#endif
    return 0;

}

static void ssv6006_turismoA_init_cali (struct ssv_hw *sh)
{

#ifdef USE_COMMON_MACRO
    TU_INIT_TURISMOA_CALI;
#else

    int i , regval;
    u32 wifi_dc_addr;                                                                                     

    // set RG_MODE HS 3 wire manual to 0
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        0 << RG_TURISMO_TRX_HS_3WIRE_MANUAL_SFT, RG_TURISMO_TRX_HS_3WIRE_MANUAL_MSK);
    udelay(50); // should delay a while when set external RF          

    /* bit[ 0]=0, turn off RG_MODE_BY_HS_3WIRE*/
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_RF_D_MODE_CTRL, 
        0 << RG_TURISMO_TRX_MODE_BY_HS_3WIRE_SFT, RG_TURISMO_TRX_MODE_BY_HS_3WIRE_MSK);
    udelay(50); // should delay a while when set external RF  
    
    // turn off rg_hw_pinsel for HW RX gain control
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER, 
        0 << RG_TURISMO_TRX_HW_PINSEL_SFT, RG_TURISMO_TRX_HW_PINSEL_MSK);
    udelay(50); // should delay a while when set external RF          
    
    // enable ADC auto function
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_2_4G_TRX_MANUAL_ENABLE_REGISTER,
        ((1 << RG_TURISMO_TRX_EN_RX_ADC_SFT) | (0 <<RG_TURISMO_TRX_RX_ADC_MANUAL_SFT)), 
        (RG_TURISMO_TRX_EN_RX_ADC_MSK | RG_TURISMO_TRX_RX_ADC_MANUAL_MSK));
    udelay(50); // should delay a while when set external RF          
    
    printk("Start WiFi Rx DC calibration...\n");  
   
    // set RG_MODE_MANUAL ON        
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        1 << RG_TURISMO_TRX_MODE_MANUAL_SFT, RG_TURISMO_TRX_MODE_MANUAL_MSK);
    udelay(50); // should delay a while when set external RF          
  
    // set RG_MODE to Calibration mode //??? check this
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        1 << RG_TURISMO_TRX_MODE_SFT, RG_TURISMO_TRX_MODE_MSK);
    udelay(50); // should delay a while when set external RF          
    
    //set RG_CAL_INDEX to WiFi DC 2.4G
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        1 << RG_TURISMO_TRX_CAL_INDEX_SFT, RG_TURISMO_TRX_CAL_INDEX_MSK);
    
    msleep(10); // sleep 10 ms, for DC calibration done
  
    printk("--------------------------------------------\n");
    printk("--------2.4G Calibration result-------------");
    for (i = 0; i < 22; i++) {
       if (i %4 == 0)
          printk("\n");
       wifi_dc_addr = (ADR_TURISMO_TRX_WF_DCOC_IDAC_REGISTER1)+ (i << 2);
       SMAC_REG_READ(sh, wifi_dc_addr, &regval);
       printk("addr %x : val %x, ", wifi_dc_addr, regval);
    }

    //set RG_CAL_INDEX to NONE
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        0 << RG_TURISMO_TRX_CAL_INDEX_SFT, RG_TURISMO_TRX_CAL_INDEX_MSK);
    udelay(50); // should delay a while when set external RF          
   
    // set RG_MODE to IDLE mode
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        0 << RG_TURISMO_TRX_MODE_SFT, RG_TURISMO_TRX_MODE_MSK);
    udelay(50); // should delay a while when set external RF          

    if (sh->cfg.hw_caps & SSV6200_HW_CAP_5GHZ) {
  
        // set RG_MODE to Calibration mode //??? check this
        SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
            1 << RG_TURISMO_TRX_MODE_SFT, RG_TURISMO_TRX_MODE_MSK);
        udelay(50); // should delay a while when set external RF          
        
        //set RG_CAL_INDEX to WiFi DC 5G
        SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
            9 << RG_TURISMO_TRX_CAL_INDEX_SFT, RG_TURISMO_TRX_CAL_INDEX_MSK);
        
        msleep(10); // sleep 10 ms, for DC calibration done
        
        printk("\n--------------------------------------------\n");
        printk("--------5G Calibration result---------------");
        for (i = 0; i < 21; i++) {
           if (i %4 == 0)
              printk("\n");
           wifi_dc_addr = (ADR_TURISMO_TRX_5G_DCOC_IDAC_REGISTER1)+ (i << 2);
           SMAC_REG_READ(sh, wifi_dc_addr, &regval);
           printk("addr %x : val %x, ", wifi_dc_addr, regval);
        }
        printk("\n");
        //set RG_CAL_INDEX to NONE
        SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
            0 << RG_TURISMO_TRX_CAL_INDEX_SFT, RG_TURISMO_TRX_CAL_INDEX_MSK);
        udelay(50); // should delay a while when set external RF          
        
        // set RG_MODE to IDLE mode
        SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
            0 << RG_TURISMO_TRX_MODE_SFT, RG_TURISMO_TRX_MODE_MSK);
        udelay(50); // should delay a while when set external RF  

    }
   
    //set RG_MODE_MANUAL off
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        0 << RG_TURISMO_TRX_MODE_MANUAL_SFT, RG_TURISMO_TRX_MODE_MANUAL_MSK);
    udelay(50); // should delay a while when set external RF          

    // set RG_MODE HS 3 wire manual to 1
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        1 << RG_TURISMO_TRX_HS_3WIRE_MANUAL_SFT, RG_TURISMO_TRX_HS_3WIRE_MANUAL_MSK);
    udelay(50); // should delay a while when set external RF          
   
     /* bit[ 0]=1, turn on RG_MODE_BY_HS_3WIRE*/
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_RF_D_MODE_CTRL, 
        1 << RG_TURISMO_TRX_MODE_BY_HS_3WIRE_SFT, RG_TURISMO_TRX_MODE_BY_HS_3WIRE_MSK);
    udelay(50); // should delay a while when set external RF     
   
    // turn on rg_hw_pinsel for HW RX gain control
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        1 << RG_TURISMO_TRX_HW_PINSEL_SFT, RG_TURISMO_TRX_HW_PINSEL_MSK);
    udelay(50); // should delay a while when set external RF          
  
    // enable ADC manual off function
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_2_4G_TRX_MANUAL_ENABLE_REGISTER,
        ((0 << RG_TURISMO_TRX_EN_RX_ADC_SFT) | (1 <<RG_TURISMO_TRX_RX_ADC_MANUAL_SFT)), 
        (RG_TURISMO_TRX_EN_RX_ADC_MSK | RG_TURISMO_TRX_RX_ADC_MANUAL_MSK));
    udelay(50); // should delay a while when set external RF          

    // enable rg_en_rx_padsw
    //SMAC_REG_READ(sh, 0xCBB0A008, &regval); // dummy read , should do this in advance.
    //SMAC_REG_SET_BITS(sh, 0xCBB0A008, 1<< RG_EN_RX_PADSW_SFT, RG_EN_RX_PADSW_MSK);
#endif
}



static void ssv6006_turismoA_init_turismoA_trx(struct ssv_hw *sh)
{
#ifdef USE_COMMON_MACRO

    TU_INIT_TURISMOA_TRX;

#else
    // u32    regval;	   Nov 23 10:40:36 ssv-System-Product-Name kernel: [61378.477321] crystal_frequency_offset- 0

    // ToDo Liam: this is for turismoA only, should change for real chip.
  
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_PMU_REG_2, 1 << RG_TURISMO_TRX_LOAD_RFTABLE_RDY_SFT,
        RG_TURISMO_TRX_LOAD_RFTABLE_RDY_MSK);
    msleep(1); /* delay 1ms for 80M clock settle*/
    
    // turn on rg_hs3w_manual
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER, 
        1 << RG_TURISMO_TRX_HS_3WIRE_MANUAL_SFT, RG_TURISMO_TRX_HS_3WIRE_MANUAL_MSK);
    udelay(50); // should delay a while when set external RF          

    /* bit[ 0]=1, turn on RG_MODE_BY_HS_3WIRE*/
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_RF_D_MODE_CTRL, 
        1 << RG_TURISMO_TRX_MODE_BY_HS_3WIRE_SFT, RG_TURISMO_TRX_MODE_BY_HS_3WIRE_MSK);
    udelay(50); // should delay a while when set external RF  

    // turn on rg_hw_pinsel for HW RX gain control
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        1 << RG_TURISMO_TRX_HW_PINSEL_SFT, RG_TURISMO_TRX_HW_PINSEL_MSK);
    udelay(50); // should delay a while when set external RF          

    // turn on rg_txgain_phyctrl for Tx gain manual control
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        0 << RG_TURISMO_TRX_TXGAIN_PHYCTRL_SFT, RG_TURISMO_TRX_TXGAIN_PHYCTRL_MSK);
    udelay(50); // should delay a while when set external RF          

    // set rg_txgain to maximum gain
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MODE_REGISTER,
        3 << RG_TURISMO_TRX_TX_GAIN_SFT, RG_TURISMO_TRX_TX_GAIN_MSK);//<<24, 0x7f000000
    udelay(50); // should delay a while when set external RF          

    // disable DAC function
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_2_4G_TRX_MANUAL_ENABLE_REGISTER,
        ((0 << RG_TURISMO_TRX_EN_TX_DAC_SFT) | (1 <<RG_TURISMO_TRX_TX_DAC_MANUAL_SFT)), 
        (RG_TURISMO_TRX_EN_TX_DAC_MSK | RG_TURISMO_TRX_TX_DAC_MANUAL_MSK));
    udelay(50); // should delay a while when set external RF          

    // disable ADC function
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_2_4G_TRX_MANUAL_ENABLE_REGISTER, 
        ((0 << RG_TURISMO_TRX_EN_RX_ADC_SFT) | (1 <<RG_TURISMO_TRX_RX_ADC_MANUAL_SFT)), 
        (RG_TURISMO_TRX_EN_RX_ADC_MSK | RG_TURISMO_TRX_RX_ADC_MANUAL_MSK));
    udelay(50); // should delay a while when set external RF          

    // enable rg_en_tx_vtoi_2nd
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_2_4G_CALIBRATION__AMP__TEST_REGISTER, 
        1 << RG_TURISMO_TRX_EN_TX_VTOI_2ND_SFT, RG_TURISMO_TRX_EN_TX_VTOI_2ND_MSK);
    udelay(50); // should delay a while when set external RF          

    // enable rg_en_rx_padsw
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_2_4G_CALIBRATION__AMP__TEST_REGISTER, 
        1 << RG_TURISMO_TRX_EN_RX_PADSW_SFT, RG_TURISMO_TRX_EN_RX_PADSW_MSK);
    udelay(50); // should delay a while when set external RF          

 /*   // disable rg_sx_fref_doub
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_SYN_REGISTER_INT3BIT_CH_TABLE, 
        0 << RG_TURISMO_TRX_SX_FREF_DOUB_SFT, RG_TURISMO_TRX_SX_FREF_DOUB_MSK);
    msleep(1);  // should wait a while for consecutive write*/
  
    // set pad mux to TRx;
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_MCU_REG_0,
        1 << RG_TURISMO_TRX_PAD_MUX_SEL_SFT, RG_TURISMO_TRX_PAD_MUX_SEL_MSK);
    udelay(50); // should delay a while when set external RF          
    
    // gpio oe for rssi[3:0], rssi_indicator total 5 bits
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_IO_REG_2, 
        (RG_TURISMO_TRX_GPIO07_OE_MSK | RG_TURISMO_TRX_GPIO06_OE_MSK | RG_TURISMO_TRX_GPIO05_OE_MSK | RG_TURISMO_TRX_GPIO04_OE_MSK | RG_TURISMO_TRX_GPIO03_OE_MSK), 
        (RG_TURISMO_TRX_GPIO07_OE_MSK | RG_TURISMO_TRX_GPIO06_OE_MSK | RG_TURISMO_TRX_GPIO05_OE_MSK | RG_TURISMO_TRX_GPIO04_OE_MSK | RG_TURISMO_TRX_GPIO03_OE_MSK));
    udelay(50); // should delay a while when set external RF          

    /*set 40M clock enable*/
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_PMU_CTRL_REG, 1 << RG_FPGA_CLK_REF_40M_EN_SFT, 
        RG_FPGA_CLK_REF_40M_EN_MSK);
    udelay(50); // should delay a while when set external RF 

    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_IO_REG_0, 1 << RG_TURISMO_TRX_GPIO17_DS_SFT, 
        RG_TURISMO_TRX_GPIO17_DS_MSK);
    udelay(50); // should delay a while when set external RF          
        
    SMAC_REG_SET_BITS(sh, ADR_TURISMO_TRX_IO_REG_0, 1 << RG_TURISMO_TRX_GPIO17_OE_SFT, 
        RG_TURISMO_TRX_GPIO17_OE_MSK);
    msleep(100);
#endif

}

void ssv6006_turismoA_write_rf_table(struct ssv_hw *sh )
{                                                                
                                                 
    u32 i = 0;
                                                    
    for( i = 0; i < sizeof(ssv6006_turismoA_rf_setting)/sizeof(ssv_cabrio_reg); i++) {
       SMAC_REG_WRITE(sh, ssv6006_turismoA_rf_setting[i].address, ssv6006_turismoA_rf_setting[i].data );
       udelay(50); // should delay a while when set external RF                              
    }                                                            
                                                         
}
static int ssv6006_turismoA_set_pll_phy_rf(struct ssv_hw *sh
    , ssv_cabrio_reg *rf_tbl, ssv_cabrio_reg *phy_tbl)
{	
    int  ret = 0;

#ifdef USE_COMMON_MACRO
    INIT_TURISMOA_SYS;
#else
    ssv6006_turismoA_write_rf_table(sh);
 
    ssv6006_turismoA_init_turismoA_trx(sh);
 
    ssv6006_turismoA_init_PLL(sh);    

    HAL_SET_PHY_MODE(sh, false); /* disable all phy mode */
    ret = SSV6XXX_SET_HW_TABLE(sh, ssv6006_turismoA_phy_setting);
    ssv6006_turismoA_init_cali(sh);
#endif
    return ret;
}

static bool ssv6006_turismoA_set_rf_enable(struct ssv_hw *sh, bool val)
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
    udelay(50); // should delay a while when set external RF          
  
	return true;   
}

static bool ssv6006_turismoA_dump_phy_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;    
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;    

    raw = ssv6006_turismoA_phy_setting;

    snprintf_res(cmd_data, ">> PHY Register Table:\n"); 
    
    for(s = 0; s < ssv6006_turismoA_phy_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);
     }
    snprintf_res(cmd_data, ">>PHY Table version: %s\n", SSV6006_TURISMOA_PHY_TABLE_VER);
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6006_turismoA_dump_rf_reg(struct ssv_hw *sh)
{
    u32 regval;
    int s;
    ssv_cabrio_reg *raw;
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    raw = ssv6006_turismoA_rf_setting;

    snprintf_res(cmd_data, ">> RF Register Table:\n");

    for(s = 0; s < ssv6006_turismoA_rf_tbl_size/sizeof(ssv_cabrio_reg); s++, raw++) {
        SMAC_REG_READ(sh, raw->address, &regval);
        snprintf_res(cmd_data, "   ADDR[0x%08x] = 0x%08x\n", 
            raw->address, regval);      
    }
    snprintf_res(cmd_data, ">>RF Table version: %s\n", SSV6006_TURISMOA_RF_TABLE_VER);
    snprintf_res(cmd_data, "\n\n");

    return 0;
}

static bool ssv6006_turismoA_support_iqk_cmd(struct ssv_hw *sh)
{
    return false;
}

static void ssv6006_cmd_cali(struct ssv_hw *sh, int argc, char *argv[])
{
    struct ssv_cmd_data *cmd_data = &sh->sc->cmd_data;
    
    if(!strcmp(argv[1], "do")){
        
        ssv6006_turismoA_init_cali(sh);
        ssv6006_turismoA_init_turismoA_trx(sh);
        snprintf_res(cmd_data,"\n   CALIRATION DONE\n");
    
    } else if(!strcmp(argv[1], "show")) {
        u32 i, regval = 0, wifi_dc_addr;

        snprintf_res(cmd_data,"--------------------------------------------\n");
        snprintf_res(cmd_data,"--------- Calibration result----------------");
        for (i = 0; i < 22; i++) {
           if (i %4 == 0)
              snprintf_res(cmd_data,"\n");
           wifi_dc_addr = ADR_TURISMO_TRX_WF_DCOC_IDAC_REGISTER1+ (i << 2); 
           SMAC_REG_READ(sh, wifi_dc_addr, &regval);
           snprintf_res(cmd_data,"addr %x : val %x, ", wifi_dc_addr, regval);
        }
    
    } else {
        snprintf_res(cmd_data,"\n cali [do] [show]\n");
    }     
    
}

void ssv_attach_ssv6006_turismoA_BBRF(struct ssv_hal_ops *hal_ops)
{
    hal_ops->load_phy_table = ssv6006_turismoA_load_phy_table;
    hal_ops->get_phy_table_size = ssv6006_turismoA_get_phy_table_size;
    hal_ops->get_rf_table_size = ssv6006_turismoA_get_rf_table_size;
    hal_ops->load_rf_table = ssv6006_turismoA_load_rf_table;
    hal_ops->init_pll = ssv6006_turismoA_init_PLL;
    hal_ops->set_channel = ssv6006_turismoA_set_channel;
    hal_ops->set_pll_phy_rf = ssv6006_turismoA_set_pll_phy_rf;
    hal_ops->set_rf_enable = ssv6006_turismoA_set_rf_enable;
    
    hal_ops->dump_phy_reg = ssv6006_turismoA_dump_phy_reg;
    hal_ops->dump_rf_reg = ssv6006_turismoA_dump_rf_reg;
    hal_ops->support_iqk_cmd = ssv6006_turismoA_support_iqk_cmd;
    hal_ops->cmd_cali = ssv6006_cmd_cali;
}
#endif
