/**
 * @file timer.c
 * @brief hardware timer source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "interrupt.h"
#include "drv_api.h"
#include "drv_module.h"
#include "timer.h"

#ifdef OS_ANYKA

//define timer register bits
#define TIMER_CLEAR_BIT                 (1<<30)
#define TIMER_FEED_BIT                  (1<<29)
#define TIMER_ENABLE_BIT                (1<<28)
#define TIMER_STATUS_BIT                (1<<27)
#define TIMER_READ_SEL_BIT              (1<<26)

//define pwm/pwm mode
#define MODE_PWM                        0x2
#define MODE_ONE_SHOT_TIMER             0x1
#define MODE_AUTO_RELOAD_TIMER          0x0       

//define timer frequency
#define TIMER_FREQ                      (12000000)

//max timer counter value
#define TIMER_MAX_COUNT                 0xffffffff

//max timer count for each hardware timer
#define TIMER_NUM_MAX                   6

//timer status
#define TIMER_INACTIVE                  0               /* inactive(available) */
#define TIMER_ACTIVE                    1               /* active */
#define TIMER_TIMEOUT                   2               /* time out */

typedef struct
{
        unsigned char    state;                  /* TIMER_INACTIVE or TIMER_ACTIVE or TIMER_TIMEOUT */
        bool  loop;                   /* loop or not */
        unsigned long   total_delay;            /* total delay, microseconds */
        unsigned long   cur_delay;              /* current delay, microseconds */
        T_fTIMER_CALLBACK callback_func;/* callback function only for current timer */
} T_TIMER_DATA;


//hardware timer struct
typedef struct
{
        unsigned long   interval; // interval time is ms
        unsigned long   clock;    // interval clock
        bool  bStatus; //open or close
        bool  bInit;   // init flag
    T_TIMER_DATA timer_data[TIMER_NUM_MAX];
}T_HARDWARE_TIMER;

//harware timer assignment 
static T_HARDWARE_TIMER m_hardtimer[ HARDWARE_TIMER_NUM ] = { 
          {250, 0, false, false, {0}},  //undefine function
          {10,  0, false, false, {0}},   //instant, keypad,touchscr 
          {5,   0, false, false, {0}},   //normal, vtimer,akos
          {1,   0, false, false, {0}},   //reserved 
          {5,   0, false, false, {0}},    //tick count 
        };

static const unsigned long timer_ctrl_reg1_grp[HARDWARE_TIMER_NUM] = {PWM_TIMER1_CTRL_REG1, PWM_TIMER2_CTRL_REG1, PWM_TIMER3_CTRL_REG1, PWM_TIMER4_CTRL_REG1, PWM_TIMER5_CTRL_REG1};
static const unsigned long timer_ctrl_reg2_grp[HARDWARE_TIMER_NUM] = {PWM_TIMER1_CTRL_REG2, PWM_TIMER2_CTRL_REG2, PWM_TIMER3_CTRL_REG2, PWM_TIMER4_CTRL_REG2, PWM_TIMER5_CTRL_REG2};
static const unsigned long timer_int_mask_grp[HARDWARE_TIMER_NUM] = {IRQ_MASK_TIMER1_BIT, IRQ_MASK_TIMER2_BIT, IRQ_MASK_TIMER3_BIT, IRQ_MASK_TIMER4_BIT, IRQ_MASK_TIMER5_BIT};

//variable contains the us tick count, increase at each timer5 interrupt
static volatile unsigned long long s_tick_count_us = 0;

//variable indicate wether timer5 is init or not
static bool m_bRTCStart = false;
//variable indicate how many timer clocks in one us 
static unsigned long  m_clkPerUs = TIMER_FREQ / 1000000;
//variable indicate how many ticks in a MAX timer cycle
static unsigned long  m_rtcCountUs = 0;

static bool timer_interrupt_handler(void);
static void timer5_interrupt_handler(void);

/**
 * @brief: Init timer, initial global variables, enable timer interrupt
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @return void
 * @retval
 */
