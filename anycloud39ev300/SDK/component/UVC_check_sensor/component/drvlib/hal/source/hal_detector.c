/**
 * @file    hal_detector.c
 * @brief   detector module, for detecting device connected or disconnected
 *          by check gpio or voltage of ADC.
 *          The detected event of gpio can be indicated by interrupt of the 
 *          gpio,or by a timer.
 *          The detected event of ADC is indicated by a timer.
 * 支持防抖处理: 
 *     断开会马上响应;
 *     接合会有防抖处理，防抖时间为1秒钟: 当接合时，若1秒内没有出现断开，即为
 *     有效接合。
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.03.09
 * @version 1.0
 */



#include <string.h>
#include "arch_init.h"
#include "anyka_types.h"
#include "hal_print.h"
#include "drv_gpio.h"
#include "drv_module.h"
#include "hal_timer.h"
#include "arch_analog.h"
#include "hal_detector.h"


#define GPIO_PROTECT(gpionum)                  //add the implement
#define GPIO_UNPROTECT(gpionum)                //add the implement 


#define DETECT_MESSAGE        3


#define  DETECT_DEV_MAX       32
#define  DEV_TIMER_MAX        DETECT_DEV_MAX
#define  DEV_INT_QUEUE_LEN    DETECT_DEV_MAX


#define  MAX_INTERVAL_CHECK   50          //ms
#define  SHAKE_CHECK_TIME     1000        //ms
#define  VOLTAGE_CHECK_TIME   100         //ms

#if (SHAKE_CHECK_TIME < VOLTAGE_CHECK_TIME)
#error "must SHAKE_CHECK_TIME >= VOLTAGE_CHECK_TIME"
#endif


/**
 * @brief type of detect object.
 */
typedef enum
{
    eDETECT_OT_GPIO_INT = 0,
    eDETECT_OT_GPIO_CHK,  
    eDETECT_OT_ADC_CHK
}T_eDETECT_OBJ_TYPE;


/**
 * @brief structure for device node.
 */
typedef struct 
{
    const char *               pdevname;     ///< device name
    T_eDETECT_OBJ_TYPE    type;         ///< detecting type
    void                *pInfo;       ///< inof about the device
    T_fDETECTOR_CALLBACK  cb;           ///< call back function of the device
}T_DETECT_DEV_LIST;

/**
 * @brief structure for gpio info.
 */
typedef struct
{
    unsigned long    gpionum;           ///< number of the gpio
    bool   active_level;      ///< active level
    bool   enable;            ///< enable flag
    bool   connect_state;     ///< connected state
    unsigned long    interval_ms;       ///< interval, unit: ms
    unsigned long    shake_count;       ///< avoid shaking time counter
}T_DETECT_GPIO_INFO;                


/**
 * @brief structure for adc info.
 */
typedef struct
{
    const T_VOLTAGE_TABLE  *pvoltage_table;     ///< volatage tabel
    unsigned long                   voltageitem_num;    ///< number of voltage item
    unsigned long                   voltage_departure;  ///< min of voltage departure
    unsigned long                   interval_ms;        ///< interval, unit: ms
    unsigned long                   dev_list_start;     ///< the index of first device
    unsigned long                   enable_bit;         ///< bitmap of enabling
    unsigned long                   connect_state;      ///< bitmap of connected flag
    unsigned long                   ad_value;           ///< the value of ad value read last time
    unsigned long                   ad_chk_count;       ///< steady adc value checking time
    unsigned long                  *pshake_count;       ///< avoid shaking time counters
}T_DETECT_ADC_INFO;                 

/**
 * @brief structure for timer maping to device.
 */
typedef struct
{
    signed long            hTimer;                  ///< timer id
    T_DETECT_DEV_LIST *pdev_list_node;          ///< pointer to device node
}T_DEV_TIMER;                       


/**
 * @brief a queue for each device detected by a gpio interrupted.
 */
typedef struct
{
    unsigned char               dev_list_index[2];
    unsigned char               front;
    unsigned char               rear;
}T_DEV_INT_QUEUE;
#define  QUEUE_INVALID_INDXE    0xFF


static void detector_unregister(void);
static void detector_set2default(void);




static bool            bdetect_module_init = false;    ///< detect module init flag
static T_DETECT_DEV_LIST dev_list[DETECT_DEV_MAX];          ///< device node list
static T_DEV_TIMER       dev_timer[DEV_TIMER_MAX];          ///< timer list
static T_DEV_INT_QUEUE   dev_int_queue[DEV_INT_QUEUE_LEN];  ///< queue list for gpio interrupted



/**
 * @brief       Get the device node by device name
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device.
 * @return      T_DETECT_DEV_LIST
 * @retval      If the return value is not NULL, means that the return 
                value is the device node;
 *              If the return value is NULL, means that the device named
 *              by devname is no exist.
 */ 
