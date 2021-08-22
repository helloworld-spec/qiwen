#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#include "ak_isp_sdk.h"
#include "ak_isp_char.h"

#define AK_ISPSDK_VERSION		"V3.1.02"
#define AK_ISP_DEV_NAME 		"/dev/isp_char"

#define ISP_DEBUG
#ifdef ISP_DEBUG
#define ISP_PRINTF(stuff...)	printf("ISP: "stuff)
#else
#define ISP_PRINTF(fmt, args...)	do{}while(0)
#endif 

#define isp_info(stuff...)		printf("ISP: " stuff)

int g_fd = -1;

int isp_sdk_ioctl(unsigned int cmd, void *arg)
{
	if (!arg)
	   return AK_ERR_ISP_NULL_PTR;
	   
	if (g_fd < 0) {
		return AK_ERR_ISP_INVALID_FD;
	}
	
	int ret = ioctl(g_fd, cmd, arg);
	if (ret) {
		ISP_PRINTF("cmd=0x%X, err:%d, estr:%s\n", cmd, errno, strerror(errno));
	}

	return ret;
}

T_S32 AK_ISP_sdk_init(void)
{
	int ret = -1;
	int i = 0;
	int fd[3];

	printf("\nispsdk_lib version:%s \n", AK_ISPSDK_VERSION);

	for (i=0; i<3; i++)
		fd[i] = -1;

	/* make sure we get a valid fd */
	for (i = 0; i < 3; i++) {
		fd[i] = open("/dev/null", O_WRONLY);
		if (fd[i] >= 2)		/* fd ok */
			break;
		else if (fd[i] < 0) {	/* error */
			printf("%s open null fail\n", __func__);
			goto init_end;
		} else {
			//close(fd[i]);	
		}
	}

	g_fd = open(AK_ISP_DEV_NAME, O_RDWR);
	if (g_fd < 0) {
		printf("open %s fail, err:%d, estr:%s\n", 
			AK_ISP_DEV_NAME, errno, strerror(errno));
	} else {
		ret = 0;
		fcntl(g_fd, F_SETFD, FD_CLOEXEC); 
	}
	
	printf("--- %s g_fd=%d ---\n", __func__, g_fd);

	/* close */
	for (i = 0; i < 3; i++) {
		if (fd[i] >= 0)	{	/* close fd */
			printf(" 1 close fd:%d\n", fd[i]);
			close(fd[i]);
		}
	}

init_end:
	return ret;
}

T_S32 AK_ISP_sdk_exit(void)
{
	printf("--- %s g_fd=%d closed ---\n", __func__, g_fd);
	if (-1 != g_fd) {
		close(g_fd);
		g_fd = -1;
	}
	
	return 0;
}

T_S32 AK_ISP_set_raw_hist_attr(const AK_ISP_RAW_HIST_ATTR *p_raw_hist_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_RAW_HIST, (void *)p_raw_hist_attr);
}

T_S32 AK_ISP_get_raw_hist_attr(AK_ISP_RAW_HIST_ATTR *p_raw_hist_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_RAW_HIST, (void *)p_raw_hist_attr);
}

T_S32 AK_ISP_get_raw_hist_stat_info(AK_ISP_RAW_HIST_STAT_INFO *p_raw_hist_stat)
{
	return isp_sdk_ioctl(AK_ISP_GET_RAW_HIST_STAT, (void *)p_raw_hist_stat);
}

T_S32 AK_ISP_set_rgb_hist_attr(const  AK_ISP_RGB_HIST_ATTR *p_rgb_hist_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_RGB_HIST, (void *)p_rgb_hist_attr);
}

T_S32 AK_ISP_get_rgb_hist_attr(AK_ISP_RGB_HIST_ATTR *p_rgb_hist_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_RGB_HIST, (void *)p_rgb_hist_attr);
}

T_S32 AK_ISP_get_rgb_hist_stat_info(AK_ISP_RGB_HIST_STAT_INFO *p_rgb_hist_stat)
{
	return isp_sdk_ioctl(AK_ISP_GET_RGB_HIST_STAT, (void *)p_rgb_hist_stat);
}

T_S32 AK_ISP_set_yuv_hist_attr(const AK_ISP_YUV_HIST_ATTR *p_yuv_hist_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_Y_HIST, (void *)p_yuv_hist_attr);
}

T_S32 AK_ISP_get_yuv_hist_attr(AK_ISP_YUV_HIST_ATTR *p_yuv_hist_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_Y_HIST, (void *)p_yuv_hist_attr);
}

