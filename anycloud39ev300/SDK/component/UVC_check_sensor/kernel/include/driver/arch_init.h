/**@file Arch_init.h
 * @brief list driver library initialize operations
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2008-02-1
 * @version 1.0
 * @note refer to ANYKA chip technical manual.
 */
#ifndef __ARCH_INIT_H__
#define __ARCH_INIT_H__

#include "anyka_types.h"

/** @defgroup Init Init group
 *  @ingroup Drv_Lib
 */
/*@{*/

/*
 * Used for initialization calls..
 */
typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);


#ifdef __GNUC__
#define __initcall(fn) \
initcall_t __initcall_##fn \
__attribute__((__section__(".initcall"))) = fn;
#endif

#ifdef __CC_ARM
#define __initcall(fn) \
initcall_t __initcall_##fn  = fn;
#endif

/**
 *  module init
 */
#define module_init(x)  __initcall(x)

/**
 *  memory alloc callback handler
 */
typedef void * (*T_RamAlloc)(unsigned long size, signed char *filename, unsigned long fileline);

/**
 *  memory free callback handler
 */
typedef void * (*T_RamFree)(void * var, signed char *filename, unsigned long fileline);

/** @brief chip name
 * define all chip supported
 */
typedef enum
{
    CHIP_3918E   = 0x3918,      ///< AK3918E

    CHIP_RESERVE = 0xffff       ///< reserve
}
E_AKCHIP_TYPE;

typedef enum
{
    CHIP_V100 = 0x100,
    CHIP_V200 = 0x200,
}
E_AKCHIP_INFO;

/** @brief driver init info
 * define chip type and memory callback 
 */
typedef struct tag_DRIVE_INITINFO
{
    E_AKCHIP_TYPE chip;     ///< chip type
    
    T_RamAlloc fRamAlloc;   ///< memory alloc callback function
    T_RamFree  fRamFree;     ///<memory free callback function
}
T_DRIVE_INITINFO, *T_PDRIVE_INITINFO;

/**
 * @brief driver library initialization
 *
 * should be called on start-up step, to initial interrupt module and register hardware as camera, lcd...etc.
 * @author xuchang
 * @date 2008-01-21
 * @return void
 */
void drv_init(T_PDRIVE_INITINFO drv_info);

/**
 * @brief memory alloc
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param size unsigned long: size of memory to alloc
 * @return void *
 */
void * drv_malloc(unsigned long size);

/**
 * @brief drv_get_chip
 *
 * @author LHD
 * @date 2012-03-06
 * @return E_AKCHIP_INFO
 */
E_AKCHIP_INFO drv_get_chip_version(void);

/**
 * @brief memory free
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param var void *: address of memory to free
 * @return void *
 */
void * drv_free(void * var);

/**
 * @brief get chip type
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @return void
 */
E_AKCHIP_TYPE drv_get_chip_type(void);

/**
 * @brief check current chip is the same series or not
 * @author xuchang
 * @date 2010-12-14
 * @param[in] chip_type chip type
 * @return bool
 * @retval if same series, return true
 */
bool drv_check_series_chip(E_AKCHIP_TYPE chip_type);
 
/**
 * @brief get dram capacity
 *
 * @author liao_zhijun
 * @date 2010-11-08
 * @return unsigned long ram size, uint (byte)
 */
unsigned long drv_get_ram_size(void);


/*@}*/
#endif //__ARCH_INIT_H__

