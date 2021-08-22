/**
 * @file drv_module.c
 * @brief provide functions of os related function
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010-07-24
 * @version 1.0
 */
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_module.h"

#ifdef AKOS
#include "akos_api.h"
#endif

//max num of driver module
#define MAX_DRV_MODULE 32
#define MAX_DRV_MODULE_NAME 8   //max 7 char

//task size
#define DRVMODULE_TASK_STACK_SIZE (10*1024)
#define DRVMODULE_HISR_STACK_SIZE (1*1024)

//message queue size
#define DRVMODULE_ADDRESS_LEN 400

//message param size
#define MESSAGE_PARAM_SIZE 7

#define UNDEFINE_PRIORITY 10

//meeage map struct
typedef struct tagT_MESSAGE_MAP
{
    unsigned long msg;
    unsigned long param[MESSAGE_PARAM_SIZE];
    T_fDRV_CALLBACK fCbk;

#ifdef AKOS
    T_hHisr hHisr;
    unsigned long *stack;
#endif

    bool flag;
    struct tagT_MESSAGE_MAP *next;
}
T_MESSAGE_MAP;

//event group struct
typedef struct
{
#ifdef AKOS
    T_hEventGroup hGroup;
    
    T_hHisr hHisr;
    unsigned long *stack;
#endif

    unsigned long events;
}
T_EVENT_GROUP;

//driver module struct
typedef struct
{
#ifdef AKOS
    T_hTask hTask;
    T_hQueue hQueue;
    
    unsigned long *stack;
    unsigned long *queue_address;

    T_hSemaphore hSem;
#endif

    T_EVENT_GROUP eGroup;
    T_MESSAGE_MAP *msgmap_head;
}
T_DRIVER_MODULE;

/*事件标识定义*/
typedef unsigned long  T_SYS_EVTID;

/*消息机构体定义*/
typedef struct
{
    /*为了兼MMI，下面两项跟以前是一致的，需要赋值*/
    T_SYS_EVTID event;   /*事件标识*/
    unsigned long param[MESSAGE_PARAM_SIZE];/*事件参数*/

}T_SYS_MAILBOX;
/////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


static void DrvModule_Free(E_DRV_MODULE module);

//global variable for driver module
volatile T_DRIVER_MODULE m_drv_module[DRV_MODULE_UNDIFINE] = {0};

#ifdef AKOS

typedef struct
{
    E_DRV_MODULE    		id;
    unsigned char           priority;   
    unsigned long           stack_size;
    unsigned char           name[MAX_DRV_MODULE_NAME];
} T_DRV_MODULE_PROPERTY;


static unsigned char get_task_priority(unsigned long module);
static unsigned long get_task_stack_size(unsigned long module);
static T_hSemaphore *DrvModule_Get_Sem(E_DRV_MODULE module);
static void HISR_DrvModule(void);
static void HISR_SetEvent(void);
static void DrvModule_Task(unsigned long argc, void *argv);

