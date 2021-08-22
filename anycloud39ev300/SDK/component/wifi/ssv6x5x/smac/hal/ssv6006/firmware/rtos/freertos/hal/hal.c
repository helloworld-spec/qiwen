
/*****************************************************************************
 *
 *            Copyright Andes Technology Corporation 2009-2010
 *                         All Rights Reserved.
 *
 *  Revision History:
 *
 *    Dec.28.2009     Created.
 ****************************************************************************/

#include "hal.h"

#if 0
inline void hal_show_err( INT8U err, const char *str)
#endif

inline int hal_register_isr(int vector, hal_isr_t isr, hal_isr_t *old)
{
    register_isr(vector, isr, old);
    return HAL_SUCCESS;
}

inline void hal_system_error(int err)
{
    KASSERT(0);
    while (1)
        ;
}

inline unsigned int hal_global_int_ctl(int int_op)
{
    int ret = GET_SYS_PSW() & PSW_mskGIE;

    if (int_op == HAL_DISABLE_INTERRUPTS) {

        __asm__("setgie.d");
        __asm__("isb");
    }
    else{
        __asm__("setgie.e");
    }

    return ret ? !HAL_DISABLE_INTERRUPTS : HAL_DISABLE_INTERRUPTS;
}

inline void *hal_current(void)
{
    return HAL_NULL;
}

inline unsigned int hal_ms2ticks(unsigned int timeout)
{
        return timeout = (timeout * configTICK_RATE_HZ)/1000;
}

inline int hal_sleep(int ms)
{
    vTaskDelay(hal_ms2ticks(ms));
    return HAL_SUCCESS;
}

inline void *hal_malloc(int size)
{
    return malloc((size_t)size);
}

inline void hal_free(void *ptr)
{
    free(ptr);
}

inline void* hal_get_pid_current()
{
    return xTaskGetCurrentTaskHandle();
}
inline int hal_create_semaphore(hal_semaphore_t *sem, int num, const void *param)
{
    unsigned int max;
    if((unsigned int)param == 0)
        max = 65535;
    else
        max = (unsigned int)param;
    /*
     * max : the maximun count value that can be reached. 
     *   When the semaphore reaches this value it can no longer be given 
     * num : the count value assigend to the semaphore when it is created  */   
    xSemaphoreHandle obj = xSemaphoreCreateCounting( max, num );
    if (obj) {
#if configQUEUE_REGISTRY_SIZE > 0
        vQueueAddToRegistry( (xQueueHandle)obj, (const char *)"queue");
#endif
        sem->obj = obj;
        return HAL_SUCCESS;
    }
    DEBUG(1,1,"semaphore create failed\n");
    return HAL_FAILURE;
}

inline int hal_destroy_semaphore(hal_semaphore_t *sem)
{

    if(sem->obj == NULL)
        return HAL_FAILURE;
    vQueueDelete(sem->obj);
    return HAL_SUCCESS;

}

inline int hal_pend_semaphore(hal_semaphore_t *sem, unsigned int timeout)
{
    if(timeout == HAL_SUSPEND)
        timeout = portMAX_DELAY;
    else
        timeout = hal_ms2ticks(timeout);
    if (xSemaphoreTake( sem->obj, ( portTickType ) timeout ) == pdTRUE)
    {
        DEBUG(0,1,"take semaphore success:%x\n", sem);
        return HAL_SUCCESS;
    }
    else
    {
        DEBUG(1,1,"take semaphore fail:%x, in 0x%x\n", sem, (unsigned long)__builtin_return_address(0));
        return HAL_FAILURE;
    }
}

inline int hal_post_semaphore(hal_semaphore_t *sem)
{
    if (xSemaphoreGive( sem->obj ) == pdTRUE)
    {   
        DEBUG(0,1,"give semaphore success:%x\n", sem);
        return HAL_SUCCESS;
    }
    else
    {   
        DEBUG(1,1,"give semaphore fail:%x, in 0x%x\n", sem, (unsigned long)__builtin_return_address(0));
        return HAL_FAILURE;
    }
}

/*
 *
 * @param pxHigherPriorityTaskWoken xSemaphoreGiveFromISR() will set
 * *pxHigherPriorityTaskWoken to pdTRUE if giving the semaphore caused a task
 * to unblock, and the unblocked task has a priority higher than the currently
 * running task.  If xSemaphoreGiveFromISR() sets this value to pdTRUE then
 * a context switch should be requested before the interrupt is exited.
 */
