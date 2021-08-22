/**
 * @FILENAME: freq.c
 * @BRIEF freq driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR liao_zhijun
 * @DATE 2010-05-24
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "timer.h"
#include "l2.h"
#include "drv_module.h"
#include "freq.h"

/*
    ASPEN CPU clock test:
    以执行下面三条指令为例(共执行100兆次，T 表示执行时间):
          ADD      r0,r0,#1
        CMP      r0,r1
        BCC      0x300064d0
    以上三条指令所操作的数据均存放在寄存器中，因此在执行
    时，只会涉及到从内存取指的操作，以下分两种情况测试澹?
1.在打开ICACHE 和DCAHE的情况下
    当使能数据和指令CACHE时，CPU对这三条指令的操作只会涉及
    到寄存器和缓存的操作，速度最快
        CPU clock=60MHZ时，T=8.4s
        CPU clock=100MHZ时，T=5.0s    
    CPU 速度经倍频后的时间为:        
        CPU clock=60MHZ时，T=4.2s
        CPU clock=100MHZ时，T=2.5s

2.在关闭ICACHE 和DCACHE的情况下
    此时CPU需要到内存中取指，这将显著的降低CPU的执行速度
        CPU clock=60MHZ， T = 45.3s
    此外，在关闭ICACHE，打开DCACHE 的情况下，
        CPU clock=60MHZ， T = 44s

    从这两个数据可以看出，CPU在执行这种指令时的大部分时间
    被消耗在取指上

3.功耗
    在小系统中，关闭LCD的情况下测定
    CPU clock=60MHZ， I = 33mA
    CPU clock=200MHZ， I = 45mA

4. 计算方法
    给定特定的指令序列，其执行消耗的时间及占用的CPU clock的
    计算方法为:

    cnt:执行指令的次数
    n:   指令条数
    clk: CPU时钟周期
    每条ARM指令占用两个CPU时钟周期

    指令的执行时间T = n*cnt*2/clk

    每条指令占用的CPU时钟周期:
        t = T*clk/(n * cnt)    
*/

/* define max freqency divider */

#define PLL_CLK_MIN            180
#define PLL_CLK_MAX            (PLL_CLK_MIN + 4 *(0x30))
#define PLL_CLK_MODERATE_MAX   (280)

#define MARK_3X_CFG             (1 << 25)
#define CPU_2X_CFG              (1 << 24)

/* clock divider register */
#define PLL_CHANGE_ENA          (1 << 12)
#define CLOCK_ASIC_ENA          (1 << 14)


/** @name asic frequency define
 *  define the asic frequency by divider
 */
/*@{*/
#define ASIC_PLL_BY_DIV2       (2)
#define ASIC_PLL_BY_DIV4       (4)
#define ASIC_PLL_BY_DIV8       (8)
#define ASIC_PLL_BY_DIV16      (16)
#define ASIC_PLL_BY_DIV32      (32)
#define ASIC_PLL_BY_DIV64      (64)
#define ASIC_PLL_BY_DIV128     (128)

/*@} */


typedef enum _clk_pll_type
{
	CPU = 0,
	ASIC,
	PERI
}T_CLK_PLL_TYPE;


typedef struct _clk_pll_clock_para
{
	unsigned long m;
	unsigned long n;
	unsigned long od;
}T_CLK_PLL_CLOCK_PARA;

typedef enum
{
    FREQ_NORMAL_MODE,
    FREQ_CPU3X_MODE,
    FREQ_BOOST_MODE,
    FREQ_CPU_LOW_MODE
}
E_FREQ_MODE;

bool is_cpu_3x(void);
bool is_cpu_2x(void);
unsigned long get_mem_freq(void);
unsigned long get_asic_pll(void);

//cpu 2x counter

#define MAX_FREQ_CALLBACK       10

//callback func
static T_fFREQ_CHANGE_CALLBACK m_mem_cbk = NULL;
static T_fFREQ_CHANGE_CALLBACK m_uart_cbk = NULL;
static T_fFREQ_CHANGE_CALLBACK m_asic_cbk[MAX_FREQ_CALLBACK] = {0};

static unsigned char index=0;

//////////////////////////////////////////////////

//Pll clk parameter N
static const unsigned long N_SET[5] = { 2, 3, 4, 5, 6};

//Pll clk parameter od
static const unsigned long OD_SET[3] = { 1, 2, 3};

