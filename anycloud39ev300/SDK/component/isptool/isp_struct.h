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


#define ISP_MODULE_ID_SIZE		2
#define ISP_MODULE_LEN_SIZE		2

#define ISP_MODULE_HEAD_SIZE	(ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE)

#define SUBFILE_NUM_MAX		5

typedef struct cfgfile_headinfo               
{
	T_U32	main_version;
	char	file_version[16];
	
	T_U32	sensorId;
	
    T_U16	year;
    T_U8	month;
    T_U8	day;
    T_U8	hour;
    T_U8	minute;
    T_U8	second;
	T_U8	subFileId;

	T_U8	styleId;
	T_U8	reserve1;
	T_U16	reserve2;

	T_U32	subfilelen;
	
	T_U8	reserve3[88];
	
	char	notes[384];
}CFGFILE_HEADINFO;


/**********************黑平衡************************/
typedef struct ak_isp_blc
{
    T_U16	black_level_enable;  //使能位   
    T_U16	bl_r_a;            //[ 0,1023]
	T_U16	bl_gr_a;           //[ 0,1023]
	T_U16	bl_gb_a;           //[ 0,1023]
	T_U16	bl_b_a;            //[ 0,1023]
	T_S16	bl_r_offset;         // [-2048,2047]
	T_S16	bl_gr_offset;        // [-2048,2047]
	T_S16	bl_gb_offset;        //[-2048,2047]
	T_S16	bl_b_offset;         /// [-2048,2047]
}AK_ISP_BLC;

typedef struct ak_isp_blc_attr
{
	T_U16		blc_mode;             //0联动模式，1手动模式
	AK_ISP_BLC	m_blc;
	AK_ISP_BLC	linkage_blc[9]; 
}AK_ISP_BLC_ATTR;

typedef struct  ak_isp_init_blc
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_BLC_ATTR	p_blc; 
}AK_ISP_INIT_BLC;




/*********************镜头校正*********************/
typedef struct
{
	T_U16 coef_b[10];    //[0,255]
	T_U16 coef_c[10];    //[0,1023]
}lens_coef;

typedef struct ak_isp_lsc_attr
{
	T_U16		enable;
	//the reference point of lens correction
	T_U16		xref;      //[0,4096]
	T_U16		yref;      //[0,4096]
	T_U16		lsc_shift;   //[0，15]
	lens_coef	lsc_r_coef;
	lens_coef	lsc_gr_coef;
	lens_coef	lsc_gb_coef;
	lens_coef	lsc_b_coef;
	//the range of ten segment
	T_U16		range[10];   //[0，1023]
}AK_ISP_LSC_ATTR;

typedef struct  ak_isp_init_lsc
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_LSC_ATTR	lsc;
}AK_ISP_INIT_LSC;


/********************raw gamma*************************/

typedef struct ak_isp_raw_lut_attr
{
	T_U16	raw_r[129];      //10bit
	T_U16	raw_g[129];      //10bit
	T_U16	raw_b[129];      //10bit
	T_U16	r_key[16];
	T_U16	g_key[16];
	T_U16	b_key[16];
	T_U16	raw_gamma_enable;
}AK_ISP_RAW_LUT_ATTR;


typedef struct ak_isp_init_raw_lut
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_RAW_LUT_ATTR	raw_lut_p;
 }AK_ISP_INIT_RAW_LUT;



/*********************NR**************************/

typedef struct   ak_isp_s_nr1
{
   	T_U16	nr1_enable;           //使能位
	T_U16	nr1_weight_rtbl[17];   //10bit
	T_U16	nr1_weight_gtbl[17];	//10bit 
	T_U16	nr1_weight_btbl[17];   //10bit
   	T_U16	nr1_k;              //[0,15]
	T_U16	nr1_lc_lut[17];       //10bit
	T_U16	nr1_lc_lut_key[16];
	T_U16	nr1_calc_g_k;
	T_U16	nr1_calc_r_k;
	T_U16	nr1_calc_b_k;

}AK_ISP_NR1;

