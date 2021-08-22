/** @file
* @brief Define the ISP structs
*
* Copyright (C) 2015 Anyka (GuangZhou) Software Technology Co., Ltd.
* @author
* @date 2015-07-06
* @version 1.0
*/

#ifndef __ISP_STRUCT_H__
#define __ISP_STRUCT_H__

#include "anyka_types.h"
#include "ak_isp_drv.h"




/**********************黑平衡************************/

typedef struct  ak_isp_init_blc
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_BLC_ATTR	p_blc; 
}AK_ISP_INIT_BLC;




/*********************镜头校正*********************/

typedef struct  ak_isp_init_lsc
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_LSC_ATTR	lsc;
}AK_ISP_INIT_LSC;


/********************raw gamma*************************/


typedef struct ak_isp_init_raw_lut
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_RAW_LUT_ATTR	raw_lut_p;
 }AK_ISP_INIT_RAW_LUT;



/*********************NR**************************/


typedef struct ak_isp_init_nr
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_NR1_ATTR			p_nr1;
	AK_ISP_NR2_ATTR			p_nr2;
}AK_ISP_INIT_NR;

/*********************3DNR************************/


typedef struct ak_isp_init_3dnr
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_3D_NR_ATTR		p_3d_nr;
}AK_ISP_INIT_3DNR;


/********************GB**********************/


typedef struct ak_isp_init_gb
{   
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
  	AK_ISP_GB_ATTR	p_gb;
}AK_ISP_INIT_GB;


/*********************Demo*************************/


typedef struct ak_isp_init_demo
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_DEMO_ATTR	p_demo_attr;
}AK_ISP_INIT_DEMO;


/***********************rgb gamma****************************/



typedef struct ak_isp_init_gamma
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_RGB_GAMMA_ATTR	p_gamma_attr;
}AK_ISP_INIT_GAMMA;


/***********************Y gamma****************************/



typedef struct ak_isp_init_y_gamma
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_Y_GAMMA_ATTR	p_gamma_attr;
}AK_ISP_INIT_Y_GAMMA;



/**********************CCM*************************/


typedef  struct  ak_isp_init_ccm
{
	T_U16						param_id;      //参数id
	T_U16						length;        //参数长度;
	AK_ISP_CCM_ATTR				p_ccm;
}AK_ISP_INIT_CCM;


/********************FCS************************/

typedef  struct  ak_isp_init_fcs
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_FCS_ATTR	p_fcs;

}AK_ISP_INIT_FCS;

/*********************Wdr*************************/


typedef struct ak_isp_init_wdr
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_WDR_ATTR		p_wdr_attr;

}AK_ISP_INIT_WDR;




/**********************SHARP*************************/

typedef struct ak_isp_init_sharp
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_SHARP_ATTR		p_sharp_attr;
	AK_ISP_SHARP_EX_ATTR	p_sharp_ex_attr;   
 }AK_ISP_INIT_SHARP;

/*********************饱和度**************************/

typedef struct ak_isp_init_saturation
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_SATURATION_ATTR	p_se_attr;
  
}AK_ISP_INIT_SATURATION;

/************************对比度********************************/

typedef struct  ak_isp_init_contrast
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_CONTRAST_ATTR	p_contrast;
}AK_ISP_INIT_CONTRAST;


/*********************rgb to yuv*****************************/

typedef  struct ak_isp_init_rgb2yuv
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_RGB2YUV_ATTR	p_rgb2yuv;
}AK_ISP_INIT_RGB2YUV;


/*************************YUV effect********************************/


typedef  struct ak_isp_init_effect
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_EFFECT_ATTR  p_isp_effect;
}AK_ISP_INIT_EFFECT;


/**************************坏点校正**********************************/


typedef  struct ak_isp_init_dpc
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_DDPC_ATTR	p_ddpc; 
	AK_ISP_SDPC_ATTR	p_sdpc; 

}AK_ISP_INIT_DPC;



/***********************zone weight********************************/