//max frequence
static const unsigned long MAX_PLL_FREQ_SET[3] = { 540,  400,  600 }; //{ 450,  400,  600 }

//divider val
static const unsigned long DIV_VAL_SET[7] = { 2,  4,  8,  16,  32,  64,  128 };
static const unsigned long DIV_SEL_SET[7] = { 1,  2,  3,  4,  5,  6,  7 };


/**
* @BRIEF  calculate the m , n, od
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -26
* @PARAM  freq
* @PARAM  T_CLK_PLL_CLOCK_PARA
* @RETURN   true:   calculate m, n, od parameter success 
*                  false:  calculate m, n, od parameter false 
* @COMMENT  程序按照传入的freq自动计算M , N , OD.
* @NOTE: 
*/
static bool clk_calc_m_n_od(unsigned long freq_mhz,  T_CLK_PLL_CLOCK_PARA *p_pll_clk_para )
{
	unsigned long i, j;
	unsigned long freq_mul_od, m_val, m_remain;
	unsigned long set_clk_ok;
	T_CLK_PLL_CLOCK_PARA cpu_pll_para;


	set_clk_ok = false;
	for( i = 0; i < sizeof(OD_SET)/4; i++)
	{
		freq_mul_od = (freq_mhz * (1<<OD_SET[i]) );

		// result  must   500MHZ <= freq_mul_od <= 1500MHZ
		// mean:   500MHZ <= 12M/N <= 1500MHZ
		if ( (freq_mul_od < 500) || (freq_mul_od > 1500) )
		{
			continue;
		}
		for(j = 0; j < sizeof(N_SET)/4; j++)
		{
			//calculate the m value could divider by 12
			m_remain = (freq_mul_od * N_SET[j] ) % 12;
			if( m_remain == 0)
			{
				m_val = (freq_mul_od * N_SET[j] ) / 12;

				//m[0] is no use
				if((m_val & 0x1) == 0x1)
				{
					continue;
				}
				if( (m_val >= 84) && (m_val <= 254))
				{
					p_pll_clk_para->m = (freq_mul_od * N_SET[j] ) / 12;
					p_pll_clk_para->n = N_SET[j];	
					p_pll_clk_para->od = OD_SET[i];
					set_clk_ok = true;		
					return set_clk_ok;
				}
			}
		}
	}

	return set_clk_ok;
	
}


/**
* @BRIEF  set pll frequence
* @AUTHOR 
* @DATE   2012-10 -26
* @PARAM  freq        unit: MHZ
* @PARAM  T_CLK_PLL_CLOCK_PARA
* @RETURN   true:   Set pll frequence success 
*                  false:  Set pll frequence false 
* @COMMENT   
*              
*			
* @NOTE: 
*/
static bool clk_set_pll_freq(unsigned long freq,  T_CLK_PLL_TYPE pll_type)
{
	unsigned long i;
	unsigned long pll_reg_addr;
	unsigned long refresh_val;
	unsigned long powerof_od = 1;
	T_CLK_PLL_CLOCK_PARA pll_para;
	
	
	if( (freq < 63) || (freq > MAX_PLL_FREQ_SET[pll_type]) )
	{
		akprintf(C1, M_DRVSYS, "pll_clk must bigger than 63MHZ and smaller than MAX_PLL_FREQ_SET !\n");
		return false;
	}
	
	if( clk_calc_m_n_od( freq,  &pll_para ) == false)
	{
		akprintf(C1, M_DRVSYS, "calculate m, n, od failed!\n");
		return false;
	}
	
	switch(pll_type)
	{
		case CPU:			
			// write m, n, od
			REG32(CPU_CLOCK_DIV_REG) &= (~0x3fff);
			REG32(CPU_CLOCK_DIV_REG) |= (pll_para.m | (pll_para.n<<8) | (pll_para.od<<12)) ;

			// wait pll adjust finish
			while( (inl(CPU_CLOCK_DIV_REG)  & (1<<14)) !=0)
			{;}
			break;
			
		case ASIC:
			// wait pll adjust finish
			while( (inl(CPU_CLOCK_DIV_REG)  & (1<<28)) !=0)
			{;}
			
			// write m, n, od
			REG32(ASIC_CLOCK_DIV_REG) &= (~0x3fff);
			REG32(ASIC_CLOCK_DIV_REG) |= (pll_para.m | (pll_para.n<<8) | (pll_para.od<<12)) ;
			REG32(CPU_CLOCK_DIV_REG) |= (1<<28);
			
			// wait pll adjust finish
			while( (inl(CPU_CLOCK_DIV_REG)  & (1<<28)) !=0)
			{;}
			break;

		case PERI:
			// wait pll adjust finish
			while( (inl(CPU_CLOCK_DIV_REG)  & (1<<29)) !=0)
			{;}
			
			// write m, n, od
			REG32(PERI_CLOCK_DIV_REG_1) &= (~0x3fff);
			REG32(PERI_CLOCK_DIV_REG_1) |= (pll_para.m | (pll_para.n<<8) | (pll_para.od<<12)) ;
			REG32(CPU_CLOCK_DIV_REG) |= (1<<29);
			
			// wait pll adjust finish
			while( (inl(CPU_CLOCK_DIV_REG)  & (1<<29)) !=0)
			{;}
			break;
			
		default:
			break;			
	}

	return true;	
	
}


