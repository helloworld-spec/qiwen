#include <nds32_intrinsic.h>
#include "debug.h"
#include "n12_def.h"
#include "nds32_defs.h" 
#include "ag101_defs.h"
#ifdef CONFIG_PLAT_AG101P_4GB
	/* 4GB Platform  */
	#include "ag101_regs.h"
#else
	/* 16MB Platform */
	#include "ag101p_16mb_regs.h"
#endif

/***********************************
 *	HAL Level : Memory interface
 ***********************************/
#ifdef XIP_MODE
/* This must be a leaf function, no child function */
void _nds32_init_mem(void) __attribute__((no_prologue, optimize("Os")));

void _nds32_init_mem()
{
	// do Low Level Init
}
#endif

/* This must be a leaf function, no child function */
void hal_mem_init( )
{
	
}
#if 0
#define AG101P_SDRAM_INIT()                                         \
{                                                                   \
        /* SDRAM controller - set timing parameter */               \
        NDS32_REG_WRITE(SDRAMC_BASE + PARM1_OFFSET, SDRAMC_PARM1_INIT);\
        NDS32_REG_WRITE(SDRAMC_BASE + PARM2_OFFSET, SDRAMC_PARM2_INIT);\
        /* SDRAM controller - set config register */                \
        NDS32_REG_WRITE(SDRAMC_BASE + CONFIG1_OFFSET, SDRAMC_CONFIG1_INIT);\
        /* refer to ATFSDMC appendix, and boot.S of bootcode */     \
        NDS32_REG_WRITE(SDRAMC_BASE + CONFIG2_OFFSET, SDRAMC_CONFIG2_INIT);\
        NDS32_REG_WAIT4_BIT_OFF(SDRAMC_BASE + CONFIG2_OFFSET, 0x1c);\
        NDS32_REG_WRITE(SDRAMC_BASE + CONFIG2_OFFSET, 0x4);         \
        NDS32_REG_WAIT4_BIT_OFF(SDRAMC_BASE + CONFIG2_OFFSET, 0x1c);\
        NDS32_REG_WRITE(SDRAMC_BASE + CONFIG2_OFFSET, 0x8);         \
        NDS32_REG_WAIT4_BIT_OFF(SDRAMC_BASE + CONFIG2_OFFSET, 0x1c);\
}

#define HAL_MEMORY_SETUP(_base_)                                    \
do {                                                                \
        /* static memory bank 0 timing parameter register */        \
        NDS32_REG_WRITE(SMC_BASE + BANK0TPR_OFFSET, SMC_BANK0TPR_INIT);\
        /* PMU - PLL/DLL Control Register 0 - Enable DLL */         \
        NDS32_SET_BITS(PMUC_BASE + PLLDLLCR0_OFFSET, 0x00010000);   \
        /* Initial SDRAM controller */                              \
        AG101P_SDRAM_INIT();                                        \
        /* Enable SDRAM */                                          \
        NDS32_REG_WRITE(SDRAMC_BASE + 0x10, 0x1000 | (_base_ >> 20));\
        /* RAM (base,size) : (base, 2GB)(4GB mode) / (base, 8MB)(16MB mode) */    \
        NDS32_REG_WRITE(AHBC_BASE + DEVICE6_OFFSET, _base_ | AHBC_DEV6_SIZE_INIT);\
} while(0)

#define HAL_MEMORY_REMAP()                                          \
do {                                                                \
        /* Remapping */                                             \
        /* Bank Enable, BASE = 0x100 for haddr[31:20] of the AHB address bus. */\
        /* External Bank Configuration Registers */                 \
        NDS32_REG_WRITE(SDRAMC_BASE + 0x14, 0x0);                   \
        NDS32_REG_WRITE(SDRAMC_BASE + 0x18, 0x0);                   \
        NDS32_REG_WRITE(SDRAMC_BASE + 0x1c, 0x0);                   \
        /* Bank Enable, BASE = 0x0 for haddr[31:20] of the AHB address bus. */  \
        NDS32_REG_WRITE(SDRAMC_BASE + 0x10, 0x1000);                \
        /* When writing a 1 to this bit, the base/size configuration\
 *          * of AHB slaves 4 (ROM) and 6 (SDRAM/DDR) will be interchanged. */     \
        NDS32_SET_BITS(AHBC_BASE + INTC_OFFSET, 0x1);               \
} while(0)