typedef struct ak_isp_init_weight
{   
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_WEIGHT_ATTR  p_weight;
}AK_ISP_INIT_WEIGHT;


/***************************AF*******************************/


typedef struct ak_isp_af
{   
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_AF_ATTR  p_af_attr;    
}AK_ISP_INIT_AF;


/*************************WB**********************************/

typedef struct ak_isp_init_wb
{   
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_WB_TYPE_ATTR			wb_type; 
	AK_ISP_MWB_ATTR			p_mwb;
	AK_ISP_AWB_ATTR			p_awb;
	AK_ISP_AWB_EX_ATTR	p_awb_ex;
}AK_ISP_INIT_WB;


/*************************Expsoure****************************/

typedef struct ak_isp_init_exp
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;

	AK_ISP_RAW_HIST_ATTR	p_raw_hist;
	AK_ISP_RGB_HIST_ATTR	p_rgb_hist;

	AK_ISP_YUV_HIST_ATTR	p_yuv_hist;
	AK_ISP_EXP_TYPE			p_exp_type;

	AK_ISP_FRAME_RATE_ATTR	p_frame_rate;
	AK_ISP_AE_ATTR			p_ae;
}AK_ISP_INIT_EXP;



/*************************杂项*****************************/


typedef struct ak_isp_init_misc
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_MISC_ATTR	p_misc;
}AK_ISP_INIT_MISC;



/*************************HUE*****************************/


typedef struct ak_isp_init_hue
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_HUE_ATTR		p_hue;
}AK_ISP_INIT_HUE;


/*************************sensor*****************************/

typedef struct ak_isp_init_sensor
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_SENSOR_INIT_PARA	p_sensor;
}AK_ISP_INIT_SENSOR;

/**************************总结构体*******************************/

typedef struct ak_isp_init_param
{
	//黑平衡结构体
	AK_ISP_INIT_BLC			p_Isp_blc;
	//镜头校正结构体
	AK_ISP_INIT_LSC			p_Isp_lsc;
	//raw gamma
	AK_ISP_INIT_RAW_LUT		p_Isp_raw_lut;
	//NR
	AK_ISP_INIT_NR			p_Isp_nr;
	//3DNR
	AK_ISP_INIT_3DNR		p_Isp_3dnr;
	//绿平衡
	AK_ISP_INIT_GB			p_Isp_gb;
	//demosaic
	AK_ISP_INIT_DEMO		p_Isp_demo;
	//gamma
	AK_ISP_INIT_GAMMA		p_Isp_gamma;
	//ccm
	AK_ISP_INIT_CCM			p_Isp_ccm;
	//fcs
	AK_ISP_INIT_FCS			p_Isp_fcs;
	//wdr
	AK_ISP_INIT_WDR			p_Isp_wdr;
	//sharp
	AK_ISP_INIT_SHARP		p_Isp_sharp;
	//饱和度
	AK_ISP_INIT_SATURATION	p_Isp_saturation;
	//对比度
	AK_ISP_INIT_CONTRAST	p_Isp_contrast;
	//rgb to yuv
	AK_ISP_INIT_RGB2YUV		p_Isp_rgb2yuv;
	//yuv 效果
	AK_ISP_INIT_EFFECT		p_Isp_effect;
	//坏点校正
	AK_ISP_INIT_DPC			p_Isp_dpc;
	//zone weight
	AK_ISP_INIT_WEIGHT		p_Isp_weight;
	//af
	AK_ISP_INIT_AF			p_Isp_af;
	//白平衡
	AK_ISP_INIT_WB			p_Isp_wb;
	//expsoure
	AK_ISP_INIT_EXP			p_Isp_exp;
	//杂项
	AK_ISP_INIT_MISC		p_Isp_misc;
	//y gamma
	AK_ISP_INIT_Y_GAMMA		p_Isp_y_gamma;
	//hue
	AK_ISP_INIT_HUE			p_Isp_hue;

	//sensor
	AK_ISP_INIT_SENSOR      p_Isp_sensor;

}AK_ISP_INIT_PARAM;


#endif

