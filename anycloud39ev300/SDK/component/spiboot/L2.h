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

#include "anyka_types.h"
#include "spi.h"

T_VOID l2_init(T_VOID);


T_BOOL dma_init(T_VOID);


void force_cs(T_U8 spi_id);

void unforce_cs(T_U8 spi_id);


void set_tx(T_U8 spi_id);

void set_rx(T_U8 spi_id);

T_U8 l2_get_status(T_U8 buf_id);

T_VOID l2_combuf_cpu(T_U32 ram_addr, T_U8 buf_id, T_U32 tran_byte, T_U8 tran_dir);

T_VOID l2_combuf_dma(T_U32 ram_addr, T_U8 buf_id, T_U32 tran_byte, T_U8 tran_dir, T_BOOL bIntr);

T_VOID l2_clr_status(T_U8 buf_id);

T_BOOL l2_combuf_wait_dma_finish(T_U8 buf_id);

T_U8 l2_alloc(DEVICE_SELECT dev_slct);

T_VOID l2_free(DEVICE_SELECT dev_slct);

#endif