/**
* @BRIEF  set asic vclk div value
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -19
* @PARAM  div   div is  2, 4, 8 , 16, 32, 64, 128
* @PARAM  
* @RETURN   true:   Set jclk hclk div success 
*                  false:  Set jclk hclk div false 
*			
* @NOTE: 
*/
static bool clk_set_vclk_asicclk_div(unsigned long div)
{
	unsigned long i;
	unsigned long div_right;
	
	div_right = false;
	for( i = 0;  i < sizeof(DIV_VAL_SET)/4; i++)
	{
		if( DIV_VAL_SET[i] ==  div)
		{
			div_right = true;
			break;
		}
	}

	if( div_right == false )
	{
		akprintf(C1, M_DRVSYS, "Not support , div must be 2, 4, 8, 16, 32, 64, 128.\n");
		return false;
	}
	
	REG32(ASIC_CLOCK_DIV_REG) &= (~(7<<17));
	REG32(ASIC_CLOCK_DIV_REG) |= (DIV_SEL_SET[i]<<17);
	REG32(ASIC_CLOCK_DIV_REG) |= (1<<23);	

	while( (REG32(ASIC_CLOCK_DIV_REG) & (1<<23)) != 0)
	{;}
	
	return true;
		
	
}



/**
 * @brief    get PERI PLL .
 * @author    
 * @date     2010-04-06
 * @return    unsigned long the frequency of asic running ,HZ
 */
static unsigned long get_pll(T_CLK_PLL_TYPE pll_type)
{
	unsigned long m, n, od;
	unsigned long pll_cfg_val;
	unsigned long pll_freq;

	switch(pll_type)
	{
		case CPU:
			pll_cfg_val = inl(CPU_CLOCK_DIV_REG) ;
			break;
		case ASIC:
			pll_cfg_val = inl(ASIC_CLOCK_DIV_REG) ;
			break;
		case PERI:
			pll_cfg_val = inl(PERI_CLOCK_DIV_REG_1) ;
			break;
		default:
			akprintf(C1, M_DRVSYS, "Not PLL type\n");
			return 0;
		
	}
	m = (pll_cfg_val & 0xfe);
	n = ((pll_cfg_val & 0xf00)>>8);
	od = ((pll_cfg_val & 0x3000)>>12); 

	pll_freq = (m * 12 * 1000000) / (n *  (1<<od));
	return pll_freq ;
}



/**
* @BRIEF  get cpu pll clk
* @AUTHOR  
* @DATE   2012-10 -24
* @PARAM  
* @PARAM  
* @RETURN cpu pll frequence   unit: HZ
* @NOTE: 
*/
static unsigned long clk_get_cpu_pll_freq(void)
{
	return get_pll(CPU); 
}

/**
* @BRIEF  get  cpu_hclk
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24
* @PARAM  
* @PARAM  
* @RETURN cpu hclk frequence   unit: HZ
* @NOTE: 		内部的HCLK
*/
static unsigned long clk_get_cpu_hclk_freq(void)
{
	unsigned long cpu_hclk_sel;
	unsigned long cpu_pll_freq;
	unsigned long cpu_hclk_freq;

	cpu_hclk_sel =  ((inl(CPU_CLOCK_DIV_REG) >> 17) & 0x7);
	if(cpu_hclk_sel == 0)
	{
		cpu_hclk_sel += 1;
	}
	cpu_pll_freq = clk_get_cpu_pll_freq();
	cpu_hclk_freq = (cpu_pll_freq / (1<<cpu_hclk_sel));

	return cpu_hclk_freq;
}

