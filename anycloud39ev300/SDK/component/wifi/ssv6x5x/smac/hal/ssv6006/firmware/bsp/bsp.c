#include "n12_def.h"

#include "bsp_hal.h"
#include "hal.h"
//#include "uart/uart.h"
#include "timer/drv_timer.h"

#define TICK_HZ		100



void OS_Trap_Default_Handler(uint32_t vec )
{
	while(1){};
}

void *OS_CPU_Vector_Table[32] = 
{
	OS_Trap_Default_Handler,	// HW0
	OS_Trap_Default_Handler,	// HW1
	OS_Trap_Default_Handler,	// HW2
	OS_Trap_Default_Handler,	// HW3
	OS_Trap_Default_Handler,	// HW4
	OS_Trap_Default_Handler,	// HW5
	OS_Trap_Default_Handler,	// HW6
	OS_Trap_Default_Handler,	// HW7
	OS_Trap_Default_Handler,	// HW8
	OS_Trap_Default_Handler,	// HW9
	OS_Trap_Default_Handler,	// HW10
	OS_Trap_Default_Handler,	// HW11
	OS_Trap_Default_Handler,	// HW12
	OS_Trap_Default_Handler,	// HW13
	OS_Trap_Default_Handler,	// HW14
	OS_Trap_Default_Handler,	// HW15
	OS_Trap_Default_Handler,	// HW16
	OS_Trap_Default_Handler,	// HW17
	OS_Trap_Default_Handler,	// HW18
	OS_Trap_Default_Handler,	// HW19
	OS_Trap_Default_Handler,	// HW20
	OS_Trap_Default_Handler,	// HW21
	OS_Trap_Default_Handler,	// HW22
	OS_Trap_Default_Handler,	// HW23
	OS_Trap_Default_Handler,	// HW24
	OS_Trap_Default_Handler,	// HW25
	OS_Trap_Default_Handler,	// HW26
	OS_Trap_Default_Handler,	// HW27
	OS_Trap_Default_Handler,	// HW28
	OS_Trap_Default_Handler,	// HW29
	OS_Trap_Default_Handler,	// HW30
	OS_Trap_Default_Handler		// HW31
};




/* DavidHu 2010/12/6, enable FPU if the CPU support FPU */
#if defined(__NDS32_EXT_FPU_DP__) || defined(__NDS32_EXT_FPU_SP__)
DEFINE_GET_SYS_REG(CR6);
DEFINE_GET_SYS_REG(FUCPR);
DEFINE_PUT_SYS_REG(FUCPR);
#endif


#if defined(CONFIG_OS_UCOS_III)
#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
#include <os.h>
void  CPU_TS_TmrInit(void)
{
	/* Enable Timer #2 for uC/OS-III Time stamp */
	hal_timer_set_period(0x2,0x0);
	//hal_timer_set_match1(0x2,0x0);
	hal_timer_set_upward(0x2,0x1);
	hal_timer_start(0x2);
#if 0
	REG32(TMRC_TM2_LOAD)		= 0;
	REG32(TMRC_TM2_COUNTER)		= 0;
	REG32(TMRC_TM2_MATCH1)		= 0;
	REG32(TMRC_TMCR)		|= (1UL << TMRC_TM2_ENABLE_BIT) | (1UL << TMRC_TM2_UPDOWN_BIT);
#endif
	CPU_TS_TmrFreqSet(TICK_HZ);
}
CPU_TS  CPU_TS_TmrRd (void)
{
	return hal_timer_read(2);
}
CPU_INT64U  CPU_TS32_to_uSec(CPU_TS32  ts_cnts)
{
	return (CPU_INT64U)(ts_cnts * 1000000)/ MB_PCLK;
}
#else
void  CPU_TS_TmrInit(void)
{
	KASSERT(HAL_NULL);
}
CPU_TS  CPU_TS_TmrRd (void)
{
	KASSERT(HAL_NULL);
	return HAL_NULL;
}
#endif
#endif

