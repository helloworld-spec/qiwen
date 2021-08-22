#include <config.h>
#include "drv_timer.h"
#include <plat.h>
#include <rtos.h>
#include <hal.h>
#include <bsp_hal.h>
#include <intc/intc.h>

/**
 *  struct HW_TIMER_st - Hardware timer register definition (ms/us)
 *
 *  @ TMR_CTRL[00:15] :  Counter value
 *  @ TMR_CTRL[16:16] :  Operation mode
 */
/*lint -save -e751 */
typedef struct HW_TIMER_st {
    union {
        struct {
            SSV6XXX_REG    counter:16;       // [15:0]: Counter, [16]: Mode, [17]: INT status, [18]: INT mask
            SSV6XXX_REG    mode:1;
            SSV6XXX_REG    status:1;
            SSV6XXX_REG    mask:1;
            SSV6XXX_REG    __pad0:13;
        } __attribute__((packed));
        SSV6XXX_REG    ctrl:32;
    };
    SSV6XXX_REG    count:16;         // [15:0]: Current counting value
    SSV6XXX_REG    __pad1:16;
    SSV6XXX_REG    prescaler:9;      // [8:0]: Prescaler to system clock as the counting unit. 40 is the default value as 40MHz is the expected default system clock.
    SSV6XXX_REG    __pad2:23;
    SSV6XXX_REG    __pad3;
} __attribute__ ((packed)) HW_TIMER, *P_HW_TIMER;

#define REG_US_TIMER            ((HW_TIMER  *)(TU0_US_REG_BASE))
#define REG_MS_TIMER            ((HW_TIMER  *)(TM0_MS_REG_BASE))
#define REG_TIMER               REG_US_TIMER

#define US_TIMER0                       (REG_US_TIMER + 0)
#define US_TIMER1                       (REG_US_TIMER + 1)
#define US_TIMER2                       (REG_US_TIMER + 2)
#define US_TIMER3                       (REG_US_TIMER + 3)
#define MS_TIMER0                       (REG_MS_TIMER + 0)
#define MS_TIMER1                       (REG_MS_TIMER + 1)
#define MS_TIMER2                       (REG_MS_TIMER + 2)
#define MS_TIMER3                       (REG_MS_TIMER + 3)

#define HW_TIMER_MAX_VALUE_MS          0xffff*1000  //16bits    //65535000
#define HW_TIMER_MAX_VALUE_US          0xffff       //16bits    //65535

#define HTMR_COUNT_VAL(x)               ((u32)(x)&0xFFFF)
#define HTMR_OP_MODE(x)                 (((u32)(x)&0x01)<<16)
#define HTMR_GET_IRQ(x)                 (((HW_TIMER*)(x)-REG_TIMER)+ \
                                         IRQ_US_TIMER0)

#define HTMR_GET_ID_BY_IRQ(x)            ((x) - IRQ_US_TIMER0)
#define HTMR_GET_IRQ_BY_ID(x)            (IRQ_US_TIMER0 + (x))


#define SETUP_TIMER(timer, period, mode) \
    do { \
        (timer)->ctrl = HTMR_COUNT_VAL(period) | HTMR_OP_MODE(mode); \
    } while (0)

/*lint -restore */

s32 hwtmr_stop(TimerID_E timer_id)
{
    if ((timer_id != SYS_TICK_TIMER) && (timer_id >= TIMER_ID_MIN) && (timer_id <= TIMER_ID_MAX))
    {
        P_HW_TIMER timer = ((P_HW_TIMER)REG_US_TIMER) + timer_id;
        timer->ctrl = 0;
        return OS_SUCCESS;
    }
    //u32 irq = (u32)HTMR_GET_IRQ(tmr);
    //tmr->ctrl = 0x00;
    //return irq_request(irq, NULL, NULL);
    return OS_FAILED;
}

static timer_handler       _timer_handler[TIMER_ID_MAX] = {0};
static void *              _timer_handler_param[TIMER_ID_MAX] = {0};

static void _timer_irq_handler (int irq_id)
{
    TimerID_E   timer_id = HTMR_GET_ID_BY_IRQ(irq_id);

    if (_timer_handler[timer_id] == NULL)
        return;

    _timer_handler[timer_id](_timer_handler_param[timer_id]);
}

