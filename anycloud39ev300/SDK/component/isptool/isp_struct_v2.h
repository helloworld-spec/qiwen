/** @file
* @brief Define the ISP structs
*
* Copyright (C) 2015 Anyka (GuangZhou) Software Technology Co., Ltd.
* @author
* @date 2015-07-06
* @version 1.0
*/

#ifndef __ISP_STRUCT_V2_H__
#define __ISP_STRUCT_V2_H__

#include "anyka_types.h"


#define ISP_MODULE_ID_SIZE		2
#define ISP_MODULE_LEN_SIZE		2

#define ISP_MODULE_HEAD_SIZE	(ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE)

#define CFG_FILE_NOTES_LEN_V2		300
#define CFG_FILE_VERSION_LEN_V2		5


typedef enum {
	//前n个为ISP模块结构体id
	ISP_BB_V2 = 0,   			//黑平衡
	ISP_LSC_V2,				//镜头校正
	ISP_RAW_LUT_V2,			//raw gamma
	ISP_NR_V2,					//NR
	ISP_3DNR_V2,				//3DNR

	ISP_GB_V2,					//绿平衡
	ISP_DEMO_V2,				//DEMOSAIC
	ISP_GAMMA_V2,				//GAMMA
	ISP_CCM_V2,				//颜色校正
	ISP_FCS_V2,				//FCS

	ISP_WDR_V2,				//WDR
	ISP_EDGE_V2,				//EDGE
	ISP_SHARP_V2,				//SHARP
	ISP_SATURATION_V2,			//饱和度
	ISP_CONTRAST_V2,			//对比度

	ISP_RGB2YUV_V2,			//rgb to yuv
	ISP_YUVEFFECT_V2,			//YUV效果
	ISP_DPC_V2,				//坏点校正
	ISP_ZONE_WEIGHT_V2,		//权重系数
	ISP_AF_V2,					//AF

	ISP_WB_V2,					//WB
	ISP_EXP_V2,				//Expsoure
	ISP_MISC_V2,				//杂项

	//统计结果
	ISP_3DSTAT_V2,				//3D降噪统计
	ISP_AESTAT_V2,				//AE统计
	ISP_AFSTAT_V2,				//AF统计
	ISP_AWBSTAT_V2,			//AWB统计

	ISP_SENSOR_V2,				//sensor参数

	//下面是一些特殊值、扩展值
	ISP_PARM_CODE_V2,			//按参数编码
	ISP_REGISTER_V2,			//寄存器参数

	ISP_RAW_IMG_V2,			//一帧raw图像数据
	ISP_YUV_IMG_V2,			//一帧yuv图像数据
	ISP_ENCODE_IMG_V2,			//一帧encode图像数据

	ISP_CFG_DATA_V2,			//day cfg data
	
	ISP_HEARTBEAT_V2,			//heartbeat

    ISP_ATTR_TYPE_NUM_V2
} T_ATTR_TYPE_V2;

typedef struct cfgfile_headinfo_v2               
{
	T_U8	sensorId;
	T_U8	styleId;
	
    T_U16	year;
    T_U8	month;
    T_U8	day;
    T_U8	hour;
    T_U8	minute;
    T_U8	second;
	T_U8	subFileId;
	
	T_U8	version[6];
}CFGFILE_HEADINFO_V2;


typedef  struct  ak_isp_awb_ctrl_V2        
{
	int rgain_max;
	int rgain_min;
	int ggain_max;
	int ggain_min;
	int bgain_max;
	int bgain_min;
	int rgain_ex;
	int bgain_ex;
}AK_ISP_AWB_CTRL_V2;

typedef  struct  ak_isp_awb_ex_attr_V2        
{
	int awb_ex_ctrl_enable;
	AK_ISP_AWB_CTRL_V2 awb_ctrl[5];
}AK_ISP_AWB_EX_ATTR_V2;



/**********************黑平衡************************/
typedef struct ak_isp_blc_V2
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
}AK_ISP_BLC_V2;

