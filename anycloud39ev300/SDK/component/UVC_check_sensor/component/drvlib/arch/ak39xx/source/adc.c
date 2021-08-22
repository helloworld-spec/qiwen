/**
 * @FILENAME: adc.c
 * @BRIEF adc driver file
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-30
 * @VERSION 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "l2.h"
#include "adc.h"
#include "dac.h"
#include "interrupt.h"
#include "hal_sound.h"
#include "sysctl.h"


static const unsigned long adc_OSR_table[2] = {256, 512};


// ADC configuration parameter
typedef struct _ADC_CFG_PARA
{
	unsigned long adc_clk_div;
	unsigned long adc_hsclk_div;
	unsigned long osr_val;
}T_ADC_CFG_PARA;



static unsigned long adc_calculate_para(T_ADC_CFG_PARA *p_adc_para, SOUND_INFO *info)
{
	unsigned long core_pll_freq;
	unsigned long i;
	unsigned long osr_index;
	unsigned long ideal_adc_clk;
	unsigned long real_adc_clk;
	unsigned long diff;
	unsigned long min_diff;
	unsigned long clk_div;

	unsigned long final_ideal_adc_clk;
	unsigned long final_osr_index;			//最后计算出来的osr_index
	unsigned long final_adc_clk_div;		//最后计算出来的adc clk div
	unsigned long final_adc_clk;			//最后计算出来的adc clk freq
	
	unsigned long final_adc_hsclk_div;		//最后计算出来的adc hsclk div
	unsigned long final_adc_hsclk;			//最后计算出来的adc hsclk freq

	unsigned long true_samplerate;
	

	if( info->nSampleRate > 96000)
	{
		printf("Not support adc sample rate bigger than 48000.\n");
		return false;
	}

	core_pll_freq = get_asic_pll();

	min_diff = 0xffffffff;

	for(osr_index = 0; osr_index < 2; osr_index++)
	{
		//理想的ADC CLK
		ideal_adc_clk =adc_OSR_table[osr_index] * (info->nSampleRate);

		//计算adc clk div
		clk_div = core_pll_freq/ideal_adc_clk;

		// clk_div 如果小于0, 则代表CORE PLL 时钟太低了
		if(clk_div == 0)
		{
			printf("Error, the core pll frequence is too low to calculate the adc clk div.\n");
			return false;
		}

		// 真实计算出来的ADC CLK
		real_adc_clk = core_pll_freq / clk_div;

		// 计算差值
		diff = (real_adc_clk > ideal_adc_clk) ? (real_adc_clk - ideal_adc_clk) : ( ideal_adc_clk - real_adc_clk );
		//printf("diff is %d\n", diff);

		// 保留最小的差值和记录最小差值对应的adc clk div
		if( diff < min_diff)
		{
			final_osr_index = osr_index;
			final_ideal_adc_clk = ideal_adc_clk;
			min_diff = diff;
			final_adc_clk = real_adc_clk;
			final_adc_clk_div = clk_div -1;
		}
	}
	printf("core pll clk is %ld\n", core_pll_freq);
	printf("osr is %ld\n", adc_OSR_table[final_osr_index]);
	printf("ideal clk is %ld\n", final_ideal_adc_clk);
	printf("adc clk is %ld,  adc clk div is %ld.\n", final_adc_clk, final_adc_clk_div);
	printf("Adc clk ideal and real difference is %ld\n", min_diff);
	

	true_samplerate = core_pll_freq/((final_adc_clk_div+1) * adc_OSR_table[final_osr_index]);
	printf("real samplerate is %ld\n", true_samplerate);
	info->nSampleRate = true_samplerate;
	//fix the hsclk to 60MHZ
	final_adc_hsclk_div =  core_pll_freq /90000000 ;
		
	if( final_adc_hsclk_div == 0 )
	{
		printf("Error, the core pll frequence is too low to calculate the adc hsclk div.\n");
		return false;
	}
	
	final_adc_hsclk_div = final_adc_hsclk_div - 1;
	
	//calculate the real hsclk
	final_adc_hsclk = core_pll_freq/(final_adc_hsclk_div+1);
	printf("adc hsclk is %ld,  adc hsclk div is %ld.\n", final_adc_hsclk, final_adc_hsclk_div);	
		
	//	osr register value
	if( adc_OSR_table[final_osr_index] == 256)
	{
		p_adc_para->osr_val = 1;
	}
	else if( adc_OSR_table[final_osr_index] == 512)
	{
		p_adc_para->osr_val = 0;
	}
	
				
	//return the div and osr_val
	p_adc_para->adc_clk_div = final_adc_clk_div;
	p_adc_para->adc_hsclk_div = final_adc_hsclk_div;
	
	return true;
	
}

//ADC inner module
typedef enum _ADC_INNER_MODULE_TYPE
{
	ADC_CONTROLLER = 0,
	ADC_FILTER_WITH_CLK,
	ADC_FILTER_WITH_HSCLK,
	ADC_ANALOG,
	ADC_ALL_MODULE
}T_ADC_INNER_MODULE;




static void adc_reset(T_ADC_INNER_MODULE module_type)
{
	switch(module_type)
	{
		case ADC_CONTROLLER:								//reset adc controller
			REG32(RESET_CTRL_REG) |= (1<<3);
			REG32(RESET_CTRL_REG) &= (~(1<<3));
			break;
			
		case  ADC_FILTER_WITH_CLK:							//reset adc fillter witch use adc clk
			REG32(RESET_CTRL_REG) &= (~(1<<27));
			REG32(RESET_CTRL_REG) |= (1<<27);
			break;

		case  ADC_FILTER_WITH_HSCLK:						//reset adc hsfillter witch use adc hsclk
			REG32(RESET_CTRL_REG) &= (~(1<<29));
			REG32(RESET_CTRL_REG) |= (1<<29);
			break;
			
		case  ADC_ALL_MODULE:
			//reset adc controller
			REG32(RESET_CTRL_REG) |= (1<<3);
			REG32(RESET_CTRL_REG) &= (~(1<<3));

			//reset adc fillter witch use adc clk
			REG32(RESET_CTRL_REG) &= (~(1<<27));
			REG32(RESET_CTRL_REG) |= (1<<27);

			//reset adc hsfillter witch use adc hsclk
			REG32(RESET_CTRL_REG) &= (~(1<<29));
			REG32(RESET_CTRL_REG) |= (1<<29);

			break;
			
		defalut:
			printf("The module type not exist in adc_reset function!\n");
			break;
	}
}


static void adc_clk_gate_open(T_ADC_INNER_MODULE module_type)
{
	switch(module_type)
	{
		case ADC_CONTROLLER:								//open adc controller gate
			REG32(CLOCK_CTRL_REG) &= (~(1<<3));
			break;
			
		case  ADC_FILTER_WITH_CLK:							//open adc fillter clk gate 
			REG32(ADC_HCLK_DIV) |= (1<<8);
			break;

		case  ADC_FILTER_WITH_HSCLK:						//open adc fillter hsclk gate
			REG32(ADC_HCLK_DIV) |= (1<<28);
			break;

		case  ADC_ALL_MODULE:
			//open adc controller gate
			REG32(CLOCK_CTRL_REG) &= (~(1<<3));

			//open adc fillter clk gate 
			REG32(ADC_HCLK_DIV) |= (1<<8);

			//open adc fillter hsclk gate
			REG32(ADC_HCLK_DIV) |= (1<<28);

			break;
			
		defalut:
			printf("The module type not exist in adc_clk_gate_open function!\n");
			break;
	}

}

static void adc_controller_open()
{
	//reset adc controller
	adc_reset(ADC_CONTROLLER);

	//open adc controller clk gate
	adc_clk_gate_open(ADC_CONTROLLER);

	//enable inner adc
	REG32(USB_I2S_CTRL_REG) &= (~(1<<27));

	//enable adc controller
	REG32(ADC2_CONFIG_REG) |= (1<<0);	

	//enable L2 for adc
	REG32(ADC2_CONFIG_REG) |= (1<<1);	

	//disable adc arm read interrupt
	REG32(ADC2_CONFIG_REG) &= (~(1<<2));

	//set channel polarity selection, receive left channel data when LRCK is high 
	REG32(ADC2_CONFIG_REG) |= (1<<3);
	
	//use inner adc mode
	REG32(ADC2_CONFIG_REG) &= (~(1<<4));

	// 16bit data width
	REG32(ADC2_CONFIG_REG) |= (0xf<<8);


	
}

static void adc_filter_open()
{
	//reset adc filter which use clk 
	adc_reset(ADC_FILTER_WITH_CLK);

	//reset adc filter which use hsclk 
	adc_reset(ADC_FILTER_WITH_HSCLK);

	//open adc filter clk gate
	adc_clk_gate_open(ADC_FILTER_WITH_CLK);

	//open adc filter hsclk gate
	adc_clk_gate_open(ADC_FILTER_WITH_HSCLK);


}


/**
 * @brief  open a adc device 
 * @author LianGenhui
 * @date   2010-07-28
 * @return bool
 * @retval true  open successful
 * @retval false open failed
 */
