/*****************************************************************
 * file name :            except.c
 * Copyright :            Anyka (GuangZhou) Software Technology Co., Ltd. 
 * version   :        0.0.1
 * author    :        Yao Hongshi
 * date      :        2007-11-19
 * brief     :        exception handle 
 ***************************************************************** */
//include file

#include "anyka_types.h"
#include "anyka_cpu.h"
#include "hal_except.h"
#include "drv_api.h"

static T_fCExceptCallback m_excpt_cb=NULL;

//error message store struct
typedef  struct
{
        unsigned char    error_type;

        unsigned long   old_pc;                          //thread current PC value
        unsigned long   old_spsr;
        unsigned long   old_reg[13];
        unsigned long   old_sp;        
        unsigned long   old_stack_value[20];

}T_EXCEPTION_CONTEXT;

//reserve watchdog error message 
T_EXCEPTION_CONTEXT       exception_context;

// thread interface for get thread control block


/***************************************************
* @brief:  when system_error occur, save the context and display it
           for debugging purpose
* @note: the context is saved in exception stack as follow:
        ------------ stack top
          lr(svc pc)
        ------------
            r12
        ------------
            r11
        ------------
            r10
        ------------
            r9
        ------------
            r8
        ------------
            r7
        ------------
            r6
        ------------
            r5
        ------------
            r4
        ------------
            r3
        ------------
            r2
        ------------
            r1
        ------------
            r0
        ------------
          err type
        ------------
            spsr
        ------------
           svc sp
        ------------ stack bottom(current sp)
*/
void system_error_check( unsigned long *addr_sp )
{
        unsigned long  *current_stack = NULL;
        unsigned long  *old_stack = NULL;
        unsigned long   temp = 0;
        unsigned long   spsr_value = 0;
        unsigned long   error_type = 0;
        int     i;


        current_stack = addr_sp;
#ifdef  ABORT_DEBUG
        akprintf(C1, M_DRVSYS, "\t current stack pointer value: %x\n", addr_sp);
#endif


        /* the follow is get error message into a struct */
        //get old stack pointer
        old_stack =(unsigned long *)(*current_stack);
#ifdef  ABORT_DEBUG 
        akprintf(C1, M_DRVSYS, "\t old stack : %x \n",old_stack);
#endif
        exception_context.old_sp = (*current_stack);
        current_stack++;


        //get  spsr value       
        spsr_value = (*current_stack);
#ifdef  ABORT_DEBUG 
        akprintf(C1, M_DRVSYS, "\t spsr : %x \n",(*current_stack));
#endif
        exception_context.old_spsr = (*current_stack);
        current_stack++;        

        //print  error type
        temp = (*current_stack);
#ifdef  ABORT_DEBUG 
        akprintf(C1, M_DRVSYS, "\t error type :%x\n", (*current_stack));
#endif
        exception_context.error_type = (*current_stack);
        current_stack++;


        //get old reg value  r0-r12
        for (i = 0; i < 13; i++)
        {
                exception_context.old_reg[i] = (*current_stack);

#ifdef  ABORT_DEBUG 
                akprintf(C1, M_DRVSYS, "\t reg[%d] : %x\n",i, (*current_stack));
#endif
                current_stack++;
        }

        //get   previous PC  value
        exception_context.old_pc = (*current_stack);

#ifdef  ABORT_DEBUG 
        akprintf(C1, M_DRVSYS, "\t old pc :%x\n", (*current_stack));
#endif 

        current_stack++;

        //get  the value in  stack
        //      old_stack = (unsigned long *)exception_context.old_sp;
        for     (i = 0; i < 20; i++)
        {
                exception_context.old_stack_value[i] = (*old_stack);
                old_stack++;

#ifdef  ABORT_DEBUG 
                akprintf(C1, M_DRVSYS, "\t stack value  :%x\n", exception_context.old_stack_value[i]);
#endif
        }

        akprintf(C1, M_DRVSYS, "\n###########################################################\n");
        
        //print error message
        switch( temp )  
        {
                case  UNDEF_ERROR:      
                        akprintf(C1, M_DRVSYS, "\t Error type:  undefined error\n");
                        exception_context.old_pc -= 4;
                        break;          
                case  ABT_ERROR:        
                        akprintf(C1, M_DRVSYS, "\t Error type:  abort error \n");                       
                        akprintf(C1, M_DRVSYS, "\t abort address: 0x%x \n", MMU_Reg6());        
                        exception_context.old_pc -= 8;
                        break;          
                case  PREF_ERROR:               
                        akprintf(C1, M_DRVSYS, "\t Error type:  prefetch error \n");
                        exception_context.old_pc -= 4;
                        break;
                default:
                        break;
        }

        //print old pc ,old spsr, old stack pointer and reg0-reg10
        akprintf(C1, M_DRVSYS, "\t Stack pointer : 0x%x\n",exception_context.old_sp);

        akprintf(C1, M_DRVSYS, "\t PC    value   is : 0x%x\n",exception_context.old_pc);
        akprintf(C1, M_DRVSYS, "\t SPSR  value   is : 0x%x\n",exception_context.old_spsr);

        for (i = 0; i < 13; i++)
        {
                akprintf(C1, M_DRVSYS, "\t R[%d]  value   is : 0x%x\n",i,(exception_context.old_reg[i]));
        }

        //print the values in front of the stack
        akprintf(C1, M_DRVSYS, "\t the values in the stack is:\n");
        for     (i = 0; i < 20; i++)
        {
                akprintf(C1, M_DRVSYS, "\t 0x%x", exception_context.old_stack_value[i]);
        }
        akprintf(C1, M_DRVSYS, "\n###########################################################\n");

        if (m_excpt_cb)
                m_excpt_cb(exception_context.error_type);
                
        while(1)
        {
            console_send_buffer();
        }
}

