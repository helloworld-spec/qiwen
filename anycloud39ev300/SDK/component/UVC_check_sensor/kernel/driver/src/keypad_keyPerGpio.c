#include "drv_api.h"
#include "drv_keypad.h"
#include "anyka_types.h"
#include "drv_timer.h"
#include "dev_drv.h"
#include "platform_devices.h"
#include "kernel.h"


#define KEYPAD_TIMER        uiTIMER1

static T_eKEY_PRESSMODE CurrentPressMode = eSINGLE_PRESS;

/*
扫描一个键盘
*/
const T_PLATFORM_KEYPAD_GPIO *pKeypadPara=NULL;
static T_f_H_KEYPAD_CALLBACK sys_keypad_callback = NULL;

static unsigned long ShakeTime=100;
static unsigned long LongPressTime=500;
//static unsigned long LongPressTime=2000;


#define LONGPRESSCNTFLAG (LongPressTime/ShakeTime + 5)
/*
当按下时间计数为0xff时，判断为该gpio被Mask
*/
#define GPIO_MASK_FLAG 0xff

static signed long KeypadTimer = ERROR_TIMER;
static signed long KeypadLockTimer = ERROR_TIMER;

static signed long keypad_scan_KeyPerGpio(void) 
{
    unsigned long i;
        
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if (gpio_get_pin_level(pKeypadPara->RowGpio[i]) == pKeypadPara->active_level)        
            return pKeypadPara->keypad_matrix[i];
    }
    return 0xffffffff;
}



 void keypad_enable_intr_KeyPerGpio(void)
{
    unsigned long i;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            continue;    
            
        gpio_int_control(pKeypadPara->RowGpio[i], true);
    }    
}

 void keypad_disable_intr_KeyPerGpio(void)
{
    unsigned long i;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            continue;    
    
        gpio_int_control(pKeypadPara->RowGpio[i], false);
    }
}

static bool keypad_SetMode_KeyPerGpio(T_eKEY_PRESSMODE press_mode)
{
    CurrentPressMode = press_mode;
    return true;
}


static void keypad_timer_scan(signed long timer_id, unsigned long delay)
{
    unsigned long i,j;
    bool isSomeKeyPress;
    T_KEYPAD keypad;
    
    isSomeKeyPress = false;
    //save press cnt
    for(i=0;i<pKeypadPara->row_qty;i++)
    {   
        if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            continue;            
        if (gpio_get_pin_level(pKeypadPara->RowGpio[i]) == pKeypadPara->active_level)//down             
        {
            if(0 == pKeypadPara->updown_matrix[i])//this key is first down
            {
                //send down   
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 	
				
                keypad.longPress = false;         
                keypad.status = eKEYDOWN;
                sys_keypad_callback(&keypad);
            }
            else if(pKeypadPara->updown_matrix[i] < (LongPressTime/ShakeTime))//send long
            {
                //no action,wait long comming,or short up
            }
            else if(pKeypadPara->updown_matrix[i] == (LongPressTime/ShakeTime))//send long
            {
                //send long+press
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = true;         
                keypad.status = eKEYPRESS;
                sys_keypad_callback(&keypad);
            }
            else
            {
                //send press after long flag
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = false;         
                keypad.status = eKEYPRESS;
                sys_keypad_callback(&keypad);
                
                pKeypadPara->updown_matrix[i] = LONGPRESSCNTFLAG;//Prevent overflow    
            }
            
            (pKeypadPara->updown_matrix[i])++;
            isSomeKeyPress = true;
        }
        else//up
        {
            if (pKeypadPara->updown_matrix[i] == 0)//up of long press
            {
                //no press,no action
            }
            else if (pKeypadPara->updown_matrix[i] < (LongPressTime/ShakeTime))
            {
                //short press up,send press and up
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = false;         
                keypad.status = eKEYPRESS;
                sys_keypad_callback(&keypad);
                
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = false;         
                keypad.status = eKEYUP;
                sys_keypad_callback(&keypad);
            }
            else
            {
                //long press up, send up 
                keypad.keyID = pKeypadPara->keypad_matrix[i]; 
                keypad.longPress = false;         
                keypad.status = eKEYUP;
                sys_keypad_callback(&keypad);
            }
            
            pKeypadPara->updown_matrix[i] = 0;   
        }
    }
    
    //No key press
    if (!isSomeKeyPress)
    {
        timer_stop(KeypadTimer);
        //vtimer_stop(KeypadTimer);
        KeypadTimer = ERROR_TIMER;
        keypad_enable_intr_KeyPerGpio();
    }
}