typedef struct ak_isp_blc_attr_V2
{
	T_U16		blc_mode;             //0联动模式，1手动模式
	AK_ISP_BLC_V2	m_blc;
	AK_ISP_BLC_V2	linkage_blc[9]; 
}AK_ISP_BLC_ATTR_V2;

typedef struct  ak_isp_init_blc_V2
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_BLC_ATTR_V2	p_blc; 
}AK_ISP_INIT_BLC_V2;




/*********************镜头校正*********************/
typedef struct
{
	T_U16 coef_b[10];    //[0,255]
	T_U16 coef_c[10];    //[0,1023]
}lens_coef_V2;

typedef struct ak_isp_lsc_attr_V2
{
	T_U16		enable;
	//the reference point of lens correction
	T_U16		xref;      //[0,4096]
	T_U16		yref;      //[0,4096]
	T_U16		lsc_shift;   //[0，15]
	lens_coef_V2	lsc_r_coef;
	lens_coef_V2	lsc_gr_coef;
	lens_coef_V2	lsc_gb_coef;
	lens_coef_V2	lsc_b_coef;
	//the range of ten segment
	T_U16		range[10];   //[0，1023]
}AK_ISP_LSC_ATTR_V2;

typedef struct  ak_isp_init_lsc_V2
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_LSC_ATTR_V2	lsc;
}AK_ISP_INIT_LSC_V2;


/********************raw gamma*************************/

typedef struct ak_isp_raw_lut_attr_V2
{
	T_U16	raw_r[129];      //10bit
	T_U16	raw_g[129];      //10bit
	T_U16	raw_b[129];      //10bit
	T_U16	r_key[16];
	T_U16	g_key[16];
	T_U16	b_key[16];
	T_U16	raw_gamma_enable;
}AK_ISP_RAW_LUT_ATTR_V2;


typedef struct ak_isp_init_raw_lut_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_RAW_LUT_ATTR_V2	raw_lut_p;
 }AK_ISP_INIT_RAW_LUT_V2;



/*********************NR**************************/

typedef struct   ak_isp_s_nr1_V2
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

}AK_ISP_NR1_V2;

typedef struct   ak_isp_s_nr1_attr_V2
{
	T_U16		nr1_mode;             //nr1 模式，自动或者联动模式
	AK_ISP_NR1_V2	manual_nr1;
	AK_ISP_NR1_V2	linkage_nr1[9];   //联动参数      
}AK_ISP_NR1_ATTR_V2;

typedef struct  ak_isp_nr2_V2
{
	T_U16	nr2_enable;
	T_U16	nr2_weight_tbl[17];    //10bit
	T_U16	nr2_k;               //[0,15]
	T_U16	nr2_calc_y_k;
}AK_ISP_NR2_V2;

typedef struct  ak_isp_nr2_attr_V2
{
	T_U16		nr2_mode;             //手动或者联动模式
	AK_ISP_NR2_V2	manual_nr2;
	AK_ISP_NR2_V2	linkage_nr2[9];
}AK_ISP_NR2_ATTR_V2;



typedef struct ak_isp_init_nr_V2
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_NR1_ATTR_V2			p_nr1;
	AK_ISP_NR2_ATTR_V2			p_nr2;
}AK_ISP_INIT_NR_V2;

/*********************3DNR************************/
typedef struct  ak_isp_3d_nr_ex_attr_V2
{
	T_U16	tnr_skin_max_th;
	T_U16	tnr_skin_min_th;
	T_U16	tnr_skin_v_max_th;
	T_U16 	tnr_skin_v_min_th;
	T_U16	tnr_skin_y_max_th;
	T_U16	tnr_skin_y_min_th;
}AK_ISP_3D_NR_EX_ATTR_V2;

