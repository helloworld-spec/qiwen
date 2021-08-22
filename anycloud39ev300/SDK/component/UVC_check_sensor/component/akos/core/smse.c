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
/*      smse.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      SM - Semaphore Management                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the error checking routines for the           */
/*      supplemental functions of the Semaphore component.  This permits */
/*      easy removal of error checking logic when it is not needed.      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      SMSE_Reset_Semaphore                Reset a semaphore            */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      sm_extr.h                           Semaphore functions          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Created initial version 1.1 from                 */
/*                      routines originally in core                      */
/*                      error checking file                              */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "sm_extr.h"                 /* Semaphore functions       */



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMSE_Reset_Semaphore                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the reset semaphore function.                                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      SMS_Reset_Semaphore                 Actual reset semaphore func. */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Semaphore control block ptr  */
/*      initial_count                       Initial count to reset the   */
/*                                            semaphore to               */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_SEMAPHORE                Invalid semaphore pointer    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMSE_Reset_Semaphore(NU_SEMAPHORE *semaphore_ptr, 
                                                    UNSIGNED initial_count)
{

SM_SCB         *semaphore;                  /* Semaphore control blk ptr */
STATUS          status;                     /* Completion status         */


    /* Move input semaphore pointer into internal pointer.  */
    semaphore =  (SM_SCB *) semaphore_ptr;

    /* Determine if the semaphore pointer is valid.  */
    if ((semaphore) && (semaphore -> sm_id == SM_SEMAPHORE_ID))
    
        /* Semaphore pointer is valid, call function to reset it.  */
        status =  SMS_Reset_Semaphore(semaphore_ptr, initial_count);
        
    else
    
        /* Semaphore pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_SEMAPHORE;
        
    /* Return completion status.  */
    return(status);
}




