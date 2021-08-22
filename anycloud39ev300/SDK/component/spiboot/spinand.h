/**@file arch_nand.h
 * @brief AK322x nand controller
 *
 * This file describe how to control the AK322x nandflash driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan, chenyanyan
 * @date    2007-1-10
 * @version 1.0
 */
#ifndef __ARCH_NAND_H__
#define __ARCH_NAND_H__


#include "anyka_types.h"


#define NAND_FAIL_STATUS          (1UL << 31)
#define NAND_FAIL_NFC_TIMEOUT     (1 << 30)
#define NAND_FAIL_PARAM           (1 << 29)
#define NAND_FAIL_ECC             (1 << 28)
#define NAND_FAIL_MASK            (NAND_FAIL_STATUS |NAND_FAIL_NFC_TIMEOUT | NAND_FAIL_PARAM | NAND_FAIL_ECC)

#define HEAD_SIZE 8               //asa head info size


#define ASA_MAX_BLOCK_TRY 50    //define asa block max try use
#define ASA_BLOCK_COUNT 10       //define asa block max count


typedef T_U32 T_NAND_RET;

typedef struct NAND_ECC_CTL
{
    T_U16 nMainLen;
    T_U16 nSectOffset;
    T_U8   nMainEcc;
    T_U8   nAddEcc;
    T_U8   nAddLen;//fixed 8 bytes?
    T_BOOL   bSeperated;
    T_U8    nMainSectCnt;
}T_NAND_ECC_CTRL;

typedef struct NAND_ADDR
{
    T_U16 nSectAddr;
    T_U16 nTargetAddr;
    T_U32 nLogAbsPageAddr;
}T_NAND_ADDR;

typedef struct NAND_DATA
{
    T_U16    nSectCnt;//read bytes biaoshi �ֽ���
    T_U16    nPageCnt;
    T_U8    *pMain;
    T_U8    *pAdd;
    T_NAND_ECC_CTRL *pEccCtrl;
}T_NAND_DATA;



struct NAND_DEVICE_INFO
{
    T_U32   nID[2];
    T_U8    nChipCnt;
    T_NAND_ECC_CTRL   **ppEccCtrl;    
};
typedef struct  NAND_DEVICE_INFO T_NAND_DEVICE_INFO;

#if 0
typedef struct Nand_phy_info{
    T_U32  chip_id;//chip id
    T_U16  page_size; //page size
    T_U16  page_per_blk; //page of one block
    T_U16  blk_num;//total block number
    T_U16  group_blk_num;//the same concept as die, according to nand's struture
    T_U16  plane_blk_num;
    T_U8   spare_size;//spare�����С�ĵ�λ��������255 Byte
    T_U8   col_cycle;//column address cycle
    T_U32  flag;//character bits
    T_U32  cmd_len;//nandflash command length
    T_U32  data_len;//nandflash data length
}T_NAND_PARAM;
#endif

