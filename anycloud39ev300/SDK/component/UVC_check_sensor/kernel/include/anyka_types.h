/**@file anyka_types.h
 * @brief Define the types for system
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */


#ifndef _ANYKA_TYPES_H_
#define _ANYKA_TYPES_H_

/** @defgroup ANYKA_TYPES Anyka_type group
 *  @ingroup M3PLATFORM
 */
/*@{*/

//wmj+ to fix compile error
/*
#ifndef TYPE_BOOL_DEFINE
#define TYPE_BOOL_DEFINE
typedef unsigned char bool;
#endif
*/
#include <stdbool.h>

#if 0
#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif 

#ifndef TRUE
#define TRUE 1 //TODO
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif

#ifndef NULL
#define NULL                   ((void *)0)
#endif

#endif  // _ANYKA_TYPES_H_

