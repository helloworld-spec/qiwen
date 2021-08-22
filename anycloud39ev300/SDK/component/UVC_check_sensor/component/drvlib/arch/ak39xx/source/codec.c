/**
 * @FILENAME: codec.c
 * @BRIEF jpeg codec module
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR liao_zhijun
 * @DATE 2007.10.23
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "drv_api.h"
#include "drv_module.h"
#include "interrupt.h"

#define EVENT_CODEC_FINISH 1

static bool codec_intr_handler()
{
    //mask interrupt
    INTR_DISABLE(IRQ_MASK_VENC_BIT);

    //set event
    DrvModule_SetEvent(DRV_MODULE_CODEC, EVENT_CODEC_FINISH);
    
    return true;
}

/**
* @brief wait jpeg codec finish
* @author liao_zhijun
* @date 2010-10-17
* @return bool
*/
bool codec_wait_finish()
{
    signed long ret;
    
    //unmask interrupt
    INTR_ENABLE(IRQ_MASK_VENC_BIT);

    //wait event
    ret = DrvModule_WaitEvent(DRV_MODULE_CODEC, EVENT_CODEC_FINISH, 1000);

    if(DRV_MODULE_SUCCESS == ret)
        return true;
    else
        return false;
}

/**
* @brief enable jpeg codec interrupt
* @author liao_zhijun
* @date 2010-10-17
* @return bool
*/
bool codec_intr_enable()
{
    //creat task 
    if(!DrvModule_Create_Task(DRV_MODULE_CODEC))
    {
        akprintf(C1, M_DRVSYS, "create codec task failed\n");
        return false;
    }

    //register jpeg codec interrupt
    int_register_irq(INT_VECTOR_VENC, codec_intr_handler);

    //mask interrupt
    INTR_DISABLE(IRQ_MASK_VENC_BIT);   
    
    akprintf(C3, M_DRVSYS, "codec_intr_enable()\n");
    
    return true;
}

/**
* @brief disable jpeg codec interrupt
* @author liao_zhijun
* @date 2010-10-17
* @return void
*/
void codec_intr_disable()
{
    //mask interrupt
    INTR_DISABLE(IRQ_MASK_VENC_BIT);   
    
    //destroy task
    DrvModule_Terminate_Task(DRV_MODULE_CODEC);

    akprintf(C3, M_DRVSYS, "codec_intr_disable()\n");
}