void timer_init( void )
{
    unsigned short   i, j;
    unsigned long   clkPerMs;

    // init hardware timer struct
    for(j = 0; j < HARDWARE_TIMER_NUM; j++)
    {
        for ( i = 0; i < TIMER_NUM_MAX; i++ )
        {
            m_hardtimer[j].timer_data[i].state = TIMER_INACTIVE;
            m_hardtimer[j].timer_data[i].callback_func = NULL;
        }
    }

    clkPerMs = TIMER_FREQ / 1000;
    m_clkPerUs = TIMER_FREQ / 1000000;
    m_rtcCountUs = TIMER_MAX_COUNT / m_clkPerUs;
    
    for (i = 0; i < HARDWARE_TIMER_NUM; i++)
        m_hardtimer[i].clock = ( clkPerMs * m_hardtimer[i].interval );

    timer_reset();
        
    m_bRTCStart = true;
    s_tick_count_us = 0;

    //enable GPIO/timer interrupt
    int_register_irq(INT_VECTOR_TIMER, timer_interrupt_handler);

    /* start timer5 for tickcount */
    REG32(PWM_TIMER5_CTRL_REG1) = TIMER_MAX_COUNT;
    REG32(PWM_TIMER5_CTRL_REG2) = TIMER_ENABLE_BIT | TIMER_FEED_BIT | (MODE_ONE_SHOT_TIMER << 24);

    /* set init flag */
    for (i=0; i<HARDWARE_TIMER_NUM; i++)
    {
        m_hardtimer[i].bInit = true;
    }

    //enable timer5 interrupt
    INTR_ENABLE_L2(IRQ_MASK_TIMER5_BIT);

    return;
}

/**
 * @brief Start timer
 * When the time reach, the vtimer callback function will be called. User must call function
 * vtimer_stop() to free the timer ID, in spite of loop is true or false.
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param unsigned long milli_sec: Specifies the time-out value, in millisecond. Caution, this value must can be divided by 10.
 * @param bool loop: loop or not
 * @param T_fVTIMER_CALLBACK callback_func: Timer callback function. If callback_func is
 *                              not NULL, then this callback function will be called when time reach.
 * @return signed long: timer ID, user can stop the timer by this ID
 * @retval ERROR_TIMER: failed
 */
signed long timer_start(T_TIMER_ID hardware_timer, unsigned long milli_sec, bool loop, T_fTIMER_CALLBACK callback_func)
{
    unsigned short   i;
    unsigned long mode;

    /* check timer id */
    if(hardware_timer > uiTIMER4)
    {
    	akprintf(C3, M_DRVSYS, "hard timer id higher\r\n", i);
        return ERROR_TIMER;
    }
    
    /* check init flag */
    if(false == m_hardtimer[hardware_timer].bInit)
    {
    	akprintf(C3, M_DRVSYS, "hard timer not Init\r\n", i);
        return ERROR_TIMER;
    }

    /* disable gpio&timer interrupt to avoid reenter*/
    INTR_DISABLE(IRQ_MASK_SYS_MODULE_BIT);
    
    for (i = 0; i < TIMER_NUM_MAX; i++ )
    {
        if (m_hardtimer[hardware_timer].timer_data[i].state == TIMER_INACTIVE)
        {
            break;
        }
    }

    //No free timer
    if( i == TIMER_NUM_MAX) //TIMER_NUM_MAX - 1 is reserved timer
    {
        akprintf(C3, M_DRVSYS, "no more timer %d!!!!!!!!!!!!!!!!!!\r\n", i);
        INTR_ENABLE(IRQ_MASK_SYS_MODULE_BIT);
        return ERROR_TIMER;
    }
    else
    {
        m_hardtimer[hardware_timer].timer_data[i].state = TIMER_ACTIVE;
        m_hardtimer[hardware_timer].timer_data[i].loop = loop;
        m_hardtimer[hardware_timer].timer_data[i].total_delay = milli_sec;
        m_hardtimer[hardware_timer].timer_data[i].cur_delay = 0;
        m_hardtimer[hardware_timer].timer_data[i].callback_func = callback_func;

        if( m_hardtimer[hardware_timer].bStatus == false )
        {
            m_hardtimer[hardware_timer].bStatus = true;

            mode = MODE_AUTO_RELOAD_TIMER;

            REG32(timer_ctrl_reg1_grp[hardware_timer]) = m_hardtimer[hardware_timer].clock;
            REG32(timer_ctrl_reg2_grp[hardware_timer]) = TIMER_ENABLE_BIT | TIMER_FEED_BIT | (mode << 24);
            INTR_ENABLE_L2(timer_int_mask_grp[hardware_timer]);
        }
    }
    
    INTR_ENABLE(IRQ_MASK_SYS_MODULE_BIT);

    return (hardware_timer * TIMER_NUM_MAX + i);
}

/**
 * @brief Stop timer
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param signed long timer_id: Timer ID
 * @return void
 * @retval
 */
