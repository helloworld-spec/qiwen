/**
 * @filename hal_freq_manager.c
 * @brief: frequency manager file.
 *
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  luheshan
 * @date    2012-05-16
 * @version 1.0
 */

#ifdef OS_ANYKA
#include    "anyka_types.h"
#include    "hal_freqmgr.h"
#include    "drv_module.h"
#include    "hal_print.h"
#include    "hal_timer.h"

#define FREQ_CHECK_CPU_UASGE_TIME       (200) //ms
#define FREQ_DELAY_DOWN_SET_TIME        (2000) //2S

#define FREQ_UNLOCK                     (0) //no lock

#define FREQ_CPU_UASGE_FACTOR1          (80)
#define FREQ_CPU_UASGE_FACTOR2          (20)

#define FREQ_MAX_DRV_NUM                (32) //max driver list number

#define FREQ_MGR_PROTECT \
        do{ \
            DrvModule_Protect(DRV_MODULE_FREQ_MGR);\
        }while(0)
        
#define FREQ_MGR_UNPROTECT \
        do{ \
            DrvModule_UnProtect(DRV_MODULE_FREQ_MGR);\
        }while(0)

typedef struct 
{
    unsigned long asic_freq;
    unsigned long count;
}T_FREQ_NODE,*T_pFREQ_NODE;

typedef struct 
{
    FreqMgr_Callback        ctl_cpu_cb;
    signed long                 ctl_timer;                  ///< timer id
    T_pFREQ_NODE            ctl_current;
    unsigned long                   ctl_lock; //init FREQ_UNLOCK
    unsigned long                   ctl_delay_down_time;
    unsigned long                   ctl_add_drv_cnt;
    unsigned long                   ctl_freq_node_max;
    unsigned long                   ctl_drv_optimum_freq;
    bool                  ctl_init;
}T_FREQ_CONTROL,*T_pFREQ_CONTROL;

static T_FREQ_CONTROL m_freq_control;

static T_pFREQ_NODE   m_freq_node_list = NULL;
static signed long          m_drv_list[FREQ_MAX_DRV_NUM];

static void FreqMgr_CheckCpuUsageTimerCb(signed long timer_id, unsigned long delay);
static T_pFREQ_NODE FreqMgr_GetNextASICFreq(bool IsUp);
static bool FreqMgr_AdjustFreq(T_pFREQ_NODE freq_node);
static unsigned long FreqMgr_GetCpuUsageFactor(void);


/**
 * @brief check timer call back,this function adjust asic frequency by cpu usage factor.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] timer_id
 * @param[in] delay
 * @return void
 * @retval void
 */
static void FreqMgr_CheckCpuUsageTimerCb(signed long timer_id, unsigned long delay)
{
    unsigned long cpu_usage_factor = 0;
    T_pFREQ_NODE  node_next = NULL;
    T_pFREQ_CONTROL control = &m_freq_control;
    bool is_need_adjust = false;
    
    FREQ_MGR_PROTECT; //protect
    
    //if lock,system do nothing
    if (control->ctl_lock != FREQ_UNLOCK)
    {
        FREQ_MGR_UNPROTECT; //unprotect
        return;
    }
        
    //get cpu usage factor.
    cpu_usage_factor = FreqMgr_GetCpuUsageFactor();

    if (cpu_usage_factor > FREQ_CPU_UASGE_FACTOR1) //up set asic
    {
        node_next = FreqMgr_GetNextASICFreq(true);
        is_need_adjust = true;
    }
    else if (cpu_usage_factor < FREQ_CPU_UASGE_FACTOR2) // down set asic
    {
        control->ctl_delay_down_time += delay;
        if (FREQ_DELAY_DOWN_SET_TIME > control->ctl_delay_down_time)
        {
            FREQ_MGR_UNPROTECT; //unprotect
            return;  //wait delay time 
        }
        
        //get down set asic frequency
        node_next = FreqMgr_GetNextASICFreq(false);
        is_need_adjust = true;
        control->ctl_delay_down_time = 0;
    }
    else // 20<=cpu_usage_factor<=80 no need to adjust asic frequency
    {}
        
    if (is_need_adjust && (node_next->asic_freq != control->ctl_current->asic_freq))
    {
        control->ctl_delay_down_time = 0;
        
        FreqMgr_AdjustFreq(node_next);
    }
    
    FREQ_MGR_UNPROTECT; //unprotect
}


