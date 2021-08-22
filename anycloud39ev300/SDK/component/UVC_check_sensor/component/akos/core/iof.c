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
/*      iof.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IO - Input/Output Driver Management                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains routines to obtain facts about the I/O Driver */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      IOF_Established_Drivers             Return total number of I/O   */
/*                                            drivers                    */
/*      IOF_Driver_Pointers                 Return list of I/O driver    */
/*                                            pointers                   */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      io_extr.h                           I/O driver functions         */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1994      Initial version of I/O driver                    */
/*                      fact service file, version 1.1                   */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "io_extr.h"                 /* I/O driver functions      */
#include        "hi_extr.h"                 /* History functions         */

/* Define external inner-component global data references.  */

extern CS_NODE         *IOD_Created_Drivers_List;
extern UNSIGNED         IOD_Total_Drivers;
extern TC_PROTECT       IOD_List_Protect;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOF_Established_Drivers                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established I/O      */
/*      drivers.  I/O drivers previously deleted are no longer           */
/*      considered established.                                          */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IOD_Total_Drivers                   Number of established I/O    */
/*                                            drivers                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  IOF_Established_Drivers(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established I/O drivers.  */
    return(IOD_Total_Drivers);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOF_Driver_Pointers                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of driver  pointers, starting at the */
/*      specified location.  The number of driver  pointers placed in    */
/*      the list is equivalent to the total number of drivers or the     */
/*      maximum number of pointers specified in the call.                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pointer_list                        Pointer to the list area     */
/*      maximum_pointers                    Maximum number of pointers   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      pointers                            Number of I/O driver pointers*/
/*                                            placed in the list         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0a                  */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Changed function interface,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  IOF_Driver_Pointers(NU_DRIVER **pointer_list,
                                                UNSIGNED maximum_pointers)
{
CS_NODE         *node_ptr;                  /* Pointer to each NU_DRIVER */
UNSIGNED         pointers;                  /* Number of pointers in list*/
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Initialize the number of pointers returned.  */
    pointers =  0;

    /* Protect against access to the list of created I/O drivers.  */
    TCT_Protect(&IOD_List_Protect);

    /* Loop until all driver  pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  IOD_Created_Drivers_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_DRIVER *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == IOD_Created_Drivers_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection against access to the list of created drivers.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the number of pointers in the list.  */
    return(pointers);
}






