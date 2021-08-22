/**
 * @file dac.c
 * @brief the source code of DA controller
 * This file is the source code of DA controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LianGenhui
 * @date 2010-07-30
 * @version 1.0
 */

#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "hal_sound.h"
#include "dac.h"
#include "sysctl.h"

#define MAX_DACCLK   14000000
#define MAX_DAC_CLK		24000000		//24Mhz


#define HANDLE_SPR_MIN_CYCLE	700
static bool enable_aec_flag = false;//执行回音消除的标志

//osr table
static const unsigned long dac_OSR_table[8] = {256, 272, 264, 248, 240, 136, 128, 120};


// DAC configuration parameter
typedef struct _DAC_CFG_PARA
{
	unsigned long dac_clk_div;
	unsigned long dac_hsclk_div;
	unsigned long osr_div;
}T_DAC_CFG_PARA;

extern unsigned long get_asic_pll(void);


static unsigned long dac_calculate_para(T_DAC_CFG_PARA *p_dac_para, unsigned long sr)
{
	unsigned long core_pll_freq;
	unsigned long dac_clk_div;
	unsigned long dac_clk_freq;
	unsigned long osr_index;
	unsigned long divisor;
	unsigned long cal_spr;
	unsigned long diff;
	
	unsigned long final_diff = 0xffffffff;		//离最终要设定的采样率差多少

	unsigned long final_dac_clk_div;			//最后计算出来的dac clk div
	unsigned long final_dac_hsclk_div;		//最后计算出来的dac hsclk div
	unsigned long final_osr_index;			//最后计算出来的osr index
	unsigned long final_dac_clk;				//最后计算出来的dac clk freq
	unsigned long final_dac_hclk;
	
	
	//get the core pll frequence
	core_pll_freq = get_asic_pll();

	//handle one sample data must use about 700 hsclk cycle , so dac_hsclk_freq must >= 700*des_sr
	//because max dac_hsclk_freq is core_pll_freq
	if(  (HANDLE_SPR_MIN_CYCLE * sr) > core_pll_freq )
	{
		printf("Handle one sample data must use about 700 hsclk cycle ,  so dac_hsclk_freq must >= 700*des_sr !\n");
		return false;
	}	

	//hsclk constrain 100MHZ by chip design
	if(  (HANDLE_SPR_MIN_CYCLE * sr) > 100000000 )
	{
		printf("Hsclk constrain to 100MHZ. \n");
		printf("So, the sample rate must not bigger than %d\n", 100000000/HANDLE_SPR_MIN_CYCLE);
		return false;
	}	
	
/*	not need because the above code
	//if sample rate > 192KHZ
	if( spr > 192000)
	{
		printf("Bigger than 192KHz sample rate not support!\n");
		return AK_FALSE;
	}	
*/	

	//calculate the minimal value
	for( dac_clk_div = 0; dac_clk_div < 0x100; dac_clk_div++)
	{
		dac_clk_freq = core_pll_freq/(dac_clk_div+1);
		
		//dac_clk freq must < 24MHZ
		if( dac_clk_freq > MAX_DAC_CLK)
		{
			continue;
		}
		for( osr_index = 0; osr_index < 8; osr_index++)
		{
			//calculate the sample rate
			divisor = (dac_clk_div+1) * dac_OSR_table[osr_index];
			cal_spr= core_pll_freq/divisor;

			//compare 
			diff = (cal_spr>sr) ? (cal_spr-sr) : ( sr-cal_spr);

			//keep the min	value
			if( diff < final_diff)
			{
				final_diff = diff;
				final_dac_clk_div = dac_clk_div;
				final_dac_clk = dac_clk_freq;
				final_osr_index = osr_index;	
				if(final_diff == 0)
				{
					goto dac_calculate_para_return;
				}
			}
		}
	}


dac_calculate_para_return:
	
	printf("core pll clk is %d\n", core_pll_freq);
	printf("The final calculate dac clk is %d,	dac div is %d.\n", final_dac_clk, final_dac_clk_div);
	printf("The final calculate osr is %d, osr index is %d.\n", dac_OSR_table[final_osr_index], final_osr_index);
	
	final_dac_hsclk_div= core_pll_freq /(HANDLE_SPR_MIN_CYCLE * sr)  - 1;

	printf("dac hsclk must > %d,  dac hsclk div recommend %d\n", (HANDLE_SPR_MIN_CYCLE * sr), final_dac_hsclk_div);

	final_dac_hclk = core_pll_freq /(final_dac_hsclk_div + 1);

	printf("The final calculate dac hclk is %d.\n", final_dac_hclk);
	
	printf("The final set sample rate different is %d\n", final_diff);


	p_dac_para->dac_clk_div = final_dac_clk_div;
	p_dac_para->dac_hsclk_div = final_dac_hsclk_div;
	p_dac_para->osr_div = final_osr_index;
	
	return true;
	
}


