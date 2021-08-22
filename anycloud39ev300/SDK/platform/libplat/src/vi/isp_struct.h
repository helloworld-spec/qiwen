#ifndef _ISP_STRUCT_H_
#define _ISP_STRUCT_H_

#include "ak_isp_drv.h"

/********************** black balance ************************/
struct  ak_isp_init_blc
{
	unsigned short	param_id;     
	unsigned short	length;        
	AK_ISP_BLC_ATTR	blc; 
};

/********************* lsc *********************/
struct  ak_isp_init_lsc
{
	unsigned short	param_id;     
	unsigned short	length;        
	AK_ISP_LSC_ATTR	lsc;
};

/******************** raw gamma *************************/
struct ak_isp_init_raw_lut
{
	unsigned short		param_id;     
	unsigned short		length;       
	AK_ISP_RAW_LUT_ATTR	raw_lut;
};

/********************* nr **************************/
struct ak_isp_init_nr
{
	unsigned short			param_id;      
	unsigned short			length;        
	AK_ISP_NR1_ATTR			nr1;
	AK_ISP_NR2_ATTR			nr2;
};

/********************* 3dnr ************************/
struct ak_isp_init_3dnr
{
	unsigned short			param_id;      
	unsigned short			length;        
	AK_ISP_3D_NR_ATTR		nr_3d;
};


/******************** green balance **********************/
struct ak_isp_init_gb
{   
	unsigned short	param_id;      
	unsigned short	length;        
  	AK_ISP_GB_ATTR	gb;
};

/********************* demosaic *************************/
struct ak_isp_init_demo
{
	unsigned short		param_id;      
	unsigned short		length;        
	AK_ISP_DEMO_ATTR	demo;
};

/*********************** rgb gamma ****************************/
struct ak_isp_init_gamma
{
	unsigned short			param_id;     
	unsigned short			length;        
	AK_ISP_RGB_GAMMA_ATTR	gamma;
};

/***********************Y gamma****************************/
struct ak_isp_init_y_gamma
{
	unsigned short			param_id;
	unsigned short			length;
	AK_ISP_Y_GAMMA_ATTR		gamma;
};

/**********************ccm *************************/
struct  ak_isp_init_ccm
{
	unsigned short				param_id;      
	unsigned short				length;       
	AK_ISP_CCM_ATTR				ccm;

};


/******************** fcs ************************/
struct  ak_isp_init_fcs
{
	unsigned short	param_id;      
	unsigned short	length;        
	AK_ISP_FCS_ATTR	fcs;

};

/********************* wdr *************************/
struct ak_isp_init_wdr
{
	unsigned short		param_id;      
	unsigned short		length;        
	AK_ISP_WDR_ATTR		wdr;

};

/********************** sharp *************************/
struct ak_isp_init_sharp
{
	unsigned short			param_id;      
	unsigned short			length;        
	AK_ISP_SHARP_ATTR		sharp;
	AK_ISP_SHARP_EX_ATTR	sharp_ex;   
};

/********************* saturation **************************/
struct ak_isp_init_saturation
{
	unsigned short			param_id;      
	unsigned short			length;        
	AK_ISP_SATURATION_ATTR	saturation;
  
};

/************************  contrast ********************************/
struct  ak_isp_init_contrast
{
	unsigned short			param_id;      
	unsigned short			length;        
	AK_ISP_CONTRAST_ATTR	contrast;
};

/********************* rgb to yuv *****************************/
struct ak_isp_init_rgb2yuv
{
	unsigned short		param_id;     
	unsigned short		length;        
	AK_ISP_RGB2YUV_ATTR	rgb2yuv;
};

/************************* YUV effect ********************************/
struct ak_isp_init_effect
{
	unsigned short		param_id;      
	unsigned short		length;        
	AK_ISP_EFFECT_ATTR  effect;
};

