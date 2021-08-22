#include <config.h>
#include <stdio.h>
#include <ssv_lib.h>
#include <regs.h>
#ifdef _NO_OS_
#include <drv_phy.h>
#define LOG_PRINTF printf
#else
#include <log.h>
#endif
#include "drv_pmu.h"

// #define __PMU_VERIFY__
// #define __LOG_DRV_PMU_TIME__

#ifdef __LOG_DRV_PMU_TIME__
#define _TSTAMP_MAX_ 20
#endif

#define _PAD_CFG_N_  60   // number-of pad configuration registers

#define _CLK_SEL_MAC_XTAL_  0
#define _CLK_SEL_MAC_PLL_   3

#define _CLK_SEL_PHY_XTAL_  0
#define _CLK_SEL_PHY_PLL_   1

#define _RF_MODE_SHUTDOWN_  0
#define _RF_MODE_STANDBY_   1
#define _RF_MODE_TRX_EN_    2

#define _DLDO_LEVEL_SLEEP_  0
#define _DLDO_LEVEL_ON_     7

#define _T_DCDC_2_LD0_      50
#define _T_LDO_2_DCDC_      50

#define _T_SLEEP_MIN_       200 
#define _T_PLL_ON_          200
#define _T_RF_STABLE_       100

#define _PMU_WAKE_TRIG_TMR_ 1
#define _PMU_WAKE_TRIG_INT_ 2

const u32 tbl_pmu_pad_cfg[][2] = {
    // Cabrio-E config, PMU wake-up by pin-15
    {0xC0000300, 0x00000000},
    {0xC0000304, 0x00000000},
    {0xC0000308, 0x00000000},
    {0xC000030C, 0x00000000},
    {0xC0000310, 0x00000000},
    {0xC0000314, 0x00000000},
    {0xC0000318, 0x00000000},
    {0xC000031C, 0x00000000},
    {0xC0000320, 0x00000000},
    {0xC0000324, 0x00000000},
    {0xC0000328, 0x00000000},
    {0xC000032C, 0x00000000},
    {0xC0000330, 0x00000000},
    {0xC0000334, 0x00000000},
    {0xC0000338, 0x00000000},
    {0xC000033C, 0x0000100a},
    {0xC0000340, 0x00000000},
    {0xC0000344, 0x00000000},
    {0xC0000348, 0x00000000},
    {0xC000034C, 0x00000000},
    {0xC0000350, 0x00000000},
    {0xC0000354, 0x00000000},
    {0xC0000358, 0x00000000},
    {0xC000035C, 0x00000000},
    {0xC0000360, 0x00000000},
    {0xC0000364, 0x00000000},
    {0xC0000368, 0x00000000},
    {0xC000036C, 0x00000000},
    {0xC0000370, 0x00100000},
    {0xC0000374, 0x00100808},
    {0xC0000378, 0x00100008},
    {0xC000037C, 0x00100008},
    {0xC0000380, 0x00100008},
    {0xC0000384, 0x00100000},
    {0xC0000388, 0x00000000},
    {0xC000038C, 0x00000002},
    {0xC0000390, 0x00000000},
    {0xC0000394, 0x00000000},
    {0xC0000398, 0x00000000},
    {0xC000039C, 0x00000000},
    {0xC00003A0, 0x00000000},
    {0xC00003A4, 0x00000000},
    {0xC00003A8, 0x00000000},
    {0xC00003AC, 0x00000000},
    {0xC00003B0, 0x00000000},
    {0xC00003B4, 0x00000000},
    {0xC00003BC, 0x00000000},
    {0xC00003C4, 0x00000000},
    {0xC00003B8, 0x00000000},
    {0xC00003C0, 0x00000000},
    {0xC00003CC, 0x08000000},
    {0xC00003D0, 0x00000000},
    // CabrioD config
    /*
    {0xC0000300, 0x00000000},
    {0xC0000304, 0x00000000},
    {0xC0000308, 0x00000000},
    {0xC000030C, 0x00000000},
    {0xC0000310, 0x00000000},
    {0xC0000314, 0x00000000},
    {0xC0000318, 0x00000000},
    {0xC000031C, 0x00000000},
    {0xC0000320, 0x00000000},
    {0xC0000324, 0x00000000},
    {0xC0000328, 0x00000000},
    {0xC000032C, 0x00000000},
    {0xC0000330, 0x00000000},
    {0xC0000334, 0x00000000},
    {0xC0000338, 0x00000000},
    {0xC000033C, 0x00000000},
    {0xC0000340, 0x00000000},
    {0xC0000344, 0x00000000},
    {0xC0000348, 0x00000000},
    {0xC000034C, 0x00000000},
    {0xC0000350, 0x00000000},
    {0xC0000354, 0x00000000},
    {0xC0000358, 0x00000000},
    {0xC000035C, 0x00000000},
    {0xC0000360, 0x00000000},
    {0xC0000364, 0x00000000},
    {0xC0000368, 0x00000000},
    {0xC000036C, 0x00000000},
    // {0xC0000370, 0x00000002},
    // {0xC0000374, 0x00000000},
    // {0xC0000378, 0x00000000},
    // {0xC000037C, 0x00000000},
    // {0xC0000380, 0x00000000},
    // {0xC0000384, 0x00000002},
    {0xC0000388, 0x00000002},
    {0xC000038C, 0x00000002},
    {0xC0000390, 0x00000000},
    {0xC0000394, 0x00000000},
    {0xC0000398, 0x00000000},
    {0xC000039C, 0x00000000},
    {0xC00003A0, 0x00000000},
    {0xC00003A4, 0x00000000},
    {0xC00003A8, 0x00000000},
    {0xC00003AC, 0x00000000},
    {0xC00003B0, 0x00000000},
    {0xC00003B4, 0x00000000},
    {0xC00003B8, 0x00000000},
    {0xC00003C4, 0x00000000},
    {0xC00003BC, 0x00000000},
    {0xC00003C0, 0x00000000}
    */
};

