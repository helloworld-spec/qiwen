#include <nds32_intrinsic.h>
#include "debug.h"
#include "n12_def.h"
#include "nds32_defs.h" 
#include "ae210p_defs.h"
#include "ae210p_regs.h"

/***********************************
 *	HAL Level : Memory interface
 ***********************************/
#ifdef XIP_MODE
/* This must be a leaf function, no child function */
void _nds32_init_mem(void) __attribute__((no_prologue, optimize("Os")));

void _nds32_init_mem()
{
	// do Low Level Init
	//
	//
	__nds32__mtsr(EDLM_BASE|0x1,NDS32_SR_DLMB);
	__nds32__dsb();


}
#endif

/* This must be a leaf function, no child function */
void hal_mem_init( )
{
	#define MEMCPY(des, src, n)     __builtin_memcpy ((des), (src), (n))
	/* Copy RW From LMA to VMA */
	extern char __rw_lma_start;
	extern char __rw_lma_end;
	extern char __rw_vma_start;
	unsigned int size = &__rw_lma_end - &__rw_lma_start;
	MEMCPY(&__rw_vma_start, &__rw_lma_start, size);

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
/* 32IVIC without SOC INTC */

/*
 *	mask/unmask priority >= _irqs_ interrupts
 *	used in ISR & gie diable
 */
uint32_t hal_intc_irq_mask(int _irqs_)
{
	uint32_t prv_msk = __nds32__mfsr(NDS32_SR_INT_MASK2);
	if (_irqs_ == -1 )
	{
		__nds32__mtsr(0, NDS32_SR_INT_MASK2);
	}
	else if (_irqs_ < 32 )
	{
		SR_CLRB32(NDS32_SR_INT_MASK2,_irqs_);
	}
	else
	{
		DEBUG(1,1,"_irqs_:%d, is invalid!\r\n",_irqs_);
		return -1;
	}

	return prv_msk;
}

void hal_intc_irq_unmask(uint32_t _msk_)
{
	__nds32__mtsr( _msk_ , NDS32_SR_INT_MASK2);
}


void hal_intc_irq_clean(int _irqs_)
{
	if ( _irqs_ == IRQ_SWI_VECTOR )
		SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);
	/* PEND2 is W1C */
	SR_SETB32(NDS32_SR_INT_PEND2,_irqs_);
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



void hal_intc_irq_config(uint8_t _irq_, uint8_t _edge_, uint8_t _falling_){}

void hal_intc_swi_enable()
{
	//SR_SETB32(NDS32_SR_INT_MASK,16);
	SR_SETB32(NDS32_SR_INT_MASK2,IRQ_SWI_VECTOR);
	
}

void hal_intc_swi_disable()
{
	SR_CLRB32(NDS32_SR_INT_MASK2,IRQ_SWI_VECTOR);
}



void hal_intc_swi_clean()
{
	SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);	
}

void hal_intc_swi_trigger()
{
	SR_SETB32(NDS32_SR_INT_PEND,INT_PEND_offSWI);
}

uint32_t hal_intc_get_all_pend()
{
	return __nds32__mfsr(NDS32_SR_INT_PEND2);	
}

void hal_intc_init()
{
	/* 
 	 * INT_MASK2 init
	 * enable PIT(2) as system tick
 	 *        SWI(11) handle task context switch
 	 */
	SR_SETB32(NDS32_SR_INT_MASK2,IRQ_PIT_VECTOR);
	SR_SETB32(NDS32_SR_INT_MASK2,IRQ_SWI_VECTOR);


#ifdef CONFIG_HW_PRIO_SUPPORT
	/*
 	 * INT_RPI2 init
 	 * 	PIT(2) with highest prio.
 	 * 	SWI(11) with lowest prio.
 	 */
	hal_intc_irq_set_priority(0xFFFFFFCF,0xFFFFFFFF);
#endif
}



/********************************
 * 	TIMER HAL Function	
 ********************************/
static const uint8_t timer_irq[4] = {IRQ_PIT_VECTOR, IRQ_PIT_VECTOR, IRQ_PIT_VECTOR, IRQ_PIT_VECTOR};


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
	/* Clean IP pending, W1C */
