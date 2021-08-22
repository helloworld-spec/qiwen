#include <config.h>
#include <intc/intc.h>
#include "drv_rtc.h"

#define M_RTC_RAM_SIZE                  (0x20*4)


#define CNT_RTC_CALI_MAX        80000
#define CNT_RTC_CALI_RDY_DLY    200    // emulate 200us

// #define __RTC_VERIFY__
// #define __LOG_DRV_RTC_MCU_TIME__

#ifdef __RTC_VERIFY__
#include <log.h>
#endif

static void (*sec_handler) (void *);
static void *sec_data;
static void (*alarm_handler) (void *);
static void *alarm_data;

void drv_rtc_int_handler (int irq_id) {
	if ((GET_RO_RTC_INT_SEC == RTC_INT_PEND) && (sec_handler != NULL))
	{
		sec_handler(sec_data);
	}

	if ((GET_RO_RTC_INT_ALARM == RTC_INT_PEND) && (alarm_handler != NULL))
	{
		alarm_handler(alarm_data);
	}
}

s32 drv_rtc_cali(void)
{
#if 0       // ToDo Liam; comment out cali    
    u32 rtc_cali_rdy;

    // bool ever0_flag  = false;
    // bool first1_flag = false;

    u32 cnt_rtc_cali;

   #ifdef __LOG_DRV_RTC_MCU_TIME__
    u32 ctime[10];
    u32 ctime_idx = 0;
    u32 loop_i;
    #endif

    #ifdef __LOG_DRV_RTC_MCU_TIME__
    SET_TU0_TM_INT_MASK  (1    );
    SET_TU0_TM_MODE      (0    );
    SET_TU0_TM_INIT_VALUE(65535);
    #endif

    // check if switch clock to 80M for calibration purpose
    if(GET_RG_RF_BB_CLK_SEL == 0) {
        #ifdef __RTC_VERIFY__
        LOG_PRINTF("RTC cali fail: Wrong clock source\n");
        #endif
        return -1;
    }

    if(GET_RG_MODE == 0) {
        #ifdef __RTC_VERIFY__
        printf("RTC cali fail: Wrong RF mode\n");
        #endif
        return -2;
    }

    // enable RTC calibration
    SET_RTC_CAL_ENA(1);
    #ifdef __LOG_DRV_RTC_MCU_TIME__
    ctime[ctime_idx++] = REG32(0xc0000204);
    #endif

    for (cnt_rtc_cali = 0; cnt_rtc_cali < CNT_RTC_CALI_MAX; cnt_rtc_cali++) {
        rtc_cali_rdy = GET_RTC_OSC_CAL_RES_RDY;

        if (cnt_rtc_cali >= CNT_RTC_CALI_RDY_DLY) {
            if (rtc_cali_rdy == 1)
                break;
        }
    }
    #ifdef __RTC_VERIFY__
    LOG_PRINTF("RTC cali: It takes %d read access to get ready\n", cnt_rtc_cali);
    #endif

    if (rtc_cali_rdy == 0)  {
        #ifdef __RTC_VERIFY__
        LOG_PRINTF("RTC cali fail: RTC cali failed\n");
        return -2;
        #endif
    }

    // recovery settings
    SET_RTC_CAL_ENA(0);

    #ifdef __RTC_VERIFY__
    LOG_PRINTF("RTC cali result %d\n", GET_RG_RTC_OSC_RES_SW);
    #endif

    #ifdef __LOG_DRV_RTC_MCU_TIME__
    ctime[ctime_idx++] = REG32(0xc0000204);
    #endif

    #ifdef __LOG_DRV_RTC_MCU_TIME__
    for(loop_i=0; loop_i < ctime_idx; loop_i++) {
        LOG_PRINTF(" time stamp %d = %d\n", loop_i, ctime[loop_i]);
    }
    #endif
#endif // ToDo Liam: comment out cali.    
    return 0;

}

void drv_rtc_init(void)
{
#if 0   //ToDo Liam: RTC is not supported yet.
	//disable rtc
	drv_rtc_en(RTC_DISABLE);
	//REG_RTC->RTC_CTRL0 =  M_SET_EN(REG_RTC->RTC_CTRL0,DISABLE);
	//set to prescaler
	drv_rtc_set_source(RTC_SOURCE_PRESCALER);
    //set sec init value to 0
    // REG_RTC->RTC_CNT = 0;
    SET_RTC_SEC_START_CNT(0);
    //set alarm init value to 0
    // REG_RTC->RTC_ALARM=0;
    SET_RTC_SEC_ALARM_VALUE(0);
	//mask sec interrupt
	SET_RTC_INT_SEC_MASK(RTC_INT_MASKED);
	//mask alarm interrupt
	SET_RTC_INT_ALARM_MASK(RTC_INT_MASKED);

	//register irq handle
    intc_peri_enable(IRQ_PERI_RTC, drv_rtc_int_handler);
	//irq_request(IRQ_RTC, drv_rtc_int_handler, NULL);

	sec_handler = NULL;
	sec_data = NULL;
	alarm_handler = NULL;
	alarm_data = NULL;

	//drv_rtc_en(RTC_ENABLE);
#endif //ToDo Liam: RTC is not supported yet.
}