/**
 * @brief get up/down set next asic frequency by current frequency
 * @author luheshan
 * @date 2012-05-18
 * @param[in] IsUp:true,is up set,AK_FLASH,is down set.
 * @return T_pFREQ_NODE
 * @retval values of the next freq node.
 */
static T_pFREQ_NODE FreqMgr_GetNextASICFreq(bool IsUp)
{
    unsigned long cur_index;
    unsigned long next_index = 0;

    //get current frequency list index
    cur_index = m_freq_control.ctl_current - &m_freq_node_list[0];

    //get next index
    if (cur_index < m_freq_control.ctl_freq_node_max)
    {
        if (IsUp) // will to set up asic frequency,index is current to max mid value
        {
            next_index = (m_freq_control.ctl_freq_node_max + cur_index) >> 1;
        }
        else //set down asic
        {
            if (m_freq_node_list[cur_index].asic_freq == m_freq_control.ctl_drv_optimum_freq)
            {
                next_index = cur_index;
            }
            else
            {
                next_index = cur_index - 1;
            }
        }
    }
    
    return &m_freq_node_list[next_index];
}


/**
 * @brief go to adjust asic frequency
 * @author luheshan
 * @date 2012-05-18
 * @param[in] freq_node:InPut asic frequency node
 * @return bool
 * @retval values of succeeds, the return value is true.
 */
static bool FreqMgr_AdjustFreq(T_pFREQ_NODE freq_node)
{
    unsigned long currFreq;
    
    if (false == set_asic_freq(freq_node->asic_freq))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:set asic freq fail\n");
        return false;
    }
    else //if success,change current frequency node
    {
        m_freq_control.ctl_current = freq_node;
    }
    
    return true;
}


/**
 * @brief get cpu usage factor by call back function what init had seted.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] void
 * @return unsigned long
 * @retval values of cpu usage is update 
 */
static unsigned long FreqMgr_GetCpuUsageFactor(void)
{
    unsigned long cpu_usage = 0;
        
    if (NULL != m_freq_control.ctl_cpu_cb)
    {
        cpu_usage = m_freq_control.ctl_cpu_cb();
    }

    return cpu_usage;
}

/**
 * @brief get asic list's node by frequency.
 * @author luheshan
 * @date 2012-06-08
 * @param[in] unsigned long Freq,the asic frequency
 * @return unsigned long
 * @retval values of asic list's node
 */
static unsigned long FreqMgr_GetAsicListNode(unsigned long Freq)
{
    unsigned long i; 
    unsigned long asic_node = 0;
    
    if (Freq >= m_freq_node_list[m_freq_control.ctl_freq_node_max - 1].asic_freq)
    {
        asic_node = m_freq_control.ctl_freq_node_max - 1;
    }
    else if (Freq <= m_freq_node_list[0].asic_freq)
    {
        asic_node = 0;
    }
    else
    {
        for(i = 1; i < m_freq_control.ctl_freq_node_max; i++)
        {
            if (Freq <= m_freq_node_list[i].asic_freq)
            {
                asic_node = i;
                break;
            }
        }
    }

    return asic_node;
}


/**
 * @brief If multi-driver request freqency,this function will get the optimum frequency.
 * @author luheshan
 * @date 2012-06-08
 * @param[in] void
 * @return T_pFREQ_NODE
 * @retval values of the optimum frequency's node
 */