static void dac_config_dac_clk_div(unsigned long div, unsigned long div_frac)
{
	//close the dac_filter_en_cfg
	REG32(DAC_CFG_REG) &= (~(1<<3));

	//reset dac from dac clk
	REG32(RESET_CTRL_REG) &= (~(1<<26));

	//wait the sddac_div_vld_cfg  clear
	while( (REG32(ADC_CLK_DIV) & (1<<29)) !=0 )
	{;}
	
	//disable the sd_dac_clk
	REG32(ADC_CLK_DIV) &= (~(1<<28));

	//change the div val
	REG32(ADC_CLK_DIV) &= (~(0xfffff<<4));
	REG32(ADC_CLK_DIV) |= (div<<4);
	REG32(ADC_CLK_DIV) |= (div_frac<<12);

	// set sddac_div_vld_cfg
	REG32(ADC_CLK_DIV) |= (1<<29);	

	//wait the sddac_div_vld_cfg  clear
	while( (REG32(ADC_CLK_DIV) & (1<<29)) !=0 )
	{;}

	//release the reset dac from dac clk
	REG32(RESET_CTRL_REG) |= (1<<26);
	

}
/**
* @BRIEF   dac filter close
* @AUTHOR Jiankui
* @DATE   2016-09 -01
* @PARAM   
* @PARAM  
* @RETURN   
* @NOTE: 
*/
static void dac_filter_close()
{
	//disable the internal dac I2S master mode
	REG32( USB_I2S_CTRL_REG )  &= (~(0x1<<25));
	
	//close dac clk gate
	REG32(ADC_CLK_DIV) &= (~DAC_CLK_EN);

	//close dac hsclk gate
	REG32(ADC_HCLK_DIV) &= (~DAC_HCLK_EN);

	//close the dac filter
	REG32(DAC_CFG_REG) &= (~(0x1<<3));
	
}
void dac_config_dac_hsclk_div(unsigned long div)
{
	//close the dac_filter_en_cfg
	REG32(DAC_CFG_REG) &= (~(1<<3));

	//reset dac from dac hsclk
	REG32(RESET_CTRL_REG) &= (~(1<<28));

	//wait the sddac_hsdiv_vld_cfg	clear
	while( (REG32(ADC_HCLK_DIV) & (1<<19)) !=0 )
	{;}
	
	//disable the sd_dac_hsclk
	REG32(ADC_HCLK_DIV) &= (~(1<<18));

	//change the div val
	REG32(ADC_HCLK_DIV) &= (~(0xff<<10));
	REG32(ADC_HCLK_DIV) |= (div<<10);

	// set sddac_hsdiv_vld_cfg
	REG32(ADC_HCLK_DIV) |= (1<<19);	

	//wait the sddac_div_vld_cfg  clear
	while( (REG32(ADC_HCLK_DIV) & (1<<19)) !=0 )
	{;}
	
	//release dac from dac hsclk
	REG32(RESET_CTRL_REG) |= (1<<28);
	
}


bool dac_filter_open()
{
	
	//reset dac filter  which use dac hsclk
	REG32(RESET_CTRL_REG) &= (~((1<<26) |(1<<28)));  //must reset dac filter clk at the same time
	REG32(RESET_CTRL_REG) |= ((1<<26) |(1<<28));
	
	//reset dac filter which use dac clk
	REG32(RESET_CTRL_REG) &= (~((1<<26) | (0x1<<28)));	//must reset dac filter hclk at the same time
	REG32(RESET_CTRL_REG) |= ((1<<26) | (0x1<<28));

	//enable the internal dac
	REG32( USB_I2S_CTRL_REG )  &= (~(0x7<<25));
	REG32( USB_I2S_CTRL_REG )  |= (0x1<<25);

	T_DAC_CFG_PARA dac_para;

	//calculate the dac_clk_div,  dac_hsclk_div, osr_div
	if(dac_calculate_para(&dac_para, 44100) == false)
	{
		printf("dac parameter config error!\n");
		return false;
	}
	
	//close the dac clk and disable dac filter 
	//config dac_clk_div
	dac_config_dac_clk_div(dac_para.dac_clk_div, 0);

	//close the dac hsclk and disable dac filter 
	//config dac_hsclk_div
	dac_config_dac_hsclk_div(dac_para.dac_hsclk_div);



	//config osr_div
	REG32(DAC_CFG_REG) &= (~7);
	REG32(DAC_CFG_REG) |= dac_para.osr_div;

	
	//open dac clk gate
	REG32(ADC_CLK_DIV) |= DAC_CLK_EN;

	//open dac hsclk gate
	REG32(ADC_HCLK_DIV) |= DAC_HCLK_EN;

	//open the dac filter
	REG32(DAC_CFG_REG) |= DAC_EN;	
}

