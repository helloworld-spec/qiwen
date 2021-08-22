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
 * @return void
 */
void  MMU_Init(unsigned long mmutt_start_addr);

/**
 * @brief set section descriptor
 *
 * @author
 * @date
 * @param[in] vaddrStart start of virtual address
 * @param[in] vaddrEnd end of virtual address
 * @param[in] paddrStart end of physical address
 * @param[in] attr attribute of access
 * @return void
 */
void  MMU_SetMTT(unsigned long vaddrStart,unsigned long vaddrEnd,unsigned long paddrStart,unsigned long attr);

/**
 * @brief enable DCACHE
 *
 * @author
 * @date
 * @return void
 */
void  MMU_EnableDCache(void);

/**
 * @brief disable DCACHE
 *
 * @author
 * @date
 * @return void
 */
void  MMU_DisableDCache(void);

/**
 * @brief enable ICACHE
 *
 * @author
 * @date
 * @return void
 */
void  MMU_EnableICache(void);

/**
 * @brief disable ICACHE
 *
 * @author
 * @date
 * @return void
 */
void  MMU_DisableICache(void);

/**
 * @brief invalidate ICACHE
 *
 * data in ICACHE will be clear
 * @author
 * @date
 * @return void
 */
void  MMU_InvalidateICache(void);

/**
 * @brief invalidate DCACHE
 *
 * data in DCACHE will be clear
 * @author
 * @date
 * @return void
 */
void  MMU_InvalidateDCache(void);

/**
 * @brief
 *
 * @author
 * @date
 * @return void
 */
void  MMU_InvalidateIDCache(void);

/**
 * @brief enable MMU
 *
 * @author
 * @date
 * @return void
 */
void  MMU_EnableMMU(void);

/**
 * @brief disable MMU
 *
 * @author
 * @date
 * @return void
 */
void  MMU_DisableMMU(void);

/**
 * @brief invalidate TLB
 *
 * @author
 * @date
 * @return void
 */
void  MMU_InvalidateTLB(void);

/**
 * @brief clean and invalidate DCACHE
 *
 * @author
 * @date
 * @return void
 */
void  MMU_Clean_Invalidate_Dcache(void);

/**
 * @brief clean all DCACHE
 *
 * @author
 * @date
 * @return void
 */
void  MMU_Clean_All_DCache(void);

/**
 * @brief Drain Write buffer
 *
 * @author
 * @date
 * @return void
 */
void  MMU_DrainWriteBuffer(void);

#ifdef __cplusplus
}
#endif

/*@}*/
#endif /*__ARCH_MMU_H__*/

