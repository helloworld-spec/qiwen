/**
 * @FILENAME: init.c
 * @BRIEF init module
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR xuchang
 * @DATE 2007.10.23
 * @VERSION 1.0
 * @REF
 */
#include "arch_init.h"
#include "l2.h"
#include "interrupt.h"
#include "drv_api.h"

#ifdef __GNUC__
extern int __initcall_start, __initcall_end;
static int *initcall_start=&__initcall_start, *initcall_end=&__initcall_end;
#endif
#ifdef __CC_ARM
extern int Image$$init$$Base, Image$$init$$Limit;
static int *initcall_start=&Image$$init$$Base, *initcall_end=&Image$$init$$Limit;
#endif

static T_DRIVE_INITINFO g_drv_info = {0};

void do_initcalls(void)
{
    initcall_t *call;
    int *addr;

    akprintf(C3, M_DRVSYS, "initcall(): start=0x%x, end=0x%x\n", initcall_start, initcall_end); 
    for (addr = initcall_start; addr < initcall_end; addr++) 
    {
        call = (initcall_t *)addr;
        (*call)();    
    }
}

void drv_init(T_PDRIVE_INITINFO drv_info)
{
    unsigned long reg_value;
    
    memcpy(&g_drv_info, drv_info, sizeof(g_drv_info));
    
    //release reset all module
    REG32(RESET_CTRL_REG) &= 0x00000000;

    /*L2 init*/
    l2_init();
    /* interrupt init */
    interrupt_init();
    /* device module registeration init */
    do_initcalls();
    /* set default arm dma priority, arm can not break dma */
    //dma_init();
    
    //close voice wakeup
    //REG32(ANALOG_CTRL_REG4) |= (1 << 15);      //power off
    //REG32(USB_DETECT_CTRL_REG) &= ~(1 << 5);   //disable voice wakeup

    DrvModule_Init();
}

/**
 * @brief memory alloc
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param size unsigned long: size of memory to alloc
 * @return void *
 */
void * drv_malloc(unsigned long size)
{
    if(NULL == g_drv_info.fRamAlloc)
        return NULL;

    return g_drv_info.fRamAlloc((size), ((signed char*)(__FILE__)), ((unsigned long)__LINE__));
}

/**
 * @brief memory free
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param var void *: address of memory to free
 * @return void *
 */
void * drv_free(void * var) 
{
    if(NULL == g_drv_info.fRamFree)
        return NULL;

    return g_drv_info.fRamFree(var, ((signed char*)(__FILE__)), ((unsigned long)__LINE__));
}  

/**
 * @brief get chip type
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @return void
 */
E_AKCHIP_TYPE drv_get_chip_type()
{
    return g_drv_info.chip;
}

/**
 * @brief check current chip is the same series or not
 * @autor xuchang
 * @date 2010-12-14
 * @param[in] chip type
 * @return bool
 * @retval if same series, return true
 */
bool drv_check_series_chip(E_AKCHIP_TYPE chip_type)
{
    if ((chip_type>>8) == (g_drv_info.chip>>8))
        return true;
    else
        return false;
}

/**
 * @brief get dram type
 *
 * @author liao_zhijun
 * @date 2010-11-08
 * @return unsigned long ram size, uint (byte)
 */
unsigned long drv_get_ram_size()
{
    unsigned long reg, size;

    reg = REG32(SDRAM_CFG_REG2);

    size = (1 << ((reg & 0x7) + 1));

    return (size << 20);
}

/**
 * @brief drv_get_chip_version
 *
 * @author 
 * @date 2012-03-06
 * @return E_AKCHIP_INFO
 */
E_AKCHIP_INFO drv_get_chip_version()
{
    //TODO    
    return CHIP_V200;
}


