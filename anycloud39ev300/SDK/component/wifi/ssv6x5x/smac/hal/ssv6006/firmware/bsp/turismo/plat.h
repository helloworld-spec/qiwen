#ifndef __PLAT_H__
#define __PLAT_H__

#include "irq.h"

#define IRQ_SYS_TICK_VECTOR IRQ_US_TIMER0
//#define IRQ_SYS_TICK2_VECTOR IRQ_US_TIMER0
#define SYS_TICK_TIMER      (0)
#define USB_TICK_TIMER      (4)

#define SWI_IRQ             (1)

#endif 
