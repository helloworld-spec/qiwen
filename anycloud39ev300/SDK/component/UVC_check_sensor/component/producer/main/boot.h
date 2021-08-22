/** @file
 * @brief Boot up code
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */


#ifndef __BOOT_H__
#define __BOOT_H__




/** @defgroup BootUp  
 *	@ingroup M3PLATFORM
 */
/*@{*/

/**CPU work mode */
#define ANYKA_CPU_Mode_USR		0x10
#define ANYKA_CPU_Mode_FIQ		0x11
#define ANYKA_CPU_Mode_IRQ		0x12
#define ANYKA_CPU_Mode_SVC		0x13
#define ANYKA_CPU_Mode_ABT		0x17
#define ANYKA_CPU_Mode_UNDEF	0x1B
#define ANYKA_CPU_Mode_SYS		0x1F		
#define ANYKA_CPU_I_Bit			0x80
#define ANYKA_CPU_F_Bit			0x40

/*@}*/

#endif


