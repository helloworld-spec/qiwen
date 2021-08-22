#include <config.h>
#include <regs.h>
#include <hal.h>
#include <bsp_hal.h>
#include <intc/intc.h>
#include "drv_usb.h"

/* Feature:
 *  (1) MCU(USB bootcode) handle Ep0, HCI accelerator handles Ep1~4
 *  (2) Support Andes MCU - N10(N1068A-S)
 */

static void irq_usb_handler(int m_data)
{
    u32 isr = GET_ADR_USB2_ISR;

    if (isr & DISCONNEVT) {
        SET_ADR_USB2_ISR(DISCONNEVT);
        /* If DISCONNEVT, reset whole chip to jump bootcode. */
        printf("USB Disconnection\n");
        SET_N10CFG_DEFAULT_IVB(0x4); //set IVB to ROM code address: 0x4000
        //SET_MCU_CLK_EN(1);
        SET_CLK_EN_CPUN10(1);
        SET_RESET_N_CPUN10(1);	// N10 might be disabled by default. Enable it.
        return;
    }

    if (isr & RESUMEEVT) {
        printf("USB RESUMEEVT\n");        
        SET_ADR_USB2_ISR(RESUMEEVT);
    } 
    else if (isr & SUSPENDEVT) {
        printf("USB SUSPENDEVT\n");
        SET_ADR_USB2_ISR(SUSPENDEVT);
    }
    else if (isr & RSTEVT) {
        printf("USB RSTEVT\n");
        SET_ADR_USB2_ISR(RSTEVT);
        SET_ALL_SW_RST(1);
    }        
}

/**
* s32 drv_usb_init() - install USB interrupt handler & configure USB setting.
*/
s32 drv_usb_init(void)
{
	/* Register USB interrupt handler here: */
    hal_register_isr(IRQ_USB, irq_usb_handler, NULL);
    hal_intc_irq_enable(IRQ_USB);
    
    if ( !(GET_ADR_USB2_UDCR & (1<<18)) ) {
        //Full speed, CPU handle EP1,2,3,4
        printf("USB Full speed\n");        
    }
    else {
        //High speed
        printf("USB High speed\n");
    }
    
    return 0;
}
