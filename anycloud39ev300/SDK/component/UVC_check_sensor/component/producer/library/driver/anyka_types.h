/**@file anyka_types.h
 * @brief Define the types for system
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2016-10-09
 * @version 1.0
 */


#ifndef _ANYKA_TYPES_H_
#define _ANYKA_TYPES_H_

/** @defgroup ANYKA_TYPES Anyka_type group
 *  @ingroup M3PLATFORM
 */
/*@{*/
#include <stdio.h>

#define T_U8_MAX              ((unsigned char)0xff)                        // maximum unsigned char value
#define T_U16_MAX             ((unsigned short)0xffff)                     // maximum unsigned short value
#define T_U32_MAX             ((unsigned long)0xffffffff)                 // maximum unsigned long value
#define T_U64_MAX             ((unsigned long)0xffffffffffffffff)         // maximum T_U64 value
#define T_S8_MIN              ((signed char)(-127-1))                    // minimum signed char value
#define T_S8_MAX              ((signed char)127)                         // maximum signed char value
#define T_S16_MIN             ((signed short)(-32767L-1L))               // minimum signed short value
#define T_S16_MAX             ((signed short)(32767L))                   // maximum signed short value
#define T_S32_MIN             ((signed long)(-2147483647L-1L))          // minimum signed long value
#define T_S32_MAX             ((signed long)(2147483647L))              // maximum signed long value
#define T_S64_MIN             ((signed long)(-9223372036854775807L-1L)) // minimum T_S64 value
#define T_S64_MAX             ((signed long)(9223372036854775807L))     // maximum T_S64 value

/* basal type definition for global area */
//typedef unsigned char                    bool;     /* BOOL type */

//#ifdef __CC_ARM
typedef unsigned char bool;
#define true 1
#define false 0
//#endif



typedef struct {
    signed short       x;
    signed short       y;
}T_POINT, *T_pPOINT;

typedef struct {
    signed short   left;
    signed short   top;
    signed short   width;
    signed short   height;
} T_RECT, *T_pRECT;

typedef struct{
    unsigned long low;
    unsigned long high;
} T_U64_INT;

//typedef unsigned char unsigned char;


/*@}*/

#endif  // _ANYKA_TYPES_H_

