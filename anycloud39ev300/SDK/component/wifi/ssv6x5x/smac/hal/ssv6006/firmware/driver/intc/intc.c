#include <turismo_regs.h>
#include "intc.h"
#include <hal.h>

#ifndef NULL
#define NULL (0)
#endif

static void _group2_irq_handler (void);
static void _group15_irq_handler (void);
static void _group31_irq_handler (void);
static void *GROUP2_Vector_Table[32] = { NULL,                  //GROUP2 INT_0
                                       NULL,                    //GROUP2 INT_1
};
static void *GROUP15_Vector_Table[32] = { NULL,                 //GROUP15 INT_0
                                       NULL,                    //GROUP15 INT_1
                                       NULL,                    //GROUP15 INT_2
                                       NULL,                    //GROUP15 INT_3
                                       NULL,                    //GROUP15 INT_4
                                       NULL,                    //GROUP15 INT_5
                                       NULL,                    //GROUP15 INT_6
                                       NULL,                    //GROUP15 INT_7
                                       NULL,                    //GROUP15 INT_8
                                       NULL,                    //GROUP15 INT_9
                                       NULL,                    //GROUP15 INT_10
                                       NULL,                    //GROUP15 INT_11
                                       NULL,                    //GROUP15 INT_12
                                       NULL,                    //GROUP15 INT_13
                                       NULL,                    //GROUP15 INT_14
                                       NULL,                    //GROUP15 INT_15
                                       NULL,                    //GROUP15 INT_16
                                       NULL,                    //GROUP15 INT_17
                                       NULL,                    //GROUP15 INT_18
                                       NULL,                    //GROUP15 INT_19
                                       NULL,                    //GROUP15 INT_20
                                       NULL,                    //GROUP15 INT_21
                                       NULL,                    //GROUP15 INT_22
                                       NULL,                    //GROUP15 INT_23
                                       NULL,                    //GROUP15 INT_24
                                       NULL,                    //GROUP15 INT_25
                                       NULL,                    //GROUP15 INT_26
                                       NULL,                    //GROUP15 INT_27
                                       NULL,                    //GROUP15 INT_28
                                       NULL,                    //GROUP15 INT_29
                                       NULL,                    //GROUP15 INT_30
                                       NULL                     //GROUP15 INT_31
};
static void *GROUP31_Vector_Table[32] = { NULL,                 //GROUP31 INT_0
                                       NULL,                    //GROUP31 INT_1
                                       NULL,                    //GROUP31 INT_2
                                       NULL,                    //GROUP31 INT_3
                                       NULL,                    //GROUP31 INT_4
                                       NULL,                    //GROUP31 INT_5
                                       NULL,                    //GROUP31 INT_6
                                       NULL,                    //GROUP31 INT_7
                                       NULL,                    //GROUP31 INT_8
                                       NULL,                    //GROUP31 INT_9
                                       NULL,                    //GROUP31 INT_10
                                       NULL,                    //GROUP31 INT_11
                                       NULL,                    //GROUP31 INT_12
                                       NULL,                    //GROUP31 INT_13
                                       NULL,                    //GROUP31 INT_14
                                       NULL,                    //GROUP31 INT_15
                                       NULL,                    //GROUP31 INT_16
                                       NULL,                    //GROUP31 INT_17
                                       NULL,                    //GROUP31 INT_18
                                       NULL,                    //GROUP31 INT_19
                                       NULL,                    //GROUP31 INT_20
                                       NULL,                    //GROUP31 INT_21
                                       NULL,                    //GROUP31 INT_22
                                       NULL,                    //GROUP31 INT_23
                                       NULL,                    //GROUP31 INT_24
                                       NULL,                    //GROUP31 INT_25
                                       NULL,                    //GROUP31 INT_26
                                       NULL,                    //GROUP31 INT_27
                                       NULL,                    //GROUP31 INT_28
                                       NULL,                    //GROUP31 INT_29
                                       NULL,                    //GROUP31 INT_30
                                       NULL                     //GROUP31 INT_31
};

/**
 *  struct INTR_CTRL_st - Definition of hardware interrupt controller registers
 *
 */
typedef struct INTR_CTRL_st {
    SSV6XXX_REG MASK_TYPMCU_INT_MAP_02;
    SSV6XXX_REG RAW_TYPMCU_INT_MAP_02;
    SSV6XXX_REG POSTMASK_TYPMCU_INT_MAP_02;
    SSV6XXX_REG MASK_TYPMCU_INT_MAP_15;
    SSV6XXX_REG RAW_TYPMCU_INT_MAP_15;
    SSV6XXX_REG POSTMASK_TYPMCU_INT_MAP_15;
    SSV6XXX_REG MASK_TYPMCU_INT_MAP_31;
    SSV6XXX_REG RAW_TYPMCU_INT_MAP_31;
    SSV6XXX_REG POSTMASK_TYPMCU_INT_MAP_31;
    SSV6XXX_REG MASK_TYPMCU_INT_MAP;
    SSV6XXX_REG RAW_TYPMCU_INT_MAP;
    SSV6XXX_REG POSTMASK_TYPMCU_INT_MAP;
} INTR_CTRL, *P_INTR_CTRL;

