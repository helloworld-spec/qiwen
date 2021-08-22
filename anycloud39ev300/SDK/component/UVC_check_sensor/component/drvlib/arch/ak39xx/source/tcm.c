/**
 * @file tcm.c
 * @brief Tightly coupled memory soucre file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-12-07
 * @version 1.0
 */

#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "tcm.h"

#define TCM_TYPE_BIT        (1 << 25)
#define TCM_MODE_BIT        (1 << 24)

#define TCM_SIZE    4096

/**
 * @brief enable tcm
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param mode DTCM or ITCM
 * @return void
 */
void tcm_enable(E_TCM_TYPE mode)
{
    //1. Select TCM type
    if(mode == MODE_DTCM)
        REG32(TCM_CTRL_REG) &= ~TCM_MODE_BIT;
    else
        REG32(TCM_CTRL_REG) |= TCM_MODE_BIT;

    //2. select image fifo as TCM
    REG32(TCM_CTRL_REG) |= TCM_TYPE_BIT;

    //3. allocate a block of memory with the size of TCM

    //4. configure system control register
}

/**
 * @brief disable tcm
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param
 * @return void
 */
void tcm_disable()
{
    //1. configure system control register

    //2. select image fifo used by camera interface
    REG32(TCM_CTRL_REG) &= ~TCM_TYPE_BIT;
}