typedef struct  ak_isp_3d_nr_V2
{
	T_U16	uv_min_enable;
	T_U16	tnr_y_enable;	
	T_U16	tnr_uv_enable;


	T_U16	mc_VAR_y_th;    //[0, 4095]
	T_U16	t_filter_y_k;      //[0-127]
	T_U16	s_filter_var_gain;  //0-15
	T_U16	s_filter_var_th;    //0-255

	T_U16	s_filter_y_th1;    //0-1023
	T_U16	s_filter_y_th2;    //0-1023
	T_U16	s_filter_y_k1;     //0-128
	T_U16	s_filter_y_k2;     //0-128
	T_U16	s_filter_y_kstep;   //0-31

	T_U16	mc_SAD_uv_th;   //[0, 4095]
	T_U16	t_filter_uv_k;     //[0, 127]
	T_U16	s_filter_uv_th1;   //[0, 128]
	T_U16	s_filter_uv_th2;   //[0, 128]
	T_U16	s_filter_uv_k1;   //  [0,128]
	T_U16	s_filter_uv_k2;     // [0,128] 
	T_U16	s_filter_uv_kstep;   //  [0,31] 

	T_U16	MD_th;              //运动检测阈值 [0-127]
	T_U16	updata_ref_y;           //更新Y
	T_U16	updata_ref_uv;         //更新uv
	T_U16	tnr_refFrame_format;   //参考帧的格式

	T_U16	tnr_skin_detect;
	T_U16	tnr_skin_k;
}AK_ISP_3D_NR_V2;

typedef struct  ak_isp_3d_nr_attr_V2
{
	T_U16			isp_3d_nr_mode;  
	AK_ISP_3D_NR_V2	manual_3d_nr;
	AK_ISP_3D_NR_V2	linkage_3d_nr[9];
}AK_ISP_3D_NR_ATTR_V2;

typedef struct ak_isp_init_3dnr_V2
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_3D_NR_EX_ATTR_V2	p_3d_nr_ex;
	AK_ISP_3D_NR_ATTR_V2		p_3d_nr;
}AK_ISP_INIT_3DNR_V2;


/********************GB**********************/

typedef struct  ak_isp_gb_V2
{
	T_U16	gb_enable;          //使能位

	T_U16	gb_en_th;        //[0,255]
	T_U16	gb_kstep;        //[0,15]
	T_U16	gb_threshold;    //[0,1023]
} AK_ISP_GB_V2;


typedef struct  ak_isp_gb_attr_V2
{
	T_U16		gb_mode;          //模式选择，手动或者联动
	AK_ISP_GB_V2	manual_gb;
	AK_ISP_GB_V2	linkage_gb[9];
} AK_ISP_GB_ATTR_V2;

typedef struct ak_isp_init_gb_V2
{   
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
  	AK_ISP_GB_ATTR_V2	p_gb;
}AK_ISP_INIT_GB_V2;


/*********************Demo*************************/

typedef struct ak_isp_demo_attr_V2
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
}AK_ISP_DEMO_ATTR_V2 ;

typedef struct ak_isp_init_demo_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_DEMO_ATTR_V2	p_demo_attr;
}AK_ISP_INIT_DEMO_V2;


/***********************gamma****************************/


typedef struct ak_isp_rgb_gamma_attr_V2
{
    T_U16   r_gamma[129];   //10bit
	T_U16   g_gamma[129];   //10bit
	T_U16   b_gamma [129];   //10bit
	T_U16	r_key[16];
	T_U16	g_key[16];
	T_U16	b_key[16];
	T_U16	rgb_gamma_enable;  //如果不使能，就是一条直线
} AK_ISP_RGB_GAMMA_ATTR_V2;

typedef struct ak_isp_init_gamma_V2
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_RGB_GAMMA_ATTR_V2	p_gamma_attr;
}AK_ISP_INIT_GAMMA_V2;


/**********************CCM*************************/

typedef struct ak_isp_ccm_V2
{
	T_U16  cc_enable;         //颜色校正使能 
	T_S16  ccm[3][3];       //[-2048, 2047]
}AK_ISP_CCM_V2;

typedef struct ak_isp_ccm_ex_attr_V2
{
	T_U16	weight_base;     //[0,255]
    T_U16   weight_step;     //[0,15]
}AK_ISP_CCM_EX_ATTR_V2;


typedef struct ak_isp_ccm_attr_V2
{
	T_U16		cc_mode;  //颜色校正矩阵联动或者手动
	AK_ISP_CCM_V2  manual_ccm; 
	AK_ISP_CCM_V2  ccm[4]; //四个联动矩阵
}AK_ISP_CCM_ATTR_V2;