void BSP_Tmr_TickInit(uint32_t period, void *isr)
{
    /* start  timer */
    hal_timer_stop(SYS_TICK_TIMER);

    /*
     * tick ISR init
     */
    /* init trigger mode */
    /* Set edge trigger, falling edge */
    hal_intc_irq_config(IRQ_SYS_TICK_VECTOR, 1, 0);
    /* clean pending */
    hal_intc_irq_clean(IRQ_SYS_TICK_VECTOR);
    if (isr)
    {
        hal_register_isr(IRQ_SYS_TICK_VECTOR, isr, NULL);
        /* enable timer interrupt */
        hal_intc_irq_enable(IRQ_SYS_TICK_VECTOR);
    }
    else
        DEBUG(1,1,"Invalid tick handler!!\r\n");

    /*
     * Set US0 as the system tick
     */
    /* set tick  period */
    hal_timer_set_period(SYS_TICK_TIMER, period * 1000);
}


void BSP_Mem_Init()
{
	/*
 	 * Low level memory init
 	 */ 
	hal_mem_init();

	/*
 	 * c run time environment init
 	 */


	/*
 	 * $ init
 	 */
	//hal_cahce_init();

}

#ifndef CONFIG_OSC_SUPPORT
void BSP_Intc_Init()
{
	/*
 	 * Interrupt control init : Int. Vec. table init 
 	 */
	hal_intc_init();

	/*
 	 * Interrupt Priority init #FIXME
 	 */ 
}
#else
void BSP_Intc_Init(){}
#endif

/* 
 *	Setup the hardware ready for system requeried.
 *	1). Memory init
 *	2). Interrupt Control init
 *	2). Tick init
 *	3). FPU init
 */
void BSP_Init(void)
{
	/*
 	 * Memory init
 	 */	 
	BSP_Mem_Init();

    /*
 	 * Interrupt controller init
 	 */ 	
	BSP_Intc_Init();

    /*
     * Initialize console after interrupt is initialized.
     */
    hal_init_console();

    printf("\n\nSystem booting ...\n");

	/*
 	 * System tick init
 	 */
	void *tick_isr_fp;

#if defined(CONFIG_OS_UCOS_II) || defined(CONFIG_OS_UCOS_III)
	extern void OSTickISR();
	tick_isr_fp = OSTickISR;
#elif defined(CONFIG_OS_FREERTOS)
	#include "FreeRTOS.h"
	#if configUSE_PREEMPTION == 0
	extern void vNonPreemptiveTick();
	tick_isr_fp = vNonPreemptiveTick;
	#else
	extern void vPreemptiveTick();
	tick_isr_fp = vPreemptiveTick;
	#endif
#elif defined(CONFIG_OS_THREADX)
	extern void _tx_timer_interrupt();
	tick_isr_fp = _tx_timer_interrupt;
	#if defined(CONFIG_TX_DEMO)
		#if(NO_EXTERNAL_INT_CTL == 0)
		void *tick_isr_fp2;
		extern void test_interrupt_dispatch();
		tick_isr_fp2 = test_interrupt_dispatch;
		#endif
	#endif
#else
	#  error "No valid OS is defined!"
#endif

	/* System tick init */
	BSP_Tmr_TickInit(1, tick_isr_fp);

#if defined(CONFIG_OS_UCOS_III)
        CPU_TS_TmrInit();
#endif

#if defined(CONFIG_OS_THREADX)
	#if defined(CONFIG_TX_DEMO)
		#if(NO_EXTERNAL_INT_CTL == 0)
		/* init timer2 & isr */
		BSP_Tmr_TickInit(0x2, (MB_PCLK / TICK_HZ) / 10, IRQ_SYS_TICK2_VECTOR, tick_isr_fp2);
		#endif
	#endif
#endif
	/*
 	 * FPU init
 	 */ 
/* DavidHu 2010/12/6, enable FPU if the CPU support FPU */
#if defined(__NDS32_EXT_FPU_DP__) || defined(__NDS32_EXT_FPU_SP__)
	if ((GET_CR6() & 0x80000001) == 0x80000001)
	{
		SET_FUCPR(GET_FUCPR() | 0x1);
	}
#endif

	hal_show_sys_info();
}