#define HAL_MEMORY_REMAP_ADJUST()                                   \
do {                                                                \
        NDS32_REG_WRITE(AHBC_BASE + DEVICE6_OFFSET, 0x000 | AHBC_DEV6_SIZE_REMAP);\
} while(0)
#endif

/********************************
 * 	HAL Level : Interrupt
 ********************************/
#if ( NO_EXTERNAL_INT_CTL == 1)
/* 32IVIC without SOC INTC */

/*
 *	mask/unmask priority >= _irqs_ interrupts
 *	used in ISR & gie diable
 */

uint32_t hal_intc_irq_mask(int _irqs_)
{


	uint32_t prv_msk = __nds32__mfsr(NDS32_SR_INT_MASK2);
	if (_irqs_ == -1 )	{	__nds32__mtsr(0, NDS32_SR_INT_MASK2); }		// Disable All
	else if (_irqs_ < 32 )	{	SR_CLRB32(NDS32_SR_INT_MASK2,_irqs_); }		// Disable #_irq_
	else{										// Exception Handle
		DEBUG(1,1,"_irqs_:%d, is invalid!\r\n",_irqs_);
		return -1;
	}
	return prv_msk;
}


void hal_intc_irq_unmask(uint32_t _msk_ )
{
	__nds32__mtsr(_msk_, NDS32_SR_INT_MASK2);
}

void hal_intc_irq_clean(int _irqs_)
{
	if ( _irqs_ == IRQ_SWI_VECTOR )
	{
		SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);
	}
	else
	{
		/* PEND2 is W1C */
		SR_SETB32(NDS32_SR_INT_PEND2,_irqs_);
	}
}

void hal_intc_irq_clean_all()
{
	__nds32__mtsr(-1,NDS32_SR_INT_PEND2);
}

void hal_intc_irq_disable(int _irqs_)
{
	SR_CLRB32(NDS32_SR_INT_MASK2,_irqs_);
}

void hal_intc_irq_disable_all()
{
	__nds32__mtsr(0x0,NDS32_SR_INT_MASK2);
}


void hal_intc_irq_enable(int _irqs_)
{
	SR_SETB32(NDS32_SR_INT_MASK2,_irqs_);
}


void hal_intc_irq_set_priority( uint32_t _prio1_, uint32_t _prio2_ )
{
	__nds32__mtsr(_prio1_, NDS32_SR_INT_PRI);
	__nds32__mtsr(_prio2_, NDS32_SR_INT_PRI2);
}



void hal_intc_irq_config(uint32_t _irq_, uint32_t _edge_, uint32_t _falling_){}

void hal_intc_swi_enable()
{
	SR_SETB32(NDS32_SR_INT_MASK2,IRQ_SWI_VECTOR);
}

void hal_intc_swi_disable()
{
	SR_CLRB32(NDS32_SR_INT_MASK2,IRQ_SWI_VECTOR);
}

uint32_t hal_intc_get_all_pend()
{
	return __nds32__mfsr(NDS32_SR_INT_PEND2);	
}


#else
#define SIZE_OF_PRIORITY_TABLE (sizeof(Int_Priority_Table) / sizeof(int))
/* 	Define Interrupt Priority Table for SW Handle	*/
static int Int_Priority_Table[] = {
	IRQ_TIMER1_VECTOR,
	IRQ_TIMER2_VECTOR,
	IRQ_TIMER3_VECTOR,
	IRQ_APBBRIDGE_VECTOR,
	IRQ_DMA_VECTOR,
	IRQ_I2SAC97_VECTOR,
	IRQ_ENETMAC_VECTOR,
	IRQ_WDT_VECTOR,
	IRQ_EXTINT0_VECTOR,
	IRQ_EXTINT1_VECTOR,
	IRQ_EXTINT2_VECTOR,
	IRQ_EXTINT3_VECTOR,
	IRQ_EXTINT4_VECTOR,
	IRQ_EXTINT5_VECTOR,
	IRQ_CFCDMA_VECTOR,
	IRQ_CFCCD_VECTOR,
	IRQ_USB_VECTOR,
	IRQ_PMU_VECTOR,
	IRQ_RTCSECOND_VECTOR,
	IRQ_RTCALARM_VECTOR,
	IRQ_GPIO_VECTOR,
	IRQ_BTUART_VECTOR,
	IRQ_FTUART_VECTOR,
	IRQ_STUART_VECTOR,
	IRQ_I2C_VECTOR,
	IRQ_SDC_VECTOR,
	IRQ_SSP_VECTOR,
	IRQ_RESERVE23_VECTOR,
	IRQ_RESERVE22_VECTOR,
	IRQ_RESERVE20_VECTOR,
	IRQ_RESERVE9_VECTOR,
	IRQ_RESERVE4_VECTOR,
};