bool adc_open(void)
{

	REG32(AUDIO_CODEC_CFG1_REG) &= (~(0x1f<<6));
	
	//enable limit ADC input between vcm3 to 0
	REG32(AUDIO_CODEC_CFG2_REG) |= (1<<28); 
	
	//power on adc conversion
	REG32(AUDIO_CODEC_CFG2_REG)  &= (~(1<<27));
	
	//power on adc
	REG32( AUDIO_CODEC_CFG2_REG) &= (~(0x1<<26));

		
	//power on vcm3
	REG32(AUDIO_CODEC_CFG1_REG) &= (~(1<<1));
				
	//disable vcm3 pull down with 2Kohm registor to ground
	REG32(AUDIO_CODEC_CFG1_REG) &= (~(1<<2));	
			
	//power on bias generator
	REG32( AUDIO_CODEC_CFG1_REG) &= (~(0x1<<0));

	adc_controller_open();
	adc_filter_open();
	
    return true;
}

/**
 * @brief   Close a adc device
 * @author  LianGenhui
 * @date    2010-07-28
 * @return  bool
 * @retval  true close successful
 * @retval  false close failed
 */
bool adc_close(void)
{


	//reset adc controller
	REG32(RESET_CTRL_REG) |= (1<<3);
	REG32(RESET_CTRL_REG) &= (~(1<<3));
	//close adc controller clk gate
	REG32(CLOCK_CTRL_REG) |= (1<<3);
	//disable inner adc
	REG32(USB_I2S_CTRL_REG) |= (1<<27);
	//disable adc controller
	REG32(ADC2_CONFIG_REG) &= (~(1<<0));	

	//disable L2 for adc
	REG32(ADC2_CONFIG_REG) &= (~(1<<1));	

	//disable adc arm read interrupt
	REG32(ADC2_CONFIG_REG) &= (~(1<<2));

	//power down vcm3
	REG32(AUDIO_CODEC_CFG1_REG) |= (1<<1);
	
	//power down bias generator
	REG32( AUDIO_CODEC_CFG1_REG) |= (0x1<<0);

	//power off adc conversion
	REG32(AUDIO_CODEC_CFG2_REG)  |= (1<<27);
	
    return true;
}

