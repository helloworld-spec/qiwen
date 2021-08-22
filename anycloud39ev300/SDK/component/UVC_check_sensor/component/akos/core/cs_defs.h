/*************************************************************************/
/*                                                                       */
/*               Copyright Mentor Graphics Corporation 2002              */
/*                         All Rights Reserved.                          */
/*                                                                       */
/* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS  */
/* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS   */
/* SUBJECT TO LICENSE TERMS.                                             */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                               VERSION       */
/*                                                                       */
/*      cs_defs.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      CS -    Common Services                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions used in the common */
/*      service linked list routines.                                    */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      CS_NODE                             Link node structure          */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      nucleus.h                           Nucleus PLUS constants       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      06-01-1993      Added padding conditional into                   */
/*                      CS_NODE structure, making                        */
/*                      version 1.0a                                     */
/*      06-01-1993      Verified version 1.0a                            */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#include        "nucleus.h"                 /* Include Nucleus constants */


/* Check to see if the file has been included already.  */
#ifndef CS_DEFS
#define CS_DEFS


/* Define a common node data structure that can be included inside of
   other system data structures.  */

typedef struct  CS_NODE_STRUCT
{
    struct CS_NODE_STRUCT  *cs_previous;
    struct CS_NODE_STRUCT  *cs_next;
    DATA_ELEMENT            cs_priority;

#if     PAD_1
    DATA_ELEMENT            cs_padding[PAD_1];
#endif

}  CS_NODE;

#endif /* CS_DEFS */



