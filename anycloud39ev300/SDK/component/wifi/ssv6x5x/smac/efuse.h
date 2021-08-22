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

#ifndef _SSV_EFUSE_H_
#define _SSV_EFUSE_H_

#include "dev.h"

struct efuse_map {
    u8 offset;
    u8 byte_cnts;
    u16 value;
};

enum efuse_data_item {
   EFUSE_R_CALIBRATION_RESULT = 1,
   EFUSE_SAR_RESULT,
   EFUSE_MAC,
   EFUSE_CRYSTAL_FREQUENCY_OFFSET,
   EFUSE_TX_POWER_INDEX_1,
   EFUSE_TX_POWER_INDEX_2,
   EFUSE_CHIP_ID,
   NO_USE,
   EFUSE_VID,
   EFUSE_PID,
   EFUSE_MAC_NEW,
   EFUSE_RATE_TABLE_1,
   EFUSE_RATE_TABLE_2
};

//#define EFUSE_DEBUG
#ifdef SSV_SUPPORT_HAL
#define SSV_READ_EFUSE(_sh, _table)             HAL_READ_EFUSE(_sh, _table)
#define SSV_WRITE_EFUSE(_sh, _pbuf, _len)       HAL_WRITE_EFUSE(_sh, _pbuf, _len)

#else
#define EFUSE_HWSET_MAX_SIZE (256-32)													// in bits
#define EFUSE_MAX_SECTION_MAP (EFUSE_HWSET_MAX_SIZE>>5)   // in 32bits.

//#define SSV_EFUSE_BASE	        0xC2000108
#define SSV_EFUSE_ID_READ_SWITCH   0xC2000128
#define SSV_EFUSE_ID_RAW_DATA_BASE 0xC200014C

#define SSV_EFUSE_READ_SWITCH	0xC200012C
#define SSV_EFUSE_RAW_DATA_BASE	0xC2000150

u8 read_efuse(struct ssv_hw *sh, u8 *pbuf);
void write_efuse(struct ssv_hw *sh, u8 *data, u8 data_length);

#define SSV_READ_EFUSE(_sh, _table)             read_efuse(_sh, _table)
#define SSV_WRITE_EFUSE(_sh, _data, _len)       write_efuse(_sh, _data, _len)
#endif

u16 parser_efuse(struct ssv_hw *sh, u8 *pbuf, u8 *mac_addr, u8 *new_mac_addr, struct efuse_map *efuse_tbl);
void efuse_read_all_map(struct ssv_hw *sh);

#endif