#ifndef CONFIG_TX_DEMO
	REG32(PIT_INT_ST) = (0x1 << (5*(_tmr_-1)));
#endif

	hal_intc_irq_clean(timer_irq[_tmr_-1]);
}

void hal_timer_set_period(uint32_t _tmr_, uint32_t _period_ )
{
	REG32(PIT_CHNx_LOAD(_tmr_-1)) = _period_;
	//REG32(PIT_CHNx_COUNT(_tmr_-1))= _period_;
}

void hal_timer_irq_control(uint32_t _tmr_, uint32_t enable )
{
	if (enable)
		REG32(PIT_INT_EN) = REG32(PIT_INT_EN) | (0x1 << (5*(_tmr_-1)));
	else
		REG32(PIT_INT_EN) = REG32(PIT_INT_EN) & ~(0x1 << (5*(_tmr_-1)));
}

void hal_timer_set_upward(uint32_t _tmr_ ,uint32_t up)
{
	if ( up )
		DEBUG(1,1,"PIT Timer only support downward!\r\n");
	
}
void hal_timer_start(uint32_t _tmr_)
{
	/* 	config channel mode 	 */
	/* 	32 bits timer, APB clock */
	REG32(PIT_CHNx_CTL(_tmr_-1)) = ( PIT_CH_CTL_APBCLK | PIT_CH_CTL_TMR32  );
	/* 	enable channel 	 */	
	REG32(PIT_CH_EN) = REG32(PIT_CH_EN) | (0x1 << (5*(_tmr_-1)));
}

void hal_timer_stop(uint32_t _tmr_ )
{
	REG32(PIT_CH_EN) = REG32(PIT_CH_EN) & ~(0x1 << (5*(_tmr_-1))); 	
}


uint32_t hal_timer_read(uint32_t _tmr_ )
{
	/* By default, timer  would decrease from load value to 0 */
	return REG32( PIT_CHNx_LOAD(_tmr_-1) ) - REG32( PIT_CHNx_COUNT(_tmr_-1) );
}

uint32_t hal_timer_count_read(uint32_t _tmr_ )
{
	return REG32( PIT_CHNx_COUNT(_tmr_-1) );
}

/*
 *	Initialize tmr register
 *	_tmr_ :
 *		0	--> all
 *		1 ~ 4	--> timer 1 ~ 4
 */
void hal_timer_init(uint32_t _tmr_ )
{
	if ( _tmr_ == 0 )
	{

		uint32_t NumCh = REG32(PIT_CFG) & PIT_CH_NUM_MASK;
		if ( NumCh > 0 )
		{
			REG32(PIT_INT_EN) = 0x0;   	 /* disable all timer interrupt */
			REG32(PIT_CH_EN)  = 0x0;	 /* disable all timer */
			REG32(PIT_INT_ST) = 0x7;	 /* clear pending events */
		
			REG32( PIT_CHNx_LOAD(0) ) = 0x0;	/* clean channel 0 reload */
			REG32( PIT_CHNx_LOAD(1) ) = 0x0;  	/* clean channel 1 reload */
			REG32( PIT_CHNx_LOAD(2) ) = 0x0;  	/* clean channel 2 reload */
			REG32( PIT_CHNx_LOAD(3) ) = 0x0;  	/* clean channel 3 reload */
			
		}
		else
			DEBUG(1,1,"Timer init fail, no avalible channel\r\n");
	
	}
	else if ( _tmr_ <= 4 )
	{
			REG32(PIT_INT_EN) = REG32(PIT_INT_EN) & ~(0x1 << (5*(_tmr_-1)));
			REG32(PIT_CH_EN) = REG32(PIT_INT_EN) & ~(0x1 << (5*(_tmr_-1)));
			REG32(PIT_INT_ST) = (0x1 << (5*(_tmr_-1)));	
	}
	else
		/* invalid tmr id */
		DEBUG(1,1,"Invalid tmr[%d]\r\n",_tmr_);
}

uint32_t hal_timer_irq_status(uint32_t _tmr_)
{
	/* return PIT int status	*/
	/* PIT need #channel & #timer 	*/
	/* just return all int status	*/
	return REG32(PIT_INT_ST);
}