static T_DETECT_DEV_LIST * get_dev_list_node_byname(const char * devname)
{
    static T_DETECT_DEV_LIST * plast_access_node = NULL; //cache for run fast
    unsigned long                      i;

    if((NULL == devname) || (0 == devname[0]))
    {
        akprintf(C2, M_DRVSYS, "Device name is empty\n");
        return NULL;
    }
    
    if((NULL != plast_access_node) && 
        (!strcmp(devname, plast_access_node->pdevname)))
    {
        return plast_access_node;
    }

    for(i=0; i<DETECT_DEV_MAX; ++i)
    {
        if(NULL == dev_list[i].pdevname)
        {
            akprintf(C2, M_DRVSYS, "Device %s not exist\n", devname);
            return NULL;
        }

        if(!strcmp(devname, dev_list[i].pdevname))
        {
            plast_access_node = &dev_list[i];
            return plast_access_node;
        }
    }

    akprintf(C2, M_DRVSYS, "Device list is full,and %s not exist.\n", devname);
    return NULL;
    
}


/**
 * @brief       Get the connect state of the device by gpio level.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   gpionum
 *                  Number of the gpio.
 * @param[in]   active_level
 *                  Active logic level, 0 or 1. If the gpio is on the active
 *                  level, means the device is connected.
 * @return      bool
 * @retval      If the return value is true, means the device is 
 *              connected.
 *              If the return value is false, means the device is 
 *              disconnected.
 */ 
static bool gpio_get_state_bynum(unsigned long gpionum, bool active_level)
{
    if(gpio_get_pin_level(gpionum) == active_level)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**
 * @brief       Get the connect state of the device named by devname.
 *              The device is detected by a gpio.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pState
 *                  Pointer to a bool type variable for fetching 
 *                  the connecting state.
 * @param[in]   
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
static bool gpio_get_state_byname(const char * devname, bool *pState)
{
    T_DETECT_DEV_LIST * pdev_list_node;


    pdev_list_node = get_dev_list_node_byname(devname);
    if(NULL == pdev_list_node)
    {
        return false;
    }
    
    *pState = gpio_get_state_bynum(
        ((T_DETECT_GPIO_INFO *)(pdev_list_node->pInfo))->gpionum,
        ((T_DETECT_GPIO_INFO *)(pdev_list_node->pInfo))->active_level);

    return true;
}


/**
 * @brief       Insert the device node index into the queue.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   dev_list_index
 *                  Device node index.
 * @return      bool              
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
static bool queue_in_int_dev(unsigned char dev_list_index)
{
    unsigned long i;


    for(i=0; i<DEV_INT_QUEUE_LEN; ++i)
    {
        if((QUEUE_INVALID_INDXE == dev_int_queue[i].dev_list_index[0]) || 
            (dev_list_index == dev_int_queue[i].dev_list_index[0]))
        {
            break;
        }
    }

    if(i >= DEV_INT_QUEUE_LEN)
    {
        return false;
    }

    if((QUEUE_INVALID_INDXE == dev_int_queue[i].dev_list_index[0]) || 
        (dev_int_queue[i].front == dev_int_queue[i].rear)) //queue empty
    {
        dev_int_queue[i].dev_list_index[dev_int_queue[i].rear] = 
            dev_list_index;

        /* Enqueue, so
         * rear = (rear + 1) % queue_lenght
         * But for this case, queue_length = 2, so rear = 0 or 1;
         * so do this simply like followed.
         */
        dev_int_queue[i].rear = dev_int_queue[i].rear ^ 0x1;
    }

    //queue full , do nothing
    
    return true;
}



/**
 * @brief       Remove the device node index from the queue.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   queue_node
 *                  The device node index in the queue which index is 
 *                  queue_node will be remove.
 * @return      bool              
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
static bool queue_out_int_dev(unsigned long queue_node)
{    
    /*empty ?*/
    if((queue_node >= QUEUE_INVALID_INDXE) || 
        (dev_int_queue[queue_node].front == dev_int_queue[queue_node].rear))
    {
        return false;
    }

    /* dequeue, so
     * front = (front + 1) % queue_lenght
     * But for this case, queue_length = 2, so front = 0 or 1;
     * so do this simply like followed.
     */
    dev_int_queue[queue_node].front = dev_int_queue[queue_node].front ^ 0x1; 

    return true;;
}


/**
 * @brief       Read the device node index from the queue,but not remove the
 *              device node index.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   pqueue_node
 *                  Ponit to a variable which indicate the first index of the 
 *                  queue.
 *                  When return ,the variable indicate the index of queue which
 *                  contain the device node index.
 * @return      unsigned char              
 * @retval      If the function succeeds, the return value is the device node 
 *              index;
 *              If the function fails, the return value is QUEUE_INVALID_INDXE.
 */ 
static unsigned char queue_read_int_dev(unsigned long *pqueue_node)
{
    unsigned long i;

    for(i=(*pqueue_node); i<DEV_INT_QUEUE_LEN; ++i)
    {
        if((QUEUE_INVALID_INDXE == dev_int_queue[i].dev_list_index[0]) ||
            (dev_int_queue[i].front != dev_int_queue[i].rear))
        {
            break;
        }
    }

    if((i >= DEV_INT_QUEUE_LEN) || 
        (QUEUE_INVALID_INDXE == dev_int_queue[i].dev_list_index[0]))
    {
        return QUEUE_INVALID_INDXE;          }

    *pqueue_node = i;

    return dev_int_queue[i].dev_list_index[dev_int_queue[i].front];    
}



