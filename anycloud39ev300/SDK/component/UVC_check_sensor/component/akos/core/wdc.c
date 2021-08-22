/*************************************************************************/
/*                                                                       */
/* FILE NAME       Wdc.c                                      */
/* VERSION         0.0.1						 */
/*                                                                       */
/* COMPONENT                                                     */
/*                                                                        */
/*      WD - watchdog control                                  */
/*                                                                        */
/* DESCRIPTION                                                    */
/*                                                                        */
/*      This file contains the core routines for the       */
/*       watchdog  Control  component.                      */
/*                                                                       */
/* DATA STRUCTURES                                            */
/*                                                                       */
/*      None                                                          */
/*                                                                       */
/* FUNCTIONS                                                      */
/*                                                                       */
/*      AK_watchdog_handler     check watchdog status*/
/*      AK_feed_watchdog         feed watchdog           */
/*      AK_Watchdog_Died        find the reason for watchdog die*/
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                            */
/************************************************/	

#include "tc_defs.h"
#include "tc_extr.h"
#include "nucleus.h"
#include "AKerror.h"
#include "wd_extr.h"
#include "akos_api.h"

#ifdef   WATCHDOG

#define		WD_CLOSE		0
#define   	WD_DIE      	1

// 
extern VOID            *TCD_Current_Thread;
extern TC_PROTECT       TCD_System_Protect;

static T_WD_CB WD_Callback = NU_NULL;
//reserve watchdog error message 
static THREAD_LOCALE thread_current_value;

//calls
static T_VOID	AK_Watchdog_Died(VOID);



/*
* @author Yao Hongshi
* @date 2007-11-06
* @param T_VOID
* @return STATUS -- it has two status, one is zero ,ir present watchdog goes well
*                            the  other is  (-1), it present watchdog has been dead
* @brief: watch dog handler of theard ,here it will check tc_wd_counter in TCB(or HCB),
* @   wether it has been underflow , if it is right, here will awake up watchdog_HISR. or
* @  it will decrease tc_wd_counter,and return.
*/

T_VOID AK_Kick_Watchdog(T_VOID)
{
	TC_TCB	*task = NU_NULL;
	TC_HCB	*hisr = NU_NULL;

	task = TCD_Current_Thread;

	//get currnet task (TCB or HCB)
	if ((task != NU_NULL) && (task ->tc_id != TC_TASK_ID))
	{
		hisr = TCD_Current_Thread;
		task = NU_NULL;
	}
	
	//check task theard  wether watcodg goes well
    if ((task != NU_NULL)&&(task->tc_wd_counter > WD_DIE) )
    {	
        //DEC watchdog counter
        task->tc_wd_counter--;
        
        //watchdog die 
        if(task->tc_wd_counter == WD_DIE)
        {
            //save thread locale
            AK_Watchdog_Died();
        }
    }

	//check hisr theard  wether watcodg goes well
    if ((hisr != NU_NULL)&&(hisr->tc_wd_counter > WD_DIE) )
    {
        //DEC watchdog counter
        hisr->tc_wd_counter--;
        
        //watchdog die
        if(hisr->tc_wd_counter == WD_DIE)
        {
            //save thread localex
            AK_Watchdog_Died();
        }
    }
}



/*********************************************************
* @author Yao Hongshi
* @date 2007-11-06
* @param unsigned int food --it can not be 1,
* @return STATUS  -- it has two status, one is zero ,ir present watchdog gets well
*                            the  other is  (-1), it present food is illegal
* @brief: watch dog handler of theard ,here it will check tc_wd_counter in TCB(or HCB),
* @   wether it has been underflow , if it is right, here will awake up watchdog_HISR. or
* @  it will decrease tc_wd_counter,and return.
**********************************************************/

T_S32 AK_Feed_WatchdogEx(T_U32  food, T_pSTR filename, T_U32 line)
{
	TC_TCB	*task = NU_NULL;
	TC_HCB	*hisr = NU_NULL;

	task = TCD_Current_Thread;

	//check wether food is illegal 
	if(food != WD_CLOSE)
	{
		food++;
	}

	//get currnet task (TCB or HCB)
	if ((task != NU_NULL) && (task ->tc_id != TC_TASK_ID))
	{	
		hisr = TCD_Current_Thread;
		task = NU_NULL;
	}
	
	// reset task  watchdog counter value
#ifdef   WATCHDOG
	if(task != NU_NULL)
	{
		task->filename = filename;
		task->fileline = line;
		task->tc_wd_counter = food;
	}
#endif		

	// reset HISR watchdog counter value	
#ifdef   WATCHDOG
	if(hisr != NU_NULL)
	{
		hisr->filename = filename;
		hisr->fileline = line;
		hisr->tc_wd_counter = food;
	}
#endif		

	return AK_SUCCESS;
}