void timer_stop(signed long timer_id)
{
    unsigned short   i, hardware_timer;

    if(timer_id < 0 || timer_id >= HARDWARE_TIMER_NUM * TIMER_NUM_MAX)
    {
        akprintf(C3, M_DRVSYS, "stop the invalid timer!\r\n");
        return;
    }

    hardware_timer = timer_id / TIMER_NUM_MAX;
    timer_id = timer_id % TIMER_NUM_MAX;

    /* check init flag */
    if(false == m_hardtimer[hardware_timer].bInit)
        return;


    INTR_DISABLE(IRQ_MASK_SYS_MODULE_BIT);

    m_hardtimer[hardware_timer].timer_data[timer_id].state = TIMER_INACTIVE;

    for(i = 0; i < TIMER_NUM_MAX; i++ )
    {
        if(m_hardtimer[hardware_timer].timer_data[i].state != TIMER_INACTIVE)
        {
            INTR_ENABLE(IRQ_MASK_SYS_MODULE_BIT);       
            return;
        }
    }

    INTR_DISABLE_L2(timer_int_mask_grp[hardware_timer]);
    REG32(timer_ctrl_reg2_grp[hardware_timer]) = 0;

    m_hardtimer[hardware_timer].bStatus = false;

    INTR_ENABLE(IRQ_MASK_SYS_MODULE_BIT);

    return;
}

/**
 * @brief Timer interrupt handler
 * If chip detect that timer counter reach 0, this function will be called.
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param void
 * @return void

 *      NOTE    1. 若使能计数, 则当计算到0后, 还会继续递减(从最大值开始);
 *                         若不使能计数, 则保护为禁止计数时的值;
 *                      2. 若计数值为0, 但使能计数, 此时打开中断, 会引起中断的产生;
 *                         若计数值为0, 但禁止计数, 此时打开中断, 不会引起中断的产生;
 *                      3. a) 读中断状态寄存器(0x20000014), 并不会清除其TIMER BIT的值,
 *                                也不会清控制寄存器(0x20090014等)的BIT30(中断状态);
 *                         b) 读TIMER的控制寄存器(0x20090014等), 会清除中断状态寄存器的TIMER BIT的值,
 *                                同时也清控制寄存器(0x20090014等)的BIT30(中断状态);
 *                                直接将这一位设为0也能达到同样效果.
 *                      4. 进入中断后立即读计数值, 并不为0, 因为计数不会自动停止.
 *                      5. 若使用中断, 则计数值不能太小, 否则中断将占用大部分CPU时间而使程序运行异常.
 *                              (除中断函数外其它函数无法运行);


 */

static void timer_handler(T_TIMER_ID hardware_timer)
{
    unsigned short i = 0;
    
    for (i = 0 ; i < TIMER_NUM_MAX; i++)
    {
        if (m_hardtimer[hardware_timer].timer_data[i].state == TIMER_ACTIVE)
        {
            m_hardtimer[hardware_timer].timer_data[i].cur_delay += m_hardtimer[hardware_timer].interval;
            
            if (m_hardtimer[hardware_timer].timer_data[i].cur_delay >= m_hardtimer[hardware_timer].timer_data[i].total_delay)
            {
                if (m_hardtimer[hardware_timer].timer_data[i].loop)
                {
                    m_hardtimer[hardware_timer].timer_data[i].cur_delay = 0;
                }
                else
                {
                    /* do not set state as TIMER_INACTIVE here, else this timer ID will be allocated by
                        another process, in this case, call vtimer_stop() will cause mistake */
//                      timer_data[i].state = TIMER_INACTIVE;
                    m_hardtimer[hardware_timer].timer_data[i].state = TIMER_TIMEOUT;
                }

                if (m_hardtimer[hardware_timer].timer_data[i].callback_func != NULL)
                {
                    m_hardtimer[hardware_timer].timer_data[i].callback_func(
                        hardware_timer * TIMER_NUM_MAX + i, 
                        m_hardtimer[hardware_timer].timer_data[i].total_delay);
                }
            }
        }
    }
}

/**
 * @brief: gpio and timer interrupt handler
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param void
 * @return void
 * @retval
 */
static bool timer_interrupt_handler(void)
{               
    if( *( volatile unsigned long* )INT_STATUS_REG_2 & INT_STATUS_TIMER1_BIT )
    {               
        REG32(PWM_TIMER1_CTRL_REG2) |= TIMER_CLEAR_BIT;
        timer_handler(uiTIMER0);
    }
    else if( *( volatile unsigned long* )INT_STATUS_REG_2 & INT_STATUS_TIMER2_BIT )
    {
        REG32(PWM_TIMER2_CTRL_REG2) |= TIMER_CLEAR_BIT;
        timer_handler(uiTIMER1);
    }
    else if( *( volatile unsigned long* )INT_STATUS_REG_2 & INT_STATUS_TIMER3_BIT )
    {
        REG32(PWM_TIMER3_CTRL_REG2) |= TIMER_CLEAR_BIT;
        timer_handler(uiTIMER2);
    }
    else if( *( volatile unsigned long* )INT_STATUS_REG_2 & INT_STATUS_TIMER4_BIT )
    {
        REG32(PWM_TIMER4_CTRL_REG2) |= TIMER_CLEAR_BIT;
        timer_handler(uiTIMER3);
    }
    else if( *( volatile unsigned long* )INT_STATUS_REG_2 & INT_STATUS_TIMER5_BIT )
    {
        REG32(PWM_TIMER5_CTRL_REG2) |= TIMER_CLEAR_BIT;
        timer5_interrupt_handler();
    }
    else
    {
        return false;
    }
    
    return true;
}



