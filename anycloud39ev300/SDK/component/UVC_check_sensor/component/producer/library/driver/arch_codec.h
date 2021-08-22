/**@file Arch_codec.h
 * @brief list driver library jpeg codec function
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2011-10-17
 * @version 1.0
 * @note refer to ANYKA chip technical manual.
 */
#ifndef __ARCH_CODEC_H__
#define __ARCH_CODEC_H__

#include "anyka_types.h"

/** @defgroup Codec Codec group
 *  @ingroup Drv_Lib
 */
/*@{*/

/**
* @brief wait jpeg codec finish
* @author liao_zhijun
* @date 2010-10-17
* @return void
*/
bool codec_wait_finish(void);


/**
* @brief enable jpeg codec interrupt
* @author liao_zhijun
* @date 2010-10-17
* @return bool
*/
bool codec_intr_enable(void);

/**
* @brief disable jpeg codec interrupt
* @author liao_zhijun
* @date 2010-10-17
* @return void
*/
void codec_intr_disable(void);


/*@}*/
#endif //__ARCH_INIT_H__