//driver module priority
static const T_DRV_MODULE_PROPERTY drv_module_property[] =
{
    {DRV_MODULE_UART0,               0,      DRVMODULE_TASK_STACK_SIZE, "D_UART0"},     //uart has the highest priority
    {DRV_MODULE_UART1,               0,      DRVMODULE_TASK_STACK_SIZE, "D_UART1"},     //uart has the highest priority
    //all the usb module has the second priority
    {DRV_MODULE_UVC,                1,      DRVMODULE_TASK_STACK_SIZE, "D_UVC"},
    {DRV_MODULE_USB_DISK,           1,      DRVMODULE_TASK_STACK_SIZE, "D_UDISK"},
    {DRV_MODULE_USB_CDC,            1,      DRVMODULE_TASK_STACK_SIZE, "D_CDC"},
    {DRV_MODULE_USB_CAMERA,         1,      DRVMODULE_TASK_STACK_SIZE, "D_UCAM"},
    {DRV_MODULE_USB_ANYKA,          1,      DRVMODULE_TASK_STACK_SIZE, "D_UAK"},
    {DRV_MODULE_USB_CMMB,           1,      DRVMODULE_TASK_STACK_SIZE, "D_UCMMB"},
    {DRV_MODULE_USB_BUS,            1,      (40*1024),                 "D_UBUS"},
    {DRV_MODULE_SPI_CTRL1,          1,      (10*1024),                  "D_SPI1"},



    {DRV_MODULE_KEYPAD,             2,      (4*1024),                 "D_KPT"},
	{DRV_MODULE_CODEC,				2,		DRVMODULE_TASK_STACK_SIZE, "D_VENC"},	

    {DRV_MODULE_VTIMER,             3,      DRVMODULE_TASK_STACK_SIZE, "D_VTIMR"},
    {DRV_MODULE_GPIO,               3,      DRVMODULE_TASK_STACK_SIZE, "D_GPIO"}, 
    {DRV_MODULE_SDIO,               1,      DRVMODULE_TASK_STACK_SIZE, "D_SDIO"}, 

    {DRV_MODULE_AD,                 4,      DRVMODULE_TASK_STACK_SIZE, "D_AD"},
    {DRV_MODULE_DA,                 4,      DRVMODULE_TASK_STACK_SIZE, "D_DA"},
    {DRV_MODULE_I2S_TX,             4,      DRVMODULE_TASK_STACK_SIZE, "D_I2ST"},
    {DRV_MODULE_I2S_RX,             4,      DRVMODULE_TASK_STACK_SIZE, "D_I2SR"},
    {DRV_MODULE_CAMERA,             0,      DRVMODULE_TASK_STACK_SIZE, "D_CAM"},
    {DRV_MODULE_DETECT,             4,      DRVMODULE_TASK_STACK_SIZE, "D_DET"},
    {DRV_MODULE_RTC,                4,      DRVMODULE_TASK_STACK_SIZE, "D_RTC"},
    
    {DRV_MODULE_UNDIFINE,           UNDEFINE_PRIORITY,  DRVMODULE_TASK_STACK_SIZE, ""}
};

/**
* @brief get priority for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the module
* @return unsigned char the priority
*/
unsigned char get_task_priority(unsigned long module)
{
    unsigned long i;
    unsigned char priority = UNDEFINE_PRIORITY;

    //check param
    if(module >= DRV_MODULE_UNDIFINE)
    {
        akprintf(C2, M_DRVSYS, "sysctrl_get_task_priority error\r\n");
        return UNDEFINE_PRIORITY;
    }

    //loop to find the proper priority
    for(i = 0; i < sizeof(drv_module_property) / sizeof(T_DRV_MODULE_PROPERTY); i++)
    {
        if(module == drv_module_property[i].id)
        {
            priority = drv_module_property[i].priority;
            break;
        }
    }

    return priority;
}

/**
* @brief get stack size for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the module
* @return unsigned char the stack size
*/
unsigned long get_task_stack_size(unsigned long module)
{
    unsigned long i;
    unsigned long stack_size = DRVMODULE_TASK_STACK_SIZE;

    //check param
    if(module >= DRV_MODULE_UNDIFINE)
    {
        akprintf(C2, M_DRVSYS, "sysctrl_get_task_priority error\r\n");
        return DRVMODULE_TASK_STACK_SIZE;
    }

    //loop to find the proper priority
    for(i = 0; i < sizeof(drv_module_property) / sizeof(T_DRV_MODULE_PROPERTY); i++)
    {
        if(module == drv_module_property[i].id)
        {
            stack_size = drv_module_property[i].stack_size;
            break;
        }
    }

    return stack_size;
}

/**
* @brief get stack name for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the module
* @return unsigned char* the task name
*/
unsigned char* get_task_name(unsigned long module)
{
    unsigned long i;
    unsigned char* name=NULL;

    //check param
    if(module >= DRV_MODULE_UNDIFINE)
    {
        akprintf(C2, M_DRVSYS, "get_task_name error\r\n");
        return NULL;
    }

    //loop to find the proper priority
    for(i = 0; i < sizeof(drv_module_property) / sizeof(T_DRV_MODULE_PROPERTY); i++)
    {
        if(module == drv_module_property[i].id)
        {
            name = (unsigned char*)drv_module_property[i].name;
            break;
        }
    }

    return name;
}