void  exception_set_callback(T_fCExceptCallback cb)
{
        m_excpt_cb = cb;
}

void exception_reset_ap(void)
{
        unsigned long reg;

    
        /* reset and close multimedia clock */
        #define RBOOT_RESET_BIT                     0x18510000 //bit[28,27,22,20,16]
        *(volatile unsigned int*)(RESET_CTRL_REG) |= RBOOT_RESET_BIT;
        mini_delay(10);
        *(volatile unsigned int*)(RESET_CTRL_REG) &= ~RBOOT_RESET_BIT;
                        
        *(volatile unsigned int *)CLOCK_CTRL_REG |= 0x1851;
}

/**
 @brief:
    angel_SWIreason_ReportException, r1 is one of these:
    Name (#defined in adp.h)
    --------------------------------------------
    Hexadecimal                         value
    --------------------------------------------
    ADP_Stopped_BranchThroughZero       0x20000 
    ADP_Stopped_UndefinedInstr          0x20001
    ADP_Stopped_SoftwareInterrupt       0x20002
    ADP_Stopped_PrefetchAbort           0x20003
    ADP_Stopped_DataAbort               0x20004
    ADP_Stopped_AddressException        0x20005
    ADP_Stopped_IRQ                     0x20006
    ADP_Stopped_FIQ                     0x20007
    --------------------------------------------
    ADP_Stopped_BreakPoint              0x20020
    ADP_Stopped_WatchPoint              0x20021
    ADP_Stopped_StepComplete            0x20022
    ADP_Stopped_RunTimeErrorUnknown     *0x20023
    ADP_Stopped_InternalError           *0x20024
    ADP_Stopped_UserInterruption        0x20025
    ADP_Stopped_ApplicationExit         0x20026
    ADP_Stopped_StackOverflow           *0x20027
    ADP_Stopped_DivisionByZero          *0x20028
    ADP_Stopped_OSSpecific              *0x20029
    ---------------------------------------------
 */
void swi_dispatch(unsigned long id, unsigned long value, unsigned long number)
{
    /* ARM Semihosting SWI */
    if (number == 0x123456)
    {   
        /* angel_SWIreason_ReportException */
        if (id == 0x18)
        {
            akprintf(C1, M_DRVSYS, "angel_SWIreason_ReportException:\n");
            akprintf(C1, M_DRVSYS, "swi_number=0x%x, id=0x%x, value=0x%x\n", number, id, value);        
        }
        else
        {
            akprintf(C1, M_DRVSYS, "other SWIreason:\n");
            akprintf(C1, M_DRVSYS, "swi_number=0x%x, id=0x%x, value=0x%x\n", number, id, value);        
        }
    }
}