typedef struct   ak_isp_s_nr1_attr
{
	T_U16		nr1_mode;             //nr1 模式，自动或者联动模式
	AK_ISP_NR1	manual_nr1;
	AK_ISP_NR1	linkage_nr1[9];   //联动参数      
}AK_ISP_NR1_ATTR;

typedef struct  ak_isp_nr2
{
	T_U16	nr2_enable;
	T_U16	nr2_weight_tbl[17];    //10bit
	T_U16	nr2_k;               //[0,15]
	T_U16	nr2_calc_y_k;
	T_U16   y_dpc_enable;
	T_U16	y_dpc_th;
	T_U16	y_black_dpc_enable;
	T_U16	y_white_dpc_enable;
}AK_ISP_NR2;

typedef struct  ak_isp_nr2_attr
{
	T_U16		nr2_mode;             //手动或者联动模式
	AK_ISP_NR2	manual_nr2;
	AK_ISP_NR2	linkage_nr2[9];
}AK_ISP_NR2_ATTR;



typedef struct ak_isp_init_nr
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_NR1_ATTR			p_nr1;
	AK_ISP_NR2_ATTR			p_nr2;
}AK_ISP_INIT_NR;

/*********************3DNR************************/
typedef struct  ak_isp_3d_nr_ex_attr
{
	T_U16	tnr_skin_max_th;
	T_U16	tnr_skin_min_th;
	T_U16	tnr_skin_v_max_th;
	T_U16 	tnr_skin_v_min_th;
	T_U16	tnr_skin_y_max_th;
	T_U16	tnr_skin_y_min_th;
}AK_ISP_3D_NR_EX_ATTR;

typedef struct  ak_isp_3d_nr
{
	T_U16		uv_min_enable;
	T_U16		tnr_y_enable;	
	T_U16		tnr_uv_enable;
	T_U16		updata_ref_y;			//??Y
	T_U16		updata_ref_uv;			//??uv
	T_U16		tnr_refFrame_format;	//??????
	T_U16		tnr_t_y_ex_k_cfg;
	T_U16		y_2dnr_enable;
	T_U16		uv_2dnr_enable;
    
    
	T_U16		uvnr_k; 		//[0, 15]
	T_U16		uvlp_k; 		//[0, 15]
	T_U16		t_uv_k; 		//[0, 127]
	T_U16		t_uv_minstep;	//[0,31]
	T_U16		t_uv_mf_th1;	//[0, 8191]
	T_U16		t_uv_mf_th2;	//[0, 8191]
	T_U16		t_uv_diffth_k1; //[0, 255]
	T_U16		t_uv_diffth_k2; //[0, 255]
	T_U16		t_uv_diffth_slop;//[0, 255]
	T_U16		t_uv_mc_k;		//[0-31]
	T_U16		t_uv_ac_th; 	//[0, 1023]
	
	T_U16		ynr_weight_tbl[17];
	T_U16		ynr_calc_k;
	T_U16		ynr_k;			//[0, 15]
	T_U16		ynr_diff_shift; //[0,1]
	T_U16		ylp_k;			//[0, 15]
	T_U16		t_y_th1;		//[0, 255]
	T_U16		t_y_k1; 		//[0, 127]
	T_U16		t_y_k2; 		//[0, 127]
	T_U16		t_y_kslop;		//[0, 127]
	T_U16		t_y_minstep;	//[0-31]
	T_U16		t_y_mf_th1; 	//[0, 8191]
	T_U16		t_y_mf_th2; 	//[0, 8191]
	T_U16		t_y_diffth_k1;	//[0, 255]
	T_U16		t_y_diffth_k2;	//[0, 255]
	T_U16		t_y_diffth_slop;//[0, 255]
	T_U16		t_y_mc_k;		//[0-31]
	T_U16		t_y_ac_th;	    //[0, 1023]
	
	T_U32		md_th;			//[0, 65535]  ?????? [0-127]
    
}AK_ISP_3D_NR;


