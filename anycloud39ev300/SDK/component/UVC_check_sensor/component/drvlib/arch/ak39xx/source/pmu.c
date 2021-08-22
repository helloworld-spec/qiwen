/**
 * @file pmu.c
 * @brief power manage unit soucr file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */

#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "arch_pmu.h"

#define DCDC12_ENABLE_BIT           (1<<25)
#define LDO12_ENABLE_BIT            (1<<16)
#define USB_DET_N_CTRL              (1<<24)
#define LDO12_SEL_BIT               (1<<15)

#define LDOPLL_CTRL_BIT             (1<<23)

/**
 * @brief set output voltage of ldo33 
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param vol voltage to be set
 * @return void
 */
void pmu_set_ldo33(E_LDO33_VOL vol)
{
    unsigned long reg;

    if(vol == LDO33_RESERVER)
        return;

    irq_mask();

    REG32(PMU_CTRL_REG) &= ~(0x7 << 17);
    REG32(PMU_CTRL_REG) |= (vol << 17);

    irq_unmask();
}

/**
 * @brief set output voltage of ldo12
              dcdc12 will be disabled according to chip spec
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param vol voltage to be set
 * @return void
 */
void pmu_set_ldo12(E_LDO12_VOL vol)
{
    unsigned long reg;

    irq_mask();

    //enable ldo12
    REG32(PMU_CTRL_REG) |= LDO12_ENABLE_BIT;

    //disable dcdc12
    REG32(PMU_CTRL_REG) &= ~DCDC12_ENABLE_BIT;

    //reset usb detect signal
    REG32(PMU_CTRL_REG) |= USB_DET_N_CTRL;
    REG32(PMU_CTRL_REG) &= ~USB_DET_N_CTRL;
    
    //set ldo12
    REG32(PMU_CTRL_REG) &= ~(0x7<<12);
    REG32(PMU_CTRL_REG) |= (vol<<12);

    irq_unmask();

}

/**
 * @brief set output voltage of dcdc12
              ldo12 will be disabled according to chip spec
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param vol voltage to be set
 * @return void
 */
void pmu_set_dcdc12(E_DCDC12_VOL vol)
{
    unsigned long reg;

    irq_mask();

    //enable DCDC12
    REG32(PMU_CTRL_REG) |= DCDC12_ENABLE_BIT;
    
    //disable ldo12
    REG32(PMU_CTRL_REG) &= ~LDO12_ENABLE_BIT;

    //reset usb detect signal
    REG32(PMU_CTRL_REG) |= USB_DET_N_CTRL;
    REG32(PMU_CTRL_REG) &= ~USB_DET_N_CTRL;

    //set ldo12
    REG32(PMU_CTRL_REG) &= ~(0x7<<6);
    REG32(PMU_CTRL_REG) |= (vol<<6);

    irq_unmask();

}

/**
 * @brief set output voltage of ldopll 
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param vol voltage to be set
 * @return void
 */
void pmu_set_ldopll(E_LDOPLL_VOL vol)
{
    unsigned long reg;
    
    irq_mask();

    if(vol == LDOPLL_125V)
        REG32(PMU_CTRL_REG) &= ~LDOPLL_CTRL_BIT;
    else
        REG32(PMU_CTRL_REG) |= LDOPLL_CTRL_BIT;

    irq_unmask();
}