static void adc_clk_gate_close(T_ADC_INNER_MODULE module_type)
{
	switch(module_type)
	{
		case ADC_CONTROLLER:								//close adc controller gate
			REG32(CLOCK_CTRL_REG) |= (1<<3);
			break;
			
		case  ADC_FILTER_WITH_CLK:							//close adc fillter clk gate 
			REG32(ADC_HCLK_DIV) &= (~(1<<8));
			break;

		case  ADC_FILTER_WITH_HSCLK:						//close adc fillter hsclk gate
			REG32(ADC_HCLK_DIV) &= (~(1<<28));
			break;

		case  ADC_ALL_MODULE:
			//close adc controller gate
			REG32(CLOCK_CTRL_REG) |= (1<<3);				

			//close adc fillter clk gate 
			REG32(ADC_HCLK_DIV) &= (~(1<<8));				

			//close adc fillter hsclk gate
			REG32(ADC_HCLK_DIV) &= (~(1<<28));			

			break;
			
		defalut:
			printf("The module type not exist in adc_clk_gate_close function!\n");
			break;
	}


}



static void adc_config_adc_clk_div(unsigned long div)
{
	
	//reset adc from adc clk
	REG32(RESET_CTRL_REG) &= (~(1<<27));

	//wait the sdadc_div_vld_cfg  clear
	while( (REG32(ADC_HCLK_DIV) & (1<<9)) !=0 )
	{;}
	
	//close the adc clk enable
	adc_clk_gate_close(ADC_FILTER_WITH_CLK);

	//change the div val
	REG32(ADC_HCLK_DIV) &= (~0xff);
	REG32(ADC_HCLK_DIV) |= div;

	// set sdadc_div_vld_cfg
	REG32(ADC_HCLK_DIV) |= (1<<9);	

	//wait the sddac_div_vld_cfg  clear
	while( (REG32(ADC_HCLK_DIV) & (1<<9)) !=0 )
	{;}

	//release the reset adc from adc clk
	REG32(RESET_CTRL_REG) |= (1<<27);
	
}