/**
* @brief driver module task
* @author liao_zhijun
* @date 2010-06-18
* @param argc [in]: num of arguments
* @param argv [in]: all the argument
* @return void
*/
static void DrvModule_Task(unsigned long argc, void *argv)
{
    unsigned long status;
    unsigned long i, module;
    T_hQueue hQueue = (T_hQueue)argv;
    unsigned long recvmsg, actual_size;
    T_SYS_MAILBOX msg;
    T_MESSAGE_MAP *pNext = NULL;
    
    module = 0xFFFFFFFF;

    //loop to find which module this task is run for
    for(i = 0; i < MAX_DRV_MODULE; i++)
    {
        if(hQueue == m_drv_module[i].hQueue)
        {
            module = i;
            break;
        }
    }

    //no module found
    if(module == 0xFFFFFFFF)
    {
        akprintf(C2, M_DRVSYS, "Error Task \r\n");
        return;
    }

    
    //akprintf(C2, M_DRVSYS, "Enter Task Module: %d\r\n", module);

    while( 1 )
    {
        //receive message
        status = AK_Receive_From_Queue(hQueue, &msg, sizeof(T_SYS_MAILBOX), &actual_size, AK_SUSPEND);
        if(status != AK_SUCCESS)
        {
            akprintf(C2, M_DRVSYS, "@@@Erro receive msg status=%d in module %d\r\n", status, module);
            continue;
        }

        //exit when receive terminate massage
        if(msg.event == DRVMODULE_EVENT_TERMINATE)
            break;

        
        //...do handle here
        
        pNext = m_drv_module[i].msgmap_head;
        while(pNext != NULL)
        {
            if((pNext->msg == msg.event) && (pNext->fCbk != NULL))
            {
                //call the callback function
                pNext->fCbk(msg.param, MESSAGE_PARAM_SIZE);
                break;
            }

            pNext = pNext->next;
        }
    }

    //set task finish event 
    status = AK_Set_Events(m_drv_module[module].eGroup.hGroup, DRVMODULE_EVENT_TASK_FINISHED, AK_OR);
    //if(status != AK_SUCCESS)
    //{
    //    akprintf(C3, M_DRVSYS, "AK_Set_Events task finish fail %d, module=%d\n", status, module);
    //}
}

/**
* @brief hisr for message
* @author liao_zhijun
* @date 2010-06-18
* @return void
*/
static void HISR_DrvModule(void)
{
    unsigned long status;
    unsigned long iAddr, sAddr;
    T_SYS_MAILBOX message;
    T_hQueue hQueue = AK_INVALID_QUEUE;
    unsigned long i;
    T_MESSAGE_MAP *pNext;
    
    iAddr = (unsigned long)&status;

    //loop to check which hisr now in
    for(i = 0; i < MAX_DRV_MODULE; i++)
    {
        pNext = m_drv_module[i].msgmap_head;
        while(pNext != NULL)
        {
            sAddr = (unsigned long)(pNext->stack);
            if((iAddr > sAddr) && (iAddr < (sAddr + DRVMODULE_HISR_STACK_SIZE)))
            {
                hQueue = m_drv_module[i].hQueue;
                
                message.event= pNext->msg;

                //save param
                memcpy((unsigned char *)(message.param), pNext->param, MESSAGE_PARAM_SIZE*4);
                
                goto HISR_FOUND;
            }

            pNext = pNext->next;
        }
    }

    return;

HISR_FOUND:
    //send message to task
    status = AK_Send_To_Queue(hQueue, &message,
                sizeof(T_SYS_MAILBOX), AK_NO_SUSPEND);

}