typedef struct  ak_isp_3d_nr_attr
{
	T_U16			isp_3d_nr_mode;  
	AK_ISP_3D_NR	manual_3d_nr;
	AK_ISP_3D_NR	linkage_3d_nr[9];
}AK_ISP_3D_NR_ATTR;

typedef struct ak_isp_init_3dnr
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_3D_NR_ATTR		p_3d_nr;
}AK_ISP_INIT_3DNR;


/********************GB**********************/

typedef struct  ak_isp_gb
{
	T_U16	gb_enable;          //使能位

	T_U16	gb_en_th;        //[0,255]
	T_U16	gb_kstep;        //[0,15]
	T_U16	gb_threshold;    //[0,1023]
} AK_ISP_GB;


typedef struct  ak_isp_gb_attr
{
	T_U16		gb_mode;          //模式选择，手动或者联动
	AK_ISP_GB	manual_gb;
	AK_ISP_GB	linkage_gb[9];
} AK_ISP_GB_ATTR;

typedef struct ak_isp_init_gb
{   
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
  	AK_ISP_GB_ATTR	p_gb;
}AK_ISP_INIT_GB;


/*********************Demo*************************/

typedef struct ak_isp_demo_attr
{
	T_U16	dm_HV_th;      //方向判别系数
	T_U16	dm_rg_thre;    //[0 1023] 
	T_U16	dm_bg_thre;    //[0 1023]
	T_U16	dm_hf_th1;      //[0, 1023]
	T_U16	dm_hf_th2;      //[0, 1023]

	T_U16	dm_rg_gain;      //[0 255]
	T_U16	dm_bg_gain;     //[0 255]
	T_U16	dm_gr_gain;      //[0 255]
	T_U16	dm_gb_gain;     //[0 255]
}AK_ISP_DEMO_ATTR ;

typedef struct ak_isp_init_demo
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_DEMO_ATTR	p_demo_attr;
}AK_ISP_INIT_DEMO;


/***********************gamma****************************/


typedef struct ak_isp_rgb_gamma_attr
{
    T_U16   r_gamma[129];   //10bit
	T_U16   g_gamma[129];   //10bit
	T_U16   b_gamma [129];   //10bit
	T_U16	r_key[16];
	T_U16	g_key[16];
	T_U16	b_key[16];
	T_U16	rgb_gamma_enable;  //如果不使能，就是一条直线
} AK_ISP_RGB_GAMMA_ATTR;

typedef struct ak_isp_init_gamma
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_RGB_GAMMA_ATTR	p_gamma_attr;
}AK_ISP_INIT_GAMMA;


/***********************Y gamma****************************/

typedef struct ak_isp_y_gamma_attr
{
	T_U16    ygamma[129];    //10bit
	T_U16    ygamma_key[16]; //曲线的关键点 
	T_U16    ygamma_uv_adjust_enable;
	T_U16    ygamma_uv_adjust_level;
	T_U16    ygamma_cnoise_yth1;   //Ygamma色差抑制门限值
	T_U16    ygamma_cnoise_yth2;   //Ygamma色差抑制门限值
	T_U16    ygamma_cnoise_slop;   
	T_U16    ygamma_cnoise_gain ;  //UV调整系数计算参数
}AK_ISP_Y_GAMMA_ATTR;

typedef struct ak_isp_init_y_gamma
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_Y_GAMMA_ATTR	p_gamma_attr;
}AK_ISP_INIT_Y_GAMMA;


/**********************CCM*************************/

typedef struct ak_isp_ccm
{
	T_U16  cc_enable;         //颜色校正使能 
	T_U16  cc_cnoise_yth;	   //亮度控制增益
	T_U16  cc_cnoise_gain;	   //亮度控制增益
	T_U16  cc_cnoise_slop;	   //亮度控制增益
	T_S16  ccm[3][3];       //[-2048, 2047]
}AK_ISP_CCM;


typedef struct ak_isp_ccm_attr
{
	T_U16		cc_mode;  //颜色校正矩阵联动或者手动
	AK_ISP_CCM  manual_ccm; 
	AK_ISP_CCM  ccm[4]; //四个联动矩阵
}AK_ISP_CCM_ATTR;




