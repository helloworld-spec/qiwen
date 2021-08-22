/**@file  hal_except.h
 * @brief system exception handlers
 * 
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */
#ifndef _HAL_EXCEPT_H_
#define _HAL_EXCEPT_H_

/** @defgroup Exception_Hanlder Exception group
 *  @ingroup Drv_Lib
 */
/*@{*/

typedef void (*T_fCExceptCallback)(unsigned char type);

/**
 * @brief set exception callback
 * @author
 * @date
 * @param[in] cb when exception occurs, the callback will be called
 * @return void
 */
void exception_set_callback(T_fCExceptCallback cb);

/**
 * @brief when exception occurs, reset processor multimedia part.
 * @author
 * @date
 * @return void
 */
void exception_reset_ap(void);

/*@}*/
#endif /* _HAL_EXCEPT_H_ */

