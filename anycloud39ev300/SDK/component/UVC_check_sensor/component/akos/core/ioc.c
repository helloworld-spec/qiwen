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
/*      ioc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IO - Input/Output Driver Management                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the I/O Driver          */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      IOC_Create_Driver                   Create an I/O driver         */
/*      IOC_Delete_Driver                   Delete an I/O driver         */
/*      IOC_Request_Driver                  Make an I/O driver request   */
/*      IOC_Resume_Driver                   Resume a task suspended in   */
/*                                            an I/O driver              */
/*      IOC_Suspend_Driver                  Suspend a task inside an I/O */
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
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0a                  */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Moved non-core functions into                    */
/*                      supplemental files, changed                      */
/*                      function interfaces to match                     */
/*                      those in prototype, changed                      */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-15-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      04-23-1996      Corrected SPR121.                                */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-07-1999      Release 1.11mA                                   */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "io_extr.h"                 /* I/O driver functions      */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */

/* Define external inner-component global data references.  */

extern CS_NODE         *IOD_Created_Drivers_List;
extern UNSIGNED         IOD_Total_Drivers;
extern TC_PROTECT       IOD_List_Protect;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOC_Create_Driver                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates an I/O driver and places it on the list of */
/*      created I/O drivers.  Note that this function does not actually  */
/*      invoke the driver.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      IOCE_Create_Driver                  Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Add node to linked-list      */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Data structure protect       */
/*      TCT_Unprotect                       Un-protect data structure    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      driver                              Driver control block pointer */
/*      name                                Driver's logical name        */
/*      driver_entry                        Driver's point of entry      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
STATUS  IOC_Create_Driver(NU_DRIVER *driver, CHAR *name,
                        VOID (*driver_entry)(NU_DRIVER *, NU_DRIVER_REQUEST *))
{

INT             i;                          /* Working index variable    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CREATE_DRIVER_ID, (UNSIGNED) driver,
                                (UNSIGNED) name, (UNSIGNED) driver_entry);

#endif

    /* First, clear the driver ID just in case it is an old Driver
       Control Block.  */
    driver -> nu_driver_id =             0;

    /* Fill in the driver's name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        driver  -> nu_driver_name[i] =  name[i];

    /* Save the driver's entry function in the control block.  */
    driver -> nu_driver_entry =  driver_entry;

    /* Protect against access to the list of created drivers.  */
    TCT_Protect(&IOD_List_Protect);

    /* At this point the driver  is completely built.  The ID can now be
       set and it can be linked into the created driver  list.  */
    driver  -> nu_driver_id =           IO_DRIVER_ID;

    /* Link the driver  into the list of created I/O drivers and increment the
       total number of drivers in the system.  */
    CSC_Place_On_List(&IOD_Created_Drivers_List, (CS_NODE *) driver);
    IOD_Total_Drivers++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpDriver(RT_PROF_CREATE_DRIVER, driver, RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* Release protection against access to the list of created I/O drivers. */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOC_Delete_Driver                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes an I/O driver and removes it from the list */
/*      of created drivers.  Note that this function does not actually   */
/*      invoke the driver.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      IOCE_Delete_Driver                  Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove node from list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      driver                              Driver control block pointer */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
STATUS  IOC_Delete_Driver(NU_DRIVER *driver)
{
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_DELETE_DRIVER_ID, (UNSIGNED) driver ,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against access to the list of created I/O drivers.  */
    TCT_Protect(&IOD_List_Protect);

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpDriver(RT_PROF_DELETE_DRIVER, driver, RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* Set the driver ID to 0.  */
    driver  -> nu_driver_id =  0;

    /* Remove the driver from the list of created I/O drivers.  */
    CSC_Remove_From_List(&IOD_Created_Drivers_List, (CS_NODE *) driver);

    /* Decrement the total number of created I/O drivers.  */
    IOD_Total_Drivers--;

    /* Release protection against access to the list of created I/O drivers. */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOC_Request_Driver                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a user request to the specified I/O driver.  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      IOCE_Request_Driver                 Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      driver                              Driver control block pointer */
/*      request                             User's I/O request           */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If service is successful     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
STATUS  IOC_Request_Driver(NU_DRIVER *driver , NU_DRIVER_REQUEST *request)
{
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_REQUEST_DRIVER_ID, (UNSIGNED) driver,
                                       (UNSIGNED) request, (UNSIGNED) 0);

#endif

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpDriver(RT_PROF_REQUEST_DRIVER, driver, RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* Call the specified I/O Driver.  */
    (*(driver -> nu_driver_entry)) (driver, request);

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOC_Resume_Driver                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function resumes a task previously suspended inside of an   */
/*      I/O driver.  Typically, this function is called from within an   */
/*      I/O driver.                                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      IOCE_Resume_Driver                  Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      TCT_Control_To_System               Transfer control to higher   */
/*                                            priority task              */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Get_Current_Protect             Pickup current protection    */
/*      TCT_Set_Current_Protect             Set current protection       */
/*      TCT_System_Protect                  Protect against system access*/
/*      TCT_System_Unprotect                Release system protection    */
/*      TCT_Unprotect                       Release system protection    */
/*      TCT_Unprotect_Specific              Release specific protection  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task                                Pointer of task to resume    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If service is successful     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match prototype, changed                         */
/*                      protection logic, resulting in                   */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-23-1996      Corrected SPR121                                 */
/*                                                                       */
/*************************************************************************/
STATUS  IOC_Resume_Driver(NU_TASK *task)
{

TC_PROTECT     *save_protect;               /* Saved protect pointer     */
NU_SUPERV_USER_VARIABLES


    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RESUME_DRIVER_ID, (UNSIGNED) task,
                                                (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Pickup current protection. */
    save_protect =  TCT_Get_Current_Protect();

    /* Protect against system access.  */
    TCT_System_Protect();

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpDriver(RT_PROF_RESUME_DRIVER, 0 , RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* Resume the specified task.  */
    if (TCC_Resume_Task(task, NU_DRIVER_SUSPEND))
    {
        /* Only unprotect if there is protection in place. */
        if (save_protect)
        {
            /* Release protection caller had.  */
            TCT_Unprotect_Specific(save_protect);
        }

        /* Transfer control to the system if the resumed task function
           detects a preemption condition.  */
        TCT_Control_To_System();
    }
    else
    {

        /* Determine if there was protection previously in force.  */
        if (save_protect)
        {

            /* Switch to original protection.  */
            TCT_Set_Current_Protect(save_protect);

            /* Release system protection.  */
            TCT_System_Unprotect();
        }
        else

            /* Release system protection.  */
            TCT_Unprotect();
    }

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(NU_SUCCESS);
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOC_Suspend_Driver                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function suspends a task inside of an I/O driver.  It is    */
/*      the responsibility of the I/O driver to keep track of tasks      */
/*      waiting inside of an I/O driver.                                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      IOCE_Suspend_Driver                 Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCT_Current_Thread                  Current task thread          */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Get_Current_Protect             Pickup current protect ptr   */
/*      TCT_Set_Suspend_Protect             Setup suspend protect field  */
/*      TCT_System_Protect                  Protect against system access*/
/*      TCT_Unprotect_Specific              Release user protection      */
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
/*      NU_SUCCESS                          If service is successful     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed protection logic,                        */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-23-1996      Corrected SPR121.                                */
/*                                                                       */
/*************************************************************************/
STATUS  IOC_Suspend_Driver(VOID (*terminate_routine)(VOID *),
                                        VOID *information, UNSIGNED timeout)
{

TC_PROTECT  *suspend_protect;               /* Current protection        */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_SUSPEND_DRIVER_ID, (UNSIGNED) terminate_routine,
                                (UNSIGNED) information, (UNSIGNED) timeout);

#endif


    /* Pickup current protect.  */
    suspend_protect =  TCT_Get_Current_Protect();

    /* Setup system protection.  */
    TCT_System_Protect();

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpDriver(RT_PROF_SUSPEND_DRIVER, 0 , RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* If no protection exists, don't unprotect. */
    if (suspend_protect)
    {
        /* Release initial protection.  */
        TCT_Unprotect_Specific(suspend_protect);

        /* Save suspend protect for timeout and terminate.  */
        TCT_Set_Suspend_Protect(suspend_protect);
    }

    /* Suspend the calling task.  */
    TCC_Suspend_Task((NU_TASK *) TCT_Current_Thread(), NU_DRIVER_SUSPEND,
                                terminate_routine, information, timeout);

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(NU_SUCCESS);
}