typedef  struct  ak_isp_init_ccm
{
	T_U16						param_id;      //参数id
	T_U16						length;        //参数长度;
	AK_ISP_CCM_ATTR				p_ccm;

}AK_ISP_INIT_CCM;


/********************FCS************************/
typedef  struct ak_isp_fcs
{
	T_U16	fcs_th;      //[0, 255]
	T_U16	fcs_gain_slop;  //[0,63]
	T_U16	fcs_enable;   //使能位
	T_U16	fcs_uv_nr_enable;  //使能位
	T_U16	fcs_uv_nr_th;  //[0, 1023]
}AK_ISP_FCS;

typedef  struct ak_isp_fcs_attr{
	T_U16		fcs_mode;   //模式选择，手动或者联动
	AK_ISP_FCS	manual_fcs;
	AK_ISP_FCS	linkage_fcs[9];
}AK_ISP_FCS_ATTR;

typedef  struct  ak_isp_init_fcs
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_FCS_ATTR	p_fcs;

}AK_ISP_INIT_FCS;

/*********************Wdr*************************/

typedef struct ak_isp_wdr
{
	T_U16	hdr_uv_adjust_level;    //uv调整程度              [0,31]
	T_U16	hdr_cnoise_suppress_slop;   //抑制斜率  
	T_U16	wdr_enable;
	T_U16	wdr_th1;	  //0-1023
	T_U16	wdr_th2;	  //0-1023
	T_U16	wdr_th3;	  //0-1023
	T_U16	wdr_th4;	  //0-1023
	T_U16	wdr_th5;	  //0-1023
    

    //T_U16 wdr_light_weight;
    
	T_U16	area_tb1[65];	  //?? 10bit
	T_U16	area_tb2[65];	  //?? 10bit
	T_U16	area_tb3[65];	  //?? 10bit
	T_U16	area_tb4[65];	  //?? 10bit
	T_U16	area_tb5[65];	  //?? 10bit
	T_U16	area_tb6[65];	  //?? 10bit
    
    
	T_U16	area1_key[16];
	T_U16	area2_key[16];
	T_U16	area3_key[16];
	T_U16	area4_key[16];
	T_U16	area5_key[16];
	T_U16	area6_key[16];
    
	T_U16	hdr_uv_adjust_enable;   //uv调整使能
	T_U16	hdr_cnoise_suppress_yth1;   //色彩噪声亮度阈值1           
	T_U16	hdr_cnoise_suppress_yth2;  //色彩噪声亮度阈值2
	T_U16 	hdr_cnoise_suppress_gain;   //色差抑制
	 
}AK_ISP_WDR;

typedef struct ak_isp_wdr_attr
{
	T_U16		wdr_mode;             //0联动模式，1手动模式
	AK_ISP_WDR	manual_wdr;
	AK_ISP_WDR	linkage_wdr[9]; 
}AK_ISP_WDR_ATTR;


typedef struct ak_isp_init_wdr
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_WDR_ATTR		p_wdr_attr;

}AK_ISP_INIT_WDR;


/*********************EDGE**************************/

typedef struct  ak_isp_edge
 {
   
    T_U16	enable;               //边缘增强使能位
    T_U16   edge_th;      // [0, 63]    
	T_U16   edge_max_len;  //[0, 31]
	T_U16   edge_gain_th;   //[0, 31]
	T_U16   edge_gain_slop; //[0, 127]
	T_U16   edge_y_th;    //[0, 255]
   
	T_U16	edge_gain;     //[0, 1023]
	T_U16	c_edge_enable;
	T_U16	edge_skin_detect;  
}AK_ISP_EDGE;

typedef struct  ak_isp_edge_attr{
	T_U16		edge_mode;               //边缘增强使能位
	AK_ISP_EDGE	manual_edge;
	AK_ISP_EDGE	linkage_edge[9];
}AK_ISP_EDGE_ATTR;