/**
 * @brief Timer4 interrupt handler
 * If chip detect that timer counter reach 0, this function will be called.
 * Function vtimer_init() must be called before call this function
 * @author MiaoBaoli
 * @date 2004-09-22
 * @param void
 * @return void

 *      NOTE    1. 若使能计数, 则当计算到0后, 还会继续递减(从最大值开始);
 *                         若不使能计数, 则保护为禁止计数时的值;
 *                      2. 若计数值为0, 但使能计数, 此时打开中断, 会引起中断的产生;
 *                         若计数值为0, 但禁止计数, 此时打开中断, 不会引起中断的产生;
 *                      3. a) 读中断状态寄存器(0x20000014), 并不会清除其TIMER BIT的值,
 *                                也不会清控制寄存器(0x20090014等)的BIT30(中断状态);
 *                         b) 读TIMER的控制寄存器(0x20090014等), 会清除中断状态寄存器的TIMER BIT的值,
 *                                同时也清控制寄存器(0x20090014等)的BIT30(中断状态);
 *                                直接将这一位设为0也能达到同样效果.
 *                      4. 进入中断后立即读计数值, 并不为0, 因为计数不会自动停止.
 *                      5. 若使用中断, 则计数值不能太小, 否则中断将占用大部分CPU时间而使程序运行异常.
 *                              (除中断函数外其它函数无法运行);


 */

static void timer5_interrupt_handler(void)
{
    /* restart timer5 */
    REG32(PWM_TIMER5_CTRL_REG1) = TIMER_MAX_COUNT;
    REG32(PWM_TIMER5_CTRL_REG2) = TIMER_ENABLE_BIT | TIMER_FEED_BIT | (MODE_ONE_SHOT_TIMER << 24);

    s_tick_count_us += m_rtcCountUs;
}

static unsigned long timer_read_current_count(T_TIMER_ID timer_id)
{
    unsigned long count;
    
    //select read current count mode
    REG32(timer_ctrl_reg2_grp[timer_id]) |= TIMER_READ_SEL_BIT;

    count = REG32(timer_ctrl_reg1_grp[timer_id]);

    //recover read mode
    REG32(timer_ctrl_reg2_grp[timer_id]) &= ~TIMER_READ_SEL_BIT;

    return count;
}
/**
 * @brief Get tick count by ms
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param void
 * @return unsigned long: tick count
 * @retval
 */
unsigned long get_tick_count(void)
{
    unsigned long tick = 0;
    unsigned long ret = 0;
    unsigned long long CurUs = 0;

    if( !m_bRTCStart )
    {
        return ret;
    }

    irq_mask();
    
    tick = timer_read_current_count(uiTIMER4);        
    tick &= TIMER_MAX_COUNT;

    //!!!notice
    //if timer interrupt comes here, need to reverse the tick count
    //otherwise we may get wrong tick
    if(REG32(PWM_TIMER5_CTRL_REG2) & (TIMER_STATUS_BIT))
    {
        tick = 0;
    }

    CurUs = ( TIMER_MAX_COUNT - tick ) / m_clkPerUs + s_tick_count_us;
    ret = (unsigned long)(CurUs / 1000);
    
    irq_unmask();

    return ret;
}

/**
 * @brief Get tick count by us
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param void
 * @return unsigned long long: tick count by us
 * @retval
 */
unsigned long long get_tick_count_us(void)
{
    unsigned long tick = 0;
    unsigned long long CurUs = 0;

    if( !m_bRTCStart )
    {
        return CurUs;
    }

    irq_mask();
    
    tick = timer_read_current_count(uiTIMER4);        
    tick &= TIMER_MAX_COUNT;

    //!!!notice
    //if timer interrupt comes here, need to reverse the tick count
    //otherwise we may get wrong tick
    if(REG32(PWM_TIMER5_CTRL_REG2) & (TIMER_STATUS_BIT))
    {
        tick = 0;
    }
    
    CurUs = ( TIMER_MAX_COUNT - tick ) / m_clkPerUs + s_tick_count_us;
    
    irq_unmask();

    return CurUs;
}

/**
 * @brief: reset timer register to default value
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @return void
 * @retval
 */
void timer_reset(void)
{
    unsigned long i;

    for(i = 0; i < 5; i++)
    {
        REG32(timer_ctrl_reg1_grp[i]) = 0;
        REG32(timer_ctrl_reg2_grp[i]) = 0;
    }
}


unsigned long timer_interval(T_TIMER_ID hardware_timer)
{
    return m_hardtimer[hardware_timer].interval;
}

#endif//#ifdef OS_ANYKA

/* end of file */

