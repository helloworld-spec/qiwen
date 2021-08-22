/**@file hal_sdio.h
 * @brief provide hal level operations of how to control sdio.
 *
 * This file describe sdio hal driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_SDIO_H
#define __HAL_SDIO_H

#include "arch_sdio.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SDIO_FUN_NUM_MASK                (3<<28)
#define SDIO_FUN_NUM_OFFSET               28
#define SDIO_MP_MASK                     (1<<27)
#define SDIO_OCR_MASK                    (0xFFFFFF)
#define SDIO_DEFAULT_VOLTAGE             (0x00FF8000)

            
/* SDIO COMMAND 52 Definitions */
#define CMD52_READ  0UL
#define CMD52_WRITE 1UL
#define CMD52_READ_AFTER_WRITE 1
#define CMD52_NORMAL_WRITE     0           

/* SDIO COMMAND 53 Definitions */
#define CMD53_READ          0UL
#define CMD53_WRITE         1UL
#define CMD53_BLOCK_BASIS   1 
#define CMD53_BYTE_BASIS    0
#define CMD53_FIXED_ADDRESS 0
#define CMD53_INCR_ADDRESS  1
#define CMD53_COUNT_MASK                    0x1ff
#define CMD53_REG_ADDR_OFFSET               9
#define CMD53_OP_CODE_OFFSET                26
#define CMD53_BLOCK_MODE_OFFSET             27
#define CMD53_FUN_NUM                       28
#define CMD53_RW_FLAG                       31

#define SDIO_SET_CMD52_ARG(arg,rw,func,raw,address,writedata) \
    (arg) = (((rw) & 1) << 31)           | \
            (((func) & 0x7) << 28)       | \
            (((raw) & 1) << 27)          | \
            (1 << 26)                    | \
            (((address) & 0x1FFFF) << 9) | \
            (1 << 8)                     | \
            ((writedata) & 0xFF)


#define SDIO_SET_CMD53_ARG(arg,rw,func,mode,opcode,address,bytes_blocks) \
    (arg) = (((rw) & 1) << CMD53_RW_FLAG)                  | \
            (((func) & 0x7) << CMD53_FUN_NUM)              | \
            (((mode) & 1) << CMD53_BLOCK_MODE_OFFSET)                | \
            (((opcode) & 1) << CMD53_OP_CODE_OFFSET)              | \
            (((address) & 0x1FFFF) << CMD53_REG_ADDR_OFFSET)        | \
            ((bytes_blocks) & CMD53_COUNT_MASK)

            
/* Card Common Control Registers (CCCR) */

#define CCCR_SDIO_REVISION                  0x00
#define CCCR_SD_SPECIFICATION_REVISION      0x01
#define CCCR_IO_ENABLE                      0x02
#define CCCR_IO_READY                       0x03
#define CCCR_INT_ENABLE                     0x04
#define CCCR_INT_PENDING                    0x05
#define CCCR_IO_ABORT                       0x06
#define CCCR_BUS_INTERFACE_CONTOROL         0x07
#define CCCR_CARD_CAPABILITY	               0x08
#define CCCR_COMMON_CIS_POINTER             0x09 /*0x09-0x0B*/
#define CCCR_FN0_BLOCK_SIZE	               0x10 /*0x10-0x11*/
#define FN0_CCCR_REG_32                     0x64


typedef enum _SDIO_STATUS
{
    SDIO_GET_OCR_VALID,
    SDIO_GET_OCR_FAIL,
    SDIO_GET_OCR_INVALID,
    SDIO_NO_FUN,
    SDIO_NEGO_SUCCESS,
    SDIO_NEGO_FAIL,
    SDIO_NEGO_TIMEOUT
}T_eSDIO_STATUS;


unsigned char init_io(bool bInitIo);


#ifdef __cplusplus
}
#endif

#endif
