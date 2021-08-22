
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
/*      profiler.h                                     Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PROFILER - Profiler Management                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file links Nucleus PLUS to optional profiling modules.      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      NU_Profiler                         Eliminates compiler warnings */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      rtprofil.h                          ProView                      */
/*      nucprof.h                           ProView                      */
/*      rtlib.h                             ProView                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#ifndef PROFILE_H
#define PROFILE_H

/* The INCLUDE_PROVIEW macro enables the Nucleus ProView profiler in the
   Nucleus PLUS kernel.  When PLUS is built with this macro defined, all
   applications linked with PLUS must also be linked with the SurroundView
   Agent library.  Refer to the SurroundView chapter in the port notes for
   more details. */

#undef INCLUDE_PROVIEW

#ifdef INCLUDE_PROVIEW

#include "plus\sm_defs.h"
#include "plus\qu_defs.h"
#include "plus\mb_defs.h"
#include "plus\dm_defs.h"
#include "plus\pi_defs.h"
#include "plus\pm_defs.h"
#include "plus\ev_defs.h"
#include "plus\tm_defs.h"
#include "plus\tc_defs.h"

#include "svagent\inc\rtprofil.h"
#include "svagent\inc\nuc_prof.h"

#ifndef PLUS
#define PLUS
#endif

#include "svagent\inc\rtlib.h"

VOID _RTProf_TaskStatus(TC_TCB*, unsigned char);
VOID _RTProf_Dispatch_LISR_No_INT_Lock(int);
VOID _RTProf_RegisterLisr(int);
VOID _RTProf_DumpTask(TC_TCB*, unsigned char);
VOID _RTProf_DumpHisr(TC_HCB*, unsigned char);
VOID _RTProf_DumpSema(unsigned char, SM_SCB*, unsigned char);
VOID _RTProf_DumpQueue(unsigned char,QU_QCB*, unsigned char );
VOID _RTProf_DumpMailBox(unsigned char, MB_MCB*, unsigned char );
VOID _RTProf_DumpMemoryPool(unsigned char, DM_PCB*, unsigned char );
VOID _RTProf_DumpPipe(unsigned char, PI_PCB*, unsigned char );
VOID _RTProf_DumpPartitionPool(unsigned char, PM_PCB*, unsigned char );
VOID _RTProf_DumpEventGroup(unsigned char, EV_GCB*, unsigned char );
VOID _RTProf_DumpTimer(unsigned char, TM_APP_TCB*, unsigned char );
VOID _RTProf_DumpDriver(unsigned char, NU_DRIVER*, unsigned char );
VOID RTprofUserEvent(rt_uint32, char *);

#endif /* INCLUDE_PROVIEW */

#endif /* PROFILE_H */


