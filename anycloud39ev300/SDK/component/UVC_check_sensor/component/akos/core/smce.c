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
/*      smce.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      SM - Semaphore Management                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the error checking routines for the core      */
/*      functions of the Semaphore component.  This permits easy removal */
/*      of error checking logic when it is not needed.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      SMCE_Create_Semaphore               Create a semaphore           */
/*      SMCE_Delete_Semaphore               Delete a semaphore           */
/*      SMCE_Obtain_Semaphore               Obtain an instance of a      */
/*                                            semaphore                  */
/*      SMCE_Release_Semaphore              Release an instance of a     */
/*                                            semaphore                  */
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
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      05-15-1993      Corrected comment problem in                     */
/*                        the header, making the new                     */
/*                        version 1.0a                                   */
/*      05-15-1993      Verified version 1.0a                            */
/*      03-01-1994      Split original error checking                    */
/*                        file and changed function                      */
/*                        interfaces, resulting in                       */
/*                        version 1.1                                    */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
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
/*      SMCE_Create_Semaphore                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the create semaphore function.                                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      SMC_Create_Semaphore                Actual semaphore create func.*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Semaphore control block ptr  */
/*      name                                Semaphore name               */
/*      initial_count                       Initial semaphore instance   */
/*                                            count                      */
/*      suspend_type                        Suspension type              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_SEMAPHORE                Semaphore control block ptr  */
/*                                            is NULL                    */
/*      NU_INVALID_SUSPEND                  Semaphore suspension is      */
/*                                            invalid                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMCE_Create_Semaphore(NU_SEMAPHORE *semaphore_ptr, CHAR *name, 
                           UNSIGNED initial_count, OPTION suspend_type)
{

SM_SCB         *semaphore;                  /* Semaphore control blk ptr */
STATUS          status;                     /* Completion status         */


    /* Move input semaphore pointer into internal pointer.  */
    semaphore =  (SM_SCB *) semaphore_ptr;

    /* Check for a NULL semaphore pointer or an already created semaphore.  */
    if ((semaphore == NU_NULL) || (semaphore -> sm_id == SM_SEMAPHORE_ID))
    
        /* Invalid semaphore control block pointer.  */
        status =  NU_INVALID_SEMAPHORE;
        
    else if ((suspend_type != NU_FIFO) && (suspend_type != NU_PRIORITY))
    
        /* Invalid suspension type.  */
        status =  NU_INVALID_SUSPEND;
        
    else
    
        /* Call the actual service to create the semaphore.  */
        status =  SMC_Create_Semaphore(semaphore_ptr, name, initial_count,
                                                        suspend_type);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMCE_Delete_Semaphore                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the delete semaphore function.                                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      SMC_Delete_Semaphore                Actual delete semaphore func.*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Semaphore control block ptr  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_SEMAPHORE                Invalid semaphore pointer    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMCE_Delete_Semaphore(NU_SEMAPHORE *semaphore_ptr)
{

SM_SCB         *semaphore;                  /* Semaphore control blk ptr */
STATUS          status;                     /* Completion status         */


    /* Move input semaphore pointer into internal pointer.  */
    semaphore =  (SM_SCB *) semaphore_ptr;

    /* Determine if the semaphore pointer is valid.  */
    if ((semaphore) && (semaphore -> sm_id == SM_SEMAPHORE_ID))
    
        /* Semaphore pointer is valid, call function to delete it.  */
        status =  SMC_Delete_Semaphore(semaphore_ptr);
        
    else
    
        /* Semaphore pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_SEMAPHORE;
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMCE_Obtain_Semaphore                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the obtain semaphore function.                                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      SMC_Obtain_Semaphore                Actual function to obtain an */
/*                                            instance of the semaphore  */
/*      TCCE_Suspend_Error                  Check for illegal suspend    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Semaphore control block ptr  */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_SEMAPHORE                Invalid semaphore pointer    */
/*      NU_INVALID_SUSPEND                  Suspension from non-task     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMCE_Obtain_Semaphore(NU_SEMAPHORE *semaphore_ptr, UNSIGNED suspend)
{

SM_SCB         *semaphore;                  /* Semaphore control blk ptr */
STATUS          status;                     /* Completion status         */


    /* Move input semaphore pointer into internal pointer.  */
    semaphore =  (SM_SCB *) semaphore_ptr;

    /* Determine if semaphore pointer is invalid.  */
    if (semaphore == NU_NULL)
    
        /* Semaphore pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_SEMAPHORE;

    else if (semaphore -> sm_id != SM_SEMAPHORE_ID)
    
        /* Semaphore pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_SEMAPHORE;

    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Suspension from an non-task thread.  */
        status =  NU_INVALID_SUSPEND;

    else 
    {
    
        /* Parameters are valid, call actual function.  */
        status =  SMC_Obtain_Semaphore(semaphore_ptr, suspend);
    }

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMCE_Release_Semaphore                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the release semaphore function.                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      SMC_Release_Semaphore               Actual release semaphore fct.*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Semaphore control block ptr  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_SEMAPHORE                Invalid semaphore pointer    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMCE_Release_Semaphore(NU_SEMAPHORE *semaphore_ptr)
{

SM_SCB         *semaphore;                  /* Semaphore control blk ptr */
STATUS          status;                     /* Completion status         */


    /* Move input semaphore pointer into internal pointer.  */
    semaphore =  (SM_SCB *) semaphore_ptr;

    /* Determine if semaphore pointer is invalid.  */
    if (semaphore == NU_NULL)
    
        /* Semaphore pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_SEMAPHORE;

    else if (semaphore -> sm_id != SM_SEMAPHORE_ID)
    
        /* Semaphore pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_SEMAPHORE;

    else 
    {
    
        /* Parameters are valid, call actual function.  */
        status =  SMC_Release_Semaphore(semaphore_ptr);
    }

    /* Return the completion status.  */
    return(status);
}