/************************** dpc **********************************/
struct ak_isp_init_dpc
{
	unsigned short		param_id;      
	unsigned short		length;        
	AK_ISP_DDPC_ATTR	ddpc; 
	AK_ISP_SDPC_ATTR	sdpc; 

};

/***********************zone weight********************************/
struct ak_isp_init_weight
{   
	unsigned short		param_id;      
	unsigned short		length;        
	AK_ISP_WEIGHT_ATTR  weight;
};

/*************************** af *******************************/
struct ak_isp_init_af
{   
	unsigned short	param_id;      
	unsigned short	length;        
	AK_ISP_AF_ATTR  af;    
};

/************************* white balance **********************************/
struct ak_isp_init_wb
{   
	unsigned short			param_id;      
	unsigned short			length;        
	AK_ISP_WB_TYPE_ATTR		wb_type; 
	AK_ISP_MWB_ATTR			mwb;
	AK_ISP_AWB_ATTR			awb;
	AK_ISP_AWB_EX_ATTR		awb_ex;
};

/************************* expsoure ****************************/
struct ak_isp_init_exp
{
	unsigned short			param_id;      
	unsigned short			length;        

	AK_ISP_RAW_HIST_ATTR	raw_hist;
	AK_ISP_RGB_HIST_ATTR	rgb_hist;

	AK_ISP_YUV_HIST_ATTR	yuv_hist;
	AK_ISP_EXP_TYPE			exp_type;

	AK_ISP_FRAME_RATE_ATTR	frame_rate;
	AK_ISP_AE_ATTR			ae;
};

/************************* misc *****************************/
struct ak_isp_init_misc
{
	unsigned short		param_id;      
	unsigned short		length;        
	AK_ISP_MISC_ATTR	misc;
};

/*************************HUE*****************************/
struct ak_isp_init_hue
{
	unsigned short		param_id;
	unsigned short		length;
	AK_ISP_HUE_ATTR		hue;
};

/************************* sensor *****************************/
struct ak_isp_init_sensor
{
	unsigned short			param_id;      
	unsigned short			length;        
	AK_ISP_SENSOR_INIT_PARA	sensor;
};

/************************** the whole struct *******************************/
struct ak_isp_init_param
{
	//black balance
	struct  ak_isp_init_blc			isp_blc;
	//lsc
	struct  ak_isp_init_lsc			isp_lsc;
	//raw gamma
	struct ak_isp_init_raw_lut		isp_raw_lut;
	//nr
	struct ak_isp_init_nr			isp_nr;
	//3dnr
	struct ak_isp_init_3dnr			isp_3dnr;
	//green balance
	struct ak_isp_init_gb			isp_gb;
	//demosaic
	struct ak_isp_init_demo			isp_demo;
	//gamma
	struct ak_isp_init_gamma		isp_gamma;
	//ccm
	struct  ak_isp_init_ccm			isp_ccm;
	//fcs
	struct  ak_isp_init_fcs			isp_fcs;
	//wdr
	struct ak_isp_init_wdr			isp_wdr;
	//sharp
	struct ak_isp_init_sharp		isp_sharp;
	//saturation
	struct ak_isp_init_saturation	isp_saturation;
	//contrast
	struct  ak_isp_init_contrast	isp_contrast;
	//rgb to yuv
	struct ak_isp_init_rgb2yuv		isp_rgb2yuv;
	//yuv effect
	struct ak_isp_init_effect		isp_effect;
	//dpc
	struct ak_isp_init_dpc			isp_dpc;
	//zone weight
	struct ak_isp_init_weight		isp_weight;
	//af
	struct ak_isp_init_af			isp_af;
	//white balance
	struct ak_isp_init_wb			isp_wb;
	//expsoure
	struct ak_isp_init_exp			isp_exp;
	//misc
	struct ak_isp_init_misc			isp_misc;
	//y gamma
	struct ak_isp_init_y_gamma		isp_y_gamma;
	//hue
	struct ak_isp_init_hue			isp_hue;

	//sensor
	struct ak_isp_init_sensor      	isp_sensor;
};

#endif