static void adc_config_adc_hsclk_div(unsigned long div)
{

	//reset adc from adc hsclk
	REG32(RESET_CTRL_REG) &= (~(1<<29));

	//wait the sdadc_hsdiv_vld_cfg  clear
	while( (REG32(ADC_HCLK_DIV) & (1<<29)) !=0 )
	{;}
	
	//disable the sd_dac_hsclk
	adc_clk_gate_close(ADC_FILTER_WITH_HSCLK);

	//change the div val
	REG32(ADC_HCLK_DIV) &= (~(0xff<<20));
	REG32(ADC_HCLK_DIV) |= (div<<20);

	// set sddac_hsdiv_vld_cfg
	REG32(ADC_HCLK_DIV) |= (1<<29);	

	//wait the sdadc_div_vld_cfg  clear
	while( (REG32(ADC_HCLK_DIV) & (1<<29)) !=0 )
	{;}

	//release adc from adc hsclk
	REG32(RESET_CTRL_REG) |= (1<<29);
	
}



/**
 * @brief   Set adc sample rate, channel, bits per sample of the sound device
 * @author  LianGenhui
 * @date    2010-07-28
 * @param[in] info     refer to SOUND_INFO
 * @return  bool
 * @retval  true set successful
 * @retval  false set failed
 */
bool adc_setinfo(SOUND_INFO *info)
{

	T_ADC_CFG_PARA adc_cfg_para;


	//calculate the adc sample rate
	if( adc_calculate_para(&adc_cfg_para, info) == false)
	{
		return false;
	}

	//config the osr value
	REG32(DAC_CFG_REG) &= (~(1UL<<31));
	REG32(DAC_CFG_REG) |= (adc_cfg_para.osr_val<<31);

	//config the adc clk
	adc_config_adc_clk_div(adc_cfg_para.adc_clk_div);

	//config the adc hsclk
	adc_config_adc_hsclk_div(adc_cfg_para.adc_hsclk_div);

	//open adc clk gate 
	adc_clk_gate_open(ADC_FILTER_WITH_CLK);

	//ope adc hsclk gate
	adc_clk_gate_open(ADC_FILTER_WITH_HSCLK);


	
    return true;
}