/**
 * @brief       Call back function of the gpio interrupt,used to insert
 *              the device node index into the dev_int_queue.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   gpionum
 *                  Gpio which has generated an interrupt;
 * @param[in]   level
 *                  The logic level of the gpio when it generated an 
 *                  interrupt.
 * @return      void
 */ 
static void gpio_int_callback( unsigned long gpionum, unsigned char level)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    unsigned long               param[7];
    unsigned long               i;

    
    for(i=0; i < DETECT_DEV_MAX; ++i)
    {
        if(NULL == dev_list[i].pdevname)
        {
            break; //continue;
        }
        
        if(eDETECT_OT_GPIO_INT == dev_list[i].type)
        {
            pgpio_info = (T_DETECT_GPIO_INFO *)dev_list[i].pInfo;
            if(pgpio_info->gpionum == gpionum)
            {                
                if(!queue_in_int_dev(i))
                {
                    akprintf(C2, M_DRVSYS,"interrupt is too frequent!\n");                    
                }
				
                level = gpio_get_pin_level(gpionum);
                gpio_set_int_p(gpionum, !level);
                gpio_int_control(gpionum, true);
                return;
            }
        }
    }

    akprintf(C2, M_DRVSYS,"\nMemory may be mussed!!!!\n\n");
    gpio_int_control(gpionum, false);
}



/**
 * @brief       gpio check function.if the device connected or disconnected, 
 *              the pcallbackfunc function seted by detector_set_callback()
 *              will be called.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   pdev_list_node
 *                  Pointer pointed to the device node.
 * @param[in]   bshake_chk
 *                  Avoid shaking flag.
 *                  true: checking with avoid_shaking;
 *                  false:checking without avoid_shaking;
 * @param[in]   bcallback
 *                  true: The call back function will be called;
 *                  false:The call back function won't be called;
 * @param[in]   pstate
 *                  If the pstate is not NULL, the connect state will be
 *                  returned by pstate;
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
static bool gpio_check_state(T_DETECT_DEV_LIST *pdev_list_node, 
                    bool bshake_chk, bool bcallback, bool *pstate)
{
    T_DETECT_GPIO_INFO       *pgpio_info;
    bool                    connect_state;


    pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;

    if(eDETECT_OT_GPIO_CHK == pdev_list_node->type)
    {
        GPIO_PROTECT(pgpio_info->gpionum);         
        gpio_set_pin_as_gpio(pgpio_info->gpionum);
        //gpio_set_pin_dir(pgpio_info->gpionum, GPIO_DIR_INPUT);
        //gpio_set_pull_up_r(pgpio_info->gpionum, false);        
    }

    connect_state = gpio_get_state_bynum(pgpio_info->gpionum, 
        pgpio_info->active_level);

    if(eDETECT_OT_GPIO_CHK == pdev_list_node->type)
    {
        GPIO_UNPROTECT(pgpio_info->gpionum);
    }

    if(NULL != pstate)
    {
        *pstate = connect_state;
    }


    if(false == bcallback)
    {
        return true;
    }
    

    if(bshake_chk && connect_state)
    {
        pgpio_info->shake_count = 
            pgpio_info->shake_count + pgpio_info->interval_ms;
        if(pgpio_info->shake_count < SHAKE_CHECK_TIME)
        {            
            return false;
        }
    }

    pgpio_info->shake_count = 0;
    
    if(pgpio_info->connect_state != connect_state)
    {
        if(pgpio_info->enable)
        {
            pgpio_info->connect_state = connect_state;
            pdev_list_node->cb(connect_state);
        }        
    }

    return true;
}


/**
 * @brief       ADC check function.if the device connected or disconnected, 
 *              the pcallbackfunc function seted by detector_set_callback()
 *              will be called.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   pdev_list_node
 *                  Pointer pointed to the device node.
 * @param[in]   bshake_chk
 *                  Avoid shaking flag.
 *                  true: checking with avoid_shaking;
 *                  false:checking without avoid_shaking;
 * @param[in]   bcallback
 *                  true: The call back function will be called;
 *                  false:The call back function won't be called;
 * @param[in]   pstate
 *                  If the pstate is not NULL, the connect state will be
 *                  returned by pstate;
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
static bool adc_check_state(T_DETECT_DEV_LIST *pdev_list_node, 
                    bool bshake_chk, bool bcallback, bool *pstate)
{
    static const T_VOLTAGE_TABLE   *plast_access_node = NULL; //cache for run fast
    const T_VOLTAGE_TABLE          *pvoltage_table;
    T_DETECT_ADC_INFO              *padc_info;
    unsigned long                           ad5value;    
    unsigned long                           connect_state;
    unsigned long                           connect_change_bit;    
    unsigned long                           i;
    bool                          bconnect;

  
    padc_info = (T_DETECT_ADC_INFO *)pdev_list_node->pInfo;

    ad5value = analog_getvalue_ain();

    if(bshake_chk)
    {
        if(((ad5value > padc_info->ad_value) && 
            (ad5value - padc_info->ad_value > padc_info->voltage_departure)) ||
            ((ad5value < padc_info->ad_value) && 
            (padc_info->ad_value - ad5value > padc_info->voltage_departure)))
        {            
            padc_info->ad_value = ad5value;
            padc_info->ad_chk_count = VOLTAGE_CHECK_TIME;
            return false;
        }

        if(padc_info->ad_chk_count >=  padc_info->interval_ms)
        {
            padc_info->ad_chk_count =  
                padc_info->ad_chk_count - padc_info->interval_ms;
            return false;
        }

    }
    

    connect_state = padc_info->connect_state;

    if((NULL != plast_access_node) && 
        ((ad5value >= plast_access_node->min_voltage) && 
        (ad5value <= plast_access_node->max_voltage)))
    {
        connect_state = plast_access_node->dev_connect_state;
    }
    else
    {
        pvoltage_table = padc_info->pvoltage_table;
        for(i=0; i < padc_info->voltageitem_num; ++i)
        {
            if((ad5value >= pvoltage_table[i].min_voltage) && 
                (ad5value <= pvoltage_table[i].max_voltage))
            {
                connect_state = pvoltage_table[i].dev_connect_state;
                plast_access_node = &pvoltage_table[i];
                break;
            }
        }
     
    }


    if(NULL != pstate)
    {
        i = pdev_list_node - &dev_list[padc_info->dev_list_start];
        
        *pstate = connect_state & (1<<i) ? true : false;
    }

    
    if(false == bcallback)
    {
        return true;
    }

    connect_change_bit = connect_state ^ padc_info->connect_state;

    if(0 == connect_change_bit)
    {
        return true;
    }
    
    for(i=0; connect_change_bit; ++i)
    {
        if(i > DETECT_DEV_MAX-1)
        {
            akprintf(C2, M_DRVSYS, "data error\n");
            break;
        }

        ad5value = connect_change_bit & 0x1;    //ad5value use for tmp        
        connect_change_bit = connect_change_bit >> 1;
        
        if(ad5value)
        {
            bconnect = connect_state & (1<<i) ? true : false;

            if(bshake_chk && bconnect)
            {
                padc_info->pshake_count[i] = 
                    padc_info->pshake_count[i] + padc_info->interval_ms;
                
                if(padc_info->pshake_count[i] < 
                    SHAKE_CHECK_TIME - VOLTAGE_CHECK_TIME)
                {
                    continue;                    
                }
            }

            padc_info->pshake_count[i] = 0;
                
            if(padc_info->enable_bit & (1<<i))
            {
                if(bconnect)
                {
                    padc_info->connect_state |= 1<<i;
                }
                else
                {
                    padc_info->connect_state &= ~(1<<i);
                }
                
                dev_list[padc_info->dev_list_start + i].cb(bconnect);
            }
        }
    }

    return true;    
}



/**
 * @brief       Call back function of the timer,used to check the gpio or
 *              ADC.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   timer_id
 *                  timer_id is used to map timer to the device;
 * @param[in]   delay
 * @return      void
 */ 
