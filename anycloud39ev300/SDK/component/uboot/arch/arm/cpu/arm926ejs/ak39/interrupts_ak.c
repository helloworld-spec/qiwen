/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch-ak39/anyka_cpu.h>
#include <asm/proc-armv/ptrace.h>

#define N_IRQS			32

struct _irq_handler {
	void                *m_data;
	void (*m_func)( void *data);
};


extern int ak_arch_interrupt_init (void);
extern void ak_do_irq (int hw_irq);

static struct _irq_handler IRQ_HANDLER[N_IRQS];

static void default_isr(void *data)
{
	printf("default_isr():  called for IRQ %d, Interrupt Status PR\n",
	       (int)data);
}

static int next_irq(void)
{
	//return (((*IXP425_ICIH & 0x000000fc) >> 2) - 1);
	return 1;
}

void ak_do_irq (int hw_irq)
{
	int irq = hw_irq; // cdh:next_irq();

	// printf("cdh:hw_irq=%d\n", hw_irq);
	IRQ_HANDLER[irq].m_func(IRQ_HANDLER[irq].m_data);
}

void ak_irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data)
{
	if (irq >= N_IRQS || !handle_irq)
		return;
	
	IRQ_HANDLER[irq].m_data = data;
	IRQ_HANDLER[irq].m_func = handle_irq;
}

int ak_arch_interrupt_init (void)
{
	int i;

	/* install default interrupt handlers */
	for (i = 0; i < N_IRQS; i++)
		ak_irq_install_handler(i, default_isr, (void *)i);

	/* configure interrupts for IRQ mode */
	// cdh: *IXP425_ICLR = 0x00000000;

	return (0);
}
