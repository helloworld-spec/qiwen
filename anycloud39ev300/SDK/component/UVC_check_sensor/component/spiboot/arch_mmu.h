/**
 * @file arch_mmu.h
 * @brief This file describe the interface of mmu module
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author YiRuoxiang
 * @date 2005-07-13
 * @version 1.0
 */
#ifndef __ARCH_MMU_H__
#define __ARCH_MMU_H__

/** @defgroup MMU MMU group
 *  @ingroup Drv_Lib
 */
/*@{*/
#ifdef __cplusplus
extern "C" {
#endif

#define DESC_SEC        (0x2|(1<<4))
#define MMU_CB          (3<<2)  //cache_on, write_back
#define MMU_CNB         (2<<2)  //cache_on, write_through 
#define MMU_NCB         (1<<2)  //cache_off,WR_BUF on
#define MMU_NCNB        (0<<2)  //cache_off,WR_BUF off
#define AP_RW           (3<<10) //supervisor=RW, user=RW
#define AP_RO           (2<<10) //supervisor=RW, user=RO

#define DOMAIN_FAULT    (0x0)
#define DOMAIN_CHK      (0x1) 
#define DOMAIN_NOTCHK   (0x3) 
#define DOMAIN0         (0x0<<5)
#define DOMAIN1         (0x1<<5)

#define DOMAIN0_ATTR    (DOMAIN_CHK<<0) 
#define DOMAIN1_ATTR    (DOMAIN_FAULT<<2) 

#define RW_CB           (AP_RW|DOMAIN0|MMU_CB|DESC_SEC)
#define RW_CNB          (AP_RW|DOMAIN0|MMU_CNB|DESC_SEC)
#define RW_NCNB         (AP_RW|DOMAIN0|MMU_NCNB|DESC_SEC)
#define RW_FAULT        (AP_RW|DOMAIN1|MMU_NCNB|DESC_SEC)   

/**
 * @brief initial MMU
 *
 * 1,set access attribute. 2,enable ICACHE and DCACHE. 3,enable MMU. 4,set TT(translation table) start address
 * @author
 * @date
 * @param[in] mmutt_start_addr MMU translation table's start address
 * @return T_VOID
 */
T_VOID  MMU_Init(T_U32 mmutt_start_addr);

/**
 * @brief set section descriptor
 *
 * @author
 * @date
 * @param[in] vaddrStart start of virtual address
 * @param[in] vaddrEnd end of virtual address
 * @param[in] paddrStart end of physical address
 * @param[in] attr attribute of access
 * @return T_VOID
 */
T_VOID  MMU_SetMTT(T_U32 vaddrStart,T_U32 vaddrEnd,T_U32 paddrStart,T_U32 attr);

/**
 * @brief enable DCACHE
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_EnableDCache(T_VOID);

/**
 * @brief disable DCACHE
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_DisableDCache(T_VOID);

/**
 * @brief enable ICACHE
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_EnableICache(T_VOID);

/**
 * @brief disable ICACHE
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_DisableICache(T_VOID);

/**
 * @brief invalidate ICACHE
 *
 * data in ICACHE will be clear
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_InvalidateICache(T_VOID);

/**
 * @brief invalidate DCACHE
 *
 * data in DCACHE will be clear
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_InvalidateDCache(T_VOID);

/**
 * @brief
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_InvalidateIDCache(T_VOID);

/**
 * @brief enable MMU
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_EnableMMU(T_VOID);

/**
 * @brief disable MMU
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_DisableMMU(T_VOID);

/**
 * @brief invalidate TLB
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_InvalidateTLB(T_VOID);

/**
 * @brief clean and invalidate DCACHE
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_Clean_Invalidate_Dcache(T_VOID);

/**
 * @brief clean all DCACHE
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_Clean_All_DCache(T_VOID);

/**
 * @brief Drain Write buffer
 *
 * @author
 * @date
 * @return T_VOID
 */
T_VOID  MMU_DrainWriteBuffer(T_VOID);

#ifdef __cplusplus
}
#endif

/*@}*/
#endif /*__ARCH_MMU_H__*/