/**
* @BRIEF  get  cpu_dclk
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24
* @PARAM  
* @PARAM  
* @RETURN cpu dclk frequence   unit: MHZ
* @NOTE: 	内部的DCLK
*/
static unsigned long clk_get_cpu_dclk_freq(void)
{
	unsigned long cpu_dclk_sel;
	unsigned long cpu_pll_freq;
	unsigned long cpu_dclk_freq;

	cpu_dclk_sel =  ((inl(CPU_CLOCK_DIV_REG) >> 20) & 0x7);
	if(cpu_dclk_sel == 0)
	{
		cpu_dclk_sel += 1;
	}
	cpu_pll_freq = clk_get_cpu_pll_freq();
	cpu_dclk_freq = (cpu_pll_freq / (1<<cpu_dclk_sel));

	return cpu_dclk_freq;
}


/**
* @BRIEF  get cpuclk ahbclk  memclk  frequence
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -25
* @PARAM  
* @return PARAM  : cpuclk ahbclk  memclk  frequence   unit: HZ , 
* @RETURN 
* @NOTE: 
*/
static void clk_get_cpuclk_ahbclk_memclk_freq(unsigned long *p_cpuclk, unsigned long *p_ahbclk, unsigned long *p_memclk)
{
	unsigned long clk2x_cfg;
	unsigned long clk3x_cfg;
	unsigned long clk3x_en_cfg;
	unsigned long pll_cfg_val;
	unsigned long cpu_pll_freq;
	unsigned long cpu_hclk_freq;
	unsigned long cpu_dclk_freq;

	cpu_pll_freq = clk_get_cpu_pll_freq();
	cpu_hclk_freq = clk_get_cpu_hclk_freq();
	cpu_dclk_freq = clk_get_cpu_dclk_freq();
	
	pll_cfg_val = inl(CPU_CLOCK_DIV_REG) ;
	
	clk2x_cfg = (pll_cfg_val & (1<<24));
	clk3x_en_cfg = (pll_cfg_val & (1<<25) );
	clk3x_cfg = (pll_cfg_val & (1<<26));
	
	if( clk2x_cfg != 0 )
	{
		// check whether the software config right or wrong
		if( (clk3x_en_cfg != 0) || (clk3x_cfg != 0))
		{
			akprintf(C1, M_DRVSYS, "error software config, don't set clk2x_cfg and clk3x_cfg clk3x_en_cfg at the same time!\n");
			while(1);
		}
			
		*p_cpuclk = cpu_pll_freq;
		*p_ahbclk = cpu_hclk_freq;
		*p_memclk = cpu_dclk_freq;
	}
	
	else
	{
		if( (clk3x_en_cfg != 0) &&  (clk3x_cfg != 0))	
		{
			*p_cpuclk = cpu_pll_freq;
			*p_ahbclk = cpu_pll_freq/3;
			*p_memclk = cpu_pll_freq/3;
		}
		else if( (clk3x_en_cfg == 0) &&  (clk3x_cfg == 0))	
		{
			*p_cpuclk = cpu_hclk_freq;
			*p_ahbclk = cpu_hclk_freq;
			*p_memclk = cpu_dclk_freq;
		}
		else
		{
			akprintf(C1, M_DRVSYS, "error software config, clk3x_en_cfg and clk3x_cfg must same!\n");
			while(1);
		}
		
	}
	

}


static void freq_adjust(unsigned long freq)
{
    unsigned long i;

    //memory_timing adjust function should be called
    if(m_mem_cbk != NULL)
        m_mem_cbk(freq);

    for(i = 0; i < index; i++)
    {
        m_asic_cbk[i](freq);
    }
}


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
bool get_asic_node(unsigned long pll_freq, unsigned long min_freq, unsigned long **node_list, unsigned long *node_cnt)
{
    unsigned long i;
    unsigned long node_max = 0;
    unsigned long *pNode_list = *node_list;
    unsigned long asic;

    if (((NULL != node_list) && (NULL == *node_list)) || (NULL == node_cnt))
    {
        akprintf(C1, M_DRVSYS, "get_asic_node(): param is null\n");
        return false;
    }

    if (min_freq > (pll_freq >> 1))
    {
        akprintf(C1, M_DRVSYS, "get_asic_node(): min_freq is no min\n");
        return false;
    }
    
    for (i = 1; ; i++)
    {
        if (min_freq > (pll_freq / (i * 2)))
        {
            node_max = i - 1;
            break;
        }
    }
    
    if (NULL != node_list)
    {   
        if (node_max > *node_cnt)
        {
            akprintf(C1, M_DRVSYS, "get_asic_node(): node list memory may be no enough\n");
            return false;
        }

        for (i = 0; i < node_max; i++)
        {
            asic = pll_freq / ((node_max - i) * 2);
            if (0 != (pll_freq % ((node_max - i) * 2)))
            {
                asic++;
            }
            pNode_list[i] = asic;
        }
    }

    *node_cnt = node_max;
    
    return true;
}