/**
* @brief hisr for event
* @author liao_zhijun
* @date 2010-06-18
* @return void
*/
static void HISR_SetEvent(void)
{
    unsigned long status;
    unsigned long iAddr, sAddr;
    unsigned long i;
    
    iAddr = (unsigned long)&status;

    //loop to check which hisr now in
    for(i = 0; i < MAX_DRV_MODULE; i++)
    {
        sAddr = (unsigned long)(m_drv_module[i].eGroup.stack);
        if((iAddr > sAddr) && (iAddr < (sAddr + DRVMODULE_HISR_STACK_SIZE)))
        {
            //set event
            status = AK_Set_Events(m_drv_module[i].eGroup.hGroup, m_drv_module[i].eGroup.events, AK_OR);
            if(status != AK_SUCCESS)
            {
                akprintf(C1, M_DRVSYS, "Error AK_Set_Events %d, module: %d\r\n", status, i);
            }

            m_drv_module[i].eGroup.events = 0;
            return;
        }
    }

}

#endif

/**
* @brief init driver module
* @author liao_zhijun
* @date 2010-06-18
* @return bool
* @retval true init success
* @retval false init fail
*/
bool DrvModule_Init()
{
    unsigned long i;

    for(i = 0; i < DRV_MODULE_UNDIFINE; i++)
    {
#ifdef AKOS
        m_drv_module[i].hTask = AK_INVALID_TASK;
        m_drv_module[i].hQueue = AK_INVALID_QUEUE;

        m_drv_module[i].stack = NULL;
        m_drv_module[i].queue_address = NULL;

        m_drv_module[i].hSem = AK_INVALID_SEMAPHORE;

        m_drv_module[i].eGroup.stack = NULL;
        m_drv_module[i].eGroup.hGroup = AK_INVALID_GROUP;
        m_drv_module[i].eGroup.hHisr = AK_INVALID_HISR;

#endif
        m_drv_module[i].eGroup.events = 0;

        m_drv_module[i].msgmap_head = NULL;
    }

    return true;
}

