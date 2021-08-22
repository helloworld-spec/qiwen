#ifndef __AK_ISP_SDK_H__
#define __AK_ISP_SDK_H__

#include "ak_isp_drv.h"
#include "ak_isp_char.h"

typedef enum {
	AK_ERR_ISP_NULL_PTR=1,
	AK_ERR_ISP_INVALID_FD,
}ERR_FLAG;

enum ae_type {
	ISP_MANU_AE=0,
	ISP_AUTO_AE=1,
};

typedef struct ak_isp_function{
	T_U32 ADFAD;
}T_FUNCTION_HANDLER;

typedef  struct  ak_isp_day_night_attr {  
	int  a;
	int  b;
}AK_ISP_DAY_NIGHT_ATTR;

enum ak_isp_wb_type {
	WB_OPS_TYPE_MANU=0,        //手动白平衡
	WB_OPS_TYPE_AUTO,          //自动白平衡
};

/*
 * AK_ISP_sdk_init - init isp sdk, other functions must be called after init.
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_sdk_init(void);

/*
 * AK_ISP_sdk_exit - exit isp sdk
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_sdk_exit();

/*
 * AK_ISP_set_raw_hist_attr - set raw hist attr
 * p_raw_hist_attr[IN]: raw hist attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_raw_hist_attr(const AK_ISP_RAW_HIST_ATTR *p_raw_hist_attr);

/*
 * AK_ISP_get_raw_hist_attr - get raw hist attr
 * p_raw_hist_attr[OUT]: raw hist attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_raw_hist_attr(AK_ISP_RAW_HIST_ATTR *p_raw_hist_attr);

/*
 * AK_ISP_get_raw_hist_stat_info - get raw hist stat info 
 * p_raw_hist_stat[OUT]: raw hist stat
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_raw_hist_stat_info(AK_ISP_RAW_HIST_STAT_INFO *p_raw_hist_stat);

/*
 * AK_ISP_set_rgb_hist_attr - set rgb hist attr
 * p_rgb_hist_attr[IN]: rgb hist attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_rgb_hist_attr(const  AK_ISP_RGB_HIST_ATTR *p_rgb_hist_attr);

/*
 * AK_ISP_get_rgb_hist_attr - get rgb hist attr
 * p_rgb_hist_attr[OUT]: rgb hist attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_rgb_hist_attr(AK_ISP_RGB_HIST_ATTR *p_rgb_hist_attr);

/*
 * AK_ISP_get_rgb_hist_stat_info - get rgb hist stat info
 * p_rgb_hist_stat[OUT]: rgb hist stat
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_rgb_hist_stat_info(AK_ISP_RGB_HIST_STAT_INFO *p_rgb_hist_stat);

/*
 * AK_ISP_set_yuv_hist_attr - set yuv hist attr
 * p_yuv_hist_attr[IN]: yuv hist attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_yuv_hist_attr(const AK_ISP_YUV_HIST_ATTR *p_yuv_hist_attr);

/*
 * AK_ISP_get_yuv_hist_attr - get yuv hist attr
 * p_yuv_hist_attr[OUT]: yuv hist attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_yuv_hist_attr(AK_ISP_YUV_HIST_ATTR *p_yuv_hist_attr);

/*
 * AK_ISP_get_yuv_hist_stat_info - get yuv hist stat info
 * p_yuv_hist_stat[OUT]: yuv hist stat
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_yuv_hist_stat_info(AK_ISP_YUV_HIST_STAT_INFO *p_yuv_hist_stat);

/*
 * AK_ISP_set_exp_type - set exp type attr
 * p_exp_type[IN]: exp type attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_exp_type(const AK_ISP_EXP_TYPE* p_exp_type);

/*
 * AK_ISP_get_exp_type -get exp type attr
 * p_exp_type[OUT]: exp type attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_exp_type(AK_ISP_EXP_TYPE* p_exp_type);

/*
 * AK_ISP_set_frame_rate - set frame rate
 * p_frame_rate[IN]: frame rate attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_frame_rate(const AK_ISP_FRAME_RATE_ATTR *p_frame_rate);

/*
 * AK_ISP_get_frame_rate - get frame rate
 * p_frame_rate[OUT]: frame rate attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_frame_rate(AK_ISP_FRAME_RATE_ATTR *p_frame_rate);

/*
 * AK_ISP_set_ae_attr - set ae attr
 * p_ae_attr[IN]: ae attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_ae_attr(const AK_ISP_AE_ATTR *p_ae_attr);

/*
 * AK_ISP_get_ae_attr - get ae attr
 * p_ae_attr[OUT]: ae attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_ae_attr(AK_ISP_AE_ATTR *p_ae_attr);

/*
 * AK_ISP_set_mae_attr - set mae attr
 * p_mae_attr[IN]: mae attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_mae_attr(const AK_ISP_MAE_ATTR *p_mae_attr);

/*
 * AK_ISP_get_mae_attr - get mae attr
 * p_mae_attr[OUT]: mae attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_mae_attr(AK_ISP_MAE_ATTR *p_mae_attr);

/*
 * AK_ISP_get_ae_run_info - get ae run stat info
 * p_ae_stat[OUT]: ae stat
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_ae_run_info(AK_ISP_AE_RUN_INFO *p_ae_stat);

/*
 * AK_ISP_set_wb_type - set wb type attr
 * p_type[IN]: wb type attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_wb_type(const AK_ISP_WB_TYPE_ATTR *p_type);

/*
 * AK_ISP_get_wb_type - get wb type attr
 * p_type[OUT]: wb type attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_wb_type(AK_ISP_WB_TYPE_ATTR *p_type);

/*
 * AK_ISP_set_mwb_attr - set mwb attr
 * p_mwb_attr[IN]: mwb attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_mwb_attr(const AK_ISP_MWB_ATTR *p_mwb_attr);

/*
 * AK_ISP_get_mwb_attr - get mwb attr
 * p_mwb_attr[OUT]: mwb attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_mwb_attr(AK_ISP_MWB_ATTR*p_mwb_attr);

/*
 * AK_ISP_set_awb_attr - set awb attr
 * p_awb_attr[IN]: awb attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_awb_attr(const  AK_ISP_AWB_ATTR *p_awb_attr);

/*
 * AK_ISP_get_awb_attr - get awb attr
 * p_awb_attr[OUT]: awb attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_awb_attr(AK_ISP_AWB_ATTR *p_awb_attr);

/*
 * AK_ISP_set_awb_ex_attr - set awb ex attr
 * p_awb_ex_attr[IN]: awb ex attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_awb_ex_attr(const  AK_ISP_AWB_EX_ATTR *p_awb_ex_attr);

/*
 * AK_ISP_get_awb_ex_attr - get awb ex attr
 * p_awb_ex_attr[OUT]: awb ex attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_awb_ex_attr(AK_ISP_AWB_EX_ATTR *p_awb_ex_attr);

/*
 * Ak_ISP_get_awb_stat_info - get awb stat info
 * p_awb_stat_info[OUT]: awb stat info
 * return: 0 success, -1 failed
 */