static void timer_chk_callback(signed long timer_id, unsigned long delay)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    unsigned long               dev_list_index;
    unsigned long               i;
    

    if(timer_id == dev_timer[DEV_TIMER_MAX - 1].hTimer)
    {
        /*
         * careful, i must be 0,when call queue_read_int_dev()
         * at first each time.
         */
        i = 0;          
        dev_list_index = queue_read_int_dev(&i);
        while(QUEUE_INVALID_INDXE != dev_list_index)
        {
            if(gpio_check_state(&dev_list[dev_list_index], true, 
                true, NULL))
            {
                queue_out_int_dev(i);
            }

            /*careful, check from the next queue*/
            ++i;       
            dev_list_index = queue_read_int_dev(&i);
        }
		
        return ;
    }
    for(i=0; (i < DEV_TIMER_MAX - 1) && (ERROR_TIMER != dev_timer[i].hTimer); ++i)
    { 
        if(timer_id == dev_timer[i].hTimer)
        {   		
            switch((dev_timer[i].pdev_list_node)->type)
            {
                case eDETECT_OT_GPIO_CHK: 
                    pgpio_info = (T_DETECT_GPIO_INFO *)
                        ((dev_timer[i].pdev_list_node)->pInfo);
                    if(false == pgpio_info->enable)
                    {
                        break;
                    }
                    
                    gpio_check_state(dev_timer[i].pdev_list_node, true, 
                        true, NULL);
                    break;
                case eDETECT_OT_ADC_CHK:
                    padc_info = (T_DETECT_ADC_INFO *)
                        ((dev_timer[i].pdev_list_node)->pInfo);
                    if(0 == padc_info->enable_bit)
                    {
                        break;
                    }
                    
                    adc_check_state(dev_timer[i].pdev_list_node, true, 
                        true, NULL);
                    break;
                default:
                    return;
                    break;
            }
            
            break;
        }
    }
}


