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

#define SSV6006_TURISMOC_RF_TABLE_VER "11.00"  
  
ssv_cabrio_reg ssv6006_turismoC_rf_setting[]={
// 2G Tx CAPSW
    {0xCCB0A420,0x0033E73F}, // RG_WF_TXPGA_CAPSW[1:0] = 11, for 2G TX capsw
// Sx
    {0xCCB0A598,0x0F1E00FF}, // bit[3:0], RG_SX5GB_DIV_PSCVDD = 4'b1111

// 2G/5G R2T ramping control
    {0xCCB0A530,0x001F1F01}, // set 2G R2T delay time
    {0xCCB0A604,0x001F1F01}, // set 5G R2T delay time

// Peisi for Amkor
//    {0xCCB0A62C,0x9264924A}, // F1, F0
//    {0xCCB0A630,0xFE4AFE4A}, // F3, F2
// Peisi for 2-bond
    {0xCCB0A62C,0x924d924A}, // F1, F0
    {0xCCB0A630,0xb6cab6cc}, // F3, F2

    {0xCCB0A634,0x00000000}, // set F3 RG_5G_TX_GAIN to  0 = bit[31:25] = 0x00
                             // set F2 RG_5G_TX_GAIN to  0 = bit[24:18] = 0x00
                             // set F1 RG_5G_TX_GAIN to  0 = bit[17:11] = 0x00
                             // set F0 RG_5G_TX_GAIN to  0 = bit[10: 4] = 0x00

// 5G band threshold
    {0xCCB0A8CC,0x141E157C}, // set 5G band threshold 5150, 5500
    {0xCCB0A8D0,0x00001644}, // set 5G band threshold 5700

//    {0xCCB0ADA8,0x80806c5b}, // 5G F0, F1, F2, F3 digital gain
//    {0xCCB0ADAC,0x00000072}, // 2G digital gain

// RF mode control selection
    // RF_PHY_MODE by PHY
    {0xccb0a88c,0x00000010},
    {0xccb0a808,0x88000000},
    
// PMU
    {0xCCB0B000,0x24844214}, //0xB000 B<25:8> =<00_1000_0100_0100_0010>
//    {0xCCB0B080,0x0002FEA4}, // bit[8], RG_EN_IOTADC_160M = 0
    {0xCCB0B008,0x486041BF}, // bit[0], RG_DCDC_MODE = 1
//    {0xCCB0B004,0xA51A8820}, // bit[31], rf_load_table_rdy
};