s32 drv_pmu_setwake_cnt (u32 rtc_ticks)
{
#if 0
    u32 wake_cnt;
    u64 tmp_long;

    // fprintf('%10d\n', round(32.768*10^3*10^-5*2^20))
    tmp_long = (u64)t_10us * 343597;
    tmp_long = tmp_long + (1 << (20-1));
    tmp_long = tmp_long >> 20;
#endif
    if (rtc_ticks > MAX_RTC_SLEEP_TICKS) {
        LOG_PRINTF("Error: PMU wake-cnt overflow, wake_cnt=%x\n", rtc_ticks);
        return -1;
    }
    else {
       // SET_SLEEP_WAKE_CNT(rtc_ticks); //ToDo Liam should check this register still be needed.
        return 0;
    }
}

void drv_pmu_tu0 (u32 t_us) {
    volatile u32 tu0_rdy = 0;

    if(t_us == 0)
        return;

    // set timer
    SET_TU0_TM_INIT_VALUE(t_us);

    // wait ready
    while(tu0_rdy == 0) {
        tu0_rdy = GET_TU0_TM_INT_STS_DONE;
    }

    // clear timer
    SET_TU0_TM_INT_STS_DONE(1);
}

void drv_pmu_tm0 (u32 t_ms) {
    volatile u32 tm0_rdy = 0;

    if(t_ms == 0)
        return;

    // set timer
    SET_TM0_TM_INIT_VALUE(t_ms);

    // wait ready
    while(tm0_rdy == 0) {
        tm0_rdy = GET_TM0_TM_INT_STS_DONE;
    }

    // clear timer
    SET_TM0_TM_INT_STS_DONE(1);
}

s32 drv_pmu_sleep (void) 
{
//Paul TODO
    return 0;
}

void drv_pmu_init (void)
{
#if 0  // ToDo Liam: disable sleep  
    SET_DIGI_TOP_POR_MASK(1);

    // config from WM to optimize leakage
    REG32(0xc0001d0c) = 0x555f0010;
    REG32(0xce010090) = 0xaaaaaaae;
    REG32(0xce01008c) = 0xaaaaaaaf;

    /*
    if(config & _PMU_CFG_INT_EN_SDIO_CLK_) {
        #ifdef __PMU_VERIFY__
        LOG_PRINTF("## enable interupt from SDIO clock\n");
        #endif
        SET_WAKE_SOON_WITH_SCK(1);
    }
    else {
        #ifdef __PMU_VERIFY__
        LOG_PRINTF("## disable interupt from SDIO clock\n");
        #endif
        SET_WAKE_SOON_WITH_SCK(0);
    }
    */
#endif  // ToDo Liam: disable sleep  
}

void drv_pmu_chk (void)
{
#if 0  // ToDo Liam: disable sleep  
    SET_DIGI_TOP_POR_MASK(0);
#endif
}

bool drv_pmu_check_interupt_event(void)
{
#if 0  // ToDo Liam: disable sleep  
    u32 pmu_event = GET_PMU_WAKE_TRIG_EVENT;

    if(pmu_event == WAKEUP_EVENT_INTERRUPT)
        return true;
    else
        return false;
#endif  // ToDo Liam: disable sleep
return false;  
}