/**
 * @brief       Init detector module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   void 
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_init(void)
{
    unsigned long  i;

    if(true == bdetect_module_init)
    {
        return true;
    }
    
    for(i=0; i < DEV_TIMER_MAX; ++i)
    {
        dev_timer[i].hTimer = ERROR_TIMER;
    }

    for(i=0; i < DEV_INT_QUEUE_LEN; ++i)
    {
        dev_int_queue[i].dev_list_index[0] = 
            dev_int_queue[i].dev_list_index[1] = QUEUE_INVALID_INDXE;
        dev_int_queue[i].front = dev_int_queue[i].rear = 0;
    }
    
    bdetect_module_init = true;
    return true;    
}

/**
 * @brief       Free detector module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   void
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_free(void)
{
    if(false == bdetect_module_init)
    {
        return false;
    }
    
    detector_set2default();    
    bdetect_module_init = false;
    return true;
}




/**
 * @brief       Set the call back function of device named by devname,the call 
 *              back function will be call when the device inserted or removed.
 *              After seting the call back function, the dettector of the 
 *              device will start automatically.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pcallbackfunc
 *                  Call back function of the device.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_set_callback(const char * devname,
            T_fDETECTOR_CALLBACK pcallbackfunc)
{
    T_DETECT_DEV_LIST * pdev_list_node;
    T_DETECT_GPIO_INFO *pgpio_info;    
    bool              gpio_level;
    T_DETECT_ADC_INFO  *padc_info;
    unsigned long               adc_no;
    unsigned long               i,j;



    if(false == bdetect_module_init)
    {
        akprintf(C2, M_DRVSYS, "Must call detector_init() first!\n");
        return false;
    }
    
    if(NULL == pcallbackfunc)
    {
        akprintf(C2, M_DRVSYS, "Callback function is null!\n");
        return false;
    }

    pdev_list_node = get_dev_list_node_byname(devname);
    if(NULL == pdev_list_node)
    {
        return false;
    }
    pdev_list_node->cb = pcallbackfunc;

    if((eDETECT_OT_GPIO_INT == pdev_list_node->type) || 
        (eDETECT_OT_GPIO_CHK == pdev_list_node->type))
    {
        pgpio_info = (T_DETECT_GPIO_INFO*)(pdev_list_node->pInfo);

        gpio_set_pin_as_gpio(pgpio_info->gpionum);
        gpio_set_pin_dir(pgpio_info->gpionum, GPIO_DIR_INPUT);
        gpio_set_pull_up_r(pgpio_info->gpionum, false);
        mini_delay(1);              // discharge 
        gpio_level = gpio_get_pin_level(pgpio_info->gpionum);     

    }


    switch(pdev_list_node->type)    
    {
        case eDETECT_OT_GPIO_INT:
            i = DEV_TIMER_MAX-1;
            if(NULL == dev_timer[i].pdev_list_node)
            {
                dev_timer[i].hTimer = vtimer_start(MAX_INTERVAL_CHECK, 
                    true, timer_chk_callback);
                if(ERROR_TIMER == dev_timer[i].hTimer)
                {
                    akprintf(C2, M_DRVSYS, "vtimer_start error\n");
                    return false;
                }
				
                dev_timer[i].pdev_list_node = pdev_list_node;                 
            }

            pgpio_info = (T_DETECT_GPIO_INFO*)(pdev_list_node->pInfo);
            gpio_register_int_callback(pgpio_info->gpionum, !gpio_level, 
                true, gpio_int_callback);
			
            pgpio_info->interval_ms = MAX_INTERVAL_CHECK;
                       

            /* If the device is connected when the system start up,
             * there is not a interrupt, so do ... 
             */
            gpio_check_state(pdev_list_node, false, 
                    false, &(pgpio_info->connect_state)); 
            pgpio_info->enable = true; 
			
            if(pgpio_info->connect_state)
            {
                pcallbackfunc(true);
            }
            
            break;
            
        case eDETECT_OT_GPIO_CHK:
            for(i=0; i<DEV_TIMER_MAX; ++i)
            {
                if(NULL == dev_timer[i].pdev_list_node)
                {
                    break;
                }
            }

            if(i >= DEV_TIMER_MAX)
            {
                akprintf(C1, M_DRVSYS, "Timer number is over %d\n", DEV_TIMER_MAX);
                return false;
            }

            pgpio_info = (T_DETECT_GPIO_INFO*)(pdev_list_node->pInfo);

            j = pgpio_info->interval_ms < MAX_INTERVAL_CHECK ? 
                    MAX_INTERVAL_CHECK : pgpio_info->interval_ms;

            dev_timer[i].hTimer = vtimer_start(j, true, timer_chk_callback);
            if(ERROR_TIMER == dev_timer[i].hTimer)
            {
                akprintf(C2, M_DRVSYS, "vtimer_start error\n");
                return false;
            }
            dev_timer[i].pdev_list_node = pdev_list_node;
            pgpio_info->interval_ms     = j;

            gpio_check_state(pdev_list_node, false, 
                    false, &(pgpio_info->connect_state)); 
            
            pgpio_info->enable = true; 
            
            if(pgpio_info->connect_state)
            {
                pcallbackfunc(true);
            }
            
            break;
            
        case eDETECT_OT_ADC_CHK:
            padc_info = (T_DETECT_ADC_INFO *)(pdev_list_node->pInfo);
            adc_no = pdev_list_node - &dev_list[padc_info->dev_list_start];            
            pdev_list_node = &dev_list[padc_info->dev_list_start];
            
            for(i=0; i<DEV_TIMER_MAX; ++i)
            {
                if((pdev_list_node == dev_timer[i].pdev_list_node) || 
                    (NULL == dev_timer[i].pdev_list_node))
                {
                    break;
                }
            }

            if(i >= DEV_TIMER_MAX)
            {
                akprintf(C2, M_DRVSYS, "Timer number is over %d\n", DEV_TIMER_MAX);
                return false;
            }

            if(NULL == dev_timer[i].pdev_list_node)
            {
                j = padc_info->interval_ms < MAX_INTERVAL_CHECK ? 
                        MAX_INTERVAL_CHECK : padc_info->interval_ms;
                
                dev_timer[i].hTimer = vtimer_start(j, true, timer_chk_callback);
                if(ERROR_TIMER == dev_timer[i].hTimer)
                {
                    akprintf(C2, M_DRVSYS, "vtimer_start error\n");
                    return false;
                }
                
                dev_timer[i].pdev_list_node = pdev_list_node;
                padc_info->interval_ms      = j; 
            } 

            adc_check_state(pdev_list_node + adc_no, false, 
                        false, &gpio_level);     //gpio_level use for tmp

            if(gpio_level)
            {
                padc_info->connect_state |= 1<<adc_no;
            }

            padc_info->enable_bit |= 1<<adc_no;            

            if(gpio_level)
            {                      
                pcallbackfunc(true);
            }
            
            break;
            
        default:
            akprintf(C2, M_DRVSYS,"\nMemory may be mussed!!!!\n\n");
            return false;
    }

    return true;
}


