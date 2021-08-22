/**
 * @FILENAME tcm.h
 * @BRIEF  Tightly coupled memory driver
 * Copyright @ 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR   LianGenhui
 * VERSION   1.0
 * @REF 
 */    

#ifndef __TCM_H__
#define __TCM_H__

#include "anyka_types.h"


typedef enum
{
    MODE_DTCM = 0,
    MODE_ITCM = 1
}
E_TCM_TYPE;

/**
 * @brief enable tcm
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param mode DTCM or ITCM
 * @return void
 */
void tcm_enable(E_TCM_TYPE mode);

/**
 * @brief disable tcm
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param
 * @return void
 */
void tcm_disable();

#endif // #ifndef __HTIMER_H__