typedef struct  ak_isp_edge_ex_attr{
	T_U16	edge_skin_max_th;//[0, 255]
	T_U16	edge_skin_min_th; //[0, 255]
	T_U16	edge_skin_uv_max_th;  //[0, 255]
	T_U16	edge_skin_uv_min_th;  //[0, 255]
	T_U16	edge_skin_y_max_th;  //[0, 255]
	T_U16	edge_skin_y_min_th;  //[0, 255]
}AK_ISP_EDGE_EX_ATTR;

typedef struct  ak_isp_init_edge
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_EDGE_ATTR	p_edge_attr;
	AK_ISP_EDGE_EX_ATTR	p_edge_ex_attr;
     
}AK_ISP_INIT_EDGE;



/**********************SHARP*************************/
typedef struct ak_isp_sharp{

	T_U16	mf_hpf_k;                  //[0,127]
	T_U16	mf_hpf_shift;               //[0,15]

	T_U16	hf_hpf_k;                 //[0,127]
	T_U16	hf_hpf_shift;                  //[0,15]

	T_U16	sharp_method;                  //[0,3]      
   	T_U16	sharp_skin_gain_weaken; //[0，3]

    T_U16	sharp_skin_gain_th;   //[0, 255]
    T_U16	sharp_skin_detect_enable;
	T_U16	ysharp_enable;            //[0,1]
	T_S16	MF_HPF_LUT[256];    //[-256,255]
	T_S16	HF_HPF_LUT[256];  // [-256,255]
	T_U16	MF_LUT_KEY[16];
	T_U16	HF_LUT_KEY[16];
}AK_ISP_SHARP;

typedef struct ak_isp_sharp_attr{	
	T_U16			ysharp_mode;
	AK_ISP_SHARP	manual_sharp_attr;
	AK_ISP_SHARP	linkage_sharp_attr[9];	
}AK_ISP_SHARP_ATTR;



typedef struct ak_isp_sharp_ex_attr{	
    T_S16	mf_HPF[6];            //
    T_S16	hf_HPF[3];            //
  	T_U16	sharp_skin_max_th;     //[0, 255]
	T_U16	sharp_skin_min_th;     //[0, 255]
	T_U16	sharp_skin_v_max_th;  //[0, 255]
	T_U16	sharp_skin_v_min_th;  //[0, 255]
	T_U16	sharp_skin_y_max_th;  //[0, 255]
	T_U16	sharp_skin_y_min_th;  //[0, 255]
}AK_ISP_SHARP_EX_ATTR;

typedef struct ak_isp_init_sharp
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_SHARP_ATTR		p_sharp_attr;
	AK_ISP_SHARP_EX_ATTR	p_sharp_ex_attr;   
 }AK_ISP_INIT_SHARP;

/*********************饱和度**************************/
typedef struct  ak_isp_saturation
{
	T_U16	SE_enable;        //使能位
	T_U16	SE_th1;       //[0, 1023]
	T_U16	SE_th2;       //[0, 1023]
	T_U16	SE_th3;	      //[0, 1023]
	T_U16	SE_th4;	      //[0, 1023]
	T_U16	SE_scale_slop1;  //[0, 255]
	T_U16	SE_scale_slop2;  //[0, 255]
	T_U16	SE_scale1; 	  //[0,255]
	T_U16	SE_scale2;	      //[0,255]
	T_U16	SE_scale3; 	  //[0,255]    
}AK_ISP_SATURATION;

typedef struct  ak_isp_saturation_attr
{
	T_U16				SE_mode;        //饱和度模式
	AK_ISP_SATURATION	manual_sat;
	AK_ISP_SATURATION	linkage_sat[9];    
}AK_ISP_SATURATION_ATTR;

typedef struct ak_isp_init_saturation
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_SATURATION_ATTR	p_se_attr;
  
}AK_ISP_INIT_SATURATION;

/************************对比度********************************/

typedef struct  ak_isp_contrast
{
     T_U16  y_contrast;  //[0,511]
     T_S16  y_shift;    //[0, 511]
}AK_ISP_CONTRAST;
 