T_S32 Ak_ISP_get_awb_stat_info   (AK_ISP_AWB_STAT_INFO *p_awb_stat_info);

/*
 * AK_ISP_set_af_attr - set af attr
 * p_af_attr[IN]: af attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_af_attr(const AK_ISP_AF_ATTR *p_af_attr);

/*
 * AK_ISP_set_af_attr - set af win3 and win4 attr
 * p_af_attr[IN]: af attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_af_win34_attr(const AK_ISP_AF_ATTR *p_af_attr);

/*
 * AK_ISP_get_af_attr - get af attr
 * p_af_attr[OUT]: af attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_af_attr(AK_ISP_AF_ATTR *p_af_attr);

/*
 * AK_ISP_get_af_stat_info - get af stat info
 * p_af_satt_info[OUT]: af stat info
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_af_stat_info(AK_ISP_AF_STAT_INFO *p_af_satt_info);

/*
 * AK_ISP_set_weight_attr - set weight attr
 * p_weight_attr[IN]: weight attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_weight_attr(const AK_ISP_WEIGHT_ATTR *p_weight_attr);
/*
 * AK_ISP_get_weight_attr - get weight attr
 * p_weight_attr[OUT]: weight attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_weight_attr(AK_ISP_WEIGHT_ATTR *p_weight_attr);


/*
 * AK_ISP_set_blc_attr - set blc attr
 * p_blc_attr[IN]: blc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_blc_attr(const  AK_ISP_BLC_ATTR *p_blc_attr);

/*
 * AK_ISP_get_blc_attr - get blc attr
 * p_blc_attr[IN]: blc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_blc_attr(AK_ISP_BLC_ATTR *p_blc_attr);

/*
 * AK_ISP_set_dpc_attr - set dpc attr
 * p_dpc[IN]: dpc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_dpc_attr(const AK_ISP_DDPC_ATTR *p_dpc);

/*
 * AK_ISP_get_dpc_attr - get dpc attr
 * p_dpc[OUT]: dpc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_dpc_attr(AK_ISP_DDPC_ATTR* p_dpc);

/*
 * AK_ISP_set_sdpc_attr - set sdpc attr
 * p_sdpc[IN]: sdpc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_sdpc_attr(const AK_ISP_SDPC_ATTR *p_sdpc);

/*
 * AK_ISP_get_sdpc_attr - get sdpc attr
 * p_sdpc[OUT]: sdpc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_sdpc_attr(AK_ISP_SDPC_ATTR *p_sdpc);

/*
 * AK_ISP_set_lsc_attr - set lsc attr
 * p_lsc_attr[IN]: lsc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_lsc_attr(const AK_ISP_LSC_ATTR *p_lsc_attr);

/*
 * AK_ISP_get_lsc_attr - get lsc attr
 * p_lsc_attr[OUT]: lsc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_lsc_attr(AK_ISP_LSC_ATTR *p_lsc_attr);

/*
 * AK_ISP_set_nr1_attr - set nr1 attr
 * p_nr1_attr[IN]: nr1 attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_nr1_attr(const AK_ISP_NR1_ATTR *p_nr1_attr);

/*
 * AK_ISP_get_nr1_attr - get nr1 attr
 * p_nr1_attr[OUT]: nr1 attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_nr1_attr(AK_ISP_NR1_ATTR* p_nr1_attr);

/*
 * AK_ISP_set_nr2_attr - set nr2 attr
 * p_nr2_attr[IN]: nr2 attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_nr2_attr(const AK_ISP_NR2_ATTR *p_nr2_attr);

/*
 * AK_ISP_get_nr2_attr - get nr2 attr
 * p_nr2_attr[OUT]: nr2 attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_nr2_attr(AK_ISP_NR2_ATTR *p_nr2_attr);

/*
 * AK_ISP_set_3d_nr_attr - set 3dnr attr
 * p_3d_nr_attr[IN]: 3dnr attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_3d_nr_attr(const AK_ISP_3D_NR_ATTR * p_3d_nr_attr);

/*
 * AK_ISP_get_3d_nr_attr - get 3dnr attr
 * p_3d_nr_attr[OUT]: 3dnr attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_3d_nr_attr(AK_ISP_3D_NR_ATTR* p_3d_nr_attr);

/*
 * AK_ISP_set_3d_nr_ref_attr - set 3dnr ref attr
 * p_3d_nr_ref_attr[IN]: 3dnr ref attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_3d_nr_ref_attr(const AK_ISP_3D_NR_REF_ATTR * p_3d_nr_ref_attr);

/*
 * AK_ISP_get_3d_nr_ref_attr - get 3dnr ref attr
 * p_3d_nr_ref_attr[OUT]: 3dnr ref attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_3d_nr_ref_attr(AK_ISP_3D_NR_REF_ATTR* p_3d_nr_ref_attr);

/*
 * AK_ISP_get_3d_nr_stat_info - get 3dnr stat info
 * p_3d_nr_stat_info[OUT]: 3dnr stat info
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_3d_nr_stat_info(AK_ISP_3D_NR_STAT_INFO * p_3d_nr_stat_info);

/*
 * AK_ISP_set_gb_attr - set gb attr
 * p_gb_attr[IN]: gb attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_gb_attr(const AK_ISP_GB_ATTR *p_gb_attr);

/*
 * AK_ISP_get_gb_attr - get gb attr
 * p_gb_attr[OUT]: gb attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_gb_attr(AK_ISP_GB_ATTR *p_gb_attr);

/*
 * AK_ISP_set_demo_attr - set demo attr
 * p_demo_attr[IN]: demo attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_demo_attr(const AK_ISP_DEMO_ATTR *p_demo_attr);

/*
 * AK_ISP_get_demo_attr - get demo attr
 * p_demo_attr[OUT]: demo attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_demo_attr(AK_ISP_DEMO_ATTR *p_demo_attr);

/*
 * AK_ISP_set_rgb_gamma_attr - set rgb gamma attr
 * p_rgb_gamma_attr[IN]: rgb gamma attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_rgb_gamma_attr(const AK_ISP_RGB_GAMMA_ATTR *p_rgb_gamma_attr);

/*
 * AK_ISP_get_rgb_gamma_attr - get rgb gamma attr
 * p_rgb_gamma_attr[OUT]: rgb gamma attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_rgb_gamma_attr(AK_ISP_RGB_GAMMA_ATTR *p_rgb_gamma_attr);

/*
 * AK_ISP_set_Y_gamma_attr - set Y gamma attr
 * p_y_gamma_attr[IN]: Y gamma attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_Y_gamma_attr(const AK_ISP_Y_GAMMA_ATTR *p_y_gamma_attr);

/*
 * AK_ISP_get_Y_gamma_attr - get Y gamma attr
 * p_y_gamma_attr[OUT]: Y gamma attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_Y_gamma_attr(AK_ISP_Y_GAMMA_ATTR *p_y_gamma_attr);

/*
 * AK_ISP_set_raw_lut_attr - set raw lut attr
 * p_raw_lut_attr[IN]: raw lut attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_raw_lut_attr(const AK_ISP_RAW_LUT_ATTR *p_raw_lut_attr);

/*
 * AK_ISP_get_raw_lut_attr - get raw lut attr
 * p_raw_lut_attr[OUT]: raw lut attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_raw_lut_attr(AK_ISP_RAW_LUT_ATTR *p_raw_lut_attr);

/*
 * AK_ISP_set_ccm_attr - set ccm attr
 * p_ccm[IN]: ccm attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_ccm_attr(const AK_ISP_CCM_ATTR *p_ccm);

/*
 * AK_ISP_get_ccm_attr - get ccm attr
 * p_ccm[OUT]: ccm attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_ccm_attr(AK_ISP_CCM_ATTR *p_ccm);

/*
 * AK_ISP_set_sharp_attr - set sharp attr
 * p_sharp[IN]: sharp attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_sharp_attr(const AK_ISP_SHARP_ATTR *p_sharp);

/*
 * AK_ISP_get_sharp_attr - get sharp attr
 * p_sharp[OUT]: sharp attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_sharp_attr(AK_ISP_SHARP_ATTR* p_sharp);

/*
 * AK_ISP_set_sharp_ex_attr - set sharp ex attr
 * p_sharp_ex[IN]: sharp ex attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_sharp_ex_attr(const AK_ISP_SHARP_EX_ATTR *p_sharp_ex);

/*
 * AK_ISP_get_sharp_ex_attr - get sharp ex attr
 * p_sharp_ex[OUT]: sharp ex attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_sharp_ex_attr(AK_ISP_SHARP_EX_ATTR* p_sharp_ex);

/*
 * AK_ISP_set_fcs_attr - set fcs attr
 * p_fcs_attr[IN]: fcs attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_fcs_attr(const AK_ISP_FCS_ATTR *p_fcs_attr);

/*
 * AK_ISP_get_fcs_attr - get fcs attr
 * p_fcs_attr[OUT]: fcs attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_fcs_attr(AK_ISP_FCS_ATTR *p_fcs_attr);

/*
 * AK_ISP_set_wdr_attr - set wdr attr
 * p_wdr_attr[IN]: wdr attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_wdr_attr(const AK_ISP_WDR_ATTR *p_wdr_attr);

/*
 * AK_ISP_get_wdr_attr - get wdr attr
 * p_wdr_attr[OUT]: wdr attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_wdr_attr(AK_ISP_WDR_ATTR*p_wdr_attr);

/*
 * AK_ISP_set_contrast_attr - set contrast attr
 * p_contrast[IN]: contrast attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_contrast_attr(const AK_ISP_CONTRAST_ATTR  *p_contrast);

/*
 * AK_ISP_get_contrast_attr - get contrast attr
 * p_contrast[OUT]: contrast attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_contrast_attr(AK_ISP_CONTRAST_ATTR  *p_contrast);

/*
 * AK_ISP_set_saturation_attr - set saturation attr
 * p_sat[IN]: saturation attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_saturation_attr(const AK_ISP_SATURATION_ATTR *p_sat);

/*
 * AK_ISP_get_saturation_attr - get saturation attr
 * p_sat[OUT]: saturation attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_saturation_attr(AK_ISP_SATURATION_ATTR *p_sat);

/*
 * AK_ISP_set_rgb2yuv_attr - set rgb2yuv attr
 * p_rgb2yuv_attr[IN]: rgb2yuv attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_rgb2yuv_attr(const AK_ISP_RGB2YUV_ATTR *p_rgb2yuv_attr);

/*
 * AK_ISP_get_rgb2yuv_attr - get rgb2yuv attr
 * p_rgb2yuv_attr[OUT]: rgb2yuv attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_rgb2yuv_attr(AK_ISP_RGB2YUV_ATTR *p_rgb2yuv_attr);

/*
 * AK_ISP_set_hue_attr - set hue attr
 * p_hue_attr[IN]: hue attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_hue_attr(const  AK_ISP_HUE_ATTR *p_hue_attr);

/*
 * AK_ISP_get_hue_attr - get hue attr
 * p_hue_attr[OUT]: hue attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_hue_attr(AK_ISP_HUE_ATTR *p_hue_attr);

/*
 * AK_ISP_set_effect_attr - set effect attr
 * p_effect_attr[IN]: effect attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_effect_attr(const AK_ISP_EFFECT_ATTR *p_effect_attr);

/*
 * AK_ISP_get_effect_attr - get effect attr
 * p_effect_attr[OUT]: effect attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_effect_attr(AK_ISP_EFFECT_ATTR *p_effect_attr);

/*
 * AK_ISP_set_main_chan_mask_area - set main channel mask area
 * p_mask[IN]: mask area attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_main_chan_mask_area(const AK_ISP_MAIN_CHAN_MASK_AREA_ATTR *p_mask);

/*
 * AK_ISP_get_main_chan_mask_area - get main channel mask area
 * p_mask[OUT]: mask area attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_main_chan_mask_area(AK_ISP_MAIN_CHAN_MASK_AREA_ATTR *p_mask);

/*
 * AK_ISP_set_sub_chan_mask_area - set sub channel mask area
 * p_mask[IN]: mask area attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_sub_chan_mask_area(const AK_ISP_SUB_CHAN_MASK_AREA_ATTR *p_mask);

/*
 * AK_ISP_get_sub_chan_mask_area - get sub channel mask area
 * p_mask[OUT]: mask area attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_sub_chan_mask_area(AK_ISP_SUB_CHAN_MASK_AREA_ATTR *p_mask);

/*
 * AK_ISP_set_mask_color - set mask color
 * p_mask[IN]: mask color attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_mask_color(const AK_ISP_MASK_COLOR_ATTR *p_mask);

/*
 * AK_ISP_get_mask_color - get mask color
 * p_mask[OUT]: mask color attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_mask_color(AK_ISP_MASK_COLOR_ATTR *p_mask);

/*
 * AK_ISP_set_misc_attr - set misc color
 * misc[IN]: misc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_set_misc_attr(const AK_ISP_MISC_ATTR *misc);

/*
 * AK_ISP_get_misc_attr - get misc color
 * misc[OUT]: misc attr
 * return: 0 success, -1 failed
 */
