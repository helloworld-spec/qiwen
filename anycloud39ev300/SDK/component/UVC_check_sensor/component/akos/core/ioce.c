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
/*      ioce.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IO - Input/Output Driver Management                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the error checking routines for the functions */
/*      in the Input/Output Driver component.  This permits easy removal */
/*      of error checking logic when it is not needed.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      IOCE_Create_Driver                  Create an I/O driver         */
/*      IOCE_Delete_Driver                  Delete an I/O driver         */
/*      IOCE_Request_Driver                 Make an I/O driver request   */
/*      IOCE_Resume_Driver                  Resume a task suspended in   */
/*                                            an I/O driver              */
/*      IOCE_Suspend_Driver                 Suspend a task inside an I/O */
/*                                            driver                     */
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
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified logic that checked task                 */
/*                      status without protection of                     */
/*                      scheduling structures,                           */
/*                      resulting in version 1.0a                        */
/*      03-01-1994      Verified version 1.0a                            */
/*      03-01-1994      Changed name original error                      */
/*                      checking file and changed                        */
/*                      function interfaces, resulting                   */
/*                      in version 1.1                                   */
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
#include        "io_extr.h"                 /* I/O driver functions      */
#include        "hi_extr.h"                 /* History functions         */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOCE_Create_Driver                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the I/O driver create function.                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IOC_Create_Driver                   Actual create driver routine */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      driver                              Driver control block pointer */
/*      name                                Driver's logical name        */
/*      driver_entry                        Driver's point of entry      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_DRIVER                   Indicates driver pointer is  */
/*                                            NULL or is already in use  */
/*      NU_INVALID_POINTER                  Indicates the driver's entry */
/*                                            pointer is NULL            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface,                      */
/*                                        resulting in version 1.1       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  IOCE_Create_Driver(NU_DRIVER *driver, CHAR *name, 
                        VOID (*driver_entry)(NU_DRIVER *, NU_DRIVER_REQUEST *))
{

STATUS          status;                     /* Completion status         */


    /* Check for an invalid driver pointer.  */
    if ((driver == NU_NULL) || (driver -> nu_driver_id == IO_DRIVER_ID))
    
        /* Indicate that the driver pointer is invalid.  */
        status =  NU_INVALID_DRIVER;
    
    else if (driver_entry == NU_NULL)
    
        /* Indicate that the driver entry point is invalid.  */
        status =  NU_INVALID_POINTER;
    
    else
    
        /* Parameters are okay, call actual function to create an I/O 
           driver.  */
        status =  IOC_Create_Driver(driver, name, driver_entry);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOCE_Delete_Driver                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the I/O driver delete function.                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IOC_Delete_Driver                   Actual delete driver routine */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      driver                              Driver control block pointer */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_DRIVER                   Indicates the driver pointer */
/*                                            is invalid                 */
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
STATUS  IOCE_Delete_Driver(NU_DRIVER *driver)
{

STATUS          status;                     /* Completion status         */

    /* Determine if the driver pointer is valid.  */
    if ((driver) && (driver -> nu_driver_id == IO_DRIVER_ID))
    
        /* Driver pointer is valid, call function to delete it.  */
        status =  IOC_Delete_Driver(driver);
        
    else
    
        /* Driver pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_DRIVER;
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOCE_Request_Driver                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the I/O driver request function.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IOC_Request_Driver                  Actual request driver routine*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      driver                              Driver control block pointer */
/*      request                             User's I/O request           */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_DRIVER                   Indicates the driver pointer */
/*                                            is invalid                 */
/*      NU_INVALID_POINTER                  Indicates the request pointer*/
/*                                            is invalid                 */
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
STATUS  IOCE_Request_Driver(NU_DRIVER *driver , NU_DRIVER_REQUEST *request)
{

STATUS          status;                     /* Completion status         */


    /* Determine if driver pointer is invalid.  */
    if (driver == NU_NULL)
    
        /* Driver pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_DRIVER;

    else if (driver -> nu_driver_id != IO_DRIVER_ID)
    
        /* Driver pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_DRIVER;

    else if (request == NU_NULL)
    
        /* Request pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_POINTER;

    else
    
        /* Parameters are valid, call actual function.  */
        status =  IOC_Request_Driver(driver, request);

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOCE_Resume_Driver                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the I/O driver resume function.                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCCE_Validate_Resume                Validate resume driver       */
/*                                            request against actual     */
/*                                            status                     */
/*      IOC_Resume_Driver                   Actual resume driver routine */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task                                Pointer of task to resume    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_TASK                     Indicates the task pointer   */
/*                                            is invalid                 */
/*      NU_INVALID_RESUME                   Indicates the task is not    */
/*                                            suspended                  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified logic that checked task                 */
/*                      status without protection of                     */
/*                      scheduling structures,                           */
/*                      resulting in version 1.0a                        */
/*      03-01-1994      Verified version 1.0a                            */
/*      03-01-1994      Changed function interface,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  IOCE_Resume_Driver(NU_TASK *task)
{

STATUS          status;                     /* Completion status         */


    /* Determine if the task pointer is valid.  */
    if ((task == NU_NULL) || (((TC_TCB *) task) -> tc_id != TC_TASK_ID))
    
        /* Task pointer is invalid.  */
        status =  NU_INVALID_TASK;

    /* Check actual status of task to see if request is valid.  */
    else if (TCCE_Validate_Resume(NU_DRIVER_SUSPEND, task))
    
        /* Task is not suspended in a driver, return error status.  */
        status =  NU_INVALID_RESUME;
        
    else 
    
        /* Call the actual resume service.  */
        status =  IOC_Resume_Driver(task);
        
    /* Return the completion status.  */
    return(status);
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOCE_Suspend_Driver                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the I/O driver suspend function.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      IOC_Suspend_Driver                  Actual driver suspend routine*/
/*      TCCE_Suspend_Error                  Check for a legal suspension */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      terminate_routine                   Termination/Timeout cleanup  */
/*                                            routine                    */
/*      information                         Information pointer of the   */
/*                                            cleanup routine            */
/*      timeout                             Suspension timeout request   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_SUSPEND                  Indicates suspension is not  */
/*                                            legal                      */
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
STATUS  IOCE_Suspend_Driver(VOID (*terminate_routine)(VOID *), 
                                        VOID *information, UNSIGNED timeout)
{

STATUS          status;                     /* Completion status         */


    /* Determine if there is a suspension error.  */
    if (TCCE_Suspend_Error())
    
        /* Suspension error, not called from a legal thread.  */
        status =  NU_INVALID_SUSPEND;
        
    else 
    
        /* Call the actual suspend service.  */
        status =  IOC_Suspend_Driver(terminate_routine, information, timeout);
        
    /* Return the completion status.  */
    return(status);
}