static T_pFREQ_NODE FreqMgr_GetDriverOptimumFreq(void)
{
    unsigned long i; 
    unsigned long sum = 0;
    unsigned long asic_node = 0;

    //get all driver request frequency summation.
    for (i = 0; i < m_freq_control.ctl_freq_node_max; i++)
    {
        if (0 != m_freq_node_list[i].count)
        {
            sum += (m_freq_node_list[i].asic_freq * m_freq_node_list[i].count);
        }
    }

    //no need add to the system min frequency
    sum -= m_freq_node_list[0].asic_freq;
        
    asic_node = FreqMgr_GetAsicListNode(sum);
    
    m_freq_control.ctl_drv_optimum_freq = m_freq_node_list[asic_node].asic_freq;
        
    akprintf(C3, M_DRVSYS, "FreqMgr:driver_optimum_freq:%d\n" , m_freq_control.ctl_drv_optimum_freq);
    
    return &m_freq_node_list[asic_node];
}

/**
 * @brief frequency manager mode to initial.
 *        1.will init asic frequency and driver list, 
 *        2.start A timer to check cpu usage factor
 *        3.set the pll and defaule asic,cpu 2X.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] cpu_max_freq: input the cpu max value,37xx chip the max is 280000000 (hz) 
 * @param[in] asic_max_freq: input the asic min value,(hz)
 * @param[in] FreqMgr_Callback: get cpu usage factor call back function.
 * @return bool
 * @retval values of  succeeds, the return value is true.
 */
bool FreqMgr_Init(unsigned long cpu_max_freq, unsigned long asic_min_freq, FreqMgr_Callback CpuCb)
{

#if 1

	return false;

#else
	unsigned long i;
    T_pFREQ_CONTROL control = &m_freq_control;
    unsigned long *pAsic_list = NULL;
    unsigned long pll_freq;

    if (control->ctl_init)
    {
        return true;
    }

    if (NULL == CpuCb)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,call back function is null\n");
        return false;
    }

    if (!set_pll_value(cpu_max_freq/1000000))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,set pll is fail\n");
        return false;
    }
    
    //init frequency node list,min to max
    pll_freq = get_pll_value() * 1000000;
    if (!get_asic_node(pll_freq, asic_min_freq, NULL, &(control->ctl_freq_node_max)))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,get asic node list is fail 1\n");
        return false;
    }
    
    pAsic_list = (unsigned long *)drv_malloc(sizeof(unsigned long) * control->ctl_freq_node_max);
    if (NULL == pAsic_list)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,malloc is fail 1\n");
        return false;
    }

    if (!get_asic_node(pll_freq, asic_min_freq, &pAsic_list, &(control->ctl_freq_node_max)))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,get asic node list is fail 2\n");
        return false;
    }
    
    m_freq_node_list = (T_pFREQ_NODE)drv_malloc(sizeof(T_FREQ_NODE) * control->ctl_freq_node_max);
    if (NULL == m_freq_node_list)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init,malloc is fail 2\n");
        drv_free(pAsic_list);
        return false;
    }
    
    for (i = 0; i < control->ctl_freq_node_max; i++)
    {
        m_freq_node_list[i].asic_freq = pAsic_list[i];
        m_freq_node_list[i].count = 0;
    }
    m_freq_node_list[0].count = 1;

    drv_free(pAsic_list);

    //set the defaule asic,cpu 2X
    if ((!set_asic_freq(m_freq_node_list[control->ctl_freq_node_max - 1].asic_freq)) \
            || (!set_cpu_2x_asic(true)))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:init freq fail\n");
        drv_free(m_freq_node_list);
        m_freq_node_list = NULL;
        return false;
    }

    //start check timer
    control->ctl_timer = vtimer_start(FREQ_CHECK_CPU_UASGE_TIME, true, FreqMgr_CheckCpuUsageTimerCb);
    if (ERROR_TIMER == control->ctl_timer)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:timer start fail\n");
        drv_free(m_freq_node_list);
        m_freq_node_list = NULL;
        return false;
    }
    
    akprintf(C3, M_DRVSYS, "FreqMgr:init success,pll:%d,cpu:%d,asic:%d,max node num:%d\n",\
             get_pll_value(), get_cpu_freq(), get_asic_freq(), control->ctl_freq_node_max);

    //init driver list
    for (i = 0; i < FREQ_MAX_DRV_NUM; i++)
    {
        m_drv_list[i] = FREQ_INVALID_HANDLE;
    }
    
    //init conrtol variable
    control->ctl_current = &m_freq_node_list[control->ctl_freq_node_max - 1];
    control->ctl_lock = FREQ_UNLOCK;
    control->ctl_delay_down_time = 0;
    control->ctl_add_drv_cnt = 0;
    control->ctl_cpu_cb = CpuCb;
    control->ctl_drv_optimum_freq = m_freq_node_list[0].asic_freq;
    control->ctl_init = true;
    
    return true;