T_S32 AK_ISP_get_yuv_hist_stat_info(AK_ISP_YUV_HIST_STAT_INFO *p_yuv_hist_stat)
{
	return isp_sdk_ioctl(AK_ISP_GET_Y_HIST_STAT, (void *)p_yuv_hist_stat);
}

T_S32  AK_ISP_set_exp_type(const AK_ISP_EXP_TYPE* p_exp_type)
{
	return isp_sdk_ioctl(AK_ISP_SET_EXP_TYPE, (void *)p_exp_type);
}

T_S32  AK_ISP_get_exp_type(AK_ISP_EXP_TYPE* p_exp_type)
{
	return isp_sdk_ioctl(AK_ISP_GET_EXP_TYPE, (void *)p_exp_type);
}

T_S32 AK_ISP_set_ae_attr(const AK_ISP_AE_ATTR *p_ae_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_AE, (void *)p_ae_attr);
}

T_S32 AK_ISP_get_ae_attr(AK_ISP_AE_ATTR *p_ae_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_AE, (void *)p_ae_attr);
}

T_S32 AK_ISP_set_mae_attr(const AK_ISP_MAE_ATTR *p_mae_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_MAE, (void *)p_mae_attr);
}

T_S32 AK_ISP_get_mae_attr(AK_ISP_MAE_ATTR *p_mae_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_MAE, (void *)p_mae_attr);
}


T_S32 AK_ISP_get_ae_run_info(AK_ISP_AE_RUN_INFO *p_ae_stat)
{
	return isp_sdk_ioctl(AK_ISP_GET_AE_RUN_INFO, (void *)p_ae_stat);
}

T_S32 AK_ISP_set_wb_type(const AK_ISP_WB_TYPE_ATTR *p_type)
{
	return isp_sdk_ioctl(AK_ISP_SET_WB_TYPE, (void *)p_type);
}

T_S32 AK_ISP_get_wb_type(AK_ISP_WB_TYPE_ATTR *p_type)
{
	return isp_sdk_ioctl(AK_ISP_GET_WB_TYPE, (void *)p_type);
}

T_S32 AK_ISP_set_mwb_attr(const AK_ISP_MWB_ATTR *p_mwb_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_MWB, (void *)p_mwb_attr);
}

T_S32 AK_ISP_get_mwb_attr(AK_ISP_MWB_ATTR*p_mwb_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_MWB, (void *)p_mwb_attr);
}

T_S32 AK_ISP_set_awb_attr(const  AK_ISP_AWB_ATTR *p_awb_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_AWB, (void *)p_awb_attr);
}

T_S32 AK_ISP_get_awb_attr(AK_ISP_AWB_ATTR *p_awb_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_AWB, (void *)p_awb_attr);
}

T_S32 AK_ISP_set_awb_ex_attr(const  AK_ISP_AWB_EX_ATTR *p_awb_ex_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_AWB_EX, (void *)p_awb_ex_attr);
}

T_S32 AK_ISP_get_awb_ex_attr(AK_ISP_AWB_EX_ATTR *p_awb_ex_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_AWB_EX, (void *)p_awb_ex_attr);
}

T_S32 Ak_ISP_get_awb_stat_info(AK_ISP_AWB_STAT_INFO *p_awb_stat_info)
{
	return isp_sdk_ioctl(AK_ISP_GET_AWB_STAT_INFO, (void *)p_awb_stat_info);
}

T_S32 AK_ISP_set_af_attr(const AK_ISP_AF_ATTR *p_af_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_AF, (void *)p_af_attr);
}

T_S32 AK_ISP_set_af_win34_attr(const AK_ISP_AF_ATTR *p_af_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_AF_WIN34, (void *)p_af_attr);
}


T_S32 AK_ISP_get_af_attr(AK_ISP_AF_ATTR *p_af_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_AF, (void *)p_af_attr);
}

T_S32 AK_ISP_get_af_stat_info(AK_ISP_AF_STAT_INFO *p_af_satt_info)
{
	return isp_sdk_ioctl(AK_ISP_GET_AF_STAT, (void *)p_af_satt_info);
}

T_S32 AK_ISP_set_weight_attr(const AK_ISP_WEIGHT_ATTR *p_weight_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_WEIGHT, (void *)p_weight_attr);
}

T_S32 AK_ISP_get_weight_attr(AK_ISP_WEIGHT_ATTR *p_weight_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_WEIGHT, (void *)p_weight_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_blc_attr(const  AK_ISP_BLC_ATTR *p_blc_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_SET_BLC, (void *)p_blc_attr);
}

