/*************************************************************************/
/*                                                                       */
/* FILE NAME       Wd_extr.h                                      */
/* VERSION         0.0.1						 */
/*                                                                       */
/* COMPONENT                                                     */
/*                                                                        */
/*      WD - watchdog control head file                                  */
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
/*      AK_watchdog_HISR        find the reason for watchdog die*/
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                            */
/************************************************/	
#ifndef _WD_EXTR_H
#define 	_WD_EXTR_H
#include	"anyka_types.h"

//#define		WD_DEBUG	
#define   	WATCHDOG	 

#define		KEEP_LOACLA		0	   //locale is stored in thread_current_value
#define		THREAD_TIMEOUT  -1     //thread is no executing
#define     STACK_DESTROY	-2	   //stack is destroyed	


#endif
//end file