typedef struct  ak_isp_auto_contrast
{
	T_U16  dark_pixel_area; //[0, 511]
	T_U16  dark_pixel_rate;  //[1, 256]
	T_U16  shift_max;    //[0, 127]
}AK_ISP_AUTO_CONTRAST;
 
typedef struct  ak_isp_contrast_ATTR
{
	T_U16 cc_mode; //模式选择，手动或者联动
	AK_ISP_CONTRAST manual_contrast;
	AK_ISP_AUTO_CONTRAST linkage_contrast[9];
}AK_ISP_CONTRAST_ATTR;


typedef struct  ak_isp_init_contrast
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_CONTRAST_ATTR	p_contrast;
}AK_ISP_INIT_CONTRAST;


/*********************rgb to yuv*****************************/

typedef  struct ak_isp_rgb2yuv_attr
{
	T_U16	mode;                      //bt601 或者bt709
}AK_ISP_RGB2YUV_ATTR;

typedef  struct ak_isp_init_rgb2yuv
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_RGB2YUV_ATTR	p_rgb2yuv;
}AK_ISP_INIT_RGB2YUV;


/*************************YUV effect********************************/

typedef struct ak_isp_effect_attr
{
	T_U16  y_a;      // [0, 255]
	T_S16  y_b;     //[-128, 127]
	T_S16  uv_a;    //[0, 255]
	T_S16  uv_b;    //[0, 255]
	T_U16  dark_margin_en;    //黑边使能
}AK_ISP_EFFECT_ATTR;

typedef  struct ak_isp_init_effect
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_EFFECT_ATTR  p_isp_effect;
}AK_ISP_INIT_EFFECT;


/**************************坏点校正**********************************/

typedef struct ak_isp_ddpc
{ 
	T_U16	ddpc_enable;          //动态坏点使能位
	T_U16	ddpc_th;             //10bit
	T_U16	white_dpc_enable;    //白点消除使能位
	T_U16	black_dpc_enable;    //黑点消除使能位
}AK_ISP_DDPC;

typedef struct ak_isp_ddpc_attr
{
	T_U16		ddpc_mode;             //0联动模式，1手动模式
	AK_ISP_DDPC	manual_ddpc;
	AK_ISP_DDPC	linkage_ddpc[9]; 
}AK_ISP_DDPC_ATTR;

typedef struct ak_isp_sdpc_attr
{ 
	T_U32	sdpc_enable;                 //静态坏点使能位
	T_U32	sdpc_table[1024];             //静态坏点坐标值，最大为1024个，数据格式{6h0 ,y_position[9:0],5'h0,x_position[10:0]}
}AK_ISP_SDPC_ATTR;

typedef  struct ak_isp_init_dpc
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_DDPC_ATTR	p_ddpc; 
	AK_ISP_SDPC_ATTR	p_sdpc; 

}AK_ISP_INIT_DPC;



/***********************zone weight********************************/
typedef struct ak_isp_weight_attr  
{
	T_U16   zone_weight[8][16];            //权重系数  
}AK_ISP_WEIGHT_ATTR;


typedef struct ak_isp_init_weight
{   
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_WEIGHT_ATTR  p_weight;
}AK_ISP_INIT_WEIGHT;


/***************************AF*******************************/

typedef struct ak_isp_af_attr{
   // int af_statics;      //统计结果  
	
	T_U16	af_win0_left; 	//[0, 1279]
	T_U16	af_win0_right;	//[0, 1279]
	T_U16	af_win0_top;	    //[0, 959]
	T_U16	af_win0_bottom;   //[0, 959]
   
	T_U16	af_win1_left; 	//[0, 1279]
	T_U16	af_win1_right;	//[0, 1279]
	T_U16	af_win1_top;	    //[0, 959]
	T_U16	af_win1_bottom;   //[0, 959]
   
	T_U16	af_win2_left; 	//[0, 1279]
	T_U16	af_win2_right;	//[0, 1279]
	T_U16	af_win2_top;	    //[0, 959]
	T_U16	af_win2_bottom;   //[0, 959]
   
	T_U16	af_win3_left; 	//[0, 1279]
	T_U16	af_win3_right;	//[0, 1279]
	T_U16	af_win3_top;	    //[0, 959]
	T_U16	af_win3_bottom;   //[0, 959]
   
	T_U16	af_win4_left; 	//[0, 1279]
	T_U16	af_win4_right;	//[0, 1279]
	T_U16	af_win4_top;	    //[0, 959]
	T_U16	af_win4_bottom;   //[0, 959]
    	
	T_U16   af_th;       //[0, 128]
}AK_ISP_AF_ATTR;