typedef struct ak_isp_ccm_fine_attr_V2
{
	T_U16	cc_fine_enable;   //增强特定颜色
	T_U16	weight_fine_base; //值域范围[0-255]
	T_U16	weight_fine_step;  //值域范围[0,31]
	T_U16	cc_fine_RG_target;  //16bit
	T_U16	cc_fine_BG_target;  //16bit
	T_U16	cc_fine_RG_scale;   //[0, 7]
	T_U16	cc_fine_BG_scale;   //[0, 7]
    T_S16	ccm_fine[3][3];  //[-2048, 2047]
}AK_ISP_CCM_FINE_ATTR_V2;

typedef struct ak_isp_white_color_s_attr_V2
{
	T_U16  cc_wc_s_enable;   //抑制白色的颜色
	T_U16  RG_target;  //RG_target、BG_target是软件设定的偏色参考点[0,511]
	T_U16  BG_target;    // [0,511]
	T_U16  damp_base;     //范围[0-255]
	T_U16  damp_step;    //范围[0,15]
}AK_ISP_WHITE_COLOR_S_ATTR_V2;


typedef  struct  ak_isp_init_ccm_V2
{
	T_U16						param_id;      //参数id
	T_U16						length;        //参数长度;
	AK_ISP_CCM_ATTR_V2				p_ccm;
	AK_ISP_CCM_EX_ATTR_V2			p_ccm_ex;
	//CCM_FINE
	AK_ISP_CCM_FINE_ATTR_V2		p_ccm_fine;

	//cw
	AK_ISP_WHITE_COLOR_S_ATTR_V2	p_cw; 

}AK_ISP_INIT_CCM_V2;


/********************FCS************************/
typedef  struct ak_isp_fcs_V2
{
	T_U16	fcs_th;      //[0, 255]
	T_U16	fcs_gain_slop;  //[0,63]
	T_U16	fcs_enable;   //使能位
	T_U16	fcs_uv_nr_enable;  //使能位
	T_U16	fcs_uv_nr_th;  //[0, 1023]
}AK_ISP_FCS_V2;

typedef  struct ak_isp_fcs_attr_V2{
	T_U16		fcs_mode;   //模式选择，手动或者联动
	AK_ISP_FCS_V2	manual_fcs;
	AK_ISP_FCS_V2	linkage_fcs[9];
}AK_ISP_FCS_ATTR_V2;

typedef  struct  ak_isp_init_fcs_V2
{
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_FCS_ATTR_V2	p_fcs;

}AK_ISP_INIT_FCS_V2;

/*********************Wdr*************************/

typedef struct ak_isp_wdr_V2
{
	T_U16	hdr_uv_adjust_level;    //uv调整程度              [0,31]
	T_U16	hdr_cnoise_suppress_slop;   //抑制斜率  
	T_U16	wdr_enable;
	T_U16	wdr_dark_thre;     //0-1023
	T_U16	wdr_light_thre;     //0-1023

	//T_U16 wdr_light_weight;
	T_U16	dark_tbl[129];    //曲线 10bit
	T_U16	mid_tbl[129];    //曲线  10bit
	T_U16	light_tbl[129];    //曲线  10bit
	T_U16	dark_key[16];
	T_U16	mid_key[16];
	T_U16	light_key[16];
	T_U16	hdr_uv_adjust_enable;   //uv调整使能
	T_U16	hdr_cnoise_suppress_yth1;   //色彩噪声亮度阈值1           
	T_U16	hdr_cnoise_suppress_yth2;  //色彩噪声亮度阈值2
	T_U16 	hdr_cnoise_suppress_gain;   //色差抑制
	 
}AK_ISP_WDR_V2;

typedef struct ak_isp_wdr_attr_V2
{
	T_U16		wdr_mode;             //0联动模式，1手动模式
	AK_ISP_WDR_V2	manual_wdr;
	AK_ISP_WDR_V2	linkage_wdr[9]; 
}AK_ISP_WDR_ATTR_V2;