/**
 * @brief    set asic frequency.
 *
 * @author    liaozhijun
 * @date     2010-04-06
 * @param    freq [in] the freq value to be set, refer to T_ASIC_FREQ definition
 * @return    bool
 * @retval    true  set asic frequency successful
 * @retval    false set asic frequency unsuccessful
 */
bool set_asic_freq(unsigned long freq)  ///
{
#if 0
    unsigned long pre_freq, later_freq = 0;
	T_CLK_PLL_CLOCK_PARA pll_para;

    pre_freq = get_asic_freq();
    

    if (freq == pre_freq)
    {
        return true;
    }

    if (is_cpu_3x())
    {
        akprintf(C1, M_DRVSYS, "set_asic_freq(): under cpu3x!!\n");
        return false;
    }
	freq = freq << 1;  	//asic freq *2 = asic PLL
    //calculate divide
    if( clk_calc_m_n_od( freq, &pll_para) == false)
    {
        akprintf(C1, M_DRVSYS, "set_asic_freq(): cannot get proper asic div\n");
        return false;
    }
    
    DrvModule_Protect(DRV_MODULE_FREQ);

	REG32(ASIC_CLOCK_DIV_REG) &= ~((1<<24)|(7<<17));	 //asic clk = VCLK = ASIC PLL
	//clk_set_pll_freq(0,,ASIC);

    later_freq = get_asic_freq();
    
    //adjust asic module clock
    if(m_uart_cbk != NULL)
        m_uart_cbk(later_freq);

    //freq high to low, adjust asic module timing later
    if (freq < pre_freq)
    {
        freq_adjust(later_freq);
    }
    DrvModule_UnProtect(DRV_MODULE_FREQ);
#else
	clk_set_vclk_asicclk_div(ASIC_PLL_BY_DIV2);
#endif
    return true;
}


/**
 * @brief    get current asic frequency.
 *
 * @author    
 * @date     2010-04-06
 * @return    unsigned long the frequency of asic running
 */
unsigned long get_asic_freq(void)
{
	unsigned long vclk_sel;
	unsigned long core_pll_freq;
	unsigned long vclk_freq;

	vclk_sel =  ((inl(ASIC_CLOCK_DIV_REG) >> 17) & 0x7);
	if(vclk_sel == 0)
	{
		vclk_sel += 1;
	}
	core_pll_freq = get_asic_pll();
	vclk_freq = (core_pll_freq / (1<<vclk_sel));

	return vclk_freq;  //asic clock = vclk
}



/**
 * @brief    set asic PLL.
 *
 * @author    
 * @date     2010-04-06
 * @param    freq [in] the PLL value to be set,Mhz
 * @return    bool
 * @retval    true  set asic frequency successful
 * @retval    false set asic frequency unsuccessful
 */
bool set_asic_pll(unsigned long freq)
{
	return (bool)clk_set_pll_freq(freq, ASIC);
}


/**
 * @brief    get current asic frequency.
 *
 * @author    
 * @date     2010-04-06
 * @return    unsigned long the frequency of asic running ,HZ
 */
unsigned long get_asic_pll(void)
{
	return get_pll(ASIC); 
}



/**
 * @brief    get current memory clock.
 *
 * @author    
 * @date     2010-04-06
 * @return    unsigned long the frequency of memory
 */
unsigned long get_mem_freq(void)
{
	unsigned long cpuclk_freq, ahbclk_freq, memclk_freq;

	//check clock gate
	if( inl(CLOCK_CTRL_REG) & (1<<24)  == (1<<24) )
	{
		return 0;
	}

	clk_get_cpuclk_ahbclk_memclk_freq(&cpuclk_freq, &ahbclk_freq, &memclk_freq);
	return memclk_freq;
}