#define hal_intc_irq_disptch OS_CPU_IRQ_ISR_Handler
extern void* OS_CPU_Vector_Table[32];
//void hal_intc_irq_dispatch(void){
void OS_CPU_IRQ_ISR_Handler(void){

        int i;
        int int_no;
        void (*pfnct)(int);
	
	/* Get pending vector number with highest priority */
	for (i = 0; i < SIZE_OF_PRIORITY_TABLE; i++){

		/* Get vector number from prirority table */
		int_no = Int_Priority_Table[i];

		/* Check status register for pending event of the vector */
		if ((1 << int_no) & IN32(INTC_HW1_STATUS))
			break;

	}

	if (i >= SIZE_OF_PRIORITY_TABLE)
		return;
	pfnct = OS_CPU_Vector_Table[int_no];

	if (pfnct) {
		pfnct(int_no);				/* call ISR */
	}
}


/* 	Define Interrupt Priority/Mask Table for SW Handle	*/
static int IRQ_Mask_Table[32] = {
	-1,			//HW0
	-1,			//HW1
	-1,			//HW2
	-1,			//HW3
	-1,			//HW4
	-1,			//HW5
	-1,			//HW6
	-1,			//HW7
	-1,			//HW8
	-1,			//HW9
	-1,			//HW10
	-1,			//HW11
	-1,			//HW12
	(0x1<<IRQ_TIMER1_VECTOR)|(0x1<<IRQ_TIMER2_VECTOR)|(0x1<<IRQ_TIMER3_VECTOR)|(0x1<<IRQ_APBBRIDGE_VECTOR)|(0x1<<IRQ_DMA_VECTOR)|(0x1<<IRQ_I2SAC97_VECTOR)|(0x1<<IRQ_ENETMAC_VECTOR)|(0x1<<IRQ_WDT_VECTOR)|(0x1<<IRQ_EXTINT0_VECTOR),			//HW13 GPIO
	-1,			//HW14
	-1,			//HW15
	-1,			//HW16
	-1,			//HW17
	-1,			//HW18
	0x0,			//HW19	/ TIMER1 / HIGHEST 
	-1,			//HW20
	(0x1<<IRQ_TIMER1_VECTOR)|(0x1<<IRQ_TIMER2_VECTOR)|(0x1<<IRQ_TIMER3_VECTOR)|(0x1<<IRQ_APBBRIDGE_VECTOR),	//HW21
	-1,
	-1,
	(0x1<<IRQ_TIMER1_VECTOR)|(0x1<<IRQ_TIMER2_VECTOR)|(0x1<<IRQ_TIMER3_VECTOR),				//HW24
	(0x1<<IRQ_TIMER1_VECTOR)|(0x1<<IRQ_TIMER2_VECTOR)|(0x1<<IRQ_TIMER3_VECTOR)|(0x1<<IRQ_APBBRIDGE_VECTOR)|(0x1<<IRQ_DMA_VECTOR)|(0x1<<IRQ_I2SAC97_VECTOR),			//HW25  / MAC / 
	-1,					//HW26
	-1,					//HW27
	(0x1<<IRQ_TIMER1_VECTOR)|(0x1<<IRQ_TIMER2_VECTOR)|(0x1<<IRQ_TIMER3_VECTOR)|(0x1<<IRQ_APBBRIDGE_VECTOR)|(0x1<<IRQ_DMA_VECTOR)|(0x1<<IRQ_I2SAC97_VECTOR)|(0x1<<IRQ_ENETMAC_VECTOR)|(0x1<<IRQ_WDT_VECTOR),					//HW28
	-1,				    //HW29
	-1,				    //HW30
	-1				//HW31
	};

/* 6IVIC with SOC INTC 	*/
uint32_t hal_intc_irq_mask(int _irqs_)
{
	unsigned int mask = REG32(INTC_HW1_ER);
	REG32(INTC_HW1_ER) = IRQ_Mask_Table[_irqs_];
	return mask;
}

