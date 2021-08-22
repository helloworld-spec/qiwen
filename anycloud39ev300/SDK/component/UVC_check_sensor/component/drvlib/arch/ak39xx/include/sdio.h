/**@file arch_sd.h
 * @brief provide arch level operations of how to control sd&sdio.
 *
 * This file describe sd&sdio controller driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __ARCH_SD_H
#define __ARCH_SD_H

#include "anyka_types.h"
#include "arch_mmc_sd.h"
#include "l2.h"
#ifdef __cplusplus
extern "C" {
#endif

//clock ctrl reg
#define CLK_DIV_L_OFFSET           0
#define CLK_DIV_H_OFFSET           8
#define SD_CLK_ENABLE              (1<<16)
#define PWR_SAVE_ENABLE            (1<<17)
#define FALLING_TRIGGER            (1<<19)
#define SD_INTERFACE_ENABLE        (1<<20)

//cmd reg
#define CMD_INDEX_OFFSET           1
#define CPSM_ENABLE                (1<<0)
#define WAIT_REP_OFFSET            7
#define WAIT_CMD_PEND              (1<<9)
#define RSP_CRC_NO_CHK             (1<<10)
        
//status reg
#define CMD_CRC_FAIL               (1 << 0)
#define DATA_CRC_FAIL              (1 << 1)
#define CMD_TIME_OUT               (1 << 2)
#define DATA_TIME_OUT              (1 << 3)
#define CMD_RESP_END               (1 << 4)
#define CMD_SENT                   (1 << 5)
#define DATA_END                   (1 << 6)
#define DATA_BLOCK_END             (1 << 7)
#define DATA_START_BIT_ERR         (1 << 8)
#define CMD_ACTIVE                 (1 << 9)
#define TX_ACTIVE                  (1 << 10)
#define RX_ACTIVE                  (1 << 11)
#define DATA_BUF_FULL              (1 << 12)
#define DATA_BUF_EMPTY             (1 << 13)
#define DATA_BUF_HALF_FULL         (1 << 14)
#define DATA_BUF_HALF_EMPTY        (1 << 15)

//dma reg
#define BUF_SIZE_OFFSET         (17)
#define DMA_EN                  (1<<16)
#define START_ADDR_OFFSET       (1)
#define START_ADDR_MASK         (0x7fff<<1)
#define BUF_EN                  (1)

//data ctrl reg
#define SD_DATA_CTL_ENABLE              1
#define SD_DATA_CTL_DISABLE             0
#define SD_DATA_CTL_TO_HOST             1
#define SD_DATA_CTL_TO_CARD             0
#define SD_DATA_CTL_BUS_MODE_OFFSET     3
#define SD_DATA_CTL_DIRECTION_OFFSET    1
#define SD_DATA_CTL_BLOCK_LEN_OFFSET    16
#define SD_DAT_MAX_TIMER_V              0x800000

#define SD_CMD(n)                       n       
#define SD_NO_RESPONSE                  0                           
#define SD_SHORT_RESPONSE               1                               
#define SD_LONG_RESPONSE                3
#define SD_NO_ARGUMENT                  0
#define SD_POWER_SAVE_ENABLE            1
#define SD_POWER_SAVE_DISABLE           0 
#define SD_DEFAULT_BLOCK_LEN            512
#define SD_DEFAULT_BLOCK_LEN_BIT        9
#define SDIO_MAX_BLOCK_LEN              2048
#define SD_BUS_WIDTH_1BIT               0
#define SD_BUS_WIDTH_4BIT               2
#define SD_DMA_SIZE_32K                 (32*1024)
#define SD_DMA_SIZE_64K                 (64*1024)
#define INNER_BUF_MODE                  0
#define L2_CPU_MODE                     1
#define L2_DMA_MODE                     2



void sdio_set_interface(T_eCARD_INTERFACE cif);
bool sdio_send_cmd( unsigned char cmd_index, unsigned char request, unsigned long arg );
unsigned long  sdio_get_short_resp();
void sdio_get_long_resp();
bool sdio_read_multi_byte(unsigned long arg,unsigned char data[],unsigned long size);
bool sdio_write_multi_byte(unsigned long arg,unsigned char data[],unsigned long size);
bool sdio_read_multi_block(unsigned long arg,unsigned char data[],unsigned long size);
bool sdio_read_multi_block_L2(unsigned long arg,unsigned char data[],unsigned long size);
bool sdio_write_multi_block(unsigned long arg,unsigned char data[],unsigned long size);
bool sdio_write_multi_block_L2(unsigned long arg,unsigned char data[],unsigned long size);
bool sdio_trans_busy();
bool sdio_trans_data_dma(unsigned long ram_addr,unsigned long size,unsigned char dir);
bool sdio_trans_data_cpu(unsigned long ram_addr,unsigned long size,unsigned char dir);



#ifdef __cplusplus
}
#endif


#endif 
  