/**
 * @brief  open a dac device 
 * @author LianGenhui
 * @date   2010-07-30
 * @return bool
 * @retval true  open successful
 * @retval false open failed
 */
bool dac_open(void)
{
    unsigned long reg_value;    

    // soft reset DAC
    sysctl_reset(RESET_DAC);

    //enable DAC clock
    sysctl_clock(CLOCK_DAC_ENABLE);

	REG32(DAC_CONFIG_REG) |= MUTE;
    
	//config the dac word length ,
	REG32(DAC_I2S_CFG_REG) &= (~0x1F);
	REG32(DAC_I2S_CFG_REG) |= 0x10; //bit_width,set 16bit		

	//config the I2S  polarity , send the data of left channel when I2S_BCLK is low.
	REG32(DAC_I2S_CFG_REG) &= (~0x1<<5);
	
	//set dac controller :  memory saving format,   not generate ARM interrupt, not mute,  DAC accept from L2
	REG32(DAC_CONFIG_REG) = L2_EN | FORMAT;
	
	// not mute and disable interrupt
	REG32(DAC_CONFIG_REG) &= ~(MUTE | ARM_INT);

	//enable the dac controller 
	REG32(DAC_CONFIG_REG) |= DAC_CTRL_EN;

	if(dac_filter_open() == false)
	{
		return false;
	}
	else
	{
    	return true;
	}
}

/**
 * @brief   Close a dac device
 * @author  LianGenhui
 * @date    2010-07-30
 * @return  bool
 * @retval  true close successful
 * @retval  false close failed
 */
bool dac_close(void)
{    
    // to power off DACs/DAC clock, 
    REG32(AUDIO_CODEC_CFG1_REG) |= PD_OP | PD_CK;        
    REG32(AUDIO_CODEC_CFG1_REG) |= RST_DAC_MID;  

	REG32(DAC_CONFIG_REG) &= (~L2_EN);
	
    //mute: send the same data as that sent in last clock cycle
	REG32(DAC_CONFIG_REG) |= MUTE;

	//disable the dac controller 
	REG32(DAC_CONFIG_REG) &= ~DAC_CTRL_EN;
	//close DAC control module clock
	REG32(CLOCK_CTRL_REG) |= (1<<4);
	//close dac filter
	dac_filter_close();
    enable_aec_flag = false;
    return true;
}