/**
 * @brief    set PLL register value.
 *
 * main clock is controlled by pll register.
 * @author    
 * @date     2010-04-06
 * @param    pll_value [in] pll register value(63-440)(M)
 * @return    bool
 * @retval    true  set pll frequency successful
 * @retval    false set pll frequency unsuccessful
  */
bool set_cpu_pll(unsigned long pll_value)
{
	if( (( inl( CPU_CLOCK_DIV_REG) & (1<<25) ) != 0)   ||   (( inl( CPU_CLOCK_DIV_REG) & (1<<26) ) != 0) )
	{
		akprintf(C1, M_DRVSYS, " Don't  set clk3x config at the same time.\n" );
		return false;
	}
	
	return clk_set_pll_freq(pll_value, CPU);
;
}

/**
 * @brief    get cpu frequency.
 *
 * cpu frequency can the same as asic frequency or 2X/3X of that
 * @author    
 * @date     2010-04-06
 * @return    unsigned long the cpu frequency
 */
unsigned long get_cpu_freq(void)
{
	unsigned long cpuclk_freq, ahbclk_freq, memclk_freq;
	clk_get_cpuclk_ahbclk_memclk_freq(&cpuclk_freq, &ahbclk_freq, &memclk_freq);
	return cpuclk_freq;
}


/**
 * @brief    set PERI PLL register value.
 *
 * main clock is controlled by pll register.
 * @author    
 * @date     2010-04-06
 * @param    pll_value [in] pll register value(63-600)(M)
 * @return    bool
 * @retval    true  set pll frequency successful
 * @retval    false set pll frequency unsuccessful
  */
bool set_peri_pll(unsigned long pll_value)
{
	return clk_set_pll_freq(pll_value, PERI);;
}


/**
 * @brief    set PERO register value.
 * @author    
 * @date     2010-04-06 
 * @return    unsigned long
 * @retval    PERI  frequency value
  */
unsigned long get_peri_pll(void)
{
	return get_pll(PERI); 
}
//check if cpu runs at 3x asic
bool is_cpu_3x(void)
{
    return (REG32(CPU_CLOCK_DIV_REG) & MARK_3X_CFG) ? (true) : (false);
}


//check if cpu runs at 2x asic
bool is_cpu_2x(void)
{
    if (true == is_cpu_3x()){
        return false;
    }
    
    return (REG32(CPU_CLOCK_DIV_REG) & CPU_2X_CFG) ? (true) : (false);
}


/**
 * @brief   set cpu PLL twice of cpu frequency or not
 *
 * this function just set cpu_clk = PLL1_clk 
 * @author  kjp
 * @date    2016-08-20
 * @param   set_value  set twice or not
 * @return  bool
 * @retval  true setting successful
 * @retval  false setting fail
 */

bool set_cpu_2x(bool set_value)
{
	if(set_value)
	{
		if( (( REG32(CPU_CLOCK_DIV_REG) & (1<<25) ) != 0) || (( REG32(CPU_CLOCK_DIV_REG) & (1<<26) ) != 0) )
		{
			akprintf(C1, M_DRVSYS, " Don't  set clk3x config at the same time.\n" );
			return false;
		}
		
		REG32(CPU_CLOCK_DIV_REG) |= (1<<24);
	}
	else
	{
		REG32(CPU_CLOCK_DIV_REG) &= (~(1<<24));
	}
	
	return true;
}


/**
 * @brief   judge whether cpu PLL is twice of cpu frequency or not
 *          
 * @author  kjp
 * @date    2016-08-20
 * @return  bool
 * @retval  true cpu pll is twice of CPU frequency
 * @retval  false cpu pll is not twice of CPU frequency
 */
bool get_cpu_2x(void)
{
	return is_cpu_2x();
}



/**
 * @brief        register call back function for freq change
 *          
 * @author      liaozhijun
 * @date         2010-04-06
 * @param      type callback function type, may be memory/uart/nand/sd/spi, and so on
 * @param      callback callback function 
 * @return      void
 */
void freq_register(E_FREQ_CALLBACK_TYPE type, T_fFREQ_CHANGE_CALLBACK callback)
{
#if 0
	if(E_MEMORY_CALLBACK == type)
    {
        m_mem_cbk = callback;
    }
    else if(E_UART_CALLBACK == type)
    {
        m_uart_cbk = callback;
    }
    else
    {
        if(index < MAX_FREQ_CALLBACK)
            m_asic_cbk[index++] = callback;
    }
#endif
	
}