/**
* @brief create task for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return bool
* @retval true create task success
* @retval false fail to create task
*/
bool DrvModule_Create_Task(E_DRV_MODULE module)
{
#ifdef AKOS

    signed long status;
    unsigned char tmpStr[] = "";
    
    //return when task already create
    if(m_drv_module[module].hTask > 0)
    {
        akprintf(C2, M_DRVSYS, "task for module %d already init\r\n", module);
        return true;
    }

    //alloc memory for stack
    m_drv_module[module].stack = (unsigned long *)drv_malloc(get_task_stack_size(module));
    if(NULL == m_drv_module[module].stack)
    {
        akprintf(C2, M_DRVSYS, "Error alloc memory task stack in module %d\r\n", module);
        return false;
    }

    //alloc memory for queue
    m_drv_module[module].queue_address = (unsigned long *)drv_malloc(DRVMODULE_ADDRESS_LEN * sizeof(unsigned long));
    if(NULL == m_drv_module[module].queue_address)
    {
        drv_free(m_drv_module[module].stack);
        
        akprintf(C2, M_DRVSYS, "Error alloc memory queue in module %d\r\n", module);
        return false;
    }

    //alloc stack for event hisr
    m_drv_module[module].eGroup.stack = (unsigned long *)drv_malloc(DRVMODULE_HISR_STACK_SIZE);
    if(NULL == m_drv_module[module].eGroup.stack)
    {
        drv_free(m_drv_module[module].stack);
        drv_free(m_drv_module[module].queue_address);

        akprintf(C3, M_DRVSYS, "Error Alloc HISR stack, module: %d\r\n", module);
        return false;
    }

    //init os variables
    m_drv_module[module].eGroup.hGroup = AK_INVALID_GROUP;
    m_drv_module[module].eGroup.hHisr= AK_INVALID_HISR;
    m_drv_module[module].hTask = AK_INVALID_TASK;
    m_drv_module[module].hQueue = AK_INVALID_QUEUE;

    //create event group
    m_drv_module[module].eGroup.hGroup = AK_Create_Event_Group();
    if (AK_IS_INVALIDHANDLE( m_drv_module[module].eGroup.hGroup))
    {
        akprintf(C2, M_DRVSYS, "AK_Create_Event_Group Error:%x, Module: %d\r\n", m_drv_module[module].eGroup.hGroup, module);
        goto ERROR_CREATE_TASK;
    }
    
    //create hisr for events
    m_drv_module[module].eGroup.hHisr = AK_Create_HISR(HISR_SetEvent, 
                            tmpStr, 
                            2, 
                            (void *)m_drv_module[module].eGroup.stack, 
                            DRVMODULE_HISR_STACK_SIZE);

    if (AK_IS_INVALIDHANDLE(m_drv_module[module].eGroup.hHisr))
    {
        akprintf(C3, M_DRVSYS, "Error Create Event HISR: %d, module: %d\r\n", m_drv_module[module].eGroup.hHisr, module);
        goto ERROR_CREATE_TASK;
    }    


    //create queue
    m_drv_module[module].hQueue = AK_Create_Queue((void *)m_drv_module[module].queue_address,
                            DRVMODULE_ADDRESS_LEN,
                            AK_FIXED_SIZE,
                            sizeof(T_SYS_MAILBOX),
                            AK_FIFO);
    if(AK_IS_INVALIDHANDLE(m_drv_module[module].hQueue))
    {
        akprintf(C3, M_DRVSYS, "AK_Create_Queue Error %x, Module: %d\r\n", 
            m_drv_module[module].hQueue, module);
        goto ERROR_CREATE_TASK;
    }

    //create task
	if (get_task_name(module) == NULL)
	{
        akprintf(C3, M_DRVSYS, "AK_Create_Task error, module%d not has a name yet!!\r\n", 
             module);
        goto ERROR_CREATE_TASK;

	}
	
    m_drv_module[module].hTask = AK_Create_Task((void *)DrvModule_Task, 
                            get_task_name(module),
                            1, 
                            (void *)m_drv_module[module].hQueue, 
                            (void *)m_drv_module[module].stack, 
                            get_task_stack_size(module), 
                            get_task_priority(module), 
                            2, 
                            AK_PREEMPT, 
                            AK_START);
    if(AK_IS_INVALIDHANDLE(m_drv_module[module].hTask))
    {
        akprintf(C3, M_DRVSYS, "AK_Create_Task error %x, module = %d\r\n", 
            m_drv_module[module].hTask, module);
        goto ERROR_CREATE_TASK;
    }

    return true;
    
ERROR_CREATE_TASK:
    DrvModule_Free(module);
    return false;
#else

    //do nothing if no AKOS defined
    return true;
#endif
}

/**
* @brief terminate task for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return void
*/
void DrvModule_Terminate_Task(E_DRV_MODULE module)
{
#ifdef AKOS
    signed long status;
    T_SYS_MAILBOX message;
    unsigned long tmp_event;

    message.event= DRVMODULE_EVENT_TERMINATE;

    if(AK_IS_INVALIDHANDLE(m_drv_module[module].hQueue))  //task no exist
    {
        return ;
    }
    //send terminate message
    status = AK_Send_To_Queue(m_drv_module[module].hQueue, (void *)&message, sizeof(T_SYS_MAILBOX), AK_SUSPEND);
    if(status != AK_SUCCESS)
    {
        akprintf(C1, M_DRVSYS, "+++++++++++send msg error %x, module %d ++++++++++++\r\n", status, module);
    }

    
    akprintf(C3, M_DRVSYS, "wait for module %d task finished\n", module);

    //waiting for task finish
    status = AK_Retrieve_Events(m_drv_module[module].eGroup.hGroup, 
               DRVMODULE_EVENT_TASK_FINISHED, 
               AK_OR_CONSUME,
               &tmp_event,
               AK_SUSPEND);
    if(status != AK_SUCCESS)
    {
        akprintf(C1, M_DRVSYS, "AK_Retrieve_Events fail %x, module %d\r\n", status, module);
    }
#endif

    //release all resource
    DrvModule_Free(module);

}