T_S32 AK_ISP_get_blc_attr(AK_ISP_BLC_ATTR *p_blc_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_GET_BLC, (void *)p_blc_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_dpc_attr(const AK_ISP_DDPC_ATTR *p_dpc)
{
	return isp_sdk_ioctl(AK_ISP_SET_DPC, (void *)p_dpc);
}

T_S32 AK_ISP_get_dpc_attr(AK_ISP_DDPC_ATTR* p_dpc)
{
	return isp_sdk_ioctl(AK_ISP_GET_DPC, (void *)p_dpc);
}

T_S32 AK_ISP_set_sdpc_attr(const AK_ISP_SDPC_ATTR *p_sdpc)
{
	return 0;
}

T_S32 AK_ISP_get_sdpc_attr(AK_ISP_SDPC_ATTR *p_sdpc)
{
	return 0;
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_lsc_attr(const AK_ISP_LSC_ATTR *p_lsc_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_SET_LSC, (void *)p_lsc_attr);
}

T_S32 AK_ISP_get_lsc_attr(AK_ISP_LSC_ATTR *p_lsc_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_GET_LSC, (void *)p_lsc_attr);
}

/***************************************************************************

******************************************************************************/
T_S32  AK_ISP_set_nr1_attr(const AK_ISP_NR1_ATTR *p_nr1_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_SET_RAW_NR1, (void *)p_nr1_attr);
}

T_S32  AK_ISP_get_nr1_attr(AK_ISP_NR1_ATTR* p_nr1_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_GET_RAW_NR1, (void *)p_nr1_attr);
}

T_S32 AK_ISP_set_nr2_attr(const AK_ISP_NR2_ATTR *p_nr2_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_Y_NR2, (void *)p_nr2_attr);
}

T_S32 AK_ISP_get_nr2_attr(AK_ISP_NR2_ATTR *p_nr2_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_Y_NR2, (void *)p_nr2_attr);
}

T_S32  AK_ISP_set_3d_nr_attr(const AK_ISP_3D_NR_ATTR * p_3d_nr_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_3D_NR, (void *)p_3d_nr_attr);
}

T_S32  AK_ISP_get_3d_nr_attr(AK_ISP_3D_NR_ATTR* p_3d_nr_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_3D_NR, (void *)p_3d_nr_attr);
}

T_S32  AK_ISP_set_3d_nr_ref_attr(const AK_ISP_3D_NR_REF_ATTR * p_3d_nr_ref_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_3D_NR_REF, (void *)p_3d_nr_ref_attr);
}

T_S32  AK_ISP_get_3d_nr_ref_attr(AK_ISP_3D_NR_REF_ATTR* p_3d_nr_ref_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_3D_NR_REF, (void *)p_3d_nr_ref_attr);
}

T_S32 AK_ISP_get_3d_nr_stat_info(AK_ISP_3D_NR_STAT_INFO * p_3d_nr_stat_info)
{
	return isp_sdk_ioctl(AK_ISP_GET_3D_NR_STAT_INFO, (void *)p_3d_nr_stat_info);
}

/***************************************************************************

******************************************************************************/
T_S32  AK_ISP_set_gb_attr(const AK_ISP_GB_ATTR *p_gb_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_SET_GB, (void *)p_gb_attr);
}

T_S32  AK_ISP_get_gb_attr(AK_ISP_GB_ATTR *p_gb_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_GET_GB, (void *)p_gb_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_demo_attr(const AK_ISP_DEMO_ATTR *p_demo_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_SET_DEMO, (void *)p_demo_attr);
}

T_S32 AK_ISP_get_demo_attr(AK_ISP_DEMO_ATTR *p_demo_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_GET_DEMO, (void *)p_demo_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_rgb_gamma_attr(const AK_ISP_RGB_GAMMA_ATTR *p_rgb_gamma_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_RGB_GAMMA, (void *)p_rgb_gamma_attr);
}

T_S32 AK_ISP_get_rgb_gamma_attr(AK_ISP_RGB_GAMMA_ATTR *p_rgb_gamma_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_RGB_GAMMA, (void *)p_rgb_gamma_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_raw_lut_attr(const AK_ISP_RAW_LUT_ATTR *p_raw_lut_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_SET_RAW_LUT, (void *)p_raw_lut_attr);
}