static void keypad_gpio_callback(unsigned long pin, unsigned char polarity)
{
    unsigned long j;
    
    keypad_disable_intr_KeyPerGpio();//close all gpio intr,use timer scan
    
    //akprintf(C3, M_DRVSYS, "keypad gpio shake,PIN=%d",pin);
    
    if(KeypadTimer != ERROR_TIMER)
    {
        akprintf(C3, M_DRVSYS, "keypad scan is running,PIN=%d\n",pin);
        keypad_enable_intr_KeyPerGpio();
        return;
    }
    
    KeypadTimer = timer_start(KEYPAD_TIMER,ShakeTime, true, keypad_timer_scan);
	
    //KeypadTimer = vtimer_start(ShakeTime, true, keypad_timer_scan);
    if (ERROR_TIMER == KeypadTimer)
    {
        akprintf(C3, M_DRVSYS, "can't malloc timer in keypad_gpio_callback(keypergpio keypad)\n");
        keypad_enable_intr_KeyPerGpio();
        return;
    }
}


#if 0

//**************************lock handle*************************************//

static void keypad_locktimer_scan(signed long timer_id, unsigned long delay)
{
    unsigned long i;
    T_KEYPAD keypad;
    
    if(KeypadLockTimer != ERROR_TIMER)
    {
        timer_stop(KeypadLockTimer);
        KeypadLockTimer = ERROR_TIMER;
    }
    
    if (gpio_get_pin_level(pKeypadPara->switch_key_id) == pKeypadPara->switch_key_active_level)       
    {
        akprintf(C3, M_DRVSYS, "Keypad is locked by pin %d",pKeypadPara->switch_key_id);
        
        for(i=0;i<pKeypadPara->row_qty;i++)
        {
            if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            {
                akprintf(C3, M_DRVSYS, "pin %d is been masked",pKeypadPara->updown_matrix[i]);
                continue;
            }
            
            gpio_int_control(pKeypadPara->RowGpio[i], false);
        }    
        
        
        //send hardware lock 
        keypad.keyID = pKeypadPara->switch_key_value; 
        keypad.longPress = true;         
        keypad.status = eKEYPRESS;
        sys_keypad_callback(&keypad);    
        
        //change int polarity
        gpio_set_int_p(pKeypadPara->switch_key_id, 1 - pKeypadPara->switch_key_active_level);     
        
    }
    else
    {
        akprintf(C3, M_DRVSYS, "Keypad locked shake by pin %d",pKeypadPara->switch_key_id);
    }
    
    gpio_int_control(pKeypadPara->switch_key_id, true);
}

static void keypad_unlocktimer_scan(signed long timer_id, unsigned long delay)
{
    unsigned long i;
    T_KEYPAD keypad;
    
    if(KeypadLockTimer != ERROR_TIMER)
    {
        timer_stop(KeypadLockTimer);
        KeypadLockTimer = ERROR_TIMER;
    }
    
    if (gpio_get_pin_level(pKeypadPara->switch_key_id) == (1-pKeypadPara->switch_key_active_level))       
    {
        akprintf(C3, M_DRVSYS, "Keypad is unlocked by pin %d",pKeypadPara->switch_key_id);
        
        for(i=0;i<pKeypadPara->row_qty;i++)
        {
            if(pKeypadPara->updown_matrix[i] == GPIO_MASK_FLAG)
            {
                akprintf(C3, M_DRVSYS, "pin %d is been masked",pKeypadPara->updown_matrix[i]);
                continue;
            }
            
            pKeypadPara->updown_matrix[i] = 0;
            gpio_int_control(pKeypadPara->RowGpio[i], true);
        }    
        
        
        //send hardware UNlock 
        keypad.keyID = pKeypadPara->switch_key_value; 
        keypad.longPress = false;         
        keypad.status = eKEYPRESS;
        sys_keypad_callback(&keypad);       
        
        //change int polarity
        gpio_set_int_p(pKeypadPara->switch_key_id, pKeypadPara->switch_key_active_level); 
        
    }
    else
    {
        akprintf(C3, M_DRVSYS, "Keypad unlocked shake by pin %d",pKeypadPara->switch_key_id);
    }
    
    gpio_int_control(pKeypadPara->switch_key_id, true);
}


static void keypad_lockgpio_callback(unsigned long pin, unsigned char polarity)
{
    akprintf(C3, M_DRVSYS, "keypad_lockgpio_callback,pin = %d",pin);
    
    gpio_int_control(pin, false);
    if(KeypadLockTimer != ERROR_TIMER)
    {
        akprintf(C3, M_DRVSYS, "KeypadLockTimer is running,PIN=%d\n",pin);
        gpio_int_control(pin, true);
        return;
    }
    
    if (polarity == pKeypadPara->switch_key_active_level)
        KeypadLockTimer = timer_start(KEYPAD_TIMER,LongPressTime, false, keypad_locktimer_scan);
    else
        KeypadLockTimer = timer_start(KEYPAD_TIMER,LongPressTime, false, keypad_unlocktimer_scan);
    
    
    if (ERROR_TIMER == KeypadLockTimer)
    {
        akprintf(C3, M_DRVSYS, "can't malloc timer in keypad_lockgpio_callback(keypergpio keypad)\n");
        gpio_int_control(pin, true);
        return;
    }
}