/**
* @brief release all resource for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return void
*/
static void DrvModule_Free(E_DRV_MODULE module)
{
    signed long status;
    T_MESSAGE_MAP *pNext, *pTmp;
    
#ifdef AKOS
    //free memory
    drv_free(m_drv_module[module].stack);
    m_drv_module[module].stack = NULL;
    
    drv_free(m_drv_module[module].queue_address);
    m_drv_module[module].queue_address= NULL;

    drv_free(m_drv_module[module].eGroup.stack);
    m_drv_module[module].eGroup.stack= NULL;

    //delete group
    if(AK_IS_VALIDHANDLE(m_drv_module[module].eGroup.hGroup))
    {
        status = AK_Delete_Event_Group(m_drv_module[module].eGroup.hGroup);
        if(status != AK_SUCCESS)
        {
            akprintf(C1, M_DRVSYS, "Error delete group, status = %d, module: %d\r\n", status, module);
        }
        
        m_drv_module[module].eGroup.hGroup = AK_INVALID_GROUP;
    }

    //delete event hisr
    if(AK_IS_VALIDHANDLE(m_drv_module[module].eGroup.hHisr))
    {
        status = AK_Delete_HISR(m_drv_module[module].eGroup.hHisr);
        if(status != AK_SUCCESS)
        {
            akprintf(C1, M_DRVSYS, "Error delete hisr, status = %d, module: %d\r\n", status, module);
        }

        m_drv_module[module].eGroup.hHisr = AK_INVALID_GROUP;
    }
    
    //delete queue
    if(AK_IS_VALIDHANDLE(m_drv_module[module].hQueue))
    {
        status = AK_Delete_Queue(m_drv_module[module].hQueue);
        if(status != AK_SUCCESS)
        {
            akprintf(C1, M_DRVSYS, "Error delete queue, status = %d, module: %d\r\n", status, module);
        }

        m_drv_module[module].hQueue = AK_INVALID_QUEUE;
    }

    //delete task
    if(AK_IS_VALIDHANDLE(m_drv_module[module].hTask))
    {
        status = AK_Delete_Task(m_drv_module[module].hTask);
        if(status != AK_SUCCESS)
        {
            
            akprintf(C1, M_DRVSYS, "Error delete task, status = %d, module: %d\r\n", status, module);
        }

        m_drv_module[module].hTask = AK_INVALID_TASK;
    }
#endif

    //free all hisr
    pNext = m_drv_module[module].msgmap_head;
    while(pNext != NULL)
    {
#ifdef AKOS
        drv_free(pNext->stack);
        pNext->stack = NULL;

        //delete hisr
        status = AK_Delete_HISR(pNext->hHisr);
        if(status != AK_SUCCESS)
        {
            akprintf(C1, M_DRVSYS, "Error delete hisr, status = %d, module: %d\r\n", status, module);
        }
#endif
        pTmp = pNext;
        pNext = pNext->next;

        drv_free(pTmp);    
    }

    m_drv_module[module].msgmap_head = NULL;
}