T_S32 AK_ISP_get_raw_lut_attr(AK_ISP_RAW_LUT_ATTR *p_raw_lut_attr)
{
	return isp_sdk_ioctl(AK_ISP_VP_GET_RAW_LUT, (void *)p_raw_lut_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_ccm_attr(const AK_ISP_CCM_ATTR *p_ccm)
{
	return isp_sdk_ioctl(AK_ISP_SET_CCM, (void *)p_ccm);
}

T_S32 AK_ISP_get_ccm_attr(AK_ISP_CCM_ATTR *p_ccm)
{
	return isp_sdk_ioctl(AK_ISP_GET_CCM, (void *)p_ccm);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_sharp_attr(const AK_ISP_SHARP_ATTR *p_sharp)
{
	return isp_sdk_ioctl(AK_ISP_SET_SHARP, (void *)p_sharp);
}

T_S32 AK_ISP_get_sharp_attr(AK_ISP_SHARP_ATTR* p_sharp)
{
	return isp_sdk_ioctl(AK_ISP_GET_SHARP, (void *)p_sharp);
}

T_S32 AK_ISP_set_sharp_ex_attr(const AK_ISP_SHARP_EX_ATTR *p_sharp_ex)
{
	return isp_sdk_ioctl(AK_ISP_SET_SHARP_EX, (void *)p_sharp_ex);
}

T_S32 AK_ISP_get_sharp_ex_attr(AK_ISP_SHARP_EX_ATTR* p_sharp_ex)
{
	return isp_sdk_ioctl(AK_ISP_GET_SHARP_EX, (void *)p_sharp_ex);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_fcs_attr(const AK_ISP_FCS_ATTR *p_fcs_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_FCS, (void *)p_fcs_attr);
}

T_S32 AK_ISP_get_fcs_attr(AK_ISP_FCS_ATTR *p_fcs_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_FCS, (void *)p_fcs_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_wdr_attr(const AK_ISP_WDR_ATTR *p_wdr_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_WDR, (void *)p_wdr_attr);
}

T_S32 AK_ISP_get_wdr_attr(AK_ISP_WDR_ATTR*p_wdr_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_WDR, (void *)p_wdr_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_contrast_attr(const AK_ISP_CONTRAST_ATTR  *p_contrast)
{
	return isp_sdk_ioctl(AK_ISP_SET_CONTRAST, (void *)p_contrast);
}

T_S32 AK_ISP_get_contrast_attr(AK_ISP_CONTRAST_ATTR  *p_contrast)
{
	return isp_sdk_ioctl(AK_ISP_GET_CONTRAST, (void *)p_contrast);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_saturation_attr(const AK_ISP_SATURATION_ATTR *p_sat)
{
	return isp_sdk_ioctl(AK_ISP_SET_SAT, (void *)p_sat);
}

T_S32 AK_ISP_get_saturation_attr(AK_ISP_SATURATION_ATTR *p_sat)
{
	return isp_sdk_ioctl(AK_ISP_GET_SAT, (void *)p_sat);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_rgb2yuv_attr(const AK_ISP_RGB2YUV_ATTR *p_rgb2yuv_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_RGB2YUV, (void *)p_rgb2yuv_attr);
}

T_S32 AK_ISP_get_rgb2yuv_attr(AK_ISP_RGB2YUV_ATTR *p_rgb2yuv_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_RGB2YUV, (void *)p_rgb2yuv_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_effect_attr(const AK_ISP_EFFECT_ATTR *p_effect_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_YUV_EFFECT, (void *)p_effect_attr);
}

T_S32 AK_ISP_get_effect_attr(AK_ISP_EFFECT_ATTR *p_effect_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_YUV_EFFECT, (void *)p_effect_attr);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_main_chan_mask_area(const AK_ISP_MAIN_CHAN_MASK_AREA_ATTR *p_mask)
{
	return isp_sdk_ioctl(AK_ISP_SET_MAIN_CHAN_MASK_AREA, (void *)p_mask);
}

T_S32 AK_ISP_get_main_chan_mask_area(AK_ISP_MAIN_CHAN_MASK_AREA_ATTR *p_mask)
{
	return isp_sdk_ioctl(AK_ISP_GET_MAIN_CHAN_MASK_AREA, (void *)p_mask);
}

T_S32 AK_ISP_set_sub_chan_mask_area(const AK_ISP_SUB_CHAN_MASK_AREA_ATTR *p_mask)
{
	return isp_sdk_ioctl(AK_ISP_SET_SUB_CHAN_MASK_AREA, (void *)p_mask);
}

T_S32 AK_ISP_get_sub_chan_mask_area(AK_ISP_SUB_CHAN_MASK_AREA_ATTR *p_mask)
{
	return isp_sdk_ioctl(AK_ISP_GET_SUB_CHAN_MASK_AREA, (void *)p_mask);
}

/***************************************************************************

******************************************************************************/
T_S32 AK_ISP_set_mask_color(const AK_ISP_MASK_COLOR_ATTR *p_mask)
{
	return isp_sdk_ioctl(AK_ISP_SET_MASK_COLOR, (void *)p_mask);
}

T_S32 AK_ISP_get_mask_color(AK_ISP_MASK_COLOR_ATTR *p_mask)
{
	return isp_sdk_ioctl(AK_ISP_GET_MASK_COLOR, (void *)p_mask);
}

/***************************************************************************

******************************************************************************/
T_S32 Ak_ISP_Sensor_Load_Conf(AK_ISP_SENSOR_INIT_PARA *sensor_params)
{
	return isp_sdk_ioctl(AK_ISP_INIT_SENSOR_DEV, (void *)sensor_params);
}

T_S32 Ak_ISP_Sensor_Set_Reg(AK_ISP_SENSOR_REG_INFO *reg_info)
{
	return isp_sdk_ioctl(AK_ISP_SET_SENSOR_REG, (void *)reg_info);
}

T_S32 Ak_ISP_Sensor_Get_Reg(AK_ISP_SENSOR_REG_INFO *reg_info)
{
	return isp_sdk_ioctl(AK_ISP_GET_SENSOR_REG, (void *)reg_info);
}

T_S32 Ak_ISP_Set_User_Params(AK_ISP_USER_PARAM *param)
{
	return isp_sdk_ioctl(AK_ISP_SET_USER_PARAMS, (void *)param);
}

T_S32 AK_ISP_set_misc_attr(const AK_ISP_MISC_ATTR *misc)
{
	return isp_sdk_ioctl(AK_ISP_SET_MISC_ATTR, (void *)misc);
}

T_S32 AK_ISP_get_misc_attr(AK_ISP_MISC_ATTR *misc)
{
	return isp_sdk_ioctl(AK_ISP_GET_MISC_ATTR, (void *)misc);
}

T_S32 AK_ISP_set_Y_gamma_attr(const AK_ISP_Y_GAMMA_ATTR *p_y_gamma_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_Y_GAMMA, (void *)p_y_gamma_attr);
}

T_S32 AK_ISP_get_Y_gamma_attr(AK_ISP_Y_GAMMA_ATTR *p_y_gamma_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_Y_GAMMA, (void *)p_y_gamma_attr);
}

T_S32 AK_ISP_set_hue_attr(const AK_ISP_HUE_ATTR *p_hue_attr)
{
	return isp_sdk_ioctl(AK_ISP_SET_HUE, (void *)p_hue_attr);
}

T_S32 AK_ISP_get_hue_attr(AK_ISP_HUE_ATTR *p_hue_attr)
{
	return isp_sdk_ioctl(AK_ISP_GET_HUE, (void *)p_hue_attr);
}

T_S32 AK_ISP_set_frame_rate(const AK_ISP_FRAME_RATE_ATTR *p_frame_rate)
{
	return isp_sdk_ioctl(AK_ISP_SET_FRAME_RATE, (void *)p_frame_rate);
}

T_S32 AK_ISP_get_frame_rate(AK_ISP_FRAME_RATE_ATTR *p_frame_rate)
{
	return isp_sdk_ioctl(AK_ISP_GET_FRAME_RATE, (void *)p_frame_rate);
}

T_S32 AK_ISP_set_isp_capturing(int resume)
{
	return isp_sdk_ioctl(AK_ISP_SET_ISP_CAPTURING, (void *)&resume);
}

T_S32 Ak_ISP_Sensor_Get_Id(int *id)
{
	return isp_sdk_ioctl(AK_ISP_GET_SENSOR_ID, (void *)id);
}

T_S32 Ak_ISP_Set_Flip_Mirror(struct isp_flip_mirror_info *info)
{
	return isp_sdk_ioctl(AK_ISP_SET_FLIP_MIRROR, (void *)info);
}

T_S32 Ak_ISP_Set_Sensor_Fps(const int *fps)
{
	return isp_sdk_ioctl(AK_ISP_SET_SENSOR_FPS, (void *)fps);
}

T_S32 Ak_ISP_Get_Sensor_Fps(void)
{
	int fps = 0;

	isp_sdk_ioctl(AK_ISP_GET_SENSOR_FPS, (void *)&fps);                          

	return fps;
}

T_S32 Ak_ISP_Get_Work_Scene(void)
{
	int scene = 0;

	isp_sdk_ioctl(AK_ISP_GET_WORK_SCENE, (void *)&scene);                            

	return scene;
}

T_S32 Ak_ISP_Get_ISO (void)
{
	int iso = 0;

	isp_sdk_ioctl (AK_ISP_GET_ISO, (void *)&iso);

	return iso;
}