typedef struct ak_isp_af
{   
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_AF_ATTR  p_af_attr;    
}AK_ISP_INIT_AF;


/*************************WB**********************************/

typedef  struct  ak_isp_wb_type_attr
{
	T_U16	wb_type;  
}AK_ISP_WB_TYPE_ATTR;


typedef  struct  ak_isp_mwb_attr
{
	T_U16	r_gain;
	T_U16	g_gain;
	T_U16	b_gain;
	T_S16	r_offset;
	T_S16	g_offset;
	T_S16	b_offset;

}AK_ISP_MWB_ATTR;


typedef  struct  ak_isp_awb_attr        
{   
	T_U16	g_weight[16];
	T_U16	y_low;               //y_low<=y_high
	T_U16	y_high;
	T_U16   err_est;
    T_U16   gr_low[10];            //gr_low[i]<=gr_high[i]
    T_U16   gb_low[10];            //gb_low[i]<=gb_high[i]
    T_U16   gr_high[10];
    T_U16   gb_high[10];
    T_U16   rb_low[10];           //rb_low[i]<=rb_high[i]
    T_U16   rb_high[10];

    //awb软件部分需要设置的参数
    T_U16   auto_wb_step;                 //白平衡步长计算
    T_U16   total_cnt_thresh;            //像素个数阈值
    T_U16   colortemp_stable_cnt_thresh; //稳定帧数，多少帧一样认为环境色温改变
    T_U16	colortemp_envi[10];
}AK_ISP_AWB_ATTR;

typedef  struct  ak_isp_awb_ctrl        
{
	int rgain_max;
	int rgain_min;
	int ggain_max;
	int ggain_min;
	int bgain_max;
	int bgain_min;
	int rgain_ex;
	int bgain_ex;
}AK_ISP_AWB_CTRL;

typedef  struct  ak_isp_awb_ex_attr        
{
	int awb_ex_ctrl_enable;
	AK_ISP_AWB_CTRL awb_ctrl[10];
}AK_ISP_AWB_EX_ATTR;



typedef struct ak_isp_init_wb
{   
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_WB_TYPE_ATTR			wb_type; 
	AK_ISP_MWB_ATTR			p_mwb;
	AK_ISP_AWB_ATTR			p_awb;
	AK_ISP_AWB_EX_ATTR		p_awb_ex;
}AK_ISP_INIT_WB;


/*************************Expsoure****************************/

typedef struct ak_isp_raw_hist_attr
{
	T_U16	enable;
}AK_ISP_RAW_HIST_ATTR;

typedef struct ak_isp_rgb_hist_attr
{
	T_U16	enable;
}AK_ISP_RGB_HIST_ATTR;

typedef struct  ak_isp_yuv_hist_attr
{
	T_U16	enable;
}AK_ISP_YUV_HIST_ATTR;

typedef  struct  ak_isp_exp_type
{
	T_U16	exp_type;  
}AK_ISP_EXP_TYPE;