typedef struct ak_isp_wdr_ex_attr_V2
{ 
	T_U16	hdr_blkW;
	T_U16	hdr_blkH;
	T_U16	hdr_reverseW_g;   //[0,511]
	T_U16	hdr_reverseW_shift;    //[0,7];
	T_U16	hdr_reverseH_g;       //[0,511]
	T_U16	hdr_reverseH_shift;     //[0,7]
	T_U16	hdr_weight_g;       //[0,511]
	T_U16	hdr_weight_shift;    //[0,7]
}AK_ISP_WDR_EX_ATTR_V2;


typedef struct ak_isp_init_wdr_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_WDR_ATTR_V2		p_wdr_attr;
    AK_ISP_WDR_EX_ATTR_V2	p_wdr_ex;

}AK_ISP_INIT_WDR_V2;


/*********************EDGE**************************/

typedef struct  ak_isp_edge_V2
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
}AK_ISP_EDGE_V2;

typedef struct  ak_isp_edge_attr_V2{
	T_U16		edge_mode;               //边缘增强使能位
	AK_ISP_EDGE_V2	manual_edge;
	AK_ISP_EDGE_V2	linkage_edge[9];
}AK_ISP_EDGE_ATTR_V2;

typedef struct  ak_isp_edge_ex_attr_V2{
	T_U16	edge_skin_max_th;//[0, 255]
	T_U16	edge_skin_min_th; //[0, 255]
	T_U16	edge_skin_uv_max_th;  //[0, 255]
	T_U16	edge_skin_uv_min_th;  //[0, 255]
	T_U16	edge_skin_y_max_th;  //[0, 255]
	T_U16	edge_skin_y_min_th;  //[0, 255]
}AK_ISP_EDGE_EX_ATTR_V2;

typedef struct  ak_isp_init_edge_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_EDGE_ATTR_V2	p_edge_attr;
	AK_ISP_EDGE_EX_ATTR_V2	p_edge_ex_attr;
     
}AK_ISP_INIT_EDGE_V2;



/**********************SHARP*************************/
typedef struct ak_isp_sharp_V2{

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
}AK_ISP_SHARP_V2;

typedef struct ak_isp_sharp_attr_V2{	
	T_U16			ysharp_mode;
	AK_ISP_SHARP_V2	manual_sharp_attr;
	AK_ISP_SHARP_V2	linkage_sharp_attr[9];	
}AK_ISP_SHARP_ATTR_V2;



typedef struct ak_isp_sharp_ex_attr_V2{	
    T_S16	mf_HPF[6];            //
    T_S16	hf_HPF[3];            //
  	T_U16	sharp_skin_max_th;     //[0, 255]
	T_U16	sharp_skin_min_th;     //[0, 255]
	T_U16	sharp_skin_v_max_th;  //[0, 255]
	T_U16	sharp_skin_v_min_th;  //[0, 255]
	T_U16	sharp_skin_y_max_th;  //[0, 255]
	T_U16	sharp_skin_y_min_th;  //[0, 255]
}AK_ISP_SHARP_EX_ATTR_V2;

typedef struct ak_isp_init_sharp_V2
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_SHARP_ATTR_V2		p_sharp_attr;
	AK_ISP_SHARP_EX_ATTR_V2	p_sharp_ex_attr;   
 }AK_ISP_INIT_SHARP_V2;

/*********************饱和度**************************/
typedef struct  ak_isp_saturation_V2
{
	T_U16	SE_enable;        //使能位
	T_U16	SE_th1;       //[0, 1023]
	T_U16	SE_th2;       //[0, 1023]
	T_U16	SE_scale_slop;  //[0, 255]
	T_U16	SE_scale1;     //[0,255]
	T_U16	SE_scale2;        //[0,255]
}AK_ISP_SATURATION_V2;

typedef struct  ak_isp_saturation_attr_V2
{
	T_U16				SE_mode;        //饱和度模式
	AK_ISP_SATURATION_V2	manual_sat;
	AK_ISP_SATURATION_V2	linkage_sat[9];    
}AK_ISP_SATURATION_ATTR_V2;

