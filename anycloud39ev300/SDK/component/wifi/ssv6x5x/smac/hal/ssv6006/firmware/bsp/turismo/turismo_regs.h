/*****************************************************************************
 *
 *            Copyright Andes Technology Corporation 2007-2008
 *                         All Rights Reserved.
 *
 *  Revision History:
 *
 *    Aug.21.2007     Created.
 ****************************************************************************/

#ifndef __TURISMO_REGS_H__
#define __TURISMO_REGS_H__

#include "turismo_cfg.h"
#ifndef __ASSEMBLER__
#include <inttypes.h>
#include <nds32_intrinsic.h>
#endif

#include <types.h>
#include <ssv6006C_reg.h>


/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/
#if( NO_EXTERNAL_INT_CTL == 1 )
    #include "irq.h"

    /* IRQ priority */
    #define LOWEST              (3)
    #define LOW                 (2)
    #define HIGH                (1)
    #define HIGHEST             (0)

    #define IRQ_PRIO(irq)            (irq##_PRIO << (2 * (((irq) >= 16) ? ((irq) - 16): (irq))))

    #define IRQ_WDT_PRIO                        LOWEST
    #define IRQ_SWI_PRIO                        LOWEST
    #define IRQ_SPI_SDIO_WAKE_PRIO              LOWEST
    #define IRQ_SPI_DONE_PRIO                   LOWEST
    #define IRQ_IPC_PRIO                        LOWEST
    #define IRQ_BTCX_PRIO                       LOWEST
    #define IRQ_FLASH_DMA_DONE_PRIO             LOWEST
    #define IRQ_BEACON_DONE_PRIO                LOWEST
    #define IRQ_BEACON_DTIM_PRIO                LOWEST
    #define IRQ_FBUSDMA_PRIO                    LOWEST
    #define IRQ_DMA_PRIO                        LOWEST
    #define IRQ_USB_PRIO                        HIGH
    #define IRQ_RTC_PRIO                        LOWEST
    #define IRQ_SYSCTL_PRIO                     LOWEST
    #define IRQ_PMU_PRIO                        LOWEST
    #define IRQ_15_GROUP_PRIO                   LOW

    #define IRQ_I2C_PRIO                        LOWEST
    #define IRQ_MTX_STAT_INT_PRIO               LOWEST
    #define IRQ_CPU_INT_ALT_PRIO                LOWEST
    #define IRQ_MBOX_PRIO                       HIGHEST
    #define IRQ_US_TIMER0_PRIO                  HIGH
    #define IRQ_US_TIMER1_PRIO                  LOW
    #define IRQ_US_TIMER2_PRIO                  LOW
    #define IRQ_US_TIMER3_PRIO                  LOW
    #define IRQ_MS_TIMER0_PRIO                  LOW
    #define IRQ_MS_TIMER1_PRIO                  LOW
    #define IRQ_MS_TIMER2_PRIO                  LOW
    #define IRQ_MS_TIMER3_PRIO                  LOW
    #define IRQ_MRX_PRIO                        LOWEST
    #define IRQ_HCI_PRIO                        LOWEST
    #define IRQ_CO_DMA_PRIO                     LOWEST
    #define IRQ_31_GROUP_PRIO                   LOW   


    #define PRI1_DEFAULT            (   IRQ_PRIO(IRQ_WDT) \
                                      | IRQ_PRIO(IRQ_SWI) \
                                      | IRQ_PRIO(IRQ_SPI_SDIO_WAKE) \
                                      | IRQ_PRIO(IRQ_SPI_DONE) \
                                      | IRQ_PRIO(IRQ_IPC) \
                                      | IRQ_PRIO(IRQ_BTCX) \
                                      | IRQ_PRIO(IRQ_FLASH_DMA_DONE) \
                                      | IRQ_PRIO(IRQ_BEACON_DONE) \
                                      | IRQ_PRIO(IRQ_BEACON_DTIM) \
                                      | IRQ_PRIO(IRQ_FBUSDMA) \
                                      | IRQ_PRIO(IRQ_DMA) \
                                      | IRQ_PRIO(IRQ_USB) \
                                      | IRQ_PRIO(IRQ_RTC) \
                                      | IRQ_PRIO(IRQ_SYSCTL) \
                                      | IRQ_PRIO(IRQ_PMU) \
                                      | IRQ_PRIO(IRQ_15_GROUP))

    #define PRI2_DEFAULT            (   IRQ_PRIO(IRQ_I2C) \
                                      | IRQ_PRIO(IRQ_MTX_STAT_INT) \
                                      | IRQ_PRIO(IRQ_CPU_INT_ALT) \
                                      | IRQ_PRIO(IRQ_MBOX) \
                                      | IRQ_PRIO(IRQ_US_TIMER0) \
                                      | IRQ_PRIO(IRQ_US_TIMER1) \
                                      | IRQ_PRIO(IRQ_US_TIMER2) \
                                      | IRQ_PRIO(IRQ_US_TIMER3) \
                                      | IRQ_PRIO(IRQ_MS_TIMER0) \
                                      | IRQ_PRIO(IRQ_MS_TIMER1) \
                                      | IRQ_PRIO(IRQ_MS_TIMER2) \
                                      | IRQ_PRIO(IRQ_MS_TIMER3) \
                                      | IRQ_PRIO(IRQ_MRX) \
                                      | IRQ_PRIO(IRQ_HCI) \
                                      | IRQ_PRIO(IRQ_CO_DMA) \
                                      | IRQ_PRIO(IRQ_31_GROUP))
    

