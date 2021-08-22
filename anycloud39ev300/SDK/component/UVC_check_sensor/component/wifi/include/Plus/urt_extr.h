/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1999-2000  Accelerated Technology, Inc.          */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      urt_defs.h                                 PLUS/SNDS100 1.11.3   */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      UART                                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes for the UART module.      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Barry Sellew,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*     NAME            DATE                    REMARKS                   */
/*                                                                       */
/*    Bobby Iden     12-21-99   Created initial version 1.11.1 for the   */
/*                              the Samsung SNDS100 with the KS32C50100. */ 
/*    D. Phillips   01-18-2000  Updated port to new structuring          */
/*                               scheme                                  */
/*    D. Phillips   03-26-2000  Released version 1.11.3                  */
/*                                                                       */
/*************************************************************************/
#ifndef URT_EXTR
#define URT_EXTR

#include "urt_defs.h"

/* UART function prototypes */

STATUS  UART_Init_Port(UART_INIT *uart_init);
VOID    UART_Put_Char(INT first,CHAR ch);
VOID    UART_Put_String(CHAR *str);
CHAR    UART_Get_Char(VOID);
VOID    UART_LISR(VOID);
VOID    UART_Set_Baud_Rate(UNSIGNED baud_rate);
STATUS  UART_Data_Ready(VOID);

#endif