typedef struct ak_isp_init_saturation_V2
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_SATURATION_ATTR_V2	p_se_attr;
  
}AK_ISP_INIT_SATURATION_V2;

/************************对比度********************************/

typedef struct  ak_isp_contrast_V2
{
     T_U16  y_contrast;  //[0,511]
     T_S16  y_shift;    //[0, 511]
}AK_ISP_CONTRAST_V2;
 
typedef struct  ak_isp_auto_contrast_V2
{
	T_U16  dark_pixel_area; //[0, 511]
	T_U16  dark_pixel_rate;  //[1, 256]
	T_U16  shift_max;    //[0, 127]
}AK_ISP_AUTO_CONTRAST_V2;
 
typedef struct  ak_isp_contrast_ATTR_V2
{
	T_U16 cc_mode; //模式选择，手动或者联动
	AK_ISP_CONTRAST_V2 manual_contrast;
	AK_ISP_AUTO_CONTRAST_V2 linkage_contrast[9];
}AK_ISP_CONTRAST_ATTR_V2;


typedef struct  ak_isp_init_contrast_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_CONTRAST_ATTR_V2	p_contrast;
}AK_ISP_INIT_CONTRAST_V2;


/*********************rgb to yuv*****************************/

typedef  struct ak_isp_rgb2yuv_attr_V2
{
	T_U16	mode;                      //bt601 或者bt709
}AK_ISP_RGB2YUV_ATTR_V2;

typedef  struct ak_isp_init_rgb2yuv_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_RGB2YUV_ATTR_V2	p_rgb2yuv;
}AK_ISP_INIT_RGB2YUV_V2;


/*************************YUV effect********************************/

typedef struct ak_isp_effect_attr_V2
{
	T_U16  y_a;      // [0, 255]
	T_S16  y_b;     //[-128, 127]
	T_U16  uv_a;    //[0, 255]
	T_U16  uv_b;    //[0, 255]
	T_U16  dark_margin_en;    //黑边使能
}AK_ISP_EFFECT_ATTR_V2;

typedef  struct ak_isp_init_effect_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_EFFECT_ATTR_V2  p_isp_effect;
}AK_ISP_INIT_EFFECT_V2;


/**************************坏点校正**********************************/

typedef struct ak_isp_ddpc_V2
{ 
	T_U16	ddpc_enable;          //动态坏点使能位
	T_U16	ddpc_th;             //10bit
}AK_ISP_DDPC_V2;

typedef struct ak_isp_ddpc_attr_V2
{
	T_U16		ddpc_mode;             //0联动模式，1手动模式
	AK_ISP_DDPC_V2	manual_ddpc;
	AK_ISP_DDPC_V2	linkage_ddpc[9]; 
}AK_ISP_DDPC_ATTR_V2;

typedef struct ak_isp_sdpc_attr_V2
{ 
	T_U32	sdpc_enable;                 //静态坏点使能位
	T_U32	sdpc_table[1024];             //静态坏点坐标值，最大为1024个，数据格式{6h0 ,y_position[9:0],5'h0,x_position[10:0]}
}AK_ISP_SDPC_ATTR_V2;

typedef  struct ak_isp_init_dpc_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_DDPC_ATTR_V2	p_ddpc; 
	AK_ISP_SDPC_ATTR_V2	p_sdpc; 

}AK_ISP_INIT_DPC_V2;



/***********************zone weight********************************/
typedef struct ak_isp_weight_attr_V2  
{
	T_U16   zone_weight[8][16];            //权重系数  
}AK_ISP_WEIGHT_ATTR_V2;


typedef struct ak_isp_init_weight_V2
{   
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_WEIGHT_ATTR_V2  p_weight;
}AK_ISP_INIT_WEIGHT_V2;


/***************************AF*******************************/

typedef struct ak_isp_af_attr_V2{
   // int af_statics;      //统计结果  
	T_U16	af_win_left;   //[0, 1279]
	T_U16	af_win_right;  //[0, 1279]
	T_U16	af_win_top;   //[0, 959]
	T_U16	af_win_bottom; //[0, 959]
	T_U16   af_th;       //[0, 128]
}AK_ISP_AF_ATTR_V2;