#define REG_INTR_CTRL           ((INTR_CTRL *)(ADR_MASK_TYPMCU_INT_MAP_02))

void intc_mask_all (void)
{
    /**
     *  Mask out all IRQs and set to IRQ mode by default.
     *  Note that for INT_MASK, set to 1 disable interrupt
     *  while 0 enable interrupt.
     */
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_02 = 0xFFFFFFFF;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_15 = 0xFFFFFFFF;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_31 = 0xFFFFFFFF;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP = 0xFFFFFFFF;
    REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP = 0;
}

void intc_init (void)
{
    intc_mask_all();
    hal_register_isr(IRQ_SPI_SDIO_WAKE, (hal_isr_t)_group2_irq_handler, NULL);
    hal_register_isr(IRQ_15_GROUP, (hal_isr_t)_group15_irq_handler, NULL);
    hal_register_isr(IRQ_31_GROUP, (hal_isr_t)_group31_irq_handler, NULL);
}

void intc_clear_all (void)
{
    REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP_02 = 0xFFFFFFFF;
    REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP_15 = 0xFFFFFFFF;
    REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP_31 = 0xFFFFFFFF;
    REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP = 0xFFFFFFFF;
}

s32 intc_status (void)
{

    return REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP;
}

void intc_mask (u32 mask)
{
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP = mask;
}

void intc_int_mask (u32 int_no)
{

    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP |= (1U << int_no);
}

void intc_int_unmask (u32 int_no)
{
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP &= ~(1U << int_no);
}

void intc_int_clear (u32 int_no)
{
    REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP = (1U << int_no);
}

s32  intc_int_status (u32 int_no)
{   u32 status;

    status = REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP;

    return (status & (1 << int_no));
}

s32 intc_group2_status (void)
{
    return REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP_02;
}

void intc_group2_enable (u32 int_no, void *irq_func)
{
    GROUP2_Vector_Table[int_no] = irq_func;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_02 &= ~(1U << int_no);
    intc_int_unmask(IRQ_SPI_SDIO_WAKE);
}

void intc_group2_disable (u32 int_no)
{
    GROUP15_Vector_Table[int_no] = NULL;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_02 |= (1U << int_no);
}

void _group2_irq_handler (void)
{
    u32 status = REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP_02;
    int i;

    for (i = 0; (i < 2) && (status != 0); i++, status >>= 1)
    {
        if ((status & 1) == 0)
            continue;

        if (GROUP2_Vector_Table[i] == NULL)
        {
            intc_group2_disable(i);
            continue;
        }

        ((void (*)(int))GROUP2_Vector_Table[i])(i);
    }
} // end of - _group2_irq_handler -


s32 intc_group15_status (void)
{
    return REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP_15;
}

void intc_group15_enable (u32 int_no, void *irq_func)
{
    GROUP15_Vector_Table[int_no] = irq_func;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_15 &= ~(1U << int_no);
    intc_int_unmask(IRQ_15_GROUP);
}

void intc_group15_disable (u32 int_no)
{
    GROUP15_Vector_Table[int_no] = NULL;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_15 |= (1U << int_no);
}

void _group15_irq_handler (void)
{
    u32 status = REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP_15;
    int i;

    for (i = 0; (i < 32) && (status != 0); i++, status >>= 1)
    {
        if ((status & 1) == 0)
            continue;

        if (GROUP15_Vector_Table[i] == NULL)
        {
            intc_group15_disable(i);
            continue;
        }

        ((void (*)(int))GROUP15_Vector_Table[i])(i);
    }
} // end of - _group15_irq_handler -

void intc_group31_enable (u32 int_no, void *irq_func)
{
    GROUP31_Vector_Table[int_no] = irq_func;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_31 &= ~(1U << int_no);
    intc_int_unmask(IRQ_31_GROUP);
}

void intc_group31_disable (u32 int_no)
{
    GROUP31_Vector_Table[int_no] = NULL;
    REG_INTR_CTRL->MASK_TYPMCU_INT_MAP_31 |= (1U << int_no);
}

void _group31_irq_handler (void)
{
    u32 status = REG_INTR_CTRL->POSTMASK_TYPMCU_INT_MAP_31;
    int i;

    for (i = 0; (i < 32) && (status != 0); i++, status >>= 1)
    {
        if ((status & 1) == 0)
            continue;

        if (GROUP31_Vector_Table[i] == NULL)
        {
            intc_group31_disable(i);
            continue;
        }

        ((void (*)(int))GROUP31_Vector_Table[i])(i);
    }
} // end of - _group31_irq_handler -
