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
/*      er_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      ER - Error Management                                            */
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
/* MACROS                                                                */
/*                                                                       */
/*      NU_CHECK                                                         */
/*      NU_ASSERT                                                        */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      nucleus.h                           System definitions           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*          DATE                    REMARKS                              */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      11-24-1998      Added NU_CHECK and NU_ASSERT.                    */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#include        "nucleus.h"                 /* Include system definitions */


/* Check to see if the file has been included already.  */

#ifndef ER_EXTR
#define ER_EXTR


/*  Initialization function.  */

VOID            ERI_Initialize(VOID);


/* System error handling function definition.  */

VOID            ERC_System_Error(INT error_code);

#ifdef NU_DEBUG_MEMORY

STATUS ERC_Allocate_Memory(NU_MEMORY_POOL *pool_ptr, VOID **ptr,
                           UNSIGNED size, UNSIGNED suspend, 
                           unsigned long line, const char* file);

STATUS ERC_Deallocate_Memory(VOID *ptr);

#endif /* NU_DEBUG_MEMORY */

#ifdef NU_DEBUG

void ERC_Assert(CHAR *test, CHAR *name, UNSIGNED line);

#endif


#ifdef NU_ASSERT
#undef NU_ASSERT
#endif


#ifdef NU_CHECK
#undef NU_CHECK
#endif


#ifdef NU_DEBUG
  #define NU_ASSERT( test ) \
    if ( !(test) ) ERC_Assert( #test, __FILE__, __LINE__ );  ((void) 0)
#else
  #define NU_ASSERT( test ) ((void) 0)
#endif /* NU_DEBUG */


#ifdef NU_DEBUG
  #define NU_ASSERT2( test ) \
    if ( !(test) ) ERC_Assert( #test, __FILE__, __LINE__ );  ((void) 0)
#else
  #define NU_ASSERT2( test ) ((void) 0)
#endif /* NU_DEBUG */


#ifndef NU_NO_ERROR_CHECKING
  #define NU_CHECK( test, statement ) \
    NU_ASSERT2( test );  if ( !(test) ) { statement; }  ((void) 0)
#else
  #define NU_CHECK( test, statement ) NU_ASSERT2( test )
#endif /* NU_NO_ERROR_CHECKING */



#endif