void hal_intc_irq_unmask(unsigned int _msk_)
{
	REG32(INTC_HW1_ER) = _msk_;
}

void hal_intc_irq_clean(int _irqs_)
{
	
	REG32(INTC_HW1_CLR) = (1 << _irqs_);	
}

void hal_intc_irq_clean_all()
{
	REG32(INTC_HW1_CLR) = -1;
}

void hal_intc_irq_disable(int _irqs_)
{
	REG32(INTC_HW1_ER) = REG32(INTC_HW1_ER) & (~(0x1 << _irqs_));
}

void hal_intc_irq_disable_all()
{
	REG32(INTC_HW1_ER) = 0;
}

void hal_intc_irq_set_priority( uint32_t _prio1_, uint32_t _prio2_)
{
}

void hal_intc_irq_enable(int _irqs_)
{
	REG32(INTC_HW1_ER) = REG32(INTC_HW1_ER) | (0x1 << _irqs_);
}

uint32_t hal_intc_get_all_pend()
{
	return REG32(INTC_HW1_STATUS);
}

void hal_intc_irq_config(uint32_t _irq_, uint32_t _edge_, uint32_t _active_high_)
{
	/* 1: Edge trigger */
	if ( _edge_ )	{REG32(INTC_HW1_TMR) |= (0x1 << _irq_);}
	else		{REG32(INTC_HW1_TMR) &= ~(0x1 << _irq_);}

	/* 1: Active high */
	if ( _active_high_ ){REG32(INTC_HW1_TLR) &= ~(0x1 << _irq_);}
	else		{REG32(INTC_HW1_TLR) |= (0x1 << _irq_);}	
}


void hal_intc_swi_enable()
{
	SR_SETB32(NDS32_SR_INT_MASK,INT_PEND_offSWI);
}

void hal_intc_swi_disable()
{
	SR_CLRB32(NDS32_SR_INT_MASK, INT_PEND_offSWI);
}


#endif /* NO_EXTERNAL_INT_CTL */

void hal_intc_swi_clean()
{
	SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);	
}

void hal_intc_swi_trigger()
{
	SR_SETB32(NDS32_SR_INT_PEND,INT_PEND_offSWI);
}


void hal_intc_init()
{
	extern unsigned long OS_Int_Vectors;
	extern unsigned long OS_Int_Vectors_End;

	volatile unsigned long *vector_srcptr = &OS_Int_Vectors;
	volatile unsigned long *vector_dstptr = (unsigned long *)VECTOR_BASE;

	/* Disable & clean all IRQ interrupts */
	hal_intc_irq_disable_all();
	hal_intc_irq_clean_all();

	/* FIXME  Set interrupt priority */
#if ( NO_EXTERNAL_INT_CTL == 0)
	/* 32IVIC enable CPU interrupt priority */
	/* Apply HW interrupt priority 		*/
//	__nds32__mtsr(0x0,NDS32_SR_INT_CTRL);
//#else
	/* 6IVIC+INTC, config as fixed priority */
	/* backward compatible for SW interrupt priority */
	__nds32__mtsr(0x1,NDS32_SR_INT_CTRL);
#endif
	hal_intc_irq_set_priority(PRI1_DEFAULT, PRI2_DEFAULT);

	/* enable SWI for context switch */
	hal_intc_swi_enable();
	
	/* copy vector table to IVB.base */
	while (vector_srcptr != &OS_Int_Vectors_End)
		*vector_dstptr++ = *vector_srcptr++;


#if (defined(CONFIG_CPU_DCACHE_ENABLE) && !defined(CONFIG_CPU_DCACHE_WRITETHROUGH))
	/* Vector size */
	unsigned long size = (unsigned long)&OS_Int_Vectors_End - (unsigned long)&OS_Int_Vectors;
	/* Vector end */
	unsigned long vector_end = (unsigned long)VECTOR_BASE + size;
	extern void n12_icache_invalidate_range();
	extern void n12_dcache_flush_range();

	/* write back data cache */
	NDS_DCache_Writeback((unsigned long)VECTOR_BASE, vector_end);
	/* invalidate I cache */
	n12_icache_invalidate_range((unsigned long)VECTOR_BASE, vector_end);
#endif

}