/**
 * @brief       Enable or disable the detector.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   benable
 *                  true：Enable the detector;
 *                  false：Disable the detector;
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 * @remark      It's not suggested to disable the detector,if the detector
 *              is disable, the connecting state of the device can't be 
 *              informed the user.
 *
 */ 
bool detector_enable(const char * devname,bool benable)
{    
    T_DETECT_DEV_LIST  *pdev_list_node;
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    unsigned long               i;

    pdev_list_node = get_dev_list_node_byname(devname);
    if(NULL == pdev_list_node)
    {
        return false;
    }

    if(NULL == pdev_list_node->cb)
    {
        akprintf(C2, M_DRVSYS, "Must call detector_set_callback() first!\n");
        return false;
    }


    switch(pdev_list_node->type)
    {
        case eDETECT_OT_GPIO_INT:
            pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;
            gpio_int_control(pgpio_info->gpionum, benable);
        case eDETECT_OT_GPIO_CHK:
            pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;
            pgpio_info->enable = benable;
            break;
        case eDETECT_OT_ADC_CHK:
            padc_info = (T_DETECT_ADC_INFO *)(pdev_list_node->pInfo);
            i = pdev_list_node - &dev_list[padc_info->dev_list_start];
            if(benable)
            {
                padc_info->enable_bit |= 1<<i;
            }
            else
            {
                padc_info->enable_bit &= ~(1<<i);
            }
            break;
        default:
            break;
    }

    return true;
}

/**
 * @brief       Determine whether the specified window is enabled.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pbenable
 *                  Pointer to a bool type variable for fetching the 
 *                  detector state.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_is_enabled(const char * devname, bool *pbenable)
{
    T_DETECT_DEV_LIST  *pdev_list_node;
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    unsigned long               i;

    pdev_list_node = get_dev_list_node_byname(devname);
    if(NULL == pdev_list_node)
    {
        return false;
    }


    switch(pdev_list_node->type)
    {
        case eDETECT_OT_GPIO_INT:
        case eDETECT_OT_GPIO_CHK:
            pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;
            *pbenable  = pgpio_info->enable;
            break;
        case eDETECT_OT_ADC_CHK:
            padc_info = (T_DETECT_ADC_INFO *)(pdev_list_node->pInfo);
            i = pdev_list_node - &dev_list[padc_info->dev_list_start];
         
            *pbenable = (padc_info->enable_bit & (1<<i))? true : false;
            break;
        default:
            break;
    }

    return true;
}



/**
 * @brief       Get the connecting state of the device named by devname.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pState
 *                  Pointer to a bool type variable for fetching the 
 *                  connecting state.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_get_state(const char * devname, bool *pState)
{
    T_DETECT_DEV_LIST  *pdev_list_node;
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    unsigned long               adc_no;
    bool              state;


    pdev_list_node = get_dev_list_node_byname(devname);
    if(NULL == pdev_list_node)
    {
        return false;
    }
    

    switch(pdev_list_node->type)
    {
        case eDETECT_OT_GPIO_INT:
        case eDETECT_OT_GPIO_CHK:
            pgpio_info = (T_DETECT_GPIO_INFO *)pdev_list_node->pInfo;
            if(false == pgpio_info->enable)
            {
                gpio_check_state(pdev_list_node, false, false, &state);
            }
            else
            {                
                state = pgpio_info->connect_state;                
            }
            break;            
        case eDETECT_OT_ADC_CHK:
            padc_info = (T_DETECT_ADC_INFO *)pdev_list_node->pInfo;
            adc_no = pdev_list_node - &dev_list[padc_info->dev_list_start];
            if(0 == (padc_info->enable_bit & (1<<adc_no)))
            {
                adc_check_state(pdev_list_node, false, false, &state);
            }  
            else
            {       
                state = (padc_info->connect_state) & (1<<adc_no) ? true : false;
            }
            break;
        default:
            break;         
    }

    *pState = state;
    return true;
}



/**
 * @brief       Set the detect module the default state just after registered.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   void
 * @return      void
 */ 
