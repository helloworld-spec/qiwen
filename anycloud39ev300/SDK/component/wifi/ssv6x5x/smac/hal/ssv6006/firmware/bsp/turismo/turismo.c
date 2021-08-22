#include <nds32_intrinsic.h>
#include "debug.h"
#include "n12_def.h"
#include "nds32_defs.h" 
#include "turismo_defs.h"
#include "turismo_regs.h"
#include <timer/drv_timer.h>
#include <efuse/drv_efuse.h>
#include <intc/intc.h>
#include <uart/drv_uart.h>
#include <hal.h>
#include <ssv6xxx_common.h>
#include <soc_config.h>
#include "ssv_chip.h"

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
    if (_irqs_ == -1 ) // Disable All
    {
        __nds32__mtsr(0, NDS32_SR_INT_MASK2);
        intc_mask_all();
    }
    else if ((_irqs_ >= 0) && (_irqs_ < 63)) // Disable #_irq_
    {
        if (_irqs_ < 32)
            SR_CLRB32(NDS32_SR_INT_MASK2, _irqs_);
        intc_int_mask(_irqs_);
    }
	else // Exception Handle
	{
		DEBUG(1,1,"_irqs_:%d, is invalid!\r\n",_irqs_);
		return -1;
	}
	return prv_msk;
}

void hal_intc_irq_unmask(uint32_t _msk_ )
{
	__nds32__mtsr(_msk_, NDS32_SR_INT_MASK2);
	// Freddie ToDo: unmask with external interrupt controller
}

void hal_intc_irq_clean(int _irqs_)
{
#if 0
	if ( _irqs_ == IRQ_SWI_VECTOR )
	{
		SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);
	}
	else
#endif
	{
		/* PEND2 is W1C */
	    if (_irqs_ < 32)
	    {
	        if (_irqs_ == IRQ_SWI)
	            SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);
		    SR_SETB32(NDS32_SR_INT_PEND2, _irqs_);
        }
		intc_int_clear(_irqs_);
	}
}

void hal_intc_irq_clean_all()
{
	__nds32__mtsr(-1, NDS32_SR_INT_PEND2);
	intc_clear_all();
}

void hal_intc_irq_disable(int _irqs_)
{
    if (_irqs_ < 32)
        SR_CLRB32(NDS32_SR_INT_MASK2,_irqs_);
    intc_mask(_irqs_);
}

void hal_intc_irq_disable_all()
{
	__nds32__mtsr(0x0,NDS32_SR_INT_MASK2);
	intc_mask_all();
}


void hal_intc_irq_enable(int _irqs_)
{
    if (_irqs_ < 32)
	    SR_SETB32(NDS32_SR_INT_MASK2,_irqs_);
    intc_int_unmask(_irqs_);
}

void hal_intc_irq_set_priority( uint32_t _prio1_, uint32_t _prio2_ )
{
	__nds32__mtsr(_prio1_, NDS32_SR_INT_PRI);
	__nds32__mtsr(_prio2_, NDS32_SR_INT_PRI2);
}

void hal_intc_irq_config(uint32_t _irq_, uint32_t _edge_, uint32_t _falling_)
{
}

void hal_intc_swi_enable()
{
    SR_SETB32(NDS32_SR_INT_MASK2, IRQ_SWI);
    SR_SETB32(NDS32_SR_INT_MASK, INT_PEND_offSWI);
    // SW_INT is connected to Turismo's interrupt controller.
    intc_int_unmask(IRQ_SWI);
}

void hal_intc_swi_disable()
{
    SR_CLRB32(NDS32_SR_INT_MASK2, IRQ_SWI);
    SR_CLRB32(NDS32_SR_INT_MASK, INT_PEND_offSWI);
    intc_int_mask(IRQ_SWI);
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
        gOsFromISR++;
		pfnct(int_no);				/* call ISR */
        gOsFromISR--;
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
	SR_SETB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);
}

