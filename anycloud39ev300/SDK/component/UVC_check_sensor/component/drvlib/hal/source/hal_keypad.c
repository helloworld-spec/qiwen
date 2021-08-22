/**
 * @file hal_keypad.c
 * @brief keypad source file
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010-07-24
 * @version 1.0
 */

#include "anyka_types.h"
#include "drv_api.h"
#include "hal_probe.h"

#include "drv_module.h"

//define keypad message
#define KEYPAD_MESSAGE 3

#define KEYPAD_VALUE_BUF_SIZE    64

//keypad buffer control struct
typedef struct
{
    unsigned long    keypad_head;
    unsigned long    keypad_tail;
    T_KEYPAD keypad_buf[KEYPAD_VALUE_BUF_SIZE];
} T_KEYPAD_BUF_CTRL;

//keypad struct
typedef struct
{
    unsigned long               init_flag;
    T_KEYPAD_BUF_CTRL   buf_ctrl;
    T_fKEYPAD_CALLBACK  callback_func;
} T_HAL_KEYPAD;


static void keypad_send_key(const T_KEYPAD *key);
static void keypad_callback(unsigned long *param, unsigned long len);

static T_KEYPAD_HANDLE *pKeypadHandle = NULL;

static T_HAL_KEYPAD hal_keypad = {0};

unsigned long pc_lr = 0;

/**
 * @brief Initialize keypad
 *
 * @author liao_zhijun
 * @date 2004-09-21
 * @param callback_func [in]: Keypad callback function
 * @param type_index [in]: keypad type index 
 * @param keypad_parm [in]: param for keypad init
 * @return void
 */
void keypad_init(T_fKEYPAD_CALLBACK callback_func, unsigned long type_index, const void* para)
{
    //init global keypad variables
    memset(&hal_keypad, 0, sizeof(hal_keypad));

    //save callback function
    hal_keypad.callback_func = callback_func;

    //probe keypad type
    pKeypadHandle = keypad_type_probe(type_index);
    if(NULL == pKeypadHandle)
    {
        akprintf(C1, M_DRVSYS, "THIS KEYPAD SCAN MODE NOT FIND,type_index=%d\r\n",type_index);
        return;
    }

    //call the specifical keypad type init function
    pKeypadHandle->KeyPadInit(keypad_send_key, para);

    //create task and map message for keypad
    DrvModule_Create_Task(DRV_MODULE_KEYPAD);
    DrvModule_Map_Message(DRV_MODULE_KEYPAD, KEYPAD_MESSAGE, keypad_callback);

    //change init flag
    hal_keypad.init_flag = 1;
}


/**
 * @brief send key
 * 
 * @author liao_zhijun
 * @date 2010-08-31
 * @param p_key [in]: key key structure pointer 
 * @return void
 */
static void keypad_send_key(const T_KEYPAD *p_key)
{
    bool ret = false;
    T_KEYPAD_BUF_CTRL *buf_ctrl = &hal_keypad.buf_ctrl;

    //do nothing here if keypad module not initialized
    if (0 == hal_keypad.init_flag)
        return;

    //if keypad queue is full, alarm here
    if ((buf_ctrl->keypad_tail + 1) % KEYPAD_VALUE_BUF_SIZE == buf_ctrl->keypad_head)
    {
        akprintf(C2, M_DRVSYS, "keypad_send_key key is full\r\n");
        akprintf(C2, M_DRVSYS, "pc_lr=0x%x\n", pc_lr);
    }

    buf_ctrl->keypad_buf[buf_ctrl->keypad_tail] = *p_key;
    buf_ctrl->keypad_tail = (buf_ctrl->keypad_tail + 1) % KEYPAD_VALUE_BUF_SIZE;

    /* active hisr */
    DrvModule_Send_Message(DRV_MODULE_KEYPAD, KEYPAD_MESSAGE, NULL);
}

/**
 * @brief get the keypad currently pressed
 * @author liao_zhijun
 * @date 2007-12-17
 * @param[in] key key structure pointer 
 * @return bool
 * @retval true get key success
 * @retval false get key failure, and key->keyID will kbNULL 
 */
