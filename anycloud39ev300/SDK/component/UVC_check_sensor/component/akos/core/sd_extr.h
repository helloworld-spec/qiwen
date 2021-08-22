/*************************************************************************
*                                                                       
*               Copyright Mentor Graphics Corporation 2002              
*                         All Rights Reserved.                          
*                                                                       
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS  
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS   
* SUBJECT TO LICENSE TERMS.                                             
*                                                                       
*************************************************************************/

/*************************************************************************
*                                                                       
* FILE NAME                                VERSION                        
*                                                                       
*      sd_extr.h                 Nucleus PLUS\ARM925\Code Composer 1.14.1 
*                                                                       
* COMPONENT                                                             
*                                                                       
*      SD - Serial Driver                                               
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*      This file contains function prototypes for the Serial Driver     
*  module.                                                              
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*      none                                                             
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*      sd_defs.h   
*
* HISTORY                                                               
*                                                                       
*         NAME            DATE                    REMARKS             
*  
*      B. Ronquillo     08-28-2002           Released version 1.14.1     
*************************************************************************/
#ifndef SD_EXTR
#define SD_EXTR

#include "sd_defs.h"

/* SDC function prototypes */
STATUS  SDC_Init_Port(SD_PORT *);

/* Fixed SPR 249.  Changed the first parameter of SDC_Put_Char from UNSIGNED_CHAR 
   to CHAR to match function */
VOID    SDC_Put_Char(UINT8, SD_PORT *);

VOID    SDC_Put_String(CHAR *, SD_PORT *);
CHAR    SDC_Get_Char(SD_PORT *);
STATUS  SDC_Data_Ready(SD_PORT *);
VOID    SDC_LISR(INT vector);
#endif