typedef struct Nand_phy_info{
    unsigned long  chip_id;//chip id
    unsigned short  page_size; //page size
    unsigned short  page_per_blk; //page of one blok
    unsigned short  blk_num;//total block number
    unsigned short  group_blk_num;//the same concept as die, according to nand's struture
    unsigned short  plane_blk_num;   
    unsigned char   spare_size;//spare�����С�ĵ�λ��������?55 Byte
    unsigned char   col_cycle;//column address cycle
    unsigned char   lst_col_mask;//last column  addrress cycle mask bit
    unsigned char   row_cycle;//row address cycle
    unsigned char   delay_cnt;//Rb delay, unit is 1024 asic clock, default value corresponds to 84MHz
    unsigned char   custom_nd;//nand type flag, used to detect the original invilid block
        //currently there are 7 types, more types might be added when new nand come out
        //˵����������ǰһ����page��,��һ����page�е�λ��, �����Щλ�ò��?xFF���block�ǳ�������
        //NAND_TYPE_SAMSUNG:        0x1 СҳSLC([0,1],[517]),   ��ҳSLC([0,1],[2048]),          MLC([127], [2048/4096])
        //NAND_TYPE_HYNIX:          0x2 СҳSLC([0,1],[517]),   ��ҳSLC([0,1],[2048]),          MLC([125,127], [2048/4096])
        //NAND_TYPE_TOSHIBA:        0x3 СҳSLC([0,1],[0,512]), ��ҳSLC([0,1],[0,2048]),        MLC([127], [0,2048/4096])
        //NAND_TYPE_TOSHIBA_EXT:    0x4 СҳSLC(),              ��ҳSLC(),                      MLC([0,127/255], [0,2048/4096/8192])
        //NAND_TYPE_MICRON:         0x5 СҳSLC([0,1],[512]),   ��ҳSLC([0,1],[2048]),          MLC([0,1], [2048/4096])
        //NAND_TYPE_ST:             0x6 СҳSLC([0,1],[517]),   ��ҳSLC([0],[2048,2053]),       MLC([127], [0])
        //NAND_TYPE_MICRON_4K       0x7 СҳSLC(),              ��ҳSLC(),                      MLC([0], [4096 ~ 4096+218])
    unsigned long  flag;//character bits, ���?λ��ʾplane���ԣ����λ��ʾ�Ƿ���Ҫblock��˳��дpage
    //bit31��ʾ�Ƿ���copyback��1��ʾ��copyback
    //bit30��ʾ�Ƿ�ֻ��һ��plane��1��ʾֻ��һ��plane
    //bit29��ʾ�Ƿ�ǰ��plane��1��ʾ��ǰ��plane
    //bit28��ʾ�Ƿ���żplane��1��ʾ����żplane

    //����bit��Ϊ�˽��page��block��ַ�����������ӵĿ���bit:
    //bit11��ʾblock number per die�Ƿ���Ҫ���Ϲ�������Toshiba TH58NVG6D2ETA20��2048 block/die(ʵ����2084 block/die)
         //Ϊ�˶�����һ��die��block����Ҫ����Ϊ4096 block/die���ײ�����
    //bit10��ʾpage number�Ƿ���Ҫ���Ϲ�������TLC��192page/block��Ϊ�˶�����һ��block����Ҫ����Ϊ256page/block������
    
    //bit8~9��ʾspare�����С�ĸ�λ����λ��?56 Bytes����spare_size��ΪT_U8�������Ա�ʾ����nand��400����ֽڵ�spare��С
    //bit4-7��ʾECC���ͣ�0Ϊ4 bit/512B��1Ϊ8 bit/512B��2Ϊ12 bit/512B��3Ϊ16 bit/512B��4Ϊ24 bit/1024B��5Ϊ32 bit/1024B
    //bit0��ʾ��ͬһ��block���Ƿ���Ҫ˳��дpage��1��ʾ��Ҫ��˳��д������nandΪMLC
    //ע��: ���?bit29��bit28)Ϊ'11'�����ʾ��chip����4��plane��������żҲ��ǰ��plane

    unsigned long  cmd_len;//nandflash command length
    unsigned long  data_len;//nandflash data length
    unsigned char   des_str[32];//descriptor string
}T_NAND_PARAM, T_NAND_PHY_INFO;



typedef struct {
 unsigned char head_str[8];
 unsigned long verify[2];
 unsigned long item_num;
 unsigned long info_end;
}
T_ASA_HEAD;

typedef struct
{
    unsigned short page_start;
    unsigned short page_count;
    unsigned short info_start;
    unsigned short info_len;
 }T_ASA_ITEM;


 typedef struct
{
    unsigned char file_name[8];
    unsigned long file_length;
    unsigned long start_page;
    unsigned long end_page;
}T_ASA_FILE_INFO;

typedef struct tag_ASA_Param
{
    unsigned short   PagePerBlock;       
    unsigned short   BytesPerPage;
    unsigned short   BlockNum;           //blocks of one chip 
}T_ASA_PARAM;

typedef struct tag_ASA_Block
{
    unsigned char  asa_blocks[ASA_BLOCK_COUNT];   //保存安全区块表的数组
    unsigned char  asa_count;                     //初始化以后用于安全区的block个数，仅指安全区可用�?
    unsigned char  asa_head;                      //安全区块数组中数据最新的块的索引 
    unsigned char  asa_block_cnt;                 //安全区所有块个数，包含坏�?
    unsigned long write_time;                    //块被擦写次数
}T_ASA_BLOCK;


#define		PARTITION_CNT	            3


 typedef struct
{
    unsigned char update_flag[6];
}T_PARTITION_NAME_INFO;

 typedef struct
{
    unsigned long partition_cnt;
    T_PARTITION_NAME_INFO partition_name_info[PARTITION_CNT];
}T_PARTITION_INFO;






T_BOOL spi_nand_init(T_eSPI_ID spi_id);

#endif //__ARCH_NAND_H__
