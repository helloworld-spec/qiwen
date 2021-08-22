/**
 * @filename nand_list.h
 * @brief: AK3223M interrupt
 *
 * This file describe what is in the table of nand list
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @modify chenyanyan
 * @date    2007-1-10
 * @version 1.0
 * @ref
 */

#ifndef __CHIP_NFC_3224__
#define __CHIP_NFC_3224__

#include	"anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup NandFlash Architecture NandFlash Interface
 *  @ingroup Architecture
 */
/*@{*/

/**
* @BRIEF    Nandflash info define
* @AUTHOR   zhaojiahuan
* @DATE     2006-7-17
*/

typedef struct
Nand_phy_info{
    unsigned long  chip_id;//chip id
    unsigned short  page_size; //page size
    unsigned short  page_per_blk; //page of one blok
    unsigned short  blk_num;//total block number
    unsigned short  group_blk_num;//the same concept as die, according to nand's struture
    unsigned short  plane_blk_num;   
    unsigned char   spare_size;//spare区域大小的低位，不超过255 Byte
    unsigned char   col_cycle;//column address cycle
    unsigned char   lst_col_mask;//last column  addrress cycle mask bit
    unsigned char   row_cycle;//row address cycle
    unsigned char   delay_cnt;//Rb delay, unit is 1024 asic clock, default value corresponds to 84MHz
    unsigned char   custom_nd;//nand type flag, used to detect the original invilid block
        //currently there are 7 types, more types might be added when new nand come out
        //说明：括号里前一个是page号,后一个是page中的位置, 如果这些位置不为0xFF则该block是出厂坏快
        //NAND_TYPE_SAMSUNG:        0x1 小页SLC([0,1],[517]),   大页SLC([0,1],[2048]),          MLC([127], [2048/4096])
        //NAND_TYPE_HYNIX:          0x2 小页SLC([0,1],[517]),   大页SLC([0,1],[2048]),          MLC([125,127], [2048/4096])
        //NAND_TYPE_TOSHIBA:        0x3 小页SLC([0,1],[0,512]), 大页SLC([0,1],[0,2048]),        MLC([127], [0,2048/4096])
        //NAND_TYPE_TOSHIBA_EXT:    0x4 小页SLC(),              大页SLC(),                      MLC([0,127/255], [0,2048/4096/8192])
        //NAND_TYPE_MICRON:         0x5 小页SLC([0,1],[512]),   大页SLC([0,1],[2048]),          MLC([0,1], [2048/4096])
        //NAND_TYPE_ST:             0x6 小页SLC([0,1],[517]),   大页SLC([0],[2048,2053]),       MLC([127], [0])
        //NAND_TYPE_MICRON_4K       0x7 小页SLC(),              大页SLC(),                      MLC([0], [4096 ~ 4096+218])
    unsigned long  flag;//character bits, 最高4位表示plane属性，最低位表示是否需要block内顺序写page
    //bit31表示是否有copyback，1表示有copyback
    //bit30表示是否只有一个plane，1表示只有一个plane
    //bit29表示是否前后plane，1表示有前后plane
    //bit28表示是否奇偶plane，1表示有奇偶plane

    //以下bit是为了解决page和block地址不连续而增加的控制bit:
    //bit11表示block number per die是否需要向上规整，如Toshiba TH58NVG6D2ETA20是2048 block/die(实际有2084 block/die)
         //为了对齐下一个die的block则需要规整为4096 block/die给底层驱动
    //bit10表示page number是否需要向上规整，如TLC是192page/block，为了对齐下一个block则需要规整为256page/block给驱动
    
    //bit8~9表示spare区域大小的高位，单位是256 Bytes。因spare_size仅为unsigned char，不足以表示新型nand的400多个字节的spare大小
    //bit4-7表示ECC类型，0为4 bit/512B，1为8 bit/512B，2为12 bit/512B，3为16 bit/512B，4为24 bit/1024B，5为32 bit/1024B
    //bit0表示在同一个block内是否需要顺序写page，1表示需要按顺序写，即该nand为MLC
    //注意: 如果(bit29和bit28)为'11'，则表示该chip包括4个plane，既有奇偶也有前后plane

    unsigned long  cmd_len;//nandflash command length
    unsigned long  data_len;//nandflash data length
    unsigned char   des_str[32];//descriptor string
}T_NAND_PHY_INFO;

#define ERROR_CHIP_ID   0xFFFFFFFF

/*@}*/
#ifdef __cplusplus
}
#endif

#endif