/**
* @brief map callback function for message
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the giving message
* @param callback [in]:  callback function to be mapped
* @return bool
* @retval true map message success
* @retval false fail to map message 
*/
bool DrvModule_Map_Message(E_DRV_MODULE module, unsigned long msg, T_fDRV_CALLBACK callback)
{
    T_MESSAGE_MAP *pMsgMap = NULL;
    T_MESSAGE_MAP *pNext = NULL;
    unsigned char strTmp[] = "";

    //check if message already exit or not
    pNext = m_drv_module[module].msgmap_head;
    while(pNext != NULL)
    {
        if(pNext->msg == msg)
        {
            pNext->fCbk = callback;
            return true;
        }

        pNext = pNext->next;
    }

    //alloc memory
    pMsgMap = (T_MESSAGE_MAP *)drv_malloc(sizeof(T_MESSAGE_MAP));
    if(NULL == pMsgMap)
    {
        akprintf(C1, M_DRVSYS, "Error Alloc T_MESSAGE_MAP, module: %d\r\n", module);
        return false;
    }

    //save msg and callback
    pMsgMap->msg = msg;
    pMsgMap->fCbk = callback;
    pMsgMap->flag = false;
    pMsgMap->next = NULL;

#ifdef AKOS
    //alloc memory for hisr stack
    pMsgMap->stack = (unsigned long *)drv_malloc(DRVMODULE_HISR_STACK_SIZE);
    if(NULL == pMsgMap->stack)
    {
        drv_free(pMsgMap);

        akprintf(C1, M_DRVSYS, "Error Alloc HISR stack, module: %d\r\n", module);
        return false;
    }
    
    //create hisr
    pMsgMap->hHisr = AK_Create_HISR(HISR_DrvModule, 
                            strTmp, 
                            2, 
                            (void *)pMsgMap->stack, 
                            DRVMODULE_HISR_STACK_SIZE);

    if (AK_IS_INVALIDHANDLE(pMsgMap->hHisr))
    {
        drv_free(pMsgMap->stack);
        drv_free(pMsgMap);

        akprintf(C1, M_DRVSYS, "Error Create HISR: %d, module: %d\r\n", pMsgMap->hHisr, module);
        return false;
    }
#endif

    //add to map list
    if(NULL == m_drv_module[module].msgmap_head)
    {
        m_drv_module[module].msgmap_head = pMsgMap;
    }
    else
    {
        pNext = m_drv_module[module].msgmap_head;
        while(pNext->next != NULL)
        {
            pNext = pNext->next;
        }

        pNext->next = pMsgMap;
    }

    return true;
}

/**
* @brief  send message to giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the message to be send
* @param param [in]:  param for the message, its size is 12 bytes
* @return bool
* @retval true send message success
* @retval false fail to send message 
*/
bool DrvModule_Send_Message(E_DRV_MODULE module, unsigned long msg, unsigned long *param)
{

    signed long status;
    T_MESSAGE_MAP *pNext;
    
    pNext = m_drv_module[module].msgmap_head;

    //loop to find the mapping hisr or callback function
    while(pNext != NULL)
    {
        if(pNext->msg == msg)
        {
            //save param
            if(param != NULL)
                memcpy(pNext->param, param, MESSAGE_PARAM_SIZE*4);

            pNext->flag = true;

#ifdef AKOS
            //active hisr for the message
            status = AK_Activate_HISR(pNext->hHisr);
            if (AK_SUCCESS != status)
            {
                akprintf(C1, M_DRVSYS, "Error AK_Activate_HISR %d, module: %d\r\n", status, module);
                return false;
            }
#else
            //if AKOS not used, we call callback function the directly
            if(module != DRV_MODULE_USB_DISK)
            {
                pNext->fCbk(param, MESSAGE_PARAM_SIZE*4);
            }
#endif
            return true;
        }

        pNext = pNext->next;
    }

    return false;
}

/**
* @brief  check if message come or not, if come, call the call back func
*            mainly used in non-os situation
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the message to be send
* @return bool
* @retval true send message success
* @retval false fail to send message 
*/
void DrvModule_Retrieve_Message(E_DRV_MODULE module, unsigned long msg)
{
    T_MESSAGE_MAP *pNext;
    
    pNext = m_drv_module[module].msgmap_head;

    //loop to find the mapping hisr or callback function
    while(pNext != NULL)
    {
        if(pNext->msg == msg)
        {
            if(pNext->flag && pNext->fCbk)
            {
                pNext->fCbk(pNext->param, MESSAGE_PARAM_SIZE*4);

                pNext->flag = false;

                return;
            }
        }

        pNext = pNext->next;
    }
}

/**
* @brief  send event for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param event [in]: the event to be send
* @return bool
* @retval true set event success
* @retval false fail to set event
*/
bool DrvModule_SetEvent(E_DRV_MODULE module, unsigned long event)
{
    signed long status;

#ifdef AKOS
    //save event to be used in hisr
    m_drv_module[module].eGroup.events |= event;

    //active hisr
    status = AK_Activate_HISR(m_drv_module[module].eGroup.hHisr);
    if (AK_SUCCESS != status)
    {
        akprintf(C1, M_DRVSYS, "Error AK_Activate_HISR %d in set event, module: %d\r\n", status, module);
        return false;
    }
#else
    //if akos not defined, just set bits in event
    m_drv_module[module].eGroup.events |= event;
    
#endif

    return true;
}