/********************************
 * 	TIMER HAL Function	
 ********************************/
static const uint32_t timer_irq[3] = {IRQ_TIMER1_VECTOR,IRQ_TIMER2_VECTOR,IRQ_TIMER3_VECTOR};


uint32_t hal_timer_irq_mask(uint32_t _tmr_ )
{
	return hal_intc_irq_mask(timer_irq[_tmr_-1]);
}
void hal_timer_irq_unmask(uint32_t _msk_ )
{
	hal_intc_irq_unmask(_msk_);
	
}

void hal_timer_irq_clear(uint32_t _tmr_ )
{
	/* Clean IP pending */
#ifndef CONFIG_TX_DEMO
	REG32(TMRC_INTSTATE) = (0x7) << (3*(_tmr_-1));
#endif

	hal_intc_irq_clean(timer_irq[_tmr_-1]);
}

void hal_timer_set_period(uint32_t _tmr_, uint32_t _period_ )
{
	REG32(TMRC_TM_LOAD(_tmr_)) =  _period_;
	REG32(TMRC_TM_COUNTER(_tmr_)) = _period_;
}

void hal_timer_irq_control(uint32_t _tmr_, uint32_t enable )
{
	if (enable)
		REG32(TMRC_INTMASK) = REG32(TMRC_INTMASK) & ~(0x1 <<(3*(_tmr_-1)));
	else
		REG32(TMRC_INTMASK) = REG32(TMRC_INTMASK) | (0x1 <<(3*(_tmr_-1)));
}

void hal_timer_set_upward(uint32_t _tmr_ ,uint32_t up)
{
	if (up)
		REG32(TMRC_TMCR) |= (1UL << (0x8+_tmr_));
	else
		REG32(TMRC_TMCR) &= ~(1UL << (0x8+_tmr_));
}
void hal_timer_start(uint32_t _tmr_)
{
	/* enable tmr & overflow interrupt */	
	REG32(TMRC_TMCR) = REG32(TMRC_TMCR) | (0x101 << (3 * (_tmr_-1)));
}

void hal_timer_stop(uint32_t _tmr_ )
{
	REG32(TMRC_TMCR) = REG32(TMRC_TMCR) & ~(0x1 << (3 * (_tmr_-1)));
}


uint32_t hal_timer_read(uint32_t _tmr_ )
{
	/* By default, timer would decrease from load value to 0 */
	return REG32(TMRC_TM_LOAD(_tmr_)) - REG32(TMRC_TM_COUNTER(_tmr_));
}

uint32_t hal_timer_count_read(uint32_t _tmr_ )
{
	return REG32(TMRC_TM_COUNTER(_tmr_));
}

/*
 *	Initialize tmr register
 *	_tmr_ :
 *		0	--> all
 *		1 ~ 3	--> timer 1 ~ 3
 */
void hal_timer_init(uint32_t _tmr_ )
{
	if ( _tmr_ == 0 )
	{
		REG32(TMRC_TMCR) = 0;		 /* disable all timer */
		REG32(TMRC_TM_MATCH1(1)) = 0;
		REG32(TMRC_TM_MATCH1(2)) = 0;
		REG32(TMRC_TM_MATCH1(3)) = 0;
		REG32(TMRC_INTMASK) = 0xFFFFFFFF; /* disable all timer interrupt */
		REG32(TMRC_INTSTATE) = 0xFFFFFFFF;/* clear pending events */
	}
	else if ( _tmr_ <= 3 )
	{
		REG32(TMRC_TMCR) = REG32(TMRC_TMCR) & ~(0x3 << (3* (_tmr_-1)));
		REG32(TMRC_TM_MATCH1(_tmr_)) = 0;
		REG32(TMRC_INTMASK) = REG32(TMRC_INTMASK) & ~(0x3 << (3* (_tmr_-1)));
		REG32(TMRC_INTSTATE) = REG32(TMRC_INTSTATE) & ~(0x3 << (3* (_tmr_-1)));
	}
	else
		/* invalid tmr id */
		DEBUG(1,1,"Invalid tmr[%d]\r\n",_tmr_);
}

uint32_t hal_timer_irq_status(uint32_t _tmr_)
{
//	return REG32(TMRC_INTSTATE)&(0x7 << (3*(_tmr_)));
	return REG32(TMRC_INTSTATE);

}