#endif
	
}


/**
 * @brief frequency manager mode to request asic min frequency for the specifically driver.
 *        1.if a driver is need a specifically asic frequency for runing, this function will be call
 *        2.if call this function,must be call FreqMgr_CancelFreq function to cancel.
 *        3.when the driver is runing and can't to change frequency, we will call FreqMgr_Lock to lock.
 *    notes: if request asic > current asic,and is lock,will return fail.
 *         The DrvA request the least frequency frist,and then,DrvB request.them need runing the some time.
 *         If the DrvA must be locked,the DrvB request frequency success,but will no to adjust asic frequency.
 *         If DrvA'least frequency less then DrvB,the system may be make a mistake.
 *         so ,the driver request frequency and need lock,the max value asic must be request frist.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] ReqFreq: input the driver specifically min frequency for runing.
 * @return T_hFreq:request frequency handle.
 * @retval values of  fail, the return value is FREQ_INVALID_HANDLE.
 */
T_hFreq FreqMgr_RequestFreq(unsigned long ReqFreq)
{
    unsigned long i;
    unsigned long asic_node = 0;
    T_hFreq freq_handle = FREQ_INVALID_HANDLE;
    T_pFREQ_CONTROL control = &m_freq_control;
    T_pFREQ_NODE optimum_freq_node;

    if (!control->ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return FREQ_INVALID_HANDLE;
    }
    
    FREQ_MGR_PROTECT; //protect
        
    //if Lock and request freq > current freq,return fail
    if (control->ctl_lock != FREQ_UNLOCK)
    {
        if (ReqFreq > control->ctl_current->asic_freq)
        {
            akprintf(C1, M_DRVSYS, "FreqMgr:freq is lock and request > current,return fail\n");
            FREQ_MGR_UNPROTECT; //unprotect
            return FREQ_INVALID_HANDLE;
        }
    }
    
    //get the frequency node for drvier request freq
    asic_node = FreqMgr_GetAsicListNode(ReqFreq);
    
    //add to driver list,will add the new driver to the list next one which is the last time add
    for (i = 0; i < FREQ_MAX_DRV_NUM; i++)
    {
        if (FREQ_MAX_DRV_NUM <= control->ctl_add_drv_cnt)
        {
            control->ctl_add_drv_cnt = 0;
        }
                    
        if (FREQ_INVALID_HANDLE == m_drv_list[control->ctl_add_drv_cnt])
        {
            m_freq_node_list[asic_node].count++;
            m_drv_list[control->ctl_add_drv_cnt] = asic_node;
            freq_handle = control->ctl_add_drv_cnt;
            control->ctl_add_drv_cnt++;
            break;
        }
        
        control->ctl_add_drv_cnt++;
    }

    if (i == FREQ_MAX_DRV_NUM)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:add drv to list is the max.:%d\n", FREQ_MAX_DRV_NUM);
        FREQ_MGR_UNPROTECT; //unprotect
        return FREQ_INVALID_HANDLE;
    }

    //get the optimum freq node
    optimum_freq_node = FreqMgr_GetDriverOptimumFreq();
    
    if (optimum_freq_node->asic_freq > control->ctl_current->asic_freq)
    {
        FreqMgr_AdjustFreq(optimum_freq_node);
    }

    FREQ_MGR_UNPROTECT; //unprotect
    //return handle
    return freq_handle;
}


