#include <stdio.h>

#include "hw_memmap.h"
#include "hw_types.h"
#include "pin.h"
#include "rom_map.h"
#include "prcm.h"

#include "gpio_hal.h"
#include "timer.h"

//#include "uart_if.h"
//#include "gpio_if.h"
#include "timer_if.h"

//#include "ak_gpio.h"
#include "ak_key_led.h"

#include "cc_types.h"
#include "cc_timer.h"


static void key_time_callback(cc_hndl handle);



#define KEY_NUMBER_MAX 4

typedef struct Key
{
	cc_hndl gpio_hndl;
	unsigned long gpio_num;
	unsigned char int_level;
	unsigned char status;
	int time_id;
	struct u64_time sIntervalTimer;
	cc_hndl tTimerHndl;
	void (* keyInt_Handle)(void);
}KEY,*pKEY;


//extern void (*key_callback)(unsigned char io_num);
static unsigned char key_count = 0;

static const unsigned int interrupt_level[2] = {2, 6};

	
static KEY s_Key[KEY_NUMBER_MAX];

void gpioKey_function_callback(unsigned char io_num)
{
	unsigned int keyPort = 0;
	unsigned char keymask, i;
	i32 time_rt;
	pKEY key = NULL;
	for(i = 0 ; i < key_count ; i++)
	{
		if(s_Key[i].gpio_num== io_num)
		{
			break;
		}
	}
	if(i >= key_count)return;
	//Report("k\r\n");
	key = &s_Key[i];
	//setup 1: close gpio interrupt
	//gpio_int_control(key->gpio_num, 0);
	//setup 2: start timer 
	if((0 == key->sIntervalTimer.nsec)&&(0 == key->sIntervalTimer.secs))
	{
		key->keyInt_Handle();
		return ;
	}
	else
	{
		cc_gpio_disable_notification(key->gpio_hndl,key->gpio_num);
		time_rt = cc_timer_start(key->tTimerHndl, &(key->sIntervalTimer), OPT_TIMER_ONE_SHOT);
	}
	//s_Key[i].time_id = vtimer_start(100, false, key_time_callback);
	//setup 3:if timer_Id error, open gpio interrupt 
	//if(-1== time_rt)gpio_int_control(key->gpio_num, 1);

}

static void key_time_callback(cc_hndl handle)
{
	//unsigned long value;
	enum gpio_int_type int_type;
	unsigned int keyPort = 0;
	unsigned char level, keymask, i;
	pKEY key = NULL;
	unsigned char pre_status = 0;
	unsigned long gpio_num;
	gpio_num = *((unsigned long *)handle);
	//Report("time_addr: %x,num: %d\n", handle, gpio_num);
	for(i = 0 ; i < key_count ; i++)
	{
		if(s_Key[i].gpio_num== gpio_num)
		{
			break;
		}
	}
	if(i >= key_count)return;
#if 1
	key = &s_Key[i];
	
	//level = gpio_get_pin_level(key->gpio_num);
	cc_gpio_read(key->gpio_hndl,key->gpio_num,&level);

	if(level != key->int_level)
	{
		pre_status = 0;
		
	}
	else 
	{
		pre_status = 1;
	}
	//setup 1: if status is changed
	if(pre_status != key->status)
	{
		key->status = pre_status;
		//setup a: goto function();
		if(pre_status)key->keyInt_Handle();
		else
		{
			//led_ctrol(10,0);
			//Report("key pull up\n");
		}

		//setup b:set interrupt polarity reverse
		//if(level)level = 0;
		//else level = 1;
		//gpio_set_int_p(key->gpio_num,interrupt_level[level]);
		
	}
	if(level)
	{
		int_type = INT_LEVEL_LOW;

	}
	else
	{
		int_type = INT_LEVEL_HIGH;
	}
	//setup 2: open gpio interrupt
	cc_gpio_enable_notification(key->gpio_hndl, key->gpio_num, int_type, GPIO_TYPE_NORMAL);
	//gpio_int_control(key->gpio_num, 1);

	//setup 3:close timer interrutp
	//vtimer_stop(key->time_id);
#endif	
}

/**
 * @brief init led gpio with output function
 * @pram [in] gpio_num : gpio wake up -> gpio_num
 * @author Jiankui
 * @date 2016-12-08
 * @return void
 */
void led_open(unsigned char gpio_num)
{
	//gpio_init(gpio_num,0);
	//gpio_set_pin_dir(gpio_num,1);
}

/**
 * @brief set led on/off function,befor must led_open function
 * @pram [in] gpio_num : gpio wake up -> gpio_num
 * @pram [in] value :write gpio out date value, 0 : led off,   1:led on
 * @author Jiankui
 * @date 2016-12-08
 * @return void
 */
void led_ctrol(unsigned char gpio_num, unsigned char value)
{
	//gpio_set_pin_level(gpio_num,value);
}


/**
 * @brief Key function open
 * @pram [in] gpio_num : key -> gpio_num
 * @pram [in] IntType  : interrupt triggered condition , 1:high level    0:low level
 * @pram [in] sec  :key put down effective time 
 * @pram [in] key_Int_Handler : effective key interrupt callback function address 
 * @author Jiankui
 * @date 2016-12-08
 * @return int
 * @retval 0 success
  * @retval -1 fail
 */
int key_open(unsigned char gpio_num, unsigned char IntType, unsigned long MilliSecs, void (*key_Int_Handler)(void))
{
	enum gpio_int_type int_type;
	unsigned int keyPort;
	unsigned char port = 0,keymask;
	struct cc_timer_cfg sRealTimeTimer;
    pKEY key = NULL;
	if(key_count > KEY_NUMBER_MAX) return -1;
	key = &s_Key[key_count];
	key->gpio_num = gpio_num;
	key->int_level = IntType;
	key->status = 0;
	//key->ms = ms;
	key->keyInt_Handle = key_Int_Handler;
	//key->time_id = ERROR_TIMER;
	if(0 != MilliSecs)
	{
		key->sIntervalTimer.secs = MilliSecs/1000;
	    key->sIntervalTimer.nsec = (MilliSecs%1000)*1000000;
		sRealTimeTimer.source = HW_REALTIME_CLK;
	    sRealTimeTimer.timeout_cb = key_time_callback;
	    sRealTimeTimer.cb_param = &(key->gpio_num);

	    key->tTimerHndl = cc_timer_create(&sRealTimeTimer);
		//Report(" creat time: %x,%x\n", key->tTimerHndl,&(key->gpio_num));
	}
	if(0 == IntType)
	{
		int_type = INT_LEVEL_LOW;
	}	
	else
	{
		int_type = INT_LEVEL_HIGH;
	}
	
	//key_callback = gpioKey_function_callback;
	//Report("cb:%x\r\n",gpioKey_function_callback);
	key->gpio_hndl = cc_gpio_open(key->gpio_num, GPIO_DIR_INPUT);
    cc_gpio_enable_notification(key->gpio_hndl, key->gpio_num, int_type, GPIO_TYPE_NORMAL);
	//port = gpio_num/8;
	
	//gpio_init(key->gpio_num, 2); 
	//gpio_set_pin_dir(key->gpio_num, 0);  //input
	//gpio_set_int_p(key->gpio_num, interrupt_level[IntType]); //set interrupt
	//gpio_register_int_callback(key->gpio_num, gpioKey_function_callback);  //register callback
	//gpio_int_control(key->gpio_num, 1); //enable gpio interrupt
	key_count++;
	return 0;
}