/**
* @brief  wait for sepcifical event
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @param event [in]: the event to be wait for
* @param timeout [in]: timeout value, if the event still not come after the giving time, a timeout will be returned 
* @return signed long
* @retval DRV_MODULE_SUCCESS wait event success
* @retval DRV_MODULE_TIMEOUT wait event timeout
* @retval DRV_MODULE_ERROR wait event error
*/
signed long DrvModule_WaitEvent(E_DRV_MODULE module, unsigned long event, unsigned long timeout)
{
    signed long status;
    unsigned long retrieved_events;
    unsigned long tick;
    bool bTimeout = true;
    
#ifdef AKOS
    //retrieve events
    status = AK_Retrieve_Events(m_drv_module[module].eGroup.hGroup, 
                                       event, 
                                       AK_OR_CONSUME,
                                       &retrieved_events,
                                       timeout);
    //check result, sucess, timeout or error?
    if(AK_SUCCESS == status)
    {
        return DRV_MODULE_SUCCESS;
    }
    else if(AK_TIMEOUT == status)
    {
        return DRV_MODULE_TIMEOUT;
    }
    else
    {
        return DRV_MODULE_ERROR;
    }
#else
    //if no akos defined, we use get_tick_count to check if timeout or not 
    tick = 0;//get_tick_count();
    do
    {
        //check event bits in event group
        if(m_drv_module[module].eGroup.events & event)
        {            
            bTimeout = false;
            break;
        }
    }
    while(tick++ < (timeout<<12));

    //clear events bits
    m_drv_module[module].eGroup.events &= ~(event);

    if(bTimeout)
    {
        return DRV_MODULE_TIMEOUT;
    }
    else
    {
        return DRV_MODULE_SUCCESS;
    }
#endif

}


/**
* @brief  start protection for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return void
*/
void DrvModule_Protect(E_DRV_MODULE module)
{
    unsigned long new_module;

    new_module = sysctl_get_share_module(module);
    if(0xFFFFFFFF == new_module)
    {
        akprintf(C1, M_DRVSYS, "DrvModule_Protect():module name error!\n");
        return;
    }

#ifdef AKOS
{
    T_hSemaphore *pSem = NULL;

    //get sem
    pSem = (T_hSemaphore *)&m_drv_module[new_module].hSem;
    if(AK_INVALID_SEMAPHORE == *pSem)
    {
        *pSem = AK_Create_Semaphore(1, AK_FIFO);

        if(AK_INVALID_SUSPEND == *pSem)
        {
            *pSem = AK_INVALID_SEMAPHORE;
            akprintf(C1, M_DRVSYS, "DrvModule_Protect():create smph failed!\n");
            return;
        }
    }

    //obtain sem
    AK_Obtain_Semaphore(*pSem, AK_SUSPEND);
}
#endif
}

/**
* @brief  stop protection for giving module
* @author liao_zhijun
* @date 2010-06-18
* @param module [in]: the giving module
* @return void
*/
void DrvModule_UnProtect(E_DRV_MODULE module)
{
    unsigned long new_module;

    new_module = sysctl_get_share_module(module);
    if(0xFFFFFFFF == new_module)
    {
        akprintf(C1, M_DRVSYS, "DrvModule_UnProtect():module name error!\n");
        return;
    }

#ifdef AKOS
{
    T_hSemaphore *pSem = NULL;

    //get sem
    pSem = (T_hSemaphore *)&m_drv_module[new_module].hSem;
    //release sem
    if(*pSem != AK_INVALID_SEMAPHORE)
    {
        AK_Release_Semaphore(*pSem);
    }
    else
    {
        akprintf(C1, M_DRVSYS, "DrvModule_UnProtect():semph error!\n");
    }
}
#endif
}

