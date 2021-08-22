/**
 * @file interrupt.h
 * @brief: This file describe how to control the AK3223M interrupt issues.
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-07-13
 * @version 1.0
 */
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__


/** @defgroup Interrupt  
 *  @ingroup M3PLATFORM
 */
/*@{*/
#include "anyka_cpu.h"
#include "anyka_types.h"


/** @{@name Interrupt Operator Define
 *  Define the macros to operate interrupt register, to enable/disable interrupt
 */
 /**IRQ mode*/
#define INTR_ENABLE(int_bits)   \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)IRQINT_MASK_REG |= (int_bits); \
        irq_unmask(); \
    }while(0)
#define INTR_DISABLE(int_bits) \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)IRQINT_MASK_REG &= ~(int_bits); \
        irq_unmask(); \
    }while(0)

 /**IRQ Level2 mode*/
#define INTR_ENABLE_L2(int_bits) \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)INT_SYS_MODULE_REG |= (int_bits); \
        irq_unmask(); \
    }while(0)


#define INTR_DISABLE_L2(int_bits) \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)INT_SYS_MODULE_REG &= ~(int_bits); \
        irq_unmask(); \
    }while(0)


/** FIQ mode*/
#define FIQ_INTR_ENABLE(int_bits)   \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)FRQINT_MASK_REG |= (int_bits); \
        irq_unmask(); \
    }while(0)

#define FIQ_INTR_DISABLE(int_bits)  \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)FRQINT_MASK_REG &= ~(int_bits); \
        irq_unmask(); \
    }while(0)


typedef bool (*T_INTR_HANDLER)(void);

typedef enum 
{
    INT_VECTOR_MIN=100,
		
    INT_VECTOR_MEM, 
    INT_VECTOR_CAMERA,
    INT_VECTOR_VENC,
    INT_VECTOR_MMCSD,
    
    INT_VECTOR_SDIO,
    INT_VECTOR_ADC,
    INT_VECTOR_DAC,
    INT_VECTOR_SPI1,
    INT_VECTOR_SPI2,
    
    INT_VECTOR_UART1,    
    INT_VECTOR_UART2,    
    INT_VECTOR_L2,    
    INT_VECTOR_IIC,
    INT_VECTOR_IRDA,
    
    INT_VECTOR_GPIO,
    INT_VECTOR_MAC,
    INT_VECTOR_ENCRYPT,
    INT_VECTOR_USB,
    INT_VECTOR_USB_DMA,
    
     /*INT_VECTOR_SYSTEM,*/
    INT_VECTOR_SAR_ADC,
    INT_VECTOR_TIMER,
    INT_VECTOR_WAKEUP_TRIGGER,
    INT_VECTOR_RTC,
	
    INT_VECTOR_MAX
}INT_VECTOR;

/**
 * @brief: interrupt init, called before int_register_irq()
 */
void interrupt_init(void);

/**
 * @brief: register irq interrupt
 */
bool int_register_irq(INT_VECTOR v, T_INTR_HANDLER handler);

#endif

