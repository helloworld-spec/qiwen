/**
 * @file l2.h
 * @brief l2 driver header file, define l2 register and api.
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-11-15
 * @version 1.0
 * @ref AK37XX technical manual.
 */

#ifndef __L2_H__
#define __L2_H__

#include <asm/arch/anyka_types.h>


#define    BUF_NULL     0xff
#define    BUF2MEM                0
#define    MEM2BUF                1

typedef enum
{
    ADDR_USB_EP2 = 0,           ///< usb ep2      0
    ADDR_USB_EP3 = 1,           ///< usb ep3      1
    ADDR_RESERVED = 2,          ///< usb ep4      2
    ADDR_NFC = 3,               ///< nfc            3
    ADDR_MMC_SD = 4,            ///< sdmmc       4
    ADDR_SDIO = 5,              ///< sdio           5
    ADDR_SPI1_RX = 7,           ///< spi1 rx        7
    ADDR_SPI1_TX = 8,           ///< spi1 tx        8
    ADDR_DAC = 9,               ///< dac            9
    ADDR_SPI2_RX = 10,           ///< spi2 rx        10
    ADDR_SPI2_TX = 11,           ///< spi2 tx        11
    ADDR_ADC = 14              ///< adc            14
}DEVICE_SELECT;



T_VOID l2_init(T_VOID);


T_BOOL dma_init(T_VOID);


T_U8 l2_get_status(T_U8 buf_id);

T_VOID l2_combuf_cpu(T_U32 ram_addr, T_U8 buf_id, T_U32 tran_byte, T_U8 tran_dir);

T_VOID l2_combuf_dma(T_U32 ram_addr, T_U8 buf_id, T_U32 tran_byte, T_U8 tran_dir, T_BOOL bIntr);

T_VOID l2_clr_status(T_U8 buf_id);

T_BOOL l2_combuf_wait_dma_finish(T_U8 buf_id);

T_U8 l2_alloc(DEVICE_SELECT dev_slct);

T_VOID l2_free(DEVICE_SELECT dev_slct);

#endif

