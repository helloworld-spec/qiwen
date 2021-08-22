/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKError.h
* Function: 
* Author: Eric
* Date: 2007-05-31
* Version: 1.0
*
***************************************************************************/
#ifndef __AKERROR_H__
#define __AKERROR_H__

/* Define service completion status constants.  */

#define         AK_SUCCESS                      0
#define         AK_END_OF_LOG                   -1
#define         AK_GROUP_DELETED                -2
#define         AK_INVALID_DELETE               -3
#define         AK_INVALID_DRIVER               -4
#define         AK_INVALID_ENABLE               -5
#define         AK_INVALID_ENTRY                -6
#define         AK_INVALID_FUNCTION             -7
#define         AK_INVALID_GROUP                -8
#define         AK_INVALID_HISR                 -9
#define         AK_INVALID_MAILBOX              -10
#define         AK_INVALID_MEMORY               -11
#define         AK_INVALID_MESSAGE              -12
#define         AK_INVALID_OPERATION            -13
#define         AK_INVALID_PIPE                 -14
#define         AK_INVALID_POINTER              -15
#define         AK_INVALID_POOL                 -16
#define         AK_INVALID_PREEMPT              -17
#define         AK_INVALID_PRIORITY             -18
#define         AK_INVALID_QUEUE                -19
#define         AK_INVALID_RESUME               -20
#define         AK_INVALID_SEMAPHORE            -21
#define         AK_INVALID_SIZE                 -22
#define         AK_INVALID_START                -23
#define         AK_INVALID_SUSPEND              -24
#define         AK_INVALID_TASK                 -25
#define         AK_INVALID_TIMER                -26
#define         AK_INVALID_VECTOR               -27
#define         AK_MAILBOX_DELETED              -28
#define         AK_MAILBOX_EMPTY                -29
#define         AK_MAILBOX_FULL                 -30
#define         AK_MAILBOX_RESET                -31
#define         AK_NO_MEMORY                    -32
#define         AK_NO_MORE_LISRS                -33
#define         AK_NO_PARTITION                 -34
#define         AK_NOT_DISABLED                 -35
#define         AK_NOT_PRESENT                  -36
#define         AK_NOT_REGISTERED               -37
#define         AK_NOT_TERMINATED               -38
#define         AK_PIPE_DELETED                 -39
#define         AK_PIPE_EMPTY                   -40
#define         AK_PIPE_FULL                    -41
#define         AK_PIPE_RESET                   -42
#define         AK_POOL_DELETED                 -43
#define         AK_QUEUE_DELETED                -44
#define         AK_QUEUE_EMPTY                  -45
#define         AK_QUEUE_FULL                   -46
#define         AK_QUEUE_RESET                  -47
#define         AK_SEMAPHORE_DELETED            -48
#define         AK_SEMAPHORE_RESET              -49
#define         AK_TIMEOUT                      -50
#define         AK_UNAVAILABLE                  -51
#define         AK_INVALID_DESCRIPTION          -52
#define         AK_INVALID_REGION               -53
#define         AK_MEMORY_CORRUPT               -54
#define         AK_INVALID_DEBUG_ALLOCATION     -55
#define         AK_EMPTY_DEBUG_ALLOCATION_LIST  -56
#define         AK_EXIST_MESSAGE                -57



//common
#define         AK_EFAILED                      1
#define         AK_ENOMEMORY                    2
#define         AK_EBADPARAM                    3
#define         AK_EUNSUPPORT                   4
#define         AK_EBADCLASS                    5
#define         AK_ENOTFOUND                    6
//... ...

/* Define constants that are target dependent and/or are used for internal
   purposes.  */

#define         AK_MIN_STACK_SIZE               240
#define         AK_MAX_NAME                     8
#define         AK_MAX_VECTORS                  64
#define         AK_MAX_LISRS                    8

/* Define constants for the number of UNSIGNED words in each of the basic
   system data structures.  */

#define         AK_TASK_SIZE                    42
#define         AK_HISR_SIZE                    22
#define         AK_MAILBOX_SIZE                 13
#define         AK_QUEUE_SIZE                   18
#define         AK_PIPE_SIZE                    18
#define         AK_SEMAPHORE_SIZE               10
#define         AK_EVENT_GROUP_SIZE             9
#define         AK_PARTITION_POOL_SIZE          15
#define         AK_MEMORY_POOL_SIZE             17
#define         AK_TIMER_SIZE                   17
#define         AK_PROTECT_SIZE                 2
#define         AK_DRIVER_SIZE                  3

/* Define interrupt lockout and enable constants.  */

#define         AK_DISABLE_INTERRUPTS           0xC0
#define         AK_ENABLE_INTERRUPTS            0x00

/* Define system errors.  */

#define         AK_ERROR_CREATING_TIMER_HISR    1
#define         AK_ERROR_CREATING_TIMER_TASK    2
#define         AK_STACK_OVERFLOW               3
#define         AK_UNHANDLED_INTERRUPT          4


/* Define I/O driver constants.  */

#define         AK_IO_ERROR                     -1
#define         AK_INITIALIZE                   1
#define         AK_ASSIGN                       2
//#define         AK_RELEASE                      3
#define         AK_INPUT                        4
#define         AK_OUTPUT                       5
#define         AK_STATUS                       6
#define         AK_TERMINATE                    7

//File error...
//...  ...

//...  ...



/*=================================================================================*/
/*#################################################################################*/
/*=================================================================================*/

#define AK_IS_SUCCESS(ret)    (ret == AK_SUCCESS)
#define AK_IS_FAILURE(ret)    (ret != AK_SUCCESS)

#endif //__AKERROR_H__