#if 0   //ToDo Liam: RTC is not supported yet.
void drv_rtc_en(RTC_EN en)
{
	// REG_RTC->RTC_CTRL0 =  M_SET_EN(REG_RTC->RTC_CTRL0,en);
    SET_RTC_EN(en);
}

void drv_rtc_set_source(RTC_SOURCE source)
{
	// REG_RTC->RTC_CTRL0 =  M_SET_SRC(REG_RTC->RTC_CTRL0,source);
	SET_RTC_SRC(source);
}

RTC_SOURCE drv_rtc_get_source(void)
{
	// return  (RTC_SOURCE)M_GET_SRC(REG_RTC->RTC_CTRL0 );
	return  (RTC_SOURCE)GET_RTC_SRC;
}

s32 drv_rtc_getrealtime(u32 *pTime)
{
	//REG_RTC->RTC_CTRL0 =  M_SET_EN(REG_RTC->RTC_CTRL0,ENABLE);
	// *pTime = REG_RTC->RTC_CNT;
	//REG_RTC->RTC_CTRL0 =  M_SET_EN(REG_RTC->RTC_CTRL0,DISABLE);
	*pTime = GET_RTC_SEC_CNT;

	return 0;
}

s32 drv_rtc_setalarmtime(u32 alarmtime)
{
	// REG_RTC->RTC_ALARM = alarmtime;
    SET_RTC_SEC_ALARM_VALUE(alarmtime);
	return 0;
}

s32 drv_rtc_getalarmtime(u32 *pAlarmTime)
{
	//*pAlarmTime = REG_RTC->RTC_ALARM;
    *pAlarmTime = GET_RTC_SEC_ALARM_VALUE;
	return 0;
}
#endif   //ToDo Liam: RTC is not supported yet.

s32 drv_rtc_set_sec_alarm_handler (void (*e_sec_handler) (void *), void *e_m_data)
{
	sec_handler = e_sec_handler;
	sec_data = e_m_data;
	return 0;
}

#if 0
RTC_INT_STATUS drv_rtc_get_sec_interrupt_status(void)
{
	// return (RTC_INT_STATUS)M_GET_INT_SEC_STATUS(REG_RTC->RTC_CTRL1);
	return (RTC_INT_STATUS) GET_RTC_INT_SEC; 
}

#if 0 //ToDo Liam: RTC is not supported yet.
void drv_rtc_set_sec_interrupt_mask(void)
{
	// REG_RTC->RTC_CTRL1 = M_SET_INT_SEC_MASK (REG_RTC->RTC_CTRL1,RTC_INT_MASKED);
        SET_RTC_INT_SEC_MASK(RTC_INT_MASKED);
}

void drv_rtc_clear_sec_interrupt_mask(void)
{
	// REG_RTC->RTC_CTRL1 =  M_SET_INT_SEC_MASK (REG_RTC->RTC_CTRL1,RTC_INT_UN_MASKED);
        SET_RTC_INT_SEC_MASK(RTC_INT_UN_MASKED);
}
#endif //ToDo Liam: RTC is not supported yet.

void drv_rtc_clear_sec_interrupt_pending(void)
{
	// REG_RTC->RTC_CTRL1 =  M_SET_INT_SEC_STATUS(REG_RTC->RTC_CTRL1,RTC_INT_PEND);
        SET_RTC_INT_SEC(RTC_INT_PEND);
}
#endif // 0

s32 drv_rtc_set_alarm_handler (void (*e_alarm_handler)(void *), void *e_m_data)
{
	alarm_handler = e_alarm_handler;
	alarm_data = e_m_data;
	return 0;
}

#if 0
RTC_INT_STATUS drv_rtc_get_alarm_interrupt_status(void)
{
	// return (RTC_INT_STATUS)M_GET_INT_ALARM_STATUS (REG_RTC->RTC_CTRL1);
	return (RTC_INT_STATUS)GET_RTC_INT_ALARM;
}
#if 0 //ToDo Liam: RTC is not supported yet.
void drv_rtc_set_alarm_interrupt_mask(void)
{
	// REG_RTC->RTC_CTRL1 =  M_SET_INT_ALARM_MASK(REG_RTC->RTC_CTRL1 ,RTC_INT_MASKED);
	SET_RTC_INT_ALARM_MASK(RTC_INT_MASKED);
}

void drv_rtc_clear_alarm_interrupt_mask(void)
{
	// REG_RTC->RTC_CTRL1 = M_SET_INT_ALARM_MASK (REG_RTC->RTC_CTRL1 ,RTC_INT_UN_MASKED);
	SET_RTC_INT_ALARM_MASK(RTC_INT_UN_MASKED);
}

void drv_rtc_clear_alarm_interrupt_pending(void)
{
	// REG_RTC->RTC_CTRL1 =  M_SET_INT_ALARM_STATUS(REG_RTC->RTC_CTRL1 ,RTC_INT_PEND);
	SET_RTC_INT_ALARM(RTC_INT_PEND);
}

u32* drv_rtc_get_ramaddress(void)
{
	// return (u32*)REG_RTC->RTC_RAM00;
	return (u32*)RTC_RAM_BASE;
}
#endif

u32 drv_rtc_get_ramsize(void)
{
	return M_RTC_RAM_SIZE;
}
#endif