#else                                                   
	#define IRQ_CFCCD_VECTOR		0
	#define IRQ_CFCDMA_VECTOR		1
	#define IRQ_SSP_VECTOR			2
	#define IRQ_I2C_VECTOR			3
	#define IRQ_RESERVE4_VECTOR		4
	#define IRQ_SDC_VECTOR			5
	#define IRQ_I2SAC97_VECTOR		6
	#define IRQ_STUART_VECTOR		7
	#define IRQ_PMU_VECTOR			8
	#define IRQ_RESERVE9_VECTOR		9
	#define IRQ_FTUART_VECTOR		10
	#define IRQ_BTUART_VECTOR		11
	#define IRQ_EXTINT5_VECTOR		12
	#define IRQ_GPIO_VECTOR			13
	#define IRQ_TIMER2_VECTOR		14
	#define IRQ_TIMER3_VECTOR		15
	#define IRQ_WDT_VECTOR			16
	#define IRQ_RTCALARM_VECTOR		17
	#define IRQ_RTCSECOND_VECTOR    18
	#define IRQ_TIMER1_VECTOR		19
	#define IRQ_RESERVE20_VECTOR	20
	#define IRQ_DMA_VECTOR			21
	#define IRQ_RESERVE22_VECTOR	22
	#define IRQ_RESERVE23_VECTOR	23
	#define IRQ_APBBRIDGE_VECTOR	24
	#define IRQ_ENETMAC_VECTOR		25
	#define IRQ_USB_VECTOR			26
	#define IRQ_EXTINT4_VECTOR		27
	#define IRQ_EXTINT0_VECTOR		28
	#define IRQ_EXTINT1_VECTOR		29
	#define IRQ_EXTINT2_VECTOR		30
	#define IRQ_EXTINT3_VECTOR		31

	#define PRI1_DEFAULT		0xffffffff
	#define PRI2_DEFAULT		0xffffffff
#endif

/*****************************************************************************
 * UARTC - AG101 DMA APB
 ****************************************************************************/
/* Device base address */
#define STUARTC_BASE			UART_REG_BASE /* standard/IR UART */

/* UART register offsets (4~8-bit width) */
/* SD_LCR_DLAB == 0 */
#define UARTC_RBR_OFFSET		0x00 /* receiver biffer register */
#define UARTC_THR_OFFSET		0x00 /* transmitter holding register */
#define UARTC_IER_OFFSET		0x04 /* interrupt enable register */
#define UARTC_IIR_OFFSET		0x08 /* interrupt identification register */
#define UARTC_FCR_OFFSET		0x08 /* FIFO control register */
#define UARTC_LCR_OFFSET		0x0c /* line control regitser */
#define UARTC_MCR_OFFSET		0x10 /* modem control register */
#define UARTC_LSR_OFFSET		0x14 /* line status register */
#define UARTC_TST_OFFSET		0x14 /* testing register */
#define UARTC_MSR_OFFSET		0x18 /* modem status register */
#define UARTC_SPR_OFFSET		0x1c /* scratch pad register */

/* SD_LCR_DLAB == 0 */
#define UARTC_DLL_OFFSET		0x00 /* baudrate divisor latch LSB */
#define UARTC_DLM_OFFSET		0x04 /* baudrate divisor latch MSB */
#define UARTC_PSR_OFFSET		0x08 /* prescaler register */

#define UARTC_MDR_OFFSET		0x20 /* mode definition register */
#define UARTC_ACR_OFFSET		0x24 /* auxiliary control register */


/*****************************************************************************
 * Macros for Register Access
 ****************************************************************************/
#define REG32(reg)		(  *( (volatile uint32_t *) (reg) ) )
#define REG32_R(_REG_) (*(volatile u32 *) _REG_)
#define REG32_W(_REG_,_val_) (*(volatile u32 *) _REG_ = _val_)


#ifdef REG_IO_HACK