s32 hwtmr_start(TimerID_E timer_id, u16 count, timer_handler tmr_handler,
                void *m_data, enum hwtmr_op_mode mode)
{
    int         irq_id;

    if ((timer_id == SYS_TICK_TIMER) || (timer_id < TIMER_ID_MIN) || (timer_id > TIMER_ID_MAX))
        return OS_FAILED;

    _timer_handler[timer_id] = tmr_handler;
    _timer_handler_param[timer_id] = m_data;
    // Register timer handler
    irq_id = HTMR_GET_IRQ_BY_ID(timer_id);
    hal_register_isr(irq_id, _timer_irq_handler, NULL);
    // Enable timer interrupt
    hal_intc_irq_enable(irq_id);
    // Set timer and go
    SETUP_TIMER(&REG_TIMER[timer_id], count, mode);

    return OS_SUCCESS;
}

#if 0
void hwtmr_init(void)
{
    s32 i;
    for (i=0; i < MAX_HW_TIMER; i++)
    {
        /*lint -save -e845 */
        REG_TIMER[i].ctrl = HTMR_COUNT_VAL(0)|HTMR_OP_MODE(HTMR_ONESHOT);
        /*lint -restore */
    }
}
#endif // 0

void hwtmr_init (TimerID_E timer_id, u32 period, TimerMode_E mode)
{
    if (timer_id >= TIMER_ID_MIN && timer_id <= TIMER_ID_MAX)
    {
        P_HW_TIMER timer = ((P_HW_TIMER)REG_US_TIMER) + timer_id;
        //u32        counter = (period * 400 + 205) / 410;
        u32        counter = period;
        
        SETUP_TIMER(timer, counter, mode);
        // set prescaler
        timer->prescaler = 0x50;
    }
} // end of - hwtmr_init -


void hwtmr_enable (TimerID_E timer_id, u32 enable)
{
    if (timer_id >= TIMER_ID_MIN && timer_id <= TIMER_ID_MAX)
    {
        P_HW_TIMER timer = ((P_HW_TIMER)REG_US_TIMER) + timer_id;

        timer->mask = enable ? 0 : 1;
    }
} // end of - hwtmr_enable -

u32 hwtmr_get_count (TimerID_E timer_id)
{
    if (timer_id >= TIMER_ID_MIN && timer_id <= TIMER_ID_MAX)
    {
        P_HW_TIMER timer = ((P_HW_TIMER)REG_US_TIMER) + timer_id;

        if (timer->counter > 0)
            return (timer->counter - timer->count);
    }

    return -1;
} // end of - hwtmr_get_count -

u32 hwtmr_read_count (TimerID_E timer_id)
{
    if (timer_id >= TIMER_ID_MIN && timer_id <= TIMER_ID_MAX)
    {
        P_HW_TIMER timer = ((P_HW_TIMER)REG_US_TIMER) + timer_id;

        return timer->count;
    }

    return -1;
} // end of - hwtmr_read_count -

u32 hwtmr_get_status (TimerID_E timer_id)
{
    if (timer_id >= TIMER_ID_MIN && timer_id <= TIMER_ID_MAX)
    {
        P_HW_TIMER timer = ((P_HW_TIMER)REG_US_TIMER) + timer_id;

       return timer->status;
    }

    return -1;
} // end of - hwtmr_get_status -

void hwtmr_clear_irq (TimerID_E timer_id)
{
    if (timer_id >= TIMER_ID_MIN && timer_id <= TIMER_ID_MAX)
    {
        P_HW_TIMER timer = ((P_HW_TIMER)REG_US_TIMER) + timer_id;
        timer->status = 0;
    }
} // end of - hwmtr_clear_irq -

u32 hwtmr_get_irq (TimerID_E timer_id)
{
    if (timer_id >= TIMER_ID_MIN && timer_id <= TIMER_ID_MAX)
    {
        return (IRQ_US_TIMER0 + timer_id);
    }
    return 0;
} // end of - hwtmr_get_irq -
