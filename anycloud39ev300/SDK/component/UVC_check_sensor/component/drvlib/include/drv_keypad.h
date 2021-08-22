/**
 * @file drv_keypad.h
 * @brief keypad module, for keypad register
 *
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2005.12.20
 * @version 1.0
 */
#ifndef __DRV_KEYPAD_H__
#define __DRV_KEYPAD_H__

#include "hal_keypad.h"

/** @defgroup Keypad Keypad group
 *  @ingroup Drv_Lib
 */


/** @defgroup drv_keypad Keypad driver group
 *  @ingroup Keypad
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus


/**
 * @brief: Keypad callback define.
 */
typedef void (*T_f_H_KEYPAD_CALLBACK)(const T_KEYPAD *keypad); 

/**
 * @brief  key handler define
 */
typedef struct
{
    void (*KeyPadInit)(T_f_H_KEYPAD_CALLBACK callback_func, const void *keypad_parm);//keypad init function
    
    signed long  (*KeyPadScan)(void); //directly scan function, get the key value currently pressed
    void (*KeyPadEnIntr)(void);//enable keypad interrupt
    void (*KeyPadDisIntr)(void);//disable keypad interrupt
    
    T_eKEY_PRESSMODE (*GetMode)(void);//get curent keypad mode
    bool (*SetMode)(T_eKEY_PRESSMODE press_mode);//set press mode
    
    void (*SetDelay)(signed long keydown_delay, signed long keyup_delay, signed long keylong_delay, signed long powerkey_long_delay, signed long loopkey_delay);//adjust keypad param
} T_KEYPAD_HANDLE;

/**
 * @brief register keypad scan mode
 *
 * @author Miaobaoli
 * @date 2004-09-21
 * @param[in] index keypad index
 * @param[in] handler keypad handler 
 * @return bool
 */
bool keypad_reg_scanmode(unsigned long index, T_KEYPAD_HANDLE *handler);


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

/*@}*/
#endif // #ifndef __DRV_KEYPAD_H__