/* 8 bit access */
//#define IN8(reg)			(  *( (volatile uint8_t *) (reg) ) )
#define OUT8(reg, data)			( (*( (volatile uint8_t *) (reg) ) ) = (uint8_t)(data) )

#define CLR8(reg)			(  *( (volatile uint8_t *) (reg) ) = (uint8_t)0 )
#define MASK8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) & (uint8_t)(mask) )
#define UMSK8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) )

#define SETR8SHL(reg, mask, shift, v)	(  *( (volatile uint8_t *) (reg) ) = \
					( ( *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) ) | \
					( ( (uint8_t)(v) << (shift) ) & (uint8_t)(mask)          ) ) )

#define SETR8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) = \
					( ( *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) ) | (uint8_t)(mask) ) )

#define CLRR8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) &= ~( (uint8_t)(mask) ) )

#define SETB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) |= (uint8_t)( (uint8_t)1 << (bit) ) )
#define CLRB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) &= ( ~( (uint8_t) ( (uint8_t)1 << (bit) ) ) ) )
#define GETB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) & (uint8_t) ( (uint8_t)1 << (bit) ) )
#define GETB8SHR(reg, bit)		( (*( (volatile uint8_t *) (reg) ) & (uint8_t) ( (uint8_t)1 << (bit) )) >> (bit) )

/* 16 bit access */
#define IN16(reg)			(  *( (volatile uint16_t *) (reg) ) )
#define OUT16(reg, data)		( (*( (volatile uint16_t *) (reg) ) ) = (uint16_t)(data) )

#define CLR16(reg)			(  *( (volatile uint16_t *) (reg) ) = (uint16_t)0 )
#define MASK16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) & (uint16_t)(mask) )
#define UMSK16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) )

#define SETR16SHL(reg, mask, shift, v)	(  *( (volatile uint16_t *) (reg) ) = \
					( ( *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) ) | \
					( ( (uint16_t)(v) << (shift) ) & (uint16_t)(mask)          ) ) )

#define SETR16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) = \
					( ( *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) ) | (uint16_t)(mask) ) )

#define CLRR16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) &= ~( (uint16_t)(mask) ) )

#define SETB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) |= (uint16_t)( (uint16_t)1 << (bit) ) )
#define CLRB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) &= ( ~( (uint16_t) ( (uint16_t)1 << (bit) ) ) ) )
#define GETB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) & (uint16_t) ( (uint16_t)1 << (bit) ) )
#define GETB16SHR(reg, bit)		( (*( (volatile uint16_t *) (reg) ) & (uint16_t) ( (uint16_t)1 << (bit) )) >> (bit) )

/* 32 bit access */
#define IN32(reg)			_IN32((uint32_t)(reg))
#define OUT32(reg, data)		_OUT32((uint32_t)(reg), (uint32_t)(data))

#define CLR32(reg)			_CLR32((uint32_t)(reg))
#define MASK32(reg, mask)		_MASK32((uint32_t)(reg), (uint32_t)(mask))
#define UMSK32(reg, mask)		_UMSK32((uint32_t)(reg), (uint32_t)(mask))

#define SETR32SHL(reg, mask, shift, v)	_SETR32SHL((uint32_t)(reg), (uint32_t)(mask), (uint32_t)(shift), (uint32_t)(v))
#define SETR32(reg, mask)		_SETR32((uint32_t)(reg), (uint32_t)(mask))
#define CLRR32(reg, mask)		_CLRR32((uint32_t)(reg), (uint32_t)(mask))

#define SETB32(reg, bit)		_SETB32((uint32_t)(reg), (uint32_t)(bit))
#define CLRB32(reg, bit)		_CLRB32((uint32_t)(reg), (uint32_t)(bit))
#define GETB32(reg, bit)		_GETB32((uint32_t)(reg), (uint32_t)(bit))
#define GETB32SHR(reg, bit)		_GETB32SHR((uint32_t)(reg), (uint32_t)(bit))

#else /* REG_IO_HACK */

/* 8 bit access */
//#define IN8(reg)			(  *( (volatile uint8_t *) (reg) ) )
#define OUT8(reg, data)			( (*( (volatile uint8_t *) (reg) ) ) = (uint8_t)(data) )

#define CLR8(reg)			(  *( (volatile uint8_t *) (reg) ) = (uint8_t)0 )
#define MASK8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) & (uint8_t)(mask) )
#define UMSK8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) )

#define SETR8SHL(reg, mask, shift, v)	(  *( (volatile uint8_t *) (reg) ) = \
					( ( *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) ) | \
					( ( (uint8_t)(v) << (shift) ) & (uint8_t)(mask)          ) ) )

#define SETR8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) = \
					( ( *( (volatile uint8_t *) (reg) ) & ~( (uint8_t)(mask) ) ) | (uint8_t)(mask) ) )

