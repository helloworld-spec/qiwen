/**
 * @file arch_freq.h
 * @brief list frequency operation interfaces.

 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Liao_Zhijun
 * @date 2010-04-15
 * @version 1.0
 */

#ifndef __ARCH_FREQ_H__
#define __ARCH_FREQ_H__

/** @defgroup Frequency Frequency group
 *  @ingroup Drv_Lib
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif

/** @name define the main clock 
 */
#define MAIN_CLK    (get_asic_pll())

/** @name define the asic clock 
 */
#define ASIC_CLK    (MAIN_CLK>>1)

/** @name asic frequency define
 *  define the asic frequency by divider
 */
/*@{*/
#define ASIC_FREQ_BY_DIV1       (ASIC_CLK/1)
#define ASIC_FREQ_BY_DIV2       (ASIC_CLK/2)
#define ASIC_FREQ_BY_DIV4       (ASIC_CLK/4)
#define ASIC_FREQ_BY_DIV8       (ASIC_CLK/8)
#define ASIC_FREQ_BY_DIV16      (ASIC_CLK/16)
#define ASIC_FREQ_BY_DIV32      (ASIC_CLK/32)
#define ASIC_FREQ_BY_DIV64      (ASIC_CLK/64)
/*@} */


/**
 * @brief    get asic frequency node.
 *
 * @author    Luheshan
 * @date      2012-06-08
 * @param    [in] pll_val: the pll frequency (HZ).
 * @param    [in] min_freq: the min asic frequency(HZ).
 * @param    [in/out] node_list:input a pointer what is note frequency list.can be null,and to get node_cnt
 * @param    [in/out] node_cnt:input a pointer and get the asic frequency count.null is invalidation.
 * @return    bool
 * @retval    true  set asic frequency successful
 * @retval    false set asic frequency unsuccessful
 */
bool get_asic_node(unsigned long pll_freq, unsigned long min_freq, unsigned long **node_list, unsigned long *node_cnt);

/**
 * @brief   set asic frequency.
 *
 * @author  kjp
 * @date    2016-08-20
 * @param   freq [in] the freq value to be set, refer to T_ASIC_FREQ definition
 * @return  bool
 * @retval  true  set asic frequency successful
 * @retval  false set asic frequency unsuccessful
 */
bool set_asic_freq(unsigned long freq);

/**
 * @brief   get current asic frequency.
 *
 * @author  kjp
 * @date    2016-08-20
 * @return  unsigned long the frequency of asic running
 */
unsigned long get_asic_freq(void);

/**
 * @brief   set CPU PLL register value.
 *
 * main clock is controlled by pll register.
 * @author  kjp
 * @date    2016-08-20
 * @param   pll_value [in] pll register value(63-450)(Mhz)
 * @return  bool
 * @retval  true  set pll frequency successful
 * @retval  false set pll frequency unsuccessful
  */
bool set_cpu_pll(unsigned long pll_value);

/**
 * @brief   get CPU PLL register value.
 *
 * main clock is controlled by pll register.
 * @author  kjp
 * @date    2016-08-20
 * @return  unsigned long the frequency of pll(Mhz)
*/
unsigned long get_cpu_freq(void);

/**
 * @brief   set ASIC PLL register value.
 *
 * main clock is controlled by pll register.
 * @author  kjp
 * @date    2016-08-20
 * @param   pll_value [in] pll register value(63-450)(Mhz)
 * @return  bool
 * @retval  true  set pll frequency successful
 * @retval  false set pll frequency unsuccessful
  */
bool set_asic_pll(unsigned long freq);

/**
 * @brief   get PLL register value.
 *
 * main clock is controlled by pll register.
 * @author  kjp
 * @date    2016-08-20
 * @return  unsigned long the frequency of pll(Mhz)
*/
unsigned long get_asic_pll(void);


/**
 * @brief   set peri PLL register value.
 *
 * main clock is controlled by pll register.
 * @author  kjp
 * @date    2016-08-20
 * @param   pll_value [in] pll register value(63-450)(Mhz)
 * @return  bool
 * @retval  true  set pll frequency successful
 * @retval  false set pll frequency unsuccessful
  */
bool set_peri_pll(unsigned long freq);

/**
 * @brief   get PLL register value.
 *
 * main clock is controlled by pll register.
 * @author  kjp
 * @date    2016-08-20
 * @return  unsigned long the frequency of pll(Mhz)
*/
unsigned long get_peri_pll(void);


/**
 * @brief   set cpu frequency twice of asic frequency or not
 *
 * this function just set cpu_clk = PLL1_clk 
 * @author  kjp
 * @date    2016-08-20
 * @param   set_value  set twice or not
 * @return  bool
 * @retval  true setting successful
 * @retval  false setting fail
 */
bool set_cpu_2x(bool set_value);

/**
 * @brief   judge whether cpu frequency is twice of asic frequency or not
 *          
 * @author  kjp
 * @date    2016-08-20
 * @return  bool
 * @retval  true cpu frequency is twice of asic frequency
 * @retval  false cpu frequency is not twice of asic frequency
 */
bool get_cpu_2x(void);


#ifdef __cplusplus
}
#endif

/*@}*/
#endif //#ifndef __ARCH_FREQ_BASE__