static unsigned long dac_aec_calculate_para(T_DAC_CFG_PARA *p_dac_para, unsigned long spr)
{
	
	unsigned long cal_spr;
	unsigned long final_dac_clk_div;			//最后计算出来的dac clk div
	unsigned long final_dac_hsclk_div;		//最后计算出来的dac hsclk div
	unsigned long final_osr_index;			//最后计算出来的osr index
	unsigned long final_dac_clk;				//最后计算出来的dac clk freq
	unsigned long final_dac_hclk;

	unsigned long core_pll_freq;	
	unsigned long divisor;
	
	//get the core pll frequence
	core_pll_freq = get_asic_pll();

	//handle one sample data must use about 700 hsclk cycle , so dac_hsclk_freq must >= 700*des_sr
	//because max dac_hsclk_freq is core_pll_freq
	if(  (HANDLE_SPR_MIN_CYCLE * spr) > core_pll_freq )
	{
		printf("Handle one sample data must use about 700 hsclk cycle ,  so dac_hsclk_freq must >= 700*des_sr !\n");
		return false;
	}	

	//hsclk constrain 100MHZ by chip design
	if(  (HANDLE_SPR_MIN_CYCLE * spr) > 100000000 )
	{
		printf("Hsclk constrain to 100MHZ. \n");
		printf("So, the sample rate must not bigger than %d\n", 100000000/HANDLE_SPR_MIN_CYCLE);
		return false;
	}
	/*
	8012是8k采样率adc计算的真实采样率，dac_OSR_table[0] = 256，选择256是为了
	使计算与adc计算的相等。
	16108是16k采样率adc计算的真实采样率。

	*/
	if(spr == 8000)
	{
		final_dac_clk = 8012 * dac_OSR_table[0];//dac_clk = dac_sample_frequency * osr(oversample rate)
	}
	else if(spr == 16000)
	{
		final_dac_clk = 16108 * dac_OSR_table[0];//dac_clk = dac_sample_frequency * osr(oversample rate)
	}
	//dac_clk = asic_pll_clk/(dac_div + 1)
	final_dac_clk_div = (core_pll_freq/final_dac_clk) - 1;
	final_osr_index = 0;	

	//calculate the sample rate
	divisor = (final_dac_clk_div+1) *	dac_OSR_table[final_osr_index];
	cal_spr= core_pll_freq/divisor;

	
dac_calculate_para_return:
		
	printf("core pll clk is %d\n", core_pll_freq);
	printf("The final calculate dac clk is %d,	dac div is %d.\n", final_dac_clk, final_dac_clk_div);
	printf("The final calculate osr is %d, osr index is %d.\n", dac_OSR_table[final_osr_index], final_osr_index);
	
	final_dac_hsclk_div= core_pll_freq /(HANDLE_SPR_MIN_CYCLE * spr)  - 1;

	printf("dac hsclk must > %d,  dac hsclk div recommend %d\n", (HANDLE_SPR_MIN_CYCLE * spr), final_dac_hsclk_div);

	final_dac_hclk = core_pll_freq /(final_dac_hsclk_div + 1);

	printf("The final calculate dac hclk is %d.\n", final_dac_hclk);
	
	printf("The real sample rate  is %d\n", cal_spr);


	p_dac_para->dac_clk_div = final_dac_clk_div;
	p_dac_para->dac_hsclk_div = final_dac_hsclk_div;
	p_dac_para->osr_div = final_osr_index;


	return true;
}

bool dac_setinfo_select(bool option)
{
	enable_aec_flag = option;
	return true;
}

/**
 * @brief   Set sound sample rate, channel, bits per sample of the sound device
 * @author  Jiankui
 * @date    2016-08-29
 * @param[in] info     refer to SOUND_INFO
 * @return  bool
 * @retval  true set successful
 * @retval  false set failed
 */
bool dac_setinfo(SOUND_INFO *info)
{

	T_DAC_CFG_PARA dac_para;
	if(enable_aec_flag)//执行回音消除,调用这个接口，为了实现跟adc的真实采样率一样
	{
		//calculate the dac_clk_div,  dac_hsclk_div, osr_div
		if(dac_aec_calculate_para(&dac_para, info->nSampleRate) == false)
		{
			printf("dac parameter config error!\n");
			return false;
		}
	}
	else
	{	
		//calculate the dac_clk_div,  dac_hsclk_div, osr_div
		if(dac_calculate_para(&dac_para, info->nSampleRate) == false)
		{
			printf("dac parameter config error!\n");
			return false;
		}

	}
	//close the dac clk and disable dac filter 
	//config dac_clk_div
	dac_config_dac_clk_div(dac_para.dac_clk_div, 0);

	//close the dac hsclk and disable dac filter 
	//config dac_hsclk_div
	dac_config_dac_hsclk_div(dac_para.dac_hsclk_div);



	//config osr_div
	REG32(DAC_CFG_REG) &= (~7);
	REG32(DAC_CFG_REG) |= dac_para.osr_div;
	
	//config the dac word length ,
	REG32(DAC_I2S_CFG_REG) &= (~0x1F);
	REG32(DAC_I2S_CFG_REG) |= info->BitsPerSample; //bit_width,set 16bit	

	//open dac clk gate
	REG32(ADC_CLK_DIV) |= DAC_CLK_EN;

	//open dac hsclk gate
	REG32(ADC_HCLK_DIV) |= DAC_HCLK_EN;
	

	//open the dac filter
	REG32(DAC_CFG_REG) |= DAC_EN;

    return true;
}


