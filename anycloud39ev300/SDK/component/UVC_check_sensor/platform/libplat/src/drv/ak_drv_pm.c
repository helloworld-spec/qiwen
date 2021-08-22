#include "command.h"
#include "drv_api.h"
#include "ak_common.h"
#include "ak_drv_pm.h"

static bool idle_flag = false;
/**
 * @brief   get vbat value
 * @author  
 * @date    2016-11-23
 * @param[in]  bat voltage 
 * @return  int *
 * @retval  0 no bat
 * @retval  value bat_value
 */

int ak_drv_pm_get_bat_value(int *voltage)
{
    unsigned long analog_value = 0,actual_value = 0;
	
    analog_value = analog_getvalue_bat();
     
    actual_value += ((analog_value * 2 * 3000) >> 12);
	if(actual_value)
	{
		*voltage = actual_value;
		return 0;
	}
	else
	{
		*voltage = 0;
		return -1;
	}

}
#if 0
/**
 * @brief   get ain value
 * @author  
 * @date    2016-11-23
 * @param[in]  ain voltage 
 * @return  int *
 * @retval  ain_value
 * @retval  value ain_value
 */
int ak_drv_pm_get_ain_value(int *voltage)
{
    unsigned long analog_value = 0,actual_value = 0;
	analog_value = analog_getvalue_ain();	
    actual_value = (analog_value  * 3000) >> 12;	
	*voltage = actual_value;
    return 0;

}
/**
 * @brief   enter standby 
 * @author 
 * @date    2016-11-23
 * @param[in]   
 * @return  int
 * @retval  0 failed
 * @retval  1 successful
 */

int ak_drv_pm_enter_standby()
{
#if 0	
	unsigned long pin, bit;
	unsigned long reason;
	unsigned long wgpio_mask;
	unsigned long ctreg;
	unsigned long ret;
	T_SYSTIME systime;
	rtc_init(2010);		
	pin = 47;	 // key ”Ô“Ù∞¥º¸
	
	gpio_set_pin_as_gpio(pin);
	gpio_set_pull_up_r(pin, false); 		

	gpio_set_pin_dir(pin, 0);	//input mode

	wgpio_mask = (1<<get_wGpio_Bit(pin));  //get gpio control bit
	rtc_set_gpio_p(1, wgpio_mask);	//set gpio wake up polarity
	rtc_set_wgpio(wgpio_mask);	  //set gpio wake up


	printf("press any key to enter standby: \n");
	getch();

	do
	{
#if 1
	   //set alarm after ten second
		systime = rtc_get_RTCsystime();
		systime.second += 10;
		if(systime.second > 59)
		{
			systime.second = systime.second % 60;
			systime.minute += 1;
			if(systime.minute > 59)
			{
				systime.minute = systime.minute % 60;
				systime.hour += 1;
			}
		}
		rtc_set_AlarmBySystime(&systime);  //set alarm after ten second
#endif
		rtc_set_AIN_p(1, WU_AIN0 | WU_AIN1);
		rtc_set_wakeup_type(WU_ALARM | WU_AIN0 | WU_AIN1);

		rtc_enter_standby();
		
		reason = rtc_exit_standby();
		
		switch(reason&0xFF)
		{
			case WU_GPIO:
				printf("gpio wakeup, gpio pin=%d\n", reason>>8);break;
			case WU_ALARM:
				printf("alarm wakeup\n");break;
			case WU_USB:
				printf("usb wakeup\n");break;
			case WU_AIN0:
				printf("AIN0 wakeup\n");break;	  
			case WU_AIN1:
				printf("AIN1 wakeup\n");break; 
		}
		
		mini_delay(2000);
	} while(0);   
#endif
return 0;	
}



/**
 * @brief   enter idle mode 
 * @author  
 * @date    2016-11-23
 * @param[in]   
 * @return  int
 * @retval  0 failed
 * @retval  1 successful
 */

int ak_drv_pm_enter_idle_mode()
{
	if(idle_flag)
	{
		ak_print_normal("have been idle_mode,no need to aet again!\n");
		return 0;
	}
	else
	{
		if(set_cpu_2x(true))
		{
			idle_flag = true;
			return 0;
		}
		else
		{
			return -1;
		}
	}
}
/**
 * @brief   leave idle mode 
 * @author  
 * @date    2016-11-23
 * @param[in]   
 * @return  int
 * @retval  0 successful
 * @retval  1 failed
 */

int ak_drv_pm_leave_idle_mode()
{
	if(idle_flag)
	{
		set_cpu_2x(false);
		idle_flag = false;
		return 0;
	}
	else
	{
		return -1;
	}
}
#endif
