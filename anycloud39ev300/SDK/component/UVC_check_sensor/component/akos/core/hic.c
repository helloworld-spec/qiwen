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
/*      hic.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      HI - History Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the History Management  */
/*      component.                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      HIC_Disable_History_Saving          Disable history saving       */
/*      HIC_Enable_History_Saving           Enable history saving        */
/*      HIC_Make_History_Entry_Service      Make history entry service   */
/*      HIC_Make_History_Entry              Make system history entry    */
/*      HIC_Retrieve_History_Entry          Retrieve history entry       */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_extr.h                           Thread Control functions     */
/*      tm_extr.h                           Timer management functions   */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Replaced void with VOID,                         */
/*                        modified protection logic,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "in_defs.h"                 /* Initialization defines    */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "tm_extr.h"                 /* Timer functions           */
#include        "hi_extr.h"                 /* History functions         */

/* Define external inner-component global data references.  */

extern INT              INC_Initialize_State;
extern INT              HID_History_Enable;
extern INT              HID_Write_Index;
extern INT              HID_Read_Index;
extern INT              HID_Entry_Count;
extern TC_PROTECT       HID_History_Protect;


/* Define the actual history table.  Note that this is defined in this file
   in order to eliminate this table if none of the run-time history functions
   are accessed.  */

HI_HISTORY_ENTRY  HIC_History_Table[HI_MAX_ENTRIES];


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      HIC_Disable_History_Saving                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function disables the history saving function.              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCT_Protect                         Protect history structures   */
/*      TCT_Unprotect                       Release history protection   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Replaced void with VOID,                         */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  HIC_Disable_History_Saving(VOID)
{
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Protect the history data structures.  */
    TCT_Protect(&HID_History_Protect);

    /* Disable history saving by setting the enable flag to false.  */
    HID_History_Enable =  NU_FALSE;

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      HIC_Enable_History_Saving                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function enables the history saving function.               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCT_Protect                         Protect history structures   */
/*      TCT_Unprotect                       Release history protection   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Replaced void with VOID,                         */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  HIC_Enable_History_Saving(VOID)
{
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Protect the history data structures.  */
    TCT_Protect(&HID_History_Protect);

    /* Enable history saving by setting the enable flag to true.  */
    HID_History_Enable =  NU_TRUE;

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      HIC_Make_History_Entry_Service                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function makes an application entry in the history table.   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      HIC_Make_History_Entry              Make a history entry         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      param1                              First history parameter      */
/*      param2                              Second history parameter     */
/*      param3                              Third history parameter      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  HIC_Make_History_Entry_Service(UNSIGNED param1,
                                        UNSIGNED param2, UNSIGNED param3)
{
    /* Call actual function to make the history entry.  */
    HIC_Make_History_Entry(NU_USER_ID, param1, param2, param3);
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      HIC_Make_History_Entry                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function makes an entry in the next available location in   */
/*      the history table- if history saving is enabled.                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Current_HISR_Pointer            Retrieve current HISR pointer*/
/*      TCC_Current_Task_Pointer            Retrieve current task pointer*/
/*      TCT_Get_Current_Protect             Pickup current protection    */
/*      TCT_Protect                         Protect history structures   */
/*      TCT_Set_Current_Protect             Set current protection       */
/*      TCT_Unprotect                       Release history protection   */
/*      TCT_Unprotect_Specific              Release history protection   */
/*      TMT_Retrieve_Clock                  Retrieve system clock        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      param1                              First history parameter      */
/*      param2                              Second history parameter     */
/*      param3                              Third history parameter      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified protection logic,                       */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  HIC_Make_History_Entry(DATA_ELEMENT id, UNSIGNED param1,
                                        UNSIGNED param2, UNSIGNED param3)
{
TC_PROTECT              *save_protect;      /* Save protect pointer     */
HI_HISTORY_ENTRY        *pointer;           /* Quick access pointer     */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* If we are not in initialization, get the current protection state */
    if (INC_Initialize_State ==  INC_END_INITIALIZE)

        /* Pickup current protection.  */
        save_protect =  TCT_Get_Current_Protect();

    else
        /* we are in initialization, just clear save_protect */
        save_protect = 0;

    /* Protect the history data structures.  */
    TCT_Protect(&HID_History_Protect);

    /* Determine if history saving is enabled.  */
    if (HID_History_Enable)
    {

        /* Yes, history saving is enabled.  */

        /* Build a pointer to the next location to write to in the table. */
        pointer =  &HIC_History_Table[HID_Write_Index];

        /* Place the necessary information into the history table at the
           current location.  */
        pointer -> hi_id =      id;
        pointer -> hi_param1 =  param1;
        pointer -> hi_param2 =  param2;
        pointer -> hi_param3 =  param3;
        pointer -> hi_time =    TMT_Retrieve_Clock();

        /* Now determine what thread we are currently in.  */
        if ((pointer -> hi_thread =
                (VOID *) TCC_Current_Task_Pointer()) != NU_NULL)

            /* Task thread.  Set the caller flag accordingly.  */
            pointer -> hi_caller =  HI_TASK;

        else if ((pointer -> hi_thread =
                (VOID *) TCC_Current_HISR_Pointer()) != NU_NULL)

            /* HISR thread.  Set the caller flag accordingly.  */
            pointer -> hi_caller =  HI_HISR;

        else

            /* Neither a task or HISR, it caller must be initialization.  */
            pointer -> hi_caller =  HI_INITIALIZE;

        /* Move the write index.  */
        HID_Write_Index++;

        /* Check for a wrap condition on the write index.  */
        if (HID_Write_Index >= HI_MAX_ENTRIES)

            /* Wrap condition present, adjust the write index to the top of the
               table.  */
            HID_Write_Index =  0;

        /* Increment the entries counter, if the maximum has not yet been
           reached.  */
        if (HID_Entry_Count < HI_MAX_ENTRIES)

            /* Increment the total entries counter.  */
            HID_Entry_Count++;
        else

            /* Drag the read index along with the write index.  */
            HID_Read_Index =  HID_Write_Index;
    }

    /* Determine if there was protection in force before call.  */
    if (save_protect)
    {

        /* Make saved protection the current again.  */
        TCT_Set_Current_Protect(save_protect);

        /* Release the history protection.  */
        TCT_Unprotect_Specific(&HID_History_Protect);
    }
    else

        /* Release protection.  */
        TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      HIC_Retrieve_History_Entry                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function retrieves the next oldest entry in the history     */
/*      table.  If no more entries are available, an error status is     */
/*      returned.                                                        */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCT_Protect                         Protect history structures   */
/*      TCT_Unprotect                       Release history protection   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      id                                  Destination for entry id     */
/*      param1                              Destination for parameter 1  */
/*      param2                              Destination for parameter 2  */
/*      param3                              Destination for parameter 3  */
/*      time                                Destination for time of entry*/
/*      task                                Destination of task pointer  */
/*      hisr                                Destination of hisr pointer  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
STATUS  HIC_Retrieve_History_Entry(DATA_ELEMENT *id, UNSIGNED *param1,
                                        UNSIGNED *param2, UNSIGNED *param3,
                                        UNSIGNED *time, NU_TASK **task,
                                        NU_HISR **hisr)
{

STATUS                  status;             /* Completion status        */
HI_HISTORY_ENTRY        *pointer;           /* Quick access pointer     */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Initialize status.  */
    status =  NU_SUCCESS;

    /* Protect the history data structures.  */
    TCT_Protect(&HID_History_Protect);

    /* Determine if there is an entry in the history log.  */
    if (HID_Entry_Count)
    {

        /* Yes, there is at least one entry in the history log.  */

        /* Build a pointer to the next location to read from in the table. */
        pointer =  &HIC_History_Table[HID_Read_Index];

        /* Place the necessary information into the history table at the
           current location.  */
        *id =           pointer -> hi_id;
        *param1 =       pointer -> hi_param1;
        *param2 =       pointer -> hi_param2;
        *param3 =       pointer -> hi_param3;
        *time =         pointer -> hi_time;

        /* Now determine what thread the entry was made from.  */
        if (pointer -> hi_caller == HI_TASK)
        {

            /* Setup the task return parameter.  */
            *task =     (NU_TASK *) pointer -> hi_thread;
            *hisr =     NU_NULL;
        }
        else
        {

            /* In either HISR or initialize case place the thread value
               in the HISR return parameter.  */
            *hisr =     (NU_HISR *) pointer -> hi_thread;
            *task =     NU_NULL;
        }

        /* Move the read index.  */
        HID_Read_Index++;

        /* Check for a wrap condition on the read index.  */
        if (HID_Read_Index >= HI_MAX_ENTRIES)

            /* Wrap condition present, adjust the read index to the top of the
               table.  */
            HID_Read_Index =  0;

        /* Decrement the entries counter.  */
        HID_Entry_Count--;
    }
    else

        /* Return the end of history log status.  */
        status =  NU_END_OF_LOG;

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return completion status to the caller.  */
    return(status);
}