T_S32 AK_ISP_get_misc_attr(AK_ISP_MISC_ATTR *misc);

/*
 * Ak_ISP_Sensor_Load_Conf - load sensor params
 * sensor_params[OUT]: sensor params
 * return: 0 success, -1 failed
 */
T_S32 Ak_ISP_Sensor_Load_Conf(AK_ISP_SENSOR_INIT_PARA *sensor_params);

/*
 * Ak_ISP_Sensor_Set_Reg - set sensor register value
 * reg_info[IN]: register info
 * return: 0 success, -1 failed
 */
T_S32 Ak_ISP_Sensor_Set_Reg(AK_ISP_SENSOR_REG_INFO *reg_info);

/*
 * Ak_ISP_Sensor_Get_Reg - get sensor register value by addr
 * reg_info[IN/OUT]: register info
 * return: 0 success, -1 failed
 */
T_S32 Ak_ISP_Sensor_Get_Reg(AK_ISP_SENSOR_REG_INFO *reg_info);

/*
 * Ak_ISP_Set_User_Params - set user params
 * param[IN]: user params
 * return: 0 success, -1 failed
 */
T_S32 Ak_ISP_Set_User_Params(AK_ISP_USER_PARAM *param);

/*
 * Ak_ISP_Sensor_Get_Id - get sensor id
 * id[OUT]: sensor id
 * return: 0 success, -1 failed
 */
T_S32 Ak_ISP_Sensor_Get_Id(int *id);

T_S32 AK_ISP_set_isp_capturing(int resume);

/*
 * Ak_ISP_Set_Flip_Mirror - set flip mirror
 * info[IN]: flip mirror info
 * return: 0 success, -1 failed
 */
T_S32 Ak_ISP_Set_Flip_Mirror(struct isp_flip_mirror_info *info);

/*
 * Ak_ISP_Set_Sensor_Fps - set sensor fps
 * fps[IN]: sensor fps
 * return: 0 success, -1 failed
 */
T_S32 Ak_ISP_Set_Sensor_Fps(const int *fps);

/*
 * Ak_ISP_Get_Sensor_Fps - get sensor fps
 * return: sensor fps
 */
T_S32 Ak_ISP_Get_Sensor_Fps(void);

/*
 * Ak_ISP_Get_Work_Scene - get work scene, indoor or outdoor
 * return: work scene, 0 indoor, 1 outdoor
 */
T_S32 Ak_ISP_Get_Work_Scene(void);

/*
 * get iso value.
 * return: iso value.
 */
T_S32 Ak_ISP_Get_ISO (void);

#endif