typedef struct ak_isp_af_V2
{   
	T_U16			param_id;      //参数id
	T_U16			length;        //参数长度;
	AK_ISP_AF_ATTR_V2  p_af_attr;    
}AK_ISP_INIT_AF_V2;


/*************************WB**********************************/

typedef  struct  ak_isp_wb_type_attr_V2
{
	T_U16	wb_type;  
}AK_ISP_WB_TYPE_ATTR_V2;


typedef  struct  ak_isp_mwb_attr_V2
{
	T_U16	r_gain;
	T_U16	g_gain;
	T_U16	b_gain;
	T_S16	r_offset;
	T_S16	g_offset;
	T_S16	b_offset;

}AK_ISP_MWB_ATTR_V2;


typedef  struct  ak_isp_awb_attr_V2        
{   
	T_U16	g_weight[16];
	T_U16	y_low;               //y_low<=y_high
	T_U16	y_high;
	T_U16	gr_low[5];           //gr_low[i]<=gr_high[i]
	T_U16	gb_low[5];           //gb_low[i]<=gb_high[i]
	T_U16	gr_high[5];
	T_U16	gb_high[5];
	T_U16	rb_low[5];          //rb_low[i]<=rb_high[i]
	T_U16	rb_high[5];
	 
    //awb软件部分需要设置的参数
    T_U16   auto_wb_step;                 //白平衡步长计算
	T_U16   total_cnt_thresh;         //像素个数阈值
	T_U16   colortemp_stable_cnt_thresh;    //稳定帧数，多少帧一样认为环境色温改变
}AK_ISP_AWB_ATTR_V2;

typedef  struct  ak_isp_awb_default_attr_V2        
{   
	T_U16	y_low;
	T_U16	y_high;

	T_U16	gr_low[5];
	T_U16	gb_low[5];
	T_U16	gr_high[5];
	T_U16	gb_high[5];
	T_U16	rb_low[5];
	T_U16	rb_high[5];
	 
}AK_ISP_AWB_DEFAULT_ATTR_V2;


typedef struct ak_isp_init_wb_V2
{   
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;
	AK_ISP_WB_TYPE_ATTR_V2			wb_type; 
	AK_ISP_MWB_ATTR_V2			p_mwb;
	AK_ISP_AWB_ATTR_V2			p_awb;
	AK_ISP_AWB_DEFAULT_ATTR_V2	p_awb_default;
}AK_ISP_INIT_WB_V2;


/*************************Expsoure****************************/

typedef struct ak_isp_raw_hist_attr_V2
{
	T_U16	enable;
}AK_ISP_RAW_HIST_ATTR_V2;

typedef struct ak_isp_rgb_hist_attr_V2
{
	T_U16	enable;
}AK_ISP_RGB_HIST_ATTR_V2;

typedef struct  ak_isp_yuv_hist_attr_V2
{
	T_U16	enable;
}AK_ISP_YUV_HIST_ATTR_V2;

typedef  struct  ak_isp_exp_type_V2
{
	T_U16	exp_type;  
}AK_ISP_EXP_TYPE_V2;


typedef  struct ak_isp_me_attr_V2
{
	T_U32  init_frame_rate;             //初始帧率                -->a_gain
    T_U32  init_frame_rate_exp_time;    //初始帧率下最大曝光时间  -->d_gain
    T_U32  init_frame_rate_plus;        //初始帧率下的增益        -->isp_d_gain
    T_U32  reduce_frame_rate;           //降帧帧率                -->exp_time
    T_U32  reduce_frame_exp_time;       //降帧帧率下最大曝光时间  -->a_gain_en
    T_U32  reduce_frame_plus;           //降帧增益                -->d_gain_en
    T_U32  other;                        //暂时无用               -->isp_d_gain_en
}AK_ISP_ME_ATTR_V2;

typedef struct ak_isp_ae_attr_V2
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
}AK_ISP_AE_ATTR_V2;