static void detector_set2default(void)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    T_DETECT_ADC_INFO  *padc_info;
    unsigned long i;

    for(i=0; i < DEV_TIMER_MAX; ++i)
    {
        if(NULL != dev_timer[i].pdev_list_node)
        {
            vtimer_stop(dev_timer[i].hTimer);
        }
    }
    memset(dev_timer, 0, sizeof(dev_timer));    

    for(i=0; i < DETECT_DEV_MAX; ++i)
    {
        if(NULL == dev_list[i].pdevname)
        {
            break; //continue;
        }

        switch(dev_list[i].type)
        {
            case eDETECT_OT_GPIO_INT:
                pgpio_info = (T_DETECT_GPIO_INFO *)dev_list[i].pInfo;
                gpio_int_control(pgpio_info->gpionum, false);
            case eDETECT_OT_GPIO_CHK:
                pgpio_info = (T_DETECT_GPIO_INFO *)dev_list[i].pInfo;
                pgpio_info->enable = false;
                pgpio_info->connect_state = false;
                break;
            case eDETECT_OT_ADC_CHK:
                padc_info = (T_DETECT_ADC_INFO *)(dev_list[i].pInfo);         
                padc_info->enable_bit = 0;   
                padc_info->connect_state = 0;
                break;
            default:
                break;
        }
    } 
    
    memset(&dev_int_queue, 0, sizeof(dev_int_queue));    }
    

/**
 * @brief       Unregister all the devices that has registered to the
 *              detect module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   void 
 * @return      void
 */ 
static void detector_unregister(void)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    void             *ptmp = NULL;
    unsigned long i;


    detector_set2default();

    for(i=0; i < DETECT_DEV_MAX; ++i)
    {
        if(NULL == dev_list[i].pdevname)
        {
            break; //continue;
        }
        
        switch(dev_list[i].type)
        {
            case eDETECT_OT_GPIO_INT:
            case eDETECT_OT_GPIO_CHK:
                drv_free(dev_list[i].pInfo);
                break;
            case eDETECT_OT_ADC_CHK:
                if(NULL == ptmp)
                {
                    ptmp = dev_list[i].pInfo;
                    continue;
                }
                else if(ptmp == dev_list[i].pInfo)
                {
                    continue;
                }
                else     //(ptmp != dev_list[i].pInfo)
                {
                    drv_free(ptmp);
                    ptmp = dev_list[i].pInfo;
                }

                break;
            default:
                break;
        }
    }

    if(NULL != ptmp)
    {
        drv_free(ptmp);
    }

    memset(dev_list, 0, sizeof(dev_list)); 
    
}



/**
 * @brief       Register the detector of GPIO type.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 *                  devname must point to a const string, because the detect 
 *                  module won't hold a copy of the device name. 
 * @param[in]   gpio_num
 *                  Number of the gpio.
 * @param[in]   active_level
 *                  Active logic level, 0 or 1. If the gpio is on the active
 *                  level, means the device is connected.
 * @param[in]   interrupt_mode
 *                  Detect type, true: interrupt, false: time.
 * @param[in]   interval_ms
 *                  The interval of checking, in ms.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */
bool detector_register_gpio(const char * devname, unsigned long gpio_num, 
            bool active_level, bool interrupt_mode, unsigned long interval_ms)
{
    T_DETECT_GPIO_INFO *pgpio_info;
    unsigned long               i;

    /* Checking if the device name is effective ?
     * Is the device has been registered ?
     */
    if((NULL == devname) || (0 == devname[0]))
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): " \
            "device name is empty\n");
        return false;
    }

    if(NULL != get_dev_list_node_byname(devname))
    {
        akprintf(C1, M_DRVSYS, "Device %s has registered\n", devname);
        return false;
    }    
    /* Checking if the gpio number is effective ?
     */
    if((gpio_num == INVALID_GPIO) || (gpio_num >= GPIO_NUMBER))
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): " \
            "gpio number is invalid\n");
        return false;   
    }
    
    /* Get a valid device node     
     */
    for(i=0; i<DETECT_DEV_MAX; ++i)
    {
        if(NULL == dev_list[i].pdevname)
        {
            break;
        }
    }

    if(i >= DETECT_DEV_MAX)
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): " \
            "detect device number is over %d\n", DETECT_DEV_MAX);
        return false;
    }

    /* Malloc memory for adc info.
     */
    pgpio_info = (T_DETECT_GPIO_INFO *)drv_malloc(sizeof(T_DETECT_GPIO_INFO));
    if(NULL == pgpio_info)
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): malloc failed\n");
        return false;
    }

    pgpio_info->gpionum       = gpio_num;
    pgpio_info->active_level  = active_level;
    pgpio_info->connect_state = false;
    pgpio_info->interval_ms   = interval_ms;
    pgpio_info->enable        = false;
    pgpio_info->shake_count   = 0;

    dev_list[i].pdevname = devname;
    dev_list[i].type     = interrupt_mode ? eDETECT_OT_GPIO_INT : eDETECT_OT_GPIO_CHK;
    dev_list[i].pInfo    = pgpio_info;
    dev_list[i].cb       = NULL;
    
    return true;    
}


 
 /**
  * @brief       Register the detector of ADC type.
  * @author      wangguotian
  * @date        2012.03.09
  * @param[in]   devname_list
  *                  Name list of the devices to be detected.
  *                  Each pointer in the name list must point to a const 
  *                  string, because the detect module won't hold a copy 
  *                  of the device name. 
  * @param[in]   devnum
  *                  Number of devices to be detected.
  * @param[in]   pvoltage_table
  *                  Voltage table .
  *                  pvoltage_table must point to a const memory, because
  *                  the detect module won't hold a copy of the Voltage 
  *                  table.
  * @param[in]   voltageitem_num
  *                  Number of voltage item of voltage table.
  * @param[in]   interval_ms
  *                  The interval of checking, in ms.
  * @return      bool
  * @retval      If the function succeeds, the return value is true;
  *              If the function fails, the return value is false.
  *
  * @remark      The member dev_connect_state of T_VOLTAGE_TABLE is a bitmap 
  *              of the devices' connecting state.1 means the device is 
  *              connected,and 0 means the device is disconnected.
  *              The bitmap order must correspond to the device's name in 
  *              devname_list, that is, bit 0 of dev_connect_state correspond
  *              to the first device named by devname_list[0], bit 1 of 
  *              dev_connect_state correspond to the second device named by 
  *              devname_list[1].
  *              All the devices specified by devname_list, must have different
  *              name, or else, the action of detector is not foreseeable.
  */ 