/**********************************************************************************************
* @author Yao Hongshi
* @date 2007-11-06
* @param NO,
* @return no
* @brief: watch dog handler of theard ,here it will check tc_wd_counter in TCB(or HCB),
* @   wether it has been underflow , if it is right, here will awake up watchdog_HISR. or
* @  it will decrease tc_wd_counter,and return.
;@@HISR stack frame has the following format:
;@@               (Lower Address) Stack Top ->    0       (Solicited stack type)
;@@                !!FOR THUMB ONLY!!             0/0x20  Saved state mask
;@@                                               R4      Saved R4
;@@                                               R5      Saved R5
;@@                                               R6      Saved R6
;@@                                               R7      Saved R7
;@@                                               R8      Saved R8
;@@                                               R9/sb   Saved R9/sl
;@@                                               R10/sl   Saved R10/sl
;@@                                               fp      Saved fp
;@@                                               ip      Saved ip
;@@               (Higher Address) Stack Bottom-> pc      Saved pc
* @Task stack frame has the following format:
* @ 	(Lower Address) Stack Top ->     1       Interrupt stack type)
* @                                               CPSR    Saved CPSR
* @                                               R0      Saved R0
* @                                               R1      Saved R1
* @                                               R2      Saved R2
* @                                               R3      Saved R3
* @                                               R4      Saved R4
* @                                               R5      Saved R5
* @                                               R6      Saved R6
* @                                               R7      Saved R7
* @                                               R8      Saved R8
* @                                               R9/sb   Saved R9/sl
* @                                               R10/sl  Saved R10/sl
* @                                               fp      Saved fp
* @                                               ip      Saved ip
* @                                               sp      Saved sp
* @                                               lr      Saved lr
* @               (Higher Address) Stack Bottom-> pc      Saved pc
* @
***********************************************************************************************/

