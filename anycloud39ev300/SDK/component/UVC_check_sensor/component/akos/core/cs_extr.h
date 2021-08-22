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
/*      cs_extr.h                                      Nucleus PLUS 1.14 */
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
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */
/*                      added inline capability for                      */
/*                      linked-list functions,                           */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      02-02-1998      Corrected SPR347 where NU_INLINE                 */
/*                      created a linker error because                   */
/*                      CSC_Priority_Place_On_List was                   */
/*                      "extern"d instead of "VOID"d                     */
/*                      resulting in version 1.2a                        */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
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