typedef struct ak_isp_init_exp_V2
{
	T_U16					param_id;      //参数id
	T_U16					length;        //参数长度;

	AK_ISP_RAW_HIST_ATTR_V2	p_raw_hist;
	AK_ISP_RGB_HIST_ATTR_V2	p_rgb_hist;

	AK_ISP_YUV_HIST_ATTR_V2	p_yuv_hist;
	AK_ISP_EXP_TYPE_V2			p_exp_type;

	AK_ISP_ME_ATTR_V2			p_me;
	AK_ISP_AE_ATTR_V2			p_ae;
}AK_ISP_INIT_EXP_V2;



/*************************杂项*****************************/

typedef struct ak_isp_misc_attr_V2
{
	T_U16	hsyn_pol;
	T_U16	vsync_pol;
	T_U16	test_pattern_en;
	T_U16	test_pattern_cfg;
	T_U16	cfa_mode;
    T_U16	one_line_cycle;
    T_U16	hblank_cycle;
    T_U16	frame_start_delay_en;
    T_U16	frame_start_delay_num;
} AK_ISP_MISC_ATTR_V2;  

typedef struct ak_isp_init_misc_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_MISC_ATTR_V2	p_misc;
}AK_ISP_INIT_MISC_V2;

/*************************sensor*****************************/

typedef struct ak_isp_sensor_attr_V2
{
	T_U16	sensor_addr;
	T_U16	sensor_value;
} AK_ISP_SENSOR_ATTR_V2;  

typedef struct ak_isp_init_sensor_V2
{
	T_U16				param_id;      //参数id
	T_U16				length;        //参数长度;
	AK_ISP_SENSOR_ATTR_V2	*p_sensor;
}AK_ISP_INIT_SENSOR_V2;

/**************************总结构体*******************************/

typedef struct ak_isp_init_param_V2
{
	//黑平衡结构体
	AK_ISP_INIT_BLC_V2			p_Isp_blc;
	//镜头校正结构体
	AK_ISP_INIT_LSC_V2			p_Isp_lsc;
	//raw gamma
	AK_ISP_INIT_RAW_LUT_V2		p_Isp_raw_lut;
	//NR
	AK_ISP_INIT_NR_V2			p_Isp_nr;
	//3DNR
	AK_ISP_INIT_3DNR_V2		p_Isp_3dnr;
	//绿平衡
	AK_ISP_INIT_GB_V2			p_Isp_gb;
	//demosaic
	AK_ISP_INIT_DEMO_V2		p_Isp_demo;
	//gamma
	AK_ISP_INIT_GAMMA_V2		p_Isp_gamma;
	//ccm
	AK_ISP_INIT_CCM_V2			p_Isp_ccm;
	//fcs
	AK_ISP_INIT_FCS_V2			p_Isp_fcs;
	//wdr
	AK_ISP_INIT_WDR_V2			p_Isp_wdr;
	//edge
	AK_ISP_INIT_EDGE_V2		p_Isp_edge;
	//sharp
	AK_ISP_INIT_SHARP_V2		p_Isp_sharp;
	//饱和度
	AK_ISP_INIT_SATURATION_V2	p_Isp_saturation;
	//对比度
	AK_ISP_INIT_CONTRAST_V2	p_Isp_contrast;
	//rgb to yuv
	AK_ISP_INIT_RGB2YUV_V2		p_Isp_rgb2yuv;
	//yuv 效果
	AK_ISP_INIT_EFFECT_V2		p_Isp_effect;
	//坏点校正
	AK_ISP_INIT_DPC_V2			p_Isp_dpc;
	//zone weight
	AK_ISP_INIT_WEIGHT_V2		p_Isp_weight;
	//af
	AK_ISP_INIT_AF_V2			p_Isp_af;
	//白平衡
	AK_ISP_INIT_WB_V2			p_Isp_wb;
	//expsoure
	AK_ISP_INIT_EXP_V2			p_Isp_exp;
	//杂项
	AK_ISP_INIT_MISC_V2		p_Isp_misc;

	//sensor
	AK_ISP_INIT_SENSOR_V2      p_Isp_sensor;

}AK_ISP_INIT_PARAM_V2;


#endif