inline int hal_post_semaphore_in_isr(hal_semaphore_t *sem)
{
    signed portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    if (xSemaphoreGiveFromISR( sem->obj, &xHigherPriorityTaskWoken ) == pdTRUE)
    {   
        DEBUG(0,1,"give semaphore success:%x\n", sem);
        //return HAL_SUCCESS;
    }
    else
    {   
        DEBUG(1,1,"give semaphore fail:%x, in 0x%x\n", sem, (unsigned long)__builtin_return_address(0));
        return HAL_FAILURE;
    }

    // xHigherPriorityTaskWoken would be TRUE if xSemaphoreGiveFromISR cause a context switch
    if (xHigherPriorityTaskWoken)
    {
        //unsigned int msk = portSET_INTERRUPT_MASK_FROM_ISR();
        //call scheduler    
        //vTaskSwitchContext();
        //portCLEAR_INTERRUPT_MASK_FROM_ISR(msk);
        portYIELD();
    }
    return HAL_SUCCESS;
}

inline int hal_post_queue_from_lisr(hal_queue_t *p_queue)
{
        hal_queue_t *queue = p_queue;
        signed  portBASE_TYPE xHigherPriorityTaskWoken;
        xHigherPriorityTaskWoken = pdFALSE;

        if (xQueueSendFromISR(queue->event, &queue->msg, &xHigherPriorityTaskWoken) != pdTRUE)
        {
                DEBUG(1,1,"Send msg to queue fail:%x, in 0x%x\n", queue, (unsigned long)__builtin_return_address(0));
                return HAL_FAILURE;
        }

        if (xHigherPriorityTaskWoken)
        {
    #if 0
        unsigned int msk = portSET_INTERRUPT_MASK_FROM_ISR();
                //call scheduler
                vTaskSwitchContext();
        portCLEAR_INTERRUPT_MASK_FROM_ISR(msk);
        #endif
        portYIELD();
    }
        return HAL_SUCCESS;
}

inline int hal_create_mutex(hal_mutex_t *mutex, const void *param)
{
    return hal_create_semaphore((hal_semaphore_t *)mutex, 1, (const void *)1);
}
inline int hal_destroy_mutex(hal_mutex_t *mutex)
{
    return hal_destroy_semaphore((hal_semaphore_t *)mutex);
}
inline int hal_wait_for_mutex(hal_mutex_t *mutex, unsigned int timeout)
{
    return hal_pend_semaphore((hal_semaphore_t *)mutex, HAL_SUSPEND);
}
inline int hal_release_mutex(hal_mutex_t *mutex)
{
    return hal_post_semaphore((hal_semaphore_t *)mutex);
}

inline int hal_create_queue(hal_queue_t *queue)
{
        xQueueHandle obj = xQueueCreate( queue->size, sizeof( void * ) );
        if (obj) {
                queue->event = (void *)obj;
                return HAL_SUCCESS;
        }

        return HAL_FAILURE;
}

inline int hal_destroy_queue(hal_queue_t *queue)
{
        if( queue == NULL )
                return HAL_FAILURE;
        vQueueDelete( queue->event );
        return HAL_SUCCESS;
}
inline int hal_pend_queue(hal_queue_t *queue)
{
        unsigned int timeout = queue->timeout;

        if(timeout == HAL_SUSPEND)
                timeout = portMAX_DELAY;
        else
                timeout = hal_ms2ticks(timeout);

        if ( xQueueReceive( queue->event, &queue->msg, hal_ms2ticks(queue->timeout) ) == pdTRUE )
                return HAL_SUCCESS;
        else
                return HAL_FAILURE;
}

inline int hal_post_queue(hal_queue_t *queue)
{
        if ( xQueueSend(queue->obj, &queue->msg, queue->timeout ) == pdTRUE)
                return HAL_SUCCESS;
        else
                return HAL_FAILURE;
}

#if 0
inline int hal_mbox_create(hal_mbox_t *mbox, const void *param)
{
    OS_EVENT *obj = OSMboxCreate((void *)0);

    KASSERT(obj);

    if (obj){

        mbox->obj = obj;
        return HAL_SUCCESS;
    }

    return HAL_FAILURE;
}

