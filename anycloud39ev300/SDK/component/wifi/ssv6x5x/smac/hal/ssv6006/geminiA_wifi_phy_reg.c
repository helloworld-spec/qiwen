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
 
#define SSV6006_GEMINIA_PHY_TABLE_VER "342.00"

ssv_cabrio_reg ssv6006_geminiA_phy_setting[]={
    // turn off phy
    //{0xccb0e004,0x00000000},

    // Tx packet generator
    {0xccb0e010,0x00000FFF},
    {0xccb0e014,0x00807f03},
    {0xccb0e018,0x0055003C},
    {0xccb0e01C,0x00000064},
    {0xccb0e020,0x00000000},

    // AGC
    {0xccb0e02C,0x70046072},
    {0xccb0e030,0x70046072},
    {0xccb0e034,0x1A040800},
    {0xccb0e038,0x630F36D0},
    {0xccb0e03C,0x100c0003},

    // DAGC 11b
    {0xccb0e040,0x11600400},
    {0xccb0e044,0x00080868},

    // DAGC 11gn HT20
    {0xccb0e048,0xFF001160},
    {0xccb0e04C,0x00100040},

    // agc bandwidth
    {0xccb0e060,0x55500000},

    // DAGC 11gn HT40
    {0xccb0e12C,0x00001160},
    {0xccb0e130,0x00100040},

    // DAGC 11gn init gain
    {0xccb0e134,0x00100010},

    // baseband power control
    {0xccb0e180,0x00010080},//barker_cck
    {0xccb0e184,0xc0c09090},//Legacy bpsk,qpsk,16qam,64qam
    {0xccb0e188,0xc0c09090},//ht20 bpsk,qpsk,16qam,64qam
    {0xccb0e18c,0xc0c09090},//ht40 bpsk,qpsk,16qam,64qam

    // rf power control
    {0xccb0e190,0x00010003},//barker_cck
    {0xccb0e194,0x03030303},//Legacy bpsk,qpsk,16qam,64qam
    {0xccb0e198,0x03030303},//ht20 bpsk,qpsk,16qam,64qam
    {0xccb0e19c,0x03030303},//ht40 bpsk,qpsk,16qam,64qam

    // RSSI
    {0xccb0e080,0x0110000F},

    // Tx 11b ramp setting
    {0xccb0e4b4,0x00002001},

    // Tx 11gn ramp setting
    {0xccb0ecA4,0x00009001},

    // Tx 11gn ifft gain setting
    {0xccb0ecB8,0x000C50CC},

    // Tx IQ swap
    {0xccb0fc44,0x00028080},

    // RX 11gn setting
    {0xccb0f010,0x00004775},
    {0xccb0f00c,0x10000075},
    {0xccb0f010,0x3F304905},
    {0xccb0f014,0x40182000},
    {0xccb0f018,0x20600000},
    {0xccb0f01C,0x0c010090},
    {0xccb0f03C,0x00000066},

    {0xccb0f020,0x50505050},
    {0xccb0f024,0x50000000},
    {0xccb0f028,0x50505050},
    {0xccb0f02c,0x50505050},
    {0xccb0f030,0x50000000},
    {0xccb0f034,0x00002424},

    {0xccb0f09c,0x0000300A},
    {0xccb0f0C0,0x0f0003f0},
    {0xccb0f0C4,0x30023003},

    {0xccb0f0CC,0x00000120},

    {0xccb0f130,0x40000000},
    {0xccb0f164,0x000f0090},
    {0xccb0f188,0x85000000},
    {0xccb0f190,0x00000020},
    {0xccb0f3F8,0x00100001},
    {0xccb0f3FC,0x00010425},

    // 11b Rx
    {0xccb0e804,0x00020000},
    {0xccb0e808,0x20400050},
    {0xccb0e80c,0x00003467},
    {0xccb0e810,0x00560000},
    {0xccb0e814,0x30000015},
    {0xccb0e818,0x00390002},
    {0xccb0e81C,0x03030003},
    {0xccb0e830,0x04041606},

    {0xccb0e89c,0x00c0000A},
    {0xccb0e8A0,0x00000000}, 
    {0xccb0ebF8,0x00100000},
    {0xccb0ebFC,0x00000001},

    //{0xccb0e000,0x80000000}, // ht20
    //{0xccb0e000,0x8000c000}, // ht40 high primary channel
    //{0xccb0e000,0x80008000}, // ht40 low primary channel

    // Tx DAC
    {0xccb0a810,0x01000000},

    // Rx ADC
    {0xccb0a808,0x08000000},

    // RF_PHY_MODE by PHY
    {0xccb0a88c,0x00000010},
};