static T_VOID	AK_Watchdog_Died(VOID)
{
	INT			 i;
	TC_TCB		*task = NU_NULL;
	TC_HCB		*hisr = NU_NULL;
	UNSIGNED	*tc_stack_pointer = 0;

    if(TCD_Current_Thread == NU_NULL)
    {
        thread_current_value.error_type = THREAD_TIMEOUT;
        return ;
    }
    
    task = TCD_Current_Thread;
    
    //get currnet task (TCB or HCB)
    if (task ->tc_id != TC_TASK_ID )
    {	
        hisr = TCD_Current_Thread;
        task = NU_NULL;
    }


	//check wether current theard' watchdog die , and save locale
    if ( (hisr != NU_NULL) && ( hisr->tc_wd_counter == WD_DIE))
    {	
        //initialize current thread  base information according task'TCB
        for (i = 0; i < NU_MAX_NAME; i++)
        {
            thread_current_value.tc_name[i] = hisr ->tc_name[i];
        }
        thread_current_value.tc_name[NU_MAX_NAME] = '\0';
        
        thread_current_value.tc_stack_start 	= hisr ->tc_stack_start;	    // Stack starting address
        thread_current_value.tc_stack_end		= hisr ->tc_stack_end;			// Stack ending address 	
        thread_current_value.tc_stack_pointer	= hisr ->tc_stack_pointer;
        thread_current_value.tc_stack_size		= hisr ->tc_stack_size; 		// HISR or Task stack's size	
        thread_current_value.feed_file 			= hisr ->filename; 				// Last feed in which file	
        thread_current_value.feed_line			= hisr ->fileline; 				// and the fileline	
    }

	//check wether current theard' watchdog die , and save locale
    if ( (task != NU_NULL) && ( task->tc_wd_counter == WD_DIE))
    {
        //initialize current thread  base information according task'TCB
        for (i = 0; i < NU_MAX_NAME; i++)
        {
            thread_current_value.tc_name[i] = task ->tc_name[i];
        }
        
        thread_current_value.tc_name[NU_MAX_NAME] = '\0';
        
        thread_current_value.tc_stack_start		= task ->tc_stack_start;	    // Stack starting address
        thread_current_value.tc_stack_end		= task ->tc_stack_end;			// Stack ending address 	
        thread_current_value.tc_stack_pointer  	= task ->tc_stack_pointer;
        thread_current_value.tc_stack_size 		= task ->tc_stack_size;			// HISR or Task stack's size	
        thread_current_value.feed_file 			= task ->filename; 				// Last feed in which file	
        thread_current_value.feed_line			= task ->fileline; 				// and the fileline	
    }
    
    
    //get interrupted statck of thread
    tc_stack_pointer = thread_current_value.tc_stack_pointer;
    
    //check wether interrupted stack of thread is illegal.
    thread_current_value.error_type = KEEP_LOACLA;

    if ((*tc_stack_pointer) == 0)         /*HISR Stack*/
    {
        //get locale value
        tc_stack_pointer++;       //jump 0
        tc_stack_pointer++;		  //jump 0/0x20

        //get reg4--reg12
        for(i = 4; i<13; i++)     //get reg4--reg12
        {
            thread_current_value.reg[i] = (*tc_stack_pointer);
            tc_stack_pointer++;            
        }

         //set sp;
        thread_current_value.tc_current_sp = 0;
        
        //set the caller lr        
        thread_current_value.func_caller   = 0;
        
        //get thread current pc value
        thread_current_value.tc_pc_value   = (*tc_stack_pointer);
        tc_stack_pointer = NU_NULL;
    }
    else if((*tc_stack_pointer) == 1)     /*Task Stack*/
    {        
        //get locale value
        tc_stack_pointer++;       //jump 1
        tc_stack_pointer++;		  //jump CPSR
        
        //get reg0--reg12
        for(i = 0; i<13; i++)     //get reg0--reg12
        {
            thread_current_value.reg[i] = (*tc_stack_pointer);
            tc_stack_pointer++;            
        }
        
        //get sp;
        thread_current_value.tc_current_sp = (*tc_stack_pointer);
        tc_stack_pointer++;
        
        //ger the caller lr        
        thread_current_value.func_caller   = (*tc_stack_pointer);
        tc_stack_pointer++;
        
        //get thread current pc value
        thread_current_value.tc_pc_value   = (*tc_stack_pointer);
        tc_stack_pointer = NU_NULL;
        
        //get the 20 values in the front of stack
        tc_stack_pointer = (T_U32*)thread_current_value.tc_current_sp;
        for (i = 0; i < 20; i++)
        {
            thread_current_value.stack_current_value[i] = (*tc_stack_pointer);
            tc_stack_pointer++;
            
        }
        tc_stack_pointer = NU_NULL;
    }
    else
    {
        thread_current_value.error_type = STACK_DESTROY;
    }

    //stop watchdog
    AK_Feed_Watchdog(WD_CLOSE);
	
    if (thread_current_value.error_type == KEEP_LOACLA)
	{
		if (WD_Callback != NU_NULL)
		{
			(WD_Callback)((void *)&thread_current_value);
		}
    }
}

T_VOID AK_Set_WD_Callback(T_WD_CB cb)
{
	WD_Callback = cb;
}

#else

/*
* @author Yao Hongshi
* @date 2007-11-06
* @param T_VOID
* @return STATUS -- it has two status, one is zero ,ir present watchdog goes well
*                            the  other is  (-1), it present watchdog has been dead
* @brief: watch dog handler of theard ,here it will check tc_wd_counter in TCB(or HCB),
* @   wether it has been underflow , if it is right, here will awake up watchdog_HISR. or
* @  it will decrease tc_wd_counter,and return.
*/

T_VOID AK_Kick_Watchdog(T_VOID)
{
	return ; 
}



/*********************************************************
* @author Yao Hongshi
* @date 2007-11-06
* @param unsigned int food --it can not be 1,
* @return STATUS  -- it has two status, one is zero ,ir present watchdog gets well
*                            the  other is  (-1), it present food is illegal
* @brief: watch dog handler of theard ,here it will check tc_wd_counter in TCB(or HCB),
* @   wether it has been underflow , if it is right, here will awake up watchdog_HISR. or
* @  it will decrease tc_wd_counter,and return.
**********************************************************/

T_S32 AK_Feed_WatchdogEx(T_U32  food, T_pSTR filename, T_U32 line)
{
	return AK_SUCCESS;
}

/*********************************************************
* @author Yao Hongshi
* @date 2007-11-06
* @param NO,
* @return no
* @brief: watch dog handler of theard ,here it will check tc_wd_counter in TCB(or HCB),
* @   wether it has been underflow , if it is right, here will awake up watchdog_HISR. or
* @  it will decrease tc_wd_counter,and return.
* @
**********************************************************/

T_VOID	AK_Watchdog_Died(VOID)
{
	return ;
}

T_VOID AK_Set_WD_Callback(T_WD_CB cb)
{
	WD_Callback = cb;
}

#endif

//end file