void hal_intc_init()
{
	extern unsigned long OS_Int_Vectors;
	extern unsigned long OS_Int_Vectors_End;

	volatile unsigned long *vector_srcptr = &OS_Int_Vectors;
	volatile unsigned long *vector_dstptr = (unsigned long *)VECTOR_BASE;

	intc_init();

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
    /* enable peripheral interrupt */
    hal_intc_irq_enable(IRQ_SPI_SDIO_WAKE);
	hal_intc_irq_enable(IRQ_15_GROUP);
    hal_intc_irq_enable(IRQ_31_GROUP);

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
 *  TIMER HAL Functio
 ********************************/

uint32_t hal_timer_irq_mask(uint32_t _tmr_)
{
    if ((_tmr_ >= TIMER_ID_MIN) && (_tmr_ <= TIMER_ID_MAX))
        return hal_intc_irq_mask(hwtmr_get_irq(_tmr_));
    else
        return (-1);
}

void hal_timer_irq_unmask (uint32_t _tmr_ )
{
    hal_intc_irq_unmask(_tmr_);
}

void hal_timer_irq_clear (uint32_t _tmr_)
{
    /* Clean IP pending */
    hwtmr_clear_irq(_tmr_);
    if ((_tmr_ >= TIMER_ID_MIN) && (_tmr_ <= TIMER_ID_MAX))
        hal_intc_irq_clean(hwtmr_get_irq(_tmr_));
}

void hal_timer_set_period (uint32_t _tmr_, uint32_t _period_ )
{
    hwtmr_init(_tmr_, _period_, HTMR_PERIODIC);
}

void hal_timer_irq_control(uint32_t _tmr_, uint32_t enable )
{
     hwtmr_enable(_tmr_, enable);
}

void hal_timer_set_upward(uint32_t _tmr_ ,uint32_t up)
{
}

void hal_timer_start (uint32_t _tmr_)
{
    hwtmr_enable(_tmr_, 1);
}

void hal_timer_stop (uint32_t _tmr_)
{
    hwtmr_enable(_tmr_, 0);
}

uint32_t hal_timer_read(uint32_t _tmr_ )
{
	/* By default, timer would decrease from load value to 0 */
	return hwtmr_get_count(_tmr_);
}

uint32_t hal_timer_count_read(uint32_t _tmr_ )
{
	return hwtmr_read_count(_tmr_);
}

/*
 *	Initialize tmr register
 *	_tmr_ :
 *		0	--> all
 *		1 ~ 3	--> timer 1 ~ 3
 */
void hal_timer_init(uint32_t _tmr_ )
{
}

uint32_t hal_timer_irq_status(uint32_t _tmr_)
{
//  return REG32(TMRC_INTSTATE)&(0x7 << (3*(_tmr_)));
    return hwtmr_get_status(_tmr_);
}


static void _serial_init (eBaud baud, eDataBits data_bits, eStopBits stop_bits, eParity parity)
{
    drv_uart_init(0);
    //drv_uart_initialize();
}

static void _serial_register_rx_int (void (*irq_func)(void))
{
    intc_group31_enable(IRQ_31_UART_DBG_RX, irq_func);
}

static void _serial_deregister_rx_int (void)
{
    intc_group31_disable(IRQ_31_UART_DBG_RX);
}

static void _serial_register_tx_int (void (*irq_func)(void))
{
    intc_group31_enable(IRQ_31_UART_DBG_TX, irq_func);
}

static void _serial_deregister_tx_int (void)
{
    intc_group31_disable(IRQ_31_UART_DBG_TX);
}

static sSerialDrv   console_serial_drv = { .init              = _serial_init,
                                           //.tx_char           = drv_uart_put_char,
                                           .tx_char           = drv_uart_tx_0,
                                           .rx_char           = drv_uart_rx_0,
                                           .tx_multi          = drv_uart_multiple_tx_0,
                                           .get_fifo_length   = drv_uart_get_uart_fifo_length_0,
                                           .is_tx_fifo_full   = drv_uart_is_tx_busy_0,
                                           .register_tx_int   = _serial_register_tx_int,
                                           .deregister_tx_int = _serial_deregister_tx_int,
                                           .register_rx_int   = _serial_register_rx_int,
                                           .deregister_rx_int = _serial_deregister_rx_int,
                                           .enable_tx_int     = drv_uart_enable_tx_int_0,
                                           .disable_tx_int    = drv_uart_disable_tx_int_0,
                                           .enable_rx_int     = drv_uart_enable_rx_int_0,
                                           .disable_rx_int    = drv_uart_disable_rx_int_0
};

void hal_init_console (void)
{
    hal_console_init(&console_serial_drv);
}

void hal_show_sys_info (void)
{
    // Variables from memory manager
    extern void *g_heap_start;
    extern void *g_heap_end;
    // LD variables
    extern u32 end; // In turismo.ld
    extern u32 __executable_start;

    printf("System memory layout:\n");
    printf("\tImage: 0 - %08x (%d B)\n", 0, (u32)&__executable_start, (u32)&end);
    printf("\tHeap : %08x - %08x (%d B)\n", g_heap_start, g_heap_end, configTOTAL_HEAP_SIZE);
    printf("\nIRQ mask:CPU:0x%08X-INTC:0x%08X-MAP15:0x%08X-MAP31:0x%08X \n",
         __nds32__mfsr(NDS32_SR_INT_MASK2), GET_MASK_TYPMCU_INT_MAP, GET_MASK_TYPMCU_INT_MAP_15, GET_MASK_TYPMCU_INT_MAP_31);
    //sys_init_prnf("IRQ mode: 0x%08x\n", REG_INTR_CTRL->INT_MODE);

    {
        u32 efuse_chip_identity;
        read_efuse_identify(&efuse_chip_identity);
        printf("\nE-fuse block 0 [%08x]\n", efuse_chip_identity);
    }
    printf("Firmware is built for for chip ID [%08x]\n\n", CHIP_IDENTITY);

    printf("OS version: %s\n", tskKERNEL_VERSION_NUMBER);
    
    #if (CONFIG_SIM_PLATFORM == 0)
    printf("%s FW built at %s %s\n", CONFIG_PRODUCT_STR, __DATE__, __TIME__);
    #endif

    #include <ssv_version.h>
    printf("Firmware:\n");
    printf("  SVN\n"
           "    URL: %s\n"
           "    Rev: %u\n",
           SSV_FIRMWARE_URl, ssv_firmware_version);
    printf("  Compile:\n"
           "    On: %s\n"
           "    OS: %s / %s\n"
           "    At: %s\n",
           FRIMWARE_COMPILERHOST, FRIMWARE_COMPILEROS, FRIMWARE_COMPILEROSARCH, FRIMWARE_COMPILERDATE);

} // end of - hal_show_sys_info -


extern u32 firmware_checksum;
extern u32 firmware_block_count;

static void _fw_checksum (void)
{
    volatile u32 *FW_RDY_REG = (volatile u32 *)(SPI_REG_BASE + 0x10);
    u32          block_count = (*FW_RDY_REG >> 16) & 0x0FF;
    u32          fw_checksum = FW_CHECKSUM_INIT;
    u32         *addr = 0;
    u32          total_words = block_count * CHECKSUM_BLOCK_SIZE / sizeof(u32);


    //printf("Calculating checksum from %d blocks...", block_count);
    // Don't calculate checksum if host does not provide block count.
    if (block_count == 0)
    {
        //printf("skip.\n");
    	firmware_block_count = 0;
        return;
    }

    while (total_words--) {
        fw_checksum += *addr++;
    }
    fw_checksum = ((fw_checksum >> 24) + (fw_checksum >> 16) + (fw_checksum >> 8) + fw_checksum) & 0x0FF;

    //printf("done with %02X.\n", fw_checksum);

    fw_checksum = (fw_checksum << 16) & 0x00FF0000;

    *FW_RDY_REG = (*FW_RDY_REG & 0xFF00FFFF) | fw_checksum;

    firmware_checksum = fw_checksum;
    firmware_block_count = block_count;
}

void HAL_pre_init (void)
{
	_fw_checksum();
}