#endif

 void keypad_init_KeyPerGpio(T_f_H_KEYPAD_CALLBACK callback_func,const void *keypad_parm)
{
    unsigned long i;
	
    pKeypadPara = (T_PLATFORM_KEYPAD_GPIO *)keypad_parm;
/*
    check parameter,in here,we only chage updown_matrix
*/    
    if (NULL==pKeypadPara->updown_matrix)
    {
        akprintf(C3, M_DRVSYS, "keypad_parm->updown_matrix is null\n");
    }    
    
    KeypadTimer = ERROR_TIMER;
    sys_keypad_callback = callback_func;
   
    for(i=0;i<pKeypadPara->row_qty;i++)
    {	
        pKeypadPara->updown_matrix[i] = 0;
        gpio_set_pin_dir(pKeypadPara->RowGpio[i], 0);//input
        gpio_set_int_mode(pKeypadPara->RowGpio[i], 0);//set level triggered interrupt
        gpio_set_pull_up_r(pKeypadPara->RowGpio[i], false);//closepullup
        gpio_set_pull_down_r(pKeypadPara->RowGpio[i], true);//openpulldown
    }
	 
    //register callback
    for(i=0;i<pKeypadPara->row_qty;i++)
    {        
        gpio_register_int_callback(pKeypadPara->RowGpio[i], pKeypadPara->active_level, 0, keypad_gpio_callback);//para=(gpio,high,disable,callback)       
    }
	#if 0
    /*next is keypad lock seting*/
    if (pKeypadPara->switch_key_id == INVALID_GPIO)
    {
        akprintf(C3, M_DRVSYS, "lock key gpio is invalid\n");
        return;
    }
    
    gpio_set_pin_dir(pKeypadPara->switch_key_id, 0);//input
    gpio_set_pull_up_r(pKeypadPara->switch_key_id, false);//closepullup
    gpio_set_pull_down_r(pKeypadPara->switch_key_id, true);//openpulldown
    gpio_register_int_callback(pKeypadPara->switch_key_id, pKeypadPara->switch_key_active_level, 1, keypad_lockgpio_callback);//para=(gpio,high,disable,callback) 
	#endif
}

#if 0
static void keypad_KeyPerGpio_AddMaskGpio(unsigned long pin)
{
    unsigned long i;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if(pin == pKeypadPara->RowGpio[i])
        {
            pKeypadPara->updown_matrix[i] = GPIO_MASK_FLAG;
            gpio_int_control(pin, false);
            //akprintf(C3, M_DRVSYS, "KeyPerGpio_AddMaskGpio success,pin=%d\n",pin);
            return;
        }   
    }
    
    akprintf(C3, M_DRVSYS, "KeyPerGpio_AddMaskGpiofail,pin=%d\n",pin);
}

static void keypad_KeyPerGpio_RemoveMaskGpio(unsigned long pin)
{
    unsigned long i;
    
    for(i=0;i<pKeypadPara->row_qty;i++)
    {
        if(pin == pKeypadPara->RowGpio[i])
        {
            pKeypadPara->updown_matrix[i] = 0;
            //akprintf(C3, M_DRVSYS, "open intr,pin=%d\n",pin);
            gpio_int_control(pin, true);
            return;
        }   
    }
    
    akprintf(C3, M_DRVSYS, "pin=%d not add by KeyPerGpio_AddMaskGpio\n",pin);
}
#endif
//******************************reg my handle info***************************//

static T_KEYPAD_HANDLE KeyPerGpio_handler = 
{
	.KeyPadInit    = keypad_init_KeyPerGpio,
	.KeyPadScan    = keypad_scan_KeyPerGpio,
	.KeyPadEnIntr  = keypad_enable_intr_KeyPerGpio, 
	.KeyPadDisIntr = keypad_disable_intr_KeyPerGpio, 
	.SetMode        = keypad_SetMode_KeyPerGpio,
	
};

static int keyPerGpio_reg(void)
{
	keypad_reg_scanmode(KEYPAD_KEY_PER_GPIO,&KeyPerGpio_handler);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(keyPerGpio_reg)
#ifdef __CC_ARM
#pragma arm section
#endif



