#include <stdlib.h>

#include "dana_sem.h"
#include "dana_mem.h"
#include "osi.h"

//dana库内部不需要知道采用那种信号量， 本测试例子采用有名信号量， 厂商根据例子和平台特性自行实现
struct dana_sem_handler {
    OsiSyncObj_t pSyncObj;
};

/*
 * 创建一个信号量(库内部不管是有名还是无名)
 */
dana_sem_handler_t * dana_sem_create()
{
    dana_sem_handler_t *dana_sem_handler = (dana_sem_handler_t *)dana_calloc(1, sizeof(dana_sem_handler_t));
    if (NULL == dana_sem_handler) {
        return NULL; 
    }

    if (0 != osi_SyncObjCreate(&(dana_sem_handler->pSyncObj))) {
        dana_free(dana_sem_handler); 
        return NULL;
    }

    return dana_sem_handler;
}


/*
 * 等待信号量触发
 */
int32_t dana_sem_wait(dana_sem_handler_t *sem)
{
    return osi_SyncObjWait(&(sem->pSyncObj), OSI_WAIT_FOREVER);

}

/*
 * 触发信号量
 */
int32_t dana_sem_post(dana_sem_handler_t *sem)
{
    return osi_SyncObjSignal(&(sem->pSyncObj));
}

/*
 * 销毁信号量
 */
int32_t dana_sem_destroy(dana_sem_handler_t *sem)
{
    osi_SyncObjDelete(&(sem->pSyncObj));
    dana_free(sem);
    return 0;
}
