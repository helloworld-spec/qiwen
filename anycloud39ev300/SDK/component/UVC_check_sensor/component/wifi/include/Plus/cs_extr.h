/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
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
/*      cs_extr.h                                       PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      CS -    Common Services                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes of all functions          */
/*      accessible to other components.                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_defs.h                           Common service definitions   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Moved include files outside of   */
/*                                        the file #ifndef to allow the  */
/*                                        use of actual data structures, */
/*                                        added inline capability for    */
/*                                        linked-list functions,         */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       02-02-1998      Corrected SPR347 where NU_INLINE */
/*                                        created a linker error because */
/*                                        CSC_Priority_Place_On_List was */
/*                                        "extern"d instead of "VOID"d   */
/*                                        resulting in version 1.2a      */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/

#include        "cs_defs.h"                 /* Include CS definitions    */


/* Check to see if the file has been included already.  */

#ifndef CS_EXTR
#define CS_EXTR

#ifndef NU_INLINE
VOID            CSC_Place_On_List(CS_NODE **head, CS_NODE *new_node);
VOID            CSC_Priority_Place_On_List(CS_NODE **head, CS_NODE *new_node);
VOID            CSC_Remove_From_List(CS_NODE **head, CS_NODE *node);
#else
#define         CSC_Place_On_List(head, new_node);                           \
                if (*((CS_NODE **) (head)))                                  \
                {                                                            \
                    ((CS_NODE *) (new_node)) -> cs_previous=                 \
                            (*((CS_NODE **) (head))) -> cs_previous;         \
                    (((CS_NODE *) (new_node)) -> cs_previous) -> cs_next =   \
                            (CS_NODE *) (new_node);                          \
                    ((CS_NODE *) (new_node)) -> cs_next =                    \
                            (*((CS_NODE **) (head)));                        \
                    (((CS_NODE *) (new_node)) -> cs_next) -> cs_previous =   \
                            ((CS_NODE *) (new_node));                        \
                }                                                            \
                else                                                         \
                {                                                            \
                    (*((CS_NODE **) (head))) = ((CS_NODE *) (new_node));     \
                    ((CS_NODE *) (new_node)) -> cs_previous =                \
                            ((CS_NODE *) (new_node));                        \
                    ((CS_NODE *) (new_node)) -> cs_next =                    \
                            ((CS_NODE *) (new_node));                        \
                }     

VOID            CSC_Priority_Place_On_List(CS_NODE **head, CS_NODE *new_node);

#define         CSC_Remove_From_List(head, node);                            \
                if (((CS_NODE *) (node)) -> cs_previous ==                   \
                                            ((CS_NODE *) (node)))            \
                {                                                            \
                    (*((CS_NODE **) (head))) =  NU_NULL;                     \
                }                                                            \
                else                                                         \
                {                                                            \
                    (((CS_NODE *) (node)) -> cs_previous) -> cs_next =       \
                                         ((CS_NODE *) (node)) -> cs_next;    \
                    (((CS_NODE *) (node)) -> cs_next) -> cs_previous =       \
                                     ((CS_NODE *) (node)) -> cs_previous;    \
                    if (((CS_NODE *) (node)) == *((CS_NODE **) (head)))      \
                        *((CS_NODE **) (head)) =                             \
                            ((CS_NODE *) (node)) -> cs_next;                 \
                }                                                        
#endif

#endif