typedef struct ak_isp_ae_attr
{
	T_U32	exp_time_max;    //曝光时间的最大值
	T_U32	exp_time_min;    //曝光时间的最小值
	T_U32	d_gain_max;      //数字增益的最大值
	T_U32	d_gain_min;     //数字增益的最小值
	T_U32	isp_d_gain_min;  //isp数字增益的最小值
	T_U32	isp_d_gain_max;  //isp数字增益的最大值
	T_U32	a_gain_max;     //模拟增益的最大值
	T_U32	a_gain_min;      //模拟增益的最小值     
	T_U32	exp_step;            //用户曝光调整步长
	T_U32	exp_stable_range;     //稳定范围
	T_U32	target_lumiance;     //目标亮度
	T_U32	envi_gain_range[10][2];
	T_U32	hist_weight[16];        //曝光计算权重    [0 ,16]
    T_U32	OE_suppress_en;    //过曝抑制使能
    T_U32	OE_detect_scope; //[0,255]    过曝检测范围
    T_U32	OE_rate_max; //[0, 255]    过曝检测系数最大值
    T_U32	OE_rate_min; //[0, 255]    过曝检测系数最小值
}AK_ISP_AE_ATTR;


//帧率控制结构体
typedef  struct ak_isp_frame_rate_attr
{
	T_U32  	hight_light_frame_rate ;
	T_U32  	hight_light_max_exp_time ;
	T_U32  	hight_light_to_low_light_gain;
	T_U32  	low_light_frame_rate;
	T_U32  	low_light_max_exp_time;
	T_U32  	low_light_to_hight_light_gain;
}AK_ISP_FRAME_RATE_ATTR;

typedef  struct ak_isp_frame_rate_attr_ex {
	T_U8  	fps1;
	T_U8  	gain11;
	T_U8  	gain12;
	T_U8  	reserve1;
	T_U16  	max_exp_time1;

	T_U8  	fps2;
	T_U8  	gain21;
	T_U8  	gain22;
	T_U8  	reserve2;
	T_U16  	max_exp_time2;

	T_U8  	fps3;
	T_U8  	gain31;
	T_U8  	gain32;
	T_U8  	reserve3;
	T_U16  	max_exp_time3;

	T_U8  	fps4;
	T_U8  	gain41;
	T_U8  	gain42;
	T_U8  	reserve4;
	T_U16  	max_exp_time4;
}AK_ISP_FRAME_RATE_ATTR_EX;


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

typedef struct ak_isp_misc_attr
{
	T_U16	hsyn_pol;
	T_U16	vsync_pol;
	T_U16	pclk_pol;
	T_U16	test_pattern_en;
	T_U16	test_pattern_cfg;
	T_U16	cfa_mode;
	T_U16 	inputdataw;
    T_U16	one_line_cycle;
    T_U16	hblank_cycle;
    T_U16	frame_start_delay_en;
    T_U16	frame_start_delay_num;
	T_U16	flip_en;
	T_U16	mirror_en;
	T_U16	twoframe_merge_en;
	T_U16	mipi_line_end_sel;
	T_U16	mipi_line_end_cnt_en_cfg;
	T_U16	mipi_count_time;
} AK_ISP_MISC_ATTR;  

typedef struct ak_isp_init_misc
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_MISC_ATTR	p_misc;
}AK_ISP_INIT_MISC;



/*************************HUE*****************************/

typedef struct ak_isp_hue
{
	T_U16  hue_sat_en;       //hue使能
	T_S8   hue_lut_a[65];     //[-128, 127]
	T_S8   hue_lut_b[65];     //[-128, 127]
	T_U8   hue_lut_s[65];     //[0, 255]
}AK_ISP_HUE;

typedef struct ak_isp_hue_attr
{
	T_U16        hue_mode;      //联动或者手动
	AK_ISP_HUE   manual_hue; 
	AK_ISP_HUE   hue[4];         //四个联动参数
}AK_ISP_HUE_ATTR;

typedef struct ak_isp_init_hue
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_HUE_ATTR		p_hue;
}AK_ISP_INIT_HUE;

/*************************sensor*****************************/

typedef struct ak_isp_sensor_attr
{
	T_U16	sensor_addr;
	T_U16	sensor_value;
} AK_ISP_SENSOR_ATTR;  

typedef struct ak_isp_init_sensor
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_SENSOR_ATTR	*p_sensor;
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
	//edge
	//AK_ISP_INIT_EDGE		p_Isp_edge;
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