bool keypad_get_key(T_KEYPAD *key)
{
    bool ret = false;
    T_KEYPAD_BUF_CTRL *buf_ctrl = &hal_keypad.buf_ctrl;

    //do nothing here if keypad module not initialized
    if (0 == hal_keypad.init_flag)
        return false;

    //param error or no key currently pressed
    if (key == NULL || (buf_ctrl->keypad_head == buf_ctrl->keypad_tail))
    {
        if (NULL != key)
        {
            key->keyID = kbUnKnown;
        }
        
        return ret;
    }

    //get key value from queue
    *key = buf_ctrl->keypad_buf[buf_ctrl->keypad_head];
    buf_ctrl->keypad_head = (buf_ctrl->keypad_head + 1) % KEYPAD_VALUE_BUF_SIZE;
    
    return true;
}


//************************************************************************

/**
 * @brief scan keypad and get the value of the keypad currently pressed 
 *
 * Function keypad_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-08-31
 * @return signed long 
 * @retval The pressed key's scan code
 */
signed long keypad_scan(void)
{
    signed long key = kbUnKnown;
    
    if(pKeypadHandle->KeyPadScan)
        key = pKeypadHandle->KeyPadScan();
    else    
        akprintf(C2, M_DRVSYS, "hal_keypad_scan is null\r\n"); 

    return key;
}

/**
 * @brief enable interrupt for keypad
 *
 * Function keypad_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-08-31
 * @return void
 */
void keypad_enable_intr(void)
{
    if(pKeypadHandle->KeyPadEnIntr)
        pKeypadHandle->KeyPadEnIntr();
    else    
        akprintf(C2, M_DRVSYS, "hal_keypad_enable_intr is null\r\n");        
}

/**
 * @brief disable interrupt for keypad
 *
 * Function keypad_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-08-31
 * @return void
 */
void keypad_disable_intr(void)
{
    if(pKeypadHandle->KeyPadDisIntr)
        pKeypadHandle->KeyPadDisIntr();
    else    
        akprintf(C2, M_DRVSYS, "hal_keypad_enable_intr is null\r\n");        
}

/**
 * @brief set press mode for keypad
 *
 * @author liao_zhijun
 * @date 2010-08-31
 * @param press_mode [in]: press mode to be set
 * @return void
 */
bool keypad_set_pressmode(T_eKEY_PRESSMODE press_mode)
{
    bool ret = false;
    
    if(pKeypadHandle->SetMode)
        ret = pKeypadHandle->SetMode(press_mode);
    else    
        akprintf(C2, M_DRVSYS, "hal_keypad_set_pressmode is null\r\n");

    return ret;
}


/**
 * @brief delete keypad driver and release keypad task 
 * 
 * @author pyy
 * @date 2017-05-13
 * @param[in]  
 * @return 
 * @retval 
 * @retval 
 */
void keypad_delect_key(void)
{
	
    DrvModule_Terminate_Task(DRV_MODULE_KEYPAD);//release keypad task
	
}

/**
 * @brief get press mode of keypad
 *
 * @author liao_zhijun
 * @date 2010-08-31
 * @return T_eKEY_PRESSMODE
 * @retval eSINGLE_PRESS singel press
 * @retval eMULTIPLE_PRESS multiple press
 */
T_eKEY_PRESSMODE keypad_get_pressmode(void)
{
    T_eKEY_PRESSMODE mode = eKEYPRESSMODE_NUM;
    
    if(pKeypadHandle->GetMode)
       mode = pKeypadHandle->GetMode();
    else    
        akprintf(C2, M_DRVSYS, "hal_keypad_get_pressmode is null\r\n");

    return mode;
}


/**
 * @brief set all kinds of delay for keypad
 * @author liao_zhijun
 * @date 2010-08-31
 * @param[in] keylong_delay long key delay time (millisecond),must >0
 * @param[in] keydown_delay long key delay time (millisecond),must >=0
 * @param[in] keyup_delay long key delay time (millisecond),must >=0
 * @param[in] powerkey_long_delay long key delay time (millisecond),must >0
 * @param[in] loopkey_delay loop key delay time (millisecond),must >0
 * @return void
 */
void keypad_set_delay(signed long keydown_delay, signed long keyup_delay, signed long keylong_delay, signed long powerkey_long_delay, signed long loopkey_delay)
{
    if(pKeypadHandle->SetDelay)
       pKeypadHandle->SetDelay(keydown_delay, keyup_delay, keylong_delay, powerkey_long_delay, loopkey_delay);
    else    
        akprintf(C2, M_DRVSYS, "hal_keypad_get_pressmode is null\r\n");        
}


//************************************************************************

static void keypad_callback(unsigned long *param, unsigned long len)
{
    //just call the callback function here
    if (NULL != hal_keypad.callback_func && 0 != hal_keypad.init_flag)
    {
        hal_keypad.callback_func();
    }
}