bool detector_register_adc(const char * *devname_list, unsigned long devnum, 
            const T_VOLTAGE_TABLE  *pvoltage_table, unsigned long voltageitem_num, 
            unsigned long interval_ms)
{
    T_DETECT_ADC_INFO *padc_info;
    unsigned long              voltage_departure;
    unsigned long              i,j;

    /* Checking if device list and each device name are effective ?
     * Is the device has been registered ?
     */
    for(i=0; i<devnum; ++i)
    {
        if((NULL == devname_list) || (NULL == devname_list[i]) || 
            (0 == devname_list[i][0]))
        {
            akprintf(C1, M_DRVSYS, "detector_register_adc(): " \
                "device name is empty\n");  
            return false;
        }

        if(NULL != get_dev_list_node_byname(devname_list[i]))
        {
            akprintf(C1, M_DRVSYS, "Device %s has registered\n", devname_list[i]);
            return false;
        }    }
    
    if(NULL == pvoltage_table)
    {
        akprintf(C1, M_DRVSYS, "detector_register_adc(): " \
                "voltage_table is empty\n");  
        return false;
    }


    /* Checking if dev_connect_state in pvoltage_table is effective ?
     * Should be dev_connect_state < (1<<devnum) 
     */
    for(i=0; i < voltageitem_num; ++i)
    {        
        if((pvoltage_table[i].max_voltage < pvoltage_table[i].min_voltage) ||
            (pvoltage_table[i].dev_connect_state >= (1<<devnum)))
        {
            akprintf(C1, M_DRVSYS, "detector_register_adc(): " \
                           "voltage_table data error\n");  
            return false;
        }
    }
    

    /* Get a valid device node     
     */
    for(i=0; i<DETECT_DEV_MAX; ++i)
    {
        if(NULL == dev_list[i].pdevname)
        {
            break;
        }
    }

    if((i >= DETECT_DEV_MAX) || (devnum > (DETECT_DEV_MAX - i)))
    {
        akprintf(C1, M_DRVSYS, "detector_register_gpio(): " \
            "detect device number is over %d\n", DETECT_DEV_MAX);
        return false;
    }


    /* Malloc memory for adc info and shake_count.
     */
    padc_info = (T_DETECT_ADC_INFO *)drv_malloc(sizeof(T_DETECT_ADC_INFO) + 
        sizeof(*(padc_info->pshake_count)) * devnum);    
    if(NULL == padc_info)
    {
        akprintf(C1, M_DRVSYS, "detector_register_adc(): malloc failed\n");
        return false;
    }

    

    padc_info->pvoltage_table   = pvoltage_table;
    padc_info->voltageitem_num  = voltageitem_num;
    padc_info->interval_ms      = interval_ms;
    padc_info->dev_list_start   = i;
    padc_info->connect_state    = 0;
    padc_info->enable_bit       = 0;
    padc_info->ad_value         = 0;
    padc_info->ad_chk_count     = 0;
    padc_info->pshake_count     = 
        (void *)((unsigned long)padc_info + sizeof(T_DETECT_ADC_INFO));

    memset(padc_info->pshake_count, 0, 
        sizeof(*(padc_info->pshake_count)) * devnum);

    /* Get the min voltage departure
     */
    padc_info->voltage_departure = ~0;
    for(j=0; j < voltageitem_num; ++j)
    {        
        voltage_departure = 
            pvoltage_table[j].max_voltage - pvoltage_table[j].min_voltage;
        voltage_departure = voltage_departure >> 1;
        if(voltage_departure < padc_info->voltage_departure)
        {
            padc_info->voltage_departure = voltage_departure;
        }     
    }


    while(devnum--)
    {
        dev_list[i].pdevname = *devname_list;
        dev_list[i].type     = eDETECT_OT_ADC_CHK;
        dev_list[i].pInfo    = padc_info;
        dev_list[i].cb       = NULL;

        ++devname_list;
        ++i;
    }
    
    return true;
}