#define CLRR8(reg, mask)		(  *( (volatile uint8_t *) (reg) ) &= ~( (uint8_t)(mask) ) )

#define SETB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) |= (uint8_t)( (uint8_t)1 << (bit) ) )
#define CLRB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) &= ( ~( (uint8_t) ( (uint8_t)1 << (bit) ) ) ) )
#define GETB8(reg, bit)			(  *( (volatile uint8_t *) (reg) ) & (uint8_t) ( (uint8_t)1 << (bit) ) )
#define GETB8SHR(reg, bit)		( (*( (volatile uint8_t *) (reg) ) & (uint8_t) ( (uint8_t)1 << (bit) )) >> (bit) )

/* 16 bit access */
#define IN16(reg)			(  *( (volatile uint16_t *) (reg) ) )
#define OUT16(reg, data)		( (*( (volatile uint16_t *) (reg) ) ) = (uint16_t)(data) )

#define CLR16(reg)			(  *( (volatile uint16_t *) (reg) ) = (uint16_t)0 )
#define MASK16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) & (uint16_t)(mask) )
#define UMSK16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) )

#define SETR16SHL(reg, mask, shift, v)	(  *( (volatile uint16_t *) (reg) ) = \
					( ( *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) ) | \
					( ( (uint16_t)(v) << (shift) ) & (uint16_t)(mask)          ) ) )

#define SETR16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) = \
					( ( *( (volatile uint16_t *) (reg) ) & ~( (uint16_t)(mask) ) ) | (uint16_t)(mask) ) )
#define CLRR16(reg, mask)		(  *( (volatile uint16_t *) (reg) ) &= ~( (uint16_t)(mask) ) )

#define SETB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) |= (uint16_t)( (uint16_t)1 << (bit) ) )
#define CLRB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) &= ( ~( (uint16_t) ( (uint16_t)1 << (bit) ) ) ) )
#define GETB16(reg, bit)		(  *( (volatile uint16_t *) (reg) ) & (uint16_t) ( (uint16_t)1 << (bit) ) )
#define GETB16SHR(reg, bit)		( (*( (volatile uint16_t *) (reg) ) & (uint16_t) ( (uint16_t)1 << (bit) )) >> (bit) )

/* 32 bit access */
#define IN32(reg)			(  *( (volatile uint32_t *) (reg) ) )
#define OUT32(reg, data)		( (*( (volatile uint32_t *) (reg) ) ) = (uint32_t)(data) )

#define CLR32(reg)			(  *( (volatile uint32_t *) (reg) ) = (uint32_t)0 )
#define MASK32(reg, mask)		(  *( (volatile uint32_t *) (reg) ) & (uint32_t)(mask) )
#define UMSK32(reg, mask)		(  *( (volatile uint32_t *) (reg) ) & ~( (uint32_t)(mask) ) )

#define SETR32SHL(reg, mask, shift, v)	(  *( (volatile uint32_t *) (reg) ) = \
					( ( *( (volatile uint32_t *) (reg) ) & ~( (uint32_t)(mask) ) ) | \
					( ( (uint32_t)(v) << (shift) ) & (uint32_t)(mask)          ) ) )

#define SETR32(reg, mask)		(  *( (volatile uint32_t *) (reg) ) = \
					( ( *( (volatile uint32_t *) (reg) ) & ~( (uint32_t)(mask) ) ) | (uint32_t)(mask) ) )

#define CLRR32(reg, mask)		(  *( (volatile uint32_t *) (reg) ) &= ~( (uint32_t)(mask) ) )

#define SETB32(reg, bit)		(  *( (volatile uint32_t *) (reg) ) |= (uint32_t)( (uint32_t)1 << (bit) ) )
#define CLRB32(reg, bit)		(  *( (volatile uint32_t *) (reg) ) &= ( ~( (uint32_t) ( (uint32_t)1 << (bit) ) ) ) )
#define GETB32(reg, bit)		(  *( (volatile uint32_t *) (reg) ) & (uint32_t) ( (uint32_t)1 << (bit) ) )
#define GETB32SHR(reg, bit)		( (*( (volatile uint32_t *) (reg) ) & (uint32_t) ( (uint32_t)1 << (bit) )) >> (bit) )

#endif /* REG_IO_HACK */

#define SR_CLRB32(reg, bit)             \
{                                       \
        int mask = __nds32__mfsr(reg)& ~(1<<bit);\
        __nds32__mtsr(mask, reg);        \
        __nds32__dsb();                          \
}

#define SR_SETB32(reg,bit)\
{\
        int mask = __nds32__mfsr(reg)|(1<<bit);\
        __nds32__mtsr(mask, reg);                       \
        __nds32__dsb();                                         \
}


#endif /* __TURISMO_REGS_INC__ */
