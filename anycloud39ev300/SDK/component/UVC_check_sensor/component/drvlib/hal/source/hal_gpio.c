/**
 * @FILENAME: hal_gpio.c
 * @BRIEF config gpio
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-12-19
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "drv_module.h"

static T_GPIO_SET *m_pGpioSetting = NULL;  //no use  17-7-21

//#define GPIO_DRV_MESSAGE 1

volatile static int gpio_task_use = 0;

/**
 * @BRIEF gpio pin config init
 * @AUTHOR guoshaofeng
 * @DATE 2007-12-19
 * @PARAM [in] pGpioSetting: config array pointer
 * @RETURN void
 * @RETVAL
 */
void gpio_pin_config_init(T_GPIO_SET *pGpioSetting)
{
    unsigned long i;

    for (i=0; ;i++)
    {
        if (GPIO_END_FLAG == pGpioSetting[i].pinNum)
        {
            break;
        }

        if (INVALID_GPIO == pGpioSetting[i].pinNum)
        {
            continue;
        }

        if (ePullUpEn == pGpioSetting[i].pull)
        {
            gpio_set_pull_up_r(pGpioSetting[i].pinNum, true);
        }
        else if (ePullUpDis == pGpioSetting[i].pull)
        {
            gpio_set_pull_up_r(pGpioSetting[i].pinNum, false);
        }
        else if (ePullDownEn == pGpioSetting[i].pull)
        {
            gpio_set_pull_down_r(pGpioSetting[i].pinNum, true);
        }
        else if (ePullDownDis == pGpioSetting[i].pull)
        {
            gpio_set_pull_down_r(pGpioSetting[i].pinNum, false);
        }
        
        gpio_set_pin_dir(pGpioSetting[i].pinNum, pGpioSetting[i].pinDir);
        if (GPIO_DIR_OUTPUT == pGpioSetting[i].pinDir)
        {
            gpio_set_pin_level(pGpioSetting[i].pinNum, pGpioSetting[i].pinDefaultLevel);
        }
    }	

    m_pGpioSetting = pGpioSetting;
}

/**
 * @BRIEF gpio_pin_get_ActiveLevel
 * @AUTHOR guoshaofeng
 * @DATE 2007-12-19
 * @PARAM [in] pin: gpio pin ID.
 * @RETURN unsigned char: pin level, 1: high; 0: low; 0xff: invalid_level
 * @RETVAL
 */
unsigned char gpio_pin_get_ActiveLevel(unsigned char pin)
{
    unsigned long i;
    unsigned char activeLevel = 0xff;

    if (NULL == m_pGpioSetting)
    {
        return activeLevel;
    }

    if (INVALID_GPIO == pin)
    {
        return activeLevel;
    }
    else
    {
        for (i=0; ;i++)
        {
            if (GPIO_END_FLAG == m_pGpioSetting[i].pinNum)
            {
                activeLevel = 0xff;
                break;
            }

            if (pin == m_pGpioSetting[i].pinNum)
            {
                activeLevel = m_pGpioSetting[i].pinActiveLevel;
                break;
            }
        }
    }

    return activeLevel;
}


/**
 * @brief get which pin wakeup  
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] reason wake up status.
 * @return unsigned char
 * @retval gpio pin
 */
unsigned char gpio_get_wakeup_pin(unsigned long reason)
{
    unsigned long i;    

    for (i = 0 ; i <=31  ; i++)
    {
        if (((1 << i)  & reason) != 0 )
        {
            break;
        }
    }

    return get_wGpio_Pin(i);
}


typedef struct  _GPIO_MSG 
{
	unsigned int pin;
	unsigned char polarity; 
	
}T_GPIO_MSG;

static void gpio_drv_callback( unsigned int pin, unsigned char polarity )
{
	unsigned long gpio_nb = pin;
	volatile T_GPIO_MSG gpio_msg;
	unsigned char level;
	
	gpio_msg.pin = pin;
	//gpio_msg.polarity = polarity;
	
    level = gpio_get_pin_level(gpio_nb);
	gpio_msg.polarity = level;
	//akprintf(C1, M_DRVSYS, "============gpio %d, %d\n",gpio_msg.pin,gpio_msg.polarity);

    gpio_set_int_p(gpio_nb, !level);
    gpio_int_control(gpio_nb, true);	
	if(!DrvModule_Send_Message(DRV_MODULE_GPIO, gpio_nb, (unsigned long*)&gpio_msg))
	{
		akprintf(C1, M_DRVSYS, "send GPIO msg fail\n");
	}
}

/**
 * @brief  gpio_intr_enable
 * @author wumingjin
 * @date   2017-05-11
 * @param[in] pin
 * @param[in] mode  0 level trigger, 1 edge trigger
 * @param[in] polarity 0 low /falling, 1 high/rising
 * @param[in] callback
 * @return bool
 */
bool gpio_intr_enable ( unsigned long pin, unsigned char mode, unsigned char polarity, T_fDRV_CALLBACK callback )//T_fDRV_CALLBACK kjp
{
    unsigned long gpio_nb = pin;
    unsigned char gpio_level = polarity;


	if(false == gpio_assert_legal(pin))
	{
		return false;
	}
	
	if(0 == gpio_task_use) //change  k
	{
	    //create gpio drv task
	    if(!DrvModule_Create_Task(DRV_MODULE_GPIO))
	    {
	       akprintf(C1, M_DRVSYS, "create gpio intrrupt task failed\n");
	       return false;
	    }

	}
	gpio_task_use++;
	
    //set massage and handler
    if(!DrvModule_Map_Message(DRV_MODULE_GPIO, gpio_nb, callback))
    {
		gpio_task_use--;
		
		if(0 ==  gpio_task_use)// just
		{
			DrvModule_Terminate_Task(DRV_MODULE_GPIO);
		}
		akprintf(C1, M_DRVSYS, "map gpio %d message  failed\n",gpio_nb);
		return false;
	}

    gpio_set_pin_dir(pin, 0);
	gpio_set_int_mode(pin, mode);
	
	if(0 == mode)
	{
		gpio_level = gpio_get_pin_level(gpio_nb); 
		//akprintf(C1, M_DRVSYS, "============map gpio %d\n",gpio_level);
		gpio_level  = !gpio_level;
	    //akprintf(C1, M_DRVSYS, "============map gpio %d\n",gpio_level);

	}
	
	gpio_register_int_callback(pin, gpio_level, 1, gpio_drv_callback);

	
    return true;
}

/**
 * @brief  gpio_intr_disable
 * @author wumingjin
 * @date   2017-05-11
 * @param[in] pin
 * @return bool
 */
bool gpio_intr_disable ( unsigned long pin)
{

	if(false == gpio_assert_legal(pin) || (0 == gpio_task_use))
	{
		return false;
	}
	gpio_int_control( pin, 0);
	
	 //destroy task
	gpio_task_use--;
	 
	if(0 ==  gpio_task_use)// just
	{
		DrvModule_Terminate_Task(DRV_MODULE_GPIO);
	}
    
	
    akprintf(C3, M_DRVSYS, "gpio %d intr disabled\n", pin);
	return true;
}




