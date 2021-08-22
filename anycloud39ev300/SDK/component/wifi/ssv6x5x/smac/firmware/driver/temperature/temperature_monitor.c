#include <regs.h>
#include <log.h>

static OsTimer temperature_timer;
static s32 uSarCode=0;

//#define TEMPERATURE_DEBUG

void temperatureHandler()
{
    s32 tempSarCode;
    s32 temperatureRecal=0;

    //Check every 10 seconds
    {
        //Eanble sensor
        SET_RG_SARADC_THERMAL(0);
        SET_RG_EN_SARADC(0);
        SET_RG_SARADC_THERMAL(1);
        SET_RG_EN_SARADC(1);

        do{
            if (GET_SAR_ADC_FSM_RDY)
                break;
        }while(1);

        tempSarCode = GET_RG_SARADC_BIT;
#ifdef TEMPERATURE_DEBUG
        LOG_PRINTF("uSarCode[%d] tempSarCode[%d]\n",uSarCode,tempSarCode);
#endif
        //Sar code more than 8 to recalibration
        //[About 20 degrees]
        if (uSarCode >= tempSarCode) {
            if ((uSarCode - tempSarCode) > 8)
                temperatureRecal = 1;
        }
        else {
            if ((tempSarCode - uSarCode) > 8)
                temperatureRecal = 1;
        }
        //Disable sensor
        SET_RG_SARADC_THERMAL(0);
        SET_RG_EN_SARADC(0);
        //recalibration
        if (temperatureRecal) {

            LOG_PRINTF("temperatureRecal uSarCode[%d] tempSarCode[%d]\n",uSarCode,tempSarCode);

            uSarCode = tempSarCode;
            SET_RG_EN_SX_LCK_BIN(0);
            SET_RG_EN_SX_LCK_BIN(1);
        }
    }

	return;
}

#define TEMPERATURE_MONITOR_TIME 10000
s32 temperature_monitor_init()
{
	//Init 10 seconds timer 
	if( OS_TimerCreate(&temperature_timer, TEMPERATURE_MONITOR_TIME, (u8)TRUE, NULL, (OsTimerHandler)temperatureHandler) == OS_FAILED)
		return OS_FAILED;

    OS_TimerStop(temperature_timer);
	return OS_SUCCESS;
}

void temperature_monitor_start()
{
	OS_TimerStart(temperature_timer);
}

void temperature_monitor_stop()
{
    OS_TimerStop(temperature_timer);
}