inline int hal_mbox_destroy(hal_mbox_t *mbox)
{
    INT8U err;

    OSMboxDel(mbox->obj, OS_DEL_ALWAYS, &err);

    if (err == OS_ERR_NONE)
        return HAL_SUCCESS;

    hal_show_err(err, __func__);

    return HAL_FAILURE;
}

inline int hal_mbox_pend(hal_mbox_t *mbox, void **msg, unsigned int timeout)
{
    INT8U err;
    INT16U ucos_timeout;

    ucos_timeout = hal_ms2ticks(timeout);

    *msg = OSMboxPend(mbox->obj, ucos_timeout, &err);
    return *msg ? HAL_SUCCESS : HAL_FAILURE;
}

inline int hal_mbox_try_pend(hal_mbox_t *mbox, void **msg)
{
    *msg = OSMboxAccept(mbox->obj);

    return *msg ? HAL_SUCCESS : HAL_FAILURE;
}

inline int hal_mbox_post(hal_mbox_t *mbox, void *msg)
{
    INT8U err;

    err = OSMboxPost(mbox->obj, msg);

    if (err == OS_ERR_NONE)
        return HAL_SUCCESS;

    hal_show_err(err, __func__);

    return HAL_FAILURE;
}

inline int hal_validate_mbox(hal_mbox_t *mbox_obj)
{
    return HAL_TRUE;
}
#endif
inline int hal_create_thread(hal_thread_t *th)
{
    unsigned int low_stack = 0;
    if (th->ptos != NULL) {
        low_stack = (unsigned int)th->ptos - th->stack_size;
        DEBUG(0,0,"%s:low_stk=%08x,ptos=%08x,stk_sz=%08x\n",
        th->name, low_stack, th->ptos, th->stack_size);
    }
    /* 
     * th->stack_size in byte for th->stack_size
     * th->stack_size in word for xTaskGenericCreate 
     *
     * pxNewTCB->pxStack is a unsigned long pointer
     * pxTopOfStack = pxNewTCB->pxStack + ( usStackDepth - 1 );
     * */
    if (xTaskGenericCreate( th->fn, ( const char * ) th->name, (th->stack_size >> 2), th->arg, th->prio, th->task, (portSTACK_TYPE *)low_stack, NULL ) == pdTRUE)
        return HAL_SUCCESS;
    else
        return HAL_FAILURE;
}

inline int hal_destroy_thread(hal_thread_t *th)
{
    if(th != NULL)
        vTaskDelete(th->task);
    else 
        vTaskDelete(NULL);
    return HAL_SUCCESS;
}

inline int hal_create_bh(hal_bh_t *bh)
{

    if(hal_create_semaphore(&bh->sem, 0, (void *)0) == HAL_FAILURE)
        return HAL_FAILURE;
    return hal_create_thread(&bh->th);
}

inline int hal_destroy_bh(hal_bh_t *bh)
{
    if(bh != NULL) {
        vTaskDelete(bh->th.task);
        return HAL_SUCCESS;
    } else
        return HAL_FAILURE;
}

inline int hal_register_bh(hal_bh_t *bh, void *param)
{
    return HAL_SUCCESS;
}

inline int hal_raise_bh(hal_bh_t *bh)
{
    return hal_post_semaphore_in_isr(&bh->sem);
}

inline void hal_init_os()
{   
    return;
}

inline void hal_start_os()
{   
    hal_console_end_fatal();
    vTaskStartScheduler();
}

#include <serial/serial.h>
inline void hal_console_init (const sSerialDrv *ser_drv)
{
    xSerialPortInitDrv(ser_drv);
    xSerialPortInitMinimal(ser115200, 256);
}

inline void hal_console_puts (const signed char * s, unsigned int l)
{
    vSerialPutString(NULL, s, l);
}

inline int hal_console_getc (signed char *pcRxedChar)
{
    return xSerialGetChar(NULL, pcRxedChar, 1);
}

inline int hal_console_putc (signed char cOutChar)
{
    return xSerialPutChar(NULL, cOutChar, 1);
}

inline void hal_console_begin_fatal (void)
{
    gOsInFatal = 1;
}

inline void hal_console_end_fatal (void)
{
    gOsInFatal = 0;
}