/**
 * @brief frequency manager mode to cancle asic min frequency what had requested for the specifically driver.
 *        1.A driver had requested specifically asic frequency this function will be call by cancle request.
 *        2.If the driver had locked, when cancel,we will call FreqMgr_Lock to UnLock.
 * @author luheshan
 * @date 2012-05-18
 * @param[in] hFreq: input a handle what request freq return the handle.
 * @return bool
 * @retval values of  succeeds, the return value is true.
 */
bool FreqMgr_CancelFreq(T_hFreq hFreq)
{
    signed long freq_node;

    if (!m_freq_control.ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return false;
    }
        
    if ((FREQ_INVALID_HANDLE >= hFreq) ||(FREQ_MAX_DRV_NUM <= hFreq))
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:handle is invalid,may be has canceled 1!\n");
        return false;
    }
    
    FREQ_MGR_PROTECT; //protect
    
    //get frequency list node
    freq_node = m_drv_list[hFreq]; 

    if (FREQ_INVALID_HANDLE == freq_node)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:handle is invalid,may be has canceled 2!\n");
        FREQ_MGR_UNPROTECT; //unprotect
        return false;
    }
    
    //clean freq list drv request notes
    m_freq_node_list[freq_node].count--; 
    m_drv_list[hFreq] = FREQ_INVALID_HANDLE;

    //this function will change driver optimum freq.
    FreqMgr_GetDriverOptimumFreq();

    FREQ_MGR_UNPROTECT; //unprotect
    
    return true;
}


/**
 * @brief frequency manager mode to lock asic frequency not to be changed.
 *        1.if a driver when is runing and can't to change asic frequency, this function will be call
 *        2.if call this function,must be keep lock and unlock one by one.
 * @author luheshan
 * @date 2012-05-18
 * @paramvoid:
 * @return void:
 * @retval values of  void.
 */
void FreqMgr_Lock(void)
{
    if (!m_freq_control.ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return;
    }
    
    FREQ_MGR_PROTECT; //protect
    if (m_freq_control.ctl_lock != ~0)
    {
        m_freq_control.ctl_lock++;
    }
    FREQ_MGR_UNPROTECT; //unprotect
}

/**
 * @brief frequency manager mode to UnLock asic frequency.
 * @author luheshan
 * @date 2012-05-18
 * @paramvoid:
 * @return void:
 * @retval values of  void.
 */
void FreqMgr_UnLock(void)
{
    if (!m_freq_control.ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return;
    }
    
    FREQ_MGR_PROTECT; //protect
    
    if (FREQ_UNLOCK == m_freq_control.ctl_lock)
    {
        akprintf(C3, M_DRVSYS, "FreqMgr: warning: has been reUnLocked!!!\n");
    }
    else
    {
        m_freq_control.ctl_lock--;
    }
    
    FREQ_MGR_UNPROTECT; //unprotect
    
}

/**
 * @brief frequency manager mode to get lock status
 * @author luheshan
 * @date 2012-05-22
 * @paramvoid:
 * @return bool:
 * @retval values of  TRUE is lock.
 */
bool FreqMgr_IsLock(void)
{
    bool ret = false;

    if (!m_freq_control.ctl_init)
    {
        akprintf(C1, M_DRVSYS, "FreqMgr:is no init\n");
        return ret;
    }
    
    FREQ_MGR_PROTECT; //protect
    if (FREQ_UNLOCK != m_freq_control.ctl_lock)
    {
        ret = true;
    }
    FREQ_MGR_UNPROTECT; //unprotect

    return ret;
}

#endif

