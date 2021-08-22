/**
 * @file hal_keypad.h
 * @brief keypad module, for keypad operations
 *
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2005.12.20
 * @version 1.0
 */
#ifndef __HAL_KEYPAD_H__
#define __HAL_KEYPAD_H__

/** @defgroup hal_keypad Keypad hal group
 *      @ingroup Keypad
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus


#define kbUnKnown                           (-1)

/*@{*/
#define DEFAULT_LONG_KEY_DELAY              1000    ///< default value, if a key is pressed down and hold more than 1000ms, a long press event will be sent
#define DEFAULT_POWERKEY_LONG_DELAY         3000    ///< this is the default value for long press of power key, that means if u press down pover key and hold more than 3000ms, power down event will be occured
#define DEFAULT_LOOP_KEY_DELAY              500     ///< default value, if u press down a key and don't release, after the first long key event, every 500 ms, a long key event will be sent

#define DEFAULT_KEYDOWN_DELAY               10      ///< default value, when key down,firstly delay 10ms,then detect
#define DEFAULT_KEYUP_DELAY                 10      ///< default value, when key up,firstly delay 10ms,then detect 
/* @} */

/**
 * @brief  key status define
 */
typedef enum
{
    eKEYDOWN,                       ///< key press down
    eKEYPRESS,                      ///< key press down and hold 
    eKEYUP,                         ///< key up
    eKEYSTATUS_NUM                  ///< num of key status
} T_KEYPADSTATUS;

/**
 * @brief  press mode define, it can be single mode or multiple mode
 */
typedef enum
{
    eSINGLE_PRESS = 0,      ///< single mode, only one key will be recognized, even there are other key pressed down, this is ordinary mode
    eMULTIPLE_PRESS,        ///< support more than one key pressed,  only used in games
    eKEYPRESSMODE_NUM       ///< the number of key press mode
} T_eKEY_PRESSMODE;

/**
 * @brief key value struct
 */
typedef struct
{
    unsigned long           keyID;          ///< key ID 
    bool          longPress;      ///< long press or short press
    T_KEYPADSTATUS  status;         ///< key status: down/hold/up 
}T_KEYPAD;

/**
 * @brief keypad callback function
 */
typedef void (*T_fKEYPAD_CALLBACK)(void);

/**
 * @brief Initialize keypad
 *
 * only need to call one time during system init
 * Function gpio_init() must be called before this function
 * @author Miaobaoli
 * @date 2004-09-21
 * @param[in] callback_func user defined Keypad callback function
 * @param[in] type_index keypad type
 * @param[in] para keypad parameter
 * @return void
 */
void keypad_init(T_fKEYPAD_CALLBACK callback_func, unsigned long type_index, const void* para);

/**
 * @brief get the keypad currently pressed, mainly used in keypad callback function
 * 
 * @author liao_zhijun
 * @date 2010-08-31
 * @param[in] key key structure pointer 
 * @return bool
 * @retval true get key success
 * @retval false get key failure, and key->keyID will kbNULL 
 */
bool keypad_get_key(T_KEYPAD *key);

/**
 * @brief  enable keypad interrupt
 *
 * Function keypad_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-08-31
 * @return void
 */
void keypad_enable_intr(void);

/**
 * @brief disable keypad interrupt
 *
 * Function keypad_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-08-31
 * @return void
 */
void keypad_disable_intr(void);

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
void keypad_set_delay(signed long keydown_delay, signed long keyup_delay, signed long keylong_delay, signed long powerkey_long_delay, signed long loopkey_delay);

/**
 * @brief scan keypad and get the value of the keypad currently pressed 
 *
 * Function keypad_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-08-31
 * @return signed long 
 * @retval The pressed key's scan code
 */
signed long keypad_scan(void);

/**
 * @brief set press mode for keypad
 * single press or multiple press, now multiple press just use in game
 * @author liao_zhijun
 * @date 2010-08-31
 * @param[in] press_mode single press or multiple press
 * @return bool
 * @retval true: set success
 * @retval false: set failure
 */
bool keypad_set_pressmode(T_eKEY_PRESSMODE press_mode);

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
void keypad_delect_key(void);

/**
 * @brief get keypad press mode
 * single press or multiple press, now multiple press just use in game
 * @author liao_zhijun
 * @date 2010-08-31
 * @return T_eKEY_PRESSMODE single press or multiple press
 * @retval eSINGLE_PRESS singel press
 * @retval eMULTIPLE_PRESS multiple press
 */
T_eKEY_PRESSMODE keypad_get_pressmode(void);


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

/*@}*/
#endif // #ifndef __KEYPAD_H__

